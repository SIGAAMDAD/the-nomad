#include "n_shared.h"

typedef struct
{
	uint32_t id;
	uint64_t size;
	char name[14];
} hunk_t;

typedef struct memblock_s
{
    uint32_t size;
    void **user;
    int32_t tag;
    int32_t id;
    char name[14];
    struct memblock_s* next;
    struct memblock_s* prev;
} memblock_t;

typedef struct memzone_s
{
	uint64_t size;
    
	// start/end cap for linked list
	memblock_t blocklist;

	// the roving block for general operations
	memblock_t* rover;
} memzone_t;

static void* Hunk_AllocHigh(uint64_t size, const char *name);
static void* Hunk_AllocLow(uint64_t size, const char *name);
static void Z_MergeNB(memblock_t *block);
static void Z_MergePB(memblock_t *block);
static void Z_ScanForBlock(void *start, void *end);
static void Z_Defrag(void);

#define UNOWNED    ((void *)666)
#define ZONEID     0xa21d49

#define MIN_FRAGMENT 64

#define DEFAULT_HEAP_SIZE (1700*1024*1024)
#define MIN_HEAP_SIZE (1000*1024*1024)
#define DEFAULT_ZONE_SIZE (400*1024*1024)

static byte *hunk_base;
static uint64_t hunk_size;
static uint64_t hunk_low_used;
static uint64_t hunk_high_used;
static bool hunk_tempactive;
static uint64_t hunk_tempmark;
static memzone_t *mainzone;

#define MEM_ALIGN  16
#define RETRY_AMOUNT (256*1024)

void Memory_Shutdown(void)
{
	Com_Printf("Memory_Shutdown: deallocating allocation daemons");
    free(hunk_base);
    Mem_Shutdown();
}

static void Z_Print_f(void)
{
	Z_Print(true);
}

static void Com_Meminfo_f(void)
{
	uint64_t unused;

	Con_Printf("%8lu bytes total hunk", hunk_size);
	Con_Printf(" ");
	Con_Printf("%8lu low mark", hunk_low_used);
	Con_Printf("%8lu high mark", hunk_high_used);
	Con_Printf("%8s temp active", N_booltostr(hunk_tempactive));
	Con_Printf("%8lu temp mark", hunk_tempmark);
	Con_Printf(" ");
	Con_Printf("%8lu total hunk used", hunk_low_used + hunk_high_used);

	unused = 0;
	unused += hunk_size - hunk_low_used;
	unused += hunk_size - hunk_high_used;

	Con_Printf("%8lu total hunk unused", unused);
}

static uint64_t Com_TouchMemory(void)
{
	const memblock_t *block;
//	uint32_t start, end;
	uint64_t i, j;
	uint64_t sum;

	Z_CheckHeap();

	sum = 0;

	j = hunk_low_used >> 2;
	for (i = 0; i < j; i += 64) { // only need to touch each page
		sum += ((uint32_t *)hunk_base)[i];
	}

	i = (hunk_size - hunk_high_used) >> 2;
	j = hunk_high_used >> 2;
	for (; i < j; i += 64) { // only need to touch each page
		sum += ((uint32_t *)hunk_base)[i];
	}

	for (block = mainzone->blocklist.next;; block = block->next) {
		if (block->next == &mainzone->blocklist) {
			break; // all blocks have been hit
		}
		if (block->tag != TAG_FREE) {
			j = block->size >> 2;
			for (i = 0; i < j; i += 16) {
				sum += ((uint32_t *)block)[i];
			}
		}
	}

	return sum;
}

