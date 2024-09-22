#include "rgl_local.h"

gpuBuffer_t *R_CreateBuffer( const char *name, bufferUsage_t nUsage, bufferType_t nType, uint32_t nSize, const void *pData )
{
	gpuBuffer_t *buf;

	buf = ri.Hunk_Alloc( sizeof( *buf ), h_low );

	switch ( nUsage ) {
	case USAGE_VERTEX:
		buf->glTarget = GL_ARRAY_BUFFER;
		break;
	case USAGE_INDEX:
		buf->glTarget = GL_ELEMENT_ARRAY_BUFFER;
		break;
	case USAGE_SSBO:
		buf->glTarget = GL_SHADER_STORAGE_BUFFER;
		break;
	case USAGE_UNIFORM:
		buf->glTarget = GL_UNIFORM_BUFFER;
		break;
	default:
	};

	switch ( nType ) {
	case BUFFER_DYNAMIC:
		buf->glUsage = GL_DYNAMIC_DRAW;
		break;
	case BUFFER_FRAME:
	case BUFFER_STREAM:
		buf->glUsage = GL_STREAM_DRAW;
		break;
	case BUFFER_STATIC:
		buf->glUsage = GL_STATIC_DRAW;
		break;
	default:
		break;
	};

	N_strncpyz( buf->debugName, name, sizeof( buf->debugName ) );

	return buf;
}

void R_StreamBuffer( gpuBuffer_t *buf, uint32_t nSize, const void *pData )
{
#ifdef _WIN32

#else
	nglBindBuffer( buf->glTarget, buf->nBufferID );
	nglBufferData( buf->glTarget, nSize, NULL, buf->glUsage );
	nglBufferSubData( buf->glTarget, 0, nSize, pData );
	nglBindBuffer( buf->glTarget, 0 );
#endif
}

void R_SetBufferData( gpuBuffer_t *buf, uint32_t nSize, uint32_t nOffset, const void *pData )
{
	nglBindBuffer( buf->glTarget, buf->nBufferID );	
	if ( buf->nBufferSize < nSize ) {
		nglBufferData( buf->glTarget, nSize, pData, buf->glUsage );
		buf->nBufferSize = nSize;
	} else {
		nglBufferSubData( buf->glTarget, nOffset, nSize, pData );
	}
	nglBindBuffer( buf->glTarget, 0 );
}