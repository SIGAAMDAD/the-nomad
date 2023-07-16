#include "n_shared.h"

#ifndef USE_LIBC_MALLOC
	#define USE_LIBC_MALLOC		0
#endif

#ifndef CRASH_ON_STATIC_ALLOCATION
//	#define CRASH_ON_STATIC_ALLOCATION
#endif

//===============================================================
//
//	GDRHeap
//
//===============================================================

#define SMALL_HEADER_SIZE		( (uintptr_t) ( sizeof( byte ) + sizeof( byte ) ) )
#define MEDIUM_HEADER_SIZE		( (uintptr_t) ( sizeof( mediumHeapEntry_t ) + sizeof( byte ) ) )
#define LARGE_HEADER_SIZE		( (uintptr_t) ( sizeof( uint32_t * ) + sizeof( byte ) ) )

#define ALIGN_SIZE( bytes )		( ( (bytes) + ALIGN - 1 ) & ~(ALIGN - 1) )
#define SMALL_ALIGN( bytes )	( ALIGN_SIZE( (bytes) + SMALL_HEADER_SIZE ) - SMALL_HEADER_SIZE )
#define MEDIUM_SMALLEST_SIZE	( ALIGN_SIZE( 256 ) + ALIGN_SIZE( MEDIUM_HEADER_SIZE ) )

class GDRHeap
{
public:
	GDRHeap();
	~GDRHeap();							// frees all associated data
	void Init(void);					// initialize
	void* Allocate( const uint32_t bytes );	// allocate memory
	void Free( void *p );				// free memory
	void* Allocate16( const uint32_t bytes );// allocate 16 byte aligned memory
	void Free16( void *p );				// free 16 byte aligned memory
	uint32_t Msize( void *p );				// return size of data block
	void Dump( void  );

	void AllocDefragBlock( void );		// hack for huge renderbumps
	bool DefragIsActive(void);

private:

	enum {
		ALIGN = 8									// memory alignment in bytes
	};

	enum
	{
		INVALID_ALLOC	= 0xdd,
		SMALL_ALLOC		= 0xaa,						// small allocation
		MEDIUM_ALLOC	= 0xbb,						// medium allocaction
		LARGE_ALLOC		= 0xcc						// large allocaction
	};

	// allocation page
	typedef struct page_s
	{
		void* data;						// data pointer to allocated memory
		uint32_t dataSize;				// number of bytes of memory 'data' pouint32_ts to
		struct page_s* next;			// next free page in same page manager
		struct page_s* prev;			// used only when allocated
		uint32_t largestFree;			// this data used by the medium-size heap manager
		void* firstFree;				// pouint32_ter to first free entry
	} page_t;

	typedef struct mediumHeapEntry_s
	{
		page_t* page;							// pointer to page
		uint32_t size;							// size of block
		struct mediumHeapEntry_s* prev;			// previous block
		struct mediumHeapEntry_s* next;			// next block
		struct mediumHeapEntry_s* prevFree;		// previous free block
		struct mediumHeapEntry_s* nextFree;		// next free block
		uint32_t freeBlock;						// non-zero if free block
	} mediumHeapEntry_t;

	// variables
	void* smallFirstFree[256/ALIGN+1];		// small heap allocator lists (for allocs of 1-255 bytes)
	page_t* smallCurPage;					// current page for small allocations
	uint32_t smallCurPageOffset;			// byte offset in current page
	page_t* smallFirstUsedPage;				// first used page of the small heap manager

	page_t* mediumFirstFreePage;			// first partially free page
	page_t* mediumLastFreePage;				// last partially free page
	page_t* mediumFirstUsedPage;			// completely used page

	page_t* largeFirstUsedPage;				// first page used by the large heap manager

	page_t* swapPage;

	uint32_t pagesAllocated;				// number of pages currently allocated
	uint32_t pageSize;						// size of one alloc page in bytes

	uint32_t pageRequests;					// page requests
	uint32_t OSAllocs;						// number of allocs made to the OS

	uint32_t c_heapAllocRunningCount;

	void* defragBlock;						// a single huge block that can be allocated
											// at startup, then freed when needed

	// methods
	page_t* AllocatePage(uint32_t bytes);	// allocate page from the OS
	void FreePage(GDRHeap::page_t *p);		// free an OS allocated page

	void* SmallAllocate(uint32_t bytes);	// allocate memory (1-255 bytes) from small heap manager
	void SmallFree(void *ptr);				// free memory allocated by small heap manager

	void* MediumAllocateFromPage(GDRHeap::page_t *p, uint32_t sizeNeeded);
	void* MediumAllocate(uint32_t bytes);	// allocate memory (256-32768 bytes) from medium heap manager
	void MediumFree(void *ptr);				// free memory allocated by medium heap manager

	void* LargeAllocate(uint32_t bytes);	// allocate large block from OS directly
	void LargeFree(void *ptr);				// free memory allocated by large heap manager

	void ReleaseSwappedPages(void);
	void FreePageReal(GDRHeap::page_t *p);
};

//
// GDRHeap::Init
//
void GDRHeap::Init(void)
{
	OSAllocs			= 0;
	pageRequests		= 0;
	pageSize			= 65536 - sizeof(GDRHeap::page_t);
	pagesAllocated		= 0;								// reset page allocation counter

	largeFirstUsedPage	= NULL;								// init large heap manager
	swapPage			= NULL;

	memset( smallFirstFree, 0, sizeof(smallFirstFree) );	// init small heap manager
	smallFirstUsedPage	= NULL;
	smallCurPage		= AllocatePage( pageSize );
	assert( smallCurPage );
	smallCurPageOffset	= SMALL_ALIGN( 0 );

	defragBlock = NULL;

	mediumFirstFreePage	= NULL;								// init medium heap manager
	mediumLastFreePage	= NULL;
	mediumFirstUsedPage	= NULL;

	c_heapAllocRunningCount = 0;
}

