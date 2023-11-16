#ifndef __N_BUFFER__
#define __N_BUFFER__

#pragma once

class CFileBuffer
{
public:
    constexpr CFileBuffer( void );
    ~CFileBuffer();

    void *GetData( void );
    const void *GetData( void ) const;
    uint64_t Size( void ) const;
    void Free( void );
    void Load( const char *npath );
private:
    void *m_pData;
    uint64_t m_nSize;
};

GDR_INLINE constexpr CFileBuffer::CFileBuffer( void )
    : m_pData( NULL ), m_nSize( 0 )
{
}

GDR_INLINE CFileBuffer::~CFileBuffer() {
    Free();
}

GDR_INLINE void *CFileBuffer::GetData( void ) {
    return m_pData;
}
GDR_INLINE const void *CFileBuffer::GetData( void ) const {
    return m_pData;
}
GDR_INLINE uint64_t CFileBuffer::Size( void ) const {
    return m_nSize;
}

GDR_INLINE void CFileBuffer::Free( void ) {
    if (m_pData) {
        Hunk_FreeTempMemory( m_pData );
    }
}

GDR_INLINE void CFileBuffer::Load( const char *npath ) {
    FS_LoadFile( npath, &m_pData );
}

#endif