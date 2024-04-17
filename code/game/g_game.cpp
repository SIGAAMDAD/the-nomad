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

CModuleLib *g_pModuleLib;
CModuleInfo *sgvm;
renderExport_t re;
gameInfo_t gi;

// only one gl context is allowed at a time
static void *r_GLcontext = NULL;
static SDL_Window *r_window = NULL;
static SDL_Renderer *r_context = NULL;

static cvar_t *cl_title;
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
cvar_t *r_stereoEnabled;
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
        Con_Printf( "%s", msg);
        break;
    case PRINT_DEVELOPER:
        Con_DPrintf( "%s", msg );
        break;
    case PRINT_WARNING:
        Con_Printf( COLOR_YELLOW "WARNING: %s", msg );
        break;
    case PRINT_ERROR:
        Con_Printf( COLOR_RED "ERROR: %s", msg );
    default:
        N_Error( ERR_FATAL, "G_RefPrintf: Bad print level" );
    };
}

static void *G_RefMalloc( uint64_t size ) {
    return Z_Malloc( size, TAG_RENDERER );
}

static void *G_RefRealloc(void *ptr, uint64_t nsize) {
    return Z_Realloc( ptr, nsize, TAG_RENDERER );
}

static void G_RefFreeAll(void) {
    Z_FreeTags( TAG_RENDERER, TAG_RENDERER );
}

static void G_RefImGuiFree(void *ptr, void *) {
    if ( ptr != NULL ) {
        Z_Free( ptr );
    }
}

static void G_RefImGuiShutdown(void) {
    ImGuiIO& io = ImGui::GetIO();
    io.BackendRendererName = NULL;
    io.BackendRendererUserData = NULL;
    io.BackendLanguageUserData = NULL;
    io.BackendFlags = ImGuiBackendFlags_None;

    FontCache()->ClearCache();

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    // clean everything up
    Z_FreeTags( TAG_IMGUI, TAG_IMGUI );
}

static void G_RefImGuiNewFrame(void) {
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableSetMousePos;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
    io.DisplaySize.x = r_customWidth->i;
    io.DisplaySize.y = r_customHeight->i;
    io.DeltaTime = 1.0 / com_maxfps->i;
    io.Fonts->Flags |= ImFontAtlasFlags_NoBakedLines;

    ImGuiStyle& style = ImGui::GetStyle();

    switch ((antialiasType_t)r_multisampleType->i) {
    case AntiAlias_4xMSAA:
    case AntiAlias_8xMSAA:
        style.AntiAliasedFill = true;
        break;
    case AntiAlias_16xMSAA:
    case AntiAlias_32xMSAA:
        style.AntiAliasedLines = true;
        style.AntiAliasedFill = true;
        break;
    default:
        // its ssaa
        break;
    };

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();
}

static void G_RefImGuiDraw(void) {
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData( ImGui::GetDrawData() );
}

