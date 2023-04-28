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

// 29 without id, 33 with id
typedef struct memblock_s
{
#ifdef ZONEIDCHECK
	unsigned id;
#endif
	uint8_t tag;
	uint32_t size;
	void *user;
	
	struct memblock_s* next;
	struct memblock_s* prev;
#ifdef ZONEDEBUG
	const char *file;
	const char *func;
	unsigned line;
#endif
} memblock_t;

typedef struct
{
	// size of the zone, including size of the header
	uint64_t size;
	
	// start/end cap for blocklist
	memblock_t blocklist;
	
	// rover block pointer
	memblock_t *rover;
} memzone_t;

memzone_t* mainzone;

static uint64_t active_memory = 0;
static uint64_t purgable_memory = 0;
static uint64_t free_memory = 0;
static uint64_t total_blocks = 0;
static uint64_t block_stats[NUMTAGS];

static const char* Z_TagToStr(uint8_t tag)
{
	switch (tag) {
	case TAG_FREE: return "TAG_FREE";
	case TAG_STATIC: return "TAG_STATIC";
	case TAG_SCOPE: return "TAG_SCOPE";
	case TAG_PURGELEVEL: return "TAG_PURGELEVEL";
	case TAG_CACHE: return "TAG_CACHE";
	};
	return "No Tag Given";
}

static int indexer = 0;

void Z_Print(bool all)
{
	memblock_t* block, *next;
	size_t count, sum;
	size_t totalblocks;
	char	name[15];

	name[14] = 0;
	count = 0;
	sum = 0;
	totalblocks = 0;
	
	block = mainzone->blocklist.next;

	fprintf(stdout, "          :%8li total zone size\n", mainzone->size);
	fprintf(stdout, "-------------------------\n");
	fprintf(stdout, "-------------------------\n");
	fprintf(stdout, "          :%8li REMAINING\n", mainzone->size - active_memory - purgable_memory);
	fprintf(stdout, "-------------------------\n");

	while (1) {
		if (block == &mainzone->blocklist)
			break;
		
		// run consistancy checks
//		if (block->id != ZONEID)
//			N_Error("Z_Print: block id isn't ZONEID");
		
		next = block->next;
		count++;
		totalblocks++;
		sum += block->size;
		
		// print the single block
//		memcpy(name, block->name, 8);
		if (all)
			fprintf(stdout, "%8p :%8i\n", (void *)block, block->size);
		
		// print the total
		if (next == &mainzone->blocklist) {
			if (!all)
				fprintf(stdout, "          :%8li (TOTAL)\n", sum);
			
			count = 0;
			sum = 0;
		}

		block = next;
	}

	fprintf(stdout, "-------------------------\n");
	fprintf(stdout, "%8li total blocks\n", totalblocks);
	fflush(stdout);
}

extern "C" void Z_KillHeap(void)
{
	if (!mainzone) // if an error occurs before the zone is actually allocated
		return;
	
	con.ConPrintf("Z_KillHeap: freeing mainzone pointer of size {}", mainzone->size);
	free(mainzone);
}

//
// Z_Free
//
extern "C" void Z_Free(void *ptr)
{
#ifdef ZONEDEBUG
	LOG_INFO("freeing up zone-allocated block memory at {}", ptr);
#endif
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
		LOG_WARN("Z_Free trying to free pointer without ZONEID, aborting.");
		return;
	}
#endif
	
	--block_stats[block->tag];
	if (block->tag != TAG_FREE && block->user)
		block->user = (void *)NULL;
	if (block->tag == TAG_PURGELEVEL)
		purgable_memory -= block->size;
	if (block->user > (void *)0x100)
		block->user = (void *)NULL;

	--total_blocks;
	++indexer;

	// mark as free
	block->user = (void *)NULL;
	block->tag = TAG_FREE;
#ifdef ZONEIDCHECK
	block->id = 0;
