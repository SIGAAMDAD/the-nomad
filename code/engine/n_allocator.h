#ifndef __N_ALLOCATOR__
#define __N_ALLOCATOR__

#pragma once

#include "../engine/n_common.h"
#include <EASTL/allocator.h>
#include <EASTL/vector.h>
#include <EASTL/type_traits.h>

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

template<ha_pref where>
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

template<ha_pref where>
GDR_INLINE void *CHunkAllocator<where>::allocate( size_t n, int flags )
{
    return Hunk_Alloc( n, where );
}

template<ha_pref where>
GDR_INLINE void *CHunkAllocator<where>::allocate( size_t n, size_t alignment, size_t offset, int flags )
{
    return Hunk_Alloc( n, where );
}

template<ha_pref where>
GDR_INLINE void CHunkAllocator<where>::deallocate( void *ptr, size_t )
{
}

template<memtag_t tag = TAG_STATIC>
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

template<memtag_t tag>
GDR_INLINE void *CZoneAllocator<tag>::allocate( size_t n, int flags )
{
    return Z_Malloc( n, tag );
}

template<memtag_t tag>
GDR_INLINE void *CZoneAllocator<tag>::allocate( size_t n, size_t alignment, size_t offset, int flags )
{
    return Z_Malloc( n, TAG_STATIC );
}

template<memtag_t tag>
GDR_INLINE void CZoneAllocator<tag>::deallocate( void *ptr, size_t )
{
    if (ptr != NULL) {
        Z_Free( ptr );
    }
}

template<typename T>
class CStdSmallZoneAllocator
{
public:
    EASTL_ALLOCATOR_EXPLICIT CStdSmallZoneAllocator(const char* pName = EASTL_NAME_VAL(EASTL_ALLOCATOR_DEFAULT_NAME)) { }
	CStdSmallZoneAllocator(const CStdSmallZoneAllocator& x) { }
	CStdSmallZoneAllocator(const CStdSmallZoneAllocator& x, const char* pName) { }

	typedef T value_type;
	typedef char char_type;

	CStdSmallZoneAllocator& operator=( const CStdSmallZoneAllocator& x ) = default;

	T* allocate( size_t n ) const;
	void* allocate( size_t n, int flags = 0 );
	void* allocate( size_t n, size_t alignment, size_t offset, int flags = 0 );
	void  deallocate( T* p, size_t n );

	const char* get_name( void ) const { return NULL; }
	void        set_name( const char* pName ) { }
private:
	#if EASTL_NAME_ENABLED
		const char* mpName; // Debug name, used to track memory.
	#endif
};

template<typename T>
GDR_INLINE T *CStdSmallZoneAllocator<T>::allocate( size_t n ) const
{
	T *pMem = (T *)S_Malloc( n );
	::new (pMem) T();
	return pMem;
}

template<typename T>
GDR_INLINE void *CStdSmallZoneAllocator<T>::allocate( size_t n, int flags )
{
	return S_Malloc( n );
}

template<typename T>
GDR_INLINE void *CStdSmallZoneAllocator<T>::allocate( size_t n, size_t alignment, size_t offset, int flags )
{
	return S_Malloc( n );
}

template<typename T>
GDR_INLINE void CStdSmallZoneAllocator<T>::deallocate( T *ptr, size_t )
{
    if (ptr != NULL) {
		ptr->~T();
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

template<typename T>
class CZoneDeleter
{
public:
	EA_CONSTEXPR CZoneDeleter( void ) = default;

	template<typename U>  // Enable if T* can be constructed with U* (i.e. U* is convertible to T*).
	CZoneDeleter( const CZoneDeleter<U>&, typename eastl::enable_if<eastl::is_convertible<U*, T*>::value>::type* = 0 ) { }

	void operator()( T* p ) const {
		static_assert(eastl::internal::is_complete_type_v<T>, "Attempting to call the destructor of an incomplete type");
		Z_Free( p );
	}
};

#endif