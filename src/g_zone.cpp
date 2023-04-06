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

#include "n_shared.h"
#include "g_game.h"

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

#define UNOWNED    ((void *)666)
#define ZONEID     0xa21d49

#ifdef __unix__
#include <sys/mman.h>
#endif

//#ifdef _NOMAD_DEBUG
//#define ZONEIDCHECK
//#define CHECKHEAP
//#define ZONEDEBUG
//#endif

//#if defined(_NOMAD_DEBUG) || defined(_NOMAD_LOGS)
//#define ZONE_HISTORY 4
//#define HEADER_SIZE 33
//#else
//#define HEADER_SIZE 29
//#endif
#undef _NOMAD_DEBUG
#define HEADER_SIZE 29
#define CHUNK_SIZE HEADER_SIZE
#define MEM_ALIGN  8
#define RETRY_AMOUNT (256*1024)

extern "C" void *N_mmap(void *start, size_t size, int32_t prot, int32_t flags, int32_t fd, off_t offset)
{
#ifdef __unix__
	return mmap(start, size, prot, flags, fd, offset);
#elif defined(_WIN32)
	if (prot & ~(PROT_READ | PROT_WRITE | PROT_EXEC))
		return MAP_FAILED;
	if (fd == -1) {
		if (!(flags & MAP_ANON) || offset)
			return MAP_FAILED;
	} else if (flags & MAP_ANON)
		return MAP_FAILED;

	DWORD flProtect;
	if (prot & PROT_WRITE) {
		if (prot & PROT_EXEC)
			flProtect = PAGE_EXECUTE_READWRITE;
		else
			flProtect = PAGE_READWRITE;
	} else if (prot & PROT_EXEC) {
		if (prot & PROT_READ)
			flProtect = PAGE_EXECUTE_READ;
		else if (prot & PROT_EXEC)
			flProtect = PAGE_EXECUTE;
	} else
		flProtect = PAGE_READONLY;

	off_t end = length + offset;
	HANDLE mmap_fd, h;
	if (fd == -1)
		mmap_fd = INVALID_HANDLE_VALUE;
	else
		mmap_fd = (HANDLE)_get_osfhandle(fd);
	h = CreateFileMapping(mmap_fd, NULL, flProtect, DWORD_HI(end), DWORD_LO(end), NULL);
	if (h == NULL)
		return MAP_FAILED;

	DWORD dwDesiredAccess;
	if (prot & PROT_WRITE)
		dwDesiredAccess = FILE_MAP_WRITE;
	else
		dwDesiredAccess = FILE_MAP_READ;
	if (prot & PROT_EXEC)
		dwDesiredAccess |= FILE_MAP_EXECUTE;
	if (flags & MAP_PRIVATE)
		dwDesiredAccess |= FILE_MAP_COPY;
	void *ret = MapViewOfFile(h, dwDesiredAccess, DWORD_HI(offset), DWORD_LO(offset), length);
	if (ret == NULL) {
		CloseHandle(h);
		ret = MAP_FAILED;
	}
	return ret;
#endif
}
extern "C" void N_munmap(void *addr, size_t size)
{
#ifdef __unix__
	munmap(addr, size);
#elif defined(_WIN32)
	UnmapViewOfFile(addr);
#endif
}

// 29 without id, 33 with id
typedef struct memblock_s
{
#if defined(_NOMAD_DEBUG) || defined(_NOMAD_LOGS)
	unsigned id;
#endif
	uint8_t tag;
	uint32_t size;
	void *user;
	
	struct memblock_s* next;
	struct memblock_s* prev;
} memblock_t;

typedef struct
{
	// size of the zone, including size of the header
	uint32_t size;
	
	// start/end cap for blocklist
	memblock_t blocklist;
	
	// rover block pointer
	memblock_t *rover;
} memzone_t;

memzone_t* mainzone;

static uint_fast64_t active_memory = 0;
static uint_fast64_t purgable_memory = 0;
static uint_fast64_t free_memory = 0;

