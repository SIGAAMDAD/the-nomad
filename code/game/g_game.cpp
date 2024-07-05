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

#include "g_game.h"
#include "g_sound.h"
#include "g_world.h"
#include "g_archive.h"
#include "../rendercommon/imgui.h"
#include "../rendercommon/imgui_impl_sdl2.h"
#include "../rendercommon/imgui_impl_opengl3.h"
#include "imgui_memory_editor.h"
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "g_threads.h"
#include "../engine/n_steam.h"
#include "snd_public.h"

CModuleLib *g_pModuleLib;
CModuleInfo *sgvm;
renderExport_t re;
gameInfo_t gi;

cvar_t *r_glDebug;
cvar_t *r_allowLegacy;
cvar_t *r_displayRefresh;
cvar_t *r_allowSoftwareGL;
cvar_t *vid_xpos;
cvar_t *vid_ypos;
cvar_t *g_renderer; // current rendering api
cvar_t *r_fullscreen;
cvar_t *r_customWidth;
cvar_t *r_customHeight;
cvar_t *r_aspectRatio;
cvar_t *r_driver;
cvar_t *r_noborder;
cvar_t *r_drawFPS;
cvar_t *r_swapInterval;
cvar_t *r_mode;
cvar_t *r_customPixelAspect;
cvar_t *r_colorBits;
cvar_t *g_stencilBits;
cvar_t *g_depthBits;
cvar_t *r_multisampleAmount;
cvar_t *r_multisampleType;
cvar_t *g_drawBuffer;
cvar_t *g_paused;
cvar_t *r_debugCamera;

static void *renderLib;

#if 0
#if defined(__OS2__) || defined(_WIN32)
static SDL_Thread *PFN_SDL_CreateThread(SDL_ThreadFunction fn, const char *name, void *data)
{
	return SDL_CreateThread(fn, name, data);
}
static SDL_Thread *PFN_SDL_CreateThreadWithStackSize(SDL_ThreadFunction fn, const char *name, const size_t stacksize, void *data)
{
	return SDL_CreateThreadWithStackSize(fn, name, stacksize, data);
}
#endif
#endif

static void GDR_ATTRIBUTE((format(printf, 2, 3))) GDR_DECL G_RefPrintf( int level, const char *fmt, ... )
{
    va_list argptr;
    char msg[MAXPRINTMSG];

    va_start( argptr, fmt );
    N_vsnprintf( msg, sizeof(msg), fmt, argptr );
    va_end( argptr );

    switch ( level ) {
    case PRINT_INFO:
        Con_Printf( "%s", msg );
        break;
    case PRINT_DEVELOPER:
        Con_DPrintf( "%s", msg );
        break;
    case PRINT_WARNING:
        Con_Printf( COLOR_YELLOW "WARNING: %s", msg );
        break;
    case PRINT_ERROR:
        Con_Printf( COLOR_RED "ERROR: %s", msg );
        break;
    default:
        N_Error( ERR_FATAL, "G_RefPrintf: Bad print level" );
    };
}

static void *G_RefMalloc( uint64_t size ) {
    return Z_Malloc( size, TAG_RENDERER );
}

static void *G_RefRealloc( void *ptr, uint64_t nsize ) {
    return Z_Realloc( ptr, nsize, TAG_RENDERER );
}

static void G_RefFreeAll( void ) {
    Z_FreeTags( TAG_RENDERER );
}

static void G_RefImGuiFree( void *ptr, void * ) {
    if ( ptr != NULL ) {
        Z_Free( ptr );
    }
}

static void G_RefImGuiShutdown( void ) {
    const char *iniData;
    size_t iniDataSize;

    ImGuiIO& io = ImGui::GetIO();
    io.BackendRendererName = NULL;
    io.BackendRendererUserData = NULL;
    io.BackendLanguageUserData = NULL;
    io.BackendFlags = ImGuiBackendFlags_None;

    Con_Printf( "ImGui_Shutdown: shutting down ImGui renderer backend...\n" );

    FontCache()->ClearCache();

    iniData = ImGui::SaveIniSettingsToMemory( &iniDataSize );
    FS_WriteFile( LOG_DIR "/imgui.ini", iniData, iniDataSize );

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
    ImGui::SetCurrentContext( NULL );

    // clean everything up
    Z_FreeTags( TAG_IMGUI );
}

static void G_RefImGuiNewFrame( void ) {
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableSetMousePos;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
    io.DisplaySize.x = r_customWidth->i;
    io.DisplaySize.y = r_customHeight->i;
    io.DeltaTime = 1.0 / com_maxfps->i;
    io.Fonts->Flags |= ImFontAtlasFlags_NoBakedLines;
    io.BackendUsingLegacyNavInputArray = false;
    io.BackendUsingLegacyKeyArrays = false;
    io.WantSaveIniSettings = false;

    ImGuiStyle& style = ImGui::GetStyle();

    switch ( r_multisampleType->i ) {
    case AntiAlias_2xMSAA:
    case AntiAlias_4xMSAA:
        style.AntiAliasedFill = true;
        break;
    case AntiAlias_16xMSAA:
    case AntiAlias_32xMSAA:
        style.AntiAliasedLines = true;
        style.AntiAliasedFill = true;
        break;
    };

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();
}

static void G_SetScaling(float factor, uint32_t captureWidth, uint32_t captureHeight)
{
    if ( gi.con_factor != factor ) {
        // rescale console
        con_scale->modified = qtrue;
    }

    gi.con_factor = factor;
    
    // set custom capture resolution
    gi.captureWidth = captureWidth;
    gi.captureHeight = captureHeight;
}

static void *G_RefImGuiMalloc( size_t size ) {
    return Z_Malloc( size, TAG_IMGUI );
}

//
// G_RefImGuiInit: called during internal renderer initialization
// renderContext can be either a SDL_GLContext or SDL_Renderer, or NULL if using D3D11, Vulkan, or Metal
//
static void G_RefImGuiInit( void *shaderData, const void *importData ) {
    extern SDL_Window *SDL_window;
    extern SDL_GLContext SDL_glContext;
    union {
        void *v;
        char *b;
    } f;
    uint64_t nLength;

    IMGUI_CHECKVERSION();
    ImGui::SetAllocatorFunctions( (ImGuiMemAllocFunc)G_RefImGuiMalloc, (ImGuiMemFreeFunc)G_RefImGuiFree );
    ImGui::CreateContext();

    ImGui::LogToFile( 4, LOG_DIR "/imgui_log.txt" );

    nLength = FS_LoadFile( LOG_DIR "/imgui.ini", &f.v );
    if ( !nLength || !f.v ) {
        Con_Printf( COLOR_RED "Error loading " LOG_DIR "/imgui.ini!\n" );
    } else {
        ImGui::LoadIniSettingsFromMemory( f.b, nLength );
        FS_FreeFile( f.v );
    }

    ImGuiIO& io = ImGui::GetIO();

    io.BackendPlatformName = OS_STRING;
    io.WantSaveIniSettings = false;

    Con_Printf( "-------- ImGui_Init( %s ) --------\n", g_renderer->s );

    // init font cache
    g_pFontCache = new ( Hunk_Alloc( sizeof( *g_pFontCache ), h_low ) ) CUIFontCache();

    if ( !N_stricmp( g_renderer->s, "opengl" ) ) {
        ImGui_ImplSDL2_InitForOpenGL( SDL_window, SDL_glContext );
        ImGui_ImplOpenGL3_Init( shaderData, NULL, (const imguiGL3Import_t *)importData);
    }
    else if ( !N_stricmp( g_renderer->s, "vulkan" ) ) {
        ImGui_ImplSDL2_InitForVulkan( SDL_window );
    }
}

static void GLM_TransformToGL( const vec3_t world, vec3_t *xyz, const glm::mat4& vpm )
{
    glm::mat4 mvp, model;
    glm::vec4 pos;

    model = glm::translate( glm::mat4( 1.0f ), glm::vec3( world[0], world[1], world[2] ) );
    mvp = gi.viewProjectionMatrix * model;

    const glm::vec4 positions[4] = {
#if 0
        { 0.0f, 1.0f, 0.0f, 1.0f },
        { 1.0f, 1.0f, 0.0f, 1.0f },
        { 1.0f, 0.0f, 0.0f, 1.0f },
        { 0.0f, 0.0f, 0.0f, 1.0f }
#else
        { 0.0f, 1.0f, 0.0f, 1.0f },
        { 1.0f, 1.0f, 0.0f, 1.0f },
        { 1.0f, 0.0f, 0.0f, 1.0f },
        { 0.0f, 0.0f, 0.0f, 1.0f }
#endif
//        { 1.0f,  1.0f, 0.0f, 1.0f },
//        { 1.0f,  0.0f, 0.0f, 1.0f },
//        { 0.0f,  0.0f, 0.0f, 1.0f },
//        { 0.0f,  1.0f, 0.0f, 1.0f },
    };

    for ( uint32_t i = 0; i < 4; i++ ) {
        pos = mvp * positions[i];
        VectorCopy( xyz[i], pos );
    }
}

static void GLM_TransformCameraPosition( const glm::mat4& viewProjectionMatrix )
{
    glm::vec4 position;
    glm::mat4 model, mvp;

    model = glm::translate( gi.viewProjectionMatrix, glm::vec3( gi.cameraPos[0], gi.cameraPos[1], gi.cameraZoom ) );
    mvp  = gi.viewProjectionMatrix * model;

    position = mvp * glm::vec4( 0.0f, 0.0f, 0.0f, 1.0f );
}

