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

#include "../game/g_game.h"
#include "ui_public.hpp"
#include "ui_menu.h"
#include "ui_lib.h"
#include "ui_window.h"
#include "ui_string_manager.h"
#include "../rendercommon/imgui_impl_opengl3.h"
#include "../rendercommon/imgui_internal.h"
#include "../game/imgui_memory_editor.h"
#include "RobotoMono-Bold.h"
#define FPS_FRAMES 60

uiGlobals_t *ui;
CUIFontCache *g_pFontCache;

ImFont *AlegreyaSC;
ImFont *PressStart2P;
ImFont *RobotoMono;

cvar_t *ui_language;
cvar_t *ui_cpuString;
cvar_t *ui_printStrings;
cvar_t *ui_active;
cvar_t *ui_diagnostics;
cvar_t *r_gpuDiagnostics;
cvar_t *ui_debugOverlay;
cvar_t *ui_maxLangStrings;
cvar_t *ui_menuStyle;
dif_t difficultyTable[NUMDIFS];

static cvar_t *com_drawFPS;

/*
=================
UI_Cache
=================
*/
static void UI_Cache_f( void ) {
	Con_Printf( "Caching ui resources...\n" );

	MainMenu_Cache();
	ModsMenu_Cache();
	DemoMenu_Cache();
	PlayMenu_Cache();
	PauseMenu_Cache();
	CreditsMenu_Cache();
	ConfirmMenu_Cache();
	DataBaseMenu_Cache();
	SettingsMenu_Cache();
}

CUIFontCache::CUIFontCache( void ) {
	union {
		void *v;
		char *b;
	} f;
	uint64_t nLength;
	const char **text;
	const char *tok, *text_p;
	float scale;
	char name[MAX_NPATH];

	Con_Printf( "Initializing font cache...\n" );

	memset( m_FontList, 0, sizeof( m_FontList ) );
	m_pCurrentFont = NULL;

	nLength = FS_LoadFile( "fonts/font_config.txt", &f.v );
	if ( !nLength || !f.v ) {
		N_Error( ERR_FATAL, "CUIFontCache::Init: failed to load fonts/font_config.txt" );
	}

	text_p = f.b;
	text = (const char **)&text_p;

	COM_BeginParseSession( "fonts/font_config.txt" );

	tok = COM_ParseExt( text, qtrue );
	if ( tok[0] != '{' ) {
		COM_ParseError( "expected '{' at beginning of file" );
		FS_FreeFile( f.v );
		return;
	}
	while ( 1 ) {
		tok = COM_ParseExt( text, qtrue );
		if ( !tok[0] ) {
			COM_ParseError( "unexpected end of file" );
			break;
		}
		if ( tok[0] == '}' ) {
			break;
		}

		scale = 1.0f;

		if ( tok[0] == '{' ) {
			while ( 1 ) {
				tok = COM_ParseExt( text, qtrue );
				if ( !tok[0] ) {
					COM_ParseError( "unexpected end of font defintion" );
					FS_FreeFile( f.v );
					return;
				}
				if ( tok[0] == '}' ) {
					break;
				}
				if ( !N_stricmp( tok, "name" ) ) {
					tok = COM_ParseExt( text, qfalse );
					if ( !tok[0] ) {
						COM_ParseError( "missing parameter for 'name'" );
						FS_FreeFile( f.v );
						return;
					}
					N_strncpyz( name, tok, sizeof( name ) - 1 );
				}
				else if ( !N_stricmp( tok, "scale" ) ) {
					tok = COM_ParseExt( text, qfalse );
					if ( !tok[0] ) {
						COM_ParseError( "missing parameter for 'scale'" );
						FS_FreeFile( f.v );
						return;
					}
					scale = atof( tok );
				}
				else {
					COM_ParseWarning( "unrecognized token '%s'", tok );
				}
			}
			if ( !AddFontToCache( name, "", scale ) ) {
				N_Error( ERR_FATAL, "CUIFontCache::Init: failed to load font data for 'fonts/%s/%s.ttf'", name, name );
			}
		}
	}

	m_pCurrentFont = NULL;

	FS_FreeFile( f.v );
}

void CUIFontCache::SetActiveFont( ImFont *font )
{
	if ( !ImGui::GetIO().Fonts->IsBuilt() ) {
		Finalize();
	}

	if ( !ImGui::GetFont() || !ImGui::GetFont()->ContainerAtlas ) {
		return;
	}
	if ( m_pCurrentFont ) {
		ImGui::PopFont();
	}
	m_pCurrentFont = font;
	ImGui::PushFont( font );
}

