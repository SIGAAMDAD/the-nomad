#include "sg_local.h"

// 10 MiB of static memory for the vm to use
#define MEMPOOL_SIZE (10*1024*1024)
static char mempool[MEMPOOL_SIZE];
static uint32_t allocPoint;

#define STRINGPOOL_SIZE (8*1024)


#define HASH_TABLE_SIZE 2048

typedef struct stringDef_s {
	struct stringDef_s *next;
	const char *str;
} stringDef_t;

static uint32_t strPoolIndex;
static char strPool[STRINGPOOL_SIZE];

static uint32_t strHandleCount;
static stringDef_t *strHandle[HASH_TABLE_SIZE];

const char *String_Alloc( const char *p )
{
	uint32_t len;
	uint64_t hash;
	stringDef_t *str, *last;
	static const char *staticNULL = "";

	if (p == NULL) {
		return NULL;
	}

	if (*p == 0) {
		return staticNULL;
	}

	hash = Com_GenerateHashValue( p, HASH_TABLE_SIZE );

	str = strHandle[hash];
	while (str) {
		if (strcmp(p, str->str) == 0) {
			return str->str;
		}
		str = str->next;
	}

	len = strlen(p);
	if (len + strPoolIndex + 1 < STRINGPOOL_SIZE) {
		uint32_t ph = strPoolIndex;
		strcpy(&strPool[strPoolIndex], p);
		strPoolIndex += len + 1;

		str = strHandle[hash];
		last = str;
		while (str && str->next) {
			last = str;
			str = str->next;
		}

		str = (stringDef_t *)SG_MemAlloc( sizeof(stringDef_t) );
		str->next = NULL;
		str->str = &strPool[ph];
		if (last) {
			last->next = str;
		} else {
			strHandle[hash] = str;
		}
		return &strPool[ph];
	}
	return NULL;
}

void String_Report( void )
{
	float f;
	Con_Printf("Memory/String Pool Info\n");
	Con_Printf("----------------\n");
	
    f = strPoolIndex;
	f /= STRINGPOOL_SIZE;
	f *= 100;
	Con_Printf("String Pool is %.1f%% full, %i bytes out of %i used.\n", f, strPoolIndex, STRINGPOOL_SIZE);
	
    f = allocPoint;
	f /= MEMPOOL_SIZE;
	f *= 100;
	Con_Printf("Memory Pool is %.1f%% full, %i bytes out of %i used.\n", f, allocPoint, MEMPOOL_SIZE);
}

void *SG_MemAlloc( uint32_t size )
{
    char *buf;

    if (!size) {
        G_Error( "SG_MemAlloc: bad size" );
    }

    size = PAD(size, (unsigned)16); // round to 16-byte alignment

    if (allocPoint + size >= sizeof(mempool)) {
        G_Error( "SG_MemAlloc: not enough vm memory" );
        return NULL;
    }

    buf = &mempool[ allocPoint ];
    allocPoint += size;

    // zero init
    memset( buf, 0, size );

    return buf;
}

uint32_t SG_MemoryRemaining( void ) {
    return sizeof(mempool) - allocPoint;
}

void SG_MemInit( void )
{
    uint32_t i;

    memset( mempool, 0, sizeof(mempool) );
    memset( strPool, 0, sizeof(strPool) );

	for (i = 0; i < HASH_TABLE_SIZE; i++) {
		strHandle[i] = 0;
	}
	strHandleCount = 0;
	strPoolIndex = 0;
}

#if 0
//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 2005-2014 Simon Howard
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// DESCRIPTION:
//	Zone Memory Allocation. Neat.
//

#include <string.h>

#include "doomtype.h"
#include "i_system.h"
#include "m_argv.h"

#include "z_zone.h"


//
// ZONE MEMORY ALLOCATION
//
// There is never any space between memblocks,
//  and there will never be two contiguous free memblocks.
// The rover can be left pointing at a non-empty block.
//
// It is of no value to free a cachable block,
//  because it will get overwritten automatically if needed.
// 
 
#define MEM_ALIGN sizeof(void *)
#define ZONEID	0x1d4a11

typedef struct memblock_s
{
    int			size;	// including the header and possibly tiny fragments
    void**		user;
    int			tag;	// TAG_FREE if this is free
    int			id;	// should be ZONEID
    struct memblock_s*	next;
    struct memblock_s*	prev;
} memblock_t;


typedef struct
{
    // total bytes malloced, including header
    int		size;

    // start / end cap for linked list
    memblock_t	blocklist;
    
    memblock_t*	rover;
    
} memzone_t;



static memzone_t *mainzone;
static boolean zero_on_free;
static boolean scan_on_free;