static void G_SetScaling(float factor, uint32_t captureWidth, uint32_t captureHeight)
{
    if (gi.con_factor != factor) {
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
static void G_RefImGuiInit(void *shaderData, const void *importData) {
    IMGUI_CHECKVERSION();
    ImGui::SetAllocatorFunctions( (ImGuiMemAllocFunc)G_RefImGuiMalloc, (ImGuiMemFreeFunc)G_RefImGuiFree );
    ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO();

    io.BackendPlatformName = OS_STRING;

    // init font cache
    g_pFontCache = new ( Hunk_Alloc( sizeof( *g_pFontCache ), h_low ) ) CUIFontCache();

    if ( !N_stricmp( g_renderer->s, "opengl" ) ) {
        ImGui_ImplSDL2_InitForOpenGL( r_window, r_GLcontext );
        ImGui_ImplOpenGL3_Init( shaderData, NULL, (const imguiGL3Import_t *)importData);
    }
    else if ( !N_stricmp( g_renderer->s, "vulkan" ) ) {
        ImGui_ImplSDL2_InitForVulkan( r_window );
    }
}

static void GLM_MakeVPM( const vec4_t ortho, float *zoom, float zNear, float zFar, vec3_t origin, mat4_t vpm,
    mat4_t projection, mat4_t view, uint32_t orthoFlags )
{
    glm::mat4 viewProjectionMatrix, projectionMatrix, viewMatrix, transpose;

    projectionMatrix = glm::ortho( ortho[0], ortho[1], ortho[2], ortho[3], zNear, zFar );
    if ( !( orthoFlags & RSF_ORTHO_TYPE_SCREENSPACE ) ) {
        transpose = glm::translate( glm::mat4( 1.0f ), glm::vec3( gi.cameraPos[0], gi.cameraPos[1], 0.0f ) )
                    * glm::scale( glm::mat4( 1.0f ), glm::vec3( gi.cameraZoom ) );
        viewMatrix = glm::inverse( transpose );
        viewProjectionMatrix = projectionMatrix * viewMatrix;
    } else {
        viewMatrix = glm::mat4( 1.0f );
        viewProjectionMatrix = projectionMatrix;
    }

    VectorCopy( origin, gi.cameraPos );
    *zoom = gi.cameraZoom;

    memcpy( &projection[0][0], &projectionMatrix[0][0], sizeof(mat4_t) );
    memcpy( &view[0][0], &viewMatrix[0][0], sizeof(mat4_t) );
    memcpy( &vpm[0][0], &viewProjectionMatrix[0][0], sizeof(mat4_t) );
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

/*
const glm::mat4 model = glm::translate( viewProjectionMatrix, glm::vec3( worldCameraPosition[0], worldCameraPosition[1], 0.0f ) );
const glm::mat4 mvp = viewProjectionMatrix * model;
*/

static void GLM_TransformToGL( const vec3_t world, vec3_t *xyz, float scale, mat4_t vpm )
{
    glm::mat4 viewProjectionMatrix;
    glm::mat4 mvp, model;
    glm::vec4 pos;

    memcpy( &viewProjectionMatrix[0][0], &vpm[0][0], sizeof(mat4_t) );

    model = glm::translate( viewProjectionMatrix, glm::vec3( world[0], world[1], world[2] ) );
//        * glm::scale( viewProjectionMatrix, glm::vec3( scale ) );
    mvp = viewProjectionMatrix * model;

    const glm::vec4 positions[4] = {
        { 1.0f,  1.0f, 0.0f, 1.0f },
        { 1.0f,  0.0f, 0.0f, 1.0f },
        { 0.0f,  0.0f, 0.0f, 1.0f },
        { 0.0f,  1.0f, 0.0f, 1.0f },
    };

    for ( uint32_t i = 0; i < 4; i++ ) {
        pos = mvp * positions[i];
        VectorCopy( xyz[i], pos );
    }
}

static void GLM_TransformToGL( const vec3_t world, vec3_t *xyz, const glm::mat4& vpm )
{
    glm::mat4 viewProjectionMatrix;
    glm::mat4 mvp, model;
    glm::vec4 pos;

    viewProjectionMatrix = vpm;

    model = glm::translate( viewProjectionMatrix, glm::vec3( world[0], world[1], world[2] ) );
    mvp = viewProjectionMatrix * model;

    const glm::vec4 positions[4] = {
        { 1.0f,  1.0f, 0.0f, 1.0f },
        { 1.0f,  0.0f, 0.0f, 1.0f },
        { 0.0f,  0.0f, 0.0f, 1.0f },
        { 0.0f,  1.0f, 0.0f, 1.0f },
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

    snprintf(dllName, sizeof(dllName), DLL_PREFIX "TheNomad.RenderLib-%s." REND_ARCH_STRING DLL_EXT, dllPrefix);
    renderLib = Sys_LoadDLL(dllName);
    if (!renderLib) {
        Cvar_ForceReset( "g_renderer" );
        snprintf(dllName, sizeof(dllName), DLL_PREFIX "TheNomad.RenderLib-%s." REND_ARCH_STRING DLL_EXT, dllPrefix);
        renderLib = Sys_LoadDLL(dllName);
        if (!renderLib) {
            N_Error(ERR_FATAL, "Failed to load rendering library '%s', possible system error: %s", dllName, Sys_GetDLLError());
        }
    }

    GetRenderAPI = (GetRenderAPI_t)Sys_GetProcAddress(renderLib, "GetRenderAPI");
    if (!GetRenderAPI) {
        N_Error(ERR_FATAL, "Can't load symbol GetRenderAPI");
        return;
    }

    g_renderer->modified = qfalse;

    memset(&import, 0, sizeof(import));

    import.Cmd_AddCommand = Cmd_AddCommand;
    import.Cmd_RemoveCommand = Cmd_RemoveCommand;
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
    import.GLimp_EndFrame = GLimp_EndFrame;
    import.GLimp_SetGamma = GLimp_SetGamma;
    import.GLimp_LogComment = GLimp_LogComment;
    import.GLimp_Shutdown = GLimp_Shutdown;
    import.GLimp_Minimize = GLimp_Minimize;
    import.GLimp_HideFullscreenWindow = GLimp_HideFullscreenWindow;
    import.GL_GetProcAddress = GL_GetProcAddress;

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
    import.ImGui_Draw = G_RefImGuiDraw;

    import.Sys_LoadDLL = Sys_LoadDLL;
    import.Sys_CloseDLL = Sys_CloseDLL;
    import.Sys_GetProcAddress = Sys_GetProcAddress;

    ret = GetRenderAPI( NOMAD_VERSION_FULL, &import );

    Con_Printf( "-------------------------------\n" );
	if ( !ret ) {
		N_Error (ERR_FATAL, "Couldn't initialize refresh" );
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

    // for 1024x768 virtualized screen
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

    memset( &re, 0, sizeof(re) );

    gi.rendererStarted = qfalse;
}

static void G_Vid_Restart( refShutdownCode_t code )
{
    PROFILE_FUNCTION();

    // shutdown VMs
    G_ShutdownVMs( qfalse );

    // shutdown the renderer and clear the renderer interface
    G_ShutdownRenderer(code);

    Snd_Shutdown();
    gi.soundStarted = qfalse;

    G_ShutdownArchiveHandler();

    // clear resource references
	FS_ClearBFFReferences( FS_UI_REF | FS_SGAME_REF );

    G_ClearMem();

    // startup all the gamestate memory
    G_StartHunkUsers();

//    G_InitSGame();
//    gi.sgameStarted = qtrue;

    // make sure all sounds have updated volumes
    Cbuf_ExecuteText( EXEC_APPEND, "updatevolume\n" );
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

static void G_PlayDemo_f(void)
{

}

static void G_Vid_Restart_f( void )
{
    if ( N_stricmp( Cmd_Argv( 1 ), "keep_window" ) == 0 ) {
        // fast path: keep window
        G_Vid_Restart( REF_KEEP_WINDOW );
    }
    else if ( N_stricmp( Cmd_Argv( 1 ), "fast" ) == 0 ) {
        // fast path: keep context
        G_Vid_Restart( REF_KEEP_CONTEXT );
    }
    else {
        if ( gi.lastVidRestart ) {
            if ( abs( (long)( gi.lastVidRestart - Sys_Milliseconds() ) ) < 500 ) {
                // don't allow vid restart too quickly after a first one
                return;
            }
        }
        G_Vid_Restart( REF_DESTROY_WINDOW );
    }
}

//
// G_Snd_Restart_f: Restarts the sound subsystem
// The sgame and ui must also be forced to restart
// because the handles will be invalid
//
static void G_Snd_Restart_f( void )
{
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
//	{ "Mode  0: 320x240",			320,	240,	1 },
//	{ "Mode  1: 640x480",			640,	480,	1 },
//	{ "Mode  2: 800x600",			800,	600,	1 },
	{ "Mode  0: 1024x768",			1024,	768,	1 },
	{ "Mode  1: 1280x720",			1280,	720,	1 },
	{ "Mode  2: 1600x900",			1600,	900,	1 },
	{ "Mode  3: 1920x1080",			1920,	1080,	1 },
    { "Mode  4: 2048x1536",			2048,	1536,	1 },
	{ "Mode  5: 3840x2160",			3840,	2160,	1 },
//	{ "Mode  9: 4096x2160 (4K)",	4096,	2160,	1 }
};
static const int64_t numVidModes = (int64_t)arraylen( r_vidModes );

qboolean G_GetModeInfo( int *width, int *height, float *windowAspect, int mode, const char *modeFS, uint32_t dw, uint32_t dh, qboolean fullscreen )
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

//
// G_Init: called every time a new level is loaded
//
void G_Init( void )
{
    PROFILE_FUNCTION();

    Con_Printf( "----- Game State Initialization ----\n" );

    // clear the hunk before anything
    Hunk_Clear();

    G_ClearState();

    Cvar_Get( "g_mouseAcceleration", "0", CVAR_SAVE );
    Cvar_Get( "g_mouseSensitivity", "10.0", CVAR_SAVE );

    r_allowLegacy = Cvar_Get( "r_allowLegacy", "0", CVAR_SAVE | CVAR_LATCH );
    Cvar_SetDescription( r_allowLegacy, "Allow the use of old OpenGL API versions, requires \\r_drawMode 0 or 1 and \\r_allowShaders 0" );

    r_glDebug = Cvar_Get( "r_glDebug", "1", CVAR_SAVE | CVAR_LATCH );
    Cvar_SetDescription( r_glDebug, "Toggles OpenGL driver debug logging." );

    cl_title = Cvar_Get( "g_title", WINDOW_TITLE, CVAR_INIT | CVAR_PROTECTED );
    Cvar_SetDescription( cl_title, "Sets the name of the application's window." );

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
    r_multisampleAmount = Cvar_Get( "r_multisampleAmount", "8", CVAR_SAVE );
    Cvar_CheckRange( r_multisampleAmount, 0, "32", CVT_INT );

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
    Cvar_CheckRange(r_fullscreen, "0", "1", CVT_INT);
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

	Cvar_Get( "vm_sgame", "2", CVAR_SAVE | CVAR_PROTECTED );

    //
    // register game commands
    //

    Cmd_AddCommand( "demo", G_PlayDemo_f );
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
    Snd_StopAll();

    Con_Shutdown();

    G_ShutdownArchiveHandler();

    G_ShutdownVMs( quit );
    G_ShutdownRenderer( quit ? REF_UNLOAD_DLL : REF_DESTROY_WINDOW );

    remove( "nomad.pid" );

    PROFILE_STOP_LISTEN

    Cmd_RemoveCommand( "demo" );
    Cmd_RemoveCommand( "vid_restart" );
    Cmd_RemoveCommand( "snd_restart" );
    Cmd_RemoveCommand( "modelist" );
    Cmd_RemoveCommand( "maplist" );
    Cmd_RemoveCommand( "vm_restart" );
    Cmd_RemoveCommand( "togglephotomode" );
    Cmd_RemoveCommand( "viewmemory" );
    Cmd_RemoveCommand( "gamestate" );
    Cmd_RemoveCommand( "setcamerapos" );

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

    g_pRenderThread = new ( Hunk_Alloc( sizeof( *g_pRenderThread ), h_low ) ) CRenderThread();

    // cache all maps
    G_InitMapCache();

    if ( !g_pModuleLib ) {
        G_InitModuleLib();
    }

    if ( !gi.rendererStarted ) {
        gi.rendererStarted = qtrue;
        G_InitRenderer();
    }
    if ( !gi.soundStarted ) {
        gi.soundStarted = qtrue;
        Snd_Init();
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
    Snd_StopAll();
    Snd_Shutdown();

    // shutdown VMs
    G_ShutdownVMs( qfalse );

    // shutdown the renderer
    if ( re.Shutdown ) {
        if ( !com_errorEntered ) {
            G_ShutdownRenderer( REF_DESTROY_WINDOW ); // shutdown renderer & window
        }
        else {
            re.Shutdown( REF_KEEP_CONTEXT ); // don't destroy the window or context, kill the buffers tho
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
    memset( &gi, 0, sizeof(gi) );
    gi.state = GS_INACTIVE;
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
void G_ClearMem(void)
{
    // clear all game memory
    Z_FreeTags( TAG_GAME, TAG_GAME );

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

    if ( keys[KEY_W].down ) {
        gi.cameraPos[1] += 0.05f;
    }
    if ( keys[KEY_S].down ) {
        gi.cameraPos[1] -= 0.05f;
    }
    if ( keys[KEY_A].down ) {
        gi.cameraPos[0] -= 0.05f;
    }
    if ( keys[KEY_D].down ) {
        gi.cameraPos[0] += 0.05f;
    }
    if ( keys[KEY_N].down ) {
        gi.cameraZoom += 0.005f;
    }
    if ( keys[KEY_M].down ) {
        gi.cameraZoom -= 0.005f;
    }
}

void G_Frame( int msec, int realMsec )
{
    uint32_t i, j;

    // save the msec before checking pause
    gi.realFrameTime = realMsec;
    
    // decide the simulation time
//    gi.frametime = msec;
//    gi.realtime += msec;
    gi.frametime = gi.frametime - msec;
    gi.realtime += gi.frametime;

    G_MoveCamera_f();

    // update the screen
    gi.framecount++;
    SCR_UpdateScreen();

    // update audio
    Snd_Update( realMsec );

    // run the console
    Con_RunConsole();
}

void G_MouseEvent( int dx, int dy ) {
}

/*
===========================================

GLimp functions are in here until I decide
to move them somewhere else

===========================================
*/

typedef struct {
    FILE *logfile;
    qboolean isFullscreen;
    qboolean minimized;
    gpuConfig_t *config;
    uint32_t desktop_width;
    uint32_t desktop_height;
    uint32_t window_width;
    uint32_t window_height;
    uint32_t moniter_count;
} glState_t;

static glState_t glState;

void GLimp_Shutdown( qboolean unloadDLL )
{
    SDL_DestroyWindow( r_window );
    r_window = NULL;

    if ( unloadDLL ) {
        SDL_QuitSubSystem( SDL_INIT_VIDEO );
    }
}

/*
===============
GLimp_Minimize

Minimize the game so that user is back at the desktop
===============
*/
void GLimp_Minimize( void )
{
	SDL_MinimizeWindow( r_window );
}

void GLimp_LogComment( const char *comment )
{
}

SDL_Window *G_GetSDLWindow( void ) {
    return r_window;
}

static int GLimp_CreateBaseWindow( gpuConfig_t *config )
{
    uint32_t windowFlags;
    uint32_t contextFlags;
    int32_t depthBits, stencilBits, colorBits;
    int32_t perChannelColorBits;
    int32_t x, y;
    int32_t i;

    // set window flags
    windowFlags = SDL_WINDOW_OPENGL;
    if ( r_fullscreen->i ) {
        // custom fullscreen or native?
        if ( r_mode->i == -2 ) {
            windowFlags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
        }
        else {
            windowFlags |= SDL_WINDOW_FULLSCREEN;
        }
    }
    if ( r_noborder->i ) {
        windowFlags |= SDL_WINDOW_BORDERLESS;
    }

    // destroy existing context if it exists
    if  ( r_GLcontext ) {
        SDL_GL_DeleteContext( r_GLcontext );
        r_GLcontext = NULL;
    }
    if ( r_window ) {
        SDL_GetWindowPosition( r_window, &x, &y );
        Con_DPrintf( "Existing window at %ix%i before destruction\n", x, y );
        SDL_DestroyWindow( r_window );
        r_window = NULL;
    }

    colorBits = r_colorBits->i;
    if ( colorBits == 0 || colorBits > 32 ) {
        colorBits = 32;
    }
    if ( g_depthBits->i == 0 ) {
        // implicitly assume Z-buffer depth == desktop color depth
        if ( colorBits > 16 ) {
            depthBits = 24;
        } else {
            depthBits = 16;
        }
    }
    else {
        depthBits = g_depthBits->i;
    }

    stencilBits = g_stencilBits->i;

    // do not allow stencil if Z-buffer depth likely won't contain it
    if ( depthBits < 24 ) {
        stencilBits = 0;
    }
    
    for ( i = 0; i < 16; i++ ) {
        int testColorBits, testDepthBits, testStencilBits;
        int realColorBits[3];

		// 0 - default
		// 1 - minus colorBits
		// 2 - minus depthBits
		// 3 - minus stencil
		if ( ( i % 4 ) == 0 && i ) {
			// one pass, reduce
			switch ( i / 4 ) {
			case 2:
				if ( colorBits == 24 ) {
					colorBits = 16;
                }
				break;
			case 1:
				if ( depthBits == 24 ) {
					depthBits = 16;
				} else if ( depthBits == 16 ) {
					depthBits = 8;
                }
			case 3:
				if ( stencilBits == 24 ) {
					stencilBits = 16;
                } else if ( stencilBits == 16 ) {
					stencilBits = 8;
                }
			};
		}

		testColorBits = colorBits;
		testDepthBits = depthBits;
		testStencilBits = stencilBits;

		if ( ( i % 4 ) == 3 ) { // reduce colorBits
			if ( testColorBits == 24 ) {
				testColorBits = 16;
            }
		}

		if ( ( i % 4 ) == 2 ) { // reduce depthBits
			if ( testDepthBits == 24 ) {
				testDepthBits = 16;
            } else if ( testDepthBits == 16 ) {
				testDepthBits = 8;
            }
		}

		if ( ( i % 4 ) == 1 ) { // reduce stencilBits
			if ( testStencilBits == 24 ) {
				testStencilBits = 16;
            } else if ( testStencilBits == 16 ) {
				testStencilBits = 8;
            } else {
				testStencilBits = 0;
            }
		}

		if ( testColorBits == 24 ) {
			perChannelColorBits = 8;
        } else {
			perChannelColorBits = 4;
        }
        
        SDL_GL_SetAttribute( SDL_GL_RED_SIZE, perChannelColorBits );
        SDL_GL_SetAttribute( SDL_GL_GREEN_SIZE, perChannelColorBits );
        SDL_GL_SetAttribute( SDL_GL_BLUE_SIZE, perChannelColorBits );

        SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE, testDepthBits );
        SDL_GL_SetAttribute( SDL_GL_STENCIL_SIZE, testStencilBits );

        SDL_GL_SetAttribute( SDL_GL_MULTISAMPLEBUFFERS, 0 );
        SDL_GL_SetAttribute( SDL_GL_MULTISAMPLESAMPLES, 0 );

        contextFlags = 0;
        if ( r_glDebug->i ) {
            contextFlags |= SDL_GL_CONTEXT_DEBUG_FLAG;
        }
        if ( !r_allowLegacy->i ) {
//            contextFlags |= SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG;
        }

        // set the recommended version, this is not mandatory,
        // however if your driver isn't >= 3.3, that'll be
        // deprecated stuff
        SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, 3 );
        SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, 3 );
        SDL_GL_SetAttribute( SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY );

        if ( contextFlags ) {
            SDL_GL_SetAttribute( SDL_GL_CONTEXT_FLAGS, contextFlags );
        }

        if (r_stereoEnabled->i) {
            SDL_GL_SetAttribute(SDL_GL_STEREO, 1);
        }
        else {
            SDL_GL_SetAttribute(SDL_GL_STEREO, 0);
        }

        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

        if (!r_allowSoftwareGL->i) {
            SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
        }

        // [glnomad] make sure we only create ONE window
        if (!r_window) {
            if ((r_window = SDL_CreateWindow(cl_title->s, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
            config->vidWidth, config->vidHeight, windowFlags)) == NULL) {
                Con_DPrintf("SDL_CreateWindow(%s, %i, %i, %x) failed: %s",
                    cl_title->s, config->vidWidth, config->vidHeight, windowFlags, SDL_GetError());
                return -1;
            }
        }
        if (r_fullscreen->i) {
            SDL_DisplayMode mode;

	    	switch ( testColorBits ) {
	    	case 16: mode.format = SDL_PIXELFORMAT_RGB565; break;
	    	case 24: mode.format = SDL_PIXELFORMAT_RGB24;  break;
            case 32: mode.format = SDL_PIXELFORMAT_RGBA32; break;
	    	default: Con_DPrintf( "testColorBits is %d, can't fullscreen\n", testColorBits ); continue;
	    	};

	    	mode.w = config->vidWidth;
	    	mode.h = config->vidHeight;
	    	mode.refresh_rate = Cvar_VariableInteger( "r_displayRefresh" );
	    	mode.driverdata = NULL;

	    	if ( SDL_SetWindowDisplayMode( r_window, &mode ) < 0 ) {
	    		Con_DPrintf( "SDL_SetWindowDisplayMode failed: %s\n", SDL_GetError( ) );
	    		continue;
	    	}

	    	if ( SDL_GetWindowDisplayMode( r_window, &mode ) >= 0 ) {
	    		config->displayFrequency = mode.refresh_rate;
	    		config->vidWidth = mode.w;
	    		config->vidHeight = mode.h;
	    	}
        }
        if ( !r_GLcontext ) {
			if ( ( r_GLcontext = SDL_GL_CreateContext( r_window ) ) == NULL ) {
				Con_DPrintf( "SDL_GL_CreateContext failed: %s\n", SDL_GetError( ) );
				SDL_DestroyWindow( r_window );
				r_window = NULL;
				continue;
			}
		}
		if ( SDL_GL_SetSwapInterval( r_swapInterval->i ) == -1 ) {
            // NOTE: if you get negative swap isn't supported, that just means dynamic
            // vsync isn't available
			Con_DPrintf( "SDL_GL_SetSwapInterval failed: %s\n", SDL_GetError( ) );
		}
		SDL_GL_GetAttribute( SDL_GL_RED_SIZE, &realColorBits[0] );
		SDL_GL_GetAttribute( SDL_GL_GREEN_SIZE, &realColorBits[1] );
		SDL_GL_GetAttribute( SDL_GL_BLUE_SIZE, &realColorBits[2] );
		SDL_GL_GetAttribute( SDL_GL_DEPTH_SIZE, &config->depthBits );
		SDL_GL_GetAttribute( SDL_GL_STENCIL_SIZE, &config->stencilBits );

		config->colorBits = realColorBits[0] + realColorBits[1] + realColorBits[2];
    }
    Con_Printf( "Using %d color bits, %d depth, %d stencil display.\n",
        config->colorBits, config->depthBits, config->stencilBits );

    if (r_window) {
#ifdef GLNOMAD_ICON_INCLUDE
        SDL_Surface *icon = SDL_CreateRGBSurfaceFrom(
            (void *)WINDOW_ICON.pixel_data,
            WINDOW_ICON.width,
            WINDOW_ICON.height,
            WINDOW_ICON.bytes_per_pixel * 8,
            WINDOW_ICON.bytes_per_pixel * WINDOW_ICON.width
#ifdef GDR_LITTLE_ENDIAN
			0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000
#else
			0xFF000000, 0x00FF0000, 0x0000FF00, 0x000000FF
#endif
        );
        if (icon) {
            SDL_SetWindowIcon(r_window, icon);
            SDL_FreeSurface(icon);
        }
        else {
            Con_DPrintf("SDL_CreateRGBSurfaceFrom(WINDOW_ICON) == NULL\n"); // just to let us know
        }
#endif
    }
    else {
        Con_Printf("Failed video initialization\n");
        return -1;
    }
    SDL_GL_MakeCurrent(r_window, r_GLcontext);
    SDL_GL_GetDrawableSize(r_window, &config->vidWidth, &config->vidHeight);

    return 1;
}

//
// GLimp_Init: will initialize a new OpenGL
// window and context handle, will also handle all
// the cvar stuff
//
void GLimp_Init(gpuConfig_t *config)
{
    uint32_t width, height;
    uint32_t windowFlags;
    float windowAspect;
    SDL_DisplayMode dm;

    Con_Printf("---------- GLimp_Init ----------\n");

    if (!SDL_WasInit(SDL_INIT_VIDEO)) {
        if (SDL_Init(SDL_INIT_VIDEO) != 0) {
            N_Error(ERR_FATAL, "SDL_Init(SDL_INIT_VIDEO) Failed: %s", SDL_GetError());
            return;
        }
    }

    if (SDL_GetDesktopDisplayMode( 0, &dm ) != 0) {
        N_Error( ERR_FATAL, "SDL_GetDesktopDisplayMode failed: %s", SDL_GetError() );
    }

    gi.desktopWidth = dm.w;
    gi.desktopHeight = dm.h;

    const char *driverName;

    if (!GLimp_CreateBaseWindow(config)) {
        N_Error(ERR_FATAL, "Failed to init OpenGL\n");
    }

    driverName = SDL_GetCurrentVideoDriver();

    Con_Printf("SDL using driver \"%s\"\n", driverName);

    // These values force the UI to disable driver selection
	config->driverType = GLDRV_ICD;
	config->hardwareType = GLHW_GENERIC;

    Key_ClearStates();
}

void GLimp_EndFrame(void)
{
    // don't flip if drawing to front buffer
    if (N_stricmp(g_drawBuffer->s, "GL_FRONT") != 0) {
        SDL_GL_SwapWindow(r_window);
    }
}

void *GL_GetProcAddress(const char *name)
{
    return SDL_GL_GetProcAddress(name);
}

void GLimp_HideFullscreenWindow(void)
{
    if (r_window && glState.isFullscreen) {
        SDL_HideWindow(r_window);
    }
}

static Uint16 r[256];
static Uint16 g[256];
static Uint16 b[256];

void GLimp_InitGamma( gpuConfig_t *config )
{
	config->deviceSupportsGamma = qfalse;

	if ( SDL_GetWindowGammaRamp( r_window, r, g, b ) == 0 ) {
		config->deviceSupportsGamma = SDL_SetWindowBrightness( r_window, 1.0f ) >= 0 ? qtrue : qfalse;
	}
}


/*
=================
GLimp_SetGamma
=================
*/
void GLimp_SetGamma( unsigned char red[256], unsigned char green[256], unsigned char blue[256] )
{
	Uint16 table[3][256];
	uint32_t i, j;

	for ( i = 0; i < 256; i++ ) {
		table[0][i] = ( ( ( Uint16 ) red[i] ) << 8 ) | red[i];
		table[1][i] = ( ( ( Uint16 ) green[i] ) << 8 ) | green[i];
		table[2][i] = ( ( ( Uint16 ) blue[i] ) << 8 ) | blue[i];
	}

#ifdef _WIN32
#include <windows.h>

	// Win2K and newer put this odd restriction on gamma ramps...
	{
		//OSVERSIONINFO	vinfo;
		//vinfo.dwOSVersionInfoSize = sizeof( vinfo );
		//GetVersionEx( &vinfo );
		//if( vinfo.dwMajorVersion >= 5 && vinfo.dwPlatformId == VER_PLATFORM_WIN32_NT )
		{
			qboolean clamped = qfalse;
			for ( j = 0 ; j < 3 ; j++ ) {
				for ( i = 0 ; i < 128 ; i++ ) {
					if ( table[ j ] [ i] > ( ( 128 + i ) << 8 ) ) {
						table[ j ][ i ] = ( 128 + i ) << 8;
						clamped = qtrue;
					}
				}

				if ( table[ j ] [127 ] > 254 << 8 ) {
					table[ j ][ 127 ] = 254 << 8;
					clamped = qtrue;
				}
			}
			if ( clamped ) {
				Con_DPrintf( "performing gamma clamp.\n" );
			}
		}
	}
#endif

	// enforce constantly increasing
	for ( j = 0; j < 3; j++ ) {
		for ( i = 1; i < 256; i++) {
			if ( table[j][i] < table[j][i-1] ) {
				table[j][i] = table[j][i-1];
            }
		}
	}

	if ( SDL_SetWindowGammaRamp( r_window, table[0], table[1], table[2] ) < 0 ) {
		Con_DPrintf( "SDL_SetWindowGammaRamp() failed: %s\n", SDL_GetError() );
	}
}


SDL_GLContext G_GetGLContext( void ) {
    return r_GLcontext;
}

qboolean G_WindowMinimized( void ) {
    return (qboolean)(SDL_GetWindowFlags( r_window ) & SDL_WINDOW_MINIMIZED);
}

//
// G_InitDisplay: called during renderer init
//
void G_InitDisplay(gpuConfig_t *config)
{
    SDL_DisplayMode mode;

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        N_Error(ERR_FATAL, "SDL_Init(SDL_INIT_VIDEO) Failed: %s", SDL_GetError());
    }
    if (SDL_GetDesktopDisplayMode(0, &mode) != 0) {
        Con_Printf(COLOR_YELLOW "SDL_GetDesktopDisplayMode() Failed: %s\n", SDL_GetError());
        Con_Printf(COLOR_YELLOW "Setting mode to default of 1920x1080\n");

        mode.refresh_rate = 60;
        mode.w = 1920;
        mode.h = 1080;
    }

    gi.desktopWidth = mode.w;
    gi.desktopHeight = mode.h;

    if ( !G_GetModeInfo( &config->vidWidth, &config->vidHeight, &config->windowAspect, r_mode->i,
        "", mode.w, mode.h, r_fullscreen->i ) )
    {
        Con_Printf("Invalid r_mode, resetting...\n");
        Cvar_ForceReset("r_mode");
        if (!G_GetModeInfo(&config->vidWidth, &config->vidHeight, &config->windowAspect, r_mode->i,
            "", mode.w, mode.h, r_fullscreen->i))
        {
            Con_Printf(COLOR_YELLOW "Could not determine video mode, setting to default of 1920x1080\n");

            config->vidWidth = 1920;
            config->vidHeight = 1080;
            config->windowAspect = 1;
        }
    }

    Con_Printf("Setting up display\n");
    Con_Printf("...setting mode %li\n", r_mode->i);
    
    // init OpenGL
    GLimp_Init( config );
}
