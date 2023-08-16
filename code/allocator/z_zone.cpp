#include "../engine/n_shared.h"

/*
===============================
Zone Allocation Daemon:
meant for level-to-level allocations. Used by allocation callbacks and is cleared every new level. Blocks can be freed and are temporary.
===============================
*/

//
// If the program calls Z_Free on a block that doesn't have the ZONEID,
// meaning that it wasn't allocated via Z_Malloc, the allocater will
// throw an error
//

//
// This allocater is a heavily modified version of z_zone.c and z_zone.h from
// varios DOOM source ports, credits to them and John Carmack/Romero for developing
// the system
//

#define UNOWNED    ((void *)666)
#define ZONEID     0xa21d49

// 100 MiB
#define MAINZONE_DEFSIZE (300*1024*1024+sizeof(memzone_t))
#define MAINZONE_MINSIZE (280*1024*1024+sizeof(memzone_t))
// 40 MiB
#define SMALLZONE_DEFSIZE (80*1024*1024+sizeof(memzone_t))
#define SMALLZONE_MINSIZE (70*1024*1024+sizeof(memzone_t))

#define RETRYAMOUNT (256*1024)

#define MEM_ALIGN		64
#define MIN_FRAGMENT	64

typedef struct memzone_s memzone_t;

#ifdef _NOMAD_DEBUG
#define MAX_HISTORY 64

enum {malloc_history, free_history, NUM_HISTORY_TYPES};

static const char *file_history[NUM_HISTORY_TYPES][MAX_HISTORY];
static const char *func_history[NUM_HISTORY_TYPES][MAX_HISTORY];
static uint32_t line_history[NUM_HISTORY_TYPES][MAX_HISTORY];
static uint32_t history_index[NUM_HISTORY_TYPES];
#endif

// struct members are ordered to optimize for alignment and object size
typedef struct memblock_s
{
	char name[14];					// name of the allocation
	memzone_t *zone;				// pointer to the zone the block belongs to (more than one zones)
	struct memblock_s *next;		// next memblock
    struct memblock_s *prev;		// prev memblock
	void **user;					// memblock user
	uint32_t size;					// size of the block, not including size of the header
    uint32_t tag;					// memblock tag
    uint64_t id;					// should be ZONEID
} memblock_t;

struct memzone_s
{
	// size of the zone in bytes
	uint64_t size;
    
	// start/end cap for linked list
	memblock_t blocklist;

	// the roving block for general operations
	memblock_t* rover;
};

static boost::mutex commonLock;
static memzone_t *mainzone, *smallzone;

static void Z_MergeNB(memblock_t *block);
static void Z_MergePB(memblock_t *block);
static void Z_Defrag(void);

void* operator new[](size_t size, const char* pName, int flags, unsigned debugFlags, const char* file, int line)
{
	return ::operator new[](size);
}
void* operator new[](size_t size, size_t alignment, size_t alignmentOffset, const char* pName, int flags, unsigned debugFlags, const char* file, int line)
{
	return ::operator new[](size, std::align_val_t(alignment));
}

static void Z_Print_f(void)
{
	Z_Print(true);
}

GDR_INLINE const char *Z_TagToString(int tag)
{
	switch (tag) {
	case TAG_FREE: return "TAG_FREE";
	case TAG_STATIC: return "TAG_STATIC";
	case TAG_LEVEL: return "TAG_LEVEL";
	case TAG_RENDERER: return "TAG_RENDERER";
	case TAG_SFX: return "TAG_SFX";
	case TAG_MUSIC: return "TAG_MUSIC";
	case TAG_PURGELEVEL: return "TAG_PURGELEVEL";
	case TAG_CACHE: return "TAG_CACHE";
	case TAG_SEARCH_PATH: return "TAG_SEARCH_PATH";
	case TAG_BFF: return "TAG_BFF";
	case TAG_HUNK: return "TAG_HUNK";
	};
	GDR_ASSERT_MSG(0, "Unkown Tag: %i", tag);
	return "Unkown Tag";
}

void Z_InitZone(memzone_t *zone, uint64_t size)
{
	memblock_t *base;

	zone->blocklist.next =
	zone->blocklist.prev =
	base = (memblock_t *)((byte *)zone + sizeof(memzone_t));

	zone->blocklist.user = (void **)zone;
	zone->blocklist.tag = TAG_STATIC;
	zone->size = size;
	zone->rover = base;

	base->prev = base->next = &zone->blocklist;
	base->size = zone->size - sizeof(memzone_t);

	base->tag = TAG_FREE;
	base->id = ZONEID;
	N_strncpyz(base->name, "base", sizeof(base->name));
}

