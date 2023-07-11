#ifndef _RGL_PUBLIC_
#define _RGL_PUBLIC_

#pragma once

#include "../src/n_shared.h"

typedef struct
{
    void *(*Hunk_Alloc)(uint64_t size, const char *name, int where);
    void (*Hunk_Check)(void);
    void (*Hunk_Print)(void);
    void (*Hunk_FreeToLowMark)(uint64_t mark);
    void (*Hunk_FreeToHighMark)(uint64_t mark);
    void (*Hunk_Clear)(void);
    void *(*Hunk_TempAlloc)(uint32_t size);
    uint64_t (*Hunk_LowMark)(void);
    uint64_t (*Hunk_HighMark)(void);

    void *(*Z_Malloc)(uint32_t size, int32_t tag, void *user, const char* name);
    void *(*Z_Calloc)(uint32_t size, int32_t tag, void *user, const char* name);
    void *(*Z_Realloc)(uint32_t nsize, int32_t tag, void *user, void *ptr, const char* name);

    void (*Z_Free)(void *ptr);
    void (*Z_FreeTags)(int32_t lowtag, int32_t hightag);
    void (*Z_ChangeTag)(void* user, int32_t tag);
    void (*Z_ChangeUser)(void* newuser, void* olduser);
    void (*Z_ChangeName)(void* user, const char* name);
    void (*Z_CleanCache)(void);
    void (*Z_CheckHeap)(void);
    void (*Z_ClearZone)(void);
    void (*Z_Print)(bool all);
    uint64_t (*Z_FreeMemory)(void);
    uint32_t (*Z_NumBlocks)(int32_t tag);

    void *(*Alloca)(uint32_t size);

    void *(*Mem_Alloc)(const uint32_t size);
    void (*Mem_Free)(void *ptr);
    uint32_t (*Mem_Msize)(void *ptr);
    bool (*Mem_DefragIsActive)(void);
    void (*Mem_AllocDefragBlock)(void);

    void (GDR_DECL *Con_Printf)(loglevel_t level, const char *fmt, ...) GDR_ATTRIBUTE((format(printf, 2, 3)));
    void (GDR_DECL *Con_Error)(const char *fmt, ...) GDR_ATTRIBUTE((format(printf, 1, 2)));
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

    eventState_t *(*Com_GetEvents)(void);

    bffinfo_t *(*BFF_FetchInfo)(void);
    bffscript_t *(*BFF_FetchScript)(const char *name);
    bfflevel_t *(*BFF_FetchLevel)(const char *name);
    const eastl::vector<const bfflevel_t*>& (*BFF_OrderLevels)(const bffinfo_t *info);
    const eastl::vector<const bfftexture_t*>& (*BFF_OrderTextures)(const bffinfo_t *info);

    const eastl::shared_ptr<GDRMap>& (*G_GetCurrentMap)(void);
} renderImport_t;

// rendering engine interface
void RE_Init(renderImport_t *import);
void RE_Shutdown(void);
void RE_BeginFrame(void);
void RE_EndFrame(void);
void RE_InitFrameData(void);
void RE_SubmitMapTilesheet(const char *chunkname, const bffinfo_t *info);
void RE_CacheTextures(void);

#endif