//
// GDRHeap::GDRHeap
//
GDRHeap::GDRHeap(void)
{
	Init();
}

//
// GDRHeap::~GDRHeap: returns all allocated memory back to OS
//
GDRHeap::~GDRHeap(void)
{
	GDRHeap::page_t	*p;

	if ( smallCurPage ) {
		FreePage( smallCurPage );			// free small-heap current allocation page
	}
	p = smallFirstUsedPage;					// free small-heap allocated pages
	while( p ) {
		GDRHeap::page_t *next = p->next;
		FreePage( p );
		p= next;
	}

	p = largeFirstUsedPage;					// free large-heap allocated pages
	while( p ) {
		GDRHeap::page_t *next = p->next;
		FreePage( p );
		p = next;
	}

	p = mediumFirstFreePage;				// free medium-heap allocated pages
	while( p ) {
		GDRHeap::page_t *next = p->next;
		FreePage( p );
		p = next;
	}

	p = mediumFirstUsedPage;				// free medium-heap allocated completely used pages
	while( p ) {
		GDRHeap::page_t *next = p->next;
		FreePage( p );
		p = next;
	}

	ReleaseSwappedPages();

	if ( defragBlock ) {
		free( defragBlock );
	}

	assert( pagesAllocated == 0 );
}

/*
================
GDRHeap::AllocDefragBlock
================
*/
void GDRHeap::AllocDefragBlock(void)
{
	uint32_t		size = 0x40000000;

	if ( defragBlock ) {
		return;
	}
	while( 1 ) {
		defragBlock = malloc( size );
		if ( defragBlock ) {
			break;
		}
		size >>= 1;
	}
	Con_Printf("Allocated a %i mb defrag block", size / (1024*1024) );
}

/*
================
GDRHeap::Allocate
================
*/
void *GDRHeap::Allocate( const uint32_t bytes )
{
	if ( !bytes ) {
		return NULL;
	}
	c_heapAllocRunningCount++;

#if USE_LIBC_MALLOC
	return malloc( bytes );
#else
	if ( !(bytes & ~255) ) {
		return SmallAllocate( bytes );
	}
	if ( !(bytes & ~32767) ) {
		return MediumAllocate( bytes );
	}
	return LargeAllocate( bytes );
#endif
}

/*
================
GDRHeap::Free
================
*/
void GDRHeap::Free( void *p )
{
	if ( !p ) {
		return;
	}
	c_heapAllocRunningCount--;

#if USE_LIBC_MALLOC
	free( p );
#else
	switch( ((byte *)(p))[-1] ) {
		case SMALL_ALLOC: {
			SmallFree( p );
			break;
		}
		case MEDIUM_ALLOC: {
			MediumFree( p );
			break;
		}
		case LARGE_ALLOC: {
			LargeFree( p );
			break;
		}
		default: {
			N_Error( "GDRHeap::Free: invalid memory block" );
			break;
		}
	}
#endif
}

/*
================
GDRHeap::Allocate16
================
*/
void *GDRHeap::Allocate16( const uint32_t bytes )
{
	byte *ptr, *alignedPtr;

	ptr = (byte *) malloc( bytes + 16 + sizeof(uintptr_t) );
	if ( !ptr ) {
		if ( defragBlock ) {
			Con_Printf("Freeing defragBlock on alloc of %i.", bytes );
			free( defragBlock );
			defragBlock = NULL;
			ptr = (byte *) malloc( bytes + 16 + sizeof(uintptr_t) );
			AllocDefragBlock();
		}
		if ( !ptr ) {
			N_Error( "malloc failure for %i", bytes );
		}
	}
	alignedPtr = (byte *) ( ( ( (uintptr_t) ptr ) + 15) & ~15 );
	if ( alignedPtr - ptr < sizeof(uintptr_t) ) {
		alignedPtr += 16;
	}
	*((uintptr_t *)(alignedPtr - sizeof(uintptr_t))) = (uintptr_t) ptr;
	return (void *) alignedPtr;
}

/*
================
GDRHeap::Free16
================
*/
void GDRHeap::Free16( void *p )
{
	free( (void *) *((uintptr_t *) (( (byte *) p ) - sizeof(uintptr_t))) );
}

/*
================
GDRHeap::Msize

  returns size of allocated memory block
  p	= pointer to memory block
  Notes:	size may not be the same as the size in the original
			allocation request (due to block alignment reasons).
================
*/
uint32_t GDRHeap::Msize( void *p )
{

	if ( !p ) {
		return 0;
	}

#if USE_LIBC_MALLOC
	#ifdef _WIN32
		return _msize( p );
	#else
		return 0;
	#endif
#else
	switch( ((byte *)(p))[-1] ) {
		case SMALL_ALLOC: {
			return SMALL_ALIGN( ((byte *)(p))[-SMALL_HEADER_SIZE] * ALIGN );
		}
		case MEDIUM_ALLOC: {
			return ((mediumHeapEntry_t *)(((byte *)(p)) - ALIGN_SIZE( MEDIUM_HEADER_SIZE )))->size - ALIGN_SIZE( MEDIUM_HEADER_SIZE );
		}
		case LARGE_ALLOC: {
			return ((GDRHeap::page_t*)(*((uintptr_t *)(((byte *)p) - ALIGN_SIZE( LARGE_HEADER_SIZE )))))->dataSize - ALIGN_SIZE( LARGE_HEADER_SIZE );
		}
		default: {
			N_Error( "GDRHeap::Msize: invalid memory block" );
			return 0;
		}
	}
#endif
}

