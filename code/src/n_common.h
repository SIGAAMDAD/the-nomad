#ifndef _N_COMMON_
#define _N_COMMON_

#pragma once

#include "../common/n_vm.h"

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
<<<<<<< HEAD
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

=======

char* va(const char *fmt, ...);

#ifdef Q3_VM
void GDR_DECL Com_Error(const char *fmt, ...);
#else
void GDR_DECL Com_Error(vm_t *vm, const char *fmt, ...);
>>>>>>> parent of 7803dbe (got sound working, engine almost don)
#endif

/*
The filesystem, heavily based on quake's filesystem

THIS SHOULD NEVER BE USED BY THE VM
*/

#define FS_INVALID_HANDLE 0

typedef int32_t file_t;
<<<<<<< HEAD
#if defined(_MSVC_VER) || defined(__clang__)
typedef _off_t fileOffset_t;
#elif !defined(Q3_VM)
typedef off_t fileOffset_t;
#else
typedef long fileOffset_t;
#endif

extern cvar_t fs_gamedir;
extern cvar_t fs_numArchives;
=======
>>>>>>> parent of 7803dbe (got sound working, engine almost don)

void FS_Init(void);

int FS_Write(const void *data, uint32_t size, file_t f);
int FS_Read(void *data, uint32_t size, file_t f);
file_t FS_OpenBFF();
file_t FS_FOpenRead(const char *filepath);
file_t FS_FOpenWrite(const char *filepath);
<<<<<<< HEAD
file_t FS_CreateTmp(char **name, const char *ext);
char* FS_GetOSPath(file_t f);
//uint32_t FS_NumBFFs(void);
void* FS_GetBFFData(file_t handle);
void FS_FClose(file_t handle);
uint64_t FS_FileLength(file_t f);
void FS_Remove(const char *ospath);
uint64_t FS_FileTell(file_t f);
fileOffset_t FS_FileSeek(file_t f, fileOffset_t offset, uint32_t whence);
=======
file_t FS_BFFOpen(const char *chunkpath);
void FS_FClose(file_t* handle);
void FS_BFFClose(file_t* handle);

>>>>>>> parent of 7803dbe (got sound working, engine almost don)
qboolean FS_FileExists(const char *file);


/*
System calls, engine only stuff
*/

#ifndef Q3_VM
#ifdef _WIN32
#define nstat_t struct __stat64
#else
#include <sys/stat.h>
#define nstat_t struct stat
#endif

FILE* Sys_FOpen(const char *filepath, const char *mode);
int Sys_stat(nstat_t* buffer, const char *filepath);
void GDR_DECL Sys_Print(const char* str);
#endif

#endif