void Memory_Init(void)
{
	Con_Printf("Memory_Init: initializing allocation daemons");

	Mem_Init();

    hunk_size = 0;
    hunk_low_used = hunk_high_used = hunk_tempmark = 0;
    hunk_tempactive = false;

	uint64_t zonesize;

    int i = I_GetParm("-ram");
    if (i != -1) {
        if (i+1 >= myargc)
            N_Error("Memory_Init: you must specify the amount of ram in MB after -ram");
        else
            hunk_size = N_atoi(myargv[i+1]) * 1024 * 1024;
    }
    else {
        hunk_size = DEFAULT_HEAP_SIZE;
    }
    hunk_base = (byte *)malloc(hunk_size);
    if (!hunk_base) {
        N_Error("Memory_Init: malloc() failed on %lu bytes when allocating the hunk", hunk_size);
	}

	Con_Printf("Initialized heap from %p -> %p (%lu MiB)", (void *)hunk_base, (void *)(hunk_base + hunk_size), hunk_size >> 20);
    memset(hunk_base, 0, hunk_size);

	zonesize = DEFAULT_ZONE_SIZE;
	if (hunk_size != DEFAULT_HEAP_SIZE) {
		zonesize = hunk_size / 4;
	}
	Con_Printf("Zone Memory initialized with %lu bytes (%f MiB)", zonesize, (float)zonesize / 1024 / 1024);

    zonesize += sizeof(memzone_t);
    mainzone = (memzone_t *)Hunk_Alloc(zonesize, "mainzone", h_low);

    mainzone->size = zonesize - sizeof(memzone_t);

	atexit(Memory_Shutdown);

    // initialize the zone heap variables
    Z_Init();

	Cmd_AddCommand("meminfo", Com_Meminfo_f);
	Cmd_AddCommand("zoneinfo", Z_Print_f);
	Cmd_AddCommand("hunkinfo", Hunk_Print);
}

/*
===============================
Hunk Allocator:
Only meant for large static allocations. Each allocation is not temporary, cannot be freed (no direct function to do it), and is expected to be allocated
for the entirety of runtime. ONLY MEANT FOR MAIN ENGINE ALLOCATIONS
===============================
*/
#define HUNKID 0xffa3d9

void Hunk_Check(void)
{
    hunk_t *hunk;
    for (hunk = (hunk_t *)hunk_base; (byte *)hunk != hunk_base + hunk_low_used;) {
        if (hunk->id != HUNKID)
            N_Error("Hunk_Check: hunk id isn't correct");
        if (hunk->size < 16 || hunk->size + (byte *)hunk - hunk_base > hunk_size)
            N_Error("Hunk_Check: bad size");
        
        hunk = (hunk_t *)((byte *)hunk+hunk->size);
    }
}

void Hunk_Clear(void)
{
    Con_Printf("WARNING: clearing hunk heap");
    hunk_low_used = hunk_high_used = 0;
    memset(hunk_base, 0, hunk_size);

	// re-initialize the zone heap
	Z_Init();
}

uint64_t Hunk_LowMark(void)
{
    return hunk_low_used;
}

void Hunk_FreeToLowMark(uint64_t mark)
{
    if (mark > hunk_low_used)
        N_Error("Hunk_FreeToLowMark: bad mark %lu", mark);
    
    memset(hunk_base + mark, 0, hunk_low_used - mark);
    hunk_low_used = mark;
}

uint64_t Hunk_HighMark(void)
{
    return hunk_high_used;
}

void Hunk_FreeToHighMark(uint64_t mark)
{
    if (mark > hunk_high_used)
        N_Error("Hunk_FreeToHighMark: bad mark %lu", mark);
    
    memset(hunk_base + hunk_size - hunk_high_used, 0, hunk_high_used - mark);
    hunk_high_used = mark;
}

