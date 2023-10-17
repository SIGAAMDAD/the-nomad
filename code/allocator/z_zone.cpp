#include "../engine/n_shared.h"

/*
===============================
Zone Allocation Daemon:
meant for temp engine system allocations. Used by allocation callbacks. Blocks can be freed and are temporary.
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
#define SMALLZONEID 0xaffad21

// tunables
#define USE_MEMSTATIC
#define USE_MULTI_SEGMENT
#define USE_TRASH_TEST

// 100 MiB
#define MAINZONE_DEFSIZE (300*1024*1024+sizeof(memzone_t))
#define MAINZONE_MINSIZE (280*1024*1024+sizeof(memzone_t))
// 40 MiB
#define SMALLZONE_DEFSIZE (80*1024*1024+sizeof(memzone_t))
#define SMALLZONE_MINSIZE (70*1024*1024+sizeof(memzone_t))

#define RETRYAMOUNT (256*1024)

#define MEM_ALIGN		64
#define MIN_FRAGMENT	64

#ifdef USE_MULTI_SEGMENT
#if 1 // forward lookup, faster allocation
#define DIRECTION next
// we may have up to 4 lists to group free blocks by size
//#define TINY_SIZE	32
#define SMALL_SIZE	64
#define MEDIUM_SIZE	128
#else // backward lookup, better free space consolidation
#define DIRECTION prev
#define TINY_SIZE	64
#define SMALL_SIZE	128
#define MEDIUM_SIZE	256
#endif
#endif

#ifdef ZONE_DEBUG
typedef struct zonedebug_s {
	const char *label;
	const char *file;
	unsigned line;
	uint32_t allocSize;
} zonedebug_t;
#endif

typedef struct memblock_s {
	struct memblock_s	*next, *prev;
	uint32_t	size;	// including the header and possibly tiny fragments
	uint32_t	tag;	// a tag of 0 is a free block
	uint32_t	id;		// should be ZONEID
#ifdef ZONE_DEBUG
	zonedebug_t d;
#endif
} memblock_t;

typedef struct freeblock_s {
	struct freeblock_s *prev;
	struct freeblock_s *next;
} freeblock_t;

typedef struct memzone_s {
	uint64_t	size;			// total bytes malloced, including header
	uint64_t	used;			// total bytes used
	memblock_t	blocklist;	// start / end cap for linked list
#ifdef USE_MULTI_SEGMENT
	memblock_t	dummy0;		// just to allocate some space before freelist
	freeblock_t	freelist_tiny;
	memblock_t	dummy1;
	freeblock_t	freelist_small;
	memblock_t	dummy2;
	freeblock_t	freelist_medium;
	memblock_t	dummy3;
	freeblock_t	freelist;
#else
	memblock_t	*rover;
#endif
} memzone_t;

static boost::mutex commonLock;

static int minfragment = MINFRAGMENT; // may be adjusted at runtime

// main zone for all "dynamic" memory allocation
static memzone_t *mainzone;

// we also have a small zone for small allocations that would only
// fragment the main zone (think of cvar and cmd strings)
static memzone_t *smallzone;

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

inline const char *Z_TagToString(int tag)
{
	static const typedef struct {
		const char *name;
		int tag;
	} tagtostring[] = {
		{"TAG_FREE", TAG_FREE},
		{"TAG_STATIC", TAG_STATIC},
		{"TAG_LEVEL", TAG_LEVEL},
		{"TAG_RENDERER", TAG_RENDERER},
		{"TAG_SFX", TAG_SFX},
		{"TAG_MUSIC", TAG_MUSIC},
		{"TAG_SEARCH_PATH", TAG_SEARCH_PATH},
		{"TAG_BFF", TAG_BFF},
		{"TAG_HUNK", TAG_HUNK},
		{"TAG_PURGELEVEL", TAG_PURGELEVEL},
		{"TAG_CACHE", TAG_CACHE}
	};

	for (int i = 0; i < arraylen(tagtostring); i++) {
		if (tagtostring[i].tag == tag) {
			return tagtostring[i].name;
		}
	}
	N_Error(ERR_FATAL, "Bad tag: %i", tag);
	return NULL;
}

#ifdef USE_MULTI_SEGMENT

static void InitFree( freeblock_t *fb )
{
	memblock_t *block = (memblock_t*)( (byte*)fb - sizeof( memblock_t ) );
	memset( block, 0, sizeof( *block ) );
}


static void RemoveFree( memblock_t *block )
{
	freeblock_t *fb = (freeblock_t*)( block + 1 );
	freeblock_t *prev;
	freeblock_t *next;

#ifdef _NOMAD_DEBUG
	if ( fb->next == NULL || fb->prev == NULL || fb->next == fb || fb->prev == fb ) {
		N_Error( ERR_FATAL, "RemoveFree: bad pointers fb->next: %p, fb->prev: %p\n", fb->next, fb->prev );
	}
#endif

	prev = fb->prev;
	next = fb->next;

	prev->next = next;
	next->prev = prev;
}


static void InsertFree( memzone_t *zone, memblock_t *block )
{
	freeblock_t *fb = (freeblock_t*)( block + 1 );
	freeblock_t *prev, *next;
#ifdef TINY_SIZE
	if ( block->size <= TINY_SIZE )
		prev = &zone->freelist_tiny;
	else
#endif
#ifdef SMALL_SIZE
	if ( block->size <= SMALL_SIZE )
		prev = &zone->freelist_small;
	else
#endif
#ifdef MEDIUM_SIZE
	if ( block->size <= MEDIUM_SIZE )
		prev = &zone->freelist_medium;
	else
#endif
		prev = &zone->freelist;

	next = prev->next;

#ifdef _NOMAD_DEBUG
	if ( block->size < sizeof( *fb ) + sizeof( *block ) ) {
		N_Error( ERR_FATAL, "InsertFree: bad block size: %i\n", block->size );
	}
#endif

	prev->next = fb;
	next->prev = fb;

	fb->prev = prev;
	fb->next = next;
}


/*
================
NewBlock

Allocates new free block within specified memory zone

Separator is needed to avoid additional runtime checks in Z_Free()
to prevent merging it with previous free block
================
*/
static freeblock_t *NewBlock( memzone_t *zone, int size )
{
	memblock_t *prev, *next;
	memblock_t *block, *sep;
	int alloc_size;

	// zone->prev is pointing on last block in the list
	prev = zone->blocklist.prev;
	next = prev->next;

	size = PAD( size, 1<<21 ); // round up to 2M blocks
	// allocate separator block before new free block
	alloc_size = size + sizeof( *sep );

	sep = (memblock_t *) calloc( alloc_size, 1 );
	if ( sep == NULL ) {
		N_Error( ERR_FATAL, "Z_Malloc: failed on allocation of %i bytes from the %s zone",
			size, zone == smallzone ? "small" : "main" );
		return NULL;
	}
	block = sep+1;

	// link separator with prev
	prev->next = sep;
	sep->prev = prev;

	// link separator with block
	sep->next = block;
	block->prev = sep;

	// link block with next
	block->next = next;
	next->prev = block;

	sep->tag = TAG_GENERAL; // in-use block
	sep->id = -ZONEID;
	sep->size = 0;

	block->tag = TAG_FREE;
	block->id = ZONEID;
	block->size = size;

	// update zone statistics
	zone->size += alloc_size;
	zone->used += sizeof( *sep );

	InsertFree( zone, block );

	return (freeblock_t*)( block + 1 );
}