static byte *Z_InitBase(uint64_t *size, uint64_t default_ram, uint64_t min_ram)
{
	byte *ptr;

	ptr = NULL;
	while (ptr == NULL) {
		// resize the memory requirements until we've got a reasonable amount of RAM
		if (default_ram < min_ram) {
			N_Error("Z_Init: calloc() failed on %lu bytes", min_ram);
		}
		*size = default_ram;

		// try to allocate
		ptr = (byte *)calloc(*size, sizeof(byte));
		
		if (ptr == NULL) {
			Con_Printf(DEV, "calloc() failed on %lu bytes, retrying with %lu bytes", default_ram, default_ram - RETRYAMOUNT);
			default_ram -= RETRYAMOUNT;
		}
	}
	return ptr;
}

uint64_t Z_BlockSize(void *p)
{
	return ((memblock_t *)p - 1)->size;
}

void Z_Shutdown(void)
{
	free(mainzone);
	free(smallzone);
}

void Z_Init(void)
{
	uint64_t mainzone_size;
	uint64_t smallzone_size;

	mainzone_size = MAINZONE_DEFSIZE;
	mainzone = (memzone_t *)Z_InitBase(&mainzone_size, MAINZONE_DEFSIZE, MAINZONE_MINSIZE);

	smallzone_size = SMALLZONE_DEFSIZE;
	smallzone = (memzone_t *)Z_InitBase(&smallzone_size, SMALLZONE_DEFSIZE, SMALLZONE_MINSIZE);

	Z_InitZone(mainzone, mainzone_size);
	Z_InitZone(smallzone, smallzone_size);

	Con_Printf("Z_Init: mainzone initialized from %p -> %p, size is %5.08f MiB",
		(void *)mainzone, (void *)((byte *)mainzone+mainzone->size), (double)mainzone->size / 1024 / 1024);
	Con_Printf("Z_Init: smallzone initialized from %p -> %p, size is %5.08f MiB",
		(void *)smallzone, (void *)((byte *)smallzone+smallzone->size), (double)smallzone->size / 1024 / 1024);

    Cmd_AddCommand("zoneinfo", Z_Print_f);
}

void Z_TouchMemory(uint64_t *sum)
{
    memblock_t *block;
	uint64_t j, i;

	for (block = mainzone->blocklist.next;; block = block->next) {
		if (block->next == &mainzone->blocklist) {
			break; // all blocks have been hit
		}
		if (block->tag != TAG_FREE) {
			j = block->size >> 2;
			for (i = 0; i < j; i += 16) {
				*sum += ((uint32_t *)block)[i];
			}
		}
	}
	for (block = smallzone->blocklist.next;; block = block->next) {
		if (block->next == &smallzone->blocklist) {
			break; // all blocks have been hit
		}
		if (block->tag != TAG_FREE) {
			j = block->size >> 2;
			for (i = 0; i < j; i += 16) {
				*sum += ((uint32_t *)block)[i];
			}
		}
	}
}


void* Z_ZoneBegin(void)
{
	return (void *)mainzone;
}

void* Z_ZoneEnd(void)
{
	return (void *)((char *)mainzone+mainzone->size);
}

typedef struct
{
	const char *zone;
	uint64_t zonesize;
	uint64_t totalBlocks;

	uint64_t freeBytes;
	uint64_t rendererBytes;
	uint64_t filesystemBytes;
	uint64_t cachedBytes;
	uint64_t purgeableBytes;
	uint64_t levelBytes;
	uint64_t otherBytes;
	uint64_t audioBytes;
} zone_stats_t;

static void Z_DisplayStats(const zone_stats_t *stats)
{
	Con_Printf( "%8lu bytes total in %s", stats->zonesize, stats->zone );
	Con_Printf( "%8lu bytes free in %s", stats->freeBytes, stats->zone );
	Con_Printf( "    %8lu bytes in renderer segment", stats->rendererBytes );
	Con_Printf( "    %8lu bytes in filesystem segment", stats->filesystemBytes );
	Con_Printf( "    %8lu bytes in cache segment", stats->cachedBytes );
	Con_Printf( "    %8lu bytes in purgeable segment", stats->purgeableBytes );
	Con_Printf( "    %8lu bytes in level segment", stats->levelBytes );
	Con_Printf( "    %8lu bytes in audio segment", stats->audioBytes );
	Con_Printf( "    %8lu bytes in other", stats->otherBytes );
	Con_Printf( " " );
}

