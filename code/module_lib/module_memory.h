#ifndef __MODULE_MEMORY__
#define __MODULE_MEMORY__

#pragma once

/*
===============================================================================

	Memory Management

	This is a replacement for the compiler heap code (i.e. "C" malloc() and
	free() calls). On average 2.5-3.0 times faster than MSVC malloc()/free().
	Worst case performance is 1.65 times faster and best case > 70 times.

===============================================================================
*/

typedef struct {
	int32_t	num;
	int32_t	minSize;
	int32_t	maxSize;
	int32_t	totalSize;
} memoryStats_t;

//#define MODULE_REDIRECT_NEWDELETE

void		Mem_Init( void );
void		Mem_Shutdown( void );
void		Mem_EnableLeakTest( const char *name );
void		Mem_ClearFrameStats( void );
void		Mem_GetFrameStats( memoryStats_t &allocs, memoryStats_t &frees );
void		Mem_GetStats( memoryStats_t &stats );
void		Mem_Dump_f( const class idCmdArgs &args );
void		Mem_DumpCompressed_f( const class idCmdArgs &args );
void		Mem_AllocDefragBlock( void );


#ifndef MODULE_DEBUG_MEMORY

void *		Mem_Alloc( const uint32_t size );
void *		Mem_ClearedAlloc( const uint32_t size );
void		Mem_Free( void *ptr );
char *		Mem_CopyString( const char *in );
void *		Mem_Alloc16( const uint32_t size );
void		Mem_Free16( void *ptr );

#ifdef MODULE_REDIRECT_NEWDELETE

#undef new
#undef delete

GDR_INLINE void *operator new( size_t s ) {
	return Mem_Alloc( s );
}
GDR_INLINE void operator delete( void *p ) {
	Mem_Free( p );
}
GDR_INLINE void *operator new[]( size_t s ) {
	return Mem_Alloc( s );
}
GDR_INLINE void operator delete[]( void *p ) {
	Mem_Free( p );
}

#endif

#else /* MODULE_DEBUG_MEMORY */

void *		Mem_Alloc( const uint32_t size, const char *fileName, const uint32_t32_t lineNumber );
void *		Mem_ClearedAlloc( const uint32_t size, const char *fileName, const uint32_t32_t lineNumber );
void		Mem_Free( void *ptr, const char *fileName, const uint32_t32_t lineNumber );
char *		Mem_CopyString( const char *in, const char *fileName, const uint32_t32_t lineNumber );
void *		Mem_Alloc16( const uint32_t size, const char *fileName, const uint32_t32_t lineNumber );
void		Mem_Free16( void *ptr, const char *fileName, const uint32_t32_t lineNumber );

#ifdef MODULE_REDIRECT_NEWDELETE

GDR_INLINE void *operator new( size_t s, int t1, int t2, char *fileName, int32_t lineNumber ) {
	return Mem_Alloc( s, fileName, lineNumber );
}
GDR_INLINE void operator delete( void *p, int t1, int t2, char *fileName, int32_t lineNumber ) {
	Mem_Free( p, fileName, lineNumber );
}
GDR_INLINE void *operator new[]( size_t s, int t1, int t2, char *fileName, int32_t lineNumber ) {
	return Mem_Alloc( s, fileName, lineNumber );
}
GDR_INLINE void operator delete[]( void *p, int t1, int t2, char *fileName, int32_t lineNumber ) {
	Mem_Free( p, fileName, lineNumber );
}
GDR_INLINE void *operator new( size_t s ) {
	return Mem_Alloc( s, "", 0 );
}
GDR_INLINE void operator delete( void *p ) {
	Mem_Free( p, "", 0 );
}
GDR_INLINE void *operator new[]( size_t s ) {
	return Mem_Alloc( s, "", 0 );
}
GDR_INLINE void operator delete[]( void *p ) {
	Mem_Free( p, "", 0 );
}

