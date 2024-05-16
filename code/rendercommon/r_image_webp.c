#include "../engine/n_shared.h"
#include "../rendercommon/r_public.h"
#include "../game/g_game.h"
#ifdef USE_OPENGL_API
#include "../rendergl/rgl_local.h"
#include "../rendergl/stb_image.h"
#endif
#ifdef USE_VULKAN_API
#include "../rendervk/rvk_local.h"
#include "../rendervk/stb_image.h"
#endif
#include <webp/decode.h>

static qboolean RawImage_HasAlpha( const byte *scan, uint32_t numPixels )
{
	uint32_t i;

	if (!scan)
		return qtrue;
	
	for (i = 0; i < numPixels; i++) {
		if (scan[i*4 + 3] != 255) {
			return qtrue;
		}
	}
	return qfalse;
}


void R_LoadWebp( const char *name, byte **pic, int *width, int *height, int *channels )
{
    unsigned	columns, rows, numPixels;
	byte	*pixbuf;
	int		row, column;
	byte	*buf_p;
	byte	*end;
	union {
		byte *b;
		void *v;
	} buffer;
    WebPDecBuffer buf;
    WebPDecoderConfig config;
	int length;
    int size, stride;

	*pic = NULL;

	if ( width ) {
		*width = 0;
    }
	if ( height ) {
		*height = 0;
    }

	//
	// load the file
	//
	length = ri.FS_LoadFile( name, &buffer.v );
	if ( !buffer.b || length < 0 ) {
		return;
	}

    if ( !WebPGetInfo( buf_p, length, width, height ) ) {
        ri.Printf( PRINT_WARNING, "LoadWebp: %s has an invalid format\n", name );
        ri.FS_FreeFile( buffer.v );
        return;
    }

    stride = *width + sizeof( uint32_t );
    size = *height * stride;
    pixbuf = (byte *)ri.Malloc( size );

    if ( RawImage_HasAlpha( buf_p, length ) ) {
        // decode into rgba
        if ( !WebPDecodeRGBAInto( buf_p, length, pixbuf, size, stride ) ) {
            ri.Error( ERR_DROP, "LoadWebp: %s has bad header or data\n", name );
        }
        *channels = 4;
    } else {
        // decode into rgb
        if ( !WebPDecodeRGBInto( buf_p, length, pixbuf, size, stride ) ) {
            ri.Error( ERR_DROP, "LoadWebp: %s has bad header or data\n", name );
        }
        *channels = 3;
    }

    *pic = pixbuf;

    ri.FS_FreeFile( buffer.v );
}