void Z_PrintStats(void)
{
	if  (mainzone->size > 0) {
		uint64_t total_memory = active_memory + free_memory + purgable_memory;

		con.ConPrintf(
			"[Zone Allocation Daemon Log]\n"
			"  -> General <-\n"
			"   total memory         => %lu\n"
			"   free memory          => %lu\n"
			"   purgable memory      => %lu\n"
			"   active memory        => %lu\n",
		total_memory, free_memory, purgable_memory, active_memory);
	}
}

extern "C" void Z_KillHeap(void)
{
	free(mainzone);
}

static const char* Z_TagToStr(uint8_t tag)
{
	switch (tag) {
	case TAG_FREE: return "TAG_FREE";
	case TAG_STATIC: return "TAG_STATIC";
	case TAG_SCOPE: return "TAG_SCOPE";
	case TAG_PURGELEVEL: return "TAG_PURGELEVEL";
	case TAG_CACHE: return "TAG_CACHE";
	};
	return "Unknown tag";
}

//
// Z_Free
//
extern "C" void Z_Free(void *ptr)
{
	LOG_INFO("freeing up zone-allocated block memory at {}", ptr);
	if (ptr == NULL) {
		LOG_WARN("Z_Free pointer given is NULL, aborting.");
		return;
	}
	
	memblock_t* block;
	memblock_t* other;
	
	block = (memblock_t *)((byte *)ptr - sizeof(memblock_t));
	
//#ifdef CHECKHEAP
//	Z_CheckHeap();
//#endif
	free_memory += block->size;
	active_memory -= block->size;
#ifdef ZONEIDCHECK
	if (block->tag >= TAG_PURGELEVEL && !block->user)
		N_Error("Z_Free: an owner is required for purgable blocks");
	if (block->id != ZONEID) {
		Z_WARN("Z_Free trying to free pointer without ZONEID, aborting.");
		return;
	}
#endif
	
	if (block->tag != TAG_FREE && block->user)
		block->user = (void *)NULL;
	if (block->user > (void *)0x100)
		block->user = (void *)NULL;



	// mark as free
	block->user = (void *)NULL;
	block->tag = TAG_FREE;
#ifdef ZONEIDCHECK
	block->id = 0;
#endif
	
	other = block->prev;
	if (other->tag == TAG_FREE) {
		LOG_INFO("block->prev->tag == TAG_FREE, merging blocks");
		// merge with previous free block
		other->size += block->size;
		other->next = block->next;
		other->next->prev = other;
		if (block == mainzone->rover) {
			mainzone->rover = other;
		}
		block = other;
	}

	other = block->next;
	if (other->tag == TAG_FREE) {
		LOG_INFO("block->next->tag == TAG_FREE, merging blocks");
		// merge the next free block onto the end
		block->size += other->size;
		block->next = other->next;
		block->next->prev = block;
		
		if (other == mainzone->rover) {
			mainzone->rover = block;
		}
	}
}

#if 0
extern "C" void Z_FileDumpHeap()
{
	memblock_t* block;
	const char* filename = "Files/debug/heapdump.log";
	filestream fp(filename, "w");
	fprintf(fp.fp, "zone size:%i   location:%p\n", mainzone->size, (void *)mainzone);
	for (block = mainzone->blocklist.next;; block = block->next) {
		fprintf (fp.fp, "block:%p     size:%7i     user:%p      tag:%3i\n",
			block, block->size, block->user, block->tag);
		if (block->next == &mainzone->blocklist) {
			// all blocks have been hit
			break;
		}
		if ((byte *)block+block->size != (byte *)block->next) {
			fprintf(fp.fp, "ERROR: block size doesn't touch next block!\n");
		}
		if (block->next->prev != block) {
			fprintf(fp.fp, "ERROR: next block doesn't have proper back linkage!\n");
		}
		if (!block->user && !block->next->user) {
			fprintf(fp.fp, "ERROR: two consecutive free blocks!\n");
		}
	}
}
#endif

