#include "n_shared.h"

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
#define SMALLZONE_DEFSIZE (50*1024*1024+sizeof(memzone_t))
#define SMALLZONE_MINSIZE (40*1024*1024+sizeof(memzone_t))

#define RETRYAMOUNT (256*1024)

#define MEM_ALIGN		32
#define MIN_FRAGMENT	64

typedef struct memzone_s memzone_t;

// struct members are ordered to optimize for alignment and object size
typedef struct memblock_s
{
	char name[14];
	uint64_t size;
	memzone_t *zone;
	struct memblock_s* next;
    struct memblock_s* prev;
    void **user;
    int tag;
    int id;
} memblock_t;

struct memzone_s
{
	uint64_t size;
    
	// start/end cap for linked list
	memblock_t blocklist;

	// the roving block for general operations
	memblock_t* rover;
};

static memzone_t *mainzone, *smallzone;

static void Z_MergeNB(memblock_t *block);
static void Z_MergePB(memblock_t *block);
static void Z_ScanForBlock(void *start, void *end);
static void Z_Defrag(void);

void* operator new[](size_t size, const char* pName, int flags, unsigned debugFlags, const char* file, int line)
{
    return Mem_Alloc(size);
}
void* operator new[](size_t size, size_t alignment, size_t alignmentOffset, const char* pName, int flags, unsigned debugFlags, const char* file, int line)
{
    return Mem_Alloc(size);
}

static void Z_Print_f(void)
{
	Z_Print(true);
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
	N_strncpy(base->name, "base", 14);
}

byte *Z_InitBase(uint64_t *size, uint64_t default_ram, uint64_t min_ram)
{
	byte *ptr;

	ptr = NULL;
	while (ptr == NULL) {
		// resize the memory requirements until we've got a reasonable amount of RAM
		if (default_ram < min_ram) {
			N_Error("Z_Init: malloc() failed on %lu bytes", min_ram);
		}
		*size = default_ram;

		// try to allocate
		ptr = (byte *)calloc(*size, sizeof(byte));
		
		if (ptr == NULL) {
			Con_Printf(DEV, "malloc() failed on %lu bytes, retrying with %lu bytes", default_ram, default_ram - RETRYAMOUNT);
			default_ram -= RETRYAMOUNT;
		}
	}
	return ptr;
}

