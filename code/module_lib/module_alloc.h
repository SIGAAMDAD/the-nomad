#undef new
#undef delete

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
