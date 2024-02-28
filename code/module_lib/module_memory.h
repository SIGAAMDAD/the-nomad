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

void		Mem_Init( void );
void		Mem_Shutdown( void );
void		Mem_EnableLeakTest( const char *name );
void		Mem_ClearFrameStats( void );
void		Mem_GetFrameStats( memoryStats_t &allocs, memoryStats_t &frees );
void		Mem_GetStats( memoryStats_t &stats );
void		Mem_Dump_f( const class idCmdArgs &args );
void		Mem_DumpCompressed_f( const class idCmdArgs &args );
void		Mem_AllocDefragBlock( void );

void *		Mem_Alloc( const uint32_t size );
void *		Mem_ClearedAlloc( const uint32_t size );
void		Mem_Free( void *ptr );
char *		Mem_CopyString( const char *in );
void *		Mem_Alloc16( const uint32_t size );
void		Mem_Free16( void *ptr );

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

/*
==============================================================================

	Fast dynamic block allocator.

	No constructor is called for the 'type'.
	Allocated blocks are always 16 byte aligned.

==============================================================================
*/

#include "module_lib/idlib/containers/BTree.h"

//#define DYNAMIC_BLOCK_ALLOC_CHECK

template<class type>
class idDynamicBlock {
public:
	type *							GetMemory( void ) const { return (type *)( ( (byte *) this ) + sizeof( idDynamicBlock<type> ) ); }
	uint32_t						GetSize( void ) const { return abs( (int)size ); }
	void							SetSize( uint32_t s, bool isBaseBlock ) { size = isBaseBlock ? -s : s; }
	bool							IsBaseBlock( void ) const { return ( size < 0 ); }

#ifdef DYNAMIC_BLOCK_ALLOC_CHECK
	uint32_t						id[3];
	void *							allocator;
#endif

	uint32_t						size;					// size in bytes of the block
	idDynamicBlock<type> *			prev;					// previous memory block
	idDynamicBlock<type> *			next;					// next memory block
	idBTreeNode<idDynamicBlock<type>,uint32_t> *node;			// node in the B-Tree with free blocks
};

template<class type, uint32_t baseBlockSize, uint32_t minBlockSize>
class idDynamicBlockAlloc {
public:
									idDynamicBlockAlloc( void );
									~idDynamicBlockAlloc( void );

	void							Init( void );
	void							Shutdown( void );
	void							SetFixedBlocks( uint32_t numBlocks );
	void							SetLockMemory( bool lock );
	void							FreeEmptyBaseBlocks( void );

	type *							Alloc( const uint32_t num );
	type *							Resize( type *ptr, const uint32_t num );
	void							Free( type *ptr );
	const char *					CheckMemory( const type *ptr ) const;

	inline uint32_t					GetNumBaseBlocks( void ) const { return numBaseBlocks; }
	inline uint32_t					GetBaseBlockMemory( void ) const { return baseBlockMemory; }
	inline uint32_t					GetNumUsedBlocks( void ) const { return numUsedBlocks; }
	inline uint32_t					GetUsedBlockMemory( void ) const { return usedBlockMemory; }
	inline uint32_t					GetNumFreeBlocks( void ) const { return numFreeBlocks; }
	inline uint32_t					GetFreeBlockMemory( void ) const { return freeBlockMemory; }
	inline uint32_t					GetNumEmptyBaseBlocks( void ) const;

private:
	idDynamicBlock<type> *			firstBlock;				// first block in list in order of increasing address
	idDynamicBlock<type> *			lastBlock;				// last block in list in order of increasing address
	idBTree<idDynamicBlock<type>,uint32_t,4>freeTree;			// B-Tree with free memory blocks
	bool							allowAllocs;			// allow base block allocations
	bool							lockMemory;				// lock memory so it cannot get swapped out

#ifdef DYNAMIC_BLOCK_ALLOC_CHECK
	uint32_t						blockId[3];
#endif

	uint32_t						numBaseBlocks;			// number of base blocks
	uint32_t						baseBlockMemory;		// total memory in base blocks
	uint32_t						numUsedBlocks;			// number of used blocks
	uint32_t						usedBlockMemory;		// total memory in used blocks
	uint32_t						numFreeBlocks;			// number of free blocks
	uint32_t						freeBlockMemory;		// total memory in free blocks

