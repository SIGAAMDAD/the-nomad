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

    void (*Z_Free)(void *ptr);
    void (*Z_FreeTags)(int lowtag, int hightag);
    void (*Z_ChangeTag)(void* user, int32_t tag);
    void (*Z_ChangeUser)(void* newuser, void* olduser);
    void (*Z_ChangeName)(void* user, const char* name);
    void (*Z_CleanCache)(void);
    void (*Z_CheckHeap)(void);
    void (*Z_ClearZone)(void);
    void (*Z_Print)(bool all);
    uint64_t (*Z_FreeMemory)(void);
    uint32_t (*Z_NumBlocks)(int tag);

    void *(*Alloca)(size_t size);

    void *(*Mem_Alloc)(const uint32_t size);
    void (*Mem_Free)(void *ptr);
    uint32_t (*Mem_Msize)(void *ptr);
    bool (*Mem_DefragIsActive)(void);
    void (*Mem_AllocDefragBlock)(void);

    eastl::vector<char>& (*Con_GetBuffer)(void);
    void (GDR_DECL *Con_Printf)(loglevel_t level, const char *fmt, ...) GDR_ATTRIBUTE((format(printf, 2, 3)));
    void (GDR_DECL *Con_Error)(bool exit, const char *fmt, ...) GDR_ATTRIBUTE((format(printf, 2, 3)));
    const char *(GDR_DECL *va)(const char *fmt, ...) GDR_ATTRIBUTE((format(printf, 1, 2)));
    void GDR_NORETURN (GDR_DECL *N_Error)(const char *err, ...) GDR_ATTRIBUTE((format(printf, 1, 2)));

    cvar_t *(*Cvar_Find)(const char *name);
    void (*Cvar_RegisterName)(const char *name, const char *value, cvartype_t type, int32_t flags);
    void (*Cvar_ChangeValue)(const char *name, const char *value);
    void (*Cvar_Register)(cvar_t *cvar, const char *value);
    const char *(*Cvar_GetValue)(const char *name);

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
    qboolean *(*Com_GetKeyboard)(void);
    void *(*Com_GetEvents)(void);

    uint64_t (*FS_Write)(const void *data, uint64_t size, file_t f);
    uint64_t (*FS_Read)(void *data, uint64_t size, file_t f);
    file_t (*FS_OpenBFF)(int32_t index);
    file_t (*FS_FOpenRead)(const char *filepath);
    file_t (*FS_FOpenWrite)(const char *filepath);
    file_t (*FS_CreateTmp)(char **name, const char *ext);
    char *(*FS_GetOSPath)(file_t f);
    void *(*FS_GetBFFData)(file_t handle);
    void (*FS_FClose)(file_t handle);
    uint64_t (*FS_FileLength)(file_t f);
    void (*FS_Remove)(const char *ospath);
    uint64_t (*FS_FileTell)(file_t f);
    fileOffset_t (*FS_FileSeek)(file_t f, fileOffset_t offset, uint32_t whence);
    file_t (*FS_BFFOpen)(const char *chunkpath);
    qboolean (*FS_FileExists)(const char *file);
    uint64_t (*FS_LoadFile)(const char *path, void **buffer);
    void (*FS_FreeFile)(void *buffer);

    bffinfo_t *(*BFF_FetchInfo)(void);
    bffscript_t *(*BFF_FetchScript)(const char *name);
    bfflevel_t *(*BFF_FetchLevel)(const char *name);
    const eastl::vector<const bfflevel_t*>& (*BFF_OrderLevels)(const bffinfo_t *info);
    const eastl::vector<const bfftexture_t*>& (*BFF_OrderTextures)(const bffinfo_t *info);

    const eastl::shared_ptr<GDRMap>& (*G_GetCurrentMap)(void);

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
} renderImport_t;

#define GO_AWAY_MANGLE extern "C"

// rendering engine interface
GO_AWAY_MANGLE GDR_EXPORT void RE_Init(renderImport_t *import);
GO_AWAY_MANGLE GDR_EXPORT void RE_Shutdown(void);
GO_AWAY_MANGLE GDR_EXPORT void RE_BeginFrame(void);
GO_AWAY_MANGLE GDR_EXPORT void RE_EndFrame(void);
GO_AWAY_MANGLE GDR_EXPORT void RE_InitFrameData(void);
GO_AWAY_MANGLE GDR_EXPORT void RE_SubmitMapTilesheet(const char *chunkname, const bffinfo_t *info);
GO_AWAY_MANGLE GDR_EXPORT void RE_CacheTextures(void);
GO_AWAY_MANGLE GDR_EXPORT qboolean RE_ConsoleIsOpen(void);

#endif