static void Z_GetStats(zone_stats_t *stats, const char *name, const memzone_t *zone)
{
	const memblock_t *block;

	memset(stats, 0, sizeof(*stats));
	stats->zone = name;
	stats->zonesize = zone->size;
	for (block = mainzone->blocklist.next; block != &mainzone->blocklist; block = block->next) {
		stats->totalBlocks++;

		switch (block->tag) {
		case TAG_FREE:
			stats->freeBytes += block->size;
			break;
		case TAG_RENDERER:
			stats->rendererBytes += block->size;
			break;
		case TAG_STATIC: // non-specific stuff
			stats->otherBytes += block->size;
			break;
		case TAG_LEVEL:
			stats->levelBytes += block->size;
			break;
		case TAG_SFX:
		case TAG_MUSIC:
			stats->audioBytes += block->size;
			break;
		case TAG_CACHE:
			stats->cachedBytes += block->size;
			break;
		case TAG_PURGELEVEL:
			stats->purgeableBytes += block->size;
			break;
		case TAG_BFF:
		case TAG_SEARCH_PATH:
			stats->filesystemBytes += block->size;
			break;
		};
	}
}

void Zone_Stats(void)
{
	zone_stats_t stats;

	Z_GetStats(&stats, "mainzone", mainzone);
	Z_DisplayStats(&stats);

	Con_Printf( " " );

	Z_GetStats(&stats, "smallzone", smallzone);
	Z_DisplayStats(&stats);
}

static void Z_MergePB(memblock_t* block)
{
	memblock_t* other;
	other = block->prev;
	if (other->tag == TAG_FREE) {
		// merge with previous free block
		other->size += block->size;
		other->next = block->next;
		other->next->prev = other;
		
		if (block == mainzone->rover)
			mainzone->rover = other;
		
		block = other;
	}
}

static void Z_MergeNB(memblock_t* block)
{
	memblock_t* other;
	
	other = block->next;
	if (other->tag == TAG_FREE) {
		// merge the free block onto the end
		block->size += other->size;
		block->next = other->next;
		block->next->prev = block;
		
		if (other == mainzone->rover)
			mainzone->rover = block;
	}
}

#ifdef USE_MEMSTATIC
/*
reduces the overhead if just calling for '0'-'9'
*/
typedef struct
{
	memblock_t b;
	char mem[2];
} memstatic_t;

#define MEM_STATIC(c) {{ NULL, NULL, PAD(sizeof(memstatic_t), 4), TAG_STATIC, ZONEID}, {c, '\0'}}

static const memstatic_t emptyString = MEM_STATIC('\0');
static const memstatic_t numberString[] = {
	MEM_STATIC( '0' ),
	MEM_STATIC( '1' ),
	MEM_STATIC( '2' ),
	MEM_STATIC( '3' ),
	MEM_STATIC( '4' ),
	MEM_STATIC( '5' ),
	MEM_STATIC( '6' ),
	MEM_STATIC( '7' ),
	MEM_STATIC( '8' ),
	MEM_STATIC( '9' )
};
#endif
/*
Z_Strdup: never write over the memory Z_Strdup returns with the chance
that its from memstatic_t
*/
#ifdef _NOMAD_DEBUG
char *Z_StrdupDebug(const char *str, const char *func, const char *file, uint32_t line)
#else
char *Z_Strdup(const char *str)
#endif
{
	char *s;
	uint64_t len;
#ifdef USE_MEMSTATIC
	if (str[0] == '\0') {
		return ((char *)&emptyString) + sizeof(memblock_t);
	}
	else if (str[0] >= '0' && str[0] <= '9' && str[1] == '\0') {
		return ((char *)&numberString[str[0] - '0']) + sizeof(memblock_t);
	}
#endif
	len = strlen(str) + 1;
#ifdef _NOMAD_DEBUG
	s = (char *)Z_MallocDebug(len, TAG_STATIC, &s, "string", func, file, line);
#else
	s = (char *)Z_Malloc(len, TAG_STATIC, &s, "string");
#endif
	N_strncpyz(s, str, len);
	return s;
}

