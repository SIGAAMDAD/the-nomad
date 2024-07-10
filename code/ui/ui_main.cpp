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
//#include "RobotoMono-Bold.h"
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
    SettingsMenu_Cache();
	ModsMenu_Cache();
	SinglePlayerMenu_Cache();
	DemoMenu_Cache();
	LoadGameMenu_Cache();
	NewGameMenu_Cache();
	PauseMenu_Cache();
	CreditsMenu_Cache();
	ConfirmMenu_Cache();
	DataBaseMenu_Cache();
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

	FS_FreeFile( f.v );
}

void CUIFontCache::SetActiveFont( ImFont *font )
{
	if ( !ImGui::GetIO().Fonts->IsBuilt() ) {
		Finalize();
	}

	if ( !ImGui::GetFont()->ContainerAtlas ) {
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
    ImGui::SetWindowPos( ImVec2( 1010 * ui->scale + ui->bias, 8 * ui->scale ) );
    ImGui::SetWindowFontScale( 1.5f * ui->scale );
    ImGui::Text( "%i", fps );
    ImGui::End();
}

static uint32_t gpu_BackEndPreviousTimes[FPS_FRAMES];
static int32_t gpu_BackEndFrameTimeIndex;
static uint32_t gpu_BackEndFrameTimePrevious;

static uint32_t gpu_FrontEndPreviousTimes[FPS_FRAMES];
static int32_t gpu_FrontEndFrameTimeIndex;
static uint32_t gpu_FrontEndFrameTimePrevious;

extern "C" uint32_t GPU_CalcFrameTime( uint64_t gpuTime, uint32_t *times, uint32_t *previous, int32_t *index )
{
	uint32_t frameTime;
	uint32_t total, i;
	uint32_t realTime;

	frameTime = gpuTime - *previous;
	*previous = gpuTime;

	times[ *index % FPS_FRAMES ] = frameTime;
	(*index)++;
	if ( *index > FPS_FRAMES ) {
        // average multiple frames together to smooth changes out a bit
		total = 0;
		for ( i = 0; i < FPS_FRAMES; i++ ) {
			total += times[i];
		}
		if ( total == 0 ) {
			total = 1;
		}
		realTime = 1000.0f * FPS_FRAMES / total;
    } else {
		realTime = *previous;
	}

	return realTime;
}

void ImGui_ShowAboutWindow( void ) {
	ImGui::Text("Dear ImGui %s", ImGui::GetVersion());
    ImGui::Separator();
    ImGui::Text("By Omar Cornut and all Dear ImGui contributors.");
    ImGui::Text("Dear ImGui is licensed under the MIT License, see LICENSE for more information.");

    static bool show_config_info = false;
    ImGui::Checkbox("Config/Build Information", &show_config_info);
    if (show_config_info)
    {
        ImGuiIO& io = ImGui::GetIO();
        ImGuiStyle& style = ImGui::GetStyle();

        bool copy_to_clipboard = ImGui::Button("Copy to clipboard");
        ImVec2 child_size = ImVec2(0, ImGui::GetTextLineHeightWithSpacing() * 18);
        ImGui::BeginChildFrame(ImGui::GetID("cfg_infos"), child_size, ImGuiWindowFlags_NoMove);
        if (copy_to_clipboard)
        {
            ImGui::LogToClipboard();
            ImGui::LogText("```\n"); // Back quotes will make text appears without formatting when pasting on GitHub
        }

        ImGui::Text("Dear ImGui %s (%d)", IMGUI_VERSION, IMGUI_VERSION_NUM);
        ImGui::Separator();
        ImGui::Text("sizeof(size_t): %d, sizeof(ImDrawIdx): %d, sizeof(ImDrawVert): %d", (int)sizeof(size_t), (int)sizeof(ImDrawIdx), (int)sizeof(ImDrawVert));
        ImGui::Text("define: __cplusplus=%d", (int)__cplusplus);
#ifdef IMGUI_DISABLE_OBSOLETE_FUNCTIONS
        ImGui::Text("define: IMGUI_DISABLE_OBSOLETE_FUNCTIONS");
#endif
#ifdef IMGUI_DISABLE_OBSOLETE_KEYIO
        ImGui::Text("define: IMGUI_DISABLE_OBSOLETE_KEYIO");
#endif
#ifdef IMGUI_DISABLE_WIN32_DEFAULT_CLIPBOARD_FUNCTIONS
        ImGui::Text("define: IMGUI_DISABLE_WIN32_DEFAULT_CLIPBOARD_FUNCTIONS");
#endif
#ifdef IMGUI_DISABLE_WIN32_DEFAULT_IME_FUNCTIONS
        ImGui::Text("define: IMGUI_DISABLE_WIN32_DEFAULT_IME_FUNCTIONS");
#endif
#ifdef IMGUI_DISABLE_WIN32_FUNCTIONS
        ImGui::Text("define: IMGUI_DISABLE_WIN32_FUNCTIONS");
#endif
#ifdef IMGUI_DISABLE_DEFAULT_FORMAT_FUNCTIONS
        ImGui::Text("define: IMGUI_DISABLE_DEFAULT_FORMAT_FUNCTIONS");
#endif
#ifdef IMGUI_DISABLE_DEFAULT_MATH_FUNCTIONS
        ImGui::Text("define: IMGUI_DISABLE_DEFAULT_MATH_FUNCTIONS");
#endif
#ifdef IMGUI_DISABLE_DEFAULT_FILE_FUNCTIONS
        ImGui::Text("define: IMGUI_DISABLE_DEFAULT_FILE_FUNCTIONS");
#endif
#ifdef IMGUI_DISABLE_FILE_FUNCTIONS
        ImGui::Text("define: IMGUI_DISABLE_FILE_FUNCTIONS");
#endif
#ifdef IMGUI_DISABLE_DEFAULT_ALLOCATORS
        ImGui::Text("define: IMGUI_DISABLE_DEFAULT_ALLOCATORS");
#endif
#ifdef IMGUI_USE_BGRA_PACKED_COLOR
        ImGui::Text("define: IMGUI_USE_BGRA_PACKED_COLOR");
#endif
#ifdef _WIN32
        ImGui::Text("define: _WIN32");
#endif
#ifdef _WIN64
        ImGui::Text("define: _WIN64");
#endif
#ifdef __linux__
        ImGui::Text("define: __linux__");
#endif
#ifdef __APPLE__
        ImGui::Text("define: __APPLE__");
#endif
#ifdef _MSC_VER
        ImGui::Text("define: _MSC_VER=%d", _MSC_VER);
#endif
#ifdef _MSVC_LANG
        ImGui::Text("define: _MSVC_LANG=%d", (int)_MSVC_LANG);
#endif
#ifdef __MINGW32__
        ImGui::Text("define: __MINGW32__");
#endif
#ifdef __MINGW64__
        ImGui::Text("define: __MINGW64__");
#endif
#ifdef __GNUC__
        ImGui::Text("define: __GNUC__=%d", (int)__GNUC__);
#endif
#ifdef __clang_version__
        ImGui::Text("define: __clang_version__=%s", __clang_version__);
#endif
#ifdef __EMSCRIPTEN__
        ImGui::Text("define: __EMSCRIPTEN__");
#endif
        ImGui::Separator();
        ImGui::Text("io.BackendPlatformName: %s", io.BackendPlatformName ? io.BackendPlatformName : "NULL");
        ImGui::Text("io.BackendRendererName: %s", io.BackendRendererName ? io.BackendRendererName : "NULL");
        ImGui::Text("io.ConfigFlags: 0x%08X", io.ConfigFlags);
        if (io.ConfigFlags & ImGuiConfigFlags_NavEnableKeyboard)        ImGui::Text(" NavEnableKeyboard");
        if (io.ConfigFlags & ImGuiConfigFlags_NavEnableGamepad)         ImGui::Text(" NavEnableGamepad");
        if (io.ConfigFlags & ImGuiConfigFlags_NavEnableSetMousePos)     ImGui::Text(" NavEnableSetMousePos");
        if (io.ConfigFlags & ImGuiConfigFlags_NavNoCaptureKeyboard)     ImGui::Text(" NavNoCaptureKeyboard");
        if (io.ConfigFlags & ImGuiConfigFlags_NoMouse)                  ImGui::Text(" NoMouse");
        if (io.ConfigFlags & ImGuiConfigFlags_NoMouseCursorChange)      ImGui::Text(" NoMouseCursorChange");
        if (io.MouseDrawCursor)                                         ImGui::Text("io.MouseDrawCursor");
        if (io.ConfigMacOSXBehaviors)                                   ImGui::Text("io.ConfigMacOSXBehaviors");
        if (io.ConfigInputTextCursorBlink)                              ImGui::Text("io.ConfigInputTextCursorBlink");
        if (io.ConfigWindowsResizeFromEdges)                            ImGui::Text("io.ConfigWindowsResizeFromEdges");
        if (io.ConfigWindowsMoveFromTitleBarOnly)                       ImGui::Text("io.ConfigWindowsMoveFromTitleBarOnly");
        if (io.ConfigMemoryCompactTimer >= 0.0f)                        ImGui::Text("io.ConfigMemoryCompactTimer = %.1f", io.ConfigMemoryCompactTimer);
        ImGui::Text("io.BackendFlags: 0x%08X", io.BackendFlags);
        if (io.BackendFlags & ImGuiBackendFlags_HasGamepad)             ImGui::Text(" HasGamepad");
        if (io.BackendFlags & ImGuiBackendFlags_HasMouseCursors)        ImGui::Text(" HasMouseCursors");
        if (io.BackendFlags & ImGuiBackendFlags_HasSetMousePos)         ImGui::Text(" HasSetMousePos");
        if (io.BackendFlags & ImGuiBackendFlags_RendererHasVtxOffset)   ImGui::Text(" RendererHasVtxOffset");
        ImGui::Separator();
        ImGui::Text("io.Fonts: %d fonts, Flags: 0x%08X, TexSize: %d,%d", io.Fonts->Fonts.Size, io.Fonts->Flags, io.Fonts->TexWidth, io.Fonts->TexHeight);
        ImGui::Text("io.DisplaySize: %.2f,%.2f", io.DisplaySize.x, io.DisplaySize.y);
        ImGui::Text("io.DisplayFramebufferScale: %.2f,%.2f", io.DisplayFramebufferScale.x, io.DisplayFramebufferScale.y);
        ImGui::Separator();
        ImGui::Text("style.WindowPadding: %.2f,%.2f", style.WindowPadding.x, style.WindowPadding.y);
        ImGui::Text("style.WindowBorderSize: %.2f", style.WindowBorderSize);
        ImGui::Text("style.FramePadding: %.2f,%.2f", style.FramePadding.x, style.FramePadding.y);
        ImGui::Text("style.FrameRounding: %.2f", style.FrameRounding);
        ImGui::Text("style.FrameBorderSize: %.2f", style.FrameBorderSize);
        ImGui::Text("style.ItemSpacing: %.2f,%.2f", style.ItemSpacing.x, style.ItemSpacing.y);
        ImGui::Text("style.ItemInnerSpacing: %.2f,%.2f", style.ItemInnerSpacing.x, style.ItemInnerSpacing.y);

        if (copy_to_clipboard)
        {
            ImGui::LogText("\n```\n");
            ImGui::LogFinish();
        }
        ImGui::EndChildFrame();
    }
}

static void UI_DrawModuleProperty( asIScriptObject *pObject, asUINT index )
{
	int typeId;
	const char *name;
	asUINT n;

	name = pObject->GetPropertyName( index );
	typeId = pObject->GetPropertyTypeId( index );

	ImGui::Text( "%s - (typeId) %i", name, typeId );
	ImGui::Indent();
	switch ( typeId ) {
	case asTYPEID_BOOL:
		ImGui::Text( "%s", *(bool *)pObject->GetAddressOfProperty( index ) ? "true" : "false" );
		break;
	case asTYPEID_DOUBLE:
		ImGui::Text( "%lf", *(double *)pObject->GetAddressOfProperty( index ) );
		break;
	case asTYPEID_FLOAT:
		ImGui::Text( "%f", *(float *)pObject->GetAddressOfProperty( index ) );
		break;
	case asTYPEID_INT8:
		ImGui::Text( "%hi", *(int8_t *)pObject->GetAddressOfProperty( index ) );
		break;
	case asTYPEID_INT16:
		ImGui::Text( "%hi", *(int16_t *)pObject->GetAddressOfProperty( index ) );
		break;
	case asTYPEID_INT32:
		ImGui::Text( "%i", *(int32_t *)pObject->GetAddressOfProperty( index ) );
		break;
	case asTYPEID_INT64:
		ImGui::Text( "%li", *(int64_t *)pObject->GetAddressOfProperty( index ) );
		break;
	case asTYPEID_UINT8:
		ImGui::Text( "%hu", *(uint8_t *)pObject->GetAddressOfProperty( index ) );
		break;
	case asTYPEID_UINT16:
		ImGui::Text( "%hu", *(uint16_t *)pObject->GetAddressOfProperty( index ) );
		break;
	case asTYPEID_UINT32:
		ImGui::Text( "%u", *(uint32_t *)pObject->GetAddressOfProperty( index ) );
		break;
	case asTYPEID_UINT64:
		ImGui::Text( "%lu", *(uint64_t *)pObject->GetAddressOfProperty( index ) );
		break;
	case asTYPEID_OBJHANDLE:
		ImGui::Text( "0x%08lx", (uintptr_t)pObject->GetAddressOfProperty( index ) );
		break;
	case asTYPEID_APPOBJECT: {
		asIScriptObject *pSubObject = (asIScriptObject *)pObject->GetAddressOfProperty( index );
		for ( n = 0; n < pSubObject->GetPropertyCount(); n++ ) {
			UI_DrawModuleProperty( pSubObject, n );
		}
		break; }
	default:
		ImGui::TextUnformatted( "Cannot fetch value" );
		break;
	};
	ImGui::Unindent();
}

static void UI_DrawModuleGlobalVar( const char *name, const char *nameSpace, int typeId, bool isConst, asUINT index, asIScriptModule *pModule )
{
	asUINT n;

	ImGui::Text( "%s::%s - (typeId) %i, (isConst) %s", nameSpace, name, typeId, isConst ? "true" : "false" );
	ImGui::Indent();
	switch ( typeId ) {
	case asTYPEID_BOOL:
		ImGui::Text( "%s", *(bool *)pModule->GetAddressOfGlobalVar( index ) ? "true" : "false" );
		break;
	case asTYPEID_DOUBLE:
		ImGui::Text( "%lf", *(double *)pModule->GetAddressOfGlobalVar( index ) );
		break;
	case asTYPEID_FLOAT:
		ImGui::Text( "%f", *(float *)pModule->GetAddressOfGlobalVar( index ) );
		break;
	case asTYPEID_INT8:
		ImGui::Text( "%hi", *(int8_t *)pModule->GetAddressOfGlobalVar( index ) );
		break;
	case asTYPEID_INT16:
		ImGui::Text( "%hi", *(int16_t *)pModule->GetAddressOfGlobalVar( index ) );
		break;
	case asTYPEID_INT32:
		ImGui::Text( "%i", *(int32_t *)pModule->GetAddressOfGlobalVar( index ) );
		break;
	case asTYPEID_INT64:
		ImGui::Text( "%li", *(int64_t *)pModule->GetAddressOfGlobalVar( index ) );
		break;
	case asTYPEID_UINT8:
		ImGui::Text( "%hu", *(uint8_t *)pModule->GetAddressOfGlobalVar( index ) );
		break;
	case asTYPEID_UINT16:
		ImGui::Text( "%hu", *(uint16_t *)pModule->GetAddressOfGlobalVar( index ) );
		break;
	case asTYPEID_UINT32:
		ImGui::Text( "%u", *(uint32_t *)pModule->GetAddressOfGlobalVar( index ) );
		break;
	case asTYPEID_UINT64:
		ImGui::Text( "%lu", *(uint64_t *)pModule->GetAddressOfGlobalVar( index ) );
		break;
	case asTYPEID_OBJHANDLE:
		ImGui::Text( "0x%08lx", (uintptr_t)pModule->GetAddressOfGlobalVar( index ) );
		break;
	case asTYPEID_MASK_OBJECT: {
		ImGui::Text( "%u", *(asUINT *)pModule->GetAddressOfGlobalVar( index ) );
	
		asITypeInfo *type = g_pModuleLib->GetScriptEngine()->GetTypeInfoById( typeId );
		for ( n = type->GetEnumValueCount(); n-- > 0; ) {
			int32_t enumValue;
			const char *enumName;
				
			enumName = type->GetEnumValueByIndex( n, &enumValue );
			if ( enumValue == *(int32_t *)pModule->GetAddressOfGlobalVar( index ) ) {
				ImGui::SameLine();
				ImGui::Text( ", %s", enumName );
				break;
			}
		}
		break; }
	case asTYPEID_SCRIPTOBJECT: {
		asIScriptObject *pObject = (asIScriptObject *)pModule->GetAddressOfGlobalVar( index );
		for ( n = 0; n < pObject->GetPropertyCount(); n++ ) {
			UI_DrawModuleProperty( pObject, n );
		}
		break; }
	default:
		if ( typeId & asTYPEID_APPOBJECT ) {
			asIScriptObject *pObject = (asIScriptObject *)pModule->GetAddressOfGlobalVar( index );
			for ( n = 0; n < pObject->GetPropertyCount(); n++ ) {
				UI_DrawModuleProperty( pObject, n );
			}
		}
		if ( typeId & asTYPEID_SCRIPTOBJECT ||
			g_pModuleLib->GetScriptEngine()->GetTypeInfoById( typeId )->GetFlags() & asOBJ_REF )
		{
			ImGui::Text( "0x%08lx", (uintptr_t)pModule->GetAddressOfGlobalVar( index ) );
		} else {
			ImGui::TextUnformatted( "Cannot fetch value" );
		}
		break;
	};
	ImGui::NewLine();
	if ( typeId & asTYPEID_SCRIPTOBJECT ) {
		ImGui::TextUnformatted( " ScriptObject" );
	}
	if ( typeId & asTYPEID_APPOBJECT ) {
		ImGui::SameLine();
		ImGui::TextUnformatted( " AppObject" );
	}
	if ( typeId & asTYPEID_HANDLETOCONST ) {
		ImGui::SameLine();
		ImGui::TextUnformatted( " HandleToConst" );
	}
	if ( typeId & asTYPEID_MASK_OBJECT ) {
		ImGui::SameLine();
		ImGui::TextUnformatted( " MaskObject" );
	}
	ImGui::Unindent();
}

static void UI_DrawModuleDebugOverlay( void )
{
	asIScriptModule *pModule;
	asIScriptFunction *pFunction;
	asUINT i, j, n;
	uint32_t nModules;

	ImGui::Begin( "Module Debug", NULL, ImGuiWindowFlags_AlwaysAutoResize );

	nModules = g_pModuleLib->GetModCount();
	pModule = g_pModuleLib->GetScriptModule();
	
	{
		ImGui::SeparatorText( "Functions" );
		for ( j = 0; j < pModule->GetFunctionCount(); j++ ) {
			pFunction = pModule->GetFunctionByIndex( j );
			ImGui::TextUnformatted( pFunction->GetDeclaration() );
		}

		ImGui::SeparatorText( "Imported Functions" );
		for ( j = 0; j < pModule->GetImportedFunctionCount(); j++ ) {
			ImGui::Text( "%s -> %s", pModule->GetImportedFunctionDeclaration( j ), pModule->GetImportedFunctionSourceModule( j ) );
		}

		ImGui::SeparatorText( "Global Variables" );
		for ( j = 0; j < pModule->GetGlobalVarCount(); j++ ) {
			const char *name, *nameSpace;
			int typeId;
			bool isConst;

			pModule->GetGlobalVar( j, &name, &nameSpace, &typeId, &isConst );
				
			UI_DrawModuleGlobalVar( name, nameSpace, typeId, isConst, j, pModule );
		}

		ImGui::SeparatorText( "Object Types" );
		for ( j = 0; j < pModule->GetObjectTypeCount(); j++ ) {
			asITypeInfo *pTypeInfo = pModule->GetObjectTypeByIndex( j );

			ImGui::SeparatorText( pTypeInfo->GetName() );

			ImGui::Indent();
			ImGui::SeparatorText( "Behaviours" );
			ImGui::Indent();
			for ( n = 0; n < pTypeInfo->GetBehaviourCount(); n++ ) {
				asEBehaviours behaviour;
				ImGui::TextUnformatted( pTypeInfo->GetBehaviourByIndex( n, &behaviour )->GetDeclaration() );
			}
			ImGui::Unindent();
			ImGui::Unindent();

			ImGui::Indent();
			ImGui::SeparatorText( "Methods" );
			ImGui::Indent();
			for ( n = 0; n < pTypeInfo->GetMethodCount(); n++ ) {
				ImGui::TextUnformatted( pTypeInfo->GetMethodByIndex( n )->GetDeclaration() );
			}
			ImGui::Unindent();
			ImGui::Unindent();
			
			ImGui::Indent();
			ImGui::SeparatorText( "Properties" );
			ImGui::Indent();
			for ( n = 0; n < pTypeInfo->GetPropertyCount(); n++ ) {
				const char *name;
				bool isPrivate, isProtected, isRef;
				int typeId, offset;
				pTypeInfo->GetProperty( n, &name, &typeId, &isPrivate, &isProtected, &offset, &isRef );
				ImGui::Text( "%s - (typeId) %i, (offset) %i, (isPrivate) %s, (isProtected) %s, (isReference) %s",
					pTypeInfo->GetPropertyDeclaration( n ), typeId, offset, isPrivate ? "true" : "false",
					isProtected ? "true" : "false", isRef ? "true" : "false" );
			}
			ImGui::Unindent();
			ImGui::Unindent();
		}
	}

	ImGui::End();
}

static void UI_DrawDebugOverlay( void )
{
	uint64_t i;

	UI_DrawModuleDebugOverlay();

	if ( ImGui::Begin( "Input Debug##DebugOverlayInputSystem" ) ) {
		extern cvar_t *in_joystick;

		if ( in_joystick->i ) {
			ImGui::TextUnformatted( "Joystick: Enabled" );
		} else {
			ImGui::TextUnformatted( "Joystick: Disabled" );
		}

		ImGui::Text( "Input Mode: %s", in_mode->i == 0 ? "Keyboard & Mouse" : "GamePad/Controller" );

		ImGui::End();
	}

	if ( ImGui::Begin( "ImGui Debug##DebugOverlayImGuiDebug" ) ) {
		ImGui_ShowAboutWindow();
		ImGui::End();
	}

	ImGui::ShowIDStackToolWindow();
	ImGui::ShowMetricsWindow();
	ImGui::ShowDebugLogWindow();

	if ( ImGui::Begin( "GPU Debug##DebugOverlayGPUDriver" ) ) {
		char *p;
		char str[256];
		uint32_t gpu_FrameTime, gpu_FrameSamples, gpu_FramePrimitives;
		uint64_t frameTime;
		static uint32_t frameTimes[60];
		for ( i = 0; i < arraylen( frameTimes ); i++ ) {
			frameTimes[i] = i;
		}

		ImGui::Text( "Vendor: %s", ui->gpuConfig.vendor_string );
		ImGui::Text( "Version: %s", ui->gpuConfig.version_string );
		ImGui::Text( "GPU Driver: %s", ui->gpuConfig.renderer_string );
		ImGui::Text( "GPU Renderer: %s", g_renderer->s );
		if ( !N_stricmp( g_renderer->s, "opengl" ) || !N_stricmp( g_renderer->s, "vulkan" ) ) {
			ImGui::Text( "GLSL Version: %s", ui->gpuConfig.shader_version_str );
		} else if ( !N_stricmp( g_renderer->s, "d3d11" ) ) {
			ImGui::Text( "HLSL Version: %s", ui->gpuConfig.shader_version_str );
		}

		re.GetGPUFrameStats( &gpu_FrameTime, &gpu_FrameSamples, &gpu_FramePrimitives );
		frameTime = GPU_CalcFrameTime( time_backend, gpu_BackEndPreviousTimes, &gpu_BackEndFrameTimePrevious, &gpu_BackEndFrameTimeIndex );
		frameTime = GPU_CalcFrameTime( time_frontend, gpu_FrontEndPreviousTimes, &gpu_FrontEndFrameTimePrevious,
			&gpu_FrontEndFrameTimeIndex );

		ImGui::Text( "Frame Primitives: %u", gpu_FramePrimitives );
		ImGui::Text( "Frame Samples: %u", gpu_FrameSamples );
		ImGui::Text( "Frame Time: %u", gpu_FrameTime );
		ImGui::Text( "BackEnd Frame Time: %lu", time_backend );
		ImGui::Text( "FrontEnd Frame Time: %lu", time_frontend );

		ImGui::TextUnformatted( "Extensions Used:" );
		if ( Cvar_VariableInteger( "r_arb_shader_storage_buffer_object" ) ) {
			ImGui::PushStyleColor( ImGuiCol_Text, colorGreen );
			ImGui::TextUnformatted( "GL_ARB_shader_storage_buffer_object" );
			ImGui::PopStyleColor();
		} else {
			ImGui::PushStyleColor( ImGuiCol_Text, colorRed );
			ImGui::TextUnformatted( "GL_ARB_shader_storage_buffer_object" );
			ImGui::PopStyleColor();
		}
		if ( Cvar_VariableInteger( "r_arb_map_buffer_range" ) ) {
			ImGui::PushStyleColor( ImGuiCol_Text, colorGreen );
			ImGui::TextUnformatted( "GL_ARB_map_buffer_range" );
			ImGui::PopStyleColor();
		} else {
			ImGui::PushStyleColor( ImGuiCol_Text, colorGreen );
			ImGui::TextUnformatted( "GL_ARB_map_buffer_range" );
			ImGui::PopStyleColor();
		}
		if ( Cvar_VariableInteger( "r_arb_framebuffer_object" ) ) {
			ImGui::PushStyleColor( ImGuiCol_Text, colorGreen );
			ImGui::TextUnformatted( "GL_ARB_framebuffer_object" );
			ImGui::PopStyleColor();
		} else {
			ImGui::PushStyleColor( ImGuiCol_Text, colorRed );
			ImGui::TextUnformatted( "GL_ARB_framebuffer_object" );
			ImGui::PopStyleColor();
		}
		if ( Cvar_VariableInteger( "r_arb_vertex_buffer_object" ) ) {
			ImGui::PushStyleColor( ImGuiCol_Text, colorGreen );
			ImGui::TextUnformatted( "GL_ARB_vertex_buffer_object" );
			ImGui::PopStyleColor();
		} else {
			ImGui::PushStyleColor( ImGuiCol_Text, colorRed );
			ImGui::TextUnformatted( "GL_ARB_vertex_buffer_object" );
			ImGui::PopStyleColor();
		}
		if ( Cvar_VariableInteger( "r_arb_vertex_array_object" ) ) {
			ImGui::PushStyleColor( ImGuiCol_Text, colorGreen );
			ImGui::TextUnformatted( "GL_ARB_vertex_array_object" );
			ImGui::PopStyleColor();
		} else {
			ImGui::PushStyleColor( ImGuiCol_Text, colorRed );
			ImGui::TextUnformatted( "GL_ARB_vertex_array_object" );
			ImGui::PopStyleColor();
		}
		if ( Cvar_VariableInteger( "r_arb_sync" ) ) {
			ImGui::PushStyleColor( ImGuiCol_Text, colorGreen );
			ImGui::TextUnformatted( "GL_ARB_sync" );
			ImGui::PopStyleColor();
		} else {
			ImGui::PushStyleColor( ImGuiCol_Text, colorRed );
			ImGui::TextUnformatted( "GL_ARB_sync" );
			ImGui::PopStyleColor();
		}
		if ( Cvar_VariableInteger( "r_arb_texture_float" ) ) {
			ImGui::PushStyleColor( ImGuiCol_Text, colorGreen );
			ImGui::TextUnformatted( "GL_ARB_texture_float" );
			ImGui::PopStyleColor();
		} else {
			ImGui::PushStyleColor( ImGuiCol_Text, colorRed );
			ImGui::TextUnformatted( "GL_ARB_texture_float" );
			ImGui::PopStyleColor();
		}
		if ( Cvar_VariableInteger( "r_arb_texture_filter_anisotropic" ) ) {
			ImGui::PushStyleColor( ImGuiCol_Text, colorGreen );
			ImGui::TextUnformatted( "GL_ARB_texture_filter_anisotropic" );
			ImGui::PopStyleColor();
		} else {
			ImGui::PushStyleColor( ImGuiCol_Text, colorRed );
			ImGui::TextUnformatted( "GL_ARB_texture_filter_anisotropic" );
			ImGui::PopStyleColor();
		}

		ImGui::TextUnformatted( "Extensions Found:" );
		p = str;
		for ( i = 0; i < sizeof( ui->gpuConfig.extensions_string ); i++ ) {
			if ( !ui->gpuConfig.extensions_string[i] ) {
				break;
			} else if ( ui->gpuConfig.extensions_string[i] == ' ' ) {
				*p = '\0';
				ImGui::Text( "  %s", str );
				p = str;
			} else {
				*p++ = ui->gpuConfig.extensions_string[i];
			}
		}
		ImGui::End();
	}

	if ( ImGui::Begin( "System Debug##DebugOverlaySystemData" ) ) {
		uint64_t virtMem, physMem, peakVirt, peakPhys;
		double cpuFreq;
		const char *cpuFreqSuffix;
		static int32_t frameTimes[60];
		for ( i = 0; i < arraylen( frameTimes ); i++ ) {
			frameTimes[i] = i;
		}

		Sys_GetRAMUsage( &virtMem, &physMem, &peakVirt, &peakPhys );
		cpuFreq = Sys_CalculateCPUFreq();

		ImGui::Text( "CPU Processor: %s", ui_cpuString->s );
		ImGui::Text( "CPU Cores: %i", SDL_GetCPUCount() );
		ImGui::Text( "CPU CacheLine: %lu", com_cacheLine );

		cpuFreqSuffix = "Hz";
		if ( cpuFreq > 1000.0f ) {
			cpuFreq /= 1000;
			cpuFreqSuffix = "KHz";
		}
		if ( cpuFreq > 1000.0f ) {
			cpuFreq /= 1000.0f;
			cpuFreqSuffix = "MHz";
		}
		if ( cpuFreq > 1000.0f ) {
			cpuFreq /= 1000.0f;
			cpuFreqSuffix = "GHz";
		}
		ImGui::Text( "CPU: %lf %s", cpuFreq, cpuFreqSuffix );

		ImGui::Text( "Stack Memory: %lu", Sys_StackMemoryRemaining() );
		ImGui::Text( "Current Virtual Memory: %lu", virtMem );
		ImGui::Text( "Current Physical Memory: %lu", physMem );
		ImGui::Text( "Peak Virtual Memory: %lu", peakVirt );
		ImGui::Text( "Peak Physical Memory: %lu", peakPhys );
		
		ImGui::End();
	}

	if ( ImGui::Begin( "ModuleLib Debug##DebugOverlayModuleLib" ) ) {
		const CModuleInfo *loadList = g_pModuleLib->GetLoadList();

		ImGui::Text( "ModuleCount: %lu", g_pModuleLib->GetModCount() );
		for ( i = 0; i < g_pModuleLib->GetModCount(); i++ ) {
			ImGui::Text( "  [%lu]: %s v%i.%i.%i", i, loadList[i].m_szName, loadList[i].m_nModVersionMajor,
				loadList[i].m_nModVersionUpdate, loadList[i].m_nModVersionPatch );
		}
		ImGui::End();
	}
}

void UI_EscapeMenuToggle( void )
{
    if ( ( Key_IsDown( KEY_ESCAPE ) || Key_IsDown( KEY_PAD0_B ) && !ImGui::IsAnyItemActive() ) && ui->menusp > 1 ) {
		if ( !ui->escapeToggle ) {
			ui->escapeToggle = qtrue;
			UI_PopMenu();
			Snd_PlaySfx( ui->sfx_back );
		}
    } else {
		ui->escapeToggle = qfalse;
	}
}

extern "C" void UI_Init( void )
{
    Con_Printf( "UI_Init: initializing UI...\n" );

    // register cvars
    UI_RegisterCvars();

    // init the library
	ui = (uiGlobals_t *)Hunk_Alloc( sizeof( *ui ), h_high );

    // init the string manager
    strManager = (CUIStringManager *)Hunk_Alloc( sizeof( *strManager ), h_high );
    strManager->Init();
    // load the language string file
    strManager->LoadLanguage( ui_language->s );
    if ( !strManager->NumLangsLoaded() ) {
        N_Error( ERR_DROP, "UI_Init: no language loaded" );
    }

	//
    // init strings
    //
    difficultyTable[DIF_NOOB].name = strManager->ValueForKey( "SP_DIFF_VERY_EASY" )->value;
    difficultyTable[DIF_NOOB].tooltip = strManager->ValueForKey( "SP_DIFF_0_DESC" )->value;

    difficultyTable[DIF_RECRUIT].name = strManager->ValueForKey( "SP_DIFF_EASY" )->value;
    difficultyTable[DIF_RECRUIT].tooltip = strManager->ValueForKey( "SP_DIFF_1_DESC" )->value;

    difficultyTable[DIF_MERC].name = strManager->ValueForKey( "SP_DIFF_MEDIUM" )->value;
    difficultyTable[DIF_MERC].tooltip = strManager->ValueForKey( "SP_DIFF_2_DESC" )->value;

    difficultyTable[DIF_NOMAD].name = strManager->ValueForKey( "SP_DIFF_HARD" )->value;
    difficultyTable[DIF_NOMAD].tooltip = strManager->ValueForKey( "SP_DIFF_3_DESC" )->value;

    difficultyTable[DIF_BLACKDEATH].name = strManager->ValueForKey( "SP_DIFF_VERY_HARD" )->value;
    difficultyTable[DIF_BLACKDEATH].tooltip = strManager->ValueForKey( "SP_DIFF_4_DESC" )->value;

    difficultyTable[DIF_MINORINCONVENIECE].tooltip = "PAIN."; // no changing this one, because that's the most accurate description

	// cache redundant calulations
	re.GetConfig( &ui->gpuConfig );

	// for 640x480 virtualized screen
	ui->scale = ui->gpuConfig.vidHeight * ( 1.0f / 768.0f );
	if ( ui->gpuConfig.vidWidth * 768.0f > ui->gpuConfig.vidHeight * 1024.0f ) {
		// wide screen
		ui->bias = 0.5f * ( ui->gpuConfig.vidWidth - ( ui->gpuConfig.vidHeight * ( 1024.0f / 768.0f ) ) );
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
    UI_SetActiveMenu( UI_MENU_MAIN );

	ui->uiAllocated = qtrue;

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

//	ImGuiIO& io = ImGui::GetIO();
//	ImFontConfig config;
//
//	memset( &config, 0, sizeof( config ) );
//	config.FontDataOwnedByAtlas = false;
//
//	ImFont *font = io.Fonts->AddFontFromMemoryTTF( (void *)g_RobotoMono_Bold, sizeof( g_RobotoMono_Bold ), 16.0f, &config );
//	io.FontDefault = font;
}

void Menu_Cache( void )
{
    ui->whiteShader = re.RegisterShader( "white" );
	ui->back_0 = re.RegisterShader( "menu/backbutton0" );
	ui->back_1 = re.RegisterShader( "menu/backbutton1" );
	ui->sfx_select = Snd_RegisterSfx( "sfx/menu/menu1.ogg" );
//	ui->sfx_move = Snd_RegisterSfx( "sfx/menu/menu2.ogg" );
	ui->sfx_back = Snd_RegisterSfx( "sfx/menu/menu3.ogg" );
	ui->sfx_null = Snd_RegisterSfx( "sfx/menu/menu4.ogg" );

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
	refdef.width = ui->gpuConfig.vidWidth;
	refdef.height = ui->gpuConfig.vidHeight;
	refdef.time = Sys_Milliseconds();
	refdef.flags = RSF_NOWORLDMODEL | RSF_ORTHO_TYPE_SCREENSPACE;

	//
	// draw the background
	//
	re.ClearScene();
	re.SetColor( colorWhite );
    re.DrawImage( 0, 0, refdef.width, refdef.height, 0, 0, 1, 1, ui->menubackShader );
    re.RenderScene( &refdef );
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

static void UI_DrawGPUStats( void )
{
	char str[MAXPRINTMSG];
	char *p;
	int i;
	gpuMemory_t memstats;

	ImGui::Begin( "##RendererInformation", NULL, ImGuiWindowFlags_NoCollapse );

	if ( ImGui::TreeNodeEx( (void *)(uintptr_t)"##GPUExtensions", ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_DefaultOpen,
		"GPU Extensions" ) )
	{
		ImGui::TextUnformatted( "Extensions Used:" );

		auto extensionPrint = [&]( const char *extensionString, const char *GLstring ) -> void {
			if ( Cvar_VariableInteger( extensionString ) ) {
				ImGui::PushStyleColor( ImGuiCol_Text, colorGreen );
			} else {
				ImGui::PushStyleColor( ImGuiCol_Text, colorRed );
			}
			ImGui::TextUnformatted( GLstring );
			ImGui::PopStyleColor();
		};

		ImGui::Indent();
		extensionPrint( "r_arb_shader_storage_buffer_object", "GL_ARB_shader_storage_buffer_object" );
		extensionPrint( "r_arb_map_buffer_range", "GL_ARB_map_buffer_range" );
		extensionPrint( "r_arb_framebuffer_object", "GL_ARB_framebuffer_object" );
		extensionPrint( "r_arb_framebuffer_blit", "GL_ARB_framebuffer_blit" );
		extensionPrint( "r_arb_framebuffer_multisample", "GL_ARB_framebuffer_multisample" );
		extensionPrint( "r_arb_framebuffer_srgb", "GL_ARB_framebuffer_sRGB" );
		extensionPrint( "r_arb_texture_compression", "GL_ARB_texture_compression" );
		extensionPrint( "r_arb_vertex_buffer_object", "GL_ARB_vertex_buffer_object" );
		extensionPrint( "r_arb_vertex_array_object", "GL_ARB_vertex_array_object" );
		extensionPrint( "r_arb_sync", "GL_ARB_sync" );
		extensionPrint( "r_arb_texture_float", "GL_ARB_texture_float" );
		extensionPrint( "r_arb_texture_filter_anisotropic", "GL_ARB_texture_filter_anisotropic" );
		extensionPrint( "r_arb_color_buffer_float", "GL_ARB_color_buffer_float" );
		extensionPrint( "r_arb_multisample", "GL_ARB_multisample" );
		extensionPrint( "r_arb_multitexture", "GL_ARB_multitexture" );
		ImGui::Unindent();

		ImGui::TreePop();
	}
	if ( ImGui::TreeNodeEx( (void *)(uintptr_t)"##GPUStatistics", ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_DefaultOpen,
		"Statistics" ) )
	{
		uint32_t gpuFrameTime, gpuSamples, gpuPrimitives;

		re.GetGPUMemStats( &memstats );
		re.GetGPUFrameStats( &gpuFrameTime, &gpuSamples, &gpuPrimitives );

		ImGui::Text( "FrameNumber: %lu", com_frameNumber );
		ImGui::Text( "FrameTime: %i", gi.frametime );
		ImGui::Separator();
		ImGui::Text( "FrontEnd Time: %0.04f milliseconds", time_frontend * 0.0001f );
		ImGui::Text( "BackEnd Time: %0.04f milliseconds", time_backend * 0.0001f );
		ImGui::Text( "PostProcessing Time: %0.04f milliseconds", gi.pc.postprocessMsec * 0.0001f );
		ImGui::Text( "GPU Time: %0.04f milliseconds", gpuFrameTime * 0.0001f );
		ImGui::Separator();
		ImGui::Text( "Draw Calls: %lu", gi.pc.c_drawCalls );
		ImGui::Text( "Buffer Binds: %u", gi.pc.c_bufferBinds );
		ImGui::Text( "Processed Indices: %u", gi.pc.c_bufferIndices );
		ImGui::Text( "Processed Vertices: %u", gi.pc.c_bufferVertices );
		ImGui::Text( "Generic Shader Draws: %u", gi.pc.c_genericDraws );
		ImGui::Text( "Lightall Shader Draws: %u", gi.pc.c_lightallDraws );
		ImGui::Text( "Overdraw: %u", gi.pc.c_overDraw );
		ImGui::Separator();
		ImGui::Text( "(EST) Texture Memory Used: %u", memstats.estTextureMemUsed );
		ImGui::Text( "(EST) Total Buffer Memory Used: %u", memstats.estBufferMemUsed );
		ImGui::Text( "(EST) Vertex Buffer Memory: %u", memstats.estVertexMemUsed );
		ImGui::Text( "(EST) Index Buffer Memory: %u", memstats.estIndexMemUsed );
		ImGui::Text( "Total Buffers Generated: %u", memstats.numBuffers );
		ImGui::TreePop();
	}

	ImGui::End();
}

extern "C" void UI_Refresh( int32_t realtime )
{
	extern cvar_t *in_joystick;
	static qboolean windowFocus = qfalse;

	ui->realtime = realtime;
	ui->frametime = ui->frametime - realtime;

	UI_DrawFPS();

//	if ( !ui_active->i ) {
//		UI_EscapeMenuToggle();
//		if ( ui->menustate != UI_MENU_NONE ) {
//			UI_SetActiveMenu( UI_MENU_PAUSE );
//		}
//		return;
//	}


	{
        refdef_t refdef;

        memset( &refdef, 0, sizeof( refdef ) );
        refdef.x = 0;
        refdef.y = 0;
        refdef.width = ui->gpuConfig.vidWidth;
        refdef.height = ui->gpuConfig.vidHeight;
        refdef.time = 0;
        refdef.flags = RSF_ORTHO_TYPE_SCREENSPACE | RSF_NOWORLDMODEL;

//        re.ClearScene();
//        re.DrawImage( 0, 0, refdef.width, refdef.height, 0, 0, 1, 1, ui->backdrop );
//		re.RenderScene( &refdef );
    }

	if ( r_gpuDiagnostics->i ) {
		UI_DrawGPUStats();
	}

	if ( !( Key_GetCatcher() & KEYCATCH_UI ) ) {
		return;
	}

	if ( ui->activemenu && ui->activemenu->fullscreen ) {
		UI_DrawMenuBackground();
	}

	if ( ui_debugOverlay->i && ui->menustate != UI_MENU_SPLASH ) {
		UI_DrawDebugOverlay();
	}

	if ( in_joystick->i ) {
		UI_AddJoystickKeyEvents();
	}

	if ( ui->activemenu ) {
		if ( ui->activemenu->track != FS_INVALID_HANDLE ) {
			Snd_SetLoopingTrack( ui->activemenu->track );
		}

///		if ( !( Key_GetCatcher() & KEYCATCH_CONSOLE ) ) {
			if ( ui->activemenu->draw ) {
				ui->activemenu->draw();
			} else {
				Menu_Draw( ui->activemenu );
			}
	//	}

//		if( ui->GetFirstDraw() ) {
//			ui->MouseEvent( 0, 0 );
//			ui->SetFirstDraw( qfalse );
//		}
	}
/*
	// draw cursor
//	ui->SetColor( NULL );
//	ui->DrawHandlePic( ui->GetCursorX() - 16, ui->GetCursorY() - 16, 32, 32, cursor);

#ifdef _NOMAD_DEBUG
	if (ui->IsDebug()) {
		// cursor coordinates
		ui->DrawString( 0, 0, va("(%d,%d)", ui->GetCursorX(), ui->GetCursorY()), UI_LEFT|UI_SMALLFONT, color_red );
	}
#endif

	// delay playing the enter sound until after the
	// menu has been drawn, to avoid delay while
	// caching images
	if (m_entersound) {
		Snd_PlaySfx( menu_in_sound );
		m_entersound = qfalse;
	}
	*/
}

/*
#define TRACE_FRAMES 60

typedef struct {
	double cpuFrames[TRACE_FRAMES];
	uint32_t gpuTimes[TRACE_FRAMES];
	uint32_t gpuSamples[TRACE_FRAMES];
	uint32_t gpuPrimitives[TRACE_FRAMES];

	uint64_t virtualHeapUsed;
	uint64_t physicalHeapUsed;
	uint64_t stackMemoryUsed;

	uint32_t gpuTimeMin;
	uint32_t gpuTimeMax;
	uint32_t gpuTimeAvg;
	int32_t gpuTimeIndex;
	uint32_t gpuTimePrevious;

	qboolean cpuNewMin;
	qboolean cpuNewMax;
	double cpuMin;
	double cpuMax;
	double cpuAvg;
	int32_t cpuIndex;
	double cpuPrevious;
} sys_stats_t;


// NOTE: got all the win32 code from stack overflow, don't mess with it!!!!!
#ifdef _WIN32
#include <windows.h>
#include <psapi.h>
#include <pdh.h>

static ULARGE_INTEGER lastCPU, lastSysCPU, lastUserCPU;
static DWORD dwNumProcessors;
static HANDLE self;
#endif

#ifdef __unix__
#include <unistd.h>
#include <sys/resource.h>
#include <sys/times.h>

static FILE *cpuInfo;

static int32_t numProcessors;
static clock_t lastCPU, lastSysCPU, lastUserCPU;
#endif

CallBeforeMain(Sys_InitCPUMonitor)
{
#ifdef _WIN32
	SYSTEM_INFO sysInfo{};
	FILETIME ftime, fsys, fuser;
	
	GetSystemInfo( &sysInfo );
	dwNumProcessors = sysInfo.dwNumberOfProcessors;
	
	GetSystemTimeAsFileTime( &ftime );
	memcpy( &lastCPU, &ftime, sizeof(FILETIME) );
	
	self = GetCurrentProcess();
	GetProcessTimes( self, &ftime, &ftime, &fsys, &fuser );
	memcpy( &lastSysCPU, &fsys, sizeof(FILETIME) );
	memcpy( &lastUserCPU, &fuser, sizeof(FILETIME) );
#elif defined(__unix__)
	struct tms timeSample;
	char line[128];

	cpuInfo = fopen( "/proc/cpuinfo", "r" );
	if ( !cpuInfo ) {
		Sys_Error( "Sys_InitCPUMonitor: failed to open /proc/cpuinfo in readonly mode!" );
	}
	
	numProcessors = 0;
	lastCPU = times( &timeSample );
	lastSysCPU = timeSample.tms_stime;
	lastUserCPU = timeSample.tms_utime;
	
	while (fgets( line, sizeof(line), cpuInfo )) {
		if (strncmp( line, "processor", 9 ) == 0) {
			numProcessors++;
		}
	}

	fclose( cpuInfo );
#endif
}

static double Sys_GetCPUUsage( void )
{
#ifdef _WIN32
	FILETIME ftime, fsys, fuser;
	ULARGE_INTEGER now, sys, user;
	double percent;
	
	GetSystemTimeAsFileTime( &ftime );
	memcpy( &now, &ftime, sizeof(FILETIME) );
	
	GetProcessTimes( self, &ftime, &ftime, &fsys, &fuser );
	memcpy( &sys, &fsys, sizeof(FILETIME) );
	memcpy( &user, &fuser, sizeof(FILETIME) );
	
	percent = (sys.QuadPart - lastSysCPU.QuadPart) + (user.QuadPart - lastUserCPU.QuadPart);
	percent /= (now.QuadPart - lastCPU.QuadPart);
	percent /= dwNumProcessors;
	
	lastCPU = now;
	lastUserCPU = user;
	lastSysCPU = sys;
	
	return percent * 100;
#elif defined(__APPLE__)
#elif defined(__unix__)
	struct tms timeSample;
	clock_t now;
	double percent;
	
	now = times( &timeSample );
	if (now <= lastCPU || timeSample.tms_stime < lastSysCPU || timeSample.tms_utime < lastUserCPU) {
		// overflow detection, just skip this value
		percent = -1.0f;
	}
	else {
		percent = (timeSample.tms_stime - lastSysCPU) + (timeSample.tms_utime - lastUserCPU);
		percent /= (now - lastCPU);
		percent /= numProcessors;
		percent *= 100.0f;
	}
	
	lastCPU = now;
	lastSysCPU = timeSample.tms_stime;
	lastUserCPU = timeSample.tms_utime;
	
	return percent;
#endif
}

#ifdef __unix__
static bool Posix_GetProcessMemoryUsage( uint64_t *virtualMem, uint64_t *physicalMem )
{
	char line[1024];
	int64_t unused;
	FILE *self;

	self = fopen( "/proc/self/status", "r" );
	if ( !self ) {
		N_Error( ERR_FATAL, "Posix_GetProcessMemoryUsage: failed to open /proc/self/status" );
	}

	while ( fscanf( self, "%1023s", line ) > 0 ) {
		if ( strstr( line, "VmRSS:" ) ) {
			fscanf( self, " %lu", physicalMem );
		} else if ( strstr( line, "VmSize:" ) ) {
			fscanf( self, " %lu", virtualMem );
		}
	}

	*physicalMem *= 1024;
	*virtualMem *= 1024;

	fclose( self );
	
	return true;
}
#endif

static void Sys_GetMemoryUsage( sys_stats_t *usage )
{
#ifdef _WIN32
	PROCESS_MEMORY_COUNTERS_EX pmc{};
	MEMORYSTATUSEX memInfo{};
	PDH_HQUERY cpuQuery;
	PDH_HCOUNTER cpuTotal;
	GetProcessMemoryInfo( GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS *)&pmc, sizeof(pmc) );
	memInfo.dwLength = sizeof(MEMORYSTATUSEX);
	GlobalMemoryStatusEx( &memInfo );
	
	usage->virtualHeapUsed = pmc.PrivateUsage;
	usage->physicalHeapUsed = pmc.WorkingSetSize;
#elif defined(__APPLE__)
#elif defined(__unix__)
	Posix_GetProcessMemoryUsage( &usage->virtualHeapUsed, &usage->physicalHeapUsed );
#endif
}

static sys_stats_t *stats;

static void Sys_GetCPUStats( void )
{
	const double cpuTime = Sys_GetCPUUsage();
	double total, cpu;
	
	if ( cpuTime < stats->cpuMin ) {
		stats->cpuMin = cpuTime;
		stats->cpuNewMin = qtrue;
	} else if ( cpuTime > stats->cpuMax ) {
		stats->cpuMax = cpuTime;
		stats->cpuNewMax = qtrue;
	}
	
	stats->cpuPrevious = cpuTime;
	
	stats->cpuFrames[stats->cpuIndex % TRACE_FRAMES] = cpuTime;
	stats->cpuIndex++;
	
	if (stats->cpuIndex > TRACE_FRAMES) {
		// average multiple frames of cpu usage to smooth changes out
		for (uint32_t i = 0; i < TRACE_FRAMES; i++) {
			stats->cpuAvg += stats->cpuFrames[i];
		}
		
		stats->cpuAvg /= TRACE_FRAMES;
	}
}

static void Sys_DrawMemoryUsage( void )
{
	ImGui::SeparatorText( "Memory Usage/Stats" );
	ImGui::Text( "Blocks Currently Allocated: %i", SDL_GetNumAllocations() );
	ImGui::Text( "Total Virtual Memory Used: %lu", stats->virtualHeapUsed );	
    ImGui::Text( "Total Physical Memory Used: %lu", stats->physicalHeapUsed );
    ImGui::Text( "Total Stack Memory Remaining: %lu", Sys_StackMemoryRemaining() );
}

static void Sys_GPUStatFrame( uint32_t stat, uint32_t *min, uint32_t *max, uint32_t *avg, uint32_t *prev, uint32_t *frames, int32_t *index )
{
	if ( stat < *min ) {
		*min = stat;
	} else if ( stat > *max ) {
		*max = stat;
	}

	*prev = stat;

	frames[*index % TRACE_FRAMES] = stat;
	(*index)++;

	if ( *index > TRACE_FRAMES ) {
		// average multiple frames of stats to smooth changes out
		for ( uint32_t i = 0; i < TRACE_FRAMES; i++ ) {
			*avg += frames[i];
		}

		*avg /= TRACE_FRAMES;
	}
}

static void Sys_DrawGPUStats( void )
{
	uint32_t time, samples, primitives;

	re.GetGPUFrameStats( &time, &samples, &primitives );

	Sys_GPUStatFrame( time, &stats->gpuTimeMin, &stats->gpuTimeMax, &stats->gpuTimeAvg, &stats->gpuTimePrevious, stats->gpuTimes,
		&stats->gpuTimeIndex );

	ImGui::SeparatorText( "GPU Frame Statistics" );
	ImGui::Text( "Time Elapsed (Average): %u", stats->gpuTimeAvg );
	ImGui::Text( "Samples Passed: %u", samples );
	ImGui::Text( "Primitives Generated: %u", primitives );
}

static void Sys_DrawCPUUsage( void )
{
	ImGui::SeparatorText( "CPU Usage" );
	ImGui::Text( "Number of CPU Cores: %i", SDL_GetCPUCount() );

	ImGui::BeginTable( " ", 4 );
    {
    	ImGui::TableNextColumn();
    	ImGui::TextUnformatted( "average" );
    	ImGui::TableNextColumn();
    	ImGui::TextUnformatted( "min" );
    	ImGui::TableNextColumn();
    	ImGui::TextUnformatted( "max" );
    	ImGui::TableNextColumn();
    	ImGui::TextUnformatted( "last" );

    	ImGui::TableNextRow();

    	ImGui::TableNextColumn();
    	ImGui::Text( "%03.4f", stats->cpuAvg );
    	ImGui::TableNextColumn();

		if ( stats->cpuNewMin ) {
			ImGui::TextColored( ImVec4( g_color_table[ ColorIndex( S_COLOR_GREEN ) ] ), "%03.4f", stats->cpuMin );
			stats->cpuNewMin = qfalse;
		} else {
			ImGui::Text( "%03.4f", stats->cpuMin );
		}

    	ImGui::TableNextColumn();

		if ( stats->cpuNewMax ) {
    		ImGui::TextColored( ImVec4( g_color_table[ ColorIndex( S_COLOR_RED ) ] ), "%03.4f", stats->cpuMax );
			stats->cpuNewMax = qfalse;
		} else {
			ImGui::Text( "%03.4f", stats->cpuMax );
		}
		
		ImGui::TableNextColumn();
    	ImGui::Text( "%03.4f", stats->cpuPrevious );
    }
    ImGui::EndTable();
}

void Sys_DisplayEngineStats( void )
{
	const int windowFlags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBackground
						| ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMouseInputs | ImGuiWindowFlags_NoTitleBar;
	extern ImFont *RobotoMono;
	ImVec2 windowPos;

	if ( !ui_diagnostics->i ) {
		return;
	}

	if ( ui->GetState() == STATE_CREDITS || ui->GetState() == STATE_LEGAL || ImGui::IsWindowCollapsed() ) {
		// pay respects, don't block the words

		// if its in the legal section, just don't draw it

		// if we're collapsing, we'll segfault when drawing CPU usage
		return;
	}
	
	ImGui::Begin( "Engine Diagnostics", NULL, windowFlags );

	windowPos.x = 730 * ui->scale + ui->bias;
	windowPos.y = 16 * ui->scale;
	ImGui::SetWindowPos( windowPos );

	if ( RobotoMono ) {
		FontCache()->SetActiveFont( RobotoMono );
	}
	const float fontScale = ImGui::GetFont()->Scale;
	ImGui::SetWindowFontScale( ( ImGui::GetFont()->Scale * 0.75f ) * ui->scale );

	if ( !stats ) {
		stats = (sys_stats_t *)Hunk_Alloc( sizeof(sys_stats_t), h_low );
	}

	// draw the cpu usage chart
	if ( ui_diagnostics->i == 1 ) {
		Sys_GetCPUStats();
		Sys_DrawCPUUsage();
        return;
    }
	// draw memory statistics
	else if ( ui_diagnostics->i == 2 ) {
        Sys_GetMemoryUsage( stats );

        Sys_DrawMemoryUsage();
        return;
	}

	//
	// fetch the data
	//

	Sys_GetCPUStats();
	Sys_GetMemoryUsage( stats );
	
	//
	// draw EVERYTHING
	//
	UI_DrawFPS();

	ImGui::Text( "Frame Number: %lu", com_frameNumber );

	Sys_DrawCPUUsage();
    Sys_DrawMemoryUsage();
	Sys_DrawGPUStats();

	ImGui::SeparatorText( "Computer Information" );
	
	ImGui::Text( "%ix%i", gi.gpuConfig.vidWidth, gi.gpuConfig.vidHeight );
	ImGui::Text( "%s", ui->GetConfig().version_string );
	ImGui::Text( "%s", ui->GetConfig().vendor_string );
	ImGui::Text( "%s", ui->GetConfig().renderer_string );
    ImGui::Text( "%s", ui_cpuString->s );

	ImGui::SetWindowFontScale( fontScale );

	ImGui::End();
}
*/