void CUIFontCache::SetActiveFont( nhandle_t hFont )
{
	if ( !ImGui::GetFont()->ContainerAtlas ) {
		return;
	}
	if ( !ImGui::GetIO().Fonts->IsBuilt() ) {
		Finalize();
	}

	if ( hFont == FS_INVALID_HANDLE || !m_FontList[ hFont ] ) {
		return;
	}

	if ( m_pCurrentFont ) {
		ImGui::PopFont();
	}

	m_pCurrentFont = m_FontList[ hFont ]->m_pFont;

	ImGui::PushFont( m_pCurrentFont );
}

uiFont_t *CUIFontCache::GetFont( const char *fileName ) {
	return m_FontList[ Com_GenerateHashValue( fileName, MAX_UI_FONTS ) ];
}

void CUIFontCache::ClearCache( void ) {
	if ( ImGui::GetCurrentContext() && ImGui::GetIO().Fonts ) {
		ImGui::GetIO().Fonts->Clear();
	}
	memset( m_FontList, 0, sizeof( m_FontList ) );
	m_pCurrentFont = NULL;

	RobotoMono = NULL;
	PressStart2P = NULL;
	AlegreyaSC = NULL;
}

void CUIFontCache::Finalize( void ) {
	ImGui::GetIO().Fonts->Build();
	ImGui_ImplOpenGL3_CreateFontsTexture();

	FontCache()->SetActiveFont( FontCache()->AddFontToCache( "RobotoMono-Bold" ) );
}

nhandle_t CUIFontCache::RegisterFont( const char *filename, const char *variant, float scale ) {
	uint64_t hash;
	char rpath[MAX_NPATH];
	char hashpath[MAX_NPATH];

	COM_StripExtension( filename, rpath, sizeof( rpath ) );
	if ( rpath[ strlen( rpath ) - 1 ] == '.' ) {
		rpath[ strlen( rpath) - 1 ] = 0;
	}
	Com_snprintf( hashpath, sizeof( hashpath ) - 1, "%s", rpath );

	hash = Com_GenerateHashValue( hashpath, MAX_UI_FONTS );
	AddFontToCache( filename, variant, scale );

	return hash;
}

ImFont *CUIFontCache::AddFontToCache( const char *filename, const char *variant, float scale )
{
	uiFont_t *font;
	uint64_t size;
	uint64_t hash;
	ImFontConfig config;
	const char *path;
	union {
		void *v;
		char *b;
	} f;
	char rpath[MAX_NPATH];
	char hashpath[MAX_NPATH];

	COM_StripExtension( filename, rpath, sizeof( rpath ) );
	if ( rpath[ strlen( rpath ) - 1 ] == '.' ) {
		rpath[ strlen( rpath) - 1 ] = 0;
	}

	Com_snprintf( hashpath, sizeof( hashpath ) - 1, "%s", rpath );

	path = va( "fonts/%s.ttf", hashpath );
	hash = Com_GenerateHashValue( hashpath, MAX_UI_FONTS );

	//
	// see if we already have the font in the cache
	//
	for ( font = m_FontList[hash]; font; font = font->m_pNext ) {
		if ( !N_stricmp( font->m_szName, hashpath ) ) {
			return font->m_pFont; // its already been loaded
		}
	}

	Con_Printf( "CUIFontCache: loading font '%s'...\n", path );

	if ( strlen( hashpath ) >= MAX_NPATH ) {
		N_Error( ERR_DROP, "CUIFontCache::AddFontToCache: name '%s' is too long", hashpath );
	}

	size = FS_LoadFile( path, &f.v );
	if ( !size || !f.v ) {
		N_Error( ERR_DROP, "CUIFontCache::AddFontToCache: failed to load font file '%s'", path );
	}

	font = (uiFont_t *)Hunk_Alloc( sizeof( *font ), h_low );

	font->m_pNext = m_FontList[hash];
	m_FontList[hash] = font;

	config.FontDataOwnedByAtlas = false;
	config.GlyphExtraSpacing.x = 0.0f;

	N_strncpyz( font->m_szName, hashpath, sizeof( font->m_szName ) );
	font->m_nFileSize = size;
	font->m_pFont = ImGui::GetIO().Fonts->AddFontFromMemoryTTF( f.v, size, 16.0f * scale, &config );

	FS_FreeFile( f.v );

	return font->m_pFont;
}