/*
Z_StrdupTag: just Z_Strdup with a custom tag
*/
#ifdef _NOMAD_DEBUG
char *Z_StrdupTagDebug(const char *str, int tag, const char *func, const char *file, uint32_t line)
#else
char *Z_StrdupTag(const char *str, int tag)
#endif
{
	char *s;
	uint64_t len;
#ifdef USE_MEMSTATIC
	if (str[0] == '\0') {
		return ((char *)&emptyString) + sizeof(memblock_t);
	}
	else if (str[0] >= '0' && str[0] <= '9' && str[1] == '\0') {
		return ((char *)&numberString[str[0] - '0']) + sizeof(memblock_t);
	}
#endif
	len = strlen(str) + 1;
#ifdef _NOMAD_DEBUG
	s = (char *)Z_MallocDebug(len, tag, &s, "string", func, file, line);
#else
	s = (char *)Z_Malloc(len, tag, &s, "string");
#endif
	N_strncpyz(s, str, len);
	return s;
}

static void Z_ScanForBlock(memzone_t *zone, void *start, void *end)
{
	memblock_t *block;
	void **mem;
	size_t i, len, tag;
	
	block = zone->blocklist.next;
	
	while (block->next != &zone->blocklist) {
		tag = block->tag;
		
		if (tag == TAG_STATIC) {
			// scan for pointers on the assumption the pointers are aligned
			// on word boundaries (word size depending on pointer size)
			mem = (void **)( (byte *)block + sizeof(memblock_t) );
			len = (block->size - sizeof(memblock_t)) / sizeof(void *);
			for (i = 0; i < len; ++i) {
				if (start <= mem[i] && mem[i] <= end) {
					Con_Printf(DEV,
						"WARNING: "
						"%p (%s) has dangling pointer into freed block "
						"%p (%s) (%p -> %p)",
					(void *)mem, block->name, start, ((memblock_t *)start)->name, (void *)&mem[i],
					mem[i]);
				}
			}
		}
		block = block->next;
    }
}

void Z_ClearZone(void)
{
	Con_Printf(DEV, "WARNING: clearing zone");

	memblock_t *block;

	// set it all to zero, weed out any bugs
	memset((void *)(mainzone+1), 0, mainzone->size - sizeof(memzone_t));
	memset((void *)(smallzone+1), 0, smallzone->size - sizeof(memzone_t));
	
	// re-init the zones
	Z_InitZone(mainzone, mainzone->size);
	Z_InitZone(smallzone, smallzone->size);
}

// counts the number of blocks by the tag
uint32_t Z_NumBlocks(int tag)
{
	memblock_t* block;
	uint32_t count;
	
	count = 0;
	for (block = mainzone->blocklist.next; block != &mainzone->blocklist; block = block->next) {
		if (block->tag == tag) {
			++count;
		}
	}
	for (block = smallzone->blocklist.next; block != &smallzone->blocklist; block = block->next) {
		if (block->tag == tag) {
			++count;
		}
	}
	return count;
}

// broken, don't use
#ifdef _NOMAD_DEBUG
static void Z_DumpHistory(void)
{
	uint32_t i, j;
	static const char *desc[NUM_HISTORY_TYPES] = {"malloc()'s","free()'s"};

	for (i = 0; i < NUM_HISTORY_TYPES; i++) {
		Con_Printf("Last several %s:\n", desc[i]);

		for (j = 0; j < MAX_HISTORY; j++) {
			uint32_t k = (history_index[i] - j - 1) & (MAX_HISTORY - 1);
			if (file_history[i][k] && func_history[i][k]) {
//				snprintf(buf, sizeof(buf), "Function: %s, File: %s, Line: %i\n",
//					func_history[i][k], file_history[i][k], line_history[i][k]);
				Con_Printf("Function: %s, File: %s, Line: %i",
					func_history[i][k], file_history[i][k], line_history[i][k]);
			}
		}
	}
}
#endif

static void Z_Defrag(void)
{
	memblock_t* block;

    Con_Printf(DEV, "Defragmenting zone");

	for (block = mainzone->blocklist.next;; block = block->next) {
		if (block == &mainzone->blocklist) {
			break;
		}
		if (block->tag == TAG_FREE) {
			Z_MergePB(block);
			Z_MergeNB(block);
		}
	}
	for (block = smallzone->blocklist.next;; block = block->next) {
		if (block == &smallzone->blocklist) {
			break;
		}
		if (block->tag == TAG_FREE) {
			Z_MergePB(block);
			Z_MergeNB(block);
		}
	}
}

