#include "rgl_local.h"

typedef struct {
    char name[MAX_NPATH];
    GLuint pboId;
    uint32_t size;
} pixelBuffer_t;

pixelBuffer_t *R_AllocatePixelBuffer( const char *name, uint32_t size )
{
    pixelBuffer_t *buf;

    buf = ri.Hunk_Alloc( sizeof( *buf ), h_low );

    N_strncpyz( buf->name, name, sizeof( buf->name ) );
    buf->size = size;

    nglGenBuffers( GL_PIXEL_PACK_BUFFER, &buf->pboId );
    nglBindBuffer( GL_PIXEL_PACK_BUFFER, buf->pboId );
    nglBufferData( GL_PIXEL_PACK_BUFFER, size, NULL, GL_STREAM_DRAW );
    nglBindBuffer( GL_PIXEL_PACK_BUFFER, 0 );

    return buf;
}