void CUIFontCache::ListFonts_f( void ) {
	uint64_t memSize, i;
	uint64_t numFonts;
	const uiFont_t *font;

	Con_Printf( "---------- Font Cache Info ----------\n" );

	numFonts = 0;
	memSize = 0;
	for ( i = 0; i < MAX_UI_FONTS; i++ ) {
		font = g_pFontCache->m_FontList[i];

		if ( !font ) {
			continue;
		}

		Con_Printf( "[%s]\n", font->m_szName );
		Con_Printf( "File Size: %lu\n", font->m_nFileSize );

		memSize += font->m_nFileSize;
		numFonts++;
	}

	Con_Printf( "\n" );
	Con_Printf( "%-8lu total bytes in font cache\n", memSize );
	Con_Printf( "%-8lu total fonts in cache\n", numFonts );
}

const char *UI_LangToString( int32_t lang )
{
	switch ((language_t)lang) {
	case LANGUAGE_ENGLISH:
		return "english";
	default:
		break;
	};
	return "Invalid";
}

static void UI_RegisterCvars( void )
{
	ui_language = Cvar_Get( "ui_language", "english", CVAR_LATCH | CVAR_SAVE );
	Cvar_SetDescription( ui_language,
							"Sets the game's language: american_english, british_english, spanish, german\n"
							"Currently only english is supported, but I'm looking for some translators :)"
				   		);

	ui_cpuString = Cvar_Get( "sys_cpuString", "detect", CVAR_PROTECTED | CVAR_ROM | CVAR_NORESTART );

	ui_printStrings = Cvar_Get( "ui_printStrings", "1", CVAR_LATCH | CVAR_SAVE | CVAR_PRIVATE );
	Cvar_CheckRange( ui_printStrings, "0", "1", CVT_INT );
	Cvar_SetDescription( ui_printStrings, "Print value strings set by the language ui file" );

#ifdef _NOMAD_DEBUG
	ui_debugOverlay = Cvar_Get( "ui_debugOverlay", "1", CVAR_SAVE );
#else
	ui_debugOverlay = Cvar_Get( "ui_debugOverlay", "0", CVAR_SAVE );
#endif
	Cvar_SetDescription( ui_debugOverlay, "Draws an overlay of various debugging statistics." );

	ui_active = Cvar_Get( "g_paused", "1", CVAR_TEMP );

#ifdef _NOMAD_DEBUG
	r_gpuDiagnostics = Cvar_Get( "r_gpuDiagnostics", "1", CVAR_LATCH | CVAR_SAVE );
#else
	r_gpuDiagnostics = Cvar_Get( "r_gpuDiagnostics", "0", CVAR_LATCH | CVAR_SAVE );
#endif

	com_drawFPS = Cvar_Get( "com_drawFPS", "0", CVAR_SAVE );
	Cvar_SetDescription( com_drawFPS, "Toggles displaying the average amount of frames drawn per second." );

#ifdef _NOMAD_DEBUG
	ui_diagnostics = Cvar_Get( "ui_diagnostics", "3", CVAR_PROTECTED | CVAR_SAVE );
#else
	ui_diagnostics = Cvar_Get( "ui_diagnostics", "0", CVAR_PROTECTED | CVAR_SAVE );
#endif
	Cvar_SetDescription( ui_diagnostics, "Displays various engine performance diagnostics:\n"
											" 0 - disabled\n"
											" 1 - display gpu memory usage\n"
											" 2 - display cpu memory usage\n"
											" 3 - SHOW ME EVERYTHING!!!!" );
	
	ui_maxLangStrings = Cvar_Get( "ui_maxLangStrings", "528", CVAR_TEMP | CVAR_LATCH );
	Cvar_CheckRange( ui_maxLangStrings, "528", "8192", CVT_INT );

	ui_menuStyle = Cvar_Get( "ui_menuStyle", "0", CVAR_SAVE );
	Cvar_CheckRange( ui_menuStyle, "0", "5", CVT_INT );
	Cvar_SetDescription( ui_menuStyle, "Sets the ui's generate layout." );
}

extern "C" void UI_Shutdown( void )
{
	if ( ui ) {
		ui->activemenu = NULL;
		memset( ui->stack, 0, sizeof( ui->stack ) );
		ui->menusp = 0;
		ui->uiAllocated = qfalse;
	}

	if ( strManager ) {
		strManager->Shutdown();
		strManager = NULL;
	}

	if ( FontCache() ) {
		FontCache()->ClearCache();
	}

	Cmd_RemoveCommand( "ui.cache" );
	Cmd_RemoveCommand( "ui.fontinfo" );
	Cmd_RemoveCommand( "togglepausemenu" );
	Cmd_RemoveCommand( "ui.reload_savefiles" );
	Cmd_RemoveCommand( "reportbug" );

	if ( gi.soundStarted ) {
		Snd_StopAll();
	}
}