#define DEFAULT_SIZE (68*1024*1024) // 68 MiB
#define MIN_SIZE     (50*1024*1024) // 50 MiB

byte *I_ZoneMemory(int32_t *size)
{
	int32_t current_size = DEFAULT_SIZE;
	const int32_t min_size = MIN_SIZE;
	
	byte *ptr = (byte *)calloc(current_size, 1);
	while (ptr == NULL) {
		if (current_size < min_size) {
			N_Error("I_ZoneMemory: failed allocation of zone memory of %i bytes", current_size);
		}
		
		ptr = (byte *)calloc(current_size, 1);
		if (ptr == NULL) {
			current_size -= RETRY_AMOUNT;
		}
	}
	*size = current_size;
	return ptr;
}

static bool initialized = false;

extern "C" void Z_Init()
{
	memblock_t* base;
	int32_t size;
	mainzone = (memzone_t*)I_ZoneMemory(&size);
	if (!mainzone)
		N_Error("Z_Init: memory allocation failed");
	
	if (!initialized)
		atexit(Z_KillHeap);

	mainzone->size = size;
	
	mainzone->blocklist.next = 
	mainzone->blocklist.prev = 
	base = (memblock_t *)((byte *)mainzone+sizeof(memzone_t));
	
	mainzone->blocklist.user = (void *)mainzone;
	mainzone->blocklist.tag = TAG_STATIC;
	mainzone->rover = base;
	
	base->prev = base->next = &mainzone->blocklist;
	base->user = (void *)NULL;
	base->size = mainzone->size - sizeof(memzone_t);
	if (!initialized) {
		con.ConPrintf("Z_Init: initializing Zone Allocation Daemon from addresses %p -> %p of size %u MiB",
			(void *)mainzone, (void *)(mainzone+mainzone->size), mainzone->size >> 20);
	}
	else
		LOG_INFO("Resizing zone from {0} -> {1}, new size {2}",
			(void *)(mainzone), (void *)(mainzone+mainzone->size), mainzone->size);
	
	initialized = true;
}


extern "C" void Z_ScanForBlock(void *start, void *end)
{
	memblock_t *block;
	void **mem;
	int32_t i, len, tag;
	
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
					LOG_WARN(
						"Z_ScanForBlock: "
						"{0} has dangling pointer into freed block "
						"{1} ({2} -> {3})\n",
					(void *)mem, start, (void *)&mem[i],
					mem[i]);
				}
			}
		}
		block = block->next;
	}
}

extern "C" void Z_ChangeTag(void *ptr, uint8_t tag)
{
	memblock_t* base = (memblock_t *)( (byte *)ptr - sizeof(memblock_t) );
#ifdef ZONEIDCHECK
	if (base->id != ZONEID)
		LOG_WARN("Z_ChangeTag: block {} has invalid zoneid", (void *)base);
#endif
	LOG_INFO("changing tag of block {0} from {1} to {2}", (void *)base, Z_TagToStr(base->tag), Z_TagToStr(tag));
	base->tag = tag;
}

extern "C" void Z_ClearZone()
{
//	LOG_INFO("clearing zone");
	memblock_t*		block;
	
	// set the entire zone to one free block
	mainzone->blocklist.next =
	mainzone->blocklist.prev =
	block = (memblock_t *)( (byte *)mainzone + sizeof(memzone_t) );
	
	mainzone->blocklist.user = (void *)mainzone;
	mainzone->blocklist.tag = TAG_STATIC;
	mainzone->rover = block;
	
	block->prev = block->next = &mainzone->blocklist;
	
	// a free block.
	block->tag = TAG_FREE;
	
	block->size = mainzone->size - sizeof(memzone_t);
}

#define MIN_FRAGMENT 64