//
// Z_ClearZone
//
void Z_ClearZone (memzone_t* zone)
{
    memblock_t*		block;
	
    // set the entire zone to one free block
    zone->blocklist.next =
	zone->blocklist.prev =
	block = (memblock_t *)( (byte *)zone + sizeof(memzone_t) );
    
    zone->blocklist.user = (void *)zone;
    zone->blocklist.tag = TAG_STATIC;
    zone->rover = block;
	
    block->prev = block->next = &zone->blocklist;
    
    // a free block.
    block->tag = TAG_FREE;

    block->size = zone->size - sizeof(memzone_t);
}



//
// Z_Init
//
void Z_Init (void)
{
    memblock_t*	block;
    int		size;

    mainzone = (memzone_t *)I_ZoneBase (&size);
    mainzone->size = size;

    // set the entire zone to one free block
    mainzone->blocklist.next =
	mainzone->blocklist.prev =
	block = (memblock_t *)( (byte *)mainzone + sizeof(memzone_t) );

    mainzone->blocklist.user = (void *)mainzone;
    mainzone->blocklist.tag = TAG_STATIC;
    mainzone->rover = block;

    block->prev = block->next = &mainzone->blocklist;

    // free block
    block->tag = TAG_FREE;

    block->size = mainzone->size - sizeof(memzone_t);

    // [Deliberately undocumented]
    // Zone memory debugging flag. If set, memory is zeroed after it is freed
    // to deliberately break any code that attempts to use it after free.
    //
    zero_on_free = M_ParmExists("-zonezero");

    // [Deliberately undocumented]
    // Zone memory debugging flag. If set, each time memory is freed, the zone
    // heap is scanned to look for remaining pointers to the freed block.
    //
    scan_on_free = M_ParmExists("-zonescan");
}

// Scan the zone heap for pointers within the specified range, and warn about
// any remaining pointers.
static void ScanForBlock(void *start, void *end)
{
    memblock_t *block;
    void **mem;
    uint32_t i, len, tag;

    block = mainzone->blocklist.next;

    while (block->next != &mainzone->blocklist)
    {
        tag = block->tag;

        if (tag == TAG_STATIC || tag == TAG_LEVEL || tag == TAG_LEVSPEC) {
            // Scan for pointers on the assumption that pointers are aligned
            // on word boundaries (word size depending on pointer size):
            mem = (void **) ((byte *) block + sizeof(memblock_t));
            len = (block->size - sizeof(memblock_t)) / sizeof(void *);

            for (i = 0; i < len; ++i)
            {
                if (start <= mem[i] && mem[i] <= end)
                {
                    G_Printf(
                            "%p has dangling pointer into freed block "
                            "%p (%p -> %p)\n",
                            mem, start, &mem[i], mem[i]);
                }
            }
        }

        block = block->next;
    }
}

//
// Z_Free
//
void Z_Free (void* ptr)
{
    memblock_t *block;
    memblock_t *other;

    block = (memblock_t *) ( (byte *)ptr - sizeof(memblock_t));

    if (block->id != ZONEID) {
	    G_Error ("Z_Free: freed a pointer without ZONEID");
    }

    // mark as free
    block->tag = TAG_FREE;
    block->id = 0;

    if (sg_zeroMemOnFree.i) {
        memset(ptr, 0, block->size - sizeof(memblock_t));
    }
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

    other = block->next;
    if (other->tag == TAG_FREE) {
        // merge the next free block onto the end
        block->size += other->size;
        block->next = other->next;
        block->next->prev = block;

        if (other == mainzone->rover)
            mainzone->rover = block;
    }
}



//
// Z_Malloc
// You can pass a NULL user if the tag is < TAG_PURGELEVEL.
//
#define MINFRAGMENT		64

void *Z_Malloc( uint32_t size, uint32_t tag )
{
    uint32_t extra;
    memblock_t *start;
    memblock_t *rover;
    memblock_t *newblock;
    memblock_t *base;
    void *result;

    size = PAD(size, 16);
    
    // scan through the block list,
    // looking for the first free block
    // of sufficient size,
    // throwing out any purgable blocks along the way.

    // account for size of block header
    size += sizeof(memblock_t);
    
    // if there is a free block behind the rover,
    //  back up over them
    base = mainzone->rover;
    
    if (base->prev->tag == TAG_FREE)
        base = base->prev;
	
    rover = base;
    start = base->prev;
	
    do {
        if (rover == start) {
            // scanned all the way around the list
            G_Error ("Z_Malloc: failed on allocation of %i bytes", size);

            base = mainzone->rover;
            rover = base;
            start = base->prev;
        }
	
        if (rover->tag != TAG_FREE) {
            if (rover->tag < TAG_PURGELEVEL) {
                // hit a block that can't be purged,
                // so move base past it
                base = rover = rover->next;
            }
            else {
                // free the rover block (adding the size to base)

                // the rover can be the base block
                base = base->prev;
                Z_Free ((byte *)rover+sizeof(memblock_t));
                base = base->next;
                rover = base->next;
            }
        }
        else {
            rover = rover->next;
        }

    } while (base->tag != TAG_FREE || base->size < size);

    
    // found a block big enough
    extra = base->size - size;
    
    if (extra >  MINFRAGMENT) {
        // there will be a free fragment after the allocated block
        newblock = (memblock_t *) ((byte *)base + size );
        newblock->size = extra;
	
        newblock->tag = TAG_FREE;	
        newblock->prev = base;
        newblock->next = base->next;
        newblock->next->prev = newblock;

        base->next = newblock;
        base->size = size;
    }

    base->tag = tag;

    result  = (void *) ((byte *)base + sizeof(memblock_t));

    // next allocation will start looking here
    mainzone->rover = base->next;	
	
    base->id = ZONEID;
   
    return result;
}

