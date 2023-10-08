#include "../engine/n_shared.h"
#include "../engine/n_sound.h"
#include "../common/vm.h"
#include "g_game.h"
#include "../sgame/sg_public.h"
#include "../ui/ui_public.h"

vm_t *sgvm;
vm_t *uivm;
renderExport_t re;
gameInfo_t fi;

cvar_t *g_renderer;
static void *renderLib;

static void SG_LoadLevel(const char *name)
{

}

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

static void GDR_ATTRIBUTE((format(printf, 2, 3))) GDR_DECL G_RefPrintf(int level, const char *fmt, ...)
{
    va_list argptr;
    char msg[MAXPRINTMSG];

    va_start(argptr, fmt);
    N_vsnprintf(msg, sizeof(msg), fmt, argptr);
    va_end(argptr);

    switch (level) {
    case PRINT_INFO:
        Con_Printf("%s", msg);
        break;
    case PRINT_DEVELOPER:
        Con_DPrintf("%s", msg);
        break;
    };
}

static void *G_RefMalloc(uint32_t size) {
    return Z_Malloc(size, TAG_RENDERER);
}

static char *G_RefStrdup(const char *str) {
    return Z_Strdup(str, TAG_RENDERER);
}

static void G_RefFreeAll(void) {
    Z_FreeTags(TAG_RENDERER, TAG_RENDERER);
}