#ifdef _NOMAD_DEBUG
void Z_FreeDebug(void *ptr, const char *func, const char *file, uint32_t line)
#else
void Z_Free(void *ptr)
#endif
{
#ifdef _NOMAD_DEBUG
	func_history[free_history][history_index[free_history]] = func;
	file_history[free_history][history_index[free_history]] = file;
	line_history[free_history][history_index[free_history]++] = line;
	history_index[free_history] &= MAX_HISTORY - 1;
#endif

	memblock_t* other;
	memblock_t* block;

	if (!ptr) {
		N_Error("Z_Free: double free or invalid pointer");
	}
	
	block = (memblock_t *)((byte *)ptr - sizeof(memblock_t));
	
	if (block->id != ZONEID)
		N_Error("Z_Free: freed a pointer without ZONEID");
	if (N_stricmpn("zalloc", block->name, 7) && block->tag >= TAG_PURGELEVEL) // its a smart pointer
		block->tag = TAG_FREE;
	else if (block->tag != TAG_FREE && block->user != (void **)NULL) {
		// clear the user's mark
		*block->user = NULL;
	}
	
	// mark as free
	block->tag = TAG_FREE;
	block->user = (void **)NULL;
	block->id = ZONEID;
	N_strncpyz(block->name, "freed", sizeof(block->name));
	
#ifdef _NOMAD_DEBUG
	memset(ptr, 0, block->size - sizeof(memblock_t));
	Z_ScanForBlock(block->zone, ptr, (byte *)ptr + block->size - sizeof(memblock_t));
#endif

	Z_MergePB(block);
	Z_MergeNB(block);
}

#ifdef _NOMAD_DEBUG
static void *Z_MainAllocDebug(uint32_t size, int tag, void *user, const char *name, const char *func, const char *file, uint32_t line)
#else
static void *Z_MainAlloc(uint32_t size, int tag, void *user, const char *name)
#endif
{
	memblock_t *rover;
	memblock_t *newblock;
	memblock_t *base;
	memblock_t *start;
	uint32_t extra;
	void *retn;

	base = mainzone->rover;

	// checking behind rover
	if (base->prev->tag == TAG_FREE)
		base = base->prev;
	
	rover = base;
	start = base->prev;

	do {
		if (rover == start) {
			goto __error;
		}
		if (rover->tag != TAG_FREE) {
			if (rover->tag < TAG_PURGELEVEL) {
				// hit a block that can't be purged,
				// so move the base past it
				base = rover = rover->next;
			}
			else {
				Con_Printf(DEV, "rover->tag is >=  TAG_PURGELEVEL, freeing");

				// reuse the block for the frame, this block is probably being allocated by
				// R_FrameAlloc or something like that
				if (tag == TAG_RENDERER && rover->size >= size)
					break;
				
				// free the rover block (adding to the size of the base)
				// the rover can be the base block
				base = base->prev;
				Z_Free((byte *)rover+sizeof(memblock_t));
				base = base->next;
				rover = base->next;
			}
		}
		else {
			rover = rover->next;
		}
	} while (base->tag != TAG_FREE || base->size < size);
	// old: (base->user || base->size < size)

	extra = base->size - size;
	
	if (extra >= MIN_FRAGMENT) {
		newblock = (memblock_t *)((byte *)base + size);
		newblock->size = extra;
		newblock->user = NULL;
		newblock->prev = base;
		newblock->next = base->next;
		newblock->next->prev = newblock;
		
		base->next = newblock;
		base->size = size;
	}
	
	base->user = (void **)user;
	base->tag = tag;
	
	retn = (void *)( (byte *)base + sizeof(memblock_t) );
	
	if (base->user)
		*base->user = retn;

	// next allocation will start looking here
	mainzone->rover = base->next;
	base->id = ZONEID;
	base->zone = mainzone;

    N_strncpyz(base->name, name, sizeof(base->name));
#ifdef _NOMAD_DEBUG
	Z_CheckHeap();
#endif
    return retn;

__error:
	N_Error("Z_MainAlloc: failed on allocation of %i bytes because mainzone wasn't big enough", size);
}