#endif
	++block_stats[TAG_FREE];
	
	other = block->prev;
	if (other->tag == TAG_FREE) {
#ifdef ZONEDEBUG
		LOG_INFO("block->prev->tag == TAG_FREE, merging blocks");
#endif
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
#ifdef ZONEDEBUG
		LOG_INFO("block->next->tag == TAG_FREE, merging blocks");
#endif
		// merge the next free block onto the end
		block->size += other->size;
		block->next = other->next;
		block->next->prev = block;
		
		if (other == mainzone->rover) {
			mainzone->rover = block;
		}
	}
}

extern "C" void Z_DumpHeap()
{
	memblock_t* block;
	fprintf(stdout, "zone size:%ld   location:%p\n", mainzone->size, (void *)mainzone);
	for (block = mainzone->blocklist.next;; block = block->next) {
		fprintf (stdout, "block:%p     size:%7i     user:%p      tag:%3i\n",
			(void *)block, block->size, block->user, block->tag);
		if (block->next == &mainzone->blocklist) {
			// all blocks have been hit
			break;
		}
		if ((byte *)block+block->size != (byte *)block->next) {
			fprintf(stderr, "ERROR: block size doesn't touch next block!\n");
		}
		if (block->next->prev != block) {
			fprintf(stderr, "ERROR: next block doesn't have proper back linkage!\n");
		}
		if (block->tag == TAG_FREE && block->next->tag == TAG_FREE) {
			fprintf(stderr, "ERROR: two consecutive free blocks!\n");
		}
	}
}

extern "C" void Z_FileDumpHeap()
{
	memblock_t* block;
	const char* filename = "Files/debug/heapdump.log";
	filestream fp(filename, "w");
	fprintf(fp.get(), "zone size:%ld   location:%p\n", mainzone->size, (void *)mainzone);
	for (block = mainzone->blocklist.next;; block = block->next) {
		fprintf (fp.get(), "block:%p     size:%7i     user:%p      tag:%3i\n",
			(void *)block, block->size, block->user, block->tag);
		if (block->next == &mainzone->blocklist) {
			// all blocks have been hit
			break;
		}
		if ((byte *)block+block->size != (byte *)block->next) {
			fprintf(fp.get(), "ERROR: block size doesn't touch next block!\n");
		}
		if (block->next->prev != block) {
			fprintf(fp.get(), "ERROR: next block doesn't have proper back linkage!\n");
		}
		if (block->tag == TAG_FREE && block->next->tag == TAG_FREE) {
			fprintf(fp.get(), "ERROR: two consecutive free blocks!\n");
		}
	}
}

#define DEFAULT_SIZE (800*1024*1024) // 800 MiB
#define MIN_SIZE     (50*1024*1024) // 50 MiB

static bool initialized = false;

byte *I_ZoneMemory(uint64_t *size)
{
	uint64_t current_size = initialized ? mainzone->size << 1 : DEFAULT_SIZE;
	const uint32_t min_size = initialized ? mainzone->size + 1024 : MIN_SIZE;
	byte *ptr = (byte *)malloc(current_size);
	while (ptr == NULL) {
		if (current_size < min_size) {
			N_Error("I_ZoneMemory: failed allocation of zone memory of %ld bytes (malloc())", current_size);
		}
		ptr = (byte *)malloc(current_size);
		if (ptr == NULL) {
			current_size -= RETRY_AMOUNT;
		}
	}
	if (initialized) {
		memmove(ptr, mainzone, mainzone->size);
		free(mainzone);
	}
	*size = current_size;
	return ptr;
}

extern "C" void Z_Init()
{
	srand(time(NULL));
	memblock_t* base;
	uint64_t size;
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
		con.ConPrintf("Z_Init: initializing Zone Allocation Daemon from addresses {} -> {} of size {} MiB",
			(void *)mainzone, (void *)(mainzone+mainzone->size), mainzone->size >> 20);
		free_memory = mainzone->size;
	}
	else
		LOG_INFO("Resizing zone from {} -> {} of new size {} ({} MiB)",
			(void *)(mainzone), (void *)(mainzone+mainzone->size), mainzone->size, mainzone->size >> 20);
	
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
	--block_stats[base->tag];
#ifdef ZONEDEBUG
	LOG_INFO("changing tag of block {} from {} to {}", (void *)base, Z_TagToStr(base->tag), Z_TagToStr(tag));
#endif
	base->tag = tag;
	++block_stats[tag];
}

