#ifndef __MEMIO_H__
#define __MEMIO_H__

#pragma once

#include "n_shared.h"

typedef struct _MEMFILE MEMFILE;

typedef enum {
    MEM_SEEK_SET,
    MEM_SEEK_CUR,
    MEM_SEEK_END
} mem_rel_t;

// c-style implementation
MEMFILE *mem_fopen_read( void *buf, size_t buflen );
size_t mem_fread( void *buf, size_t size, size_t nmemb, MEMFILE *stream );
MEMFILE *mem_fopen_write( void );
size_t mem_fwrite( const void *ptr, size_t size, size_t nmemb, MEMFILE *stream );
int mem_fputs( const char *str, MEMFILE *stream );
void mem_get_buf( MEMFILE *stream, void **buf, size_t *buflen );
void mem_fclose( MEMFILE *stream );
size_t mem_ftell( MEMFILE *stream );
int mem_fseek( MEMFILE *stream, signed long offset, mem_rel_t whence );

class CMemBuffer
{
public:
    CMemBuffer( void );
    CMemBuffer( void *buf, size_t buflen );
    ~CMemBuffer();

    void Close( void );
    size_t Read( void *buf, size_t size, size_t nmemb );
    size_t Write( const void *ptr, size_t size, size_t nmemb );
    int PutString( const char *str );
    size_t GetPosition( void ) const;
    int SetPosition( signed long offset, mem_rel_t whence );

    void GetBuffer( void **buf, size_t *buflen );
private:
    MEMFILE *pFile;
};

GDR_INLINE CMemBuffer::CMemBuffer( void )
{
    pFile = mem_fopen_write();
}

GDR_INLINE CMemBuffer::CMemBuffer( void *buf, size_t buflen )
{
    pFile = mem_fopen_read( buf, buflen );
}

GDR_INLINE CMemBuffer::~CMemBuffer()
{
    if ( pFile ) {
        mem_fclose( pFile );
    }
}

GDR_INLINE size_t CMemBuffer::Read( void *buf, size_t size, size_t nmemb )
{
    if ( !pFile ) {
        return 0;
    }
    return mem_fread( buf, size, nmemb, pFile );
}

GDR_INLINE size_t CMemBuffer::Write( const void *ptr, size_t size, size_t nmemb )
{
    if ( !pFile ) {
        return 0;
    }
    return mem_fwrite( ptr, size, nmemb, pFile );
}

GDR_INLINE void CMemBuffer::Close( void )
{
    if ( pFile != NULL ) {
        mem_fclose( pFile );
    }
    pFile = NULL;
}

GDR_INLINE int CMemBuffer::PutString( const char *str )
{
    if ( !pFile ) {
        return -1;
    }
    return mem_fputs( str, pFile );
}

GDR_INLINE size_t CMemBuffer::GetPosition( void ) const
{
    if ( !pFile ) {
        return 0;
    }
    return mem_ftell( pFile );
}

GDR_INLINE int CMemBuffer::SetPosition( signed long offset, mem_rel_t whence )
{
    if ( !pFile ) {
        return -1;
    }
    return mem_fseek( pFile, offset, whence );
}

GDR_INLINE void CMemBuffer::GetBuffer( void **buf, size_t *buflen )
{
    if ( !pFile ) {
        return;
    }
    mem_get_buf( pFile, buf, buflen );
}

#endif