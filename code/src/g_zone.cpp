//----------------------------------------------------------
//
// Copyright (C) SIGAAMDAD 2022-2023
//
// This source is available for distribution and/or modification
// only under the terms of the SACE Source Code License as
// published by SIGAAMDAD. All rights reserved
//
// The source is distributed in the hope that it will be
// useful, but WITHOUT ANY WARRANTY; without even the implied
// warranty of FITNESS FOR A PARTICLAR PURPOSE. See the SACE
// Source Code License for more details. If you, however do not
// want to use the SACE Source Code License, then you must use
// this source as if it were to be licensed under the GNU General
// Public License (GPL) version 2.0 or later as published by the
// Free Software Foundation.
//
// DESCRIPTION:
//  src/g_zone.cpp
//  zone memory allocation, to prevent leaks from happening,
//  and to keep better track of allocations. It speeds stuff up
//  by allocating all the memory at the beginning, then returning
//  blocks of memory of desired size, no mallocs, callocs, or
//  reallocs (stdlib.h function calls that is) during the main
//  level loop (unless we're allocating with the reserved zone).
//----------------------------------------------------------

//
// If the program calls Z_Free on a block that doesn't have the ZONEID,
// meaning that it wasn't allocated via Z_Malloc, the allocater will
// throw an error
//

//
// When archiving and unarchiving save data, the allocater will scan the heap
// to check for heap corruption
//

//
// This allocater is a heavily modified version of z_zone.c and z_zone.h from
// varios DOOM source ports, credits to them and John Carmack/Romero for developing
// the system
//

#include "n_shared.h"
#include "g_zone.h"

void* operator new[](size_t size, const char* pName, int flags, unsigned debugFlags, const char* file, int line)
{
	Con_Printf(
		"EASTL::new allocation ->\n"
		"   size: {}\n"
		"   name: {}\n"
		"   file: {}\n"
		"   line: {}\n",
	size, pName, file, line);
	return ::operator new[](size);
}
void* operator new[](size_t size, size_t alignment, size_t alignmentOffset, const char* pName, int flags, unsigned debugFlags, const char* file, int line)
{
	Con_Printf(
		"EASTL::new aligned allocation ->\n"
		"   size: {}\n"
		"   alignment: {}\n"
		"   name: {}\n"
		"   file: {}\n"
		"   line: {}\n",
	size, alignment, pName, file, line);
	return ::operator new[](size, std::align_val_t(alignment));
}

#define UNOWNED    ((void *)666)
#define ZONEID     0xa21d49

#define CHUNK_SIZE 32
#define ZONE_HISTORY 10
#define MIN_FRAGMENT 256

#define DEFAULT_SIZE (900*1024*1024) // 990 MiB
#define MIN_SIZE     (500*1024*1024) // 500 MiB

// tunables
#define ZONEDEBUG
#define ZONEIDCHECK
#define CHECKHEAP

#define MEM_ALIGN  16
#define RETRY_AMOUNT (256*1024)

static int indexer = 0;

#ifdef __GNUC__
#define ZONE_PACK(x) x __attribute__((packed))
#elif defined(_MSVC_VER)
#define ZONE_PACK(x) __pragma(push(pack,1)) x __pragma(pop)
#endif

typedef struct memblock_s
{
	char name[15];
	size_t size;
	void **user;
#ifdef ZONEIDCHECK
	unsigned id;
#endif
	struct memblock_s* next;
	struct memblock_s* prev;
	int tag;
} __attribute__((packed)) memblock_t;

typedef struct memzone_s
{
	// start/end cap for linked list
	memblock_t blocklist;

	size_t size;
	
	// the roving block for general operations
	memblock_t* rover;
} __attribute__((packed)) memzone_t;

static size_t numzones = 0;
static memzone_t** memzones;
static memzone_t* mainzone; // the currently active zone
static memblock_t* tmp; // a lot like Quake's Hunk_TempAlloc, but has dynamic size of whatever's left at the top of the zone, min size, however, of 1024 bytes


static size_t active_memory = 0;
static size_t free_memory = 0;
static size_t inactive_memory = 0;
static size_t purgable_memory = 0;