#ifdef _NOMAD_DEBUG
static void *Z_SmallAllocDebug(uint32_t size, int tag, void *user, const char *name, const char *func,  const char *file, uint32_t line)
#else
static void *Z_SmallAlloc(uint32_t size, int tag, void *user, const char *name)
#endif
{
	memblock_t *rover;
	memblock_t *newblock;
	memblock_t *base;
	memblock_t *start;
	uint32_t extra;
	void *retn;

	base = smallzone->rover;

	// checking behind rover
	if (base->prev->tag == TAG_FREE)
		base = base->prev;
	
	rover = base;
	start = base->prev;

	do {
		if (rover == start) {
			goto __error;
		}
		if (rover->tag != TAG_FREE) {
			if (rover->tag < TAG_PURGELEVEL) {
				// hit a block that can't be purged,
				// so move the base past it
				base = rover = rover->next;
			}
			else {
				Con_Printf(DEV, "rover->tag is >=  TAG_PURGELEVEL, freeing");
				
				// reuse the block for the frame, this block is probably being allocated by
				// R_FrameAlloc or something like that
				if (tag == TAG_RENDERER && rover->size >= size)
					break;
				
				// free the rover block (adding to the size of the base)
				// the rover can be the base block
				base = base->prev;
				Z_Free((byte *)rover+sizeof(memblock_t));
				base = base->next;
				rover = base->next;
			}
		}
		else {
			rover = rover->next;
		}
	} while (base->tag != TAG_FREE || base->size < size);
	// old: (base->user || base->size < size)

	extra = base->size - size;
	
	if (extra >= MIN_FRAGMENT) {
		newblock = (memblock_t *)((byte *)base + size);
		newblock->size = extra;
		newblock->user = NULL;
		newblock->prev = base;
		newblock->next = base->next;
		newblock->next->prev = newblock;
		
		base->next = newblock;
		base->size = size;
	}
	
	base->user = (void **)user;
	base->tag = tag;
	
	retn = (void *)( (byte *)base + sizeof(memblock_t) );
	
	if (base->user)
		*base->user = retn;

	// next allocation will start looking here
	smallzone->rover = base->next;
	base->id = ZONEID;
	base->zone = smallzone;
	
	N_strncpyz(base->name, name, sizeof(base->name));
#ifdef _NOMAD_DEBUG
	Z_CheckHeap();
#endif

    return retn;

__error:
	N_Error("Z_SmallAlloc: failed on %i bytes because smallzone wasn't big enough", size);
}


/*
Z_Malloc: garbage collection and zone block allocater that returns a block of free memory
from within the zone without calling malloc
*/
#ifdef _NOMAD_DEBUG
void *Z_MallocDebug(uint32_t size, int tag, void *user, const char *name, const char *func, const char *file, uint32_t line)
#else
void *Z_Malloc(uint32_t size, int tag, void *user, const char *name)
#endif
{
#ifdef _NOMAD_DEBUG
	Z_CheckHeap();
#endif
	if (tag >= TAG_PURGELEVEL && !user && (!N_stricmpn("zalloc", name, 7))) // not a smart pointer
		N_Error("Z_Malloc: an owner is required for purgable blocks, name: %s", name);
	if (!size)
		N_Error("Z_Malloc: bad size, name: %s", name);
	
#ifdef _NOMAD_DEBUG
	func_history[malloc_history][history_index[malloc_history]] = func;
	file_history[malloc_history][history_index[malloc_history]] = file;
	line_history[malloc_history][history_index[malloc_history]++] = line;
	history_index[malloc_history] &= MAX_HISTORY - 1;
#endif
	
	// round to the cacheline
	size += sizeof(memzone_t);
	size = PAD(size, MEM_ALIGN);
	if (size < 1024) // small enough for the smallzone
#ifdef _NOMAD_DEBUG
		return Z_SmallAllocDebug(size, tag, user, name, func, file, line);
#else
		return Z_SmallAlloc(size, tag, user, name);
#endif

#ifdef _NOMAD_DEBUG
	return Z_MainAllocDebug(size, tag, user, name, func, file, line);
#else
	return Z_MainAlloc(size, tag, user, name);
#endif
}

void Z_ChangeTag(void *user, int tag)
{
	// sanity
	if (!user)
		N_Error("Z_ChangeTag: user is NULL");
	if (!tag || tag >= NUMTAGS)
		N_Error("Z_ChangeTag: invalid tag");
	
	memblock_t* block;
	
	block = (memblock_t *)((byte *)user - sizeof(memblock_t));
	if (block->id != ZONEID)
		N_Error("Z_ChangeTag: block id isn't ZONEID");
	
	block->tag = tag;
}

/*
Z_ChangeName: the name provided should ALWAYS be a string literal
*/
void Z_ChangeName(void *ptr, const char* name)
{
	memblock_t* block;
	
	block = (memblock_t *)((byte *)ptr - sizeof(memblock_t));
	if (block->id != ZONEID)
		N_Error("Z_ChangeName: block id wasn't zoneid");
	
	N_strncpyz(block->name, name, sizeof(block->name));
}