/*
================
GDRHeap::Dump

  dump contents of the heap
================
*/
void GDRHeap::Dump( void )
{
	GDRHeap::page_t	*pg;

	for ( pg = smallFirstUsedPage; pg; pg = pg->next ) {
		Con_Printf("%p  bytes %-8d  (in use by small heap)", pg->data, pg->dataSize);
	}

	if ( smallCurPage ) {
		pg = smallCurPage;
		Con_Printf("%p  bytes %-8d  (small heap active page)", pg->data, pg->dataSize );
	}

	for ( pg = mediumFirstUsedPage; pg; pg = pg->next ) {
		Con_Printf("%p  bytes %-8d  (completely used by medium heap)", pg->data, pg->dataSize );
	}

	for ( pg = mediumFirstFreePage; pg; pg = pg->next ) {
		Con_Printf("%p  bytes %-8d  (partially used by medium heap)", pg->data, pg->dataSize );
	}

	for ( pg = largeFirstUsedPage; pg; pg = pg->next ) {
		Con_Printf("%p  bytes %-8d  (fully used by large heap)", pg->data, pg->dataSize );
	}

	Con_Printf("pages allocated : %d", pagesAllocated );
}

/*
================
GDRHeap::FreePageReal

  frees page to be used by the OS
  p	= page to free
================
*/
void GDRHeap::FreePageReal( GDRHeap::page_t *p )
{
	if (!p)
		N_Error("GDRHeap::FreePageReal: NULL page");
	
	free( p );
}

/*
================
GDRHeap::ReleaseSwappedPages

  releases the swap page to OS
================
*/
void GDRHeap::ReleaseSwappedPages ()
{
	if ( swapPage ) {
		FreePageReal( swapPage );
	}
	swapPage = NULL;
}

/*
================
GDRHeap::AllocatePage

  allocates memory from the OS
  bytes	= page size in bytes
  returns pouint32_ter to page
================
*/
GDRHeap::page_t* GDRHeap::AllocatePage( uint32_t bytes )
{
	GDRHeap::page_t*	p;

	pageRequests++;

	if ( swapPage && swapPage->dataSize == bytes ) {			// if we've got a swap page somewhere
		p			= swapPage;
		swapPage	= NULL;
	}
	else {
		uint32_t size;

		size = bytes + sizeof(GDRHeap::page_t);

		p = (GDRHeap::page_t *) malloc( size + ALIGN - 1 );
		if ( !p ) {
			if ( defragBlock ) {
				Con_Printf("Freeing defragBlock on alloc of %i.\n", size + ALIGN - 1 );
				free( defragBlock );
				defragBlock = NULL;
				p = (GDRHeap::page_t *) malloc( size + ALIGN - 1 );
				AllocDefragBlock();
			}
			if ( !p ) {
				N_Error( "malloc failure for %i", bytes );
			}
		}

		p->data		= (void *) ALIGN_SIZE( (uintptr_t)((byte *)(p)) + sizeof( GDRHeap::page_t ) );
		p->dataSize	= size - sizeof(GDRHeap::page_t);
		p->firstFree = NULL;
		p->largestFree = 0;
		OSAllocs++;
	}

	p->prev = NULL;
	p->next = NULL;

	pagesAllocated++;

	return p;
}

/*
================
GDRHeap::FreePage

  frees a page back to the operating system
  p	= pouint32_ter to page
================
*/
void GDRHeap::FreePage( GDRHeap::page_t *p )
{
	if (!p)
		N_Error("GDRHeap::FreePage: NULL page");

	if ( p->dataSize == pageSize && !swapPage ) {			// add to swap list?
		swapPage = p;
	}
	else {
		FreePageReal( p );
	}

	pagesAllocated--;
}

//===============================================================
//
//	small heap code
//
//===============================================================

/*
================
GDRHeap::SmallAllocate

  allocate memory (1-255 bytes) from the small heap manager
  bytes = number of bytes to allocate
  returns pointer to allocated memory
================
*/
void *GDRHeap::SmallAllocate( uint32_t bytes )
{
	// we need the at least sizeof( uint32_t ) bytes for the free list
	if ( bytes < sizeof( uintptr_t ) ) {
		bytes = sizeof( uintptr_t );
	}

	// increase the number of bytes if necessary to make sure the next small allocation is aligned
	bytes = SMALL_ALIGN( bytes );

	byte *smallBlock = (byte *)(smallFirstFree[bytes / ALIGN]);
	if ( smallBlock ) {
		uintptr_t *link = (uintptr_t *)(smallBlock + SMALL_HEADER_SIZE);
		smallBlock[1] = SMALL_ALLOC;					// allocation identifier
		smallFirstFree[bytes / ALIGN] = (void *)(*link);
		return (void *)(link);
	}

	uint32_t bytesLeft = (size_t)(pageSize) - smallCurPageOffset;
	// if we need to allocate a new page
	if ( bytes >= bytesLeft ) {

		smallCurPage->next	= smallFirstUsedPage;
		smallFirstUsedPage	= smallCurPage;
		smallCurPage		= AllocatePage( pageSize );
		if ( !smallCurPage ) {
			return NULL;
		}
		// make sure the first allocation is aligned
		smallCurPageOffset	= SMALL_ALIGN( 0 );
	}

	smallBlock			= ((byte *)smallCurPage->data) + smallCurPageOffset;
	smallBlock[0]		= (byte)(bytes / ALIGN);		// write # of bytes/ALIGN
	smallBlock[1]		= SMALL_ALLOC;					// allocation identifier
	smallCurPageOffset  += bytes + SMALL_HEADER_SIZE;	// increase the offset on the current page
	return ( smallBlock + SMALL_HEADER_SIZE );			// skip the first two bytes
}

/*
================
GDRHeap::SmallFree

  frees a block of memory allocated by SmallAllocate() call
  data = pouint32_ter to block of memory
================
*/
void GDRHeap::SmallFree( void *ptr )
{
	((byte *)(ptr))[-1] = INVALID_ALLOC;

	byte *d = ( (byte *)ptr ) - SMALL_HEADER_SIZE;
	uintptr_t *link = (uintptr_t *)ptr;
	// index into the table with free small memory blocks
	uint32_t ix = *d;

	// check if the index is correct
	if ( ix > (256 / ALIGN) ) {
		N_Error( "SmallFree: invalid memory block" );
	}

	*link = (uintptr_t)smallFirstFree[ix];	// write next index
	smallFirstFree[ix] = (void *)d;		// link
}

