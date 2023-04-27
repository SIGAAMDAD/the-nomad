#include "n_shared.h"
#include "g_game.h"

#define ZONEID 0x49d21a
#define ZONE_HISTORY 10
#define MEM_ALIGN sizeof(void *)

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

#ifdef ZONEDEBUG

static int indexer = 0;

enum {malloc_history, free_history, realloc_history, calloc_history, NUM_HISTORY_TYPES};

struct zone_allocation_info
{
	const char *file_history[NUM_HISTORY_TYPES][ZONE_HISTORY];
	const char *func_history[NUM_HISTORY_TYPES][ZONE_HISTORY];
	unsigned line_history[NUM_HISTORY_TYPES][ZONE_HISTORY];
	int history_index[NUM_HISTORY_TYPES];
	const char *desc[NUM_HISTORY_TYPES] = {"malloc()'s", "free()'s", "realloc()'s", "calloc()'s"};
};

static struct zone_allocation_info alloc_info;
static size_t free_memory = 0;
static size_t active_memory = 0;
static size_t purgable_memory = 0;

void Z_DumpHistory(char *history)
{
	int i, j;
	char s[1024];
	
	for (i = 0; i < NUM_HISTORY_TYPES; ++i) {
		stbsp_sprintf(s, "\nLast Several %s:\n\n", alloc_info.desc[i]);
		strcat(history, s);
		
		for (j = 0; j < ZONE_HISTORY; ++j) {
			int k = (alloc_info.history_index[i] - j - 1) & (ZONE_HISTORY - 1);
			
			if (alloc_info.file_history[i][k]) {
				stbsp_sprintf(s, "File: %s, Line: %u, Function: %s\n",
					alloc_info.file_history[i][k], alloc_info.line_history[i][k],
					alloc_info.func_history[i][k]);
				
				strcat(history, s);
			}
		}
	}
}

void Z_PrintStats(void)
{
	if (bff_mode)
		return;
	
	uint64_t total_memory = active_memory + free_memory + purgable_memory;
	double s = 100.0f / total_memory;
	
	fprintf(stdout,
		"%-5lu\t%6.01f%%\tstatic\n"
		"%-5lu\t%6.01f%%\tpurgable\n"
		"%-5lu\t%6.01f%%\tfree\n"
		"%-5lu\t%6.01f%%\tfragmentary\n"
		"%-5lu\t%6.01f%%\tvirtual\n"
		"%-5lu\t\ttotal\n",
	active_memory, active_memory * s,
	purgable_memory, purgable_memory * s,
	free_memory, free_memory * s,
	total_memory);
}

#endif

void Z_KillHeap(void)
{
	if (!mainzone) // if an error occurs before the zone is actually allocated
		return;
	
	con.ConPrintf("Z_KillHeap: freeing mainzone pointer of size {}", mainzone->size);
	free(mainzone);
}

static bool Z_InZone(void *ptr)
{
	if (!ptr) {
		LOG_WARN("Z_InZone: ptr was NULL");
		return false;
	}
	// this is an insanely rare bug, but it still happens: something sneaks outside of the zone
	return (ptr > (void *)mainzone) && (ptr < (void *)((byte *)mainzone+mainzone->size+sizeof(memzone_t)));
}

//
// Z_Free
//
void (Z_Free)(void *ptr, const char* file, unsigned line)
{
	if (ptr == NULL) {
		LOG_ERROR("Z_Free pointer given is NULL, aborting.");
		return;
	}
	if (!Z_InZone(ptr)) {
		LOG_ERROR("Z_Free pointer given is not within the zone, aborting.");
		return;
	}
	
#ifdef ZONEDEBUG
#ifdef CHECKHEAP
	Z_CheckHeap();
#endif
	alloc_info.file_history[free_history][alloc_info.history_index[free_history]] = file;
	alloc_info.line_history[free_history][alloc_info.history_index[free_history]++] = line;
	alloc_info.history_index[free_history] &= ZONE_HISTORY - 1;
#endif
	
	memblock_t* other;
	memblock_t* block = (memblock_t *)((byte *)ptr - sizeof(memblock_t));
#ifdef ZONEIDCHECK
	if (block->id != ZONEID) {
		LOG_ERROR("Z_Free trying to free pointer without ZONEID, aborting.");
		return;
	}
#endif
	
	free_memory += block->size;
	
#ifdef ZONEDEBUG
	memset(ptr, ticcount & 0xff, block->size);
#endif
	
#ifdef ZONEDEBUG
	if (block->tag >= TAG_PURGELEVEL)
		purgable_memory -= block->size;
	else
		active_memory -= block->size;
#endif
		
	block->tag = TAG_FREE;
	other = block->prev;
	if (other->tag == TAG_FREE) {
#ifdef ZONEDEBUG
		LOG_INFO("block->prev->tag == TAG_FREE, merging blocks");
#endif
		// merge with previous free block
		other->size += block->size + sizeof(memblock_t);
		other->next = block->next;
		other->next->prev = other;
		if (block == mainzone->rover)
			mainzone->rover = other;
		
		block = other;
#ifdef ZONEDEBUG
		free_memory += sizeof(memblock_t);
#endif
	}
	
	other = block->next;
	if (other->tag == TAG_FREE) {
#ifdef ZONEDEBUG
		LOG_INFO("block->next->tag == TAG_FREE, merging blocks");
#endif
		// merge the next free block onto the end
		block->size += other->size;
		block->next = other->next + sizeof(memblock_t);
		block->next->prev = block;
		
		if (other == mainzone->rover)
			mainzone->rover = block;
		
#ifdef ZONEDEBUG
		free_memory += sizeof(memblock_t);
#endif
	}
#ifdef ZONEDEBUG
	Z_PrintStats();
#endif
}

