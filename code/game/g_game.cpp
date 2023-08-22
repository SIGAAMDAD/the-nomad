#include "../engine/n_shared.h"
#include "../engine/n_map.h"
#include "../rendergl/rgl_public.h"
#include "../engine/n_sound.h"
#include "../common/vm.h"
#include "g_game.h"

Game* Game::gptr;
vm_t *sgvm;

void Game::Alloc(void)
{
    gptr = (Game *)Hunk_Alloc(sizeof(*gptr), "gptr", h_low);
    construct(gptr);
}

static void SG_LoadLevel(const char *name)
{

}

static void G_ShutdownSGame(void)
{
    Key_SetCatcher(Key_GetCatcher() & ~KEYCATCH_SGAME);

    if (!sgvm) {
        return;
    }

    VM_Call(sgvm, 0, SGAME_SHUTDOWN);
    VM_Free(sgvm);
    sgvm = NULL;
    FS_VM_CloseFiles(H_SGAME);
}

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

static const nmap_t *G_GetCurrentMap(void)
{
    return level->mapData;
}
static void G_FillImport(renderImport_t *import);

/*
G_Init: initializes a new level
*/
void G_Init(void)
{
    // clear the hunk before anything
    Hunk_Clear();

    Game::Alloc();

    // init sound
    Snd_Init();
    I_CacheAudio();
    
    // init rendering engine
    renderImport_t import;
    G_FillImport(&import);
    RE_GetImport(&import);
    RE_Init();

    vm_rtChecks = Cvar_Get("vm_rtChecks", va("%i", VM_RTCHECK_DATA), CVAR_INIT | CVAR_PRIVATE);

    // load in the VMs
    G_InitSGame();
}

void G_Shutdown(void)
{
    // force stop the vm
    VM_Forced_Unload_Start();
    VM_Free(sgvm);
    VM_Forced_Unload_Done();

    // destroy the rendering context
    RE_Shutdown(qtrue);
    
    // destroy the audio device
    Snd_Shutdown(qtrue);
}

/*
G_Restart: restarts the hunk memory and all the users
*/
void G_Restart(void)
{
    // re-init the rendering engine
    RE_Shutdown(qfalse);
    RE_Init();

    // re-init the sound buffers
    Snd_Restart();
}

/*
G_ClearMem: clears all the game's hunk memory
*/
void G_ClearMem(void)
{
    RE_Shutdown(qfalse);
    Snd_Shutdown(qfalse);

    // clear the hunk
    Hunk_Clear();
}

static void *G_Malloc(uint32_t size, void *user, const char *name)
{
    return Z_Malloc(size, TAG_RENDERER, user, name);
}

static void G_Free(void *ptr)
{
    Z_Free(ptr);
}

static void *G_Realloc(void *ptr, uint32_t nsize, void *user, const char *name)
{
    return Z_Realloc(ptr, nsize, TAG_RENDERER, user, name);
}

static void *G_Calloc(uint32_t size, void *user, const char *name)
{
    return Z_Calloc(size, TAG_RENDERER, user, name);
}

static char *G_Strdup(const char *str)
{
    return Z_StrdupTag(str, TAG_RENDERER);
}