// FIXME: call UI_Shutdown instead
void G_ShutdownUI( void ) {
	UI_Shutdown();
}

/*
* UI_GetHashString: an sgame interface for the string manager
*/
extern "C" void UI_GetHashString( const char *name, char *value ) {
	const stringHash_t *hash;

	hash = strManager->ValueForKey( name );

	N_strncpyz( value, hash->value, MAX_STRING_CHARS );
}

static void UI_PauseMenu_f( void ) {
	if ( gi.state != GS_LEVEL || !gi.mapLoaded ) {
		return;
	}
	UI_SetActiveMenu( UI_MENU_PAUSE );
}

static int32_t previousTimes[FPS_FRAMES];

extern "C" void UI_DrawFPS( void )
{
	if ( !com_drawFPS->i ) {
		return;
	}

	static int32_t index;
	static int32_t previous;
	int32_t t, frameTime;
	int32_t total, i;
	int32_t fps;
	extern ImFont *RobotoMono;

	if ( RobotoMono ) {
		FontCache()->SetActiveFont( RobotoMono );
	}

	fps = 0;

	t = Sys_Milliseconds();
	frameTime = t - previous;
	previous = t;

	previousTimes[index % FPS_FRAMES] = frameTime;
	index++;
	if ( index > FPS_FRAMES ) {
		// average multiple frames together to smooth changes out a bit
		total = 0;
		for ( i = 0; i < FPS_FRAMES; i++ ) {
			total += previousTimes[i];
		}
		if ( total == 0 ) {
			total = 1;
		}
		fps = 1000 * FPS_FRAMES / total;
	} else {
		fps = previous;
	}

	ImGui::Begin( "DrawFPS##UI", NULL, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar
										| ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMouseInputs
										| ImGuiWindowFlags_NoBackground );
	ImGui::SetWindowPos( ImVec2( 900 * ui->scale + ui->bias, 8 * ui->scale ) );
	ImGui::SetWindowFontScale( 1.5f * ui->scale );
	ImGui::Text( "%i", fps );
	ImGui::End();
}

void UI_EscapeMenuToggle( void )
{
	if ( ( Key_IsDown( KEY_ESCAPE ) || ( Key_IsDown( KEY_PAD0_B ) && !ImGui::IsAnyItemActive() ) ) && ui->menusp > 1 ) {
		if ( !ui->escapeToggle ) {
			ui->escapeToggle = qtrue;
			if ( !gi.mapLoaded ) {
				Snd_PlaySfx( ui->sfx_select );
				UI_PopMenu();
			} else {
				UI_SetActiveMenu( UI_MENU_NONE );
			}
		}
	} else {
		ui->escapeToggle = qfalse;
	}
}

static void UI_BugReport_f( void ) {
	if ( !SDL_OpenURL( "https://forms.gle/wecmc52ZL7Vq9XVe8" ) ) {
		Con_Printf( COLOR_RED "WARNING: SDL_OpenURL() failed!\n" );
	}
	Con_Printf( "Opening bug reporting form...\n" );
}

