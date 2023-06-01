#ifndef _N_COMMON_
#define _N_COMMON_

#pragma once

#include "../common/n_vm.h"

/*
Common functionality for the engine and vm alike
*/
void Com_Init(void);
void GDR_DECL Com_Printf(const char *fmt, ...);
uint64_t Com_GenerateHashValue(const char *fname, const uint32_t size);

char* va(const char *fmt, ...);

#ifdef Q3_VM
void GDR_DECL Com_Error(const char *fmt, ...);
#else
void GDR_DECL Com_Error(vm_t *vm, const char *fmt, ...);
#endif

/*
The filesystem, heavily based on quake's filesystem

THIS SHOULD NEVER BE USED BY THE VM
*/

#define FS_INVALID_HANDLE 0

typedef int32_t file_t;

void FS_Init(void);

int FS_Write(const void *data, uint32_t size, file_t f);
int FS_Read(void *data, uint32_t size, file_t f);
file_t FS_OpenBFF();
file_t FS_FOpenRead(const char *filepath);
file_t FS_FOpenWrite(const char *filepath);
file_t FS_BFFOpen(const char *chunkpath);
void FS_FClose(file_t* handle);
void FS_BFFClose(file_t* handle);

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