void Z_ChangeUser(void *newuser, void *olduser)
{
	memblock_t* block;

	if (!newuser)
		N_Error("Z_ChangeUser: newuser is NULL");
	if (!olduser)
		N_Error("Z_ChangeUser: olduser is NULL");
	
	block = (memblock_t *)((byte *)olduser - sizeof(memblock_t));
	if (block->id != ZONEID)
		N_Error("Z_ChangeUser: block id isn't ZONEID");
	
	block->user = (void **)newuser;
}

uint64_t Z_FreeMemory(void)
{
	memblock_t* block;
	uint64_t memory;
	
	memory = 0;
	for (block = mainzone->blocklist.next; block != &mainzone->blocklist; block = block->next) {
		if (block->tag == TAG_FREE || block->tag >= TAG_PURGELEVEL)
			memory += block->size;
	}
	return memory;
}

void Z_FreeTags(int lowtag, int hightag)
{
	memblock_t* block;
	memblock_t* next;
	uint64_t totalBytes = 0;
	
    Con_Printf(DEV, "freeing zone blocks from tags %i (lowtag) to %i (hightag)", lowtag, hightag);
	for (block = mainzone->blocklist.next; block != &mainzone->blocklist; block = block->next) {
		next = block->next;
		
		if (block->tag == TAG_FREE) { // avoid frags
			Z_MergeNB(block);
			Z_MergePB(block);
		}
		if (block->tag >= lowtag && block->tag <= hightag) {
			totalBytes += block->size;
			Z_Free((byte *)block+sizeof(memblock_t));
		}
	}
	for (block = smallzone->blocklist.next; block != &smallzone->blocklist; block = block->next) {
		next = block->next;
		
		if (block->tag == TAG_FREE) { // avoid frags
			Z_MergeNB(block);
			Z_MergePB(block);
		}
		if (block->tag >= lowtag && block->tag <= hightag) {
			totalBytes += block->size;
			Z_Free((byte *)block+sizeof(memblock_t));
		}
	}
	Con_Printf("Total bytes freed: %lu", totalBytes);
}

void Z_Print(bool all)
{
	memblock_t *block, *next;
	uint64_t count, sum, totalblocks;
	uint64_t blockcount[NUMTAGS] = {0};
	char name[15];

	totalblocks = 0;
	count = 0;
	sum = 0;
	uint64_t static_mem, purgable_mem, cached_mem, free_mem, audio_mem, total_memory;
	static_mem = purgable_mem = cached_mem = free_mem = audio_mem = total_memory = 0;

	for (block = mainzone->blocklist.next;; block = block->next) {
		if (block == &mainzone->blocklist) {
			break;
		}
		if (block->tag == TAG_STATIC || block->tag == TAG_LEVEL) {
			static_mem += block->size;
		}
		else if (block->tag == TAG_SFX || block->tag == TAG_MUSIC) {
			audio_mem += block->size;
		}
		else if (block->tag == TAG_CACHE) {
			cached_mem += block->size;
		}
		else if (block->tag == TAG_PURGELEVEL) {
			purgable_mem += block->size;
		}
		else if (block->tag == TAG_FREE) {
			free_mem += block->size;
		}
	}
	const uint64_t totalMemory = static_mem + free_mem + cached_mem + audio_mem + purgable_mem;
	const float s = totalMemory / 100.0f;

	Zone_Stats();

	Con_Printf("\n<---- Zone Allocation Daemon Heap Report ---->");
	Con_Printf("          : %8lu total zone size", mainzone->size);
	Con_Printf("-------------------------");
	Con_Printf("-------------------------");
	Con_Printf("          : %8lu REMAINING", mainzone->size - static_mem - cached_mem - purgable_mem - audio_mem);
	Con_Printf("-------------------------");
	Con_Printf("(PERCENTAGES)");
	Con_Printf(
			"%-9lu   %5.03f%%    static\n"
			"%-9lu   %5.03f%%    cached\n"
			"%-9lu   %5.03f%%    audio\n"
			"%-9lu   %5.03f%%    purgable\n"
			"%-9lu   %5.03f%%    free",
		static_mem, (float)(static_mem)*s,
		cached_mem, (float)(cached_mem)*s,
		audio_mem, (float)(audio_mem)*s,
		purgable_mem, (float)(purgable_mem)*s,
		free_mem, (float)(free_mem)*s);
	Con_Printf("-------------------------");

	for (block = mainzone->blocklist.next; block != &mainzone->blocklist; block = block->next)
		++blockcount[block->tag];

	Con_Printf("total purgable blocks: %lu", blockcount[TAG_PURGELEVEL]);
	Con_Printf("total cache blocks:    %lu", blockcount[TAG_CACHE]);
	Con_Printf("total free blocks:     %lu", blockcount[TAG_FREE]);
	Con_Printf("total static blocks:   %lu", blockcount[TAG_STATIC]);
	Con_Printf("total level blocks:    %lu", blockcount[TAG_LEVEL]);
	Con_Printf("-------------------------");
	
	for (block = mainzone->blocklist.next;; block = block->next) {
		if (block == &mainzone->blocklist)
	        break;
		
		count++;
		totalblocks++;
		sum += block->size;
		memcpy(name, block->name, sizeof(block->name));
		name[14] = '\0';
		
		if (all)
			Con_Printf("%8p : %12i %16s %14s", (void *)block, block->size, Z_TagToString(block->tag), name);
	
		if (block->next == &mainzone->blocklist) {
			Con_Printf("          : %8lu (TOTAL)", sum);
			count = 0;
			sum = 0;
		}
	}
	Con_Printf("-------------------------");
	Con_Printf("%8lu total blocks\n\n", totalblocks);
}