void* Z_ZoneBegin(void)
{
	return (void *)mainzone;
}

void* Z_ZoneEnd(void)
{
	return (void *)(mainzone+sizeof(memzone_t)+mainzone->size);
}

#define TMP_BLOCK_MAX_SIZE (64*1024*1024)
#define TMP_BLOCK_MIN_SIZE (1*1024)

void* Z_AllocTemp(size_t size, int tag, void* user, const char *name)
{
}


byte *I_ZoneMemory(size_t *size)
{
	byte *ptr;
	int p;
	size_t current_size = DEFAULT_SIZE;
	const size_t min_size = MIN_SIZE;
    p = I_GetParm("-ram");
	if (p != -1) {
	}
	ptr = (byte *)malloc_aligned(16, current_size);
	while (ptr == NULL) {
		if (current_size < min_size) {
			N_Error("Z_Init: failed to allocate zone memory of %li bytes (aligned_alloc, 16)", current_size);
		}
		ptr = (byte *)malloc_aligned(16, current_size);
		if (ptr == NULL) {
			current_size -= RETRY_AMOUNT;
		}
	}

    return ptr;
}

void Z_Init()
{
	srand(time(NULL));
	memblock_t* base;
	size_t size = DEFAULT_SIZE;
	numzones = 1;
	memzones = (memzone_t **)malloc_aligned(16, sizeof(memzone_t *) * 1); // preallocating memzones
	mainzone = (memzone_t *)I_ZoneMemory(&size);
	if (!mainzone)
		N_Error("Z_Init: memory allocation failed");

	mainzone->size = size;
	
	mainzone->blocklist.next = 
	mainzone->blocklist.prev = 
	base = (memblock_t *)((byte *)mainzone+sizeof(memzone_t));
	
	mainzone->blocklist.user = (void **)mainzone;
	mainzone->blocklist.tag = TAG_STATIC;
	mainzone->blocklist.id = ZONEID;
	mainzone->rover = base;
	
	base->prev = base->next = &mainzone->blocklist;
	base->user = (void **)NULL;
	base->size = mainzone->size - sizeof(memzone_t);
	base->id = ZONEID;
	memset(base->name, 0, 15);
	strncpy(base->name, "base", 14);
	free_memory = mainzone->size;

	printf("Allocated zone daemon from %p to %p, size of %li bytes (%li MiB)\n",
		(void *)mainzone, (void *)((byte *)mainzone+mainzone->size+sizeof(memzone_t)),
		mainzone->size, mainzone->size >> 20);
}

static void Z_MergePB(memblock_t* block)
{
	memblock_t* other;
	Con_Printf("Z_MergePB: merging prev block for block at %p", (void *)block);
	
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
	else
		Con_Printf("Z_MergeFB: prev block not TAG_FREE");
}

static void Z_MergeNB(memblock_t* block)
{
	memblock_t* other;
	Con_Printf("Z_MergeNB: merging next block for block at %p", (void *)block);
	
	other = block->next;
	if (other->tag == TAG_FREE) {
		// merge the free block onto the end
		block->size += other->size;
		block->next = other->next;
		block->next->prev = block;
		
		if (other == mainzone->rover)
			mainzone->rover = block;
	}
	else
		Con_Printf("Z_MergeNB: next block not TAG_FREE");
}

void Z_ScanForBlock(void *start, void *end)
{
	memblock_t *block;
	void **mem;
	size_t i, len, tag;
	
	block = mainzone->blocklist.next;
	
	while (block->next != &mainzone->blocklist) {
		tag = block->tag;
		
		if (tag == TAG_STATIC) {
			// scan for pointers on the assumption the pointers are aligned
			// on word boundaries (word size depending on pointer size)
			mem = (void **)( (byte *)block + sizeof(memblock_t) );
			len = (block->size - sizeof(memblock_t)) / sizeof(void *);
			for (i = 0; i < len; ++i) {
				if (start <= mem[i] && mem[i] <= end) {
					Con_Printf(
						"Z_ScanForBlock: "
						"%p (%p) has dangling pointer into freed block "
						"%p (%p -> %p)",
					(void *)mem, block->name, start, (void *)&mem[i],
					mem[i]);
				}
			}
		}
		block = block->next;
	}
	if (indexer++ > 60) {
		indexer = 0;
		Z_Print(true);
	}
}

