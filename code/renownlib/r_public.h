#ifndef __R_PUBLIC__
#define __R_PUBLIC__

#pragma once

#include "../engine/n_shared.h"
#include "../sgame/sg_public.h"

#define RENOWN_API_VERSION 1

typedef struct
{
	void GDR_DECL GDR_ATTRIBUTE((format(printf, 2, 3))) (*Printf)( int level, const char *fmt, ... );
	void GDR_DECL GDR_ATTRIBUTE((format(printf, 2, 3))) (*Error)( errorCode_t code, const char *fmt, ... );
	
	void *(*Hunk_Alloc)( uint64_t nBytes, ha_pref where );
	void *(*Hunk_AllocateTempMemory)( uint64_t nBytes );
	void (*Hunk_FreeTempMemory)( void *pBuf );
	
	void *(*Malloc)( uint64_t nBytes );
	void (*Free)( void *pBuffer );
	void (*FreeAll)( void );
	
	uint64_t (*FS_LoadFile)( const char *pFilename, void **pBuffer );
	void (*FS_FreeFile)( void *pBuffer );
	file_t (*FS_FOpenRead)( const char *pFilename );
	file_t (*FS_FOpenWrite)( const char *pFilename );
	void (*FS_FClose)( file_t hFile );
	
	uint64_t (*FS_Read)( void *pBuffer, uint64_t nBytes, file_t hFile );
	uint64_t (*FS_Write)( const void *pBuffer, uint64_t nBytes, file_t hFile );
	uint64_t (*FS_FileLength)( file_t hFile );
	
	nhandle_t (*InitThread)( const char *name, void (*fn)( void ) );
    void (*MutexInit)( nhandle_t *pMutexHandle );
    void (*MutexLock)( nhandle_t hMutex );
    void (*MutexUnlock)( nhandle_t hMutex );
} renownImport_t;

typedef struct
{
	void (*Renown_Init)( void );
	void (*Renown_Shutdown)( void );
	void (*Renown_Update)( void );

    void (*Renown_AddKillEvent)( const linkEntity_t *killer, const linkEntity_t *killed );
} renownExport_t;

GDR_EXPORT renownExport_t *GetRenownAPI( uint64_t nVersion, const renownImport_t *imp );

#endif