#define MODULE_DEBUG_NEW						new( 0, 0, __FILE__, __LINE__ )
#undef new
#define new									ID_DEBUG_NEW

#endif

#define		Mem_Alloc( size )				Mem_Alloc( size, __FILE__, __LINE__ )
#define		Mem_ClearedAlloc( size )		Mem_ClearedAlloc( size, __FILE__, __LINE__ )
#define		Mem_Free( ptr )					Mem_Free( ptr, __FILE__, __LINE__ )
#define		Mem_CopyString( s )				Mem_CopyString( s, __FILE__, __LINE__ )
#define		Mem_Alloc16( size )				Mem_Alloc16( size, __FILE__, __LINE__ )
#define		Mem_Free16( ptr )				Mem_Free16( ptr, __FILE__, __LINE__ )

#endif /* MODULE_DEBUG_MEMORY */

GDR_INLINE void *CModuleAllocator::allocate( size_t n, int flags )
{
	return Mem_Alloc( n );
}

GDR_INLINE void *CModuleAllocator::allocate( size_t n, size_t alignment, size_t offset, int flags )
{
	return Mem_Alloc( n );
}

GDR_INLINE void CModuleAllocator::deallocate( void *ptr, size_t )
{
	Mem_Free( ptr );
}



/*
===============================================================================

	Block based allocator for fixed size objects.

	All objects of the 'type' are properly constructed.
	However, the constructor is not called for re-used objects.

===============================================================================
*/

template<class type, uint32_t blockSize>
class idBlockAlloc {
public:
							idBlockAlloc( void );
							~idBlockAlloc( void );

	void					Shutdown( void );

	type *					Alloc( void );
	void					Free( type *element );

	uint32_t				GetTotalCount( void ) const { return total; }
	uint32_t				GetAllocCount( void ) const { return active; }
	uint32_t				GetFreeCount( void ) const { return total - active; }

private:
	typedef struct element_s {
		type				t;
		struct element_s *	next;
	} element_t;
	typedef struct block_s {
		element_t			elements[blockSize];
		struct block_s *	next;
	} block_t;

	block_t *				blocks;
	element_t *				free;
	uint32_t				total;
	uint32_t				active;
};

template<class type, uint32_t blockSize>
idBlockAlloc<type,blockSize>::idBlockAlloc( void ) {
	blocks = NULL;
	free = NULL;
	total = active = 0;
}

template<class type, uint32_t blockSize>
idBlockAlloc<type,blockSize>::~idBlockAlloc( void ) {
	Shutdown();
}

template<class type, uint32_t blockSize>
type *idBlockAlloc<type,blockSize>::Alloc( void ) {
	if ( !free ) {
		block_t *block = new block_t;
		block->next = blocks;
		blocks = block;
		for ( uint32_t i = 0; i < blockSize; i++ ) {
			block->elements[i].next = free;
			free = &block->elements[i];
		}
		total += blockSize;
	}
	active++;
	element_t *element = free;
	free = free->next;
	element->next = NULL;
	return &element->t;
}

template<class type, uint32_t blockSize>
void idBlockAlloc<type,blockSize>::Free( type *t ) {
	element_t *element = (element_t *)t;
	element->next = free;
	free = element;
	active--;
}

template<class type, uint32_t blockSize>
void idBlockAlloc<type,blockSize>::Shutdown( void ) {
	while( blocks ) {
		block_t *block = blocks;
		blocks = blocks->next;
		delete block;
	}
	blocks = NULL;
	free = NULL;
	total = active = 0;
}

/*
==============================================================================

	Dynamic allocator, simple wrapper for normal allocations which can
	be interchanged with idDynamicBlockAlloc.

	No constructor is called for the 'type'.
	Allocated blocks are always 16 byte aligned.

==============================================================================
*/

