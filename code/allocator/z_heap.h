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
	TAG_PURGELEVEL	= 100, // purgeable block
	TAG_CACHE		= 101, // cached block, migh be used in the future, but can also be purged
};

#define NUMTAGS 11

#define TAG_SCOPE TAG_CACHE
#define TAG_LOAD TAG_CACHE

GO_AWAY_MANGLE void Memory_Init(void);
GO_AWAY_MANGLE void Memory_Shutdown(void);

#ifdef _NOMAD_DEBUG
GO_AWAY_MANGLE void *Hunk_AllocDebug( uint64_t size, ha_pref preference, const char *name, const char *file, uint64_t line );
#define Hunk_Alloc(size,name,preference) Hunk_AllocDebug(size,preference,name,__FILE__,__LINE__)
#else
GO_AWAY_MANGLE void *Hunk_Alloc( uint64_t size, const char *name, ha_pref preference );
#endif
GO_AWAY_MANGLE void Hunk_Clear(void);
GO_AWAY_MANGLE void *Hunk_AllocateTempMemory(uint64_t size);
GO_AWAY_MANGLE void Hunk_FreeTempMemory(void *buffer);
GO_AWAY_MANGLE void Hunk_ClearTempMemory(void);
GO_AWAY_MANGLE uint64_t Hunk_MemoryRemaining(void);
GO_AWAY_MANGLE void Hunk_Log(void);
GO_AWAY_MANGLE void Hunk_SmallLog(void);
GO_AWAY_MANGLE qboolean Hunk_CheckMark(void);
GO_AWAY_MANGLE void Hunk_Print(void);
GO_AWAY_MANGLE void Hunk_Check(void);
GO_AWAY_MANGLE void Hunk_InitMemory(void);
GO_AWAY_MANGLE qboolean Hunk_TempIsClear(void);
GO_AWAY_MANGLE void *Hunk_ReallocateTempMemory(void *ptr, uint64_t nsize);

GO_AWAY_MANGLE uint64_t Com_TouchMemory(void);

#ifdef _NOMAD_DEBUG
extern "C" void *Z_MallocDebug(uint32_t size, int tag, void *user, const char *name, const char *func, const char *file, uint32_t line);
extern "C" void *Z_CallocDebug(uint32_t size, int tag, void *user, const char *name, const char *func, const char *file, uint32_t line);
extern "C" void *Z_ReallocDebug(void *ptr, uint32_t nsize, int tag, void *user, const char *name, const char *func, const char *file, uint32_t line);
extern "C" char *Z_StrdupDebug(const char *str, const char *func, const char *file, uint32_t line);
extern "C" void Z_FreeDebug(void *ptr, const char *func, const char *file, uint32_t line);
extern "C" char *Z_StrdupTagDebug(const char *str, int tag, const char *func, const char *file, uint32_t line);

#define Z_Malloc(size,tag,user,name) Z_MallocDebug((size),(tag),(user),(name),__func__,__FILE__,__LINE__)
#define Z_Calloc(size,tag,user,name) Z_CallocDebug((size),(tag),(user),(name),__func__,__FILE__,__LINE__)
#define Z_Realloc(ptr,nsize,tag,user,name) Z_ReallocDebug((ptr),(nsize),(tag),(user),(name),__func__,__FILE__,__LINE__)
#define Z_Strdup(str) Z_StrdupDebug((str),__func__,__FILE__,__LINE__)
#define Z_Free(ptr) Z_FreeDebug((ptr),__func__,__FILE__,__LINE__)
#define Z_StrdupTag(str,tag) Z_StrdupTagDebug((str),(tag),__func__,__FILE__,__LINE__)
#else
extern "C" void* Z_Malloc(uint32_t size, int tag, void *user, const char *name);
extern "C" void* Z_Calloc(uint32_t size, int tag, void *user, const char *name);
extern "C" void* Z_Realloc(void *ptr, uint32_t nsize, int tag, void *user, const char *name);
extern "C" char* Z_Strdup(const char *str);
extern "C" void Z_Free(void *ptr);
extern "C" char* Z_StrdupTag(const char *str, int tag);
#endif

