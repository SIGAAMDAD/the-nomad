#ifndef _Z_HUNK_
#define _Z_HUNK_

#pragma once

typedef enum {
    h_low,
    h_high,
	h_dontcare
} ha_pref;

enum {
	TAG_FREE		= 0, // a free block
	TAG_STATIC		= 1, // stays allocated for entirety (or until explicitly free) of execution time
	TAG_LEVEL		= 2, // level-scoped allocations
	TAG_RENDERER	= 3, // allocations made from any of the rendering libraries
	TAG_SFX			= 4, // general sound allocations
	TAG_MUSIC		= 5, // music allocations
	TAG_SEARCH_PATH	= 6, // a filesystem searchpath
	TAG_BFF			= 7, // a bff archive file
	TAG_HUNK		= 8, // allocated with temp hunk
	TAG_SMALL		= 9, // special smallzone tag
	TAG_PURGELEVEL	= 100, // purgeable block
	TAG_CACHE		= 101, // cached block, migh be used in the future, but can also be purged
};

#define NUMTAGS 11

#define TAG_SCOPE TAG_CACHE
#define TAG_LOAD TAG_CACHE

#ifdef __cplusplus

GO_AWAY_MANGLE void Memory_Init(void);
GO_AWAY_MANGLE void Memory_Shutdown(void);

#ifdef _NOMAD_DEBUG
GO_AWAY_MANGLE void *Hunk_AllocDebug( uint64_t size, ha_pref preference, const char *name, const char *file, uint64_t line );
#define Hunk_Alloc(size,preference) Hunk_AllocDebug(size,preference,#size,__FILE__,__LINE__)
#else
GO_AWAY_MANGLE void *Hunk_Alloc( uint64_t size, ha_pref preference );
#endif
GO_AWAY_MANGLE void Hunk_Clear(void);
GO_AWAY_MANGLE void *Hunk_AllocateTempMemory(uint64_t size);
GO_AWAY_MANGLE void Hunk_FreeTempMemory(void *buffer);
GO_AWAY_MANGLE void Hunk_ClearTempMemory(void);
GO_AWAY_MANGLE uint64_t Hunk_MemoryRemaining(void);
GO_AWAY_MANGLE void Hunk_Log(void);
GO_AWAY_MANGLE void Hunk_SmallLog(void);
GO_AWAY_MANGLE qboolean Hunk_CheckMark(void);
GO_AWAY_MANGLE void Hunk_ClearToMark( void );
GO_AWAY_MANGLE void Hunk_Print(void);
GO_AWAY_MANGLE void Hunk_Check(void);
GO_AWAY_MANGLE void Hunk_InitMemory(void);
GO_AWAY_MANGLE qboolean Hunk_TempIsClear(void);
GO_AWAY_MANGLE void *Hunk_ReallocateTempMemory(void *ptr, uint64_t nsize);

GO_AWAY_MANGLE uint64_t Com_TouchMemory(void);

GO_AWAY_MANGLE void* Z_SMalloc(uint32_t size);
GO_AWAY_MANGLE void* Z_Malloc(uint32_t size, int tag);
GO_AWAY_MANGLE char* Z_Strdup(const char *str);
GO_AWAY_MANGLE void Z_Free(void *ptr);

GO_AWAY_MANGLE void Z_FreeTags(int lowtag, int hightag);
GO_AWAY_MANGLE void Z_ChangeTag(void* user, int tag);
GO_AWAY_MANGLE void Z_CleanCache(void);
GO_AWAY_MANGLE void Z_CheckHeap(void);
GO_AWAY_MANGLE void Z_ClearZone(memzone_t *zone);
GO_AWAY_MANGLE void Z_Print(bool all);
GO_AWAY_MANGLE void Z_Init(void);
GO_AWAY_MANGLE uint64_t Z_FreeMemory(void);
GO_AWAY_MANGLE void* Z_ZoneBegin(void);
GO_AWAY_MANGLE void* Z_ZoneEnd(void);
GO_AWAY_MANGLE uint64_t Z_BlockSize(void *p);
GO_AWAY_MANGLE uint32_t Z_NumBlocks(int tag);
GO_AWAY_MANGLE void Z_TouchMemory(uint64_t *sum);

GO_AWAY_MANGLE void Mem_Info(void);

typedef struct
{
	int num;
	int minSize;
	int maxSize;
	int totalSize;
} memoryStats_t;

GO_AWAY_MANGLE void Mem_Shutdown( void );
GO_AWAY_MANGLE void Mem_EnableLeakTest( const char *name );
GO_AWAY_MANGLE void Mem_ClearFrameStats( void );
GO_AWAY_MANGLE void Mem_GetFrameStats( memoryStats_t& allocs, memoryStats_t& frees );
GO_AWAY_MANGLE void Mem_GetStats( memoryStats_t& stats );
GO_AWAY_MANGLE void Mem_AllocDefragBlock( void );
GO_AWAY_MANGLE uint32_t Mem_Msize(void *ptr);
GO_AWAY_MANGLE bool Mem_DefragIsActive(void);

GO_AWAY_MANGLE void* Mem_Alloc(const uint32_t size);
GO_AWAY_MANGLE void* Mem_ClearedAlloc(const uint32_t size);
GO_AWAY_MANGLE void Mem_Free(void *ptr);
GO_AWAY_MANGLE char* Mem_CopyString(const char *in);
GO_AWAY_MANGLE void* Mem_Alloc16(const uint32_t size);
GO_AWAY_MANGLE void Mem_Free16(void *ptr);


#endif


#endif