static void GLM_MakeVPM( const vec4_t ortho, float *zoom, float zNear, float zFar, vec3_t origin, mat4_t vpm,
    mat4_t projection, mat4_t view, uint32_t orthoFlags )
{
    glm::mat4 transpose;
    glm::vec4 position;

    // first transform the origin
    gi.projectionMatrix = glm::ortho( ortho[0], ortho[1], ortho[2], ortho[3], zNear, zFar );

    if ( ( orthoFlags & RSF_ORTHO_BITS ) != RSF_ORTHO_TYPE_SCREENSPACE ) {
        position = gi.projectionMatrix * glm::mat4( 1.0f ) * glm::vec4(
            ( gi.cameraPos[0] * 8.0f ) + ( gi.cameraZoom * 100.0f ),
            ( gi.cameraPos[1] * 8.0f ) + ( gi.cameraZoom * 1000.0f ),
            0.0f, 1.0f
        );
        transpose = glm::translate( glm::mat4( 1.0f ), glm::vec3( position[0], position[1], 0.0f ) )
                    * glm::scale( glm::mat4( 1.0f ), glm::vec3( gi.cameraZoom ) );
        gi.viewMatrix = glm::inverse( transpose );
        gi.viewProjectionMatrix = gi.projectionMatrix * gi.viewMatrix;
    } else {
        gi.viewMatrix = glm::mat4( 1.0f );
        gi.viewProjectionMatrix = gi.projectionMatrix;
    }

    VectorCopy( origin, gi.cameraPos );
    *zoom = gi.cameraZoom;

    memcpy( &projection[0][0], &gi.projectionMatrix[0][0], sizeof( mat4_t ) );
    memcpy( &view[0][0], &gi.viewMatrix[0][0], sizeof( mat4_t ) );
    memcpy( &vpm[0][0], &gi.viewProjectionMatrix[0][0], sizeof( mat4_t ) );
}

void G_SetCameraData( const vec2_t origin, float zoom, float rotation ) {
    VectorCopy2( gi.cameraWorldPos, origin );
    gi.cameraZoom = zoom;
}

static float *GLM_Mat4Transform( const mat4_t m, const vec4_t p ) {
    static vec4_t out;
    glm::vec4 p0;
    glm::mat4 m0;

    memcpy( &m0[0][0], &m[0][0], sizeof(mat4_t) );
    p0 = m0 * glm::vec4( p[0], p[1], p[2], p[3] );
    VectorCopy4( out, p0 );

    return out;
}

static void GLM_TransformToGL( const vec3_t world, vec3_t *xyz, float scale, float rotation, mat4_t vpm )
{
    glm::mat4 mvp, model;
    glm::vec4 pos;

    model = glm::translate( glm::mat4( 1.0f ), glm::vec3( world[0], world[1], world[2] ) )
            * glm::scale( glm::mat4( 1.0f ), glm::vec3( scale, scale, 0.0f ) )
            * glm::rotate( glm::mat4( 1.0f ), (float)DEG2RAD( rotation ), glm::vec3( 0, 0, 1 ) );
    mvp = gi.viewProjectionMatrix * model;

    const glm::vec4 positions[4] = {
    #if 0
        { 0.0f, 1.0f, 0.0f, 1.0f },
        { 0.0f, 0.0f, 0.0f, 1.0f },
        { 1.0f, 0.0f, 0.0f, 1.0f },
        { 1.0f, 1.0f, 0.0f, 1.0f }
    #else
        { 1.0f,  1.0f, 0.0f, 1.0f },
        { 1.0f,  0.0f, 0.0f, 1.0f },
        { 0.0f,  0.0f, 0.0f, 1.0f },
        { 0.0f,  1.0f, 0.0f, 1.0f },
    #endif
    };

    for ( uint32_t i = 0; i < 4; i++ ) {
        pos = mvp * positions[i];
        VectorCopy( xyz[i], pos );
    }
}

qboolean G_CheckWallHit( const vec3_t origin, dirtype_t dir ) {
    return g_world->CheckWallHit( origin, dir );
}

static void G_InitRenderRef(void)
{
    refimport_t import;
    renderExport_t *ret;
    GetRenderAPI_t GetRenderAPI;
    char dllName[MAX_OSPATH];
    const char *dllPrefix;

    Con_Printf( "----- Initializing Renderer ----\n" );

    dllPrefix = g_renderer->s;

    if ( !N_stricmp( g_renderer->s, "vulkan" ) || !N_stricmp( g_renderer->s, "sdl2" ) ) {
        N_Error( ERR_FATAL, "Vulkan & SDL2 rendering not available yet, will be tho in the future... ;)" );
    }
#if defined (__linux__) && defined(__i386__)
#define REND_ARCH_STRING "x86"
#else
#define REND_ARCH_STRING "x64"
#endif

    snprintf( dllName, sizeof( dllName ), DLL_PREFIX "TheNomad.RenderLib-%s." REND_ARCH_STRING DLL_EXT, dllPrefix );
    renderLib = Sys_LoadDLL(dllName);
    if ( !renderLib ) {
        Cvar_ForceReset( "g_renderer" );
        snprintf( dllName, sizeof( dllName ), DLL_PREFIX "TheNomad.RenderLib-%s." REND_ARCH_STRING DLL_EXT, dllPrefix );
        renderLib = Sys_LoadDLL( dllName );
        if ( !renderLib ) {
            N_Error( ERR_FATAL, "Failed to load rendering library '%s', possible system error: %s", dllName, Sys_GetDLLError() );
        }
    }

    GetRenderAPI = (GetRenderAPI_t)Sys_GetProcAddress( renderLib, "GetRenderAPI" );
    if ( !GetRenderAPI ) {
        N_Error( ERR_FATAL, "Can't load symbol GetRenderAPI" );
        return;
    }

    g_renderer->modified = qfalse;

    memset( &import, 0, sizeof( import ) );

    import.Cmd_AddCommand = Cmd_AddCommand;
    import.Cmd_RemoveCommand = Cmd_RemoveCommand;
    import.Cmd_ExecuteCommand = Cmd_ExecuteString;
    import.Cmd_Argc = Cmd_Argc;
    import.Cmd_Argv = Cmd_Argv;
    import.Cmd_ArgsFrom = Cmd_ArgsFrom;
    import.Printf = G_RefPrintf;
    import.Error = N_Error;
#ifdef _NOMAD_DEBUG
    import.Hunk_AllocDebug = Hunk_AllocDebug;
#else
    import.Hunk_Alloc = Hunk_Alloc;
#endif
    import.Hunk_AllocateTempMemory = Hunk_AllocateTempMemory;
    import.Hunk_FreeTempMemory = Hunk_FreeTempMemory;
    import.Malloc = G_RefMalloc;
    import.Realloc = G_RefRealloc;
    import.Free = Z_Free;
    import.FreeAll = G_RefFreeAll;
    import.CopyString = CopyString;

    import.GLimp_Init = G_InitDisplay;
    import.GLimp_InitGamma = GLimp_InitGamma;
#ifdef USE_OPENGL_API
    import.GLimp_EndFrame = GLimp_EndFrame;
    import.GLimp_SetGamma = GLimp_SetGamma;
    import.GLimp_LogComment = GLimp_LogComment;
    import.GLimp_Shutdown = GLimp_Shutdown;
    import.GLimp_Minimize = GLimp_Minimize;
    import.GLimp_HideFullscreenWindow = GLimp_HideFullscreenWindow;
    import.GL_GetProcAddress = GL_GetProcAddress;
#endif

    import.G_SetScaling = G_SetScaling;
    import.G_SaveJPGToBuffer = G_SaveJPGToBuffer;
    import.G_SaveJPG = G_SaveJPG;
    import.G_LoadJPG = G_LoadJPG;

    import.GLM_TransformToGL = GLM_TransformToGL;
    import.GLM_MakeVPM = GLM_MakeVPM;

    import.Milliseconds = Sys_Milliseconds;

    import.Key_IsDown = Key_IsDown;

    import.FS_LoadFile = FS_LoadFile;
    import.FS_FreeFile = FS_FreeFile;
    import.FS_WriteFile = FS_WriteFile;
    import.FS_FileExists = FS_FileExists;
    import.FS_FreeFileList = FS_FreeFileList;
    import.FS_ListFiles = FS_ListFiles;
    import.FS_FOpenRead = FS_FOpenRead;
    import.FS_FOpenWrite = FS_FOpenWrite;
    import.FS_FClose = FS_FClose;
    import.FS_Read = FS_Read;
    import.FS_Write = FS_Write;
    import.FS_Remove = FS_Remove;
    import.FS_HomeRemove = FS_HomeRemove;

    import.Cvar_Get = Cvar_Get;
    import.Cvar_Set = Cvar_Set;
    import.Cvar_Reset = Cvar_Reset;
    import.Cvar_SetGroup = Cvar_SetGroup;
    import.Cvar_CheckRange = Cvar_CheckRange;
    import.Cvar_SetDescription = Cvar_SetDescription;
    import.Cvar_VariableStringBuffer = Cvar_VariableStringBuffer;
    import.Cvar_VariableString = Cvar_VariableString;
    import.Cvar_VariableInteger = Cvar_VariableInteger;
    import.Cvar_CheckGroup = Cvar_CheckGroup;
    import.Cvar_ResetGroup = Cvar_ResetGroup;

    import.ImGui_Init = G_RefImGuiInit;
    import.ImGui_Shutdown = G_RefImGuiShutdown;
    import.ImGui_NewFrame = G_RefImGuiNewFrame;

    import.Sys_LoadDLL = Sys_LoadDLL;
    import.Sys_CloseDLL = Sys_CloseDLL;
    import.Sys_GetProcAddress = Sys_GetProcAddress;

    ret = GetRenderAPI( NOMAD_VERSION_FULL, &import );

    Con_Printf( "-------------------------------\n" );
	if ( !ret ) {
		N_Error( ERR_FATAL, "Couldn't initialize refresh" );
	}

    re = *ret;
}