	uint32_t						numAllocs;
	uint32_t						numResizes;
	uint32_t						numFrees;

	void							Clear( void );
	idDynamicBlock<type> *			AllocInternal( const uint32_t num );
	idDynamicBlock<type> *			ResizeInternal( idDynamicBlock<type> *block, const uint32_t num );
	void							FreeInternal( idDynamicBlock<type> *block );
	void							LinkFreeInternal( idDynamicBlock<type> *block );
	void							UnlinkFreeInternal( idDynamicBlock<type> *block );
	void							CheckMemory( void ) const;
};

template<class type, uint32_t baseBlockSize, uint32_t minBlockSize>
idDynamicBlockAlloc<type, baseBlockSize, minBlockSize>::idDynamicBlockAlloc( void ) {
	Clear();
}

template<class type, uint32_t baseBlockSize, uint32_t minBlockSize>
idDynamicBlockAlloc<type, baseBlockSize, minBlockSize>::~idDynamicBlockAlloc( void ) {
	Shutdown();
}

template<class type, uint32_t baseBlockSize, uint32_t minBlockSize>
void idDynamicBlockAlloc<type, baseBlockSize, minBlockSize>::Init( void ) {
	freeTree.Init();
}

template<class type, uint32_t baseBlockSize, uint32_t minBlockSize>
void idDynamicBlockAlloc<type, baseBlockSize, minBlockSize>::Shutdown( void ) {
	idDynamicBlock<type> *block;

	for ( block = firstBlock; block != NULL; block = block->next ) {
		if ( block->node == NULL ) {
			FreeInternal( block );
		}
	}

	for ( block = firstBlock; block != NULL; block = firstBlock ) {
		firstBlock = block->next;
		assert( block->IsBaseBlock() );
		if ( lockMemory ) {
//			idLib::sys->UnlockMemory( block, block->GetSize() + sizeof( idDynamicBlock<type> ) );
		}
		Mem_Free16( block );
	}

	freeTree.Shutdown();

	Clear();
}

template<class type, uint32_t baseBlockSize, uint32_t minBlockSize>
void idDynamicBlockAlloc<type, baseBlockSize, minBlockSize>::SetFixedBlocks( uint32_t numBlocks ) {
	idDynamicBlock<type> *block;

	for ( uint32_t i = numBaseBlocks; i < numBlocks; i++ ) {
		block = ( idDynamicBlock<type> * ) Mem_Alloc16( baseBlockSize );
		if ( lockMemory ) {
//			idLib::sys->LockMemory( block, baseBlockSize );
		}
#ifdef DYNAMIC_BLOCK_ALLOC_CHECK
		memcpy( block->id, blockId, sizeof( block->id ) );
		block->allocator = (void*)this;
#endif
		block->SetSize( baseBlockSize - sizeof( idDynamicBlock<type> ), true );
		block->next = NULL;
		block->prev = lastBlock;
		if ( lastBlock ) {
			lastBlock->next = block;
		} else {
			firstBlock = block;
		}
		lastBlock = block;
		block->node = NULL;

		FreeInternal( block );

		numBaseBlocks++;
		baseBlockMemory += baseBlockSize;
	}

	allowAllocs = false;
}

template<class type, uint32_t baseBlockSize, uint32_t minBlockSize>
void idDynamicBlockAlloc<type, baseBlockSize, minBlockSize>::SetLockMemory( bool lock ) {
	lockMemory = lock;
}

template<class type, uint32_t baseBlockSize, uint32_t minBlockSize>
void idDynamicBlockAlloc<type, baseBlockSize, minBlockSize>::FreeEmptyBaseBlocks( void ) {
	idDynamicBlock<type> *block, *next;

	for ( block = firstBlock; block != NULL; block = next ) {
		next = block->next;

		if ( block->IsBaseBlock() && block->node != NULL && ( next == NULL || next->IsBaseBlock() ) ) {
			UnlinkFreeInternal( block );
			if ( block->prev ) {
				block->prev->next = block->next;
			} else {
				firstBlock = block->next;
			}
			if ( block->next ) {
				block->next->prev = block->prev;
			} else {
				lastBlock = block->prev;
			}
			if ( lockMemory ) {
//				idLib::sys->UnlockMemory( block, block->GetSize() + sizeof( idDynamicBlock<type> ) );
			}
			numBaseBlocks--;
			baseBlockMemory -= block->GetSize() + sizeof( idDynamicBlock<type> );
			Mem_Free16( block );
		}
	}

#ifdef DYNAMIC_BLOCK_ALLOC_CHECK
	CheckMemory();
#endif
}

