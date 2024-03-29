#if !defined( __MODULE_ALLOC__ ) && !defined( INCLUDE_NLOHMANN_JSON_HPP_ )
#define __MODULE_ALLOC__

#pragma once

#undef new
#undef delete

#ifdef MODULE_LIB
#ifndef _NOMAD_DEBUG

extern void *		Mem_Alloc( const uint32_t size );
extern void *		Mem_ClearedAlloc( const uint32_t size );
extern void			Mem_Free( void *ptr );

GDR_INLINE void *operator new( size_t s ) {
	return Mem_ClearedAlloc( s );
}
GDR_INLINE void *operator new[]( size_t s ) {
	return Mem_ClearedAlloc( s );
}
GDR_INLINE void operator delete( void *p ) noexcept {
	Mem_Free( p );
}
GDR_INLINE void operator delete[]( void *p ) noexcept {
	Mem_Free( p );
}
GDR_INLINE void operator delete[]( void *p, size_t ) noexcept {
    Mem_Free( p );
}

#else

extern void *		Mem_AllocDebug( const uint32_t size, const char *fileName, const uint32_t lineNumber );
extern void *		Mem_ClearedAllocDebug( const uint32_t size, const char *fileName, const uint32_t lineNumber );
extern void			Mem_FreeDebug( void *ptr, const char *fileName, const uint32_t lineNumber );
extern char *		Mem_CopyStringDebug( const char *in, const char *fileName, const uint32_t lineNumber );
extern void *		Mem_Alloc16Debug( const uint32_t size, const char *fileName, const uint32_t lineNumber );
extern void			Mem_Free16Debug( void *ptr, const char *fileName, const uint32_t lineNumber );

GDR_INLINE void *operator new( size_t s, int t1, int t2, const char *fileName, const uint32_t lineNumber ) {
	return Mem_AllocDebug( s, fileName, lineNumber );
}
GDR_INLINE void operator delete( void *p, int t1, int t2, const char *fileName, const uint32_t lineNumber ) {
	Mem_FreeDebug( p, fileName, lineNumber );
}
GDR_INLINE void *operator new[]( size_t s, int t1, int t2, const char *fileName, const uint32_t lineNumber ) {
	return Mem_AllocDebug( s, fileName, lineNumber );
}
GDR_INLINE void operator delete[]( void *p, int t1, int t2, const char *fileName, const uint32_t lineNumber ) {
	Mem_FreeDebug( p, fileName, lineNumber );
}
GDR_INLINE void *operator new( size_t s ) {
	return Mem_AllocDebug( s, "", 0 );
}
GDR_INLINE void operator delete( void *p ) {
	Mem_FreeDebug( p, "", 0 );
}
GDR_INLINE void *operator new[]( size_t s ) {
	return Mem_AllocDebug( s, "", 0 );
}
GDR_INLINE void operator delete[]( void *p ) {
	Mem_FreeDebug( p, "", 0 );
}

#define		Mem_Alloc( size )				Mem_AllocDebug( size, __FILE__, __LINE__ )
#define		Mem_ClearedAlloc( size )		Mem_ClearedAllocDebug( size, __FILE__, __LINE__ )
#define		Mem_Free( ptr )					Mem_FreeDebug( ptr, __FILE__, __LINE__ )
#define		Mem_CopyString( s )				Mem_CopyStringDebug( s, __FILE__, __LINE__ )
#define		Mem_Alloc16( size )				Mem_Alloc16Debug( size, __FILE__, __LINE__ )
#define		Mem_Free16( ptr )				Mem_Free16Debug( ptr, __FILE__, __LINE__ )

#endif
#endif

#endif