extern "C" void UI_Init( void )
{
	Con_Printf( "UI_Init: initializing UI...\n" );

	MainMenu_LoadNews();

	// register cvars
	UI_RegisterCvars();

	// init the library
	static uiGlobals_t globals;
	ui = &globals;

	// init the string manager
	static CUIStringManager string_database;
	strManager = &string_database;
	strManager->Init();
	// load the language string file
	strManager->LoadLanguage( ui_language->s );
	if ( !strManager->NumLangsLoaded() ) {
		N_Error( ERR_DROP, "UI_Init: no language loaded" );
	}

	//
	// init strings
	//
	difficultyTable[ DIF_EASY ].name = strManager->ValueForKey( "SP_DIFF_EASY" )->value;
	difficultyTable[ DIF_EASY ].tooltip = strManager->ValueForKey( "SP_DIFF_0_DESC" )->value;

	difficultyTable[ DIF_NORMAL ].name = strManager->ValueForKey( "SP_DIFF_NORMAL" )->value;
	difficultyTable[ DIF_NORMAL ].tooltip = strManager->ValueForKey( "SP_DIFF_1_DESC" )->value;

	difficultyTable[ DIF_HARD ].name = strManager->ValueForKey( "SP_DIFF_HARD" )->value;
	difficultyTable[ DIF_HARD ].tooltip = strManager->ValueForKey( "SP_DIFF_2_DESC" )->value;

	difficultyTable[ DIF_VERY_HARD ].name = strManager->ValueForKey( "SP_DIFF_VERY_HARD" )->value;
	difficultyTable[ DIF_VERY_HARD ].tooltip = strManager->ValueForKey( "SP_DIFF_3_DESC" )->value;

	difficultyTable[ DIF_INSANE ].name = strManager->ValueForKey( "SP_DIFF_INSANE" )->value;
	difficultyTable[ DIF_INSANE ].tooltip = strManager->ValueForKey( "SP_DIFF_4_DESC" )->value;

	difficultyTable[ DIF_MEME ].tooltip = "PAIN."; // no changing this one, because that's the most accurate description

	// for 1024x768 virtualized screen
	ui->scale = gi.gpuConfig.vidHeight * ( 1.0f / 768.0f );
	if ( gi.gpuConfig.vidWidth * 1024.0f > gi.gpuConfig.vidHeight * 768.0f ) {
		// wide screen
		ui->bias = 0.5f * ( gi.gpuConfig.vidWidth - ( gi.gpuConfig.vidHeight * ( 1024.0f / 768.0f ) ) );
	}
	else {
		// no wide screen
		ui->bias = 0.0f;
	}

	// initialize the menu system
	Menu_Cache();

	ui->activemenu = NULL;
	ui->menusp     = 0;

	ui->uiAllocated = qfalse;

	UI_Cache_f();

	ui->uiAllocated = qtrue;
	
	UI_SetActiveMenu( UI_MENU_MAIN );

	// are we running a demo?
	if ( FS_FOpenFileRead( "demokey.txt", NULL ) > 0 ) {
		ui->demoVersion = qtrue;
	} else {
		ui->demoVersion = qfalse;
	}

	memset( previousTimes, 0, sizeof( previousTimes ) );

	// add commands
	Cmd_AddCommand( "ui.cache", UI_Cache_f );
	Cmd_AddCommand( "ui.fontinfo", CUIFontCache::ListFonts_f );
	Cmd_AddCommand( "togglepausemenu", UI_PauseMenu_f );
	Cmd_AddCommand( "ui.reload_savefiles", UI_ReloadSaveFiles_f );
	Cmd_AddCommand( "reportbug", UI_BugReport_f );
}

void Menu_Cache( void )
{
	ui->whiteShader = re.RegisterShader( "white" );
	ui->back_0 = re.RegisterShader( "menu/backbutton0" );
	ui->back_1 = re.RegisterShader( "menu/backbutton1" );

	ui->sfx_select = Snd_RegisterSfx( "event:/sfx/menu/select_item" );
	ui->sfx_back = Snd_RegisterSfx( "event:/sfx/menu/back" );
	ui->sfx_move = Snd_RegisterSfx( "event:/sfx/menu/move" );

	ui->controller_start = re.RegisterShader( "menu/xbox_start" );
	ui->controller_back = re.RegisterShader( "menu/xbox_back" );
	ui->controller_a = re.RegisterShader( "menu/xbox_button_a" );
	ui->controller_b = re.RegisterShader( "menu/xbox_button_b" );
	ui->controller_x = re.RegisterShader( "menu/xbox_button_x" );
	ui->controller_y = re.RegisterShader( "menu/xbox_button_y" );
	ui->controller_dpad_down = re.RegisterShader( "menu/dpad_down" );
	ui->controller_dpad_up = re.RegisterShader( "menu/dpad_up" );
	ui->controller_dpad_left = re.RegisterShader( "menu/dpad_left" );
	ui->controller_dpad_right = re.RegisterShader( "menu/dpad_right" );
	ui->controller_left_button = re.RegisterShader( "menu/left_button" );
	ui->controller_right_button = re.RegisterShader( "menu/right_button" );
	ui->controller_left_trigger = re.RegisterShader( "menu/left_trigger" );
	ui->controller_right_trigger = re.RegisterShader( "menu/right_trigger" );

	// cache the textures
	// for some reason, we need to load these before the backdrop
	// if we want the lower resolution textures to load properly
	re.RegisterShader( "menu/save_0" );
	re.RegisterShader( "menu/save_1" );
	re.RegisterShader( "menu/load_0" );
	re.RegisterShader( "menu/load_1" );
	re.RegisterShader( "menu/reset_0" );
	re.RegisterShader( "menu/reset_1" );
	re.RegisterShader( "menu/accept_0" );
	re.RegisterShader( "menu/accept_1" );
	re.RegisterShader( "menu/play_0" );
	re.RegisterShader( "menu/play_1" );
	re.RegisterShader( "menu/tales_around_the_campfire" );

	ui->backdrop = re.RegisterShader( "menu/mainbackdrop" );

	// IT MUST BE THERE!
	if ( !FS_LoadFile( "textures/coconut.jpg", NULL ) || ui->backdrop == FS_INVALID_HANDLE ) {
		N_Error( ERR_FATAL, "YOU DARE DEFY THE WILL OF THE GODS!?!?!?!?!?" );
	}
}