static memblock_t *SearchFree( memzone_t *zone, int size )
{
	const freeblock_t *fb;
	memblock_t *base;

#ifdef TINY_SIZE
	if ( size <= TINY_SIZE )
		fb = zone->freelist_tiny.DIRECTION;
	else
#endif
#ifdef SMALL_SIZE
	if ( size <= SMALL_SIZE )
		fb = zone->freelist_small.DIRECTION;
	else
#endif
#ifdef MEDIUM_SIZE
	if ( size <= MEDIUM_SIZE )
		fb = zone->freelist_medium.DIRECTION;
	else
#endif
		fb = zone->freelist.DIRECTION;

	for ( ;; ) {
		// not found, allocate new segment?
		if ( fb == &zone->freelist ) {
			fb = NewBlock( zone, size );
		} else {
#ifdef TINY_SIZE
			if ( fb == &zone->freelist_tiny ) {
				fb = zone->freelist_small.DIRECTION;
				continue;
			}
#endif
#ifdef SMALL_SIZE
			if ( fb == &zone->freelist_small ) {
				fb = zone->freelist_medium.DIRECTION;
				continue;
			}
#endif
#ifdef MEDIUM_SIZE
			if ( fb == &zone->freelist_medium ) {
				fb = zone->freelist.DIRECTION;
				continue;
			}
#endif
		}
		base = (memblock_t*)( (byte*) fb - sizeof( *base ) );
		fb = fb->DIRECTION;
		if ( base->size >= size ) {
			return base;
		}
	}
	return NULL;
}
#endif // USE_MULTI_SEGMENT


