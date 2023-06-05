#include "n_shared.h"

Heap Heap::heap;

#define UNOWNED    ((void *)666)
#define ZONEID     0xa21d49

#define MIN_FRAGMENT 64

#define DEFAULT_HEAP_SIZE (1700*1024*1024)
#define DEFAULT_ZONE_SIZE (400*1024*1024)

static byte *hunk_base;
static uint64_t hunk_size;
static uint64_t hunk_low_used;
static uint64_t hunk_high_used;
static bool hunk_tempactive;
static uint64_t hunk_tempmark;
static memzone_t* mainzone;

// tunables
#define ZONEDEBUG
#define ZONEIDCHECK
#define CHECKHEAP

#define MEM_ALIGN  16
#define RETRY_AMOUNT (256*1024)

Heap::~Heap()
{
	Memory_Shutdown();
}

void Memory_Shutdown(void)
{
	Con_Printf("Memory_Shutdown: deallocating allocation daemons");
    free(hunk_base);
    Mem_Shutdown();
}

void Memory_Init(void)
{
	Con_Printf("Memory_Init: initializing allocation daemons");

	Mem_Init();

    hunk_size = 0;
    hunk_low_used = hunk_high_used = hunk_tempmark = 0;
    hunk_tempactive = false;

    int i = I_GetParm("-ram");
    if (i != -1) {
        if (i+1 > myargc)
            N_Error("Memory_Init: you must specify the amount of ram in MB after -ram");
        else
            hunk_size = N_atoi(myargv[i+1]) * 1024 * 1024;
    }
    else {
        hunk_size = DEFAULT_HEAP_SIZE;
    }
    hunk_base = (byte *)malloc(hunk_size);
    if (!hunk_base) {
        N_Error("Memory_Init: malloc() failed on %lu bytes when allocating the hunk, errno: %d", hunk_size, errno);
	}
	Con_Printf("Initialized heap from %p -> %p (%lu MiB)", (void *)hunk_base, (void *)(hunk_base + hunk_size), hunk_size >> 20);
    memset(hunk_base, 0, hunk_size);

    uint64_t zonesize;
	if (hunk_size != DEFAULT_HEAP_SIZE) {
		zonesize = hunk_size / 4;
	}
	else {
		zonesize = DEFAULT_ZONE_SIZE;
	}
	Con_Printf("Zone Memory initialized with %lu bytes", zonesize);

    zonesize += sizeof(memzone_t);
    mainzone = (memzone_t *)Hunk_Alloc(zonesize, "zoneheap", h_low);
    memset(mainzone, 0, zonesize);

    mainzone->size = zonesize - sizeof(memzone_t);

    // initialize the zone heap variables
    Heap::Get().Z_Init();
}

/*
===============================
Hunk Allocator:
Only meant for large static allocations. Each allocation is not temporary, cannot be freed (no direct function to do it), and is expected to be allocated
for the entirety of runtime. ONLY MEANT FOR MAIN ENGINE ALLOCATIONS
===============================
*/
#define HUNKID 0xffa3d9

void Heap::Hunk_Check(void)
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

void Heap::Hunk_Clear(void)
{
    Con_Printf("WARNING: clearing hunk heap");
    hunk_low_used = hunk_high_used = 0;
    memset(hunk_base, 0, hunk_size);
}

uint64_t Heap::Hunk_LowMark(void)
{
    return hunk_low_used;
}

void Heap::Hunk_FreeToLowMark(uint64_t mark)
{
    if (mark > hunk_low_used)
        N_Error("Hunk_FreeToLowMark: bad mark %lu", mark);
    
    memset(hunk_base + mark, 0, hunk_low_used - mark);
    hunk_low_used = mark;
}

uint64_t Heap::Hunk_HighMark(void)
{
    return hunk_high_used;
}

void Heap::Hunk_FreeToHighMark(uint64_t mark)
{
    if (mark > hunk_high_used)
        N_Error("Hunk_FreeToHighMark: bad mark %lu", mark);
    
    memset(hunk_base + hunk_size - hunk_high_used, 0, hunk_high_used - mark);
    hunk_high_used = mark;
}