template<class type, uint32_t baseBlockSize, uint32_t minBlockSize>
uint32_t idDynamicBlockAlloc<type, baseBlockSize, minBlockSize>::GetNumEmptyBaseBlocks( void ) const {
	uint32_t numEmptyBaseBlocks;
	idDynamicBlock<type> *block;

	numEmptyBaseBlocks = 0;
	for ( block = firstBlock; block != NULL; block = block->next ) {
		if ( block->IsBaseBlock() && block->node != NULL && ( block->next == NULL || block->next->IsBaseBlock() ) ) {
			numEmptyBaseBlocks++;
		}
	}
	return numEmptyBaseBlocks;
}

template<class type, uint32_t baseBlockSize, uint32_t minBlockSize>
type *idDynamicBlockAlloc<type, baseBlockSize, minBlockSize>::Alloc( const uint32_t num ) {
	idDynamicBlock<type> *block;

	numAllocs++;

	if ( num <= 0 ) {
		return NULL;
	}

	block = AllocInternal( num );
	if ( block == NULL ) {
		return NULL;
	}
	block = ResizeInternal( block, num );
	if ( block == NULL ) {
		return NULL;
	}

#ifdef DYNAMIC_BLOCK_ALLOC_CHECK
	CheckMemory();
#endif

	numUsedBlocks++;
	usedBlockMemory += block->GetSize();

	return block->GetMemory();
}

template<class type, uint32_t baseBlockSize, uint32_t minBlockSize>
type *idDynamicBlockAlloc<type, baseBlockSize, minBlockSize>::Resize( type *ptr, const uint32_t num ) {

	numResizes++;

	if ( ptr == NULL ) {
		return Alloc( num );
	}

	if ( num <= 0 ) {
		Free( ptr );
		return NULL;
	}

	idDynamicBlock<type> *block = ( idDynamicBlock<type> * ) ( ( (byte *) ptr ) - sizeof( idDynamicBlock<type> ) );

	usedBlockMemory -= block->GetSize();

	block = ResizeInternal( block, num );
	if ( block == NULL ) {
		return NULL;
	}

#ifdef DYNAMIC_BLOCK_ALLOC_CHECK
	CheckMemory();
#endif

	usedBlockMemory += block->GetSize();

	return block->GetMemory();
}

template<class type, uint32_t baseBlockSize, uint32_t minBlockSize>
void idDynamicBlockAlloc<type, baseBlockSize, minBlockSize>::Free( type *ptr ) {

	numFrees++;

	if ( ptr == NULL ) {
		return;
	}

	idDynamicBlock<type> *block = ( idDynamicBlock<type> * ) ( ( (byte *) ptr ) - sizeof( idDynamicBlock<type> ) );

	numUsedBlocks--;
	usedBlockMemory -= block->GetSize();

	FreeInternal( block );

#ifdef DYNAMIC_BLOCK_ALLOC_CHECK
	CheckMemory();
#endif
}

template<class type, uint32_t baseBlockSize, uint32_t minBlockSize>
const char *idDynamicBlockAlloc<type, baseBlockSize, minBlockSize>::CheckMemory( const type *ptr ) const {
	idDynamicBlock<type> *block;

	if ( ptr == NULL ) {
		return NULL;
	}

	block = ( idDynamicBlock<type> * ) ( ( (byte *) ptr ) - sizeof( idDynamicBlock<type> ) );

	if ( block->node != NULL ) {
		return "memory has been freed";
	}

#ifdef DYNAMIC_BLOCK_ALLOC_CHECK
	if ( block->id[0] != 0x11111111 || block->id[1] != 0x22222222 || block->id[2] != 0x33333333 ) {
		return "memory has invalid id";
	}
	if ( block->allocator != (void*)this ) {
		return "memory was allocated with different allocator";
	}
#endif

	/* base blocks can be larger than baseBlockSize which can cause this code to fail
	idDynamicBlock<type> *base;
	for ( base = firstBlock; base != NULL; base = base->next ) {
		if ( base->IsBaseBlock() ) {
			if ( ((int)block) >= ((int)base) && ((int)block) < ((int)base) + baseBlockSize ) {
				break;
			}
		}
	}
	if ( base == NULL ) {
		return "no base block found for memory";
	}
	*/

	return NULL;
}

