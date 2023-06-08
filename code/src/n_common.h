#ifndef _N_COMMON_
#define _N_COMMON_

#pragma once

/*
Common functionality for the engine and vm alike
*/
extern qboolean console_open;
typedef struct
{
    const uint8_t *kbstate;
} eventState_t;
extern eventState_t evState;

void Com_Init(void);
void GDR_DECL Com_Printf(const char *fmt, ...);
uint64_t Com_GenerateHashValue(const char *fname, const uint32_t size);
void Con_RenderConsole(void);
void Com_UpdateEvents(void);
void GDR_DECL Com_Error(const char *fmt, ...);

/*
commands, shouldn't be called by the vm
*/

#ifndef Q3_VM

typedef void (*completionFunc_t)(const char* args, uint32_t argnum);
typedef void (*cmdfunc_t)(void);

void Cmd_AddCommand(const char* name, cmdfunc_t function);
void Cmd_ExecuteCommand(const char* name);

#endif

/*
The filesystem, heavily based on quake's filesystem

THIS SHOULD NEVER BE USED BY THE VM
*/

#define FS_INVALID_HANDLE 0
#define FS_SEEK_CUR 0
#define FS_SEEK_BEGIN 1
#define FS_SEEK_END 2

typedef int32_t file_t;
#if defined(_MSVC_VER) || defined(__clang__)
typedef _off_t fileOffset_t;
#elif !defined(Q3_VM)
typedef off_t fileOffset_t;
#else
typedef long fileOffset_t;
#endif

extern cvar_t fs_gamedir;
extern cvar_t fs_numArchives;

void FS_Init(void);

uint64_t FS_Write(const void *data, uint64_t size, file_t f);
uint64_t FS_Read(void *data, uint64_t size, file_t f);
file_t FS_OpenBFF(int32_t index);
file_t FS_FOpenRead(const char *filepath);
file_t FS_FOpenWrite(const char *filepath);
file_t FS_CreateTmp(char **name, const char *ext);
char* FS_GetOSPath(file_t f);
//uint32_t FS_NumBFFs(void);
void* FS_GetBFFData(file_t handle);
void FS_FClose(file_t handle);
uint64_t FS_FileLength(file_t f);
void FS_Remove(const char *ospath);
uint64_t FS_FileTell(file_t f);
fileOffset_t FS_FileSeek(file_t f, fileOffset_t offset, uint32_t whence);
qboolean FS_FileExists(const char *file);
uint64_t FS_LoadFile(const char *filepath, void **buffer);
uint64_t FS_ReadFile(const char *filepath, void *buffer);


/*
System calls, engine only stuff
*/

#ifndef Q3_VM
#ifdef _WIN32
#define nstat_t struct __stat64
#else
#define nstat_t struct stat
#endif

void Sys_mkdir(const char *dirpath);
FILE* Sys_FOpen(const char *filepath, const char *mode);
const char* Sys_pwd(void);
void GDR_DECL Sys_Print(const char* str);
#endif

#endif