static void G_InitModuleLib( void )
{
    char dllName[MAX_OSPATH];
    version_t gameVersion;
    moduleImport_t import;

    Con_Printf( "----- Initializing Module Library -----\n" );


/*
    Com_snprintf( dllName, sizeof(dllName), "thenomad_modulelib_" DLL_PREFIX DLL_EXT );
    moduleLib = Sys_LoadDLL( dllName );
    if ( !moduleLib ) {
        N_Error( ERR_FATAL, "G_initModuleLib: failed to load %s", dllName );
    }

    GetModuleAPI = (GetModuleAPI_t)Sys_GetProcAddress( moduleLib, "InitModuleLib" );
    if ( !GetModuleAPI ) {
        N_Error( ERR_FATAL, "Couldn't load symbol InitModuleLib" );
    }
    */

    gameVersion.m_nVersionMajor = NOMAD_VERSION;
    gameVersion.m_nVersionUpdate = NOMAD_VERSION_UPDATE;
    gameVersion.m_nVersionPatch = NOMAD_VERSION_PATCH;

    g_pModuleLib = InitModuleLib( NULL, &re, gameVersion );
    if ( !g_pModuleLib ) {
        N_Error( ERR_FATAL, "InitModuleLib failed" );
    }

    Con_Printf( "-------------------------------\n" );
}

static void G_InitRenderer( void )
{
    PROFILE_FUNCTION();

    if ( !re.BeginRegistration ) {
        G_InitRenderRef();
    }

    re.BeginRegistration( &gi.gpuConfig );

    // load the character sets
//    gi.charSetShader = re.RegisterShader("gfx/bigchars");
    gi.whiteShader = re.RegisterShader("white");
    gi.consoleShader = re.RegisterShader("console");

//    g_console_field_width = ( ( gi.gpuConfig.vidWidth / smallchar_width ) ) - 2;

    // for 1920x1080 virtualized screen
	gi.biasY = 0;
	gi.biasX = 0;
	if ( gi.gpuConfig.vidWidth * 768.0f > gi.gpuConfig.vidHeight * 1024.0f ) {
		// wide screen, scale by height
		gi.scale = gi.gpuConfig.vidHeight * (1.0f/768.0f);
		gi.biasX = 0.5f * ( gi.gpuConfig.vidWidth - ( gi.gpuConfig.vidHeight * (1024.0f/768.0f) ) );
	} else {
		// no wide screen, scale by width
		gi.scale = gi.gpuConfig.vidWidth * (1.0f/1024.0f);
		gi.biasY = 0.5f * ( gi.gpuConfig.vidHeight - ( gi.gpuConfig.vidWidth * (768.0f/1024.0f) ) );
	}
}

void G_ShutdownRenderer( refShutdownCode_t code )
{
    if ( g_renderer && g_renderer->modified ) {
        code = REF_UNLOAD_DLL;
    }

    if ( re.Shutdown ) {
        PROFILE_SCOPE( "Renderer Shutdown" );
        re.Shutdown( code );
    }

    if ( renderLib ) {
        Sys_CloseDLL( renderLib );
        renderLib = NULL;
    }

    memset( &re, 0, sizeof( re ) );

    gi.rendererStarted = qfalse;
}

static void G_Vid_Restart( refShutdownCode_t code )
{
    PROFILE_FUNCTION();

    // clear and mute all sounds until next registration
    Snd_Shutdown();

    // shutdown VMs
    G_ShutdownVMs( qfalse );

    // shutdown the renderer and clear the renderer interface
	G_ShutdownRenderer( code ); // REF_KEEP_CONTEXT, REF_KEEP_WINDOW, REF_DESTROY_WINDOW

    // clear bff references
    FS_ClearBFFReferences( FS_SGAME_REF );

    gi.soundRegistered = qfalse;

    G_ShutdownArchiveHandler();

    G_ClearMem();

    // startup all the gamestate memory
    G_StartHunkUsers();

    // make sure all sounds have updated volumes
    Cbuf_ExecuteText( EXEC_APPEND, "updatevolume\n" );
}

static void G_GameInfo_f( void ) {
    Con_Printf( "--------- Game Information ---------\n" );
	Con_Printf( "state: %i\n", gi.state );
	Con_Printf( "User info settings:\n" );
	Info_Print( Cvar_InfoString( CVAR_USERINFO, NULL ) );
	Con_Printf( "--------------------------------------\n" );
}

/*
==================
G_OpenedBFFList_f
==================
*/
void G_OpenedBFFList_f( void ) {
	Con_Printf( "Opened BFF Names: %s\n", FS_LoadedBFFNames() );
}


/*
==================
G_ReferencedBFFList_f
==================
*/
static void G_ReferencedBFFList_f( void ) {
	Con_Printf( "Referenced BFF Names: %s\n", FS_ReferencedBFFNames() );
}

/*
* G_ViewMemory_f: use imgui's memory editor to look at the game's heap memory usage
* NOTE: only really use this for debugging
*/
static void G_ViewMemory_f( void ) {
    gi.oldState = gi.state;
    gi.state = GS_MEMORY_VIEW;

    Con_Printf( "Viewing memory of hunk block.\n" );
}


static void G_BeginRecord( void )
{
    version_t version;
    int difficulty;
    int cheat;
    char mapname[64];
    int length;
    extern eastl::vector<char *> com_consoleLines;
    extern int com_numConsoleLines;

    version.m_nVersionMajor = NOMAD_VERSION;
    version.m_nVersionUpdate = NOMAD_VERSION_UPDATE;
    version.m_nVersionPatch = NOMAD_VERSION_PATCH;

    memset( mapname, 0, sizeof( mapname ) );
    Cvar_VariableStringBuffer( "mapname", mapname, sizeof( mapname ) - 1 );

    difficulty = Cvar_VariableInteger( "sgame_Difficulty" );

    FS_Write( &version, sizeof( version ), gi.recordfile );

    length = strlen( FS_ReferencedBFFNames() ) + 1;
    FS_Write( &length, sizeof( length ), gi.recordfile );
    FS_Write( FS_ReferencedBFFNames(), length, gi.recordfile );

    // write the command line
    FS_Write( &com_numConsoleLines, sizeof( com_numConsoleLines ), gi.recordfile );
    for ( const auto& it : com_consoleLines ) {
        length = strlen( it ) + 1;
        FS_Write( &length, sizeof( length ), gi.recordfile );
        FS_Write( it, strlen( it ) + 1, gi.recordfile );
    }

    FS_Write( &difficulty, sizeof( difficulty ), gi.recordfile );
    FS_Write( mapname, sizeof( mapname ), gi.recordfile );

    // write cheats

    cheat = Cvar_VariableInteger( "sgame_cheats_enabled" );
    FS_Write( &cheat, sizeof( cheat ), gi.recordfile );

    cheat = Cvar_VariableInteger( "sgame_cheat_BlindMobs" );
    FS_Write( &cheat, sizeof( cheat ), gi.recordfile );

    cheat = Cvar_VariableInteger( "sgame_cheat_DeafMobs" );
    FS_Write( &cheat, sizeof( cheat ), gi.recordfile );
    
    cheat = Cvar_VariableInteger( "sgame_cheat_GodMode" );
    FS_Write( &cheat, sizeof( cheat ), gi.recordfile );
    
    cheat = Cvar_VariableInteger( "sgame_cheat_InfiniteAmmo" );
    FS_Write( &cheat, sizeof( cheat ), gi.recordfile );
    
    cheat = Cvar_VariableInteger( "sgame_cheat_InfiniteHealth" );
    FS_Write( &cheat, sizeof( cheat ), gi.recordfile );
    
    cheat = Cvar_VariableInteger( "sgame_cheat_InfiniteRage" );
    FS_Write( &cheat, sizeof( cheat ), gi.recordfile );
}