void Hunk_Print(void)
{
    hunk_t *h, *next, *endlow, *starthigh, *endhigh;
	uint64_t count, sum;
	uint64_t totalblocks;
	char name[15];

	name[14] = 0;
	count = 0;
	sum = 0;
	totalblocks = 0;
	
	h = (hunk_t *)hunk_base;
	endlow = (hunk_t *)(hunk_base + hunk_low_used);
	starthigh = (hunk_t *)(hunk_base + hunk_size - hunk_high_used);
	endhigh = (hunk_t *)(hunk_base + hunk_size);

    Con_Printf("\n\n<----- Hunk Heap Report ----->");
	Con_Printf("          :%8i total hunk size", hunk_size);
	Con_Printf("-------------------------");

	while (1) {
		// skip to the high hunk if done with low hunk
		if (h == endlow)
			h = starthigh;
		
		// if totally done, break
		if (h == endhigh)
			break;

		// run consistancy checks
		if (h->id != HUNKID)
			N_Error("Hunk_Check: hunk id isn't correct");
		if (h->size < 16 || h->size + (byte *)h - hunk_base > hunk_size)
			N_Error("Hunk_Check: bad size");
			
		next = (hunk_t *)((byte *)h+h->size);
		count++;
		totalblocks++;
		sum += h->size;

		// print the single block
		memcpy(name, h->name, 14);
		Con_Printf("%8p : %8i   %8s", h, h->size, name);

		h = next;
	}
	Con_Printf("-------------------------");
	Con_Printf("          :%8i REMAINING", hunk_size - hunk_low_used - hunk_high_used);
	Con_Printf("-------------------------");
	Con_Printf("          :%8i (TOTAL)", sum);
	count = 0;
	sum = 0;

	Con_Printf("-------------------------");
	Con_Printf("%8i total allocations\n\n", totalblocks);
}

static void* Hunk_AllocHigh(uint64_t size, const char *name)
{
    hunk_t *hunk;

    if (hunk_size - hunk_high_used < size) {
        Con_Printf("Hunk_AllocHigh: failed on allocation of %li bytes", size);
        return NULL;
    }

    hunk_high_used += size;

    hunk = (hunk_t *)(hunk_base + hunk_size - hunk_high_used);

    memset(hunk, 0, size);
    hunk->size = size;
    hunk->id = HUNKID;
    N_strncpy(hunk->name, name, 14);

    return (void *)(hunk+1);
}

static void* Hunk_AllocLow(uint64_t size, const char *name)
{
    hunk_t *hunk;

    if (hunk_size - hunk_low_used < size)
        N_Error("Hunk_AllocLow: failed on allocation of %li bytes", size);
    
    hunk = (hunk_t *)(hunk_base + hunk_low_used);
    hunk_low_used += size;

    memset(hunk, 0, size);
    hunk->size = size;
    hunk->id = HUNKID;
    N_strncpy(hunk->name, name, 14);

    return (void *)(hunk+1);
}

void* Hunk_Alloc(uint64_t size, const char *name, int where)
{
    if (!size)
        N_Error("Hunk_Alloc: bad size");
    if (where < h_low || where > h_high)
        N_Error("Hunk_Alloc: bad pos");
    
    if (hunk_tempactive) {
        Hunk_FreeToHighMark(hunk_tempmark);
        hunk_tempactive = false;
    }

#ifdef _NOMAD_DEBUG
    Hunk_Check();
#endif

    size += sizeof(hunk_t);
	size = PAD(size, 64);
    
    if (where == h_high)
        return Hunk_AllocHigh(size, name);
    else if (where == h_low)
        return Hunk_AllocLow(size, name);

    return NULL;
}

void* Hunk_TempAlloc(uint32_t size)
{
    void *buf;
    
	size = PAD(size, 64);
    
    if (hunk_tempactive) {
        Hunk_FreeToHighMark(hunk_tempmark);
        hunk_tempactive = false;
    }

    hunk_tempmark = Hunk_HighMark();
    buf = Hunk_AllocHigh(size, "temp");
    hunk_tempactive = true;

    return buf;
}


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

void* operator new[](size_t size, const char* pName, int flags, unsigned debugFlags, const char* file, int line)
{
    return ::operator new[](size);
}
void* operator new[](size_t size, size_t alignment, size_t alignmentOffset, const char* pName, int flags, unsigned debugFlags, const char* file, int line)
{
    return ::operator new[](size, std::align_val_t(alignment));
}

void* Z_ZoneBegin(void)
{
	return (void *)mainzone;
}

void* Z_ZoneEnd(void)
{
	return (void *)((char *)mainzone+mainzone->size);
}