/*
=================
UI_Refresh
=================
*/

extern "C" void UI_ShowDemoMenu( void )
{
	UI_SetActiveMenu( UI_MENU_DEMO );
}

extern "C" void UI_DrawMenuBackground( void )
{
	refdef_t refdef;

	memset( &refdef, 0, sizeof( refdef ) );
	refdef.x = 0;
	refdef.y = 0;
	refdef.width = gi.gpuConfig.vidWidth;
	refdef.height = gi.gpuConfig.vidHeight;
	refdef.time = ui->realtime;
	refdef.flags = RSF_NOWORLDMODEL | RSF_ORTHO_TYPE_SCREENSPACE;

	//
	// draw the background
	//
	re.SetColor( NULL );
	re.DrawImage( 0, 0, gi.gpuConfig.vidWidth, gi.gpuConfig.vidHeight, 0, 0, 1, 1, ui->menubackShader );
}

extern "C" void UI_AddJoystickKeyEvents( void )
{
	ImGuiIO& io = ImGui::GetIO();

	io.AddKeyEvent( ImGuiKey_Escape, Key_IsDown( KEY_PAD0_BACK ) );
	io.AddKeyEvent( ImGuiKey_Enter, Key_IsDown( KEY_PAD0_START ) );

	io.AddKeyEvent( ImGuiKey_DownArrow, Key_IsDown( KEY_PAD0_LEFTSTICK_DOWN ) );
	io.AddKeyEvent( ImGuiKey_UpArrow, Key_IsDown( KEY_PAD0_LEFTSTICK_UP ) );
	io.AddKeyEvent( ImGuiKey_LeftArrow, Key_IsDown( KEY_PAD0_LEFTSTICK_LEFT ) );
	io.AddKeyEvent( ImGuiKey_RightArrow, Key_IsDown( KEY_PAD0_LEFTSTICK_RIGHT ) );
}

extern "C" void UI_Refresh( int32_t realtime )
{
	static qboolean windowFocus = qfalse;

	ui->realtime = realtime;
	ui->frametime = ui->frametime - realtime;

	UI_DrawFPS();

	{
		re.SetColor( colorWhite );
		re.DrawImage( 0, 0, gi.gpuConfig.vidWidth, gi.gpuConfig.vidHeight, 0, 0, 1, 1, ui->backdrop );
	}

	if ( !( Key_GetCatcher() & KEYCATCH_UI ) ) {
		return;
	}

	if ( ui->activemenu && ui->activemenu->fullscreen ) {
		UI_DrawMenuBackground();
	}

	if ( Cvar_VariableInteger( "in_joystick" ) ) {
		UI_AddJoystickKeyEvents();
	}

	if ( ui->activemenu ) {
		if ( ui->activemenu->track != FS_INVALID_HANDLE && !ui->setMusic ) {
			ui->setMusic = qtrue;
			Snd_ClearLoopingTracks();
			Snd_AddLoopingTrack( ui->activemenu->track );
		}

		if ( ui->activemenu->draw ) {
			ui->activemenu->draw();
		} else {
			Menu_Draw( ui->activemenu );
		}
	}

	refdef_t refdef;

	memset( &refdef, 0, sizeof( refdef ) );
	refdef.x = 0;
	refdef.y = 0;
	refdef.width = gi.gpuConfig.vidWidth;
	refdef.height = gi.gpuConfig.vidHeight;
	refdef.time = 0;
	refdef.flags = RSF_ORTHO_TYPE_SCREENSPACE | RSF_NOWORLDMODEL;

	re.RenderScene( &refdef );
}