//===============================================================
//
//	medium heap code
//
//	Medium-heap allocated pages not returned to OS until heap destructor
//	called (re-used instead on subsequent medium-size malloc requests).
//
//===============================================================

/*
================
GDRHeap::MediumAllocateFromPage

  performs allocation using the medium heap manager from a given page
  p				= page
  sizeNeeded	= # of bytes needed
  returns pointer to allocated memory
================
*/
void *GDRHeap::MediumAllocateFromPage( GDRHeap::page_t *p, uint32_t sizeNeeded )
{
	mediumHeapEntry_t	*best,*nw = NULL;
	byte				*ret;

	best = (mediumHeapEntry_t *)(p->firstFree);			// first block is largest

	assert( best );
	assert( best->size == p->largestFree );
	assert( best->size >= sizeNeeded );

	// if we can allocate another block from this page after allocating sizeNeeded bytes
	if ( best->size >= (uint32_t)( sizeNeeded + MEDIUM_SMALLEST_SIZE ) ) {
		nw = (mediumHeapEntry_t *)((byte *)best + best->size - sizeNeeded);
		nw->page		= p;
		nw->prev		= best;
		nw->next		= best->next;
		nw->prevFree	= NULL;
		nw->nextFree	= NULL;
		nw->size		= sizeNeeded;
		nw->freeBlock	= 0;			// used block
		if ( best->next ) {
			best->next->prev = nw;
		}
		best->next	= nw;
		best->size	-= sizeNeeded;

		p->largestFree = best->size;
	}
	else {
		if ( best->prevFree ) {
			best->prevFree->nextFree = best->nextFree;
		}
		else {
			p->firstFree = (void *)best->nextFree;
		}
		if ( best->nextFree ) {
			best->nextFree->prevFree = best->prevFree;
		}

		best->prevFree  = NULL;
		best->nextFree  = NULL;
		best->freeBlock = 0;			// used block
		nw = best;

		p->largestFree = 0;
	}

	ret		= (byte *)(nw) + ALIGN_SIZE( MEDIUM_HEADER_SIZE );
	ret[-1] = MEDIUM_ALLOC;		// allocation identifier

	return (void *)(ret);
}

/*
================
GDRHeap::MediumAllocate

  allocate memory (256-32768 bytes) from medium heap manager
  bytes	= number of bytes to allocate
  returns pointer to allocated memory
================
*/
void *GDRHeap::MediumAllocate( uint32_t bytes )
{
	GDRHeap::page_t		*p;
	void				*data;

	uint32_t sizeNeeded = ALIGN_SIZE( bytes ) + ALIGN_SIZE( MEDIUM_HEADER_SIZE );

	// find first page with enough space
	for ( p = mediumFirstFreePage; p; p = p->next ) {
		if ( p->largestFree >= sizeNeeded ) {
			break;
		}
	}

	if ( !p ) {								// need to allocate new page?
		p = AllocatePage( pageSize );
		if ( !p ) {
			return NULL;					// malloc failure!
		}
		p->prev		= NULL;
		p->next		= mediumFirstFreePage;
		if (p->next) {
			p->next->prev = p;
		}
		else {
			mediumLastFreePage	= p;
		}

		mediumFirstFreePage		= p;

		p->largestFree	= pageSize;
		p->firstFree	= (void *)p->data;

		mediumHeapEntry_t *e;
		e				= (mediumHeapEntry_t *)(p->firstFree);
		e->page			= p;
		// make sure ((byte *)e + e->size) is aligned
		e->size			= pageSize & ~(ALIGN - 1);
		e->prev			= NULL;
		e->next			= NULL;
		e->prevFree		= NULL;
		e->nextFree		= NULL;
		e->freeBlock	= 1;
	}

	data = MediumAllocateFromPage( p, sizeNeeded );		// allocate data from page

	// if the page can no longer serve memory, move it away from free list
	// (so that it won't slow down the later alloc queries)
	// this modification speeds up the pageWalk from O(N) to O(sqrt(N))
	// a call to free may swap this page back to the free list

	if ( p->largestFree < MEDIUM_SMALLEST_SIZE ) {
		if ( p == mediumLastFreePage ) {
			mediumLastFreePage = p->prev;
		}

		if ( p == mediumFirstFreePage ) {
			mediumFirstFreePage = p->next;
		}

		if ( p->prev ) {
			p->prev->next = p->next;
		}
		if ( p->next ) {
			p->next->prev = p->prev;
		}

		// link to "completely used" list
		p->prev = NULL;
		p->next = mediumFirstUsedPage;
		if ( p->next ) {
			p->next->prev = p;
		}
		mediumFirstUsedPage = p;
		return data;
	}

	// re-order linked list (so that next malloc query starts from current
	// matching block) -- this speeds up both the page walks and block walks

	if ( p != mediumFirstFreePage ) {
		assert( mediumLastFreePage );
		assert( mediumFirstFreePage );
		assert( p->prev);

		mediumLastFreePage->next	= mediumFirstFreePage;
		mediumFirstFreePage->prev	= mediumLastFreePage;
		mediumLastFreePage			= p->prev;
		p->prev->next				= NULL;
		p->prev						= NULL;
		mediumFirstFreePage			= p;
	}

	return data;
}