/*
========================
Z_ClearZone
========================
*/
static void Z_ClearZone( memzone_t *zone, memzone_t *head, int size, int segnum )
{
	memblock_t	*block;
	int min_fragment;

#ifdef USE_MULTI_SEGMENT
	min_fragment = sizeof( memblock_t ) + sizeof( freeblock_t );
#else
	min_fragment = sizeof( memblock_t );
#endif

	if ( minfragment < min_fragment ) {
		// in debug mode size of memblock_t may exceed MINFRAGMENT
		minfragment = PAD( min_fragment, sizeof( intptr_t ) );
		Con_DPrintf( "zone.minfragment adjusted to %i bytes\n", minfragment );
	}

	// set the entire zone to one free block
	zone->blocklist.next = zone->blocklist.prev = block = (memblock_t *)( zone + 1 );
	zone->blocklist.tag = TAG_GENERAL; // in use block
	zone->blocklist.id = -ZONEID;
	zone->blocklist.size = 0;
#ifndef USE_MULTI_SEGMENT
	zone->rover = block;
#endif
	zone->size = size;
	zone->used = 0;

	block->prev = block->next = &zone->blocklist;
	block->tag = TAG_FREE;	// free block
	block->id = ZONEID;

	block->size = size - sizeof(memzone_t);

#ifdef USE_MULTI_SEGMENT
	InitFree( &zone->freelist );
	zone->freelist.next = zone->freelist.prev = &zone->freelist;

	InitFree( &zone->freelist_medium );
	zone->freelist_medium.next = zone->freelist_medium.prev = &zone->freelist_medium;

	InitFree( &zone->freelist_small );
	zone->freelist_small.next = zone->freelist_small.prev = &zone->freelist_small;

	InitFree( &zone->freelist_tiny );
	zone->freelist_tiny.next = zone->freelist_tiny.prev = &zone->freelist_tiny;

	InsertFree( zone, block );
#endif
}


/*
========================
Z_AvailableZoneMemory
========================
*/
static int Z_AvailableZoneMemory( const memzone_t *zone ) {
#ifdef USE_MULTI_SEGMENT
	return (1024*1024*1024); // unlimited
#else
	return zone->size - zone->used;
#endif
}


/*
========================
Z_AvailableMemory
========================
*/
int Z_AvailableMemory( void ) {
	return Z_AvailableZoneMemory( mainzone );
}


static void MergeBlock( memblock_t *curr_free, const memblock_t *next )
{
	curr_free->size += next->size;
	curr_free->next = next->next;
	curr_free->next->prev = curr_free;
}


