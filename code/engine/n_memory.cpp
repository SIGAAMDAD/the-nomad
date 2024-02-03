#include "n_shared.h"
#include "n_common.h"
#include "../game/g_game.h"
#include "../system/sys_thread.h"

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

#define GB(x) (1024*1024*1024*((x)*0.5))

#define HUNK_DEFSIZE 1024
#define HUNK_MINSIZE 72

// tunables
#define USE_MEMSTATIC
#define USE_MULTI_SEGMENT
#define USE_TRASH_TEST

// 100 MiB
#define MAINZONE_DEFSIZE 56
#define MAINZONE_MINSIZE 46
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

#ifdef _NOMAD_DEBUG
typedef struct zonedebug_s {
	const char *label;
	const char *file;
	unsigned line;
	uint64_t allocSize;
} zonedebug_t;
#endif

typedef struct memblock_s {
	struct memblock_s	*next, *prev;
	uint64_t	size;	// including the header and possibly tiny fragments
	uint32_t	tag;	// a tag of 0 is a free block
	uint32_t	id;		// should be ZONEID
#ifdef _NOMAD_DEBUG
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

static CThreadMutex commonLock;

uint64_t hunksize;
byte* hunkbase;
extern fileHandle_t logfile;

#define HUNKID 0x553dfa2
#define HUNKFREE 0x0ffa3d

typedef struct hunkblock_s
{
	uint64_t id;
	uint64_t size;
	struct hunkblock_s *next;
	const char *name;
} hunkblock_t;

typedef struct
{
	uint64_t id;
	uint64_t size;
} hunkHeader_t;

static hunkblock_t *hunkblocks;

typedef struct
{
	uint64_t mark;			// temp + permanent
	uint64_t permanent;		// permanent memory (can only be reset after game restart)
	uint64_t temp;			// temp memory (can be deallocated at any time)
	uint64_t tempHighwater;
} hunkUsed_t;

CThreadMutex hunkLock, allocLock;
uint64_t hunk_low_used = 0;
uint64_t hunk_high_used = 0;
uint64_t hunk_temp_used = 0;

hunkUsed_t *hunk_permanent;
hunkUsed_t *hunk_temp;
hunkUsed_t hunk_low;
hunkUsed_t hunk_high;

static uint64_t minfragment = MIN_FRAGMENT; // may be adjusted at runtime

// main zone for all "dynamic" memory allocation
static memzone_t *mainzone;

// we also have a small zone for small allocations that would only
// fragment the main zone (think of cvar and cmd strings)
static memzone_t *smallzone;

static void Z_LogHeap(void);

void* operator new[](size_t size, const char* pName, int flags, unsigned debugFlags, const char* file, int line)
{
	return ::operator new[](size);
}
void* operator new[](size_t size, size_t alignment, size_t alignmentOffset, const char* pName, int flags, unsigned debugFlags, const char* file, int line)
{
	return ::operator new[](size, std::align_val_t(alignment));
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
		N_Error( ERR_FATAL, "InsertFree: bad block size: %lu\n", block->size );
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
static freeblock_t *NewBlock( memzone_t *zone, uint64_t size )
{
	memblock_t *prev, *next;
	memblock_t *block, *sep;
	uint64_t alloc_size;

	// zone->prev is pointing on last block in the list
	prev = zone->blocklist.prev;
	next = prev->next;

	size = PAD( size, 1<<21 ); // round up to 2M blocks
	// allocate separator block before new free block
	alloc_size = size + sizeof( *sep );

	sep = (memblock_t *) calloc( alloc_size, 1 );
	if ( sep == NULL ) {
		Sys_SetError( ERR_OUT_OF_MEMORY );
		N_Error( ERR_FATAL, "Z_Malloc: failed on allocation of %lu bytes from the %s zone",
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

	sep->tag = TAG_STATIC; // in-use block
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


static memblock_t *SearchFree( memzone_t *zone, uint64_t size )
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
static void Z_ClearZone( memzone_t *zone, memzone_t *head, uint64_t size, uint64_t segnum )
{
	memblock_t	*block;
	uint64_t min_fragment;

#ifdef USE_MULTI_SEGMENT
	min_fragment = sizeof( memblock_t ) + sizeof( freeblock_t );
#else
	min_fragment = sizeof( memblock_t );
#endif

	if ( minfragment < min_fragment ) {
		// in debug mode size of memblock_t may exceed MIN_FRAGMENT
		minfragment = PAD( min_fragment, sizeof( intptr_t ) );
		Con_DPrintf( "zone.minfragment adjusted to %lu bytes\n", minfragment );
	}

	// set the entire zone to one free block
	zone->blocklist.next = zone->blocklist.prev = block = (memblock_t *)( zone + 1 );
	zone->blocklist.tag = TAG_STATIC; // in use block
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

#ifdef USE_STATIC_TAGS
// static mem blocks to reduce a lot of small zone overhead
typedef struct memstatic_s {
	memblock_t b;
	byte mem[2];
} memstatic_t;

#define MEM_STATIC(chr) { { NULL, NULL, PAD(sizeof(memstatic_t),4), TAG_STATIC, ZONEID }, {chr,'\0'} }

static const memstatic_t emptystring =
	MEM_STATIC( '\0' );

static const memstatic_t numberstring[] = {
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
#endif // USE_STATIC_TAGS

/*
========================
Z_Strdup

 NOTE:	never write over the memory Z_Strdup returns because
		memory from a memstatic_t might be returned
========================
*/
char *CopyString( const char *in )
{
	char *out;
#ifdef USE_STATIC_TAGS
	if ( in[0] == '\0' ) {
		return ((char *)&emptystring) + sizeof(memblock_t);
	}
	else if ( in[0] >= '0' && in[0] <= '9' && in[1] == '\0' ) {
		return ((char *)&numberstring[in[0]-'0']) + sizeof(memblock_t);
	}
#endif
	out = (char *)S_Malloc( strlen( in ) + 1 );
	strcpy( out, in );
	return out;
}



/*
========================
Z_AvailableZoneMemory
========================
*/
static uint64_t Z_AvailableZoneMemory( const memzone_t *zone ) {
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
uint64_t Z_AvailableMemory( void ) {
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
	if ( *(int32_t *)((byte *)block + block->size - 4 ) != ZONEID ) {
		N_Error( ERR_FATAL, "Z_Free: memory block wrote past end" );
	}
#endif

	if ( block->tag == TAG_SMALL ) {
		zone = smallzone;
	} else {
		zone = mainzone;
	}

	CThreadAutoLock lock( allocLock );
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
uint64_t Z_FreeTags( memtag_t lowtag, memtag_t hightag )
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
void *Z_AllocDebug( uint64_t size, memtag_t tag, const char *label, const char *file, uint32_t line ) {
	uint64_t allocSize;
#else
void *Z_Alloc( uint64_t size, memtag_t tag ) {
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

	CThreadAutoLock lock( allocLock );

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
			N_Error( ERR_FATAL, "Z_Malloc: failed on allocation of %lu bytes from the %s zone: %s, line: %d (%s)",
								size, zone == smallzone ? "small" : "main", file, line, label );
#else
			N_Error( ERR_FATAL, "Z_Malloc: failed on allocation of %lu bytes from the %s zone",
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
	*(int32_t *)((byte *)base + base->size - 4) = ZONEID;
#endif

	return (void *) ( base + 1 );
}

#ifdef _NOMAD_DEBUG
void *Z_ReallocDebug(void *ptr, uint64_t nsize, memtag_t tag, const char *label, const char *file, uint32_t line) {
	void *p;

	p = Z_AllocDebug(nsize, tag, label, file, line);
	if (ptr) {
		memblock_t *block = (memblock_t *)((byte *)ptr - sizeof(memblock_t));
		memcpy(p, ptr, block->size <= nsize ? nsize : block->size);
		Z_Free(ptr);
	}
	return p;
}
#else
void *Z_Realloc(void *ptr, uint64_t nsize, memtag_t tag) {
	void *p;

	p = Z_Alloc(nsize, tag);
	if (ptr) {
		memblock_t *block = (memblock_t *)((byte *)ptr - sizeof(memblock_t));
		memcpy(p, ptr, block->size <= nsize ? nsize : block->size);
		Z_Free(ptr);
	}
	return p;
}
#endif

/*
========================
Z_Malloc
========================
*/
#ifdef _NOMAD_DEBUG
void *Z_MallocDebug( uint64_t size, memtag_t tag, const char *label, const char *file, uint32_t line ) {
	return Z_AllocDebug(size, tag, label, file, line);
}
#else
void *Z_Malloc( uint64_t size, memtag_t tag ) {
	return Z_Alloc(size, tag);
}
#endif

/*
========================
Z_SMalloc
========================
*/
#ifdef _NOMAD_DEBUG
void *S_MallocDebug( uint64_t size, const char *label, const char *file, uint32_t line ) {
	return Z_AllocDebug( size, TAG_SMALL, label, file, line );
}
#else
void *S_Malloc( uint64_t size ) {
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
			if ( next->size == 0 && next->id == -ZONEID && next->tag == TAG_STATIC ) {
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
	uint32_t i, j;
#endif
	memblock_t	*block;
	char buf[4096];
	uint64_t size, allocSize, numBlocks;
	uint64_t len;

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
			len = Com_snprintf(buf, sizeof(buf), "size = %-8lu: %-8s, line: %4u (%s) [%s]\r\n", block->d.allocSize, block->d.file, block->d.line, block->d.label, dump);
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
	len = Com_snprintf( buf, sizeof( buf ), "%lu %s memory in %lu blocks\r\n", size, name, numBlocks );
	FS_Write( buf, len, logfile );
	len = Com_snprintf( buf, sizeof( buf ), "%lu %s memory overhead\r\n", size - allocSize, name );
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

void Z_InitSmallZoneMemory(void) {
	//static byte zoneBuf[512 * 1024];
	byte *zoneBuf = (byte *)malloc( 512 * 1024 );
	if (!zoneBuf) {
		Sys_Error( "Z_InitSmallZoneMemory: failed to allocate small zone heap of %i bytes", 512 * 1024 );
	}
	uint64_t size;

	size = 512 * 1024;
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
	cv = Cvar_Get("com_zoneMegs", VSTR(MAINZONE_DEFSIZE), CVAR_LATCH | CVAR_SAVE);
	Cvar_CheckRange( cv, "1", NULL, CVT_INT );
	Cvar_SetDescription( cv, "Initial amount of memory (RAM) allocated for the main block zone (in MB)." );

	mainsize = cv->i * 1024 * 1024;

	mainzone = (memzone_t *)calloc(mainsize, 1);
	if (!mainzone) {
		Sys_SetError( ERR_OUT_OF_MEMORY );
		N_Error(ERR_FATAL, "Main zone memory segment failed to allocate %lu megs", mainsize / (1024*1024));
	}
	Z_ClearZone(mainzone, mainzone, mainsize, 1);
}

static const char *tagName[ TAG_COUNT ] = {
    "TAG_FREE",
	"TAG_STATIC",
	"TAG_BFF",
	"TAG_SEARCH_PATH",
	"TAG_RENDERER",
	"TAG_GAME",
	"TAG_SMALL",
	"TAG_SFX",
	"TAG_MUSIC",
	"TAG_HUNK"
};

typedef struct zone_stats_s {
	uint64_t zoneSegments;
	uint64_t zoneBlocks;
	uint64_t zoneBytes;
	uint64_t rendererBytes;
	uint64_t gameBytes;
	uint64_t imguiBytes;
	uint64_t resourceBytes;
	uint64_t filesystemBytes;
	uint64_t hunkLeftOverBytes;
	uint64_t freeBytes;
	uint64_t freeBlocks;
	uint64_t freeSmallest;
	uint64_t freeLargest;
} zone_stats_t;

static void Zone_Stats( const char *name, const memzone_t *z, qboolean printDetails, zone_stats_t *stats )
{
	const memblock_t *block;
	const memzone_t *zone;
	zone_stats_t st;

	memset( &st, 0, sizeof( st ) );
	zone = z;
	st.zoneSegments = 1;
	st.freeSmallest = 0x7FFFFFFF;

	//if ( printDetails ) {
	//	Con_Printf( "---------- %s zone segment #%i ----------\n", name, zone->segnum );
	//}

	for ( block = zone->blocklist.next ; ; ) {
		if ( printDetails ) {
			int tag = block->tag;
			Con_Printf( "block:%p  size:%8lu  tag: %s\n", (void *)block, block->size,
				(unsigned)tag < TAG_COUNT ? tagName[ tag ] : va( "%i", tag ) );
		}
		if ( block->tag != TAG_FREE ) {
			st.zoneBytes += block->size;
			st.zoneBlocks++;
			switch ( block->tag ) {
			case TAG_RENDERER:
				st.rendererBytes += block->size;
				break;
			case TAG_BFF:
				st.resourceBytes += block->size;
				break;
			case TAG_SEARCH_DIR:
			case TAG_SEARCH_PATH:
				st.filesystemBytes += block->size;
				break;
			case TAG_GAME:
				st.gameBytes += block->size;
				break;
			case TAG_HUNK:
				st.hunkLeftOverBytes += block->size;
				break;
			case TAG_IMGUI:
				st.imguiBytes += block->size;
				break;
			};
		} else {
			st.freeBytes += block->size;
			st.freeBlocks++;
			if ( block->size > st.freeLargest )
				st.freeLargest = block->size;
			if ( block->size < st.freeSmallest )
				st.freeSmallest = block->size;
		}
		if ( block->next == &zone->blocklist ) {
			break; // all blocks have been hit
		}
		if ( (byte *)block + block->size != (byte *)block->next) {
#ifdef USE_MULTI_SEGMENT
			const memblock_t *next = block->next;
			if ( next->size == 0 && next->id == -ZONEID && next->tag == TAG_STATIC ) {
				st.zoneSegments++;
				if ( printDetails ) {
					Con_Printf( "---------- %s zone segment #%lu ----------\n", name, st.zoneSegments );
				}
				block = next->next;
				continue;
			} else
#endif
				Con_Printf( "ERROR: block size does not touch the next block\n" );
		}
		if ( block->next->prev != block) {
			Con_Printf( "ERROR: next block doesn't have proper back link\n" );
		}
		if ( block->tag == TAG_FREE && block->next->tag == TAG_FREE ) {
			Con_Printf( "ERROR: two consecutive free blocks\n" );
		}
		block = block->next;
	}

	// export stats
	if ( stats ) {
		memcpy( stats, &st, sizeof( *stats ) );
	}
}

/*
=================
Com_Meminfo_f
=================
*/
static void Com_Meminfo_f( void )
{
	zone_stats_t st;
	uint64_t unused;

	Con_Printf( "%8lu bytes total hunk\n", hunksize );
	Con_Printf( "\n" );
	Con_Printf( "%8lu low mark\n", hunk_low.mark );
	Con_Printf( "%8lu low permanent\n", hunk_low.permanent );
	if ( hunk_low.temp != hunk_low.permanent ) {
		Con_Printf( "%8li low temp\n", hunk_low.temp );
	}
	Con_Printf( "%8lu low tempHighwater\n", hunk_low.tempHighwater );
	Con_Printf( "\n" );
	Con_Printf( "%8lu high mark\n", hunk_high.mark );
	Con_Printf( "%8lu high permanent\n", hunk_high.permanent );
	if ( hunk_high.temp != hunk_high.permanent ) {
		Con_Printf( "%8lu high temp\n", hunk_high.temp );
	}
	Con_Printf( "%8lu high tempHighwater\n", hunk_high.tempHighwater );
	Con_Printf( "\n" );
	Con_Printf( "%8lu total hunk in use\n", hunk_low.permanent + hunk_high.permanent );
	unused = 0;
	if ( hunk_low.tempHighwater > hunk_low.permanent ) {
		unused += hunk_low.tempHighwater - hunk_low.permanent;
	}
	if ( hunk_high.tempHighwater > hunk_high.permanent ) {
		unused += hunk_high.tempHighwater - hunk_high.permanent;
	}
	Con_Printf( "%8lu unused highwater\n", unused );
	Con_Printf( "\n" );

	Zone_Stats( "main", mainzone, !N_stricmp( Cmd_Argv(1), "main" ) || !N_stricmp( Cmd_Argv(1), "all" ), &st );
	Con_Printf( "%8lu bytes total main zone\n\n", mainzone->size );
	Con_Printf( "%8lu bytes in %lu main zone blocks%s\n", st.zoneBytes, st.zoneBlocks,
		st.zoneSegments > 1 ? va( " and %lu segments", st.zoneSegments ) : "" );
	Con_Printf( "        %8lu bytes in renderer\n", st.rendererBytes );
	Con_Printf( "        %8lu bytes in game\n", st.gameBytes );
	Con_Printf( "        %8lu bytes in imgui\n", st.imguiBytes );
	Con_Printf( "        %8lu bytes in filesystem\n", st.filesystemBytes );
	Con_Printf( "        %8lu bytes in resources\n", st.resourceBytes );
	Con_Printf( "        %8lu bytes in leaked hunk memory\n", st.hunkLeftOverBytes );
	Con_Printf( "        %8lu bytes in other\n", st.zoneBytes - ( st.rendererBytes + st.gameBytes + st.imguiBytes + st.filesystemBytes +
																	st.hunkLeftOverBytes + st.resourceBytes ) );
	Con_Printf( "        %8lu bytes in %lu free blocks\n", st.freeBytes, st.freeBlocks );
	if ( st.freeBlocks > 1 ) {
		Con_Printf( "        (largest: %lu bytes, smallest: %lu bytes)\n\n", st.freeLargest, st.freeSmallest );
	}

	Zone_Stats( "small", smallzone, !N_stricmp( Cmd_Argv(1), "small" ) || !N_stricmp( Cmd_Argv(1), "all" ), &st );
	Con_Printf( "%8lu bytes total small zone\n\n", smallzone->size );
	Con_Printf( "%8lu bytes in %lu small zone blocks%s\n", st.zoneBytes, st.zoneBlocks,
		st.zoneSegments > 1 ? va( " and %lu segments", st.zoneSegments ) : "" );
	Con_Printf( "        %8lu bytes in %lu free blocks\n", st.freeBytes, st.freeBlocks );
	if ( st.freeBlocks > 1 ) {
		Con_Printf( "        (largest: %lu bytes, smallest: %lu bytes)\n\n", st.freeLargest, st.freeSmallest );
	}
}

/*
===============
Com_TouchMemory

Touch all known used data to make sure it is paged in
===============
*/
uint64_t Com_TouchMemory( void ) {
	const memblock_t *block;
	const memzone_t *zone;
	uint32_t i, j;
    uint64_t sum;
	CTimer timer;

	Z_CheckHeap();

//	start = Sys_Milliseconds();
	timer.Run();

	sum = 0;

	j = hunk_low.permanent >> 2;
	for ( i = 0 ; i < j ; i+=64 ) {			// only need to touch each page
		sum += ((uint32_t *)hunkbase)[i];
	}

	i = ( hunksize - hunk_high.permanent ) >> 2;
	j = hunk_high.permanent >> 2;
	for (  ; i < j ; i+=64 ) {			// only need to touch each page
		sum += ((uint32_t *)hunkbase)[i];
	}

	zone = mainzone;
	for (block = zone->blocklist.next ; ; block = block->next) {
		if ( block->tag != TAG_FREE ) {
			j = block->size >> 2;
			for ( i = 0 ; i < j ; i+=64 ) {				// only need to touch each page
				sum += ((uint32_t *)block)[i];
			}
		}
		if ( block->next == &zone->blocklist ) {
			break; // all blocks have been hit
		}
	}

	timer.Stop();
//	end = Sys_Milliseconds();

	Con_Printf( "Com_TouchMemory: %5.5lf msec\n", (double)timer.ElapsedMilliseconds().count() );

	return sum; // just to silent compiler warning
}

/*
==============================================================================

Goals:
	reproducible without history effects -- no out of memory errors on weird map to map changes
	allow restarting of the client without fragmentation
	minimize total pages in use at run time
	minimize total pages needed during load time

  Single block of memory with stack allocators coming from both ends towards the middle.

  One side is designated the temporary memory allocator.

  Temporary memory can be allocated and freed in any order.

  A highwater mark is kept of the most in use at any time.

  When there is no temporary memory allocated, the permanent and temp sides
  can be switched, allowing the already touched temp memory to be used for
  permanent storage.

  Temp memory must never be allocated on two ends at once, or fragmentation
  could occur.

  If we have any in-use temp memory, additional temp allocations must come from
  that side.

  If not, we can choose to make either side the new temp side and push future
  permanent allocations to the other side.  Permanent allocations should be
  kept on the side that has the current greatest wasted highwater mark.

==============================================================================
*/


/*
Hunk_Clear: gets called whenever a new level is loaded or is being shutdown
*/
void Hunk_Clear(void)
{
	CThreadAutoLock lock( hunkLock );

	G_ShutdownUI();
	G_ShutdownSGame();

	hunk_low.tempHighwater = 0;
	hunk_low.mark = 0;
	hunk_low.temp = 0;
	hunk_low.permanent = 0;

	hunk_high.tempHighwater = 0;
	hunk_high.mark = 0;
	hunk_high.temp = 0;
	hunk_high.permanent = 0;
	
	hunk_permanent = &hunk_low;
	hunk_temp = &hunk_high;

	Con_Printf("Hunk_Clear: reset the hunk ok\n");
	VM_Clear();

	hunkblocks = NULL;
}

uint64_t Hunk_MemoryRemaining( void )
{
	uint64_t low, high;

	low = hunk_low.permanent > hunk_low.temp ? hunk_low.permanent : hunk_low.temp;
	high = hunk_high.permanent > hunk_high.temp ? hunk_high.permanent : hunk_high.temp;

	return hunksize - ( low + high );
}

/*
Hunk_SetMark: gets called after level and game vm have been loaded
*/
void Hunk_SetMark( void )
{
	CThreadAutoLock lock( hunkLock );
	hunk_low.mark = hunk_low.permanent;
	hunk_high.mark = hunk_high.permanent;
}

void Hunk_ClearToMark( void )
{
	CThreadAutoLock lock( hunkLock );
	hunk_low.permanent = hunk_low.temp = hunk_low.mark;
	hunk_high.permanent = hunk_high.temp = hunk_high.mark;
}

static void Hunk_SwapBanks(void)
{
	hunkUsed_t	*swap;

	// can't swap banks if there is any temp already allocated
	if ( hunk_temp->temp != hunk_temp->permanent ) {
		return;
	}

	CThreadAutoLock lock( hunkLock );

	// if we have a larger highwater mark on this side, start making
	// our permanent allocations here and use the other side for temp
	if ( hunk_temp->tempHighwater - hunk_temp->permanent >
		hunk_permanent->tempHighwater - hunk_permanent->permanent ) {
		swap = hunk_temp;
		hunk_temp = hunk_permanent;
		hunk_permanent = swap;
	}
}

void *Hunk_AllocateTempMemory(uint64_t size)
{
	void *buf;
	hunkHeader_t *h;

	// if the hunk hasn't been initialized yet, but there are filesystem calls
	// being made, then just allocate with Z_Malloc
	if (hunkbase == NULL) {
		return Z_Malloc(size, TAG_HUNK);
	}

	Hunk_SwapBanks();
	
	size = PAD(size, sizeof(intptr_t)) + sizeof(hunkHeader_t);

	if (hunk_temp->temp + hunk_permanent->permanent + size > hunksize) {
		N_Error(ERR_DROP, "Hunk_AllocateTempMemory: failed on %lu", size);
	}

	CThreadAutoLock lock( hunkLock );

	if ( hunk_temp == &hunk_low ) {
		buf = (void *)(hunkbase + hunk_temp->temp);
		hunk_temp->temp += size;
	}
	else {
		hunk_temp->temp += size;
		buf = (void *)(hunkbase + hunksize - hunk_temp->temp );
	}

	if ( hunk_temp->temp > hunk_temp->tempHighwater ) {
		hunk_temp->tempHighwater = hunk_temp->temp;
	}

	h = (hunkHeader_t *)buf;
	buf = (void *)(h+1);

	h->id = HUNKID;
	h->size = size;

	// don't bother clearing, because we are going to load a file over it
	return buf;
}

void Hunk_FreeTempMemory(void *p)
{
	hunkHeader_t *h;

	// if hunk hasn't been initialized yet, just Z_Free the data
	if (hunkbase == NULL) {
		Z_Free(p);
		return;
	}

	h = ((hunkHeader_t *)p) - 1;
	if (h->id != HUNKID) {
		N_Error (ERR_FATAL, "Hunk_FreeTempMemory: hunk id isn't correct");
	}

	h->id = HUNKFREE;

	CThreadAutoLock lock( hunkLock );

	// this only works if the files are freed in the stack order,
	// otherwise the memory will stay around until Hunk_ClearTempMemory
	if ( hunk_temp == &hunk_low ) {
		if ( h == (void *)(hunkbase + hunk_temp->temp - h->size ) )
			hunk_temp->temp -= h->size;
		else
			Con_Printf( "Hunk_FreeTempMemory: not the final block\n" );
	}
	else {
		if ( h == (void *)(hunkbase + hunksize - hunk_temp->temp ) )
			hunk_temp->temp -= h->size;
		else
			Con_Printf( "Hunk_FreeTempMemory: not the final block\n" );
	}
}

void Hunk_ClearTempMemory(void)
{
	CThreadAutoLock lock( hunkLock );
	if (hunkbase) {
		hunk_temp->temp = hunk_temp->permanent;
	}
}

qboolean Hunk_TempIsClear(void)
{
	return (qboolean)(hunk_temp->temp == hunk_temp->permanent);
}

qboolean Hunk_CheckMark(void)
{
	if (hunk_low.mark || hunk_high.mark)
		return qtrue;
	
	return qfalse;
}

/*
Hunk_Alloc: allocates a chunk of memory out of the hunk heap in the ha_pref (h_low, h_high)
*/
#ifdef _NOMAD_DEBUG
void *Hunk_AllocDebug (uint64_t size, ha_pref where, const char *name, const char *file, uint64_t line)
#else
void *Hunk_Alloc (uint64_t size, const char *name, ha_pref where)
#endif
{
	void *buf;

	if (!hunkbase) {
		N_Error(ERR_FATAL, "Hunk_Alloc: Hunk memory system not initialized");
	}

	if (!size)
		N_Error(ERR_FATAL, "Hunk_Alloc: bad size");
	
	CThreadAutoLock lock( hunkLock );
	if (where == h_dontcare || hunk_temp->temp != hunk_temp->permanent) {
		Hunk_SwapBanks();
	}
	else {
		if (where == h_low && hunk_permanent != &hunk_low) {
			Hunk_SwapBanks();
		}
		else if (where == h_high && hunk_permanent != &hunk_high) {
			Hunk_SwapBanks();
		}
	}

#ifdef _NOMAD_DEBUG
	size += sizeof(hunkblock_t);
#endif

	// round to the cacheline
	size = PAD(size, com_cacheLine);

	if ( hunk_low.temp + hunk_high.temp + size > hunksize ) {
#ifdef _NOMAD_DEBUG
		Hunk_Log();
		Hunk_SmallLog();

		N_Error(ERR_DROP, "Hunk_Alloc failed on %lu: %s, line: %lu (%s)", size, file, line, name);
#else
		N_Error(ERR_DROP, "Hunk_Alloc failed on %lu", size);
#endif
	}

	if ( hunk_permanent == &hunk_low ) {
		buf = (void *)(hunkbase + hunk_permanent->permanent);
		hunk_permanent->permanent += size;
	}
	else {
		hunk_permanent->permanent += size;
		buf = (void *)(hunkbase + hunksize - hunk_permanent->permanent );
	}

	hunk_permanent->temp = hunk_permanent->permanent;

	memset( buf, 0, size );

#ifdef _NOMAD_DEBUG
	{
		hunkblock_t *block;

		block = (hunkblock_t *)buf;
		block->size = size - sizeof(hunkblock_t);
		block->name = name;
		block->id = HUNKID;
		block->next = hunkblocks;
		hunkblocks = block;
		buf = ((byte *)buf) + sizeof(hunkblock_t);
	}
#endif

	return buf;
}

void Hunk_SmallLog(void)
{
	hunkblock_t *h;
	char buf[4096];
	uint64_t size, locsize, numBlocks;

	if (!logfile || !FS_Initialized())
		return;
	
	size = 0;
	numBlocks = 0;
	Com_snprintf(buf, sizeof(buf), "\r\n================\r\nHunk Small log\r\n================\r\n");
	FS_Write(buf, strlen(buf), logfile);
	for (h = hunkblocks; h; h = h->next) {
		size += h->size;
		numBlocks++;
		Com_snprintf(buf, sizeof(buf), "size = %8lu: %s\r\n", h->size, h->name);
		FS_Write(buf, strlen(buf), logfile);
	}
	Com_snprintf(buf, sizeof(buf), "%lu total hunk memory\r\n", size);
	FS_Write(buf, strlen(buf), logfile);
	Com_snprintf(buf, sizeof(buf), "%lu total hunk blocks\r\n", numBlocks);
}

void Hunk_Log(void) {
	Hunk_SmallLog();
}

// kept for free later on
static void *hunkorig;

void Hunk_InitMemory(void)
{
    cvar_t *cv;

    hunkorig = NULL;

    // make sure the file system has allocated and "not" freed any temp blocks
	// this allows the config and product id files ( journal files too ) to be loaded
	// by the file system without redundant routines in the file system utilizing different
	// memory systems
	if ( FS_LoadStack() != 0 ) {
		N_Error( ERR_FATAL, "Hunk initialization failed. File system load stack not zero" );
	}

	// allocate the stack based hunk allocator
	cv = Cvar_Get( "com_hunkMegs", VSTR( HUNK_DEFSIZE ), CVAR_LATCH | CVAR_SAVE );
	Cvar_CheckRange( cv, VSTR( HUNK_MINSIZE ), NULL, CVT_INT );
	Cvar_SetDescription( cv, "The size of the hunk memory segment." );

	hunksize = cv->i * 1024 * 1024;
	hunkbase = (byte *)calloc( hunksize + (com_cacheLine - 1), 1 );
	if ( !hunkbase ) {
		Sys_SetError( ERR_OUT_OF_MEMORY );
		N_Error( ERR_FATAL, "Hunk data failed to allocate %lu megs", hunksize / (1024*1024) );
	}

	// cacheline align
    hunkorig = hunkbase; // store the original pointer for free()
	hunkbase = (byte *)PADP( hunkbase, com_cacheLine );
	Hunk_Clear();

	Cmd_AddCommand( "meminfo", Com_Meminfo_f );
	Cmd_AddCommand( "zonelog", Z_LogHeap );
	Cmd_AddCommand( "hunklog", Hunk_Log );
	Cmd_AddCommand( "hunksmalllog", Hunk_SmallLog );
}