void Z_Init(void)
{
	memblock_t* base;
	
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
	N_strncpy(base->name, "base", 14);
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
	Con_Printf("WARNING: clearing zone");
	memblock_t*		block;

    // set it all to zero, weed out any bugs
	memset(mainzone, 0, mainzone->size+sizeof(memzone_t));
	
	mainzone->blocklist.next = 
	mainzone->blocklist.prev = 
	block = (memblock_t *)((byte *)mainzone+sizeof(memzone_t));
	
	mainzone->blocklist.user = (void **)mainzone;
	mainzone->blocklist.tag = TAG_STATIC;
	mainzone->blocklist.id = ZONEID;
	mainzone->rover = block;
	
	block->prev = block->next = &mainzone->blocklist;
	block->user = (void **)NULL;
	block->size = mainzone->size - sizeof(memzone_t);
	block->id = ZONEID;
	N_strncpy(block->name, "base", 14);
}

// counts the number of blocks by the tag
uint32_t Z_NumBlocks(int32_t tag)
{
	memblock_t* block;
	uint32_t count = 0;
	for (block = mainzone->blocklist.next; block != &mainzone->blocklist; block = block->next) {
		if (block->tag == tag) {
			++count;
		}
	}
	return count;
}

void Z_Defrag(void)
{
	memblock_t* block;
	for (block = mainzone->blocklist.next;; block = block->next) {
		if (block == &mainzone->blocklist) {
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
	if (block->tag != TAG_FREE && block->user != (void **)NULL) {
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
	Z_ScanForBlock(ptr, (byte *)ptr + block->size - sizeof(memblock_t));
#endif
	
	Z_MergePB(block);
	Z_MergeNB(block);
}

// Z_Malloc: garbage collection and zone block allocater that returns a block of free memory
// from within the zone without calling malloc
void* Z_Malloc(uint32_t size, int32_t tag, void *user, const char *name)
{
#ifdef _NOMAD_DEBUG
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
	uint32_t extra;
	bool tryagain;
	
    // accounting for header size and alignment
	size += sizeof(memblock_t);
    size = (size + 15 - 1) & ~(15 - 1);
	tryagain = false;
	
	base = mainzone->rover;
	
	// checking behind the rover
	if (base->prev->tag == TAG_FREE)
		base = base->prev;

	rover = base;
	start = base->prev;

	do {
		if (rover == start) {
			Con_Printf("WARNING: zone size wasn't big enough for Z_Malloc size given, clearing cache");
			// clean all the caches
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
	
    N_strncpy(base->name, name, 14);
#ifdef _NOMAD_DEBUG
	Z_CheckHeap();
#endif

    return retn;
}

void Z_ChangeTag(void *user, int32_t tag)
{
	// sanity
	if (!user)
		N_Error("Z_ChangeTag: user is NULL");
	if (tag >= TAG_PURGELEVEL && !user)
		N_Error("Z_ChangeTag: user is required for purgable blocks");
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

void Z_FreeTags(int32_t lowtag, int32_t hightag)
{
	memblock_t* block;
	memblock_t* next;
	uint64_t totalBytes = 0;
	
    Con_Printf("freeing zone blocks from tags %i (lowtag) to %i (hightag)", lowtag, hightag);
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
			Con_Printf("0x%8p : %8i  %8s", (void *)block, block->size, name);
	
		if (block->next == &mainzone->blocklist) {
			Con_Printf("          : %8lu (TOTAL)", sum);
			count = 0;
			sum = 0;
		}
	}
	Con_Printf("-------------------------");
	Con_Printf("%8lu total blocks\n\n", totalblocks);
}

void* Z_Realloc(uint32_t nsize, int32_t tag, void *user, void *ptr, const char* name)
{
#ifdef _NOMAD_DEBUG
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
	return p;
}

void* Z_Calloc(uint32_t size, int32_t tag, void *user, const char* name)
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
	Con_Printf("performing garbage collection of zone");
	
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
	Con_Printf(DEBUG, "Running heap check");
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
	Con_Printf(DEBUG, "Done with heap check");
}

void Mem_Info(void)
{
    Hunk_Print();
    Z_Print(true);
}