void Z_Init(void)
{
	uint64_t mainzone_size;
	uint64_t smallzone_size;

	cvar_t *z_minRam, *z_zoneMegs;

	z_minRam = Cvar_Find("z_minRam");
	z_zoneMegs = Cvar_Find("z_zoneMegs");

	mainzone_size = z_zoneMegs->i * 1024 * 1024 + sizeof(memzone_t);
	mainzone = (memzone_t *)Z_InitBase(&mainzone_size, z_zoneMegs->i * 1024 * 1024 + sizeof(memzone_t),
		z_minRam->i * 1024 * 1024 + sizeof(memzone_t));

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


void Z_MergePB(memblock_t* block)
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

void Z_MergeNB(memblock_t* block)
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

void Z_Defrag(void)
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

void Z_Free(void *ptr)
{
	memblock_t* other;
	memblock_t* block;
	
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
	N_strncpy(block->name, "freed", 14);
	
#ifdef _NOMAD_DEBUG
	memset(ptr, 0, block->size - sizeof(memblock_t));
	Z_ScanForBlock(block->zone, ptr, (byte *)ptr + block->size - sizeof(memblock_t));
#endif


	Z_MergePB(block);
	Z_MergeNB(block);
}

static void *Z_MainAlloc(uint64_t size, int tag, void *user, const char *name)
{
	memblock_t *rover;
	memblock_t *newblock;
	memblock_t *base;
	memblock_t *start;
	uint64_t extra;
	static qboolean tryagain;

	tryagain = qfalse;

	base = mainzone->rover;

	// checking behind rover
	if (base->prev->tag == TAG_FREE)
		base = base->prev;
	
	rover = base;
	start = base->prev;

	do {
		if (rover == start) {
			Con_Printf(WARNING, "zone size wasn't big enough for Z_MainAlloc size given, clearing cache");
			// clean all the caches
			Z_FreeTags(TAG_PURGELEVEL, TAG_CACHE);
			if (tryagain)
				N_Error("Z_SmallAlloc: failed allocation of %lu bytes because zone wasn't big enough", size);
			else {
				tryagain = qtrue;
				return Z_MainAlloc(size, tag, user, name);
			}
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
	
	void *retn = (void *)( (byte *)base + sizeof(memblock_t) );
	
	if (base->user)
		*base->user = retn;

	// next allocation will start looking here
	mainzone->rover = base->next;
	base->id = ZONEID;
	base->zone = mainzone;
	
    N_strncpy(base->name, name, 14);
#ifdef _NOMAD_DEBUG
	Z_CheckHeap();
#endif

    return retn;
}

static void *Z_SmallAlloc(uint64_t size, int tag, void *user, const char *name)
{
	memblock_t *rover;
	memblock_t *newblock;
	memblock_t *base;
	memblock_t *start;
	uint64_t extra;

	base = smallzone->rover;

	// checking behind rover
	if (base->prev->tag == TAG_FREE)
		base = base->prev;
	
	rover = base;
	start = base->prev;

	do {
		if (rover == start) {
			Con_Printf(WARNING, "zone size wasn't big enough for Z_SmallAlloc size given, clearing cache");
			Z_FreeTags(TAG_PURGELEVEL, TAG_CACHE);
			return Z_MainAlloc(size, tag, user, name); // if the smallzone fails, just use the mainzone
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
	
	void *retn = (void *)( (byte *)base + sizeof(memblock_t) );
	
	if (base->user)
		*base->user = retn;

	// next allocation will start looking here
	smallzone->rover = base->next;
	base->id = ZONEID;
	base->zone = smallzone;
	
    N_strncpy(base->name, name, 14);
#ifdef _NOMAD_DEBUG
	Z_CheckHeap();
#endif

    return retn;
}


// Z_Malloc: garbage collection and zone block allocater that returns a block of free memory
// from within the zone without calling malloc
void* Z_Malloc(uint64_t size, int tag, void *user, const char *name)
{
#ifdef _NOMAD_DEBUG
	Z_CheckHeap();
#endif
	if (tag >= TAG_PURGELEVEL && !user && (!N_stricmpn("zalloc", name, 7))) // not a smart pointer
		N_Error("Z_Malloc: an owner is required for purgable blocks, name: %s", name);
	if (!size)
		N_Error("Z_Malloc: bad size, name: %s", name);
	
	size += sizeof(memzone_t);
	size = (size + (MEM_ALIGN - 1)) & ~(MEM_ALIGN - 1);
	if (size < 1024) // small enough for the smallzone
		return Z_SmallAlloc(size, tag, user, name);
	
	return Z_MainAlloc(size, tag, user, name);
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

void Z_ChangeName(void *ptr, const char* name)
{
	memblock_t* block;
	
	block = (memblock_t *)((byte *)ptr - sizeof(memblock_t));
	if (block->id != ZONEID)
		N_Error("Z_ChangeName: block id wasn't zoneid");
	
	N_strncpy(block->name, name, sizeof(block->name) - 1);
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
	const double s = totalMemory / 100.0f;

	Con_Printf("\n<---- Zone Allocation Daemon Heap Report ---->");
	Con_Printf("          : %8lu total zone size", mainzone->size);
	Con_Printf("-------------------------");
	Con_Printf("-------------------------");
	Con_Printf("          : %8lu REMAINING", mainzone->size - static_mem - cached_mem - purgable_mem - audio_mem);
	Con_Printf("-------------------------");
	Con_Printf("(PERCENTAGES)");
	Con_Printf(
			"%9lu   %3.02f%%    static\n"
			"%9lu   %3.02f%%    cached\n"
			"%9lu   %3.02f%%    audio\n"
			"%9lu   %3.02f%%    purgable\n"
			"%9lu   %3.02f%%    free",
		static_mem, static_mem*s,
		cached_mem, cached_mem*s,
		audio_mem, audio_mem*s,
		purgable_mem, purgable_mem*s,
		free_mem, free_mem*s);
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
		
		memcpy(name, block->name, 14);
		if (all)
			Con_Printf("0x%8p : %8lu  %8s", (void *)block, block->size, name);
	
		if (block->next == &mainzone->blocklist) {
			Con_Printf("          : %8lu (TOTAL)", sum);
			count = 0;
			sum = 0;
		}
	}
	Con_Printf("-------------------------");
	Con_Printf("%8lu total blocks\n\n", totalblocks);
}

void* Z_Realloc(uint64_t nsize, int tag, void *user, void *ptr, const char *name)
{
#ifdef _NOMAD_DEBUG
	Z_CheckHeap();
#endif
	void *p = Z_Malloc(nsize, tag, user, name);
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

void* Z_Calloc(uint64_t size, int tag, void *user, const char *name)
{
#ifdef _NOMAD_DEBUG
	Z_CheckHeap();
#endif
	return memset(Z_Malloc(size, tag, user, name), 0, size);
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
	Con_Printf(DEV, "Running heap check");
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
	Con_Printf(DEV, "Done with heap check");
}