void Heap::Hunk_Print(void)
{
    hunk_t	*h, *next, *endlow, *starthigh, *endhigh;
	int		count, sum;
	int		totalblocks;
	char	name[15];

	name[14] = 0;
	count = 0;
	sum = 0;
	totalblocks = 0;
	
	h = (hunk_t *)hunk_base;
	endlow = (hunk_t *)(hunk_base + hunk_low_used);
	starthigh = (hunk_t *)(hunk_base + hunk_size - hunk_high_used);
	endhigh = (hunk_t *)(hunk_base + hunk_size);

    Con_Printf ("\n\n<----- Hunk Heap Report ----->");
	Con_Printf ("          :%8i total hunk size", hunk_size);
	Con_Printf ("-------------------------");

	while (1) {
		//
		// skip to the high hunk if done with low hunk
		//
		if (h == endlow)
			h = starthigh;
		
		//
		// if totally done, break
		//
		if ( h == endhigh )
			break;

		//
		// run consistancy checks
		//
		if (h->id != HUNKID)
			N_Error ("Hunk_Check: hunk id isn't correct");
		if (h->size < 16 || h->size + (byte *)h - hunk_base > hunk_size)
			N_Error ("Hunk_Check: bad size");
			
		next = (hunk_t *)((byte *)h+h->size);
		count++;
		totalblocks++;
		sum += h->size;

		//
		// print the single block
		//
		N_strncpy (name, h->name, 14);
		Con_Printf ("%8p : %8i   %8s", h, h->size, name);

		h = next;
	}
	Con_Printf ("-------------------------");
	Con_Printf ("          :%8i REMAINING", hunk_size - hunk_low_used - hunk_high_used);
	Con_Printf ("-------------------------");
	Con_Printf ("          :%8i (TOTAL)", sum);
	count = 0;
	sum = 0;

	Con_Printf ("-------------------------");
	Con_Printf ("%8i total allocations\n\n", totalblocks);
}

void* Heap::Hunk_AllocHigh(uint64_t size, const char *name)
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

void* Heap::Hunk_AllocLow(uint64_t size, const char *name)
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

void* Heap::Hunk_Alloc(uint64_t size, const char *name, int where)
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

    size = sizeof(hunk_t) + ((size+15)&~15);
    
    if (where == h_high)
        return Hunk_AllocHigh(size, name);
    else if (where == h_low)
        return Hunk_AllocLow(size, name);

    return NULL;
}

void* Heap::Hunk_TempAlloc(uint32_t size)
{
    void *buf;
    
    size = (size + 15) & ~15;
    
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

void* Heap::Z_ZoneBegin(void)
{
	return (void *)mainzone;
}

void* Heap::Z_ZoneEnd(void)
{
	return (void *)((char *)mainzone+mainzone->size);
}

void Heap::Z_Init(void)
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

void Heap::Z_MergePB(memblock_t* block)
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

void Heap::Z_MergeNB(memblock_t* block)
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

void Heap::Z_ScanForBlock(void *start, void *end)
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

void Heap::Z_ClearZone(void)
{
	Con_Printf("WARNING: clearing zone");
	memblock_t*		block;

    // set it all to zero, weed out any bugs
    memset(mainzone, 0, mainzone->size);

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

	N_strncpy(block->name, "base", 14);
	
	// a free block.
	block->tag = TAG_FREE;
	
	block->size = mainzone->size - sizeof(memzone_t);
}

// counts the number of blocks by the tag
uint32_t Heap::Z_NumBlocks(int tag)
{
	memblock_t* block;
	int count = 0;
	for (block = mainzone->blocklist.next; block != &mainzone->blocklist; block = block->next) {
		if (block->tag == tag) {
			++count;
		}
	}
	return count;
}

void Heap::Z_Defrag(void)
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

void Heap::Z_Free(void *ptr)
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

void* Heap::Z_AlignedAlloc(uint32_t alignment, uint32_t size, int tag, void *user, const char* name)
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
	uint32_t extra;
	bool tryagain;
	
    // accounting for header size and alignment
	size += sizeof(memblock_t);
    size = (size + alignment - 1) & ~(alignment - 1);
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
			Z_CleanCache();
			Z_FreeTags(TAG_PUR)
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
#ifdef CHECKHEAP
	Z_CheckHeap();
#endif

    return retn;
}