/*
G_FillImport: fills render import functions for the dynamic library to use
*/
void G_FillImport(renderImport_t *import)
{
#ifdef _NOMAD_DEBUG
    import->Hunk_AllocDebug = Hunk_AllocDebug;
#else
    import->Hunk_Alloc = Hunk_Alloc;
#endif
    import->Hunk_MemoryRemaining = Hunk_MemoryRemaining;
	import->Hunk_AllocateTempMemory = Hunk_AllocateTempMemory;
	import->Hunk_FreeTempMemory = Hunk_FreeTempMemory;

    import->Malloc = G_Malloc;
    import->Calloc = G_Calloc;
    import->Free = G_Free;
    import->Realloc = G_Realloc;
    import->Strdup = G_Strdup;

	import->Z_FreeTags = Z_FreeTags;
	import->Z_ChangeTag = Z_ChangeTag;
	import->Z_ChangeUser = Z_ChangeUser;
	import->Z_CleanCache = Z_CleanCache;
	import->Z_CheckHeap = Z_CheckHeap;
	import->Z_ClearZone = Z_ClearZone;
    import->Z_FreeMemory = Z_FreeMemory;
    import->Z_NumBlocks = Z_NumBlocks;
	import->Z_BlockSize = Z_BlockSize;

	import->Sys_FreeFileList = Sys_FreeFileList;
	
    import->Mem_Alloc = Mem_Alloc;
    import->Mem_Free = Mem_Free;

	// get the specific logger function (it's been overloaded)
	import->Con_Printf = static_cast<void(*)(loglevel_t, const char *, ...)>(Con_Printf);
    import->N_Error = N_Error;

	import->Cvar_VariableStringBuffer = Cvar_VariableStringBuffer;
	import->Cvar_VariableStringBufferSafe = Cvar_VariableStringBufferSafe;
	import->Cvar_VariableInteger = Cvar_VariableInteger;
	import->Cvar_VariableFloat = Cvar_VariableFloat;
	import->Cvar_VariableBoolean = Cvar_VariableBoolean;
	import->Cvar_VariableString = Cvar_VariableString;
	import->Cvar_Flags = Cvar_Flags;
	import->Cvar_Get = Cvar_Get;
	import->Cvar_SetGroup = Cvar_SetGroup;
	import->Cvar_SetDescription = Cvar_SetDescription;
	import->Cvar_SetSafe = Cvar_SetSafe;
	import->Cvar_SetBooleanValue = Cvar_SetBooleanValue;
	import->Cvar_SetStringValue = Cvar_SetStringValue;
	import->Cvar_SetFloatValue = Cvar_SetFloatValue;
	import->Cvar_SetIntegerValue = Cvar_SetIntegerValue;
	import->Cvar_CheckRange = Cvar_CheckRange;

    import->Cmd_AddCommand = Cmd_AddCommand;
    import->Cmd_RemoveCommand = Cmd_RemoveCommand;
    import->Cmd_ExecuteString = Cmd_ExecuteString;
    import->Cmd_Argc = Cmd_Argc;
    import->Cmd_ArgsFrom = Cmd_ArgsFrom;
    import->Cmd_Argv = Cmd_Argv;
    import->Cmd_Clear = Cmd_Clear;

    import->FS_Write = FS_Write;
    import->FS_Read = FS_Read;
    import->FS_FileSeek = FS_FileSeek;
    import->FS_FileTell = FS_FileTell;
    import->FS_FileLength = FS_FileLength;
    import->FS_FileExists = FS_FileExists;
    import->FS_FOpenRead = FS_FOpenRead;
    import->FS_FOpenWrite = FS_FOpenWrite;
    import->FS_FClose = FS_FClose;
    import->FS_FreeFile = FS_FreeFile;
    import->FS_LoadFile = FS_LoadFile;
	import->FS_ListFiles = FS_ListFiles;

	import->Key_IsDown = Key_IsDown;

	import->G_GetCurrentMap = G_GetCurrentMap;
	import->Map_GetSpriteCoords = Map_GetSpriteCoords;

	// most of this stuff is for imgui's usage
	import->SDL_SetCursor = SDL_SetCursor;
    import->SDL_SetHint = SDL_SetHint;
    import->SDL_FreeCursor = SDL_FreeCursor;
    import->SDL_CaptureMouse = SDL_CaptureMouse;
    import->SDL_GetKeyboardFocus = SDL_GetKeyboardFocus;
    import->SDL_GetWindowFlags = SDL_GetWindowFlags;
    import->SDL_WarpMouseInWindow = SDL_WarpMouseInWindow;
    import->SDL_GetGlobalMouseState = SDL_GetGlobalMouseState;
    import->SDL_GetWindowPosition = SDL_GetWindowPosition;
	import->SDL_GetWindowSize = SDL_GetWindowSize;
    import->SDL_ShowCursor = SDL_ShowCursor;
    import->SDL_GetRendererOutputSize = SDL_GetRendererOutputSize;
    import->SDL_GameControllerGetButton = SDL_GameControllerGetButton;
    import->SDL_GameControllerGetAxis = SDL_GameControllerGetAxis;
    import->SDL_GameControllerOpen = SDL_GameControllerOpen;
    import->SDL_GetClipboardText = SDL_GetClipboardText;
    import->SDL_SetClipboardText = SDL_SetClipboardText;
    import->SDL_SetTextInputRect = SDL_SetTextInputRect;
    import->SDL_GetCurrentVideoDriver = SDL_GetCurrentVideoDriver;
    import->SDL_CreateSystemCursor = SDL_CreateSystemCursor;
    import->SDL_GetWindowWMInfo = SDL_GetWindowWMInfo;
    import->SDL_Init = SDL_Init;
    import->SDL_Quit = SDL_Quit;
    import->SDL_GetTicks64 = SDL_GetTicks64;
    import->SDL_GetPerformanceCounter = SDL_GetPerformanceCounter;
    import->SDL_GetPerformanceFrequency = SDL_GetPerformanceFrequency;
    import->SDL_GL_GetDrawableSize = SDL_GL_GetDrawableSize;
    import->SDL_CreateWindow = SDL_CreateWindow;
    import->SDL_DestroyWindow = SDL_DestroyWindow;
    import->SDL_GL_SwapWindow = SDL_GL_SwapWindow;
    import->SDL_GL_CreateContext = SDL_GL_CreateContext;
    import->SDL_GL_GetProcAddress = SDL_GL_GetProcAddress;
    import->SDL_GL_DeleteContext = SDL_GL_DeleteContext;
    import->SDL_GL_SetAttribute = SDL_GL_SetAttribute;
    import->SDL_GL_MakeCurrent = SDL_GL_MakeCurrent;
    import->SDL_GL_SetSwapInterval = SDL_GL_SetSwapInterval;
    import->SDL_GetError = SDL_GetError;
	import->SDL_PollEvent = SDL_PollEvent;

	import->Sys_Exit = Sys_Exit;

#if defined(__OS2__) || defined(_WIN32)
	import->SDL_CreateThread = PFN_SDL_CreateThread;
	import->SDL_CreateThreadWithStackSize = PFN_SDL_CreateThreadWithStackSize;
#else
	import->SDL_CreateThread = SDL_CreateThread;
	import->SDL_CreateThreadWithStackSize = SDL_CreateThreadWithStackSize;
#endif
    import->SDL_WaitThread = SDL_WaitThread;
    import->SDL_SetThreadPriority = SDL_SetThreadPriority;
    import->SDL_DetachThread = SDL_DetachThread;
    import->SDL_GetThreadName = SDL_GetThreadName;
    import->SDL_ThreadID = SDL_ThreadID;
    import->SDL_GetThreadID = SDL_GetThreadID;

	import->SDL_CreateMutex = SDL_CreateMutex;
    import->SDL_DestroyMutex = SDL_DestroyMutex;
    import->SDL_LockMutex = SDL_LockMutex;
    import->SDL_UnlockMutex = SDL_UnlockMutex;
    import->SDL_TryLockMutex = SDL_TryLockMutex;

	import->SDL_CreateCond = SDL_CreateCond;
    import->SDL_DestroyCond = SDL_DestroyCond;
    import->SDL_CondSignal = SDL_CondSignal;
    import->SDL_CondBroadcast = SDL_CondBroadcast;
    import->SDL_CondWait = SDL_CondWait;
    import->SDL_CondWaitTimeout = SDL_CondWaitTimeout;
}