/*
================
GDRHeap::MediumFree

  frees a block allocated by the medium heap manager
  ptr	= pointer to data block
================
*/
void GDRHeap::MediumFree( void *ptr )
{
	((byte *)(ptr))[-1] = INVALID_ALLOC;

	mediumHeapEntry_t	*e = (mediumHeapEntry_t *)((byte *)ptr - ALIGN_SIZE( MEDIUM_HEADER_SIZE ));
	GDRHeap::page_t		*p = e->page;
	bool				isInFreeList;

	isInFreeList = p->largestFree >= MEDIUM_SMALLEST_SIZE;

	assert( e->size );
	assert( e->freeBlock == 0 );

	mediumHeapEntry_t *prev = e->prev;

	// if the previous block is free we can merge
	if ( prev && prev->freeBlock ) {
		prev->size += e->size;
		prev->next = e->next;
		if ( e->next ) {
			e->next->prev = prev;
		}
		e = prev;
	}
	else {
		e->prevFree		= NULL;				// link to beginning of free list
		e->nextFree		= (mediumHeapEntry_t *)p->firstFree;
		if ( e->nextFree ) {
			assert( !(e->nextFree->prevFree) );
			e->nextFree->prevFree = e;
		}

		p->firstFree	= e;
		p->largestFree	= e->size;
		e->freeBlock	= 1;				// mark block as free
	}

	mediumHeapEntry_t *next = e->next;

	// if the next block is free we can merge
	if ( next && next->freeBlock ) {
		e->size += next->size;
		e->next = next->next;

		if ( next->next ) {
			next->next->prev = e;
		}

		if ( next->prevFree ) {
			next->prevFree->nextFree = next->nextFree;
		}
		else {
			assert( next == p->firstFree );
			p->firstFree = next->nextFree;
		}

		if ( next->nextFree ) {
			next->nextFree->prevFree = next->prevFree;
		}
	}

	if ( p->firstFree ) {
		p->largestFree = ((mediumHeapEntry_t *)(p->firstFree))->size;
	}
	else {
		p->largestFree = 0;
	}

	// did e become the largest block of the page ?

	if ( e->size > p->largestFree ) {
		assert( e != p->firstFree );
		p->largestFree = e->size;

		if ( e->prevFree ) {
			e->prevFree->nextFree = e->nextFree;
		}
		if ( e->nextFree ) {
			e->nextFree->prevFree = e->prevFree;
		}

		e->nextFree = (mediumHeapEntry_t *)p->firstFree;
		e->prevFree = NULL;
		if ( e->nextFree ) {
			e->nextFree->prevFree = e;
		}
		p->firstFree = e;
	}

	// if page wasn't in free list (because it was near-full), move it back there
	if ( !isInFreeList ) {

		// remove from "completely used" list
		if ( p->prev ) {
			p->prev->next = p->next;
		}
		if ( p->next ) {
			p->next->prev = p->prev;
		}
		if ( p == mediumFirstUsedPage ) {
			mediumFirstUsedPage = p->next;
		}

		p->next = NULL;
		p->prev = mediumLastFreePage;

		if ( mediumLastFreePage ) {
			mediumLastFreePage->next = p;
		}
		mediumLastFreePage = p;
		if ( !mediumFirstFreePage ) {
			mediumFirstFreePage = p;
		}
	}
}

//===============================================================
//
//	large heap code
//
//===============================================================

/*
================
GDRHeap::LargeAllocate

  allocates a block of memory from the operating system
  bytes	= number of bytes to allocate
  returns pouint32_ter to allocated memory
================
*/
void *GDRHeap::LargeAllocate( uint32_t bytes )
{
	GDRHeap::page_t *p = AllocatePage( bytes + ALIGN_SIZE( LARGE_HEADER_SIZE ) );

	assert( p );

	if ( !p ) {
		return NULL;
	}

	byte *	d	= (byte*)(p->data) + ALIGN_SIZE( LARGE_HEADER_SIZE );
	uintptr_t * dw	= (uintptr_t*)(d - ALIGN_SIZE( LARGE_HEADER_SIZE ));
	dw[0]		= (uintptr_t)p;			// write pouint32_ter back to page table
	d[-1]		= LARGE_ALLOC;			// allocation identifier

	// link to 'large used page list'
	p->prev = NULL;
	p->next = largeFirstUsedPage;
	if ( p->next ) {
		p->next->prev = p;
	}
	largeFirstUsedPage = p;

	return (void *)(d);
}

/*
================
GDRHeap::LargeFree

  frees a block of memory allocated by the 'large memory allocator'
  p	= pouint32_ter to allocated memory
================
*/
void GDRHeap::LargeFree( void *ptr)
{
	GDRHeap::page_t*	pg;

	((byte *)(ptr))[-1] = INVALID_ALLOC;

	// get page pouint32_ter
	pg = (GDRHeap::page_t *)(*((uintptr_t *)(((byte *)ptr) - ALIGN_SIZE( LARGE_HEADER_SIZE ))));

	// unlink from doubly linked list
	if ( pg->prev ) {
		pg->prev->next = pg->next;
	}
	if ( pg->next ) {
		pg->next->prev = pg->prev;
	}
	if ( pg == largeFirstUsedPage ) {
		largeFirstUsedPage = pg->next;
	}
	pg->next = pg->prev = NULL;

	FreePage(pg);
}

/*
GDRHeap::DefragIsActive
*/
bool GDRHeap::DefragIsActive(void)
{
	return defragBlock ? true : false;
}

//===============================================================
//
//	memory allocation all in one place
//
//===============================================================

#undef new

static GDRHeap *			mem_heap = NULL;
static memoryStats_t	mem_total_allocs = { 0, 0x0fffffff, -1, 0 };
static memoryStats_t	mem_frame_allocs;
static memoryStats_t	mem_frame_frees;

/*
==================
Mem_ClearFrameStats
==================
*/
void Mem_ClearFrameStats( void )
{
	mem_frame_allocs.num = mem_frame_frees.num = 0;
	mem_frame_allocs.minSize = mem_frame_frees.minSize = 0x0fffffff;
	mem_frame_allocs.maxSize = mem_frame_frees.maxSize = -1;
	mem_frame_allocs.totalSize = mem_frame_frees.totalSize = 0;
}

