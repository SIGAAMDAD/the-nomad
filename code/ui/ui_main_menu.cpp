/*
===========================================================================
Copyright (C) 2023-2024 GDR Games

This file is part of The Nomad source code.

The Nomad source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

The Nomad source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Foobar; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/

#include "ui_lib.h"
#include "../game/g_archive.h"
#include "../rendercommon/imgui_impl_opengl3.h"
#include <curl/curl.h>
#include <curl/easy.h>
#include <pthread.h>

#define ID_SINGEPLAYER      1
#define ID_MODS             2
#define ID_SETTINGS         3
#define ID_DATABASE         4
#define ID_CREDITS          5
#define ID_EXIT             6
#define ID_TABLE            7

//#define UI_FAST_EDIT

#define NEWS_FILE "Cache/newsfeed.dat"

#ifdef UI_FAST_EDIT
#include "rendercommon/imgui_internal.h"
#endif

#define EPILEPSY_WARNING_TITLE "WARNING: READ BEFORE PLAYING"

typedef struct {
	menuframework_t menu;
	const CModuleCrashData *crashData;
	char message[MAXPRINTMSG];
} errorMessage_t;

static qboolean playedSplashScreen = qfalse;

typedef struct newslabel_s {
	const char *date;
	const char *title;
	const char *body;
	const char *url;
	const char *image;
} newslabel_t;

typedef struct {
	menuframework_t menu;

	menutext_t singleplayer;
	menutext_t mods;
	menutext_t settings;
	menutext_t database;
	menutext_t credits;
	menutext_t exitGame;

	ImFont *font;

	nhandle_t background;
	qboolean noSaves;
	qboolean noMenu; // do we just want the scenery?
} mainmenu_t;

#define SPLASH_SCREEN_LOGO      0
#define SPLASH_SCREEN_WARNING   1

typedef struct {
	menuframework_t menu;

	nhandle_t companyLogoShader;
	nhandle_t engineLogoShader;
	nhandle_t fmodLogoShader;
	int splashPhase; // company logo -> engine logo -> epilepsy warning

	uint64_t timeStart;
	uint64_t lifeTime;
} splashScreenMenu_t;

static eastl::fixed_vector<newslabel_t, 5> s_newsLabelList;
static uint64_t s_nCurrentNewsLabel;
static pthread_t s_newsThread;
static errorMessage_t *s_errorMenu;
static mainmenu_t *s_main;
static splashScreenMenu_t *s_splashScreen;

static void MainMenu_EventCallback( void *item, int event )
{
	const menucommon_t *self;

	if ( event != EVENT_ACTIVATED ) {
		return;
	}

	self = (const menucommon_t *)item;

	switch ( self->id ) {
	case ID_SINGEPLAYER:
		UI_PlayMenu();
		break;
	case ID_MODS:
		UI_ModsMenu();
		break;
	case ID_SETTINGS:
		UI_SettingsMenu();
		break;
	case ID_DATABASE:
		UI_DataBaseMenu();
		break;
	case ID_EXIT:
		Cbuf_ExecuteText( EXEC_APPEND, "quit\n" );
		break;
	case ID_TABLE:
		break;
	default:
		N_Error( ERR_DROP, "MainMeu_EventCallback: unknown item id %i", self->id );
	};
}

static void TextCenterAlign( const char *text )
{
	float fontSize = ImGui::GetFontSize() * strlen( text ) / 2;
	ImGui::SameLine( ImGui::GetWindowSize().x / 2 - fontSize + ( fontSize / 2 ) );
	ImGui::TextUnformatted( text );
	ImGui::NewLine();
}

static void DrawNewsFeed( void )
{
	int i;
	const newslabel_t *label;

	ImGui::Begin( "##MainMenuNewsFeed", NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse );
	ImGui::SetWindowPos( ImVec2( 728 * ui->scale + ui->bias, 72 * ui->scale ) );
	ImGui::SetWindowSize( ImVec2( 290 * ui->scale + ui->bias, 460 * ui->scale ) );

	ImGui::SetWindowFontScale( ( ImGui::GetFont()->Scale * 1.5f ) * ui->scale );
	ImGui::SeparatorText( "NEWS" );
	ImGui::SetWindowFontScale( ImGui::GetFont()->Scale );

	FontCache()->SetActiveFont( RobotoMono );

	ImGui::SetWindowFontScale( ( ImGui::GetFont()->Scale * 1.90f ) * ui->scale );
	{
		ImGui::SeparatorText( s_newsLabelList[ s_nCurrentNewsLabel ].title );
		ImGui::NewLine();
		ImGui::SetWindowFontScale( ImGui::GetFont()->Scale );
		ImGui::Image( (ImTextureID)(uintptr_t)re.RegisterShader( s_newsLabelList[ s_nCurrentNewsLabel ].image ),
			ImVec2( 128 * ui->scale + ui->bias, 128 * ui->scale ) );
		if ( ImGui::IsItemClicked( ImGuiMouseButton_Left ) && s_newsLabelList[ s_nCurrentNewsLabel ].url ) {
			if ( !SDL_OpenURL( s_newsLabelList[ s_nCurrentNewsLabel ].url ) ) {
				Con_Printf( COLOR_RED "Error SDL_OpenURL failed: %s\n", SDL_GetError() );
			}
		}
		UI_DrawText( s_newsLabelList[ s_nCurrentNewsLabel ].body );
	}

	ImGui::NewLine();
	ImGui::Separator();
	{
		ImGui::BeginTable( "##NewsFeedSelectorTable", 3 );
		
		ImGui::TableNextColumn();
		ImGui::ArrowButton( "##MainMenuNewsFeedLeftArrow", ImGuiDir_Left );
		if ( ImGui::IsItemClicked( ImGuiMouseButton_Left ) ) {
			if ( s_nCurrentNewsLabel == 0 ) {
				s_nCurrentNewsLabel = s_newsLabelList.size() - 1;
			} else {
				s_nCurrentNewsLabel--;
			}
			Snd_PlaySfx( ui->sfx_select );
		}
		ImGui::TableNextColumn();

		for ( i = 0; i < s_newsLabelList.size(); i++ ) {
			ImGui::SameLine();
			ImGui::RadioButton( va( "##NewsLabelIndex%s", s_newsLabelList[ s_nCurrentNewsLabel ].title ), i == s_nCurrentNewsLabel );
			if ( ImGui::IsItemClicked( ImGuiMouseButton_Left ) ) {
				s_nCurrentNewsLabel = i;
				Snd_PlaySfx( ui->sfx_select );
			}
		}
		ImGui::TableNextColumn();
		ImGui::ArrowButton( "##NewsFeedRightArrow", ImGuiDir_Right );
		if ( ImGui::IsItemClicked( ImGuiMouseButton_Left ) ) {
			if ( s_nCurrentNewsLabel == s_newsLabelList.size() - 1 ) {
				s_nCurrentNewsLabel = 0;
			} else {
				s_nCurrentNewsLabel++;
			}
			Snd_PlaySfx( ui->sfx_select );
		}
		ImGui::EndTable();
	}

	ImGui::End();
}

extern void Menu_DrawItemGeneric( menucommon_t *generic );

static void DrawMenu_Text( void )
{
	int i;
	menuframework_t *menu;

	menu = &s_main->menu;

	ImGui::Begin( va( "%s##%sMainMenu", menu->name, menu->name ), NULL, menu->flags );
	if ( !( Key_GetCatcher() & KEYCATCH_CONSOLE ) ) {
		ImGui::SetWindowFocus();
	}
	ImGui::SetWindowPos( ImVec2( menu->x, menu->y ) );
	ImGui::SetWindowSize( ImVec2( menu->width, menu->height ) );

	UI_EscapeMenuToggle();
	if ( UI_MenuTitle( menu->name, menu->titleFontScale ) ) {
		UI_PopMenu();
		Snd_PlaySfx( ui->sfx_back );

		ImGui::End();
		return;
	}

	ImGui::SetCursorScreenPos( ImVec2( ImGui::GetCursorScreenPos().x, 400 * ui->scale ) );
	for ( i = 0; i < menu->nitems; i++ ) {
		Menu_DrawItemGeneric( (menucommon_t *)menu->items[i] );
	}

	ImGui::End();

	DrawNewsFeed();

	//
	// draw the version
	//
	FontCache()->SetActiveFont( RobotoMono );

	ImGui::Begin( "MainMenuVersion", NULL, MENU_DEFAULT_FLAGS | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_AlwaysAutoResize );
	ImGui::SetWindowFontScale( ImGui::GetFont()->Scale * 1.5f );
	ImGui::SetWindowPos( ImVec2( 600 * ui->scale + ui->bias, 700 * ui->scale ) );
	if ( ui->demoVersion ) {
		ImGui::TextUnformatted( "(DEMO) FOR MATURE AUDIENCES" );
	} else {
		ImGui::NewLine();
	}
	ImGui::TextUnformatted( GLN_VERSION );
	ImGui::End();
}

static void DrawMenu_Blocks( void )
{
	const int windowFlags = MENU_DEFAULT_FLAGS & ~( ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize );

	ImGui::Begin( "##MainMenuCampaignWidget", NULL, windowFlags );
	if ( s_main->noSaves ) {
		ImGui::TextUnformatted( "start a new game" );
	}
#ifdef UI_FAST_EDIT
	ImGui::InputFloat2( "Position##MainMenuCampaignPositionWidget", (float *)&ImGui::GetCurrentWindow()->Pos );
	ImGui::InputFloat2( "Size##MainMenuCampaignSizeWidget", (float *)&ImGui::GetCurrentWindow()->Size );
#endif
	ImGui::End();

	ImGui::Begin( "##MainMenuModsWidget", NULL, windowFlags );
	ImGui::TextUnformatted( "additional content" );
#ifdef UI_FAST_EDIT
	ImGui::InputFloat2( "Position##MainMenuModsPositionWidget", (float *)&ImGui::GetCurrentWindow()->Pos );
	ImGui::InputFloat2( "Size##MainMenuModsSizeWidget", (float *)&ImGui::GetCurrentWindow()->Size );
#endif
	ImGui::End();

	ImGui::Begin( "##MainMenuPlayerWidget", NULL, windowFlags );
	ImGui::TextUnformatted( "Player" );
	ImGui::End();

	ImGui::Begin( "##MainMenuSettingsWidget", NULL, windowFlags );
	ImGui::TextUnformatted( "Settings" );
	ImGui::End();

	ImGui::Begin( "##MainMenuExitWidget", NULL, windowFlags );
	ImGui::PushStyleColor( ImGuiCol_Button, ImVec4( 0.0f, 0.0f, 0.0f, 0.0f ) );
	ImGui::PushStyleColor( ImGuiCol_ButtonActive, ImVec4( 0.0f, 0.0f, 0.0f, 0.0f ) );
	ImGui::PushStyleColor( ImGuiCol_ButtonHovered, ImVec4( 0.0f, 0.0f, 0.0f, 0.0f ) );
#ifdef UI_FAST_EDIT
	ImGui::InputFloat2( "Position##MainMenuExitPositionWidget", (float *)&ImGui::GetCurrentWindow()->Pos );
	ImGui::InputFloat2( "Size##MainMenuExitSizeWidget", (float *)&ImGui::GetCurrentWindow()->Size );
#endif
	if ( ImGui::Button( "exit", ImVec2( 150 * ui->scale, 100 * ui->scale ) ) ) {
		Cbuf_ExecuteText( EXEC_APPEND, "quit\n" );
	}
	ImGui::PopStyleColor( 3 );
	ImGui::End();
}

void MainMenu_Draw( void )
{
	ui->menubackShader = s_main->background;

	if ( s_main->font ) {
		FontCache()->SetActiveFont( s_main->font );
	}

	if ( s_main->noMenu ) {
		return; // just the scenery & the music
	}

	// show the user WTF just happened
	if ( s_errorMenu->message[0] || ui->activemenu == &s_errorMenu->menu ) {
		FontCache()->SetActiveFont( FontCache()->AddFontToCache( "RobotoMono-Bold" ) );
		ImGui::Begin( "Game Error", NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize
			| ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoMove );
		ImGui::SetWindowPos( ImVec2( s_errorMenu->menu.x * ui->scale, s_errorMenu->menu.y * ui->scale ) );
		ImGui::SetWindowFontScale( ImGui::GetFont()->Scale * 1.5f );
		ui->menubackShader = re.RegisterShader( "menu/mainbackground" );
		ImGui::TextUnformatted( s_errorMenu->message );
		if ( Key_IsDown( KEY_ESCAPE ) || ImGui::IsItemClicked( ImGuiMouseButton_Left ) ) {
			Snd_PlaySfx( ui->sfx_select );
			Cvar_Set( "com_errorMessage", "" );
			UI_PopMenu();
			UI_MainMenu();
			ImGui::End();
			return;
		}
		ImGui::End();
		return;
	} else {
		switch ( ui_menuStyle->i ) {
		case 0:
			DrawMenu_Text();
			break;
		case 1:
			DrawMenu_Blocks();
			break;
		default:
			Con_Printf( COLOR_YELLOW "WARNING: bad ui_menuStyle %i\n", ui_menuStyle->i );
			Cvar_Set( "ui_menuStyle", "0" );
			break;
		};
	}
}

static void SplashScreen_Draw( void )
{
	refdef_t refdef;

	const uint64_t timeCurrent = Sys_Milliseconds();
	static const char *epilespyWarning[] = {
		"A very small percentage of individuals may experience epileptic",
		"seizures when exposed to certain light patterns or flashing lights.",
		"",
		"Exposure to certain patterns or backgrounds on a computer screen, or",
		"while playing video games, may induce an epileptic seizure in",
		"these individuals. Certain conditions may induce previously undetected",
		"epileptic symptoms even in persons who have no history of",
		"prior seizures of epilepsy.",
		"",
		"Please exercise caution."
	};

	if ( timeCurrent - s_splashScreen->timeStart > s_splashScreen->lifeTime ) {
		if ( s_splashScreen->splashPhase >= SPLASH_SCREEN_WARNING ) {
			playedSplashScreen = qtrue;
			MainMenu_Cache();
			return;
		} else {
			s_splashScreen->splashPhase++;
			s_splashScreen->timeStart = timeCurrent;
		}
	}

	re.SetColor( NULL );

	switch ( s_splashScreen->splashPhase ) {
	case SPLASH_SCREEN_LOGO: { // get attribution to 3rd party stuff done with
		int cursorX = 80;

		re.DrawImage( cursorX * ui->scale, 100 * ui->scale, 330 * ui->scale + ui->bias, 180 * ui->scale, 0, 0, 1, 1, s_splashScreen->companyLogoShader );
		cursorX += 300 * ui->scale + ui->bias;

		re.DrawImage( cursorX * ui->scale + ui->bias, 100 * ui->scale, 340 * ui->scale + ui->bias, 220 * ui->scale, 0, 0, 1, 1, s_splashScreen->engineLogoShader );
		cursorX = 200 * ui->scale + ui->bias;

		re.DrawImage( cursorX * ui->scale + ui->bias, 300 * ui->scale, 356 * ui->scale + ui->bias, 126 * ui->scale, 0, 0, 1, 1, s_splashScreen->fmodLogoShader );

		ImGui::Begin( "##SplashScreen", NULL, MENU_DEFAULT_FLAGS );
		ImGui::SetWindowPos( ImVec2( s_splashScreen->menu.x,  472 * ui->scale ) );
		ImGui::SetWindowSize( ImVec2( s_splashScreen->menu.width, s_splashScreen->menu.height ) );
		ImGui::PushStyleColor( ImGuiCol_Text, colorWhite );
		
		ImGui::SetWindowFontScale( ImGui::GetFont()->Scale * 1.75f );
		TextCenterAlign( "Powered by the SIR Engine" );
		TextCenterAlign( "The Nomad, SIR Engine, and related logs are copyright of" );
		TextCenterAlign( "GDR Games, all rights reserved. All other trademarks," );
		TextCenterAlign( "logos, and copyrights are property of their respective owners." );
		TextCenterAlign( "Made using FMOD Studio by Firelight Technologies Pty Ltd." );

		ImGui::PopStyleColor();
		ImGui::End();
		break; }
	case SPLASH_SCREEN_WARNING:
		ImGui::Begin( "##SplashScreen", NULL, MENU_DEFAULT_FLAGS );
		ImGui::SetWindowPos( ImVec2( 80 * ui->scale + ui->bias, 200 * ui->scale ) );
		ImGui::SetWindowSize( ImVec2( 860 * ui->scale + ui->bias, s_splashScreen->menu.height ) );
		ImGui::SetWindowFontScale( ImGui::GetFont()->Scale * 2.5f );

		ImGui::PushStyleColor( ImGuiCol_Text, colorRed );
		TextCenterAlign( EPILEPSY_WARNING_TITLE );
		ImGui::PopStyleColor();

		ImGui::SetWindowFontScale( ImGui::GetFont()->Scale * 1.75f );
		for ( const auto& it : epilespyWarning ) {
			TextCenterAlign( it );
		}
		ImGui::PopStyleColor();
		ImGui::End();
		break;
	default:
		break;
	};
}

static size_t CURL_WriteData( void *pData, size_t nSize, size_t nMemb, void *pStream )
{
	return FS_Write( pData, nSize * nMemb, *(fileHandle_t *)pStream );
}

static const char *CopyNewsString( const char *str )
{
	uint32_t len;
	char *out;

	len = strlen( str ) + 1;
	out = (char *)Z_Malloc( len, TAG_GAME );
	N_strncpyz( out, str, len );

	return out;
}

static void *LoadNewsThread( void *mutex )
{
	CURL *curl;
	CURLcode code;
	fileHandle_t fh;
		
	fh = FS_FOpenWrite( NEWS_FILE );
	if ( fh == FS_INVALID_HANDLE ) {
		Con_Printf( COLOR_RED "ERROR: failed to create " NEWS_FILE " in write-only mode!\n" );
		return NULL;
	}
		
	Con_Printf( "Getting latest newsfeed...\n" );
	
	code = CURLE_OK;
	
	curl = curl_easy_init();
	if ( curl ) {
		curl_easy_setopt( curl, CURLOPT_FOLLOWLOCATION, 1L );
		curl_easy_setopt( curl, CURLOPT_URL,
			"https://www.dropbox.com/scl/fi/r75o02wgoatmol1fjgb9h/newsfeed.dat?rlkey=783ntq7f138p3d9egkm0qz4g6&st=sfdf3fne&dl=1"
		);
		curl_easy_setopt( curl, CURLOPT_SSL_VERIFYPEER, false );
		curl_easy_setopt( curl, CURLOPT_WRITEFUNCTION, CURL_WriteData );
		curl_easy_setopt( curl, CURLOPT_WRITEDATA, &fh );

		code = curl_easy_perform( curl );

		curl_easy_cleanup( curl );
	} else {
		Con_Printf( COLOR_YELLOW "WARNING: curl_easy_init failed!\n" );
	}
	
	FS_FClose( fh );
	
	if ( code != CURLE_OK ) {
		Con_Printf( COLOR_RED "CURL failed to download newsfeed file: %s\n", curl_easy_strerror( code ) );
	} else {
		Con_Printf( "...fetched latest newsfeed\n" );
	}

	return NULL;
}

void MainMenu_LoadNews( void )
{
	int ret;

	if ( ( ret = pthread_create( &s_newsThread, NULL, LoadNewsThread, NULL ) ) != 0 ) {
		Con_Printf( COLOR_RED "ERROR: couldn't create posix thread for loading news data! %i\n", ret );
	} else {
		return;
	}

	// if we can't create a separate thread to load the newsfeed, then just block the main thread
	CURL *curl;
	CURLcode code;
	fileHandle_t fh;

	fh = FS_FOpenWrite( NEWS_FILE );
	if ( fh == FS_INVALID_HANDLE ) {
		Con_Printf( COLOR_RED "ERROR: failed to create " NEWS_FILE "in write-only mode!\n" );
		return;
	}
	
	Con_Printf( "Getting latest newsfeed...\n" );

	code = CURLE_OK;

	curl = curl_easy_init();
	if ( curl ) {
		curl_easy_setopt( curl, CURLOPT_FOLLOWLOCATION, 1L );
		curl_easy_setopt( curl, CURLOPT_URL,
			"https://www.dropbox.com/scl/fi/r75o02wgoatmol1fjgb9h/newsfeed.dat?rlkey=783ntq7f138p3d9egkm0qz4g6&st=sfdf3fne&dl=1"
		);
		curl_easy_setopt( curl, CURLOPT_SSL_VERIFYPEER, false );
		curl_easy_setopt( curl, CURLOPT_WRITEFUNCTION, CURL_WriteData );
		curl_easy_setopt( curl, CURLOPT_WRITEDATA, &fh );

		code = curl_easy_perform( curl );

		curl_easy_cleanup( curl );
	} else {
		Con_Printf( COLOR_YELLOW "WARNING: curl_easy_init failed!\n" );
	}

	FS_FClose( fh );

	if ( code != CURLE_OK ) {
		Con_Printf( COLOR_RED "CURL failed to download newsfeed file: %s\n", curl_easy_strerror( code ) );
	} else {
		Con_Printf( "...fetched latest newsfeed\n" );
	}
}

static void MainMenu_ParseNews( void )
{
	pthread_join( s_newsThread, (void **)NULL );

	newslabel_t tmp;
	uint64_t i;
	union {
		char *b;
		void *v;
	} f;
	const char *tok;
	const char *text_p, **text;
	uint64_t fileLength;
	uint64_t *uncompressedSize;
	char *buf;

	fileLength = FS_LoadFile( NEWS_FILE, &f.v );
	if ( !fileLength || !f.v ) {
		Con_Printf( COLOR_RED "Error opening " NEWS_FILE "\n" );
		return;
	}

	uncompressedSize = (uint64_t *)f.b;
	buf = Decompress( uncompressedSize + 1, fileLength, uncompressedSize, COMPRESS_ZLIB );
	FS_FreeFile( f.v );

	text_p = buf;
	text = (const char **)&text_p;

	s_newsLabelList.clear();
	s_nCurrentNewsLabel = 0;

	tok = COM_ParseComplex( text, qtrue );
	if ( tok[0] != '{' ) {
		COM_ParseError( "expected '{' at beginning of newsfeed file, instead got '%s'", tok );
		FS_FreeFile( f.v );
		return;
	}
	while ( 1 ) {
		tok = COM_ParseExt( text, qtrue );
		if ( !tok[0] ) {
			COM_ParseError( "unexpected end of newsfeed file" );
			FS_FreeFile( f.v );
			return;
		}
		// end-of-file
		if ( tok[0] == '}' ) {
			break;
		} else if ( tok[0] == '{' ) {
			memset( &tmp, 0, sizeof( tmp ) );
			while ( 1 ) {
				tok = COM_ParseExt( text, qtrue );
				if ( !tok[0] ) {
					COM_ParseError( "unexpected end of newsfeed definition" );
					FS_FreeFile( f.v );
					return;
				}
				if ( tok[0] == '}' ) {
					break;
				}
				if ( !N_stricmp( "Date", tok ) ) {
					tok = COM_ParseExt( text, qfalse );
					if ( !tok[0] ) {
						COM_ParseError( "missing parameter in newsfeed definition for 'Date'" );
						FS_FreeFile( f.v );
						return;
					}
					tmp.date = CopyNewsString( tok );
				} else if ( !N_stricmp( "Title", tok ) ) {
					tok = COM_ParseExt( text, qfalse );
					if ( !tok[0] ) {
						COM_ParseError( "missing parameter in newsfeed definition for 'Title'" );
						FS_FreeFile( f.v );
						return;
					}
					tmp.title = CopyNewsString( tok );
				} else if ( !N_stricmp( "Body", tok ) ) {
					tok = COM_ParseExt( text, qfalse );
					if ( !tok[0] ) {
						COM_ParseError( "missing parameter in newsfeed definition for 'Body'" );
						FS_FreeFile( f.v );
						return;
					}
					tmp.body = CopyNewsString( tok );
				} else if ( !N_stricmp( "Image", tok ) ) {
					tok = COM_ParseExt( text, qfalse );
					if ( !tok[0] ) {
						COM_ParseError( "missing parameter in newsfeed definition for 'Image'" );
						FS_FreeFile( f.v );
						return;
					}
					tmp.image = CopyNewsString( tok );
				} else if ( !N_stricmp( "URL", tok ) ) {
					tok = COM_ParseExt( text, qfalse );
					if ( !tok[0] ) {
						COM_ParseError( "missing parameter in newsfeed definition for 'URL'" );
						FS_FreeFile( f.v );
						return;
					}
					tmp.url = CopyNewsString( tok );
				} else {
					COM_ParseWarning( "unrecognized token in newsfeed file '%s'", tok );
				}
			}
			s_newsLabelList.emplace_back( tmp );
		}
	}

	Z_Free( buf );
}

void MainMenu_Cache( void )
{
	if ( !ui->uiAllocated ) {
		static mainmenu_t menu;
		static errorMessage_t error;
		static splashScreenMenu_t splash;

		s_main = &menu;
		s_errorMenu = &error;
		s_splashScreen = &splash;

		MainMenu_ParseNews();
	}
	memset( s_main, 0, sizeof( *s_main ) );
	memset( s_errorMenu, 0, sizeof( *s_errorMenu ) );
	memset( s_splashScreen, 0, sizeof( *s_splashScreen ) );

	// check for errors
	Cvar_VariableStringBuffer( "com_errorMessage", s_errorMenu->message, sizeof( s_errorMenu->message ) );
	if ( s_errorMenu->message[0] ) {
		Key_SetCatcher( KEYCATCH_UI );

		s_errorMenu->menu.draw = MainMenu_Draw;
		s_errorMenu->menu.fullscreen = qtrue;

		s_errorMenu->menu.x = 380 - strlen( s_errorMenu->message );
		s_errorMenu->menu.y = 268;

		UI_ForceMenuOff();
		UI_PushMenu( &s_errorMenu->menu );

		return;
	}

	s_splashScreen->timeStart = Sys_Milliseconds();
	s_splashScreen->lifeTime = 3500;
	s_splashScreen->splashPhase = SPLASH_SCREEN_LOGO;
	s_splashScreen->fmodLogoShader = re.RegisterShader( "menu/fmodLogo" );
	s_splashScreen->companyLogoShader = re.RegisterShader( "menu/companyLogo" );
	s_splashScreen->engineLogoShader = re.RegisterShader( "menu/engineLogo" );

	s_splashScreen->menu.draw = SplashScreen_Draw;
	s_splashScreen->menu.x = 0;
	s_splashScreen->menu.y = 0;
	s_splashScreen->menu.width = gi.gpuConfig.vidWidth;
	s_splashScreen->menu.height = gi.gpuConfig.vidHeight;

	s_main->font = FontCache()->AddFontToCache( "AlegreyaSC-Bold" );
	RobotoMono = FontCache()->AddFontToCache( "RobotoMono-Bold" );

	s_main->menu.titleFontScale = 3.5f;
	s_main->menu.textFontScale = 1.5f;
	s_main->menu.name = strManager->ValueForKey( "MENU_LOGO_STRING" )->value;
	s_main->menu.x = 0;
	s_main->menu.y = 0;
	s_main->menu.width = gi.gpuConfig.vidWidth;
	s_main->menu.height = gi.gpuConfig.vidHeight;
	s_main->menu.fullscreen = qtrue;
	s_main->menu.draw = MainMenu_Draw;
	s_main->menu.flags = MENU_DEFAULT_FLAGS;

	s_main->singleplayer.generic.type = MTYPE_TEXT;
	s_main->singleplayer.generic.id = ID_SINGEPLAYER;
	s_main->singleplayer.generic.flags = QMF_HIGHLIGHT_IF_FOCUS;
	s_main->singleplayer.generic.eventcallback = MainMenu_EventCallback;
	s_main->singleplayer.generic.font = AlegreyaSC;
	s_main->singleplayer.text = strManager->ValueForKey( "MENU_MAIN_SINGLEPLAYER" )->value;
	s_main->singleplayer.color = color_white;

	s_main->mods.generic.type = MTYPE_TEXT;
	s_main->mods.generic.id = ID_MODS;
	s_main->mods.generic.flags = QMF_HIGHLIGHT_IF_FOCUS;
	s_main->mods.generic.eventcallback = MainMenu_EventCallback;
	s_main->mods.generic.font = AlegreyaSC;
	s_main->mods.text = strManager->ValueForKey( "MENU_MAIN_MODS" )->value;
	s_main->mods.color = color_white;

	s_main->settings.generic.type = MTYPE_TEXT;
	s_main->settings.generic.id = ID_SETTINGS;
	s_main->settings.generic.flags = QMF_HIGHLIGHT_IF_FOCUS;
	s_main->settings.generic.eventcallback = MainMenu_EventCallback;
	s_main->settings.generic.font = AlegreyaSC;
	s_main->settings.text = strManager->ValueForKey( "MENU_MAIN_SETTINGS" )->value;
	s_main->settings.color = color_white;

	s_main->database.generic.type = MTYPE_TEXT;
	s_main->database.generic.id = ID_DATABASE;
	s_main->database.generic.flags = QMF_HIGHLIGHT_IF_FOCUS;
	s_main->database.generic.eventcallback = MainMenu_EventCallback;
	s_main->database.generic.font = AlegreyaSC;
	s_main->database.text = "Valden's Book";
	s_main->database.color = color_white;

	s_main->credits.generic.type = MTYPE_TEXT;
	s_main->credits.generic.id = ID_CREDITS;
	s_main->credits.generic.flags = QMF_HIGHLIGHT_IF_FOCUS;
	s_main->credits.generic.eventcallback = MainMenu_EventCallback;
	s_main->credits.generic.font = AlegreyaSC;
	s_main->credits.text = strManager->ValueForKey( "MENU_MAIN_CREDITS" )->value;
	s_main->credits.color = color_white;

	s_main->exitGame.generic.type = MTYPE_TEXT;
	s_main->exitGame.generic.id = ID_EXIT;
	s_main->exitGame.generic.flags = QMF_HIGHLIGHT_IF_FOCUS | QMF_SILENT;
	s_main->exitGame.generic.eventcallback = MainMenu_EventCallback;
	s_main->exitGame.generic.font = AlegreyaSC;
	s_main->exitGame.text = strManager->ValueForKey( "MENU_MAIN_EXIT" )->value;
	s_main->exitGame.color = color_white;

	s_main->noSaves = Cvar_VariableInteger( "sgame_NumSaves" ) == 0;
	s_main->menu.track = Snd_RegisterTrack( "event:/music/main_theme" );
	s_main->background = re.RegisterShader( "menu/mainbackground" );

	s_main->noMenu = qfalse;
	ui->menubackShader = s_main->background;

	Menu_AddItem( &s_main->menu, &s_main->singleplayer );
	Menu_AddItem( &s_main->menu, &s_main->settings );
	Menu_AddItem( &s_main->menu, &s_main->database );
	Menu_AddItem( &s_main->menu, &s_main->mods );
	Menu_AddItem( &s_main->menu, &s_main->exitGame );

	Key_SetCatcher( KEYCATCH_UI );
	ui->menusp = 0;
	if ( !playedSplashScreen ) {
		Cvar_Set( "r_clearColor", "0.0 0.0 0.0 1.0" );
		ui->menustate = UI_MENU_SPLASH;
		UI_PushMenu( &s_splashScreen->menu );
	} else {
		Cvar_Set( "r_clearColor", "0.1 0.1 0.1 1.0" );
		UI_PushMenu( &s_main->menu );
	}
}

void UI_MainMenu( void ) {
	Snd_ClearLoopingTracks();
	MainMenu_Cache();
}