template<class type, uint32_t baseBlockSize, uint32_t minBlockSize>
void idDynamicBlockAlloc<type, baseBlockSize, minBlockSize>::Clear( void ) {
	firstBlock = lastBlock = NULL;
	allowAllocs = true;
	lockMemory = false;
	numBaseBlocks = 0;
	baseBlockMemory = 0;
	numUsedBlocks = 0;
	usedBlockMemory = 0;
	numFreeBlocks = 0;
	freeBlockMemory = 0;
	numAllocs = 0;
	numResizes = 0;
	numFrees = 0;

#ifdef DYNAMIC_BLOCK_ALLOC_CHECK
	blockId[0] = 0x11111111;
	blockId[1] = 0x22222222;
	blockId[2] = 0x33333333;
#endif
}

template<class type, uint32_t baseBlockSize, uint32_t minBlockSize>
idDynamicBlock<type> *idDynamicBlockAlloc<type, baseBlockSize, minBlockSize>::AllocInternal( const uint32_t num ) {
	idDynamicBlock<type> *block;
	uint32_t alignedBytes = ( num * sizeof( type ) + 15 ) & ~15;

	block = freeTree.FindSmallestLargerEqual( alignedBytes );
	if ( block != NULL ) {
		UnlinkFreeInternal( block );
	} else if ( allowAllocs ) {
		uint32_t allocSize = MAX( baseBlockSize, alignedBytes + sizeof( idDynamicBlock<type> ) );
		block = ( idDynamicBlock<type> * ) Mem_Alloc16( allocSize );
		if ( lockMemory ) {
//			idLib::sys->LockMemory( block, baseBlockSize );
		}
#ifdef DYNAMIC_BLOCK_ALLOC_CHECK
		memcpy( block->id, blockId, sizeof( block->id ) );
		block->allocator = (void*)this;
#endif
		block->SetSize( allocSize - sizeof( idDynamicBlock<type> ), true );
		block->next = NULL;
		block->prev = lastBlock;
		if ( lastBlock ) {
			lastBlock->next = block;
		} else {
			firstBlock = block;
		}
		lastBlock = block;
		block->node = NULL;

		numBaseBlocks++;
		baseBlockMemory += allocSize;
	}

	return block;
}

template<class type, uint32_t baseBlockSize, uint32_t minBlockSize>
idDynamicBlock<type> *idDynamicBlockAlloc<type, baseBlockSize, minBlockSize>::ResizeInternal( idDynamicBlock<type> *block, const uint32_t num ) {
	uint32_t alignedBytes = ( num * sizeof( type ) + 15 ) & ~15;

#ifdef DYNAMIC_BLOCK_ALLOC_CHECK
	assert( block->id[0] == 0x11111111 && block->id[1] == 0x22222222 && block->id[2] == 0x33333333 && block->allocator == (void*)this );
#endif

	// if the new size is larger
	if ( alignedBytes > block->GetSize() ) {

		idDynamicBlock<type> *nextBlock = block->next;

		// try to annexate the next block if it's free
		if ( nextBlock && !nextBlock->IsBaseBlock() && nextBlock->node != NULL &&
				block->GetSize() + sizeof( idDynamicBlock<type> ) + nextBlock->GetSize() >= alignedBytes ) {

			UnlinkFreeInternal( nextBlock );
			block->SetSize( block->GetSize() + sizeof( idDynamicBlock<type> ) + nextBlock->GetSize(), block->IsBaseBlock() );
			block->next = nextBlock->next;
			if ( nextBlock->next ) {
				nextBlock->next->prev = block;
			} else {
				lastBlock = block;
			}
		} else {
			// allocate a new block and copy
			idDynamicBlock<type> *oldBlock = block;
			block = AllocInternal( num );
			if ( block == NULL ) {
				return NULL;
			}
			memcpy( block->GetMemory(), oldBlock->GetMemory(), oldBlock->GetSize() );
			FreeInternal( oldBlock );
		}
	}

	// if the unused space at the end of this block is large enough to hold a block with at least one element
	if ( block->GetSize() - alignedBytes - sizeof( idDynamicBlock<type> ) < MAX( minBlockSize, sizeof( type ) ) ) {
		return block;
	}

	idDynamicBlock<type> *newBlock;

	newBlock = ( idDynamicBlock<type> * ) ( ( (byte *) block ) + sizeof( idDynamicBlock<type> ) + alignedBytes );
#ifdef DYNAMIC_BLOCK_ALLOC_CHECK
	memcpy( newBlock->id, blockId, sizeof( newBlock->id ) );
	newBlock->allocator = (void*)this;
#endif
	newBlock->SetSize( block->GetSize() - alignedBytes - sizeof( idDynamicBlock<type> ), false );
	newBlock->next = block->next;
	newBlock->prev = block;
	if ( newBlock->next ) {
		newBlock->next->prev = newBlock;
	} else {
		lastBlock = newBlock;
	}
	newBlock->node = NULL;
	block->next = newBlock;
	block->SetSize( alignedBytes, block->IsBaseBlock() );

	FreeInternal( newBlock );

	return block;
}