static void G_BeginDemoPlay( void )
{
    version_t version;
    int difficulty;
    int cheat;
    char mapname[64];
    int length;
    char *referencedBffs;
    char **commandLine;
    int numLines, i;

    FS_Read( &version, sizeof( version ), gi.demofile );

    FS_Read( &length, sizeof( length ), gi.demofile );
    referencedBffs = (char *)Z_Malloc( length, TAG_STATIC );
    FS_Read( referencedBffs, length, gi.demofile );

    FS_Read( &numLines, sizeof( numLines ), gi.demofile );
    commandLine = (char **)Z_Malloc( sizeof( *commandLine ) * numLines, TAG_STATIC );
    for ( i = 0; i < numLines; i++ ) {
        FS_Read( &length, sizeof( length ), gi.demofile );
        commandLine[i] = (char *)Z_Malloc( length, TAG_STATIC );
        FS_Read( commandLine[i], length, gi.demofile );
    }

    FS_Read( &difficulty, sizeof( difficulty ), gi.demofile );
    FS_Read( mapname, sizeof( mapname ), gi.demofile );

    FS_Read( &cheat, sizeof( cheat ), gi.demofile );
    Cvar_SetIntegerValue( "sgame_cheats_enabled", cheat );

    FS_Read( &cheat, sizeof( cheat ), gi.demofile );
    Cvar_SetIntegerValue( "sgame_cheat_BlindMobs", cheat );

    FS_Read( &cheat, sizeof( cheat ), gi.demofile );
    Cvar_SetIntegerValue( "sgame_cheat_DeafMobs", cheat );

    FS_Read( &cheat, sizeof( cheat ), gi.demofile );
    Cvar_SetIntegerValue( "sgame_cheat_GodMode", cheat );
    
    FS_Read( &cheat, sizeof( cheat ), gi.demofile );
    Cvar_SetIntegerValue( "sgame_cheat_InfiniteAmmo", cheat );

    FS_Read( &cheat, sizeof( cheat ), gi.demofile );
    Cvar_SetIntegerValue( "sgame_cheat_InfiniteHealth", cheat );

    FS_Read( &cheat, sizeof( cheat ), gi.demofile );
    Cvar_SetIntegerValue( "sgame_cheat_InfiniteRage", cheat );

    Con_Printf( "--------------------------------\n" );
    Con_Printf( "Demo Playback Begun\n" );
    Con_Printf( "--------------------------------\n" );
    Con_Printf( "version: v%hu.%hu.%u\n", version.m_nVersionMajor, version.m_nVersionUpdate, version.m_nVersionPatch );
    Con_Printf( "referenced bffs: %s\n", referencedBffs );
    Con_Printf( "difficulty: %i\n", difficulty );
    Con_Printf( "map: %s\n", mapname );
    Con_Printf( "--------------------------------\n" );

    Z_Free( referencedBffs );
    for ( i = 0; i < numLines; i++ ) {
        Z_Free( commandLine[i] );
    }
    Z_Free( commandLine );
}

/*
====================
G_Record_f

record <demoname>

Begins recording a demo from the current position
====================
*/
static void G_Record_f( void ) {
	char		demoName[MAX_OSPATH];
	char		name[MAX_OSPATH];
	char		demoExt[16];
	const char	*ext;
	qtime_t		t;

	if ( Cmd_Argc() > 2 ) {
		Con_Printf( "record <demoname>\n" );
		return;
	}

	if ( gi.demorecording ) {
		if ( !gi.spDemoRecording ) {
			Con_Printf( "Already recording.\n" );
		}
		return;
	}

	if ( gi.state != GS_LEVEL ) {
		Con_Printf( "You must be in a level to record.\n" );
		return;
	}

	if ( Cmd_Argc() == 2 ) {
		// explicit demo name specified
		N_strncpyz( demoName, Cmd_Argv( 1 ), sizeof( demoName ) );
		ext = COM_GetExtension( demoName );
		if ( *ext ) {
			// strip demo extension
			sprintf( demoExt, "%s", DEMOEXT );
			if ( N_stricmp( ext, demoExt ) == 0 ) {
				*( strrchr( demoName, '.' ) ) = '\0';
			} else {
				// check both protocols
				sprintf( demoExt, "%s", DEMOEXT );
				if ( N_stricmp( ext, demoExt ) == 0 ) {
					*( strrchr( demoName, '.' ) ) = '\0';
				}
			}
		}
		Com_snprintf( name, sizeof( name ) - 1, "demos/%s", demoName );

		gi.explicitRecordName = qtrue;
	} else {
		Com_RealTime( &t );
		Com_snprintf( name, sizeof( name ) - 1, "demos/demo-%04d%02d%02d-%02d%02d%02d",
			1900 + t.tm_year, 1 + t.tm_mon,	t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec );

		gi.explicitRecordName = qfalse;
	}

	// save desired filename without extension
	N_strncpyz( gi.recordName, name, sizeof( gi.recordName ) );

	Con_Printf( "recording to %s.\n", name );

	// start new record with temporary extension
	N_strcat( name, sizeof( name ), ".tmp" );

	// open the demo file
	gi.recordfile = FS_FOpenWrite( name );
	if ( gi.recordfile == FS_INVALID_HANDLE ) {
		Con_Printf( COLOR_RED "ERROR: couldn't open.\n" );
		gi.recordName[0] = '\0';
		return;
	}

	gi.demorecording = qtrue;

	Com_TruncateLongString( gi.recordNameShort, gi.recordName );

	if ( Cvar_VariableInteger( "ui_recordSPDemo" ) ) {
        gi.spDemoRecording = qtrue;
	} else {
	    gi.spDemoRecording = qfalse;
	}

	// don't start saving messages until a non-delta compressed message is received
	gi.demowaiting = qtrue;

	// write out initial data
    G_BeginRecord();
}


/*
====================
G_CompleteRecordName
====================
*/
static void G_CompleteRecordName( const char *args, uint32_t argNum )
{
	if ( argNum == 2 ) {
		char demoExt[ 16 ];

		Com_snprintf( demoExt, sizeof( demoExt ) - 1, ".%s", DEMOEXT );
		Field_CompleteFilename( "demos", demoExt, qtrue, FS_MATCH_EXTERN );
	}
}

/*
=======================================================================

GAME DEMO PLAYBACK

=======================================================================
*/


/*
==================
G_NextDemo

Called when a demo or cinematic finishes
If the "nextdemo" cvar is set, that command will be issued
==================
*/
static void G_NextDemo( void ) {
	char v[ MAX_CVAR_VALUE ];

	Cvar_VariableStringBuffer( "nextdemo", v, sizeof( v ) );
	Con_DPrintf( "G_NextDemo: %s\n", v );
	if ( !v[0] ) {
		return;
	}

	Cvar_Set( "nextdemo", "" );
	Cbuf_AddText( v );
	Cbuf_AddText( "\n" );
	Cbuf_Execute();
}


/*
=================
G_DemoCompleted
=================
*/
static void G_DemoCompleted( void ) {
	if ( com_timedemo->i ) {
		int	time;

		time = Sys_Milliseconds() - gi.timeDemoStart;
		if ( time > 0 ) {
			Con_Printf( "%i frames, %3.*f seconds: %3.1f fps\n", gi.timeDemoFrames,
			time > 10000 ? 1 : 2, time/1000.0, gi.timeDemoFrames*1000.0 / time );
		}
	}

	G_NextDemo();
}

/*
====================
G_WalkDemoExt
====================
*/
static int G_WalkDemoExt( const char *arg, char *name, int name_len, fileHandle_t *handle )
{
	*handle = FS_INVALID_HANDLE;

	Com_snprintf( name, name_len, "demos/%s.%s", arg, DEMOEXT );
	FS_FOpenFileRead( name, handle );
	if ( *handle != FS_INVALID_HANDLE ) {
		Con_Printf( "Demo file: %s\n", name );
        return 1;
	}
	else {
		Con_Printf( "Not found: %s\n", name );
    }
	return -1;
}


/*
====================
CL_DemoExtCallback
====================
*/
static qboolean G_DemoNameCallback_f( const char *filename, int length )
{
	const int ext_len = strlen( "." DEMOEXT );
	const int num_len = 2;
	int version;

	if ( length <= ext_len + num_len || N_stricmpn( filename + length - ( ext_len + num_len ), "." DEMOEXT, ext_len ) != 0 ) {
		return qfalse;
    }

/*
	version = atoi( filename + length - num_len );
	if ( version == com_protocol->integer )
		return qtrue;

	if ( version < 66 || version > NEW_PROTOCOL_VERSION )
		return qfalse;
    */

	return qtrue;
}


/*
====================
G_CompleteDemoName
====================
*/
static void G_CompleteDemoName( const char *args, uint32_t argNum )
{
	if ( argNum == 2 ) {
		FS_SetFilenameCallback( G_DemoNameCallback_f );
		Field_CompleteFilename( "demos", "." DEMOEXT, qfalse, FS_MATCH_ANY );
		FS_SetFilenameCallback( NULL );
	}
}

/*
====================
G_StopRecording_f

stop recording a demo
====================
*/
void G_StopRecord_f( void ) {

	if ( gi.recordfile != FS_INVALID_HANDLE ) {
		char tempName[MAX_OSPATH];
		char finalName[MAX_OSPATH];
		int	len, sequence;

        FS_FClose( gi.recordfile );
		gi.recordfile = FS_INVALID_HANDLE;

		Com_snprintf( tempName, sizeof( tempName ) - 1, "%s.tmp", gi.recordName );
		Com_snprintf( finalName, sizeof( finalName ) - 1, "%s.%s", gi.recordName, DEMOEXT );

		if ( gi.explicitRecordName ) {
			FS_Remove( finalName );
		} else {
			// add sequence suffix to avoid overwrite
			sequence = 0;
			while ( FS_FileExists( finalName ) && ++sequence < 1000 ) {
				Com_snprintf( finalName, sizeof( finalName ) - 1, "%s-%02d.%s",
					gi.recordName, sequence, DEMOEXT );
			}
		}

		FS_Rename( tempName, finalName );
	}

	if ( !gi.demorecording ) {
		Con_Printf( "Not recording a demo.\n" );
	} else {
		Con_Printf( "Stopped demo recording.\n" );
	}

	gi.demorecording = qfalse;
	gi.spDemoRecording = qfalse;
}