/*
==================
Mem_GetFrameStats
==================
*/
void Mem_GetFrameStats( memoryStats_t &allocs, memoryStats_t &frees )
{
	allocs = mem_frame_allocs;
	frees = mem_frame_frees;
}

/*
==================
Mem_GetStats
==================
*/
void Mem_GetStats( memoryStats_t &stats )
{
	stats = mem_total_allocs;
}

/*
==================
Mem_UpdateStats
==================
*/
void Mem_UpdateStats( memoryStats_t &stats, uint32_t size )
{
	stats.num++;
	if ( size < stats.minSize ) {
		stats.minSize = size;
	}
	if ( size > stats.maxSize ) {
		stats.maxSize = size;
	}
	stats.totalSize += size;
}

/*
==================
Mem_UpdateAllocStats
==================
*/
void Mem_UpdateAllocStats( uint32_t size )
{
	Mem_UpdateStats( mem_frame_allocs, size );
	Mem_UpdateStats( mem_total_allocs, size );
}

/*
Mem_DefragIsActive:
*/
bool Mem_DefragIsActive(void)
{
	mem_heap->DefragIsActive();
}

/*
==================
Mem_UpdateFreeStats
==================
*/
void Mem_UpdateFreeStats( uint32_t size )
{
	Mem_UpdateStats( mem_frame_frees, size );
	mem_total_allocs.num--;
	mem_total_allocs.totalSize -= size;
}

uint32_t Mem_Msize(void *ptr)
{
	return mem_heap->Msize(ptr);
}

#ifndef ID_DEBUG_MEMORY

/*
==================
Mem_Alloc
==================
*/
void *Mem_Alloc( const uint32_t size )
{
	if ( !size ) {
		return NULL;
	}
	if ( !mem_heap ) {
#ifdef CRASH_ON_STATIC_ALLOCATION
		*((uint32_t*)0x0) = 1;
#endif
		return malloc( size );
	}
	void *mem = mem_heap->Allocate( size );
	Mem_UpdateAllocStats( mem_heap->Msize( mem ) );
	return mem;
}

/*
==================
Mem_Free
==================
*/
void Mem_Free( void *ptr )
{
	if ( !ptr ) {
		return;
	}
	if ( !mem_heap ) {
#ifdef CRASH_ON_STATIC_ALLOCATION
		*((uint32_t*)0x0) = 1;
#endif
		free( ptr );
		return;
	}
	Mem_UpdateFreeStats( mem_heap->Msize( ptr ) );
	mem_heap->Free( ptr );
}

/*
==================
Mem_Alloc16
==================
*/
void *Mem_Alloc16( const uint32_t size )
{
	if ( !size ) {
		return NULL;
	}
	if ( !mem_heap ) {
#ifdef CRASH_ON_STATIC_ALLOCATION
		*((uint32_t*)0x0) = 1;
#endif
		return malloc( size );
	}
	void *mem = mem_heap->Allocate16( size );
	// make sure the memory is 16 byte aligned
//	assert( ( ((uintptr_t)mem) & 15) == 0 );
	return mem;
}

/*
==================
Mem_Free16
==================
*/
void Mem_Free16( void *ptr )
{
	if ( !ptr ) {
		return;
	}
	if ( !mem_heap ) {
#ifdef CRASH_ON_STATIC_ALLOCATION
		*((uint32_t*)0x0) = 1;
#endif
		free( ptr );
		return;
	}
	// make sure the memory is 16 byte aligned
//	assert( ( ((uintptr_t)ptr) & 15) == 0 );
	mem_heap->Free16( ptr );
}

/*
==================
Mem_ClearedAlloc
==================
*/
void *Mem_ClearedAlloc( const uint32_t size )
{
	void *mem = Mem_Alloc( size );
	memset( mem, 0, size );
	return mem;
}

/*
==================
Mem_ClearedAlloc
==================
*/
void Mem_AllocDefragBlock( void )
{
	mem_heap->AllocDefragBlock();
}

/*
==================
Mem_CopyString
==================
*/
char *Mem_CopyString( const char *in )
{
	char	*out;

	out = (char *)Mem_Alloc( strlen(in) + 1 );
	N_strcpy( out, in );
	return out;
}


/*
==================
Mem_Init
==================
*/
GDR_INITIALIZER(Mem_Init) 
{
	mem_heap = new GDRHeap;
	Mem_ClearFrameStats();
}

/*
==================
Mem_Shutdown
==================
*/
void Mem_Shutdown( void )
{
	GDRHeap *m = mem_heap;
	mem_heap = NULL;
	delete m;
}

/*
==================
Mem_EnableLeakTest
==================
*/
void Mem_EnableLeakTest( const char *name ) {
}


#else /* !ID_DEBUG_MEMORY */

#undef		Mem_Alloc
#undef		Mem_ClearedAlloc
#undef		Com_ClearedReAlloc
#undef		Mem_Free
#undef		Mem_CopyString
#undef		Mem_Alloc16
#undef		Mem_Free16

// size of this struct must be a multiple of 16 bytes
typedef struct debugMemory_s {
	const char *			fileName;
	uint32_t						lineNumber;
	uint32_t						frameNumber;
	uint32_t						size;
	struct debugMemory_s *	prev;
	struct debugMemory_s *	next;
} debugMemory_t;

static debugMemory_t *	mem_debugMemory = NULL;
static char				mem_leakName[256] = "";