/*
========================
Z_Free
========================
*/
void Z_Free( void *ptr ) {
	memblock_t *block, *other;
	memzone_t *zone;

	if (!ptr) {
		N_Error( ERR_DROP, "Z_Free: NULL pointer" );
	}

	block = (memblock_t *) ( (byte *)ptr - sizeof(memblock_t));
	if (block->id != ZONEID) {
		N_Error( ERR_FATAL, "Z_Free: freed a pointer without ZONEID" );
	}

	if (block->tag == TAG_FREE) {
		N_Error( ERR_FATAL, "Z_Free: freed a freed pointer" );
	}

	// check the memory trash tester
#ifdef USE_TRASH_TEST
	if ( *(int *)((byte *)block + block->size - 4 ) != ZONEID ) {
		N_Error( ERR_FATAL, "Z_Free: memory block wrote past end" );
	}
#endif

	if ( block->tag == TAG_SMALL ) {
		zone = smallzone;
	} else {
		zone = mainzone;
	}

	zone->used -= block->size;

	// set the block to something that should cause problems
	// if it is referenced...
	memset( ptr, 0xaa, block->size - sizeof( *block ) );

	block->tag = TAG_FREE; // mark as free
	block->id = ZONEID;

	other = block->prev;
	if ( other->tag == TAG_FREE ) {
#ifdef USE_MULTI_SEGMENT
		RemoveFree( other );
#endif
		// merge with previous free block
		MergeBlock( other, block );
#ifndef USE_MULTI_SEGMENT
		if ( block == zone->rover ) {
			zone->rover = other;
		}
#endif
		block = other;
	}

#ifndef USE_MULTI_SEGMENT
	zone->rover = block;
#endif

	other = block->next;
	if ( other->tag == TAG_FREE ) {
#ifdef USE_MULTI_SEGMENT
		RemoveFree( other );
#endif
		// merge the next free block onto the end
		MergeBlock( block, other );
	}

#ifdef USE_MULTI_SEGMENT
	InsertFree( zone, block );
#endif
}


/*
================
Z_FreeTags
================
*/
uint64_t Z_FreeTags( int lowtag, int hightag )
{
	uint64_t count;
	memzone_t	*zone;
	memblock_t	*block, *freed;

	zone = mainzone;
	count = 0;
	for ( block = zone->blocklist.next ; ; ) {
		if ( block->tag >= lowtag && block->tag <= hightag && block->id == ZONEID ) {
			if ( block->prev->tag == TAG_FREE )
				freed = block->prev;  // current block will be merged with previous
			else
				freed = block; // will leave in place
			Z_Free( (void*)( block + 1 ) );
			block = freed;
			count++;
		}
		if ( block->next == &zone->blocklist ) {
			break;	// all blocks have been hit
		}
		block = block->next;
	}

	return count;
}


