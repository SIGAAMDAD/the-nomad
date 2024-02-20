#ifndef __MODULE_PUBLIC__
#define __MODULE_PUBLIC__

#pragma once

#include "../engine/n_shared.h"

typedef struct
{
    void GDR_ATTRIBUTE((format( printf, 2, 3 ))) ( *GDR_DECL Printf )( int level, const char *fmt, ... );
    void GDR_ATTRIBUTE((format( printf, 2, 3 ))) ( *GDR_DECL Error )( errorCode_t code, const char *fmt, ... ) GDR_NORETURN;

    void (*Cvar_Get)();
    void (*Cvar_Set)( const char *pName, const char *pValue );

    vm_t *(*VM_Create)();
} moduleImport_t;

/*
* CModuleMemoryAllocator: a dedicated heap manager for modules to avoid fragmentation or the vm touching
* engine memory, only for the modulelib's hands, no one else's
*/
class CModuleMemoryAllocator
{
public:
private:
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

    //
    // methods
    //

    // allocate page from the OS
	page_t          *AllocatePage( uint64_t nBytes );

    // free an OS allocated page
	void			FreePage( CModuleMemoryAllocator::page_t *p );

    // allocate memory (1-255 bytes) from small heap manager
	void            *SmallAllocate( uint64_t nBytes );

    // free memory allocated by small heap manager
	void			SmallFree( void *pBuffer );

	void            *MediumAllocateFromPage( CModuleMemoryAllocator::page_t *p, uint64_t sizeNeeded );

    // allocate memory (256-32768 bytes) from medium heap manager
	void            *MediumAllocate( uint64_t nBytes );

    // free memory allocated by medium heap manager
	void			MediumFree( void *pBuffer );

    // allocate large block from OS directly
	void            *LargeAllocate( uint64_t nBytes );

    // free memory allocated by large heap manager
	void			LargeFree( void *pBuffer );

	void			ReleaseSwappedPages( void );
	void			FreePageReal( CModuleMemoryAllocator::page_t *p );
};

GDR_EXPORT class CModuleLib
{
public:
    CModuleLib( void ) = default;
    ~CModuleLib() = default;

    extern "C" void Init( const moduleImport_t *pImport );
    extern "C" void Shutdown( void );

    extern "C" nhandle_t AddModule( const char *pModuleName, uint32_t nFunctionProcs, vm_t *pModule );
    extern "C" nhandle_t GetModuleHandle( const char *pModuleName ) const;
    extern "C" void CallModuleFunc( nhandle_t hModule );

	//
	// vm dynamic memory interface (not shared between vms)
	//
	extern "C" nhandle_t ModuleCreateBuffer( nhandle_t hModule, uint32_t nBytes );
	extern "C" void ModuleReleaseBuffer( nhandle_t hBuffer );
private:
    CModuleMemoryAllocator m_ModuleAllocator;
};

void *Mem_Alloc( uint32_t nBytes );
void Mem_Free( void *pBuffer );

#ifdef GDR_DLLCOMPILE
#undef new
#undef delete
GDR_INLINE void *operator new( size_t sz ) {
	return Mem_Alloc( sz );
}
GDR_INLINE void *operator new[]( size_t sz ) {
	return Mem_Alloc( sz );
}
GDR_INLINE void operator delete( void *ptr ) {
	Mem_Free( ptr );
}
GDR_INLINE void operator delete[]( void *ptr ) {
	Mem_Free( ptr );
}
#endif

GDR_EXPORT extern CModuleLib *g_pModuleLib;

#endif