/*
==================
Mem_CleanupFileName
==================
*/
const char *Mem_CleanupFileName( const char *fileName )
{
	uint32_t i1, i2;
	idStr newFileName;
	static char newFileNames[4][MAX_STRING_CHARS];
	static uint32_t index;

	newFileName = fileName;
	newFileName.BackSlashesToSlashes();
	i1 = newFileName.Find( "neo", false );
	if ( i1 >= 0 ) {
		i1 = newFileName.Find( "/", false, i1 );
		newFileName = newFileName.Right( newFileName.Length() - ( i1 + 1 ) );
	}
	while( 1 ) {
		i1 = newFileName.Find( "/../" );
		if ( i1 <= 0 ) {
			break;
		}
		i2 = i1 - 1;
		while( i2 > 1 && newFileName[i2-1] != '/' ) {
			i2--;
		}
		newFileName = newFileName.Left( i2 - 1 ) + newFileName.Right( newFileName.Length() - ( i1 + 4 ) );
	}
	index = ( index + 1 ) & 3;
	N_strncpyz( newFileNames[index], newFileName.c_str(), sizeof( newFileNames[index] ) );
	return newFileNames[index];
}

/*
==================
Mem_Dump
==================
*/
void Mem_Dump( const char *fileName )
{
	uint32_t i, numBlocks, totalSize;
	char dump[32], *ptr;
	debugMemory_t *b;
	GDRStr module, funcName;
	FILE *f;

	f = fopen( fileName, "wb" );
	if ( !f ) {
		return;
	}

	totalSize = 0;
	for ( numBlocks = 0, b = mem_debugMemory; b; b = b->next, numBlocks++ ) {
		ptr = ((char *) b) + sizeof(debugMemory_t);
		totalSize += b->size;
		for ( i = 0; i < (sizeof(dump)-1) && i < b->size; i++) {
			if ( ptr[i] >= 32 && ptr[i] < 127 ) {
				dump[i] = ptr[i];
			} else {
				dump[i] = '_';
			}
		}
		dump[i] = '\0';
		if ( ( b->size >> 10 ) != 0 ) {
			fprintf( f, "size: %6d KB: %s, line: %d [%s]\r\n", ( b->size >> 10 ), Mem_CleanupFileName(b->fileName), b->lineNumber, dump );
		}
		else {
			fprintf( f, "size: %7d B: %s, line: %d [%s], call stack: %s\r\n", b->size, Mem_CleanupFileName(b->fileName), b->lineNumber, dump );
		}
	}

	fprintf( f, "%8d total memory blocks allocated\r\n", numBlocks );
	fprintf( f, "%8d KB memory allocated\r\n", ( totalSize >> 10 ) );

	fclose( f );
}

/*
==================
Mem_DumpCompressed
==================
*/
typedef struct allocInfo_s
{
	const char *			fileName;
	uint32_t						lineNumber;
	uint32_t						size;
	uint32_t						numAllocs;
	struct allocInfo_s *	next;
} allocInfo_t;

typedef enum
{
	MEMSORT_SIZE,
	MEMSORT_LOCATION,
	MEMSORT_NUMALLOCS,
} memorySortType_t;

void Mem_DumpCompressed( const char *fileName, memorySortType_t memSort, uint32_t numFrames )
{
	uint32_t numBlocks, totalSize, r, j;
	debugMemory_t *b;
	allocInfo_t *a, *nexta, *allocInfo = NULL, *sortedAllocInfo = NULL, *prevSorted, *nextSorted;
	GDRStr module, funcName;
	FILE *f;

	// build list with memory allocations
	totalSize = 0;
	numBlocks = 0;
	for ( b = mem_debugMemory; b; b = b->next ) {

		if ( numFrames && b->frameNumber < 0 - numFrames ) {
			continue;
		}

		numBlocks++;
		totalSize += b->size;

		// search for an allocation from the same source location
		for ( a = allocInfo; a; a = a->next ) {
			if ( a->lineNumber != b->lineNumber ) {
				continue;
			}
			if ( j < MAX_CALLSTACK_DEPTH ) {
				continue;
			}
			if ( !N_strcmp( a->fileName, b->fileName ) ) {
				continue;
			}
			a->numAllocs++;
			a->size += b->size;
			break;
		}

		// if this is an allocation from a new source location
		if ( !a ) {
			a = (allocInfo_t *) malloc( sizeof( allocInfo_t ) );
			a->fileName = b->fileName;
			a->lineNumber = b->lineNumber;
			a->size = b->size;
			a->numAllocs = 1;
			a->next = allocInfo;
			allocInfo = a;
		}
	}

	// sort list
	for ( a = allocInfo; a; a = nexta ) {
		nexta = a->next;

		prevSorted = NULL;
		switch( memSort ) {
			// sort on size
			case MEMSORT_SIZE: {
				for ( nextSorted = sortedAllocInfo; nextSorted; nextSorted = nextSorted->next ) {
					if ( a->size > nextSorted->size ) {
						break;
					}
					prevSorted = nextSorted;
				}
				break;
			}
			// sort on file name and line number
			case MEMSORT_LOCATION: {
				for ( nextSorted = sortedAllocInfo; nextSorted; nextSorted = nextSorted->next ) {
					r = N_stricmp( Mem_CleanupFileName( a->fileName ), Mem_CleanupFileName( nextSorted->fileName ) );
					if ( r < 0 || ( r == 0 && a->lineNumber < nextSorted->lineNumber ) ) {
						break;
					}
					prevSorted = nextSorted;
				}
				break;
			}
			// sort on the number of allocations
			case MEMSORT_NUMALLOCS: {
				for ( nextSorted = sortedAllocInfo; nextSorted; nextSorted = nextSorted->next ) {
					if ( a->numAllocs > nextSorted->numAllocs ) {
						break;
					}
					prevSorted = nextSorted;
				}
				break;
			}
		}
		if ( !prevSorted ) {
			a->next = sortedAllocInfo;
			sortedAllocInfo = a;
		}
		else {
			prevSorted->next = a;
			a->next = nextSorted;
		}
	}

	f = fopen( fileName, "wb" );
	if ( !f ) {
		return;
	}

	// write list to file
	for ( a = sortedAllocInfo; a; a = nexta ) {
		nexta = a->next;
		fprintf( f, "size: %6d KB, allocs: %5d: %s, line: %d\r\n",
					(a->size >> 10), a->numAllocs, Mem_CleanupFileName(a->fileName),
							a->lineNumber );
		free( a );
	}

	fprintf( f, "%8d total memory blocks allocated\r\n", numBlocks );
	fprintf( f, "%8d KB memory allocated\r\n", ( totalSize >> 10 ) );

	fclose( f );
}

