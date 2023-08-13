#ifndef _RGL_PUBLIC_
#define _RGL_PUBLIC_

#pragma once

#include "../src/n_shared.h"

typedef struct
{
#ifdef _NOMAD_DEBUG
    void *(*Hunk_AllocDebug)(uint64_t size, ha_pref where, const char *label, const char *file, uint64_t line);
#else
    void *(*Hunk_Alloc)(uint64_t size, const char *name, ha_pref where);
#endif
    void (*Hunk_Log)(void);
    void (*Hunk_SmallLog)(void);
    uint64_t (*Hunk_MemoryRemaining)(void);
    void (*Hunk_SetMark)(void);
    void (*Hunk_ClearToMark)(void);
    qboolean (*Hunk_CheckMark)(void);
    void (*Hunk_Clear)(void);
    void *(*Hunk_AllocateTempMemory)(uint64_t size);
    void (*Hunk_FreeTempMemory)(void *buf);
    void (*Hunk_ClearTempMemory)(void);

    void *(*Z_Malloc)(uint64_t size, int tag, void *user, const char *name);
    void *(*Z_Calloc)(uint64_t size, int tag, void *user, const char *name);
    void *(*Z_Realloc)(uint64_t nsize, int tag, void *user, void *ptr, const char *name);
    char *(*Z_Strdup)(const char *str);

    void (*Z_Free)(void *ptr);
    void (*Z_FreeTags)(int lowtag, int hightag);
    void (*Z_ChangeTag)(void* user, int32_t tag);
    void (*Z_ChangeUser)(void* newuser, void* olduser);
    void (*Z_ChangeName)(void* user, const char* name);
    void (*Z_CleanCache)(void);
    void (*Z_CheckHeap)(void);
    void (*Z_ClearZone)(void);
    uint64_t (*Z_FreeMemory)(void);
    uint32_t (*Z_NumBlocks)(int tag);
    uint64_t (*Z_BlockSize)(void *p);

    void (*Sys_FreeFileList)(char **list);

    void *(*Mem_Alloc)(const uint32_t size);
    void (*Mem_Free)(void *ptr);
    uint32_t (*Mem_Msize)(void *ptr);
    void (*Mem_AllocDefragBlock)(void);

    void (GDR_DECL *Con_Printf)(loglevel_t level, const char *fmt, ...) GDR_ATTRIBUTE((format(printf, 2, 3)));
    const char *(GDR_DECL *va)(const char *fmt, ...) GDR_ATTRIBUTE((format(printf, 1, 2)));
    void GDR_NORETURN (GDR_DECL *N_Error)(const char *err, ...) GDR_ATTRIBUTE((format(printf, 1, 2)));

    void (*Cvar_VariableStringBuffer)(const char *name, char *buffer, uint64_t bufferSize);
    void (*Cvar_VariableStringBufferSafe)(const char *name, char *buffer, uint64_t bufferSize, uint32_t flag);
    int32_t (*Cvar_VariableInteger)(const char *name);
    float (*Cvar_VariableFloat)(const char *name);
    qboolean (*Cvar_VariableBoolean)(const char *name);
    const char *(*Cvar_VariableString)(const char *name);
    uint32_t (*Cvar_Flags)(const char *name);
    cvar_t *(*Cvar_Get)(const char *name, const char *value, uint32_t flags);
    void (*Cvar_SetGroup)(cvar_t *cv, cvarGroup_t group);
    void (*Cvar_SetDescription)(cvar_t *cv, const char *description);
    void (*Cvar_Set)(const char *name, const char *value);
    qboolean (*Cvar_SetModified)(const char *name, qboolean modified);
    void (*Cvar_SetSafe)(const char *name, const char *value);
    void (*Cvar_SetIntegerValue)(const char *name, int32_t value);
    void (*Cvar_SetFloatValue)(const char *name, float value);
    void (*Cvar_SetStringValue)(const char *name, const char *value);
    void (*Cvar_SetBooleanValue)(const char *name, qboolean value);

    void (*Cmd_AddCommand)(const char* name, cmdfunc_t function);
    void (*Cmd_RemoveCommand)(const char* name);
    void (*Cmd_ExecuteCommand)(const char* name);
    void (*Cmd_ExecuteText)(const char *str);
    void (*Cmd_ExecuteString)(const char *str);
    uint32_t (*Cmd_Argc)(void);
    char *(*Cmd_ArgsFrom)(int32_t index);
    const char *(*Cmd_Argv)(uint32_t index);
    void (*Cmd_Clear)(void);

    uint32_t (*Com_GetWindowEvents)(void);
    qboolean **(*Com_GetKeyboard)(void);
    void *(*Com_GetEvents)(void);

    void GDR_NORETURN (*Sys_Exit)(int code);

    const vec2_t* (*Map_GetSpriteCoords)(uint32_t gid);

    uint64_t (*FS_Write)(const void *buffer, uint64_t size, file_t f);
    uint64_t (*FS_Read)(void *buffer, uint64_t size, file_t);
    fileOffset_t (*FS_FileSeek)(file_t f, fileOffset_t offset, uint32_t whence);
    fileOffset_t (*FS_FileTell)(file_t f);
    uint64_t (*FS_FileLength)(file_t f);
    qboolean (*FS_FileExists)(const char *filename);
    file_t (*FS_FOpenRead)(const char *path);
    file_t (*FS_FOpenWrite)(const char *path);
    void (*FS_FClose)(file_t f);
    void (*FS_FreeFile)(void *buffer);
    uint64_t (*FS_LoadFile)(const char *path, void **buffer);
    char **(*FS_GetCurrentChunkList)(uint64_t *numchunks);
    char **(*FS_ListFiles)(const char *path, const char *extension, uint64_t *numfiles);

    const nmap_t *(*G_GetCurrentMap)(void);

    // most of this stuff is for imgui's usage
    SDL_bool (*SDL_SetHint)(const char *name, const char *value);
    void (*SDL_FreeCursor)(SDL_Cursor *cursor);
    void (*SDL_SetCursor)(SDL_Cursor *cursor);
    int (*SDL_CaptureMouse)(SDL_bool enabled);
    SDL_Window *(*SDL_GetKeyboardFocus)(void);
    Uint32 (*SDL_GetWindowFlags)(SDL_Window *window);
    void (*SDL_WarpMouseInWindow)(SDL_Window *window, int x, int y);
    Uint32 (*SDL_GetGlobalMouseState)(int *x, int *y);
    void (*SDL_GetWindowPosition)(SDL_Window *window, int *x, int *y);
    void (*SDL_GetWindowSize)(SDL_Window *window, int *w, int *h);
    int (*SDL_ShowCursor)(int toggle);
    int (*SDL_GetRendererOutputSize)(SDL_Renderer *renderer, int *w, int *h);
    Uint8 (*SDL_GameControllerGetButton)(SDL_GameController *gamecontroller, SDL_GameControllerButton button);
    Sint16 (*SDL_GameControllerGetAxis)(SDL_GameController *gamecontroller, SDL_GameControllerAxis axis);
    SDL_GameController *(*SDL_GameControllerOpen)(int joystick_index);
    char *(*SDL_GetClipboardText)(void);
    int (*SDL_SetClipboardText)(const char *text);
    void (*SDL_SetTextInputRect)(const SDL_Rect *rect);
    const char *(*SDL_GetCurrentVideoDriver)(void);
    SDL_Cursor *(*SDL_CreateSystemCursor)(SDL_SystemCursor id);
    SDL_bool (*SDL_GetWindowWMInfo)(SDL_Window *window, SDL_SysWMinfo *info);
    int (*SDL_Init)(Uint32 flags);
    void (*SDL_Quit)(void);
    Uint64 (*SDL_GetTicks64)(void);
    Uint64 (*SDL_GetPerformanceCounter)(void);
    Uint64 (*SDL_GetPerformanceFrequency)(void);
    void (*SDL_GL_GetDrawableSize)(SDL_Window *window, int *w, int *h);
    SDL_Window *(*SDL_CreateWindow)(const char *title, int x, int y, int w, int h, Uint32 flags);
    void (*SDL_DestroyWindow)(SDL_Window *window);
    void (*SDL_GL_SwapWindow)(SDL_Window *window);
    SDL_GLContext (*SDL_GL_CreateContext)(SDL_Window *window);
    void *(*SDL_GL_GetProcAddress)(const char *proc);
    void (*SDL_GL_DeleteContext)(SDL_GLContext context);
    int (*SDL_GL_SetAttribute)(SDL_GLattr attr, int value);
    int (*SDL_GL_MakeCurrent)(SDL_Window *window, SDL_GLContext context);
    int (*SDL_GL_SetSwapInterval)(int interval);
    const char *(*SDL_GetError)();
    int (*SDL_PollEvent)(SDL_Event *events);
    
    SDL_Thread *(*SDL_CreateThread)(SDL_ThreadFunction fn, const char *name, void *dat);
    SDL_Thread *(*SDL_CreateThreadWithStackSize)(SDL_ThreadFunction fn, const char *name, const size_t stacksize, void *data);
    void (*SDL_WaitThread)(SDL_Thread *thread, int *status);
    int (*SDL_SetThreadPriority)(SDL_ThreadPriority priority);
    void (*SDL_DetachThread)(SDL_Thread *thread);
    const char *(*SDL_GetThreadName)(SDL_Thread *thread);
    SDL_threadID (*SDL_ThreadID)(void);
    SDL_threadID (*SDL_GetThreadID)(SDL_Thread *thread);

    SDL_mutex *(*SDL_CreateMutex)(void);
    void (*SDL_DestroyMutex)(SDL_mutex *mutex);
    int (*SDL_LockMutex)(SDL_mutex *mutex);
    int (*SDL_UnlockMutex)(SDL_mutex *mutex);
    int (*SDL_TryLockMutex)(SDL_mutex *mutex);

    SDL_cond *(*SDL_CreateCond)(void);
    void (*SDL_DestroyCond)(SDL_cond *cond);
    int (*SDL_CondSignal)(SDL_cond *cond);
    int (*SDL_CondBroadcast)(SDL_cond *cond);
    int (*SDL_CondWait)(SDL_cond *cond, SDL_mutex *mutex);
    int (*SDL_CondWaitTimeout)(SDL_cond *cond, SDL_mutex *mutex, Uint32 ms);
} renderImport_t;

