#ifndef __N_ALLOCATOR__
#define __N_ALLOCATOR__

#pragma once

#include <EASTL/allocator.h>

class CHunkTempAllocator
{
public:
    EASTL_ALLOCATOR_EXPLICIT CHunkTempAllocator(const char* pName = EASTL_NAME_VAL(EASTL_ALLOCATOR_DEFAULT_NAME)) { }
	CHunkTempAllocator(const CHunkTempAllocator& x) { }
	CHunkTempAllocator(const CHunkTempAllocator& x, const char* pName) { }

	CHunkTempAllocator& operator=( const CHunkTempAllocator& x ) = default;

	void* allocate( size_t n, int flags = 0 );
	void* allocate( size_t n, size_t alignment, size_t offset, int flags = 0 );
	void  deallocate( void* p, size_t n );

	const char* get_name( void ) const { return NULL; }
	void        set_name( const char* pName ) { }
private:
	#if EASTL_NAME_ENABLED
		const char* mpName; // Debug name, used to track memory.
	#endif
};

GDR_INLINE void *CHunkTempAllocator::allocate( size_t n, int flags )
{
	return Hunk_AllocateTempMemory( n );
}

GDR_INLINE void *CHunkTempAllocator::allocate( size_t n, size_t alignment, size_t offset, int flags )
{
	return Hunk_AllocateTempMemory( n );
}

GDR_INLINE void CHunkTempAllocator::deallocate( void *ptr, size_t )
{
	Hunk_FreeTempMemory( ptr );
}

class CHunkAllocator
{
public:
    EASTL_ALLOCATOR_EXPLICIT CHunkAllocator(const char* pName = EASTL_NAME_VAL(EASTL_ALLOCATOR_DEFAULT_NAME)) { }
	CHunkAllocator(const CHunkAllocator& x) { }
	CHunkAllocator(const CHunkAllocator& x, const char* pName) { }

	CHunkAllocator& operator=( const CHunkAllocator& x ) = default;

	void* allocate( size_t n, int flags = 0 );
	void* allocate( size_t n, size_t alignment, size_t offset, int flags = 0 );
	void  deallocate( void* p, size_t n );

	const char* get_name( void ) const { return NULL; }
	void        set_name( const char* pName ) { }
private:
	#if EASTL_NAME_ENABLED
		const char* mpName; // Debug name, used to track memory.
	#endif
};

GDR_INLINE void *CHunkAllocator::allocate( size_t n, int flags )
{
    return Hunk_Alloc( n, h_low );
}

GDR_INLINE void *CHunkAllocator::allocate( size_t n, size_t alignment, size_t offset, int flags )
{
    return Hunk_Alloc( n, h_low );
}

GDR_INLINE void CHunkAllocator::deallocate( void *ptr, size_t )
{
}

class CZoneAllocator
{
public:
    EASTL_ALLOCATOR_EXPLICIT CZoneAllocator(const char* pName = EASTL_NAME_VAL(EASTL_ALLOCATOR_DEFAULT_NAME)) { }
	CZoneAllocator(const CZoneAllocator& x) { }
	CZoneAllocator(const CZoneAllocator& x, const char* pName) { }

	CZoneAllocator& operator=( const CZoneAllocator& x ) = default;

	void* allocate( size_t n, int flags = 0 );
	void* allocate( size_t n, size_t alignment, size_t offset, int flags = 0 );
	void  deallocate( void* p, size_t n );

	const char* get_name( void ) const { return NULL; }
	void        set_name( const char* pName ) { }
private:
	#if EASTL_NAME_ENABLED
		const char* mpName; // Debug name, used to track memory.
	#endif
};

GDR_INLINE void *CZoneAllocator::allocate( size_t n, int flags )
{
    return Z_Malloc( n, TAG_STATIC );
}

GDR_INLINE void *CZoneAllocator::allocate( size_t n, size_t alignment, size_t offset, int flags )
{
    return Z_Malloc( n, TAG_STATIC );
}

GDR_INLINE void CZoneAllocator::deallocate( void *ptr, size_t )
{
    if (ptr != NULL) {
        Z_Free( ptr );
    }
}

class CSmallZoneAllocator
{
public:
    EASTL_ALLOCATOR_EXPLICIT CSmallZoneAllocator(const char* pName = EASTL_NAME_VAL(EASTL_ALLOCATOR_DEFAULT_NAME)) { }
	CSmallZoneAllocator(const CSmallZoneAllocator& x) { }
	CSmallZoneAllocator(const CSmallZoneAllocator& x, const char* pName) { }

	CSmallZoneAllocator& operator=( const CSmallZoneAllocator& x ) = default;

	void* allocate( size_t n, int flags = 0 );
	void* allocate( size_t n, size_t alignment, size_t offset, int flags = 0 );
	void  deallocate( void* p, size_t n );

	const char* get_name( void ) const { return NULL; }
	void        set_name( const char* pName ) { }
private:
	#if EASTL_NAME_ENABLED
		const char* mpName; // Debug name, used to track memory.
	#endif
};

GDR_INLINE void *CSmallZoneAllocator::allocate( size_t n, int flags )
{
	return S_Malloc( n );
}

GDR_INLINE void *CSmallZoneAllocator::allocate( size_t n, size_t alignment, size_t offset, int flags )
{
	return S_Malloc( n );
}

GDR_INLINE void CSmallZoneAllocator::deallocate( void *ptr, size_t )
{
    if (ptr != NULL) {
        Z_Free( ptr );
    }
}

using tempBuffer = eastl::vector<char, CHunkTempAllocator>;

#endif