GO_AWAY_MANGLE void Z_FreeTags(int lowtag, int hightag);
GO_AWAY_MANGLE void Z_ChangeTag(void* user, int tag);
GO_AWAY_MANGLE void Z_ChangeUser(void* newuser, void* olduser);
GO_AWAY_MANGLE void Z_ChangeName(void* user, const char* name);
GO_AWAY_MANGLE void Z_CleanCache(void);
GO_AWAY_MANGLE void Z_CheckHeap(void);
GO_AWAY_MANGLE void Z_ClearZone(void);
GO_AWAY_MANGLE void Z_Print(bool all);
GO_AWAY_MANGLE void Z_Init(void);
GO_AWAY_MANGLE uint64_t Z_FreeMemory(void);
GO_AWAY_MANGLE void* Z_ZoneBegin(void);
GO_AWAY_MANGLE void* Z_ZoneEnd(void);
GO_AWAY_MANGLE uint64_t Z_BlockSize(void *p);
GO_AWAY_MANGLE uint32_t Z_NumBlocks(int tag);
GO_AWAY_MANGLE void Z_TouchMemory(uint64_t *sum);

GO_AWAY_MANGLE void Mem_Info(void);

#ifdef __cplusplus
template<class T>
struct zone_allocator
{
	constexpr zone_allocator(void) noexcept { }
	constexpr zone_allocator(const char* name = "zallocator") noexcept { }

	typedef T value_type;
	template<class U>
	constexpr zone_allocator(const zone_allocator<U> &) noexcept { }

	constexpr GDR_INLINE bool operator!=(const eastl::allocator) { return true; }
	constexpr GDR_INLINE bool operator!=(const zone_allocator) { return false; }
	constexpr GDR_INLINE bool operator==(const eastl::allocator) { return false; }
	constexpr GDR_INLINE bool operator==(const zone_allocator) { return true; }

	GDR_INLINE void* allocate(size_t n) const
	{ return Z_Malloc(n, TAG_STATIC, NULL, "zallocator"); }
	GDR_INLINE void* allocate(size_t& n, size_t& alignment, size_t& offset) const
	{ return Z_Malloc(n, TAG_STATIC, NULL, "zallocator"); }
	GDR_INLINE void* allocate(size_t n, size_t alignment, size_t alignmentOffset, int flags) const
	{ return Z_Malloc(n, TAG_STATIC, NULL, "zallocator"); }
	GDR_INLINE void deallocate(void *p, size_t) const noexcept
	{ Z_Free(p); }
};

class zone_allocator_notemplate
{
public:
	zone_allocator_notemplate(const char* name = "zallocator") noexcept { }

	constexpr zone_allocator_notemplate(const zone_allocator_notemplate &) noexcept { }

	GDR_INLINE bool operator!=(const eastl::allocator) { return true; }
	GDR_INLINE bool operator!=(const zone_allocator_notemplate) { return false; }
	GDR_INLINE bool operator==(const eastl::allocator) { return false; }
	GDR_INLINE bool operator==(const zone_allocator_notemplate) { return true; }

	GDR_INLINE void* allocate(size_t n) const
	{ return Z_Malloc(n, TAG_STATIC, NULL, "zallocator"); }
	GDR_INLINE void* allocate(size_t& n, size_t& alignment, size_t& offset) const
	{ return Z_Malloc(n, TAG_STATIC, NULL, "zallocator"); }
	GDR_INLINE void* allocate(size_t n, size_t alignment, size_t alignmentOffset, int flags) const
	{ return Z_Malloc(n, TAG_STATIC, NULL, "zallocator"); }
	GDR_INLINE void deallocate(void *p, size_t) const noexcept
	{ Z_Free(p); }
};

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

#include <GDRLib/threadpool.hpp>


#endif


#endif