void Z_ClearZone(void)
{
	Con_Printf("clearing zone");
	memblock_t*		block;

	// set the entire zone to one free block
	mainzone->blocklist.next =
	mainzone->blocklist.prev =
	block = (memblock_t *)( (byte *)mainzone + sizeof(memzone_t) );
	
	mainzone->blocklist.user = (void **)mainzone;
	mainzone->blocklist.tag = TAG_STATIC;
	mainzone->blocklist.id = ZONEID;
	mainzone->rover = block;
	
	block->prev = block->next = &mainzone->blocklist;
	block->id = ZONEID;
	memset(block->name, 0, 15);
	strncpy(block->name, "base", 14);
	free_memory = mainzone->size;
	active_memory = purgable_memory = 0;
	
	// a free block.
	block->tag = TAG_FREE;
	
	block->size = mainzone->size - sizeof(memzone_t);
}

// counts the number of blocks by the tag
static int Z_NumBlocks(int tag)
{
	memblock_t* block;
	int count = 0;
	for (block = mainzone->blocklist.next; block != &mainzone->blocklist; block = block->next) {
		if (block->tag == tag) {
			++count;
		}
	}
	if (indexer++ > 60) {
		indexer = 0;
		Z_Print(true);
	}
	return count;
}

void Z_Free(void *ptr)
{
	memblock_t* other;
	memblock_t* block;
	
	block = (memblock_t *)((byte *)ptr - sizeof(memblock_t));
	
	if (block->id != ZONEID)
		N_Error("Z_Free: freed a pointer without ZONEID");
	if (block->tag != TAG_FREE && block->user != (void **)NULL) {
		// clear the user's mark
		*block->user = NULL;
	}
	
	free_memory += block->size;
	if (block->tag >= TAG_PURGELEVEL)
	    purgable_memory -= block->size;
	else
	    active_memory -= block->size;
	
	// mark as free
	block->tag = TAG_FREE;
	block->user = (void **)NULL;
	block->id = ZONEID;
	memset(block->name, 0, sizeof(block->name));
	strncpy(block->name, "freed", sizeof(block->name) - 1);
	
#ifdef _NOMAD_DEBUG
	memset(ptr, 0, block->size - sizeof(memblock_t));
	Z_ScanForBlock(ptr, (byte *)ptr + block->size - sizeof(memblock_t));
#endif
	
	Z_MergePB(block);
	Z_MergeNB(block);
	if (indexer++ > 60) {
		indexer = 0;
		Z_Print(true);
	}
}

void* Z_Malloc(size_t size, int tag, void *user)
{ return Z_Malloc(size, tag, user, "unknown"); }

