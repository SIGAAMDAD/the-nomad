#undef new
#undef delete

GDR_INLINE void *operator new( size_t s ) {
	return Mem_Alloc( s );
}
GDR_INLINE void *operator new[]( size_t s ) {
	return Mem_Alloc( s );
}
GDR_INLINE void operator delete( void *p ) {
	Mem_Free( p );
}
GDR_INLINE void operator delete[]( void *p ) {
	Mem_Free( p );
}
GDR_INLINE void operator delete[]( void *p, size_t ) {
    Mem_Free( p );
}
