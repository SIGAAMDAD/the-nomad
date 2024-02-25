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
	int		num;
	int		minSize;
	int		maxSize;
	int		totalSize;
} memoryStats_t;


void		Mem_Init( void );
void		Mem_Shutdown( void );
void		Mem_EnableLeakTest( const char *name );
void		Mem_ClearFrameStats( void );
void		Mem_GetFrameStats( memoryStats_t &allocs, memoryStats_t &frees );
void		Mem_GetStats( memoryStats_t &stats );
void		Mem_Dump_f( void );
void		Mem_DumpCompressed_f( void );
void		Mem_AllocDefragBlock( void );

void *		Mem_Alloc( const int size );
void *		Mem_ClearedAlloc( const int size );
void		Mem_Free( void *ptr );
char *		Mem_CopyString( const char *in );
void *		Mem_Alloc16( const int size );
void		Mem_Free16( void *ptr );

#ifdef MODULE_LIB

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

#endif