void Z_DumpHeap()
{
	memblock_t* block;
	fprintf(stdout, "zone size:%ld   location:%p\n", mainzone->size, (void *)mainzone);
	for (block = mainzone->blocklist.next;; block = block->next) {
		fprintf (stdout, "block:%p     size:%7i      tag:%3i\n",
			(void *)block, block->size, block->tag);
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

void Z_FileDumpHeap()
{
	memblock_t* block;
	const char* filename = "Files/debug/heapdump.log";
	filestream fp(filename, "w");
	fprintf(fp.get(), "zone size:%ld   location:%p\n", mainzone->size, (void *)mainzone);
	for (block = mainzone->blocklist.next;; block = block->next) {
		fprintf (fp.get(), "block:%p     size:%7i      tag:%3i\n",
			(void *)block, block->size, block->tag);
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

void Z_ScanForBlock(void *start, void *end)
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

void Z_ClearZone(memzone_t* zone, size_t size)
{
	LOG_TRACE("clearing zone");
	memblock_t*		block;

	mainzone = zone;
	mainzone->size = size;

	free_memory = mainzone->size - sizeof(memzone_t);
	active_memory = 0;
	purgable_memory = 0;

	// set the entire zone to one free block
	mainzone->blocklist.next =
	mainzone->blocklist.prev =
	block = (memblock_t *)( (byte *)mainzone + sizeof(memzone_t) );
	
	mainzone->blocklist.tag = TAG_STATIC;
	mainzone->rover = block;
	
	block->prev = block->next = &mainzone->blocklist;
	
	// a free block.
	block->tag = TAG_FREE;
	
	block->size = mainzone->size - sizeof(memzone_t);
}

#define MIN_FRAGMENT 1024

// Z_Malloc: garbage collection and zone block allocater that returns a block of free memory
// from within the zone without calling malloc
void *Z_Malloc(size_t size, uint8_t tag)
{
#ifdef ZONEDEBUG
#ifdef CHECKHEAP
	Z_CheckHeap();
#endif
//	alloc_info.file_history[malloc_history][alloc_info.history_index[malloc_history]] = file;
//	alloc_info.line_history[malloc_history][alloc_info.history_index[malloc_history]++] = line;
//	alloc_info.history_index[malloc_history] &= ZONE_HISTORY - 1;
#endif
	
	memblock_t* rover;
	memblock_t* newblock;
	memblock_t* base;
	memblock_t* start;
	uint32_t space;
	
	size_t size_orig = size;
	size = (size + MEM_ALIGN - 1) & ~(MEM_ALIGN - 1);
	
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
#if 0
			LOG_WARN("zone size is not big enough for size given to Z_Malloc, resizing zone");
			Z_FreeTags(TAG_PURGELEVEL, TAG_CACHE);
			Z_Init();
			base = mainzone->rover;
			rover = base;
			start = base->prev;
			free_memory += mainzone->size / 2;
#else
			N_Error("Z_Malloc: zone not big enough for allocation of %li bytes", size);
#endif
		}
		if (rover->tag != TAG_FREE) {
			if (rover->tag < TAG_PURGELEVEL) {
				// hit a block that can't be purged,
				// so move the base past it
				base = rover = rover->next;
			}
			else {
				LOG_TRACE("rover->tag is >=  TAG_PURGELEVEL, freeing");
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
	
	space = base->size - size;
	
	if (space >= MIN_FRAGMENT) {
		newblock = (memblock_t *)((byte *)base + size);
		newblock->size = space;
		newblock->prev = base;
		newblock->next = base->next;
		newblock->next->prev = newblock;
		
		base->next = newblock;
		base->size = size;
	}
	
	base->tag = tag;
#ifdef ZONEIDCHECK
	base->id = ZONEID;
#endif
	
	void *retn = (void *)( (byte *)base + sizeof(memblock_t) );
	
	if (tag >= TAG_PURGELEVEL)
		purgable_memory += size_orig;
	else
		active_memory += size_orig;
	free_memory -= base->size;
	
#ifdef ZONEDEBUG
	Z_PrintStats();
#endif
	
	// next allocation will start looking here
	mainzone->rover = base->next;
	
	return retn;
}

void* (Z_Realloc)(void *ptr, uint32_t nsize, uint8_t tag, const char* file, unsigned line)
{
#ifdef ZONEDEBUG
#ifdef CHECKHEAP
	Z_CheckHeap();
#endif
	alloc_info.file_history[realloc_history][alloc_info.history_index[realloc_history]] = file;
	alloc_info.line_history[realloc_history][alloc_info.history_index[realloc_history]++] = line;
	alloc_info.history_index[realloc_history] &= ZONE_HISTORY - 1;
#endif
	void *p = Z_Malloc(nsize, tag);
	if (ptr) {
		memblock_t* block = (memblock_t *)((byte *)ptr - sizeof(memblock_t));
		size_t size = nsize <= block->size ? nsize : block->size;
		memmove(p, ptr, size);
		
		Z_Free(ptr);
	}
#ifdef ZONEDEBUG
	Z_PrintStats();
#endif
	return p;
}

void* (Z_Calloc)(uint32_t nelem, uint32_t elemsize, uint8_t tag, const char* file, unsigned line)
{
#ifdef ZONEDEBUG
#ifdef CHECKHEAP
	Z_CheckHeap();
#endif
	alloc_info.file_history[calloc_history][alloc_info.history_index[calloc_history]] = file;
	alloc_info.line_history[calloc_history][alloc_info.history_index[calloc_history]++] = line;
	alloc_info.history_index[calloc_history] &= ZONE_HISTORY - 1;
	Z_PrintStats();
#endif
	return memset((Z_Malloc)(nelem * elemsize, tag), 0, nelem * elemsize);
}

void Z_FreeTags(uint8_t lowtag, uint8_t hightag)
{
#ifdef CHECKHEAP
	Z_CheckHeap();
#endif
	int32_t numblocks = 0;
	int32_t size = 0;
	memblock_t*	block;
    memblock_t*	next;
	LOG_TRACE("freeing memblocks with lowtag {} to hightag {}", Z_TagToStr(lowtag), Z_TagToStr(hightag));

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
	LOG_TRACE("printing current state of zone after Z_FreeTags(lowtag: {}, hightag: {})", Z_TagToStr(lowtag), Z_TagToStr(hightag));
	Z_PrintStats();
}

// cleans all zone caches (only blocks from scope to free to unused)
void Z_CleanCache(void)
{
	memblock_t* block;
	LOG_TRACE("performing garbage collection of zone");

	for (block = mainzone->blocklist.next;
	block != &mainzone->blocklist; block = block->next) {
		if ((byte *)block+block->size != (byte *)block->next) {
			N_Error("Z_CleanCache: block size doesn't touch next block!");
		}
		if (block->next->prev != block) {
			N_Error("Z_CleanCache: next block doesn't have proper back linkage!");
		}
#ifdef ZONEIDCHECK
		if (block->id != ZONEID) {
			N_Error("Z_CleanCache: block id isn't ZONEID");
		}
#endif
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
	LOG_TRACE("printing current state after garbage collection");
#ifdef ZONEDEBUG
	Z_PrintStats();
#endif
}

void Z_CheckHeap()
{
	memblock_t* block;
	LOG_TRACE("running a heap check");

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
#ifdef ZONEIDCHECK
		if (block->id != ZONEID) {
			N_Error("Z_CheckHeap: block id isn't ZONEID");
		}
#endif
		if (block->next->prev != block) {
			N_Error("Z_CheckHeap: next block doesn't have proper back linkage");
		}
		if (block->tag == TAG_FREE && block->next->tag == TAG_FREE) {
			N_Error("Z_CheckHeap: two consecutive free blocks");
		}
	}
	LOG_TRACE("heap check successful");
	LOG_TRACE("printing current zone state");
	Z_PrintStats();
}

void (Z_ChangeTag)(void *ptr, uint8_t tag, const char *file, uint32_t line)
{
#ifdef CHECKHEAP
    Z_CheckHeap();
#endif
	if (ptr == NULL || file == NULL)
		N_Error("Z_ChangeTag2: nullptr given to ptr or file");
	
    memblock_t*	block;
    block = (memblock_t *) ((byte *)ptr - sizeof(memblock_t));
#ifdef ZONEIDCHECK
    if (block->id != ZONEID)
        LOG_WARN("{}:{}: Z_ChangeTag: block without a ZONEID!",
                file, line);
#endif
	LOG_TRACE("changing tag of ptr {} to {}, old tag was {}", ptr, Z_TagToStr(tag), Z_TagToStr(block->tag));
    block->tag = tag;
}

uint32_t Z_ZoneSize() { return mainzone->size; }