/*
================
Z_Alloc
================
*/
#ifdef _NOMAD_DEBUG
void *Z_AllocDebug( uint32_t size, int tag, const char *label, const char *file, unsigned line ) {
	uint32_t allocSize;
#else
void *Z_Alloc( uint32_t size, int tag ) {
#endif
	uint32_t extra;
#ifndef USE_MULTI_SEGMENT
	memblock_t *start, *rover;
#endif
	memblock_t *base;
	memzone_t *zone;

	if ( tag == TAG_FREE ) {
		N_Error( ERR_FATAL, "Z_Malloc: tried to use with TAG_FREE" );
	}
	if (tag == TAG_SMALL) {
		zone = smallzone;
	} else {
		zone = mainzone;
	}

#ifdef _NOMAD_DEBUG
	allocSize = size;
#endif

#ifdef USE_MULTI_SEGMENT
	if ( size < (sizeof( freeblock_t ) ) ) {
		size = (sizeof( freeblock_t ) );
	}
#endif

	//
	// scan through the block list looking for the first free block
	// of sufficient size
	//
	size += sizeof( *base );	// account for size of block header
#ifdef USE_TRASH_TEST
	size += 4;					// space for memory trash tester
#endif

	size = PAD(size, sizeof(uintptr_t));		// align to 32/64 bit boundary

#ifdef USE_MULTI_SEGMENT
	base = SearchFree( zone, size );

	RemoveFree( base );
#else

	base = rover = zone->rover;
	start = base->prev;

	do {
		if ( rover == start ) {
			// scanned all the way around the list
#ifdef _NOMAD_DEBUG
			Z_LogHeap();
			N_Error( ERR_FATAL, "Z_Malloc: failed on allocation of %i bytes from the %s zone: %s, line: %d (%s)",
								size, zone == smallzone ? "small" : "main", file, line, label );
#else
			N_Error( ERR_FATAL, "Z_Malloc: failed on allocation of %i bytes from the %s zone",
								size, zone == smallzone ? "small" : "main" );
#endif
			return NULL;
		}
		if ( rover->tag != TAG_FREE ) {
			base = rover = rover->next;
		} else {
			rover = rover->next;
		}
	} while (base->tag != TAG_FREE || base->size < size);
#endif

	//
	// found a block big enough
	//
	extra = base->size - size;
	if ( extra >= minfragment ) {
		memblock_t *fragment;
		// there will be a free fragment after the allocated block
		fragment = (memblock_t *)( (byte *)base + size );
		fragment->size = extra;
		fragment->tag = TAG_FREE; // free block
		fragment->id = ZONEID;
		fragment->prev = base;
		fragment->next = base->next;
		fragment->next->prev = fragment;
		base->next = fragment;
		base->size = size;
#ifdef USE_MULTI_SEGMENT
		InsertFree( zone, fragment );
#endif
	}

#ifndef USE_MULTI_SEGMENT
	zone->rover = base->next;	// next allocation will start looking here
#endif
	zone->used += base->size;

	base->tag = tag;			// no longer a free block
	base->id = ZONEID;

#ifdef _NOMAD_DEBUG
	base->d.label = label;
	base->d.file = file;
	base->d.line = line;
	base->d.allocSize = allocSize;
#endif

#ifdef USE_TRASH_TEST
	// marker for memory trash testing
	*(int *)((byte *)base + base->size - 4) = ZONEID;
#endif

	return (void *) ( base + 1 );
}

/*
========================
Z_Malloc
========================
*/
#ifdef _NOMAD_DEBUG
void *Z_MallocDebug(uint32_t size, int tag, const char *label, const char *file, unsigned line) {
	return Z_AllocDebug(size, tag, label, file, line);
}
#else
void *Z_Malloc(uint32_t size, int tag) {
	return Z_Alloc(size, tag);
}
#endif

/*
========================
Z_SMalloc
========================
*/
#ifdef _NOMAD_DEBUG
void *Z_SMallocDebug( uint32_t size, const char *label, const char *file, unsigned line ) {
	return Z_AllocDebug( size, TAG_SMALL, label, file, line );
}
#else
void *Z_SMalloc( uint32_t size ) {
	return Z_Alloc( size, TAG_SMALL );
}
#endif


/*
========================
Z_CheckHeap
========================
*/
void Z_CheckHeap( void )
{
	const memblock_t *block;
	const memzone_t *zone;

	zone =  mainzone;
	for ( block = zone->blocklist.next ; ; ) {
		if ( block->next == &zone->blocklist ) {
			break;	// all blocks have been hit
		}
		if ( (byte *)block + block->size != (byte *)block->next) {
#ifdef USE_MULTI_SEGMENT
			const memblock_t *next = block->next;
			if ( next->size == 0 && next->id == -ZONEID && next->tag == TAG_GENERAL ) {
				block = next; // new zone segment
			} else
#endif
			N_Error( ERR_FATAL, "Z_CheckHeap: block size does not touch the next block" );
		}
		if ( block->next->prev != block) {
			N_Error( ERR_FATAL, "Z_CheckHeap: next block doesn't have proper back link" );
		}
		if ( block->tag == TAG_FREE && block->next->tag == TAG_FREE ) {
			N_Error( ERR_FATAL, "Z_CheckHeap: two consecutive free blocks" );
		}
		block = block->next;
	}
}


/*
========================
Z_LogZoneHeap
========================
*/
void Z_LogZoneHeap( memzone_t *zone, const char *name )
{
#ifdef _NOMAD_DEBUG
	char dump[32], *ptr;
	int  i, j;
#endif
	memblock_t	*block;
	char buf[4096];
	int size, allocSize, numBlocks;
	int len;

	if ( logfile == FS_INVALID_HANDLE || !FS_Initialized() )
		return;

	size = numBlocks = 0;
#ifdef _NOMAD_DEBUG
	allocSize = 0;
#endif
	len = Com_snprintf( buf, sizeof(buf), "\r\n================\r\n%s log\r\n================\r\n", name );
	FS_Write( buf, len, logfile );
	for ( block = zone->blocklist.next ; ; ) {
		if ( block->tag != TAG_FREE ) {
#ifdef _NOMAD_DEBUG
			ptr = ((char *) block) + sizeof(memblock_t);
			j = 0;
			for (i = 0; i < 20 && i < block->d.allocSize; i++) {
				if (ptr[i] >= 32 && ptr[i] < 127) {
					dump[j++] = ptr[i];
				}
				else {
					dump[j++] = '_';
				}
			}
			dump[j] = '\0';
			len = Com_snprintf(buf, sizeof(buf), "size = %8d: %s, line: %d (%s) [%s]\r\n", block->d.allocSize, block->d.file, block->d.line, block->d.label, dump);
			FS_Write( buf, len, logfile );
			allocSize += block->d.allocSize;
#endif
			size += block->size;
			numBlocks++;
		}
		if ( block->next == &zone->blocklist ) {
			break; // all blocks have been hit
		}
		block = block->next;
	}
#ifdef _NOMAD_DEBUG
	// subtract debug memory
	size -= numBlocks * sizeof(zonedebug_t);
#else
	allocSize = numBlocks * sizeof(memblock_t); // + 32 bit alignment
#endif
	len = Com_snprintf( buf, sizeof( buf ), "%d %s memory in %d blocks\r\n", size, name, numBlocks );
	FS_Write( buf, len, logfile );
	len = Com_snprintf( buf, sizeof( buf ), "%d %s memory overhead\r\n", size - allocSize, name );
	FS_Write( buf, len, logfile );
	FS_Flush( logfile );
}

/*
========================
Z_LogHeap
========================
*/
void Z_LogHeap( void ) {
	Z_LogZoneHeap( mainzone, "MAIN" );
	Z_LogZoneHeap( smallzone, "SMALL" );
}

void Z_InitSmallZoneMemory(void)
{
	static byte zoneBuf[512 * 1024];
	uint64_t size;

	size = sizeof(zoneBuf);
	memset(zoneBuf, 0, size);
	smallzone = (memzone_t *)zoneBuf;
	Z_ClearZone(smallzone, smallzone, size, 1);
}

void Z_InitMemory(void)
{
	uint64_t mainsize;
	cvar_t *cv;

	// Please note: com_zoneMegs can only be set on the command line, and
	// not in q3config.cfg or Com_StartupVariable, as they haven't been
	// executed by this point. It's a chicken and egg problem. We need the
	// memory manager configured to handle those places where you would
	// configure the memory manager.

	// allocate the random block zone
	cv = Cvar_Get("com_zoneMegs", STR(MAINZONE_DEFSIZE), CVAR__LATCH | CVAR_SAVE);
	Cvar_CheckRange( cv, "1", NULL, CV_INTEGER );
	Cvar_SetDescription( cv, "Initial amount of memory (RAM) allocated for the main block zone (in MB)." );

	mainsize = cv->i * 1024 * 1024;

	mainzone = (memzone_t *)calloc(mainsize, 1);
	if (!mainzone) {
		N_Error(ERR_FATAL, "Main zone memory segment failed to allocate %lu megs", mainsize / (1024*1024));
	}
	Z_ClearZone(mainzone, mainzone, mainsize, 1);
}