extern "C" void Z_ClearZone()
{
	LOG_INFO("clearing zone");
	memblock_t*		block;
	
	free_memory = mainzone->size - sizeof(memzone_t);
	active_memory = 0;
	purgable_memory = 0;
	memset(block_stats, 0, sizeof(block_stats));
	block_stats[TAG_FREE] = 1;

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
#ifdef ZONEIDCHECK
	if (tag >= TAG_PURGELEVEL && !user)
		N_Error("Z_Malloc: an owner is required for purgable blocks");
#endif

	if (!size) {
		LOG_WARN("size of 0 given to Z_Malloc with a tag of {}", Z_TagToStr(tag));
		return user ? user = NULL : NULL;
	}
	
	memblock_t* rover;
	memblock_t* newblock;
	memblock_t* base;
	memblock_t* start;
	uint32_t space;
	
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
			free_memory = ((mainzone->size - sizeof(memzone_t)) - active_memory) - purgable_memory;
			
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
				--block_stats[TAG_PURGELEVEL];
				free_memory += rover->size;
				purgable_memory -= rover->size;
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
	if (tag == TAG_PURGELEVEL)
		purgable_memory += rover->size;

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

	++block_stats[tag];
	++total_blocks;
	++indexer;

	if (indexer > 60)
		Z_Print(true);
#ifdef ZONEIDCHECK
	base->id = ZONEID;
#endif
	
	return retn;
}

