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

#define ID_SINGEPLAYER      1
#define ID_MODS             2
#define ID_SETTINGS         3
#define ID_DATABASE         4
#define ID_CREDITS          5
#define ID_EXIT             6
#define ID_TABLE            7

//#define UI_FAST_EDIT

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

typedef struct {
	menuframework_t menu;

//    menutable_t table;

	menutext_t singleplayer;
	menutext_t mods;
	menutext_t settings;
	menutext_t database;
	menutext_t credits;
	menutext_t exitGame;

	ImFont *font;

	nhandle_t background;
	sfxHandle_t ambience;

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
	nhandle_t warningShader;
	int splashPhase; // company logo -> engine logo -> epilepsy warning

	uint64_t timeStart;
	uint64_t lifeTime;
} splashScreenMenu_t;

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
	case ID_CREDITS:
		UI_CreditsMenu();
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

static void MainMenu_ToggleMenu( void ) {
	s_main->noMenu = !s_main->noMenu;
}

extern void Menu_DrawItemList( void **items, int numitems );

static void DrawMenu_Text( void )
{
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
	ImGui::SetCursorScreenPos( ImVec2( 8, 420 * ui->scale ) );

	Menu_DrawItemList( menu->items, menu->nitems );

	ImGui::End();

	//
	// draw the version
	//
	FontCache()->SetActiveFont( RobotoMono );

	ImGui::Begin( "MainMenuVersion", NULL, MENU_DEFAULT_FLAGS | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_AlwaysAutoResize );
	ImGui::SetWindowFontScale( ImGui::GetFont()->Scale * 1.5f );
	ImGui::SetWindowPos( ImVec2( 500 * ui->scale + ui->bias, 700 * ui->scale ) );
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
		ImGui::Begin( "Game Error", NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize
			| ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoMove );
		ImGui::SetWindowPos( ImVec2( s_errorMenu->menu.x * ui->scale, s_errorMenu->menu.y * ui->scale ) );
		FontCache()->SetActiveFont( RobotoMono );
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

static void TextCenterAlign( const char *text )
{
	float fontSize = ImGui::GetFontSize() * strlen( text ) / 2;
	ImGui::SameLine( ImGui::GetWindowSize().x / 2 - fontSize + ( fontSize / 2 ) );
	ImGui::TextUnformatted( text );
	ImGui::NewLine();
}

static void SplashScreen_Draw( void )
{
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

	ImGui::Begin( "##SplashScreen", NULL, MENU_DEFAULT_FLAGS );
	ImGui::SetWindowPos( ImVec2( s_splashScreen->menu.x, s_splashScreen->menu.y ) );
	ImGui::SetWindowSize( ImVec2( s_splashScreen->menu.width, s_splashScreen->menu.height ) );

	ImGui::PushStyleColor( ImGuiCol_Text, colorWhite );

	switch ( s_splashScreen->splashPhase ) {
	case SPLASH_SCREEN_LOGO: { // get attribution to 3rd party stuff done with
		int cursorX = 100;

		ImGui::SetCursorScreenPos( ImVec2( cursorX * ui->scale + ui->bias, 100 * ui->scale ) );
		ImGui::Image( (ImTextureID)(uintptr_t)s_splashScreen->companyLogoShader, ImVec2( 330 * ui->scale + ui->bias, 180 * ui->scale ) );
		cursorX += 300 * ui->scale + ui->bias;

		ImGui::SetCursorScreenPos( ImVec2( cursorX * ui->scale + ui->bias, 100 * ui->scale ) );
		ImGui::Image( (ImTextureID)(uintptr_t)s_splashScreen->engineLogoShader, ImVec2( 340 * ui->scale + ui->bias, 220 * ui->scale ) );
		cursorX = 200 * ui->scale + ui->bias;

		ImGui::SetCursorScreenPos( ImVec2( cursorX * ui->scale + ui->bias, 300 * ui->scale ) );
		ImGui::Image( (ImTextureID)(uintptr_t)s_splashScreen->fmodLogoShader, ImVec2( 356 * ui->scale + ui->bias, 126 * ui->scale ) );

		ImGui::NewLine();
		ImGui::NewLine();
		ImGui::NewLine();
		ImGui::NewLine();
		
		ImGui::SetWindowFontScale( ImGui::GetFont()->Scale * 1.75f );
		TextCenterAlign( "Powered by the SIR Engine" );
		TextCenterAlign( "The Nomad, SIR Engine, and related logs are copyright of" );
		TextCenterAlign( "GDR Games, all rights reserved. All other trademarks," );
		TextCenterAlign( "logos, and copyrights are property of their respective owners." );
		TextCenterAlign( "Made using FMOD Studio by Firelight Technologies Pty Ltd." );
		break; }
	case SPLASH_SCREEN_WARNING:
		ImGui::SetCursorScreenPos( ImVec2( 156 * ui->scale + ui->bias, 200 * ui->scale ) );
		ImGui::SetWindowFontScale( ImGui::GetFont()->Scale * 2.5f );

		ImGui::Image( (ImTextureID)(uintptr_t)s_splashScreen->warningShader, ImVec2( 64 * ui->scale + ui->bias, 64 * ui->scale ) );
		ImGui::PushStyleColor( ImGuiCol_Text, colorRed );
		TextCenterAlign( EPILEPSY_WARNING_TITLE );
		ImGui::PopStyleColor();


		ImGui::NewLine();
		ImGui::NewLine();
		ImGui::SetWindowFontScale( ImGui::GetFont()->Scale * 1.75f );
		for ( const auto& it : epilespyWarning ) {
			TextCenterAlign( it );
		}
		break;
	default:
		break;
	};

	ImGui::PopStyleColor();

	ImGui::End();
}

void MainMenu_Cache( void )
{
	if ( !ui->uiAllocated ) {
		s_main = (mainmenu_t *)Hunk_Alloc( sizeof( *s_main ), h_high );
		s_errorMenu = (errorMessage_t *)Hunk_Alloc( sizeof( *s_errorMenu ), h_high );
		s_splashScreen = (splashScreenMenu_t *)Hunk_Alloc( sizeof( *s_splashScreen ), h_high );
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

		s_errorMenu->menu.x = 528 - strlen( s_errorMenu->message );
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
	s_splashScreen->warningShader = re.RegisterShader( "menusplashwarn" );

	s_splashScreen->menu.draw = SplashScreen_Draw;
	s_splashScreen->menu.x = 0;
	s_splashScreen->menu.y = 0;
	s_splashScreen->menu.width = ui->gpuConfig.vidWidth;
	s_splashScreen->menu.height = ui->gpuConfig.vidHeight;

	s_main->font = FontCache()->AddFontToCache( "AlegreyaSC-Bold" );
	RobotoMono = FontCache()->AddFontToCache( "RobotoMono-Bold" );

	s_main->menu.titleFontScale = 3.5f;
	s_main->menu.textFontScale = 1.5f;
	s_main->menu.name = strManager->ValueForKey( "MENU_LOGO_STRING" )->value;
	s_main->menu.x = 0;
	s_main->menu.y = 0;
	s_main->menu.width = ui->gpuConfig.vidWidth;
	s_main->menu.height = ui->gpuConfig.vidHeight;
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
	Menu_AddItem( &s_main->menu, &s_main->credits );
	Menu_AddItem( &s_main->menu, &s_main->mods );
	Menu_AddItem( &s_main->menu, &s_main->exitGame );

	// add in background ambience
//    Snd_AddLoopingTrack( Snd_RegisterTrack( "music/world/icy_gusts.wav" ) );

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
