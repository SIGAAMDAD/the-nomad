#include "n_shared.h"
#include "n_common.h"

#ifdef POSIX
#include <zlib.h>
#include <bzlib.h>

#define BUFFER_SIZE (8*1024)

static inline const char *bzip2_strerror( int err )
{
	switch ( err ) {
	case BZ_DATA_ERROR: return "(BZ_DATA_ERROR) buffer provided to bzip2 was corrupted";
	case BZ_MEM_ERROR: return "(BZ_MEM_ERROR) memory allocation request made by bzip2 failed";
	case BZ_DATA_ERROR_MAGIC: return "(BZ_DATA_ERROR_MAGIC) buffer was not compressed with bzip2, it did not contain \"BZA\"";
	case BZ_IO_ERROR: return va("(BZ_IO_ERROR) failure to read or write, file I/O error");
	case BZ_UNEXPECTED_EOF: return "(BZ_UNEXPECTED_EOF) unexpected end of data stream";
	case BZ_OUTBUFF_FULL: return "(BZ_OUTBUFF_FULL) buffer overflow";
	case BZ_SEQUENCE_ERROR: return "(BZ_SEQUENCE_ERROR) bad function call error, please report this bug";
	case BZ_OK:
		break;
	};
	return "No Error... How?";
}

static inline bool CheckBZIP2( int errcode, uint64_t buflen, const char *action )
{
	switch ( errcode ) {
	case BZ_OK:
	case BZ_RUN_OK:
	case BZ_FLUSH_OK:
	case BZ_FINISH_OK:
	case BZ_STREAM_END:
		return true; // all good
	case BZ_CONFIG_ERROR:
	case BZ_DATA_ERROR:
	case BZ_DATA_ERROR_MAGIC:
	case BZ_IO_ERROR:
	case BZ_MEM_ERROR:
	case BZ_PARAM_ERROR:
	case BZ_SEQUENCE_ERROR:
	case BZ_OUTBUFF_FULL:
	case BZ_UNEXPECTED_EOF:
	    N_Error( ERR_FATAL, "BZip2 Failure: failure on %s of %lu bytes. BZIP2 error reason:\n\t%s", action, buflen, bzip2_strerror( errcode ) );
		break;
	};
	return false;
}

static void *GetMemory( uint64_t size )
{
    void *buf;
    
    buf = malloc( size );
	if ( !buf ) {
        N_Error( ERR_FATAL, "GetMemory: failed to allocate %lu bytes", size );
    }
    return memset( buf, 0, size );
}

static char *Compress_BZIP2( void *buf, uint64_t buflen, uint64_t *outlen )
{
	char *out, *newbuf;
	unsigned int len;
	int ret;

	Con_Printf( "Compressing %lu bytes with bzip2...\n", buflen );

	len = buflen;
	out = (char *)GetMemory( buflen );

	ret = BZ2_bzBuffToBuffCompress( out, &len, (char *)buf, buflen, 9, 4, 50 );
	if ( !CheckBZIP2( ret, buflen, "Compression" ) ) {
		return (char *)buf;
	}

	Con_Printf( "Successful compression of %lu to %u bytes with bzip2.\n", buflen, len );
	newbuf = (char *)Z_Malloc( len, TAG_BFF );
	memcpy( newbuf, out, len );
	free( out );
	*outlen = len;

	return newbuf;
}

static inline const char *zlib_strerror( int err )
{
	switch ( err ) {
	case Z_DATA_ERROR: return "(Z_DATA_ERROR) buffer provided to zlib was corrupted";
	case Z_BUF_ERROR: return "(Z_BUF_ERROR) buffer overflow";
	case Z_STREAM_ERROR: return "(Z_STREAM_ERROR) bad params passed to zlib, please report this bug";
	case Z_MEM_ERROR: return "(Z_MEM_ERROR) memory allocation request made by zlib failed";
	case Z_OK:
		break;
	};
	return "No Error... How?";
}

static void *zalloc( voidpf opaque, uInt items, uInt size )
{
	(void)opaque;
	(void)items;
	return GetMemory( size );
}

static void zfreeMemory( voidpf opaque, voidpf address )
{
	(void)opaque;
	free( (void *)address );
}

static char *Compress_ZLIB( void *buf, uint64_t buflen, uint64_t *outlen )
{
	char *out, *newbuf;
	const uint64_t expectedLen = buflen / 2;
	int ret;

	out = (char *)GetMemory( buflen );

#if 0
	stream.zalloc = zalloc;
	stream.zfree = zfree;
	stream.opaque = Z_NULL;
	stream.next_in = (Bytef *)buf;
	stream.avail_in = buflen;
	stream.next_out = (Bytef *)out;
	stream.avail_out = buflen;

	ret = deflateInit2( &stream, Z_BEST_COMPRESSION, Z_DEFLATED, 15, 9, );
	if (ret != Z_OK) {
		Error( "Failed to compress buffer of %lu bytes (inflateInit2)", buflen );
		return NULL;
	}
	inflate

	do {
		ret = inflate( &stream, Z_SYNC_FLUSH );

		switch (ret) {
		case Z_NEED_DICT:
		case Z_STREAM_ERROR:
			ret = Z_DATA_ERROR;
			break;
		case Z_DATA_ERRO:
		case Z_MEM_ERROR:
			inflateEnd( &stream );
			Error( "Failed to compress buffer of %lu bytes (inflate)", buflen );
			return NULL;
		};
		
		if (ret != Z_STREAM_END) {
			newbuf = (char *)GetMemory( buflen * 2 );
			memcpy( newbuf, out, buflen * 2 );
			FreeMemory( out );
			out = newbuf;

			stream.next_out = (Bytef *)( out + buflen );
			stream.avail_out = buflen;
			buflen *= 2;
		}
	} while ( ret != Z_STREAM_END );
#endif
	Con_Printf( "Compressing %lu bytes with zlib...\n", buflen );

	ret = compress2( (Bytef *)out, (uLongf *)outlen, (const Bytef *)buf, buflen, Z_BEST_COMPRESSION );
	if ( ret != Z_OK ) {
		N_Error( ERR_FATAL, "ZLib Compression Failure: failure on compression of %lu bytes. ZLIB error reason:\n\t%s", buflen, zError( ret ) );
	}
	
	Con_Printf( "Successful compression of %lu to %lu bytes with zlib.\n", buflen, *outlen );
	newbuf = (char *)Z_Malloc( *outlen, TAG_BFF );
	memcpy( newbuf, out, *outlen );
	free( out );

	return newbuf;
}