void* Z_AlignedAlloc(size_t alignment, size_t size, int tag, void *user, const char* name)
{
#ifdef CHECKHEAP
	Z_CheckHeap();
#endif
	if (tag >= TAG_PURGELEVEL && !user)
		N_Error("Z_Malloc: an owner is required for purgable blocks, name: %s", name);
	if (size == 0)
		N_Error("Z_Malloc: bad size, name: %s", name);
	
	memblock_t* rover;
	memblock_t* newblock;
	memblock_t* base;
	memblock_t* start;
	size_t extra;
	bool tryagain;
	
	size = (size + alignment - 1) & ~(alignment - 1);
	tryagain = false;
	
	// accounting for header size
	size += sizeof(memblock_t);
	
	base = mainzone->rover;
	
	// checking behind the rover
	if (base->prev->tag == TAG_FREE)
		base = base->prev;

	rover = base;
	start = base->prev;

	do {
		if (rover == start) {
			Con_Printf("zone size wasn't big enough for Z_Malloc size given, clearing cache");
			Z_FreeTags(TAG_PURGELEVEL, TAG_CACHE);
			if (tryagain)
				N_Error("Z_Malloc: failed allocation of %li bytes because zone wasn't big enough", size);
			
			tryagain = true;
		}
		if (rover->tag != TAG_FREE) {
			if (rover->tag < TAG_PURGELEVEL) {
				// hit a block that can't be purged,
				// so move the base past it
				base = rover = rover->next;
			}
			else {
				Con_Printf("rover->tag is >=  TAG_PURGELEVEL, freeing");
				
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
	
	free_memory -= rover->size;
	if (tag >= TAG_PURGELEVEL)
		purgable_memory += rover->size;
	else
		active_memory += rover->size;
	
	extra = base->size - size;
	
	if (extra > MIN_FRAGMENT) {
		newblock = (memblock_t *)((byte *)base + size);
		newblock->size = extra;
		newblock->user = NULL;
		newblock->prev = base;
		newblock->next = base->next;
		newblock->next->prev = newblock;
		
		base->next = newblock;
		base->size = size;
	}
	else if (tag >= TAG_PURGELEVEL)
		purgable_memory += size;
	
	base->user = (void **)user;
	base->tag = tag;
	
	if (tag >= TAG_PURGELEVEL)
	    purgable_memory += size;
	else
	    active_memory += size;
	
	free_memory -= size;
	
	void *retn = (void *)( (byte *)base + sizeof(memblock_t) );
	
	if (base->user)
		*base->user = retn;

	// next allocation will start looking here
	mainzone->rover = base->next;
	base->id = ZONEID;
	
    strncpy(base->name, name, sizeof(base->name) - 1);
	if (indexer++ > 60) {
		indexer = 0;
		Z_Print(true);
	}
#ifdef CHECKHEAP
	Z_CheckHeap();
#endif

    return retn;
}

// Z_Malloc: garbage collection and zone block allocater that returns a block of free memory
// from within the zone without calling malloc
void* Z_Malloc(size_t size, int tag, void *user, const char* name)
{
#ifdef CHECKHEAP
	Z_CheckHeap();
#endif
	if (tag >= TAG_PURGELEVEL && !user)
		N_Error("Z_Malloc: an owner is required for purgable blocks, name: %s", name);
	if (size == 0)
		N_Error("Z_Malloc: bad size, name: %s", name);
	
	memblock_t* rover;
	memblock_t* newblock;
	memblock_t* base;
	memblock_t* start;
	size_t extra;
	bool tryagain;
	
	size = (size + MEM_ALIGN - 1) & ~(MEM_ALIGN - 1);
	tryagain = false;
	
	// accounting for header size
	size += sizeof(memblock_t);
	
	base = mainzone->rover;
	
	// checking behind the rover
	if (base->prev->tag == TAG_FREE)
		base = base->prev;

	rover = base;
	start = base->prev;

	do {
		if (rover == start) {
			Con_Printf("zone size wasn't big enough for Z_Malloc size given, clearing cache");
			Z_FreeTags(TAG_PURGELEVEL, TAG_CACHE);
			if (tryagain)
				N_Error("Z_Malloc: failed allocation of %li bytes because zone wasn't big enough", size);
			
			tryagain = true;
		}
		if (rover->tag != TAG_FREE) {
			if (rover->tag < TAG_PURGELEVEL) {
				// hit a block that can't be purged,
				// so move the base past it
				base = rover = rover->next;
			}
			else {
				Con_Printf("rover->tag is >=  TAG_PURGELEVEL, freeing");
				
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
	
	free_memory -= rover->size;
	if (tag >= TAG_PURGELEVEL)
		purgable_memory += rover->size;
	else
		active_memory += rover->size;
	
	extra = base->size - size;
	
	if (extra > MIN_FRAGMENT) {
		newblock = (memblock_t *)((byte *)base + size);
		newblock->size = extra;
		newblock->user = NULL;
		newblock->prev = base;
		newblock->next = base->next;
		newblock->next->prev = newblock;
		
		base->next = newblock;
		base->size = size;
	}
	else if (tag >= TAG_PURGELEVEL)
		purgable_memory += size;
	
	base->user = (void **)user;
	base->tag = tag;
	
	if (tag >= TAG_PURGELEVEL)
	    purgable_memory += size;
	else
	    active_memory += size;
	
	free_memory -= size;
	
	void *retn = (void *)( (byte *)base + sizeof(memblock_t) );
	
	if (base->user)
		*base->user = retn;

	// next allocation will start looking here
	mainzone->rover = base->next;
	base->id = ZONEID;
	
    strncpy(base->name, name, sizeof(base->name) - 1);
	if (indexer++ > 60) {
		indexer = 0;
		Z_Print(true);
	}
#ifdef CHECKHEAP
	Z_CheckHeap();
#endif

    return retn;
}

void Z_ChangeTag(void *user, int tag)
{
	// sanity
	if (!user)
		N_Error("Z_ChangeTag: user is NULL");
	if (tag >= TAG_PURGELEVEL && !user)
		N_Error("Z_ChangeTag: user is required for purgable blocks");
	if (!tag)
		N_Error("Z_ChangeTag: invalid tag");
	
	memblock_t* block;
	
	block = (memblock_t *)((byte *)user - sizeof(memblock_t));
	if (block->id != ZONEID)
		N_Error("Z_ChangeTag: block id isn't ZONEID");
	
	block->tag = tag;
	if (indexer++ > 60) {
		indexer = 0;
		Z_Print(true);
	}
}

void Z_ChangeName(void *ptr, const char* name)
{
	memblock_t* block;
	
	block = (memblock_t *)((byte *)ptr - sizeof(memblock_t));
	if (block->id != ZONEID)
		N_Error("Z_ChangeName: block id wasn't zoneid");
	
	strncpy(block->name, name, sizeof(block->name) - 1);
	if (indexer++ > 60) {
		indexer = 0;
		Z_Print(true);
	}
}

void Z_ChangeUser(void *ptr, void *user)
{
	memblock_t* block;
	
	block = (memblock_t *)((byte *)user - sizeof(memblock_t));
	if (block->id != ZONEID)
		N_Error("Z_ChangeUser: block id isn't ZONEID");
	
	*block->user = ptr;
	if (indexer++ > 60) {
		indexer = 0;
		Z_Print(true);
	}
}

int Z_FreeMemory(void)
{
	memblock_t* block;
	int memory;
	
	memory = 0;
	for (block = mainzone->blocklist.next; block != &mainzone->blocklist; block = block->next) {
		if (block->tag == TAG_FREE || block->tag >= TAG_PURGELEVEL)
			memory += block->size;
	}
	if (indexer++ > 60) {
		indexer = 0;
		Z_Print(true);
	}
	return memory;
}

void Z_FreeTags(int lowtag, int hightag)
{
	memblock_t* block;
	memblock_t* next;
	
	for (block = mainzone->blocklist.next; block != &mainzone->blocklist; block = block->next) {
		next = block->next;
		
		if (block->tag == TAG_FREE)
			continue;
		if (block->tag >= lowtag && block->tag <= hightag)
			Z_Free((byte *)block+sizeof(memblock_t));
	}
	if (indexer++ > 60) {
		indexer = 0;
		Z_Print(true);
	}
}

void Z_Print(bool all)
{
	memblock_t* block;
	memblock_t* next;
	size_t count, sum;
	size_t totalblocks;
	char name[15];
	
	name[14] = 0;
	count = 0;
	sum = 0;
	totalblocks = 0;
	
	size_t total_memory = active_memory + purgable_memory + free_memory;
	double s = 100.0f / total_memory;
	
	printf("          : %8li total zone size\n", mainzone->size);
	printf("-------------------------\n");
	printf("-------------------------\n");
	printf("          : %8li REMAINING\n", mainzone->size - active_memory - purgable_memory);
	printf("-------------------------\n");
	printf("(PERCENTAGES)\n");
	printf(
			"%8li   %6.02f%%   static\n"
			"%8li   %6.02f%%   purgable\n"
			"%8li   %6.02f%%   free\n",
	active_memory, active_memory*s,
	purgable_memory, purgable_memory*s,
	free_memory, free_memory*s);
	printf("-------------------------\n");
	
	size_t blockcount[NUMTAGS] = {0};
	for (block = mainzone->blocklist.next; block != &mainzone->blocklist; block = block->next)
		++blockcount[block->tag];
	
	printf("total purgable blocks: %li\n", blockcount[TAG_PURGELEVEL]);
	printf("total cache blocks:    %li\n", blockcount[TAG_CACHE]);
	printf("total free blocks:     %li\n", blockcount[TAG_FREE]);
	printf("total static blocks:   %li\n", blockcount[TAG_STATIC]);
	printf("total level blocks:    %li\n", blockcount[TAG_LEVEL]);
	printf("-------------------------\n");
	
	for (block = mainzone->blocklist.next;; block = block->next) {
		if (block == &mainzone->blocklist)
	        break;
		
		count++;
		totalblocks++;
		sum += block->size;
		
		memcpy(name, block->name, 14);
		if (all)
			printf("%8p : %8li %8s\n", (void *)block, block->size, name);
		
		if (block->next == &mainzone->blocklist) {
			printf("          : %8li %8s (TOTAL)\n", sum, name);
			count = 0;
			sum = 0;
		}
	}
	printf("-------------------------");
	printf("%8li total blocks\n", totalblocks);
}

void* Z_Realloc(void* ptr, size_t nsize, void* user, int tag, const char* name)
{
#ifdef CHECKHEAP
	Z_CheckHeap();
#endif
	void *p = Z_Malloc(nsize, tag, user, name);
	if (ptr) {
		memblock_t* block = (memblock_t *)((byte *)ptr - sizeof(memblock_t));
		memcpy(p, ptr, nsize <= block->size ? nsize : block->size);
		
		Z_Free(ptr);
		block->user = (void **)user;
		if (block->user)
			*block->user = p;
	}
	if (indexer++ > 60) {
		indexer = 0;
		Z_Print(true);
	}
	return p;
}

void* Z_Calloc(void *user, size_t nelem, size_t elemsize, int tag, const char* name)
{
#ifdef CHECKHEAP
	Z_CheckHeap();
#endif
	if (indexer++ > 60) {
		indexer = 0;
		Z_Print(true);
	}
	return memset(Z_Malloc(nelem * elemsize, tag, user, name), 0, nelem * elemsize);
}

// cleans all zone caches (only blocks from scope to free to unused)
void Z_CleanCache(void)
{
	memblock_t* block;
	Con_Printf("performing garbage collection of zone");
	
	for (block = mainzone->blocklist.next; block != &mainzone->blocklist; block = block->next) {
		if (block->id != ZONEID) {
			N_Error("Z_CleanCache: block id wasn't ZONEID");
		}
		if (block->next->prev != block) {
			N_Error("Z_CleanCache: next block doesn't have proper back linkage");
		}
		if (block->tag == TAG_FREE && block->next->tag == TAG_FREE) {
			Con_Printf("Z_CleanCache: two free blocks in a row, merging");
			Z_MergeNB(block);
			Z_MergePB(block);
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
	if (indexer++ > 60) {
		indexer = 0;
		Z_Print(true);
	}
}

void Z_CheckHeap(void)
{
	memblock_t* block;
//	Con_Printf("running heap check");
	for (block = mainzone->blocklist.next;; block = block->next) {
		if (block->next == &mainzone->blocklist) {
			// all blocks have been hit
			break;
		}
		if (block->id != ZONEID) {
			N_Error("Z_CheckHeap: block id wasn't ZONEID, name: %s", block->name);
		}
		if (block->next->prev != block) {
			N_Error("Z_CheckHeap: next block doesn't have proper back linkage, name: %s, back linked name: %s", block->name, block->next->name);
		}
		if (block->tag == TAG_FREE && block->next->tag == TAG_FREE) {
			Con_Printf("Z_CleanCache: two free blocks in a row, merging");
			Z_MergeNB(block);
			Z_MergePB(block);
		}
	}
	if (indexer++ > 60) {
		indexer = 0;
		Z_Print(true);
	}
//	Con_Printf("done with heap check");
}