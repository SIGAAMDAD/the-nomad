#include "n_shared.h"
#include "memio.h"

typedef enum {
    MODE_READ,
    MODE_WRITE
} memfile_mode_t;

struct _MEMFILE {
    unsigned char *buf;
    size_t buflen;
    size_t allocated;
    size_t position;
    memfile_mode_t mode;
};

MEMFILE *mem_fopen_read( void *buf, size_t buflen )
{
    MEMFILE *file;

    file = (MEMFILE *)S_Malloc( sizeof( *file ) );

    file->buf = (unsigned char *)buf;
    file->buflen = buflen;
    file->position = 0;
    file->mode = MODE_READ;

    return file;
}

size_t mem_fread( void *buf, size_t size, size_t nmemb, MEMFILE *stream )
{
	size_t items;

	if ( stream->mode != MODE_READ ) {
		Con_Printf( COLOR_YELLOW "WARNING: not a read stream\n" );
		return 0;
	}

	// trying to read more bytes than we have left?
	items = nmemb;
	if ( items * size > stream->buflen - stream->position ) {
		items = ( stream->buflen - stream->position ) / size;
	}
	
	memcpy( buf, stream->buf + stream->position, items * size );

	stream->position += items * size;
	
	return items;
}

MEMFILE *mem_fopen_write( void )
{
    MEMFILE *file;

    file = (MEMFILE *)S_Malloc( sizeof( *file ) );

    file->allocated = 1024;
    file->buf = (unsigned char *)Z_Malloc( file->allocated, TAG_STATIC );
    file->position = 0;
    file->buflen = 0;
    file->mode = MODE_WRITE;

    return file;
}

size_t mem_fwrite( const void *ptr, size_t size, size_t nmemb, MEMFILE *stream )
{
	size_t bytes;

	if ( stream->mode != MODE_WRITE ) {
		return -1;
	}
	
	// More bytes than can fit in the buffer?
	// If so, reallocate bigger.

	bytes = size * nmemb;
	
	while ( bytes > stream->allocated - stream->position ) {
		unsigned char *newbuf;

		newbuf = (unsigned char *)Z_Malloc( stream->allocated * 2, TAG_STATIC );
		memcpy( newbuf, stream->buf, stream->allocated );
		Z_Free( stream->buf );
		stream->buf = newbuf;
		stream->allocated *= 2;
	}

	// Copy into buffer
	
	memcpy( stream->buf + stream->position, ptr, bytes );
	stream->position += bytes;

	if ( stream->position > stream->buflen ) {
		stream->buflen = stream->position;
    }

	return nmemb;
}

int mem_fputs( const char *str, MEMFILE *stream )
{
	if ( str == NULL ) {
		return -1;
	}

	return mem_fwrite( str, sizeof( char ), strlen( str ), stream );
}

void mem_get_buf( MEMFILE *stream, void **buf, size_t *buflen )
{
	*buf = stream->buf;
	*buflen = stream->buflen;
}

void mem_fclose( MEMFILE *stream )
{
	if ( stream->mode == MODE_WRITE ) {
		Z_Free( stream->buf );
	}

	Z_Free( stream );
}

size_t mem_ftell( MEMFILE *stream )
{
	return stream->position;
}

int mem_fseek( MEMFILE *stream, signed long position, mem_rel_t whence )
{
	unsigned int newpos;

	switch ( whence ) {
	case MEM_SEEK_SET:
		newpos = (int)position;
		break;
	case MEM_SEEK_CUR:
	    newpos = (int)( stream->position + position );
	    break;
	case MEM_SEEK_END:
		newpos = (int)(stream->buflen + position );
		break;
	default:
		return -1;
	}

	// We should allow seeking to the end of a MEMFILE with MEM_SEEK_END to
	// match stdio.h behavior.

	if ( newpos <= stream->buflen ) {
		stream->position = newpos;
		return 0;
	}
	else {
		Con_Printf( COLOR_RED "Error seeking to %u\n", newpos );
		return -1;
	}
}