typedef struct renderEntityRef_s renderEntityRef_t;
typedef struct
{
    vec4_t color;
    vec3_t pos;
    
    float size;
    float rotation;
    qboolean filled;
    nhandle_t texture;
} renderRect_t;

// rendering engine interface
GO_AWAY_MANGLE GDR_EXPORT void RE_Init(renderImport_t *import);
GO_AWAY_MANGLE GDR_EXPORT void RE_Shutdown(void);
GO_AWAY_MANGLE GDR_EXPORT void RE_BeginFrame(void);
GO_AWAY_MANGLE GDR_EXPORT void RE_EndFrame(void);
GO_AWAY_MANGLE GDR_EXPORT void RE_InitFrameData(void);
GO_AWAY_MANGLE GDR_EXPORT nhandle_t RE_RegisterTexture(const char *name);
GO_AWAY_MANGLE GDR_EXPORT nhandle_t RE_RegisterShader(const char *name);
GO_AWAY_MANGLE GDR_EXPORT qboolean RE_ConsoleIsOpen(void);
GO_AWAY_MANGLE GDR_EXPORT void RE_ProcessConsoleEvents(SDL_Event *events);

// rendering commands
GO_AWAY_MANGLE GDR_EXPORT void RE_AddDrawEntity(renderEntityRef_t *ref);
GO_AWAY_MANGLE GDR_EXPORT void RE_SetColor(const float *color, uint32_t count);
GO_AWAY_MANGLE GDR_EXPORT void RE_DrawRect(renderRect_t *rect);

#endif