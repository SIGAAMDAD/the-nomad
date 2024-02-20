#include "module_public.h"

#define SMALL_HEADER_SIZE		( (intptr_t) ( sizeof( byte ) + sizeof( byte ) ) )
#define MEDIUM_HEADER_SIZE		( (intptr_t) ( sizeof( mediumHeapEntry_s ) + sizeof( byte ) ) )
#define LARGE_HEADER_SIZE		( (intptr_t) ( sizeof( dword * ) + sizeof( byte ) ) )

#define ALIGN_SIZE( bytes )		( ( (bytes) + ALIGN - 1 ) & ~(ALIGN - 1) )
#define SMALL_ALIGN( bytes )	( ALIGN_SIZE( (bytes) + SMALL_HEADER_SIZE ) - SMALL_HEADER_SIZE )
#define MEDIUM_SMALLEST_SIZE	( ALIGN_SIZE( 256 ) + ALIGN_SIZE( MEDIUM_HEADER_SIZE ) )

/*
* moduleHeap_t: modders will most likely not really know or care about memory fragmentation or usage,
* so there is a dedicated heap manager for that bit of the game
*/
typedef struct {
	enum {
		ALIGN = 8									// memory alignment in bytes
	};

	enum {
		INVALID_ALLOC	= 0xdd,
		SMALL_ALLOC		= 0xaa,						// small allocation
		MEDIUM_ALLOC	= 0xbb,						// medium allocaction
		LARGE_ALLOC		= 0xcc						// large allocaction
	};

	typedef struct page_s {							// allocation page
		void						*data;			// data pointer to allocated memory
		uint64_t					dataSize;		// number of bytes of memory 'data' points to
		struct page_s				*next;			// next free page in same page manager
		struct page_s				*prev;			// used only when allocated
		uint64_t					largestFree;	// this data used by the medium-size heap manager
		void						*firstFree;		// pointer to first free entry
	} page_t;

	typedef struct mediumHeapEntry_s {
		struct page_s				*page;			// pointer to page
		uint64_t					size;			// size of block
		struct mediumHeapEntry_s	*prev;			// previous block
		struct mediumHeapEntry_s	*next;			// next block
		struct mediumHeapEntry_s	*prevFree;		// previous free block
		struct mediumHeapEntry_s	*nextFree;		// next free block
		uint64_t					freeBlock;		// non-zero if free block
	} mediumHeapEntry_t;

	// variables
	void			*smallFirstFree[256/ALIGN+1];	// small heap allocator lists (for allocs of 1-255 bytes)
	page_t			*smallCurPage;					// current page for small allocations
	uint64_t		smallCurPageOffset;				// byte offset in current page
	page_t			*smallFirstUsedPage;			// first used page of the small heap manager

	page_t			*mediumFirstFreePage;			// first partially free page
	page_t			*mediumLastFreePage;			// last partially free page
	page_t			*mediumFirstUsedPage;			// completely used page

	page_t			*largeFirstUsedPage;			// first page used by the large heap manager

	page_t			*swapPage;

	uint64_t		pagesAllocated;					// number of pages currently allocated
	uint64_t		pageSize;						// size of one alloc page in bytes

	uint64_t		pageRequests;					// page requests
	uint64_t		OSAllocs;						// number of allocs made to the OS

	uint32_t		c_heapAllocRunningCount;
} moduleHeap_t;

void Mem_Init( void )
{

}

void Mem_Shutdown( void )
{

}

void *Mem_Alloc( uint64_t nBytes )
{

}

void Mem_Free( void *pBuffer )
{

}