// Z_Malloc: garbage collection and zone block allocater that returns a block of free memory
// from within the zone without calling malloc
void* Heap::Z_Malloc(uint32_t size, int tag, void *user, const char* name)
{
#ifdef CHECKHEAP
	Z_CheckHeap();
#endif
	Z_Defrag();
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
			Z_CleanCache();
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
#ifdef CHECKHEAP
	Z_CheckHeap();
#endif

    return retn;
}

void Heap::Z_ChangeTag(void *user, int tag)
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
}

void Heap::Z_ChangeName(void *ptr, const char* name)
{
	memblock_t* block;
	
	block = (memblock_t *)((byte *)ptr - sizeof(memblock_t));
	if (block->id != ZONEID)
		N_Error("Z_ChangeName: block id wasn't zoneid");
	
	N_strncpy(block->name, name, sizeof(block->name) - 1);
}

void Heap::Z_ChangeUser(void *ptr, void *user)
{
	memblock_t* block;
	
	block = (memblock_t *)((byte *)user - sizeof(memblock_t));
	if (block->id != ZONEID)
		N_Error("Z_ChangeUser: block id isn't ZONEID");
	
	*block->user = ptr;
}

uint64_t Heap::Z_FreeMemory(void)
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

void Heap::Z_FreeTags(int lowtag, int hightag)
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

void Heap::Z_Print(bool all)
{
	memblock_t *block, *next;
	uint64_t count, sum, totalblocks;
	uint64_t blockcount[NUMTAGS] = {0};
	char name[15];
	
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

	name[14] = 0;
	count = 0;
	sum = 0;
	totalblocks = 0;
	
	total_memory = static_mem + audio_mem + cached_mem + purgable_mem + free_mem;
	double s = 100.0f / total_memory;
	
    Con_Printf("\n\n<---- Zone Allocation Daemon Heap Report ---->");
	Con_Printf("          : %8lu total zone size", mainzone->size);
	Con_Printf("-------------------------");
	Con_Printf("-------------------------");
	Con_Printf("          : %8lu REMAINING", mainzone->size - static_mem - cached_mem - purgable_mem - audio_mem);
	Con_Printf("-------------------------");
	Con_Printf("(PERCENTAGES)");
	Con_Printf(
			"%8lu   %6.02f%%   static\n"
			"%8lu   %6.02f%%   cached\n"
			"%8lu   %6.02f%%   audio\n"
			"%8lu   %6.02f%%   purgable\n"
			"%8lu   %6.02f%%   free",
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
		
		N_strncpy(name, block->name, 14);
		if (all)
			Con_Printf("%8p : %8i  %8s", (void *)block, block->size, name);
		
		if (block->next == &mainzone->blocklist) {
			Con_Printf("          : %8lu (TOTAL)", sum);
			count = 0;
			sum = 0;
		}
	}
	Con_Printf("-------------------------");
	Con_Printf("%8lu total blocks\n\n", totalblocks);
}

void* Heap::Z_Realloc(void* ptr, uint32_t nsize, void* user, int tag, const char* name)
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
	return p;
}

void* Heap::Z_Calloc(void *user, uint32_t size, int tag, const char* name)
{
#ifdef CHECKHEAP
	Z_CheckHeap();
#endif
	return memset(Z_Malloc(size, tag, user, name), 0, size);
}

// cleans all zone caches (only blocks from scope to free to unused)
void Heap::Z_CleanCache(void)
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

void Heap::Z_CheckHeap(void)
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
#if 0
	Con_Printf("Done with heap check");
#endif
}

void Heap::Mem_Info(void)
{
    Hunk_Print();
    Z_Print(true);
}