static void G_WriteGamestate( void )
{
    FS_Write( glm::value_ptr( gi.cameraPos ), sizeof( gi.cameraPos ), gi.recordfile );
    FS_Write( &gi.cameraZoom, sizeof( gi.cameraZoom ), gi.recordfile );
    FS_Write( &gi.demoNumEvents, sizeof( gi.demoNumEvents ), gi.recordfile );
}

void G_RecordEvent( const sysEvent_t *ev )
{
    FS_Write( &ev->evTime, sizeof( ev->evTime ), gi.recordfile );
    FS_Write( &ev->evType, sizeof( ev->evType ), gi.recordfile );
    FS_Write( &ev->evValue, sizeof( ev->evValue ), gi.recordfile );
    FS_Write( &ev->evValue2, sizeof( ev->evValue2 ), gi.recordfile );
    FS_Write( &ev->evPtrLength, sizeof( ev->evPtrLength ), gi.recordfile );

    if ( ev->evPtrLength > 0 ) {
        FS_Write( ev->evPtr, ev->evPtrLength, gi.recordfile );
    }

    gi.demoNumEvents++;
}

static void G_ReadDemoEvents( void )
{
    sysEvent_t ev;
    int i;

    for ( i = 0; i < gi.demoNumEvents; i++ ) {
        FS_Read( &ev.evTime, sizeof( ev.evTime ), gi.demofile );
        FS_Read( &ev.evType, sizeof( ev.evType ), gi.demofile );
        FS_Read( &ev.evValue, sizeof( ev.evValue ), gi.demofile );
        FS_Read( &ev.evValue2, sizeof( ev.evValue2 ), gi.demofile );
        FS_Read( &ev.evPtrLength, sizeof( ev.evPtrLength ), gi.demofile );

        ev.evPtr = NULL;
        if ( ev.evPtrLength ) {
            ev.evPtr = Z_Malloc( ev.evPtrLength, TAG_STATIC );
            FS_Read( ev.evPtr, ev.evPtrLength, gi.demofile );
        }

        Com_QueueEvent( ev.evTime, ev.evType, ev.evValue, ev.evValue2, ev.evPtrLength, ev.evPtr );
    }
}

static void G_ReadGamestate( void )
{
    FS_Read( glm::value_ptr( gi.cameraPos ), sizeof( gi.cameraPos ), gi.demofile );
    FS_Read( &gi.cameraZoom, sizeof( gi.cameraZoom ), gi.demofile );
    FS_Read( &gi.demoNumEvents, sizeof( gi.demoNumEvents ), gi.demofile );

    G_ReadDemoEvents();
}

/*
==================
G_PlayDemo_f

demo <demoname>
==================
*/
static void G_PlayDemo_f( void )
{
    char name[MAX_OSPATH];
    const char *arg;
    char *ext_test;
    int protocol, i;
    char retry[MAX_OSPATH];
    const char *shortname, *slash;
    fileHandle_t hFile;

    if ( Cmd_Argc() != 2 ) {
        Con_Printf( "demo <demoname>\n" );
        return;
    }

    // open the demo file
    arg = Cmd_Argv( 1 );

    // check for an extension .DEMOEXT
	ext_test = const_cast<char *>( strrchr( arg, '.' ) );
	if ( ext_test && !N_stricmpn( ext_test + 1, DEMOEXT, arraylen( DEMOEXT ) - 1 ) ) {
        protocol = 1;

		Com_snprintf( name, sizeof( name ) - 1, "demos/%s", arg );
		FS_FOpenFileRead( name, &hFile );
	}
	else {
		protocol = G_WalkDemoExt( arg, name, sizeof( name ), &hFile );
    }

	if ( hFile == FS_INVALID_HANDLE ) {
		Con_Printf( COLOR_YELLOW "couldn't open %s\n", name );
		return;
	}

	FS_FClose( hFile );
	hFile = FS_INVALID_HANDLE;

	if ( FS_FOpenFileRead( name, &gi.demofile ) == -1 ) {
		// drop this time
		N_Error( ERR_DROP, "couldn't open %s\n", name );
		return;
	}

	if ( ( slash = strrchr( name, '/' ) ) != NULL ) {
		shortname = slash + 1;
    } else {
		shortname = name;
    }

	N_strncpyz( gi.demoName, shortname, sizeof( gi.demoName ) );

	Con_Close();
    
    gi.state = GS_LEVEL;
	gi.demoplaying = qtrue;

	// don't get the first snapshot this frame, to prevent the long
	// time from the gamestate load from messing causing a time skip
	gi.firstDemoFrameSkipped = qfalse;

    G_BeginDemoPlay();
}

static void G_Vid_Restart_f( void )
{
    if ( N_stricmp( Cmd_Argv( 1 ), "keep_window" ) == 0 || N_stricmp( Cmd_Argv( 1 ), "fast" ) == 0 ) {
        // fast path: keep window
        G_Vid_Restart( REF_KEEP_WINDOW );
    } else {
        if ( gi.lastVidRestart ) {
            if ( abs( (long)( gi.lastVidRestart - Sys_Milliseconds() ) ) < 500 ) {
                // don't allow vid restart too quickly after a first one
                return;
            }
        }
        G_Vid_Restart( REF_DESTROY_WINDOW );
    }
}

/*
=================
G_Snd_Restart_f

Restart the sound subsystem
The cgame and game must also be forced to restart because
handles will be invalid
=================
*/
static void G_Snd_Restart_f( void )
{
    Snd_Shutdown();

    // sound will be reinitialized by vid restart
    G_Vid_Restart( REF_KEEP_CONTEXT );
}

static void G_VM_Restart_f( void ) {
    G_ShutdownVMs( qfalse );

    G_InitUI();
    G_InitSGame();
}

const vidmode_t r_vidModes[NUMVIDMODES] =
{
    { "Mode  0: 1024x768",      1024,   768,    1 },
    { "Mode  1: 1280x720",      1280,   720,    1 },
    { "Mode  2: 1280x800",      1280,   800,    1 },
    { "Mode  3: 1280x1024",     1280,   1024,   1 },
    { "Mode  4: 1440x900",      1440,   900,    1 },
    { "Mode  5: 1440x960",      1440,   960,    1 },
    { "Mode  6: 1600x900",      1600,   900,    1 },
    { "Mode  7: 1600x1200",     1600,   1200,   1 },
    { "Mode  8: 1600x1050",     1600,   1050,   1 },
    { "Mode  9: 1920x800",      1920,   800,    1 },
    { "Mode 10: 1920x1080",     1920,   1080,   1 },
    { "Mode 11: 1920x1200",     1920,   1200,   1 },
    { "Mode 12: 1920x1280",     1920,   1280,   1 },
    { "Mode 13: 2560x1080",     2560,   1080,   1 },
    { "Mode 14: 2560x1440",     2560,   1440,   1 },
    { "Mode 15: 2560x1600",     2560,   1600,   1 },
    { "Mode 16: 2880x1620",     2880,   1620,   1 },
    { "Mode 17: 3200x1800",     3200,   1800,   1 },
    { "Mode 18: 3840x1600",     3840,   1600,   1 },
    { "Mode 19: 3840x2160",     3840,   2160,   1 }
};
static const int64_t numVidModes = (int64_t)arraylen( r_vidModes );

qboolean G_GetModeInfo( int *width, int *height, float *windowAspect, int mode, const char *modeFS, int dw, int dh, qboolean fullscreen )
{
	const vidmode_t *vm;
	float pixelAspect;

	// set dedicated fullscreen mode
	if ( fullscreen && *modeFS )
		mode = atoi( modeFS );

	if ( mode < -2 )
		return qfalse;

	if ( mode >= numVidModes )
		return qfalse;

	// fix unknown desktop resolution
	if ( mode == -2 && (dw == 0 || dh == 0) )
		mode = 3;

	if ( mode == -2 ) { // desktop resolution
		*width = dw;
		*height = dh;

        // set the width & height for aspect ratios
        Cvar_Set("r_customWidth", va("%i", dw));
        Cvar_Set("r_customHeight", va("%i", dh));

		pixelAspect = r_customPixelAspect->f;
	} else if ( mode == -1 ) { // custom resolution
		*width = r_customWidth->i;
		*height = r_customHeight->i;
		pixelAspect = r_customPixelAspect->f;
	} else { // predefined resolution
		vm = &r_vidModes[ mode ];
		*width  = vm->width;
		*height = vm->height;
		pixelAspect = vm->pixelAspect;
	}

	*windowAspect = (float)*width / ( *height * pixelAspect );

	return qtrue;
}

static void G_ModeList_f( void )
{
	uint32_t i;

	Con_Printf( "\n" );
	for ( i = 0; i < numVidModes; i++) {
		Con_Printf( "%s\n", r_vidModes[ i ].description );
	}
	Con_Printf( "\n" );
}

static qboolean isValidRenderer( const char *s )
{
	while ( *s ) {
		if ( !((*s >= 'a' && *s <= 'z') || (*s >= 'A' && *s <= 'Z') || (*s >= '1' && *s <= '9')) )
			return qfalse;
		++s;
	}
	return qtrue;
}

static void G_TogglePhotoMode_f( void ) {
    gi.togglePhotomode = !gi.togglePhotomode;
    Cvar_Set2( "r_debugCamera", va( "%i", gi.togglePhotomode ), qtrue );
    Con_Printf( "Toggle Photo Mode: %s\n", gi.togglePhotomode ? "on" : "off" );
}