static void G_InitRenderRef(void)
{
    refimport_t import;
    renderExport_t *ret;
    GetRenderAPI_t GetRenderAPI;
    char dllName[MAX_OSPATH];
    const char *dllPrefix;

    Con_Printf( "----- Initializing Renderer ----\n" );

    Con_Printf("Getting rendering API...\n");
    if (!N_stricmp(g_renderer->s, "OpenGL")) {
        Con_Printf("using OpenGL rendering\n");
        dllPrefix = "rendergl";
    }
    else if (!N_stricmp(g_renderer->s, "Vulkan")) {
        N_Error(ERR_FATAL, "Vulkan rendering not available yet, will be tho in the future... ;)");
        dllPrefix = "rendervk"; // dead code for now...
    }
#if defined (__linux__) && defined(__i386__)
#define REND_ARCH_STRING "x86"
#else
#define REND_ARCH_STRING ARCH_STRING
#endif

    snprintf(dllName, sizeof(dllName), "%s_" REND_ARCH_STRING DLL_EXT, dllPrefix);
    renderLib = FS_LoadLibrary(dllName);
    if (!renderLib) {
        Cvar_ForceReset("g_renderer");
        snprintf(dllName, sizeof(dllName), "%s_" REND_ARCH_STRING DLL_EXT, dllPrefix);
        renderLib = FS_LoadLibrary(dllName);
        if (!renderLib) {
            N_Error(ERR_FATAL, "Failed to load rendering library '%s'", dllName);
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
    import.Free = Z_Free;
    import.FreeAll = G_RefFreeAll;

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

    // most of this stuff is for imgui's usage
    import.SDL_SetHint = SDL_SetHint;
    import.SDL_GetKeyboardFocus = SDL_GetKeyboardFocus;
    import.SDL_GameControllerGetButton = SDL_GameControllerGetButton;
    import.SDL_GameControllerGetAxis = SDL_GameControllerGetAxis;
    import.SDL_GameControllerOpen = SDL_GameControllerOpen;
    import.SDL_GetClipboardText = SDL_GetClipboardText;
    import.SDL_GetCurrentVideoDriver = SDL_GetCurrentVideoDriver;
    import.SDL_CreateSystemCursor = SDL_CreateSystemCursor;
    import.SDL_GetWindowWMInfo = SDL_GetWindowWMInfo;
    import.SDL_GetWindowFlags = SDL_GetWindowFlags;
    import.SDL_GetGlobalMouseState = SDL_GetGlobalMouseState;
    import.SDL_GetTicks64 = SDL_GetTicks64;
    import.SDL_GetPerformanceCounter = SDL_GetPerformanceCounter;
    import.SDL_GetPerformanceFrequency = SDL_GetPerformanceFrequency;
    import.SDL_CreateWindow = SDL_CreateWindow;
    import.SDL_GL_CreateContext = SDL_GL_CreateContext;
    import.SDL_GetError = SDL_GetError;
    import.SDL_ShowCursor = SDL_ShowCursor;
    import.SDL_GetRendererOutputSize = SDL_GetRendererOutputSize;
    import.SDL_Init = SDL_Init;
    import.SDL_SetClipboardText = SDL_SetClipboardText;
    import.SDL_GL_SetAttribute = SDL_GL_SetAttribute;
    import.SDL_GL_MakeCurrent = SDL_GL_MakeCurrent;
    import.SDL_GL_SetSwapInterval = SDL_GL_SetSwapInterval;
    import.SDL_CaptureMouse = SDL_CaptureMouse;
    import.SDL_FreeCursor = SDL_FreeCursor;
    import.SDL_SetCursor = SDL_SetCursor;
    import.SDL_GetWindowPosition = SDL_GetWindowPosition;
    import.SDL_GetWindowSize = SDL_GetWindowSize;
    import.SDL_WarpMouseInWindow = SDL_WarpMouseInWindow;
    import.SDL_SetTextInputRect = SDL_SetTextInputRect;
    import.SDL_GL_GetDrawableSize = SDL_GL_GetDrawableSize;
    import.SDL_DestroyWindow = SDL_DestroyWindow;
    import.SDL_GL_SwapWindow = SDL_GL_SwapWindow;
    import.SDL_Quit = SDL_Quit;
    import.SDL_GL_GetProcAddress = SDL_GL_GetProcAddress;
    import.SDL_GL_DeleteContext = SDL_GL_DeleteContext;

    ret = GetRenderAPI(NOMAD_VERSION_FULL, &import);

    Con_Printf( "-------------------------------\n");
	if ( !ret ) {
		N_Error (ERR_FATAL, "Couldn't initialize refresh" );
	}

    re = *ret;
}

static void G_InitRenderer(void)
{
    if (!re.BeginRegistration) {
        G_InitRenderRef();
    }

    re.BeginRegistration();
}

static void G_ShutdownRenderer(refShutdownCode_t code)
{
    if (g_renderer->modified) {
        code = REF_UNLOAD_DLL;
    }

    if (code >= REF_DESTROY_WINDW) { // +REF_UNLOAD_DLL
        // shutdown sound system before renderer
		// because it may depend from window handle
		Snd_Shutdown(qtrue);
    }

    if (re.Shutdown) {
        re.Shutdown(code);
    }

    if (renderLib) {
        Sys_CloseDLL(renderLib);
        renderLib = NULL;
    }

    memset(&re, 0, sizeof(re));

    gi.rendererStarted = qfalse;
}

static void G_PlayDemo_f(void)
{

}

static void G_Vid_Restart_f(void)
{

}

static void G_Snd_Restart_f(void)
{

}

void G_Frame(void)
{
    if (Key_GetCatcher() & KEYCATCH_UI) {

    }
}

/*
G_Init: initializes a new level
*/
void G_Init(void)
{
    Con_Printf( "----- Game State Initialization ----\n" );

    // clear the hunk before anything
    Hunk_Clear();

    // init sound
    Snd_Init();
    
    // init rendering engine
    G_InitRenderer();

    // load in the VMs
    G_InitSGame();
    G_InitUI();

    //
    // register system commands
    //

    Cmd_AddCommand("demo", G_PlayDemo_f);
    Cmd_AddCommand("vid_restart", G_Vid_Restart_f);
    Cmd_AddCommand("snd_restart", G_Snd_Restart_f);

    Con_Printf( "----- Game State Initialization Complete ----\n" );
}

void G_Shutdown(qboolean quit)
{
    static qboolean recursive = qfalse;

    Con_Printf("----- Game State Shutdown ----\n");

    if (recursive) {
        Con_Printf("WARNING: recursive G_Shutdown\n");
        return;
    }
    recursive = qtrue;

    // clear and mute all sounds until next registration
    Snd_StopAll();

    G_ShutdownVMs();
    G_ShutdownRenderer(quit ? REF_UNLOAD_DLL : REF_DESTROY_WINDW);

    Cmd_RemoveCommand("demo");
    Cmd_RemoveCommand("vid_restart");
    Cmd_RemoveCommand("snd_restart");

    Key_SetCatcher(0);
    Con_Printf( "-------------------------------\n");
}

void G_FlushMemory(void)
{
    // shutdown all game state stuff
    G_ShutdownAll();
    G_ClearMem();
    G_Init();
}

void G_ShutdownVMs(void)
{
    G_ShutdownUI();
    G_ShutdownSGame();
}

void G_StartHunkUsers(void)
{
    if (!gi.rendererStarted) {
        gi.rendererStarted = qtrue;
        G_InitRenderer();
    }
    if (!gi.soundStarted) {
        gi.soundStarted = qtrue;
        Snd_Init();
    }
    if (!gi.uiStarted) {
        gi.uiStarted = qtrue;
        G_InitUI();
    }
}

void G_ShutdownAll(void)
{
    // clear and mute all sounds until next registration
    Snd_StopAll();

    // shutdown VMs
    G_ShutdownVMs();

    // shutdown the renderer
    if (re.Shutdown) {
        if (!com_errorEntered) {
            G_ShutdownRenderer(REF_DESTROY_WINDW); // shutdown renderer & window
        }
        else {
            re.Shutdown(REF_KEEP_CONTEXT); // don't destroy the window or context, kill the buffers tho
        }
    }

    gi.rendererStarted = qfalse;
    gi.soundStarted = qfalse;
}

void G_ClearState(void)
{
    memset(&gi, 0, sizeof(gi));
}

/*
G_Restart: restarts the hunk memory and all the users
*/
void G_Restart(void)
{
    G_Shutdown();
    G_Init();
}

/*
G_ClearMem: clears all the game's hunk memory
*/
void G_ClearMem(void)
{
    // if not in a level, clear the whole hunk
    if (!gi.mapLoaded) {
        // clear the whole hunk
        Hunk_Clear();
    }
    else {
        // clear all the gamestate data on the hunk
        Hunk_ClearToMark();
    }
}

void G_Frame(uint64_t msec, uint64_t realMsec)
{
    gi.frametime = msec;
    gi.realtime += msec;

    // update audio
    Snd_Submit();

    // run a frame for each vm
    VM_Call(uivm, 0, UI_FINISH_FRAME);
    VM_Call(sgvm, 0, SGAME_FINISH_FRAME);

    Con_DrawConsole();
}