template<class type, uint32_t baseBlockSize, uint32_t minBlockSize>
void idDynamicBlockAlloc<type, baseBlockSize, minBlockSize>::FreeInternal( idDynamicBlock<type> *block ) {

	assert( block->node == NULL );

#ifdef DYNAMIC_BLOCK_ALLOC_CHECK
	assert( block->id[0] == 0x11111111 && block->id[1] == 0x22222222 && block->id[2] == 0x33333333 && block->allocator == (void*)this );
#endif

	// try to merge with a next free block
	idDynamicBlock<type> *nextBlock = block->next;
	if ( nextBlock && !nextBlock->IsBaseBlock() && nextBlock->node != NULL ) {
		UnlinkFreeInternal( nextBlock );
		block->SetSize( block->GetSize() + sizeof( idDynamicBlock<type> ) + nextBlock->GetSize(), block->IsBaseBlock() );
		block->next = nextBlock->next;
		if ( nextBlock->next ) {
			nextBlock->next->prev = block;
		} else {
			lastBlock = block;
		}
	}

	// try to merge with a previous free block
	idDynamicBlock<type> *prevBlock = block->prev;
	if ( prevBlock && !block->IsBaseBlock() && prevBlock->node != NULL ) {
		UnlinkFreeInternal( prevBlock );
		prevBlock->SetSize( prevBlock->GetSize() + sizeof( idDynamicBlock<type> ) + block->GetSize(), prevBlock->IsBaseBlock() );
		prevBlock->next = block->next;
		if ( block->next ) {
			block->next->prev = prevBlock;
		} else {
			lastBlock = prevBlock;
		}
		LinkFreeInternal( prevBlock );
	} else {
		LinkFreeInternal( block );
	}
}

template<class type, uint32_t baseBlockSize, uint32_t minBlockSize>
GDR_INLINE void idDynamicBlockAlloc<type, baseBlockSize, minBlockSize>::LinkFreeInternal( idDynamicBlock<type> *block ) {
	block->node = freeTree.Add( block, block->GetSize() );
	numFreeBlocks++;
	freeBlockMemory += block->GetSize();
}

template<class type, uint32_t baseBlockSize, uint32_t minBlockSize>
GDR_INLINE void idDynamicBlockAlloc<type, baseBlockSize, minBlockSize>::UnlinkFreeInternal( idDynamicBlock<type> *block ) {
	freeTree.Remove( block->node );
	block->node = NULL;
	numFreeBlocks--;
	freeBlockMemory -= block->GetSize();
}

template<class type, uint32_t baseBlockSize, uint32_t minBlockSize>
void idDynamicBlockAlloc<type, baseBlockSize, minBlockSize>::CheckMemory( void ) const {
	idDynamicBlock<type> *block;

	for ( block = firstBlock; block != NULL; block = block->next ) {
		// make sure the block is properly linked
		if ( block->prev == NULL ) {
			Assert( firstBlock == block );
		} else {
			Assert( block->prev->next == block );
		}
		if ( block->next == NULL ) {
			Assert( lastBlock == block );
		} else {
			Assert( block->next->prev == block );
		}
	}
}

#endif