static void G_LogGamestate_f( void ) {
    const char *state;
    switch ( gi.state ) {
    case GS_LEVEL:
        state = "GS_LEVEL";
        break;
    case GS_INACTIVE:
        state = "GS_INACTIVE";
        break;
    case GS_MEMORY_VIEW:
        state = "GS_MEMORY_VIEW";
        break;
    case GS_PAUSE:
        state = "GS_PAUSE";
        break;
    case GS_MENU:
        state = "GS_MENU";
        break;
    case GS_SETTINGS:
        state = "GS_SETTINGS";
        break;
    };

    Con_Printf( "Current Gamestate: %s\n", state );
}

extern void G_MapInfo_f( void );
extern void G_SetMap_f( void );

static void G_SetCameraPos_f( void )
{
    vec3_t xyz;
    int i;

    if ( Cmd_Argc() != 4 ) {
        Con_Printf( "usage: setcamerapos <x> <y> <z>\n" );
        return;
    }

    for ( i = 0; i < 3; i++ ) {
        xyz[i] = N_atof( Cmd_Argv( i + 1 ) );
        if ( N_isnan( xyz[i] ) ) {
            Con_Printf( COLOR_YELLOW "WARNING: a parameter you gave has a NaN component\n" );
            return;
        }
    }
    Con_Printf( "Setting camera origin to [ %.3f, %.3f, %.3f ]\n", xyz[0], xyz[1], xyz[2] );

    VectorCopy( gi.cameraPos, xyz );
}

/*
===============
G_GenerateGameKey

test to see if a valid GAMEKEY_FILE exists. If one does not, try to generate
it by filling it with 2048 bytes of random data.
===============
*/
#ifdef USE_GAMEKEY
static void G_GenerateGameKey( void )
{
	int len = 0;
	unsigned char buf[ GAMEKEY_SIZE ];
	fileHandle_t f;

	len = FS_FOpenFileRead( GAMEKEY_FILE, &f );
	FS_FClose( f );
	if( len == GAMEKEY_SIZE ) {
		Con_Printf( "GAMEKEY found.\n" );
		return;
	}
	else {
		if ( len > 0 ) {
			Con_Printf( "GAMEKEY file size != %i, regenerating\n", GAMEKEY_SIZE );
		}

		Con_Printf( "GAMEKEY building random string\n" );
		Com_RandomBytes( buf, sizeof( buf ) );

		f = FS_FOpenWrite( GAMEKEY_FILE );
		if( !f ) {
			Con_Printf( "GAMEKEY could not open %s for write\n", GAMEKEY_FILE );
			return;
		}
		FS_Write( buf, sizeof( buf ), f );
		FS_FClose( f );
		Con_Printf( "GAMEKEY generated\n" );
	}
}
#endif

/*
====================
G_UpdateGUID

update g_guid using GAMEKEY_FILE and optional prefix
====================
*/
static void G_UpdateGUID( const char *prefix, int prefix_len )
{
#ifdef USE_GAMEKEY
	fileHandle_t f;
	int len;

	len = FS_FOpenFileRead( GAMEKEY_FILE, &f );
	FS_FClose( f );

	if ( len != GAMEKEY_SIZE ) {
		Cvar_Set( "g_guid", "" );
    } else {
		Cvar_Set( "g_guid", Com_MD5File( GAMEKEY_FILE, GAMEKEY_SIZE, prefix, prefix_len ) );
    }
#else
	Cvar_Set( "g_guid", Com_MD5Buf( &cl_cdkey[0], sizeof(cl_cdkey), prefix, prefix_len));
#endif
}

#define MAX_DESCRIPTION_LENGTH 1024
#define MAX_DISPLAY_NAME_LENGTH 128
#define MAX_ID_LENGTH 24

typedef struct skin_s {
    char *description;
    char *displayText;
    char *name;
    struct skin_s *next;
} skin_t;

static skin_t *s_pSkinList;

static void G_LoadSkins( void )
{
    char *b;
    uint64_t nLength;
    const char **text;
    const char *tok, *text_p;
    char name[MAX_ID_LENGTH];
    char display[MAX_DISPLAY_NAME_LENGTH];
    char description[MAX_DESCRIPTION_LENGTH];
    uint32_t numSkins, size;
    skin_t *skin;

    Con_Printf( "Loading skins configuration...\n" );

    nLength = FS_LoadFile( LOG_DIR "/skins.cfg", (void **)&b );
    if ( !nLength || !b ) {
        Con_Printf( COLOR_YELLOW "Error loading " LOG_DIR "/skins.cfg, using default\n" );
        Cvar_Set( "skin", "raio" );
        return;
    }

    text_p = b;
    text = (const char **)&text_p;
    numSkins = 0;

    COM_BeginParseSession( LOG_DIR "/skins.cfg" );

    while ( 1 ) {
        tok = COM_ParseExt( text, qfalse );
        if ( !tok[0] ) {
            break;
        }
        N_strncpyz( name, tok, sizeof( name ) - 1 );

        tok = COM_ParseExt( text, qfalse );
        if ( !tok[0] ) {
            COM_ParseError( "missing parameter for 'displayName' in skin definition" );
            break;
        }
        N_strncpyz( display, tok, sizeof( display ) - 1 );

        tok = COM_ParseExt( text, qfalse );
        if ( !tok[0] ) {
            COM_ParseError( "missing parameter for 'description' in skin definition" );
            break;
        }
        N_strncpyz( description, tok, sizeof( description ) - 1 );

        size = sizeof( *skin );
        size += PAD( strlen( name ) + 1, sizeof( uintptr_t ) );
        size += PAD( strlen( display ) + 1, sizeof( uintptr_t ) );
        size += PAD( strlen( description ) + 1, sizeof( uintptr_t ) );

        skin = (skin_t *)Hunk_Alloc( size, h_low );
        skin->name = (char *)( skin + 1 );
        skin->displayText = (char *)( skin->name + strlen( name ) + 1 );
        skin->description = (char *)( skin->displayText + strlen( display ) + 1 );
        
        strcpy( skin->name, name );
        strcpy( skin->displayText, display );
        strcpy( skin->description, description );
        skin->next = s_pSkinList;
        s_pSkinList = skin;

        Con_Printf( "- Loaded skin \"%s\"\n", skin->name );

        numSkins++;
    }

    FS_FreeFile( b );

    Con_Printf( "Loaded %u skins.\n", numSkins );
}

static void G_SaveSkins( void )
{
    skin_t *skin;
    fileHandle_t fh;

    Con_Printf( "Saving skins configuration...\n" );

    fh = FS_FOpenWrite( LOG_DIR "/skins.cfg" );
    if ( fh == FS_INVALID_HANDLE ) {
        Con_Printf( COLOR_RED "Error saving skins config\n" );
        return;
    }

    for ( skin = s_pSkinList; skin; skin = skin->next ) {
        FS_Printf( fh, "\"%s\" \"%s\" \"%s\"" GDR_NEWLINE, skin->name, skin->displayText, skin->description );
    }

    FS_FClose( fh );
}

skin_t *G_FindSkin( const char *name )
{
    skin_t *skin;
    
    skin = NULL;
    for ( skin = s_pSkinList; skin; skin = skin->next ) {
        if ( !N_stricmp( skin->name, name ) ) {
            return skin;
        }
    }

    return NULL;
}

static void G_ListSkins_f( void )
{
    skin_t *s;

    Con_Printf( "\n" );
    Con_Printf( "-----------------------------\n" );
    Con_Printf( "Skins:\n" );
    for ( s = s_pSkinList; s; s = s->next ) {
        Con_Printf(
            "  ID: \"%s\"\n"
            "  Name: \"%s\"\n"
            "  Description: \"%s\"\n"
        , s->name, s->displayText, s->description );
    }
    Con_Printf( "-----------------------------\n" );
}

static void G_SetSkin_f( void )
{
    const char *skin;
    skin_t *s;

    if ( Cmd_Argc() != 2 ) {
        Con_Printf( "usage: setskin <skin name>\n" );
        return;
    }

    skin = Cmd_Argv( 1 );

    s = G_FindSkin( skin );

    if ( s == NULL ) {
        Con_Printf( "couldn't find skin '%s'.\n", skin );
        return;
    }

    Cvar_Set( "skin", s->name );
}