/*
==================
Mem_AllocDebugMemory
==================
*/
void *Mem_AllocDebugMemory( const uint32_t size, const char *fileName, const uint32_t lineNumber, const bool align16 )
{
	void *p;
	debugMemory_t *m;

	if ( !size ) {
		return NULL;
	}

	if ( !mem_heap ) {
#ifdef CRASH_ON_STATIC_ALLOCATION
		*((uint32_t*)0x0) = 1;
#endif
		// NOTE: set a breakpouint32_t here to find memory allocations before mem_heap is initialized
		return malloc( size );
	}

	if ( align16 ) {
		p = mem_heap->Allocate16( size + sizeof( debugMemory_t ) );
	}
	else {
		p = mem_heap->Allocate( size + sizeof( debugMemory_t ) );
	}

	Mem_UpdateAllocStats( size );

	m = (debugMemory_t *) p;
	m->fileName = fileName;
	m->lineNumber = lineNumber;
	m->frameNumber = idLib::frameNumber;
	m->size = size;
	m->next = mem_debugMemory;
	m->prev = NULL;
	if ( mem_debugMemory ) {
		mem_debugMemory->prev = m;
	}
	mem_debugMemory = m;

	return ( ( (byte *) p ) + sizeof( debugMemory_t ) );
}

/*
==================
Mem_FreeDebugMemory
==================
*/
void Mem_FreeDebugMemory( void *p, const char *fileName, const uint32_t lineNumber, const bool align16 )
{
	debugMemory_t *m;

	if ( !p ) {
		return;
	}

	if ( !mem_heap ) {
#ifdef CRASH_ON_STATIC_ALLOCATION
		*((uint32_t*)0x0) = 1;
#endif
		// NOTE: set a breakpouint32_t here to find memory being freed before mem_heap is initialized
		free( p );
		return;
	}

	m = (debugMemory_t *) ( ( (byte *) p ) - sizeof( debugMemory_t ) );

	if ( m->size < 0 ) {
		N_Error( "memory freed twice" );
	}

	Mem_UpdateFreeStats( m->size );

	if ( m->next ) {
		m->next->prev = m->prev;
	}
	if ( m->prev ) {
		m->prev->next = m->next;
	}
	else {
		mem_debugMemory = m->next;
	}

	m->fileName = fileName;
	m->lineNumber = lineNumber;
	m->frameNumber = 0;//idLib::frameNumber;
	m->size = -m->size;

	if ( align16 ) {
		mem_heap->Free16( m );
	}
	else {
		mem_heap->Free( m );
	}
}

/*
==================
Mem_Alloc
==================
*/
void *Mem_AllocDebug( const uint32_t size, const char *fileName, const uint32_t lineNumber )
{
	if ( !size ) {
		return NULL;
	}
	return Mem_AllocDebugMemory( size, fileName, lineNumber, false );
}

/*
==================
Mem_Free
==================
*/
void Mem_FreeDebug( void *ptr, const char *fileName, const uint32_t lineNumber )
{
	if ( !ptr ) {
		return;
	}
	Mem_FreeDebugMemory( ptr, fileName, lineNumber, false );
}

/*
==================
Mem_Alloc16
==================
*/
void *Mem_Alloc16Debug( const uint32_t size, const char *fileName, const uint32_t lineNumber )
{
	if ( !size ) {
		return NULL;
	}
	void *mem = Mem_AllocDebugMemory( size, fileName, lineNumber, true );
	// make sure the memory is 16 byte aligned
	assert( ( ((uint32_t)mem) & 15) == 0 );
	return mem;
}

/*
==================
Mem_Free16
==================
*/
void Mem_Free16Debug( void *ptr, const char *fileName, const uint32_t lineNumber )
{
	if ( !ptr ) {
		return;
	}
	// make sure the memory is 16 byte aligned
//	assert( ( ((uint32_t)ptr) & 15) == 0 );
	Mem_FreeDebugMemory( ptr, fileName, lineNumber, true );
}

/*
==================
Mem_ClearedAlloc
==================
*/
void *Mem_ClearedAllocDebug( const uint32_t size, const char *fileName, const uint32_t lineNumber )
{
	void *mem = Mem_Alloc( size, fileName, lineNumber );
//	memset( mem, 0, size );
	return mem;
}

/*
==================
Mem_CopyString
==================
*/
char *Mem_CopyStringDebug( const char *in, const char *fileName, const uint32_t lineNumber )
{
	char	*out;

	out = (char *)Mem_Alloc( strlen(in) + 1, fileName, lineNumber );
	N_strcpy( out, in );
	return out;
}

/*
==================
Mem_Init
==================
*/
GDR_INITIALIZER(Mem_Init)
{
	mem_heap = new GDRHeap;
}

/*
==================
Mem_Shutdown
==================
*/
void Mem_Shutdown( void )
{
	if ( mem_leakName[0] != '\0' ) {
		Mem_DumpCompressed( va( "%s_leak_size.txt", mem_leakName ), MEMSORT_SIZE, 0 );
		Mem_DumpCompressed( va( "%s_leak_location.txt", mem_leakName ), MEMSORT_LOCATION, 0 );
	}

	GDRHeap *m = mem_heap;
	mem_heap = NULL;
	delete m;
}

/*
==================
Mem_EnableLeakTest
==================
*/
void Mem_EnableLeakTest( const char *name )
{
	N_strncpyz( mem_leakName, name, sizeof( mem_leakName ) );
}

#endif