// Z_Malloc: garbage collection and zone block allocater that returns a block of free memory
// from within the zone without calling malloc
extern "C" void* Z_Malloc(uint32_t size, uint8_t tag, void* user)
{
	if (!size) {
		LOG_WARN("size of 0 given to Z_Malloc with a tag of {}", Z_TagToStr(tag));
		return user ? user = NULL : NULL;
	}
	LOG_INFO("Z_Malloc called with size of {}, tag of {}, and user at {}", size, Z_TagToStr(tag), user);
	
	memblock_t* rover;
	memblock_t* newblock;
	memblock_t* base;
	memblock_t* start;
	int32_t space;
	
	size = (size + MEM_ALIGN - 1) & ~(MEM_ALIGN - 1);
	
	// accounting for header size
	size += sizeof(memblock_t);
	
	base = mainzone->rover;
	
	// checking behind the rover
	if ((base->prev != NULL) && !base->prev->user)
		base = base->prev;

	rover = base;
	start = base->prev;

//#ifdef CHECKHEAP
//	Z_CheckHeap();
//#endif

	do {
		if (rover == start) {
			LOG_WARN("zone size wasn't big enough for Z_Malloc size given, resizing zone");
			// allocate a new zone twice as big
			Z_Init();
			
			base = mainzone->rover;
			rover = base;
			start = base->prev;
		}
		if (rover->tag != TAG_FREE) {
			if (rover->tag < TAG_PURGELEVEL) {
				// hit a block that can't be purged,
				// so move the base past it
				base = rover = rover->next;
			}
			else {
				LOG_INFO("rover->tag is >=  TAG_PURGELEVEL, freeing");
				free_memory += rover->size;
				active_memory -= rover->size;
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
	active_memory += rover->size;
	space = base->size - size;
	
	if (space > MIN_FRAGMENT) {
		newblock = (memblock_t *)((byte *)base + size);
		newblock->size = space;
		newblock->user = NULL;
		newblock->prev = base;
		newblock->next = base->next;
		newblock->next->prev = newblock;
		
		base->next = newblock;
		base->size = size;
	}
	if (user == NULL && tag >= TAG_PURGELEVEL)
		N_Error("Z_Malloc: an owner is required for purgable blocks");
	else if (tag >= TAG_PURGELEVEL)
		purgable_memory += size;
	
	base->user = user;
	base->tag = tag;
	
	void *retn = (void *)( (byte *)base + sizeof(memblock_t) );
	
	if (base->user)
		base->user = retn;

	// next allocation will start looking here
	mainzone->rover = base->next;
	
#ifdef ZONEIDCHECK
	base->id = ZONEID;
#endif
	
	return retn;
}

extern "C" void* Z_Realloc(void *ptr, uint32_t nsize, void *user, uint8_t tag)
{
	LOG_INFO("Z_Realloc called with nsize of {0}, ptr at {1}, and user at {2}", nsize, ptr, user);
	void *p = Z_Malloc(nsize, tag, user);
	if (ptr) {
		memblock_t* block = (memblock_t *)((byte *)ptr - sizeof(memblock_t));
		N_memcpy(p, ptr, nsize <= block->size ? nsize : block->size);
		Z_Free(ptr);
		if (user)
			user  = p;
	}
	return p;
}

extern "C" void* Z_Calloc(void *user, uint32_t nelem, uint32_t elemsize, uint8_t tag)
{
#ifdef CHECKHEAP
	Z_CheckHeap();
#endif
	return memset((Z_Malloc)(nelem * elemsize, tag, user), 0, nelem * elemsize);
}

extern "C" void Z_FreeTags(uint8_t lowtag, uint8_t hightag)
{
#ifdef CHECKHEAP
	Z_CheckHeap();
#endif
	int32_t numblocks = 0;
	int32_t size = 0;
	memblock_t*	block;
    memblock_t*	next;
	LOG_INFO("freeing memblocks with lowtag {0} to hightag {1}", Z_TagToStr(lowtag), Z_TagToStr(hightag));

    for (block = mainzone->blocklist.next; 
		block != &mainzone->blocklist; block = next) {
		// get link before freeing
		next = block->next;
		
		// free block?
		if (block->tag == TAG_FREE) {
			continue;
		}
		if (block->tag >= lowtag && block->tag <= hightag) {
			++numblocks;
			size += block->size;
			Z_Free ((byte *)block+sizeof(memblock_t));
		}
	}
	LOG_INFO("printing current state of zone after Z_FreeTags(lowtag: {0}, hightag: {1})", Z_TagToStr(lowtag), Z_TagToStr(hightag));
	Z_PrintStats();
//	LOG_FREETAGS(lowtag, hightag, numblocks, size);
}

// cleans all zone caches (only blocks from scope to free to unused)
extern "C" void Z_CleanCache(void)
{
	memblock_t* block;
	LOG_INFO("performing garbage collection of zone");

	for (block = mainzone->blocklist.next;
	block != &mainzone->blocklist; block = block->next) {
		if ((byte *)block+block->size != (byte *)block->next) {
			N_Error("Z_CleanCache: block size doesn't touch next block!");
		}
		if (block->next->prev != block) {
			N_Error("Z_CleanCache: next block doesn't have proper back linkage!");
		}
		if (block->tag == TAG_FREE && block->next->tag == TAG_FREE) {
			N_Error("Z_CleanCache: two consecutive free blocks!");
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
	LOG_INFO("printing current state after garbage collection");
	Z_PrintStats();
}

extern "C" void Z_CheckHeap()
{
	memblock_t* block;
	LOG_INFO("running a heap check");

	for (block = mainzone->blocklist.next;; block = block->next) {
		if (block->next == &mainzone->blocklist) {
			// all blocks have been hit
			break;
		}
		if ((byte *)block+block->size != (byte *)block->next) {
			N_Error("Z_CheckHeap: block size doesn't touch next block");
		}
		if (block->tag == TAG_PURGELEVEL) {
			purgable_memory += block->size;
		}
		if (block->next->prev != block) {
			N_Error("Z_CheckHeap: next block doesn't have proper back linkage");
		}
		if (block->tag == TAG_FREE && block->next->tag == TAG_FREE) {
			N_Error("Z_CheckHeap: two consecutive free blocks");
		}
	}
	LOG_INFO("heap check successful");
	LOG_INFO("printing current zone state");
	Z_PrintStats();
}

extern "C" void Z_ChangeTag2(void *ptr, uint8_t tag, const char *file, uint32_t line)
{
	if (ptr == NULL || file == NULL)
		N_Error("Z_ChangeTag2: nullptr given to ptr or file");
	
    memblock_t*	block;
    block = (memblock_t *) ((byte *)ptr - sizeof(memblock_t));
#ifdef ZONEIDCHECK
    if (block->id != ZONEID)
        LOG_WARN("{0}:{1}: Z_ChangeTag: block without a ZONEID!",
                file, line);
#endif
    if (tag >= TAG_PURGELEVEL && !block->user) {
        N_Error("%s: %i: Z_ChangeTag: an owner is required "
                "for purgable blocks", file, line);
    }
	LOG_INFO("changing tag of ptr {0} to {1}, old tag was {2}", ptr, Z_TagToStr(tag), Z_TagToStr(block->tag));
    block->tag = tag;
}

extern "C" void Z_ChangeUser(void *ptr, void *user)
{
	if (ptr == NULL || user == NULL)
		N_Error("Z_ChangeUser: nullptr given to ptr or user");
	
	memblock_t*	block;
	
	block = (memblock_t *) ((byte *)ptr - sizeof(memblock_t));
#ifdef ZONEIDCHECK
	if (block->id != ZONEID)
		LOG_WARN("Z_ChangeUser: tried to change user for invalid block!");
#endif
	LOG_INFO("changing user of ptr {0} to {1}, old user was {2}", ptr, user, block->user);
	block->user = user;
	user = ptr;
}

extern "C" uint32_t Z_ZoneSize() { return mainzone->size; }