static void G_InitRenderer_Cvars( void )
{
    cvar_t *temp;

    r_allowLegacy = Cvar_Get( "r_allowLegacy", "0", CVAR_SAVE | CVAR_LATCH );
    Cvar_SetDescription( r_allowLegacy, "Allow the use of old OpenGL API versions, requires \\r_drawMode 0 or 1 and \\r_allowShaders 0" );

    r_glDebug = Cvar_Get( "r_glDebug", "1", CVAR_SAVE | CVAR_LATCH );
    Cvar_SetDescription( r_glDebug, "Toggles OpenGL driver debug logging." );

    temp = Cvar_Get( "window_title", WINDOW_TITLE, CVAR_INIT | CVAR_PROTECTED );
    Cvar_SetDescription( temp, "Sets the name of the application's window." );

    r_allowSoftwareGL = Cvar_Get( "r_allowSoftwareGL", "0", CVAR_LATCH );
	Cvar_SetDescription( r_allowSoftwareGL, "Toggle the use of the default software OpenGL driver supplied by the Operating System." );

    r_stereoEnabled = Cvar_Get( "r_stereoEnabled", "0", CVAR_SAVE | CVAR_LATCH );
	Cvar_SetDescription( r_stereoEnabled, "Enable stereo rendering for techniques like shutter glasses." );

	r_swapInterval = Cvar_Get( "r_swapInterval", "1", CVAR_SAVE );
	Cvar_SetDescription( r_swapInterval,
                        "V-blanks to wait before swapping buffers."
                        "\n  0: No V-Sync\n  1: Synced to the monitor's refresh rate.\n -1: Adaptive V-Sync" );
    
	r_displayRefresh = Cvar_Get( "r_displayRefresh", "0", CVAR_TEMP );
	Cvar_CheckRange( r_displayRefresh, "0", "500", CVT_INT );
	Cvar_SetDescription( r_displayRefresh,
                        "Override monitor refresh rate in fullscreen mode:\n"
                        "   0 - use current monitor refresh rate\n"
                        " > 0 - use custom refresh rate" );

	vid_xpos = Cvar_Get( "vid_xpos", "0", CVAR_SAVE );
	Cvar_CheckRange( vid_xpos, NULL, NULL, CVT_INT );
	Cvar_SetDescription( vid_xpos, "Saves/sets window X-coordinate when windowed, requires \\vid_restart." );

	vid_ypos = Cvar_Get( "vid_ypos", "0", CVAR_SAVE );
	Cvar_CheckRange( vid_ypos, NULL, NULL, CVT_INT );
	Cvar_SetDescription( vid_ypos, "Saves/sets window Y-coordinate when windowed, requires \\vid_restart." );

    r_multisampleType = Cvar_Get( "r_multisampleType", "1", CVAR_SAVE );
    Cvar_CheckRange( r_multisampleType, va( "%i", AntiAlias_None ), va( "%i", AntiAlias_FXAA ), CVT_INT );
    Cvar_SetDescription( r_multisampleType,
                            "Sets the anti-aliasing type to the desired:\n"
                            " 0: None\n"
                            " 1: 2x MSAA\n"
                            " 2: 4x MSAA\n"
                            " 3: 8x MSAA\n"
                            " 4: 16x MSAA\n"
                            " 5: 32x MSAA\n"
                            " 6: 2x SSAA\n"
                            " 7: 4x SSAA\n"
                            " 8: TAA\n"
                            " 9: SMAA\n"
                            " 10: FXAA\n"
                            "requires \\vid_restart." );
    r_multisampleAmount = Cvar_Get( "r_multisampleAmount", "2", CVAR_SAVE );
    Cvar_CheckRange( r_multisampleAmount, "0", "32", CVT_INT );

	r_noborder = Cvar_Get( "r_noborder", "0", CVAR_ARCHIVE_ND );
	Cvar_CheckRange( r_noborder, "0", "1", CVT_INT );
	Cvar_SetDescription( r_noborder, "Setting to 1 will remove window borders and title bar in windowed mode, hold ALT to drag & drop it with opened console." );

	r_mode = Cvar_Get( "r_mode", "-2", CVAR_SAVE );
	Cvar_CheckRange( r_mode, "-2", va( "%lu", numVidModes - 1 ), CVT_INT );
	Cvar_SetDescription( r_mode,
                            "Set video mode:\n"
                            "   -2 - use current desktop resolution\n"
                            "   -1 - use \\r_customWidth and \\r_customHeight\n"
                            " 0..N - enter \\modelist for details"
                        );
    
    r_debugCamera = Cvar_Get( "r_debugCamera", "0", CVAR_TEMP | CVAR_PRIVATE );

	r_fullscreen = Cvar_Get( "r_fullscreen", "1", CVAR_SAVE );
    Cvar_CheckRange( r_fullscreen, "0", "1", CVT_INT );
	Cvar_SetDescription( r_fullscreen, "Fullscreen mode. Set to 0 for windowed mode." );

	r_customPixelAspect = Cvar_Get( "r_customPixelAspect", "1", CVAR_ARCHIVE_ND | CVAR_LATCH );
	Cvar_SetDescription( r_customPixelAspect, "Enables custom aspect of the screen, with \\r_mode 1." );

	r_customWidth = Cvar_Get( "r_customWidth", "1980", CVAR_SAVE );
	Cvar_CheckRange( r_customWidth, "4", NULL, CVT_INT );
	Cvar_SetDescription( r_customWidth, "Custom width to use with \\r_mode -1." );

	r_customHeight = Cvar_Get( "r_customHeight", "1080", CVAR_SAVE );
	Cvar_CheckRange( r_customHeight, "4", NULL, CVT_INT );
	Cvar_SetDescription( r_customHeight, "Custom height to use with \\r_mode -1." );

    r_colorBits = Cvar_Get( "r_colorBits", "0", CVAR_ARCHIVE_ND | CVAR_LATCH );
	Cvar_CheckRange( r_colorBits, "0", "32", CVT_INT );
	Cvar_SetDescription( r_colorBits, "Sets color bit depth, set to 0 to use desktop settings." );

	// shared with renderer:
	g_stencilBits = Cvar_Get( "r_stencilBits", "8", CVAR_ARCHIVE_ND | CVAR_LATCH );
	Cvar_CheckRange( g_stencilBits, "0", "8", CVT_INT );
	Cvar_SetDescription( g_stencilBits, "Stencil buffer size, value decreases Z-buffer depth." );
	g_depthBits = Cvar_Get( "r_depthBits", "0", CVAR_ARCHIVE_ND | CVAR_LATCH );
	Cvar_CheckRange( g_depthBits, "0", "32", CVT_INT );
	Cvar_SetDescription( g_depthBits, "Sets precision of Z-buffer." );

    g_drawBuffer = Cvar_Get( "r_drawBuffer", "GL_BACK", CVAR_CHEAT );
	Cvar_SetDescription( g_drawBuffer, "Specifies buffer to draw from: GL_FRONT or GL_BACK." );

    g_paused = Cvar_Get( "g_paused", "1", CVAR_TEMP );
    Cvar_CheckRange( g_paused, "0", "1", CVT_INT );
    Cvar_SetDescription( g_paused, "Set to 1 when in the pause menu." );

    g_renderer = Cvar_Get( "g_renderer", "opengl", CVAR_SAVE | CVAR_LATCH );
    Cvar_SetDescription( g_renderer,
                        "Set your desired renderer, valid options: opengl, vulkan, sdl2, d3d11\n"
                        "NOTICE: Vulkan, SDL2, and DirectX 11 rendering not supported yet... *will be tho soon :)*\n"
                        "requires \\vid_restart when changed"
        );
}

/*
* G_Init: called every time a new level is loaded
*/
void G_Init( void )
{
    PROFILE_FUNCTION();

    SteamApp_Init();

    Con_Printf( "----- Game State Initialization -----\n" );

    // clear the hunk before anything
    Hunk_Clear();

    G_ClearState();

    G_InitRenderer_Cvars();

    G_LoadSkins();
    
    // userinfo
    Cvar_Get( "name", "The Ultimate Lad", CVAR_USERINFO | CVAR_ARCHIVE_ND );
    Cvar_Get( "skin", "raio", CVAR_USERINFO | CVAR_SAVE );
    Cvar_Get( "voice", "0", CVAR_USERINFO | CVAR_ARCHIVE_ND ); // for the future
    
    if ( !isValidRenderer( g_renderer->s ) ) {
        Cvar_ForceReset( "g_renderer" );
    }

    // init sound
    Snd_Init();
    gi.soundStarted = qtrue;

    // init archive handler
    G_InitArchiveHandler();

    // init renderer
    G_InitRenderer();
    gi.rendererStarted = qtrue;

    // init developer console
    Con_Init();

    //
    // register game commands
    //

    Cmd_AddCommand( "gameinfo", G_GameInfo_f );
    Cmd_AddCommand( "demo", G_PlayDemo_f );
    Cmd_SetCommandCompletionFunc( "demo", G_CompleteDemoName );
    Cmd_AddCommand( "record", G_Record_f );
    Cmd_SetCommandCompletionFunc( "record", G_CompleteRecordName );
    Cmd_AddCommand( "stoprecord", G_StopRecord_f );
    Cmd_AddCommand( "vid_restart", G_Vid_Restart_f );
    Cmd_AddCommand( "snd_restart", G_Snd_Restart_f );
    Cmd_AddCommand( "modelist", G_ModeList_f );
    Cmd_AddCommand( "maplist", G_MapInfo_f );
    Cmd_AddCommand( "vm_restart", G_VM_Restart_f );
    Cmd_AddCommand( "togglephotomode", G_TogglePhotoMode_f );
    Cmd_AddCommand( "viewmemory", G_ViewMemory_f );
    Cmd_AddCommand( "gamestate", G_LogGamestate_f );
    Cmd_AddCommand( "setmap", G_SetMap_f );
    Cmd_AddCommand( "mapinfo", G_MapInfo_f );
    Cmd_AddCommand( "setcamerapos", G_SetCameraPos_f );
    Cmd_AddCommand( "referenced_bffs", G_ReferencedBFFList_f );
    Cmd_AddCommand( "opened_bffs", G_OpenedBFFList_f );
    Cmd_AddCommand( "setskin", G_SetSkin_f );
    Cmd_AddCommand( "skinlist", G_ListSkins_f );

#ifdef USE_MD5
    G_GenerateGameKey();
#endif
    Cvar_Get( "g_guid", "", CVAR_USERINFO | CVAR_ROM | CVAR_PROTECTED );

    Con_Printf( "----- Game State Initialization Complete ----\n" );
}