template<class type, uint32_t baseBlockSize, uint32_t minBlockSize>
class idDynamicAlloc {
public:
									idDynamicAlloc( void );
									~idDynamicAlloc( void );

	void							Init( void );
	void							Shutdown( void );
	void							SetFixedBlocks( uint32_t numBlocks ) {}
	void							SetLockMemory( bool lock ) {}
	void							FreeEmptyBaseBlocks( void ) {}

	type *							Alloc( const uint32_t num );
	type *							Resize( type *ptr, const uint32_t num );
	void							Free( type *ptr );
	const char *					CheckMemory( const type *ptr ) const;

	uint32_t						GetNumBaseBlocks( void ) const { return 0; }
	uint32_t						GetBaseBlockMemory( void ) const { return 0; }
	uint32_t						GetNumUsedBlocks( void ) const { return numUsedBlocks; }
	uint32_t						GetUsedBlockMemory( void ) const { return usedBlockMemory; }
	uint32_t						GetNumFreeBlocks( void ) const { return 0; }
	uint32_t						GetFreeBlockMemory( void ) const { return 0; }
	uint32_t						GetNumEmptyBaseBlocks( void ) const { return 0; }

private:
	uint32_t						numUsedBlocks;			// number of used blocks
	uint32_t						usedBlockMemory;		// total memory in used blocks

	uint32_t						numAllocs;
	uint32_t						numResizes;
	uint32_t						numFrees;

	void							Clear( void );
};

template<class type, uint32_t baseBlockSize, uint32_t minBlockSize>
idDynamicAlloc<type, baseBlockSize, minBlockSize>::idDynamicAlloc( void ) {
	Clear();
}

template<class type, uint32_t baseBlockSize, uint32_t minBlockSize>
idDynamicAlloc<type, baseBlockSize, minBlockSize>::~idDynamicAlloc( void ) {
	Shutdown();
}

template<class type, uint32_t baseBlockSize, uint32_t minBlockSize>
void idDynamicAlloc<type, baseBlockSize, minBlockSize>::Init( void ) {
}

template<class type, uint32_t baseBlockSize, uint32_t minBlockSize>
void idDynamicAlloc<type, baseBlockSize, minBlockSize>::Shutdown( void ) {
	Clear();
}

template<class type, uint32_t baseBlockSize, uint32_t minBlockSize>
type *idDynamicAlloc<type, baseBlockSize, minBlockSize>::Alloc( const uint32_t num ) {
	numAllocs++;
	if ( num <= 0 ) {
		return NULL;
	}
	numUsedBlocks++;
	usedBlockMemory += num * sizeof( type );
	return Mem_Alloc16( num * sizeof( type ) );
}

template<class type, uint32_t baseBlockSize, uint32_t minBlockSize>
type *idDynamicAlloc<type, baseBlockSize, minBlockSize>::Resize( type *ptr, const uint32_t num ) {

	numResizes++;

	if ( ptr == NULL ) {
		return Alloc( num );
	}

	if ( num <= 0 ) {
		Free( ptr );
		return NULL;
	}

	Assert( 0 );
	return ptr;
}

template<class type, uint32_t baseBlockSize, uint32_t minBlockSize>
void idDynamicAlloc<type, baseBlockSize, minBlockSize>::Free( type *ptr ) {
	numFrees++;
	if ( ptr == NULL ) {
		return;
	}
	Mem_Free16( ptr );
}

template<class type, uint32_t baseBlockSize, uint32_t minBlockSize>
const char *idDynamicAlloc<type, baseBlockSize, minBlockSize>::CheckMemory( const type *ptr ) const {
	return NULL;
}

template<class type, uint32_t baseBlockSize, uint32_t minBlockSize>
void idDynamicAlloc<type, baseBlockSize, minBlockSize>::Clear( void ) {
	numUsedBlocks = 0;
	usedBlockMemory = 0;
	numAllocs = 0;
	numResizes = 0;
	numFrees = 0;
}

#endif