extern "C" void* Z_Realloc(void *ptr, uint32_t nsize, void *user, uint8_t tag)
{
	void *p = Z_Malloc(nsize, tag, user);
	if (ptr) {
		memblock_t* block = (memblock_t *)((byte *)ptr - sizeof(memblock_t));
		size_t size = nsize <= block->size ? nsize : block->size;
		memmove(p, ptr, size);
		
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
	LOG_INFO("freeing memblocks with lowtag {} to hightag {}", Z_TagToStr(lowtag), Z_TagToStr(hightag));

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
	LOG_INFO("printing current state of zone after Z_FreeTags(lowtag: {}, hightag: {})", Z_TagToStr(lowtag), Z_TagToStr(hightag));
	Z_Print(true);
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
	Z_Print(true);
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
	Z_Print(true);
}

extern "C" void Z_ChangeTag2(void *ptr, uint8_t tag, const char *file, uint32_t line)
{
	if (ptr == NULL || file == NULL)
		N_Error("Z_ChangeTag2: nullptr given to ptr or file");
	
    memblock_t*	block;
    block = (memblock_t *) ((byte *)ptr - sizeof(memblock_t));
#ifdef ZONEIDCHECK
    if (block->id != ZONEID)
        LOG_WARN("{}:{}: Z_ChangeTag: block without a ZONEID!",
                file, line);
#endif
    if (tag >= TAG_PURGELEVEL && !block->user) {
        N_Error("%s: %i: Z_ChangeTag: an owner is required "
                "for purgable blocks", file, line);
    }
	LOG_INFO("changing tag of ptr {} to {}, old tag was {}", ptr, Z_TagToStr(tag), Z_TagToStr(block->tag));
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
	LOG_INFO("changing user of ptr {} to {}, old user was {}", ptr, user, block->user);
	block->user = user;
	user = ptr;
}

extern "C" uint32_t Z_ZoneSize() { return mainzone->size; }


#ifdef _WIN32

extern "C" FILE *fmemopen(void *buf, size_t size, const char *mode)
{
	char temppath[MAX_PATH + 1];
  	char tempnam[MAX_PATH + 1];
  	DWORD l;
  	HANDLE fh;
  	FILE *fp;

  	if (strcmp(mode, "r") != 0 && strcmp(mode, "r+") != 0)
   		return 0;
  	l = GetTempPath(MAX_PATH, temppath);
  	if (!l || l >= MAX_PATH)
  		return NULL;
  	if (!GetTempFileName(temppath, "solvtmp", 0, tempnam))
    	return NULL;
  	fh = CreateFile(tempnam, DELETE | GENERIC_READ | GENERIC_WRITE, 0,
    				NULL, CREATE_ALWAYS,
    				FILE_ATTRIBUTE_NORMAL | FILE_FLAG_DELETE_ON_CLOSE,
    				NULL);
	if (fh == INVALID_HANDLE_VALUE)
		return 0;
	fp = _fdopen(_open_osfhandle((intptr_t)fh, 0), "w+b");
	if (!fp) {
    	CloseHandle(fh);
    	return NULL;
    }
	if (buf && size && fwrite(buf, size, 1, fp) != 1) {
		fclose(fp);
		return NULL;
	}
	rewind(fp);
	return fp;
}
extern "C" void *mmap(void *start, size_t size, int32_t prot, int32_t flags, int32_t fd, off_t offset)
{
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
}
extern "C" void munmap(void *addr, size_t size)
{
	UnmapViewOfFile(addr);
}

static void addONode(int o_stream_number, FILE *file, char **buf, size_t *length);
static void delONode(FILE *file);
static int get_o_stream_number(void);
static void setODirName(char *str);
static void setOFileName(char *str, int stream_number);

struct oListNode
{
  int o_stream_number;
  FILE *file;
  char **buf;
  size_t *length;
  struct oListNode *pnext;
};

static struct oListNode *oList = NULL;

static void addONode(
        int o_stream_number,
        FILE *file,
        char **buf,
        size_t *length)
{
  struct oListNode **pcur = &oList;
  struct oListNode *node = (struct oListNode *)calloc(1, sizeof(struct oListNode));

  if(node == NULL)
	N_Error("calloc() failed");

  while((*pcur) && (*pcur)->o_stream_number < o_stream_number)
    pcur = &((*pcur)->pnext);

  node->pnext = *pcur;
  node->o_stream_number = o_stream_number;
  node->file = file;
  node->buf = buf;
  node->length = length;
  (*pcur) = node;
}

static void delONode(FILE *file)
{
  struct oListNode **pcur = &oList;
  struct oListNode *todel;
  char file_name[30];

  while((*pcur) && (*pcur)->file != file)
    pcur = &((*pcur)->pnext);

  todel = (*pcur);
  if(todel == NULL){ //not found
    // WARNING(("Trying to close a simple FILE* with close_memstream()"));
  } else {
    if(EOF == fflush(file))
      abort();
    if((*(todel->length) = ftell(file)) == -1)
      abort();
    if((*(todel->buf) = calloc(1, *(todel->length) + 1)) == NULL)
      abort();
    if(EOF == fseek(file, 0, SEEK_SET))
      abort();
    fread(*(todel->buf), 1, *(todel->length), file);

    fclose(file);
    setOFileName(file_name,todel->o_stream_number);
    if(-1 == remove(file_name))
      abort();

    (*pcur) = todel->pnext;
    free(todel);
  }
}


static int get_o_stream_number(void)
{
  int o_stream_number = 1;
  struct oListNode *cur = oList;

  while(cur && o_stream_number >= cur->o_stream_number){
    o_stream_number++;
        cur = cur->pnext;
  }
  return o_stream_number;
}

static void setODirName(char *str)
{
  stbsp_sprintf(str, "ostr_job_%d", _getpid());
}

static void setOFileName(char *str, int stream_number)
{
  setODirName(str);
  char fname[30];
  memset(fname,0,sizeof(fname));
  stbsp_sprintf(fname,"/o_stream_%d",stream_number);
  strncat(str,fname,29);
}

extern "C" FILE *open_memstream(char **ptr, size_t *sizeloc)
{
  FILE *f;
  char file_name[30];
  int o_stream_number;

  if(oList == NULL){
    setODirName(file_name);
    _mkdir(file_name);
  }

  o_stream_number = get_o_stream_number();
  setOFileName(file_name,o_stream_number);
  f = fopen(file_name,"w+");

  if(!f)
    return NULL;

  addONode(o_stream_number, f, ptr, sizeloc);

  return f;
}

#endif

extern "C" void close_memstream(FILE *f)
{
#ifdef _WIN32
  char file_name[30];
  delONode(f);

  if(oList == NULL){
    setODirName(file_name);
    _rmdir(file_name);
  }
#elif defined(__unix__)
	fclose(f);
#endif
}