char *Compress( void *buf, uint64_t buflen, uint64_t *outlen, int compression )
{
	switch ( compression ) {
	case COMPRESS_BZIP2:
		return Compress_BZIP2( buf, buflen, outlen );
	case COMPRESS_ZLIB:
		return Compress_ZLIB( buf, buflen, outlen );
	default:
		break;
	};
	return (char *)buf;
}

static char *Decompress_BZIP2( void *buf, uint64_t buflen, uint64_t *outlen )
{
	char *out, *newbuf;
	unsigned int len;
	int ret, attempts;

	len = *outlen * 2;
	out = (char *)Hunk_AllocateTempMemory( len );

	attempts = 0;
	do {
		Con_Printf( "Decompressing %lu bytes with bzip2 to %u...\n", buflen, len );

		ret = BZ2_bzBuffToBuffDecompress( out, &len, (char *)buf, buflen, 0, 1 );
		if ( ret == BZ_OK ) {
			break;
		}
		if ( attempts++ > 3 || ret != BZ_OUTBUFF_FULL ) {
			// give up
			CheckBZIP2( ret, buflen, "Decompression" );
		}
		if ( ret == BZ_OUTBUFF_FULL ) {
			// resize the temp buffer
			Con_DPrintf( "BZ_OUTBUFF_FULL: retrying with a bigger buffer.\n" );
			len *= 2;
			Hunk_FreeTempMemory( out );
			out = (char *)Hunk_AllocateTempMemory( len );
		}
	} while ( 1 );

	Con_Printf( "Successful decompression of %lu to %u bytes with bzip2 (inflate %0.02f%%).\n", buflen, len,
		( (float)buflen / (float)len ) * 100.0f );
	
	*outlen = len;
	newbuf = (char *)Z_Malloc( len, TAG_BFF );
	memcpy( newbuf, out, len );
	Hunk_FreeTempMemory( out );
	out = newbuf;

	return out;
}

char *Decompress_ZLIB( void *buf, uint64_t buflen, uint64_t *outlen )
{
	char *out, *newbuf;
	Bytef *dataPtr;
	int ret;
	z_stream stream;
	uint64_t uncompressedBytes;

	*outlen = buflen * 4;
	out = (char *)Hunk_AllocateTempMemory( *outlen );

	stream.zalloc = Z_NULL;
	stream.zfree = Z_NULL;
	stream.opaque = Z_NULL;
	stream.avail_in = 0;
	stream.next_in = Z_NULL;

	Con_Printf( "Decompressing %lu bytes with zlib...\n", buflen );

/*
	ret = inflateInit2( &stream, 16 + MAX_WBITS );
	if ( ret != Z_OK ) {
		N_Error( ERR_FATAL, "inflateInit2 failed: %s", zError( ret ) );
	}

	stream.avail_in = buflen;
	stream.next_in = (Bytef *)buf;

	dataPtr = (Bytef *)out;

	do {
		stream.avail_out = *outlen;
		stream.next_out = dataPtr;
		ret = inflateInit2( &stream, -MAX_WBITS );
		switch ( ret ) {
		case Z_NEED_DICT:
			ret = Z_DATA_ERROR;
		case Z_DATA_ERROR:
		case Z_MEM_ERROR:
		case Z_STREAM_ERROR:
			N_Error( ERR_FATAL, "inflateInit2 failed: %s", zError( ret ) );
		};
		uncompressedBytes = *outlen - stream.avail_out;
		*outlen -= uncompressedBytes;
		dataPtr += uncompressedBytes;
	} while ( stream.avail_out == 0 );

	inflateEnd( &stream );
*/
	
	ret = uncompress( (Bytef *)out, (uLongf *)outlen, (const Bytef *)buf, buflen );
	if ( ret != Z_OK ) {
		N_Error( ERR_FATAL, "ZLib Decompression Failure: failure on decompression of %lu bytes. ZLIB error reason: %s", buflen,
			zlib_strerror( ret ) );
		return (char *)buf;
	}

	newbuf = (char *)Z_Malloc( *outlen, TAG_BFF );
	memcpy( newbuf, out, *outlen );
	Hunk_FreeTempMemory( out );
	out = newbuf;
	
	Con_Printf( "Successful decompression of %lu bytes to %lu bytes with zlib.\n", buflen, *outlen );

	return out;
}

char *Decompress( void *buf, uint64_t buflen, uint64_t *outlen, int compression )
{
	switch ( compression ) {
	case COMPRESS_BZIP2:
		return Decompress_BZIP2( buf, buflen, outlen );
	case COMPRESS_ZLIB:
		return Decompress_ZLIB( buf, buflen, outlen );
	default:
		break;
	};
	return (char *)buf;
}
#else
char *Compress( void *buf, uint64_t buflen, uint64_t *outlen, int compression ) {
	return (char *)buf;
}

char *Decompress( void *buf, uint64_t buflen, uint64_t *outlen, int compression ) {
	return (char *)buf;
}
#endif