#ifdef _NOMAD_DEBUG
void *Z_ReallocDebug(void *ptr, uint32_t nsize, int tag, void *user, const char *name, const char *func, const char *file, uint32_t line)
#else
void *Z_Realloc(void *ptr, uint32_t nsize, int tag, void *user, const char *name)
#endif
{
#ifdef _NOMAD_DEBUG
	Z_CheckHeap();
#endif
#ifdef _NOMAD_DEBUG
	void *p = Z_MallocDebug(nsize, tag, user, name, func, file, line);
#else
	void *p = Z_Malloc(nsize, tag, user, name);
#endif
	if (ptr) {
		memblock_t* block = (memblock_t *)((byte *)ptr - sizeof(memblock_t));
		memcpy(p, ptr, nsize <= block->size ? nsize : block->size);
		
		Z_ChangeTag(ptr, TAG_PURGELEVEL);
		block->user = (void **)user;
		if (block->user)
			*block->user = p;
	}
	return p;
}

#ifdef _NOMAD_DEBUG
void* Z_CallocDebug(uint32_t size, int tag, void *user, const char *name, const char *func, const char *file, uint32_t line)
#else
void* Z_Calloc(uint32_t size, int tag, void *user, const char *name)
#endif
{
#ifdef _NOMAD_DEBUG
	Z_CheckHeap();
#endif
#ifdef _NOMAD_DEBUG
	return memset(Z_MallocDebug(size, tag, user, name, func, file, line), 0, size);
#else
	return memset(Z_Malloc(size, tag, user, name), 0, size);
#endif
}

// cleans all zone caches (only blocks from scope to free to unused)
void Z_CleanCache(void)
{
	memblock_t* block;
	Con_Printf(DEV, "performing garbage collection of zone");
	
	for (block = mainzone->blocklist.next; block != &mainzone->blocklist; block = block->next) {
		if (block->id != ZONEID) {
			N_Error("Z_CleanCache: block id wasn't ZONEID");
		}
		if (block->next->prev != block) {
			N_Error("Z_CleanCache: next block doesn't have proper back linkage");
		}
		if (block->tag == TAG_FREE && block->next->tag == TAG_FREE) {
			N_Error("Z_CleanCache: two free blocks in a row");
		}
		if ((byte *)block+block->size != (byte *)block->next) {
			N_Error("Z_CleanCache: block size doesn't touch next block");
		}
		if (block->tag < TAG_PURGELEVEL) {
			continue;
		}
		else {
			memblock_t* base = block;
			block = block->prev;
			Z_Free((byte*)block+sizeof(memblock_t));
			block = base->next;
		}
	}
}

void Z_CheckHeap(void)
{
	memblock_t* block;
	for (block = mainzone->blocklist.next;; block = block->next) {
		if (block->next == &mainzone->blocklist) {
			// all blocks have been hit
			break;
		}
		if (block->id != ZONEID) {
			N_Error("Z_CheckHeap: block id wasn't ZONEID");
		}
		if (block->next->prev != block) {
			N_Error("Z_CheckHeap: next block doesn't have proper back linkage");
		}
		if (block->tag == TAG_FREE && block->next->tag == TAG_FREE) {
			N_Error("Z_CheckHeap: two free blocks in a row!");
		}
		if ((byte *)block+block->size != (byte *)block->next) {
			N_Error("Z_CleanCache: block size doesn't touch next block");
		}
    }
}