void G_Shutdown( qboolean quit )
{
    static qboolean recursive = qfalse;

    if ( !recursive ) {
        Con_Printf( "----- Game State Shutdown (%s) ----\n", quit ? "quit" : "restart" );
    }

    if ( recursive ) {
        Con_Printf( "WARNING: recursive G_Shutdown\n" );
        return;
    }
    recursive = qtrue;

    // clear and mute all sounds until next registration
    Snd_Shutdown();

    Con_Shutdown();

    G_ShutdownArchiveHandler();

    G_ShutdownVMs( quit );
    G_ShutdownRenderer( quit ? REF_UNLOAD_DLL : REF_DESTROY_WINDOW );

    SteamApp_Shutdown();

    if ( quit ) {
        remove( "nomad.pid" );
        PROFILE_STOP_LISTEN
    }

    Cmd_RemoveCommand( "gameinfo" );
    Cmd_RemoveCommand( "demo" );
    Cmd_RemoveCommand( "record" );
    Cmd_RemoveCommand( "stoprecord" );
    Cmd_RemoveCommand( "vid_restart" );
    Cmd_RemoveCommand( "snd_restart" );
    Cmd_RemoveCommand( "modelist" );
    Cmd_RemoveCommand( "maplist" );
    Cmd_RemoveCommand( "vm_restart" );
    Cmd_RemoveCommand( "togglephotomode" );
    Cmd_RemoveCommand( "viewmemory" );
    Cmd_RemoveCommand( "gamestate" );
    Cmd_RemoveCommand( "setmap" );
    Cmd_RemoveCommand( "mapinfo" );
    Cmd_RemoveCommand( "setcamerapos" );
    Cmd_RemoveCommand( "referenced_bffs" );
    Cmd_RemoveCommand( "opened_bffs" );
    Cmd_RemoveCommand( "setskin" );
    Cmd_RemoveCommand( "skinlist" );

    Key_SetCatcher( 0 );
    Con_Printf( "-------------------------------\n" );
}

void G_InitUI( void ) {
    Key_SetCatcher( KEYCATCH_UI );
    UI_Init();
}

void G_FlushMemory( void ) {
    // shutdown all game state stuff
    G_ShutdownAll();

    G_ClearMem();

    G_StartHunkUsers();
}

void G_ShutdownVMs( qboolean quit ) {
    G_ShutdownSGame();
    if ( g_pModuleLib ) {
        g_pModuleLib->Shutdown( quit );
    }
    G_ShutdownUI();

    gi.uiStarted = qfalse;
    gi.sgameStarted = qfalse;
}

void G_StartHunkUsers( void )
{
    G_InitArchiveHandler();

    // cache all maps
    G_InitMapCache();

    if ( !gi.rendererStarted ) {
        gi.rendererStarted = qtrue;
        G_InitRenderer();
    }
    if ( !gi.soundStarted ) {
        gi.soundStarted = qtrue;
        Snd_Init();
    }
    if ( !g_pModuleLib ) {
        G_InitModuleLib();
    }
    if ( !gi.uiStarted ) {
        gi.uiStarted = qtrue;
        G_InitUI();
    }
    if ( !gi.sgameStarted ) {
        gi.sgameStarted = qtrue;
        G_InitSGame();
    }
}

void G_ShutdownAll( void )
{
    G_ShutdownArchiveHandler();

    // clear and mute all sounds until next registration
    Snd_DisableSounds();

    // shutdown VMs
    G_ShutdownVMs( qfalse );

    Cmd_RemoveCommand( "gameinfo" );
    Cmd_RemoveCommand( "demo" );
    Cmd_RemoveCommand( "record" );
    Cmd_RemoveCommand( "stoprecord" );
    Cmd_RemoveCommand( "vid_restart" );
    Cmd_RemoveCommand( "snd_restart" );
    Cmd_RemoveCommand( "modelist" );
    Cmd_RemoveCommand( "maplist" );
    Cmd_RemoveCommand( "vm_restart" );
    Cmd_RemoveCommand( "togglephotomode" );
    Cmd_RemoveCommand( "viewmemory" );
    Cmd_RemoveCommand( "gamestate" );
    Cmd_RemoveCommand( "setmap" );
    Cmd_RemoveCommand( "mapinfo" );
    Cmd_RemoveCommand( "setcamerapos" );
    Cmd_RemoveCommand( "referenced_bffs" );
    Cmd_RemoveCommand( "opened_bffs" );

    // shutdown the renderer
    if ( re.Shutdown ) {
        if ( !com_errorEntered ) {
            re.Shutdown( REF_KEEP_CONTEXT ); // don't destroy the window or context, kill the buffers tho
        } else {
            G_ShutdownRenderer( REF_DESTROY_WINDOW ); // shutdown renderer & window
        }
    }

    gi.rendererStarted = qfalse;
    gi.soundStarted = qfalse;

    g_world = NULL;
}

/*
* G_ClearState: clears current gamestate
*/
void G_ClearState( void ) {
    memset( &gi, 0, sizeof( gi ) );
}

qboolean G_CheckPaused( void ) {
    if ( g_paused->i || g_paused->modified ) {
        return qtrue;
    }
    return qfalse;
}

/*
* G_Restart: restarts the hunk memory and all the users
*/
void G_Restart( void ) {
    G_Shutdown( qfalse );
    G_Init();
}

/*
* G_ClearMem: clears all the game's hunk memory
*/
void G_ClearMem( void )
{
    Z_FreeTags( TAG_GAME );
    Z_FreeTags( TAG_SAVEFILE );
    Z_FreeTags( TAG_MODULES );

    // if not in a level, clear the whole hunk
    if ( !gi.mapLoaded ) {
        // clear the whole hunk
        Hunk_Clear();
    }
    else {
        // clear all the map data
        Hunk_ClearToMark();
    }
}

static void G_MoveCamera_f( void )
{
    if ( !r_debugCamera->i ) {
        return;
    }

    if ( keys[KEY_W].down || keys[KEY_PAD0_LEFTSTICK_UP].down ) {
        gi.cameraPos[1] -= 0.5f;
    }
    if ( keys[KEY_S].down || keys[KEY_PAD0_LEFTSTICK_DOWN].down ) {
        gi.cameraPos[1] += 0.5f;
    }
    if ( keys[KEY_A].down || keys[KEY_PAD0_LEFTSTICK_LEFT].down ) {
        gi.cameraPos[0] -= 0.5f;
    }
    if ( keys[KEY_D].down || keys[KEY_PAD0_LEFTSTICK_RIGHT].down ) {
        gi.cameraPos[0] += 0.5f;
    }
    if ( keys[KEY_N].down || keys[KEY_PAD0_RIGHTSTICK_DOWN].down ) {
        gi.cameraZoom += 0.005f;
    }
    if ( keys[KEY_M].down || keys[KEY_PAD0_RIGHTSTICK_UP].down ) {
        gi.cameraZoom -= 0.005f;
    }
}

static void G_PhotoMode( void )
{
    if ( !gi.togglePhotomode ) {
        return;
    }

    if ( keys[KEY_W].down || keys[KEY_PAD0_LEFTSTICK_UP].down ) {
        gi.cameraWorldPos[1] -= 0.05f;
    }
    if ( keys[KEY_S].down || keys[KEY_PAD0_LEFTSTICK_DOWN].down ) {
        gi.cameraWorldPos[1] += 0.05f;
    }
    if ( keys[KEY_A].down || keys[KEY_PAD0_LEFTSTICK_LEFT].down ) {
        gi.cameraWorldPos[0] -= 0.05f;
    }
    if ( keys[KEY_D].down || keys[KEY_PAD0_LEFTSTICK_RIGHT].down ) {
        gi.cameraWorldPos[0] += 0.05f;
    }
    if ( keys[KEY_N].down || keys[KEY_PAD0_RIGHTSTICK_UP].down ) {
        gi.cameraZoom += 0.005f;
    }
    if ( keys[KEY_M].down || keys[KEY_PAD0_RIGHTSTICK_DOWN].bound ) {
        gi.cameraZoom -= 0.005f;
    }
}

/*
static void G_WriteCommands( void )
{
    int i, j;
    int count;
    const usercmd_t *cmd;

    count = gi.cmdNumber - gi.oldCmdNumber;
    for ( i = 0; i < count; i++ ) {
        j = ( gi.cmdNumber - count + i + 1 ) & CMD_MASK;
        cmd = &gi.cmds[j];
        g_pModuleLib->ModuleCall( sgvm, ModuleOnPlayerInput, 3, cmd->forwardmove, cmd->sidemove, cmd->upmove );
    }
    gi.oldCmdNumber = gi.cmdNumber;
}
*/

void G_Frame( int msec, int realMsec )
{
    uint32_t i, j;

    if ( gi.state == GS_LEVEL ) {
        if ( gi.demorecording && gi.recordfile != FS_INVALID_HANDLE ) {
            G_WriteGamestate();
        } else if ( gi.demoplaying && gi.demofile != FS_INVALID_HANDLE ) {
            G_ReadGamestate();
        }
    }

    // save the msec before checking pause
    gi.realFrameTime = realMsec;

    // decide the simulation time
    gi.frametime = msec;
    gi.realtime += msec;

    G_MoveCamera_f();
    G_PhotoMode();

    SteamApp_Frame();

    // update the screen
    gi.framecount++;
    SCR_UpdateScreen();

    // update audio
    Snd_Update( realMsec );

    // run the console
    Con_RunConsole();
}