//
// Z_FreeTags
//
void Z_FreeTags( uint32_t lowtag, uint32_t hightag )
{
    memblock_t *block;
    memblock_t *next;
	
    for (block = mainzone->blocklist.next ; block != &mainzone->blocklist ; block = next) {
	    // get link before freeing
	    next = block->next;

	    // free block?
	    if (block->tag == TAG_FREE)
	        continue;
    
	    if (block->tag >= lowtag && block->tag <= hightag)
	        Z_Free ( (byte *)block+sizeof(memblock_t));
    }
}



//
// Z_DumpHeap
// Note: TFileDumpHeap( stdout ) ?
//
void Z_DumpHeap( uint32_t lowtag, uint32_t hightag )
{
    memblock_t *block;
	
    G_Printf ("zone size: %i  location: %p\n",
	    mainzone->size,mainzone);
    
    G_Printf ("tag range: %i to %i\n",
	    lowtag, hightag);
	
    for (block = mainzone->blocklist.next ; ; block = block->next) {
	    if (block->tag >= lowtag && block->tag <= hightag) {
	        G_Printf ("block:%p    size:%7i    user:%p    tag:%3i\n",
	    	    block, block->size, block->user, block->tag);
        }
		
    	if (block->next == &mainzone->blocklist) {
    	    // all blocks have been hit
    	    break;
    	}
	
    	if ( (byte *)block + block->size != (byte *)block->next)
    	    G_Printf ("ERROR: block size does not touch the next block\n");

    	if ( block->next->prev != block)
    	    G_Printf ("ERROR: next block doesn't have proper back link\n");

    	if (block->tag == TAG_FREE && block->next->tag == TAG_FREE)
    	    G_Printf ("ERROR: two consecutive free blocks\n");
    }
}


//
// Z_FileDumpHeap
//
void Z_FileDumpHeap (file_t f)
{
    memblock_t *block;
	
    trap_FS_Printf (f,"zone size: %i  location: %p\n",mainzone->size,mainzone);
	
    for (block = mainzone->blocklist.next ; ; block = block->next) {
        trap_FS_PRintf (f,"block:%p    size:%7i    user:%p    tag:%3i\n",
	    	 block, block->size, block->user, block->tag);
    
	    if (block->next == &mainzone->blocklist)
	    {
	        // all blocks have been hit
	        break;
	    }
    
	    if ( (byte *)block + block->size != (byte *)block->next)
	        trap_FS_Printf (f,"ERROR: block size does not touch the next block\n");

	    if ( block->next->prev != block)
	        trap_FS_Printf (f,"ERROR: next block doesn't have proper back link\n");

	    if (block->tag == TAG_FREE && block->next->tag == TAG_FREE)
	        trap_FS_Printf (f,"ERROR: two consecutive free blocks\n");
    }
}



//
// Z_CheckHeap
//
void Z_CheckHeap (void)
{
    memblock_t*	block;
	
    for (block = mainzone->blocklist.next ; ; block = block->next) {
    	if (block->next == &mainzone->blocklist)
    	{
    	    // all blocks have been hit
    	    break;
    	}
    
    	if ( (byte *)block + block->size != (byte *)block->next)
    	    G_Error ("Z_CheckHeap: block size does not touch the next block");

    	if ( block->next->prev != block)
    	    G_Error ("Z_CheckHeap: next block doesn't have proper back link");

    	if (block->tag == TAG_FREE && block->next->tag == TAG_FREE) {
    	    G_Error ("Z_CheckHeap: two consecutive free blocks");
        }
    }
}

//
// Z_ChangeTag
//
void Z_ChangeTag(void *ptr, uint32_t tag)
{
    memblock_t *block;
	
    block = (memblock_t *) ((byte *)ptr - sizeof(memblock_t));

    if (block->id != ZONEID) {
        G_Error("Z_ChangeTag: block without a ZONEID!");
	}
	
    block->tag = tag;
}

//
// Z_FreeMemory
//
uint32_t Z_FreeMemory (void)
{
    memblock_t		*block;
    uint32_t		free;
	
    free = 0;
    for (block = mainzone->blocklist.next; block != &mainzone->blocklist; block = block->next) {
        if (block->tag == TAG_FREE) {
            free += block->size;
		}
    }

    return free;
}

uint32_t Z_ZoneSize(void)
{
    return mainzone->size;
}

#endif