/*
===========================================================================
Copyright (C) 2023-2024 GDR Games

This file is part of The Nomad source code.

The Nomad source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

The Nomad source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Foobar; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/


#include "rgl_local.h"

//shaderCommands_t tess;

#define HAVE_MAP_BUFFER_RANGE ( NGL_VERSION_ATLEAST( 3, 0 ) || glContext.ARB_map_buffer_range )
#define HAVE_BUFFER_STORAGE ( NGL_VERSION_ATLEAST( 4, 4 ) || glContext.ARB_buffer_storage )
#define HAVE_DIRECT_STATE_ACCESS ( glContext.directStateAccess )

static void R_InitRingbuffer( GLenum target, uint32_t elementSize, uint32_t segmentElements, glRingbuffer_t *rb );
static uint32_t R_RotateRingbuffer( glRingbuffer_t *rb );
static void R_ShutdownRingbuffer( GLenum target, glRingbuffer_t *rb );

void R_VaoPackTangent( int16_t *out, vec4_t v )
{
	out[0] = v[0] * 32767.0f + ( v[0] > 0.0f ? 0.5f : -0.5f );
	out[1] = v[1] * 32767.0f + ( v[1] > 0.0f ? 0.5f : -0.5f );
	out[2] = v[2] * 32767.0f + ( v[2] > 0.0f ? 0.5f : -0.5f );
	out[3] = v[3] * 32767.0f + ( v[3] > 0.0f ? 0.5f : -0.5f );
}

void R_VaoPackNormal( int16_t *out, vec3_t v )
{
	out[0] = v[0] * 32767.0f + ( v[0] > 0.0f ? 0.5f : -0.5f );
	out[1] = v[1] * 32767.0f + ( v[1] > 0.0f ? 0.5f : -0.5f );
	out[2] = v[2] * 32767.0f + ( v[2] > 0.0f ? 0.5f : -0.5f );
	out[3] = 0;
}

void R_VaoPackColor( uint16_t *out, const vec4_t c )
{
	out[0] = c[0] * 65535.0f + 0.5f;
	out[1] = c[1] * 65535.0f + 0.5f;
	out[2] = c[2] * 65535.0f + 0.5f;
	out[3] = c[3] * 65535.0f + 0.5f;
}

void R_VaoUnpackTangent( vec4_t v, int16_t *pack )
{
	v[0] = pack[0] / 32767.0f;
	v[1] = pack[1] / 32767.0f;
	v[2] = pack[2] / 32767.0f;
	v[3] = pack[3] / 32767.0f;
}

void R_VaoUnpackNormal( vec3_t v, int16_t *pack )
{
	v[0] = pack[0] / 32767.0f;
	v[1] = pack[1] / 32767.0f;
	v[2] = pack[2] / 32767.0f;
}

void R_VaoUnpackColor( vec4_t v, uint16_t *pack )
{
	v[0] = pack[0] / 65535.0f /*- 0.5f */;
	v[1] = pack[1] / 65535.0f /*- 0.5f */;
	v[2] = pack[2] / 65535.0f /*- 0.5f */;
	v[3] = pack[3] / 65535.0f /*- 0.5f */;
}

static void R_SetVertexPointers( const vertexAttrib_t attribs[ATTRIB_INDEX_COUNT] )
{
	uint32_t attribBit, i;
	const vertexAttrib_t *vAtb;
	
	if ( HAVE_DIRECT_STATE_ACCESS ) {
		return;
	}

	ri.GLimp_LogComment( "Setting vertex array attribute pointers...\n" );

	for ( i = 0; i < ATTRIB_INDEX_COUNT; i++ ) {
		attribBit = 1 << i;
		vAtb = &attribs[i];

		if ( vAtb->enabled ) {
			if ( !( glState.vertexAttribsEnabled & attribBit ) ) {
				if ( HAVE_DIRECT_STATE_ACCESS ) {
					nglEnableVertexArrayAttrib( glState.currentVao->vaoId, vAtb->index );
				} else {
					nglEnableVertexAttribArray( vAtb->index );
				}
			}
			if ( ( vAtb->type == GL_UNSIGNED_INT || vAtb->type == GL_UNSIGNED_SHORT ) && !vAtb->normalized ) {
				if ( HAVE_DIRECT_STATE_ACCESS ) {
					nglVertexArrayAttribIFormat( glState.currentVao->vaoId, vAtb->index, vAtb->count, vAtb->type, vAtb->offset );
				} else {
					nglVertexAttribIPointer( vAtb->index, vAtb->count, vAtb->type, vAtb->stride, (const void *)vAtb->offset );
				}
			} else {
				if ( HAVE_DIRECT_STATE_ACCESS ) {
					nglVertexArrayAttribFormat( glState.currentVao->vaoId, vAtb->index, vAtb->count, vAtb->type, vAtb->normalized, vAtb->offset );
				} else {
					nglVertexAttribPointer( vAtb->index, vAtb->count, vAtb->type, vAtb->normalized, vAtb->stride, (const void *)vAtb->offset );
				}
			}
			if ( HAVE_DIRECT_STATE_ACCESS ) {
				nglVertexArrayAttribBinding( glState.currentVao->vaoId, vAtb->index, vAtb->binding );
			}
			glState.vertexAttribsEnabled |= attribBit;
		}
		else {
			if ( ( glState.vertexAttribsEnabled & attribBit ) ) {
				if ( HAVE_DIRECT_STATE_ACCESS ) {
					nglDisableVertexArrayAttrib( glState.currentVao->vaoId, vAtb->index );
				} else {
					nglDisableVertexAttribArray( vAtb->index );
				}
			}
			glState.vertexAttribsEnabled &= ~attribBit;
		}
    }
}

void VBO_SetVertexPointers( vertexBuffer_t *vbo, uint32_t attribBits )
{
	// if nothing is set, set everything
	if ( !( attribBits & ATTRIB_BITS ) ) {
		attribBits = ATTRIB_BITS;
	}

	vbo->attribs[ ATTRIB_INDEX_POSITION ].enabled = attribBits & ATTRIB_POSITION;
	vbo->attribs[ ATTRIB_INDEX_TEXCOORD ].enabled = attribBits & ATTRIB_TEXCOORD;
	vbo->attribs[ ATTRIB_INDEX_COLOR ].enabled = attribBits & ATTRIB_COLOR;
	vbo->attribs[ ATTRIB_INDEX_WORLDPOS ].enabled = attribBits & ATTRIB_WORLDPOS;

	R_SetVertexPointers( vbo->attribs );
}

void VBO_SetVertexAttribPointers( vertexBuffer_t *vbo ) {
	VBO_Bind( vbo );
	R_SetVertexPointers( vbo->attribs );
}

/*
* R_ClearVertexPointers: clears all vertex pointers in the current GL state
*/
static void R_ClearVertexPointers( void )
{
	uint32_t attribBit, i;

	if ( HAVE_DIRECT_STATE_ACCESS ) {
		return;
	}
	
	ri.GLimp_LogComment( "Clearing vertex array attribute pointers...\n" );

    for ( i = 0; i < ATTRIB_INDEX_COUNT; i++ ) {
		attribBit = 1 << i;
		if ( ( glState.vertexAttribsEnabled & attribBit ) ) {
			if ( HAVE_DIRECT_STATE_ACCESS ) {
				nglDisableVertexArrayAttrib( glState.currentVao->vaoId, i );
			} else {
				nglDisableVertexAttribArray( i );
			}
		}
    }
	glState.vertexAttribsEnabled = 0;
}

/*
============
R_BufferList_f
============
*/
void R_BufferList_f( void )
{
	int             i, j;
	vertexBuffer_t  *vao;
	uint32_t		size;
	uint64_t        vertexesSize = 0;
	uint64_t        indexesSize = 0;

	ri.Printf( PRINT_INFO, " size          name\n" );
	ri.Printf( PRINT_INFO, "----------------------------------------------------------\n" );

	ri.Printf( PRINT_INFO, "[VERTEX MEMORY]\n" );
	for ( i = 0; i < rg.numBuffers; i++ ) {
		vao = rg.buffers[i];

		if ( HAVE_DIRECT_STATE_ACCESS ) {
			size = 0;
			for ( j = 0; j < vao->numBuffers; j++ ) {
				if ( vao->attribs[j].enabled ) {
					size += vao->vertex[j].size;
				}
			}
		} else {
			size = vao->vertex->size;
		}
		ri.Printf( PRINT_INFO, "%0.02lf MB %s\n", (double)( size / ( 1024 * 1024 ) ), vao->name );

		vertexesSize += size;
	}

	ri.Printf( PRINT_INFO, "\n[INDEX MEMORY]\n" );
	for ( i = 0; i < rg.numBuffers; i++ ) {
		vao = rg.buffers[i];

		ri.Printf( PRINT_INFO, "%0.02lf MB %s\n", (double)( vao->index.size / ( 1024 * 1024 ) ), vao->name );

		indexesSize += vao->index.size;
	}

	ri.Printf( PRINT_INFO, " %u total VAOs\n", rg.numBuffers );
	ri.Printf( PRINT_INFO, " %0.02lf MB total vertices memory\n", (double)( vertexesSize / ( 1024 * 1024 ) ) );
	ri.Printf( PRINT_INFO, " %0.02lf MB total triangle indices memory\n", (double)( indexesSize / ( 1024 * 1024 ) ) );
}

void R_InitGPUBuffers( void )
{
	uint64_t vertexesSize, indexesSize;
	uintptr_t offset;
	vertexAttrib_t attribs[ ATTRIB_INDEX_COUNT ];

	ri.Printf( PRINT_INFO, "---------- R_InitGPUBuffers ----------\n" );

	rg.numBuffers = 0;

//	if ( NGL_VERSION_ATLEAST( 4, 3 ) ) {
		// NOTE: NEVER CHANGE THIS
		srfVert_t quadVertices[] = {
			{ { 0, 0 }, { -1.0f, -1.0f, 0.0f }, { 0.0f, 0.0f }, { 1, 1, 1, 1 } },
			{ { 0, 0 }, {  1.0f, -1.0f, 0.0f }, { 1.0f, 0.0f }, { 1, 1, 1, 1 } },
			{ { 0, 0 }, {  1.0f,  1.0f, 0.0f }, { 1.0f, 1.0f }, { 1, 1, 1, 1 } },
			{ { 0, 0 }, { -1.0f,  1.0f, 0.0f }, { 0.0f, 1.0f }, { 1, 1, 1, 1 } },
		};

		rg.buffers[ rg.numBuffers ] = rg.renderPassVBO = ri.Hunk_Alloc( sizeof( *rg.renderPassVBO ), h_low );
		rg.numBuffers++;

		rg.renderPassVBO->attribs[ ATTRIB_INDEX_POSITION ].index		= ATTRIB_INDEX_POSITION;
		rg.renderPassVBO->attribs[ ATTRIB_INDEX_TEXCOORD ].index		= ATTRIB_INDEX_TEXCOORD;

		rg.renderPassVBO->attribs[ ATTRIB_INDEX_POSITION ].type			= GL_FLOAT;
		rg.renderPassVBO->attribs[ ATTRIB_INDEX_TEXCOORD ].type			= GL_FLOAT;

		rg.renderPassVBO->attribs[ ATTRIB_INDEX_POSITION ].count		= 3;
		rg.renderPassVBO->attribs[ ATTRIB_INDEX_TEXCOORD ].count		= 2;

		rg.renderPassVBO->attribs[ ATTRIB_INDEX_POSITION ].normalized	= GL_FALSE;
		rg.renderPassVBO->attribs[ ATTRIB_INDEX_TEXCOORD ].normalized	= GL_FALSE;

		rg.renderPassVBO->attribs[ ATTRIB_INDEX_POSITION ].offset		= offsetof( srfVert_t, xyz );
		rg.renderPassVBO->attribs[ ATTRIB_INDEX_TEXCOORD ].offset		= offsetof( srfVert_t, st );

		rg.renderPassVBO->attribs[ ATTRIB_INDEX_POSITION ].stride		= sizeof( srfVert_t );
		rg.renderPassVBO->attribs[ ATTRIB_INDEX_TEXCOORD ].stride		= sizeof( srfVert_t );

		rg.renderPassVBO->attribs[ ATTRIB_INDEX_POSITION ].enabled		= qtrue;
		rg.renderPassVBO->attribs[ ATTRIB_INDEX_TEXCOORD ].enabled		= qtrue;

		rg.renderPassVBO->attribs[ ATTRIB_INDEX_POSITION ].binding		= 0;
		rg.renderPassVBO->attribs[ ATTRIB_INDEX_TEXCOORD ].binding		= 0;

		rg.renderPassVBO->vertex = (buffer_t *)ri.Hunk_Alloc( sizeof( *rg.renderPassVBO->vertex ), h_low );

		if ( HAVE_DIRECT_STATE_ACCESS ) {
			nglCreateVertexArrays( 1, &rg.renderPassVBO->vaoId );
			nglCreateBuffers( 1, &rg.renderPassVBO->vertex->id );
		} else {
			nglGenVertexArrays( 1, &rg.renderPassVBO->vaoId );
			nglGenBuffers( 1, &rg.renderPassVBO->vertex->id );
		}
		VBO_Bind( rg.renderPassVBO );
		rg.renderPassVBO->vertex->size = sizeof( quadVertices );
		rg.renderPassVBO->vertex->glUsage = GL_STATIC_DRAW;
		rg.renderPassVBO->vertex->target = GL_ARRAY_BUFFER;

		if ( HAVE_DIRECT_STATE_ACCESS ) {
			if ( HAVE_BUFFER_STORAGE ) {
				nglNamedBufferStorage( rg.renderPassVBO->vertex->id, rg.renderPassVBO->vertex->size, quadVertices, 0 );
			} else {
				nglNamedBufferData( rg.renderPassVBO->vertex->id, rg.renderPassVBO->vertex->size, quadVertices, GL_STATIC_DRAW );
			}

			nglEnableVertexArrayAttrib( rg.renderPassVBO->vaoId, ATTRIB_INDEX_POSITION );
			nglVertexArrayAttribFormat( rg.renderPassVBO->vaoId, ATTRIB_INDEX_POSITION, rg.renderPassVBO->attribs[ ATTRIB_INDEX_POSITION ].count,
				rg.renderPassVBO->attribs[ ATTRIB_INDEX_POSITION ].type, rg.renderPassVBO->attribs[ ATTRIB_INDEX_POSITION ].normalized,
				rg.renderPassVBO->attribs[ ATTRIB_INDEX_POSITION ].offset );
			nglVertexArrayAttribBinding( rg.renderPassVBO->vaoId, ATTRIB_INDEX_POSITION, 0 );

			nglEnableVertexArrayAttrib( rg.renderPassVBO->vaoId, ATTRIB_INDEX_TEXCOORD );
			nglVertexArrayAttribFormat( rg.renderPassVBO->vaoId, ATTRIB_INDEX_TEXCOORD, rg.renderPassVBO->attribs[ ATTRIB_INDEX_TEXCOORD ].count,
				rg.renderPassVBO->attribs[ ATTRIB_INDEX_TEXCOORD ].type, rg.renderPassVBO->attribs[ ATTRIB_INDEX_TEXCOORD ].normalized,
				rg.renderPassVBO->attribs[ ATTRIB_INDEX_TEXCOORD ].offset );
			nglVertexArrayAttribBinding( rg.renderPassVBO->vaoId, ATTRIB_INDEX_TEXCOORD, 0 );

			nglVertexArrayVertexBuffer( rg.renderPassVBO->vaoId, 0, rg.renderPassVBO->vertex->id, 0, sizeof( srfVert_t ) );
		} else {
			nglBufferData( GL_ARRAY_BUFFER, sizeof( quadVertices ), quadVertices, GL_STATIC_DRAW );
		}
		VBO_SetVertexAttribPointers( rg.renderPassVBO );

		VBO_BindNull();
//	}

	memset( attribs, 0, sizeof( attribs ) );
	
	attribs[ATTRIB_INDEX_POSITION].enabled		= qtrue;
	attribs[ATTRIB_INDEX_TEXCOORD].enabled		= qtrue;
	attribs[ATTRIB_INDEX_COLOR].enabled			= qtrue;
	attribs[ATTRIB_INDEX_WORLDPOS].enabled		= qtrue;

	attribs[ATTRIB_INDEX_POSITION].count		= 3;
	attribs[ATTRIB_INDEX_TEXCOORD].count		= 2;
	attribs[ATTRIB_INDEX_COLOR].count			= 4;
	attribs[ATTRIB_INDEX_WORLDPOS].count		= 2;

	attribs[ATTRIB_INDEX_POSITION].type			= GL_FLOAT;
	attribs[ATTRIB_INDEX_TEXCOORD].type			= GL_FLOAT;
	attribs[ATTRIB_INDEX_COLOR].type			= GL_UNSIGNED_BYTE;
	attribs[ATTRIB_INDEX_WORLDPOS].type			= GL_UNSIGNED_SHORT;

	attribs[ATTRIB_INDEX_POSITION].index		= ATTRIB_INDEX_POSITION;
	attribs[ATTRIB_INDEX_TEXCOORD].index		= ATTRIB_INDEX_TEXCOORD;
	attribs[ATTRIB_INDEX_COLOR].index			= ATTRIB_INDEX_COLOR;
	attribs[ATTRIB_INDEX_WORLDPOS].index		= ATTRIB_INDEX_WORLDPOS;

	attribs[ATTRIB_INDEX_POSITION].normalized	= GL_FALSE;
	attribs[ATTRIB_INDEX_TEXCOORD].normalized	= GL_FALSE;
	attribs[ATTRIB_INDEX_COLOR].normalized		= GL_TRUE;
	attribs[ATTRIB_INDEX_WORLDPOS].normalized	= GL_FALSE;

	attribs[ATTRIB_INDEX_POSITION].offset		= offsetof( srfVert_t, xyz );
	attribs[ATTRIB_INDEX_TEXCOORD].offset		= offsetof( srfVert_t, st );
	attribs[ATTRIB_INDEX_COLOR].offset			= offsetof( srfVert_t, color );
	attribs[ATTRIB_INDEX_WORLDPOS].offset		= offsetof( srfVert_t, worldPos );

	attribs[ATTRIB_INDEX_POSITION].stride		= sizeof( srfVert_t );
	attribs[ATTRIB_INDEX_TEXCOORD].stride		= sizeof( srfVert_t );
	attribs[ATTRIB_INDEX_COLOR].stride			= sizeof( srfVert_t );
	attribs[ATTRIB_INDEX_WORLDPOS].stride		= sizeof( srfVert_t );

	backend.drawBuffer = R_AllocateBuffer( "batchBuffer0", NULL, 4*1024*1024, NULL,
		4*1024*1024, BUFFER_STREAM, attribs );

	VBO_BindNull();
	
	GL_CheckErrors();

	ri.Cmd_AddCommand( "vaolist", R_BufferList_f );
}

void R_ShutdownGPUBuffers( void )
{
	uint64_t i;
	vertexBuffer_t *vbo;

	ri.Printf( PRINT_INFO, "---------- R_ShutdownGPUBuffers -----------\n" );

	VBO_BindNull();

	if ( rg.world && rg.world->buffer ) {
		R_ShutdownBuffer( rg.world->buffer );
	}
	if ( backend.drawBuffer ) {
		R_ShutdownBuffer( backend.drawBuffer );
	}

	memset( rg.buffers, 0, sizeof( rg.buffers ) );
	rg.numBuffers = 0;

	ri.Cmd_RemoveCommand( "vaolist" );
}

#if defined( GL_ARB_buffer_storage ) && defined( GL_ARB_sync )
/*
============
R_InitRingbuffer
============
*/
static void R_InitRingbuffer( GLenum target, uint32_t elementSize, uint32_t segmentElements, glRingbuffer_t *rb ) {
	const uint64_t totalSize = elementSize * segmentElements * DYN_BUFFER_SEGMENTS;
	uint32_t i;

	nglBufferStorage( target, totalSize, NULL, GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT );

	rb->baseAddr = nglMapBufferRange( target, 0, totalSize,
					 GL_MAP_WRITE_BIT |
					 GL_MAP_PERSISTENT_BIT |
					 GL_MAP_FLUSH_EXPLICIT_BIT );
	rb->elementSize = elementSize;
	rb->segmentElements = segmentElements;
	rb->activeSegment = 0;

	for ( i = 1; i < DYN_BUFFER_SEGMENTS; i++ ) {
		rb->syncs[ i ] = nglFenceSync( GL_SYNC_GPU_COMMANDS_COMPLETE, 0 );
	}
}

/*
============
R_RotateRingbuffer
============
*/
static uint32_t R_RotateRingbuffer( glRingbuffer_t *rb ) {
	rb->syncs[ rb->activeSegment ] = nglFenceSync( GL_SYNC_GPU_COMMANDS_COMPLETE, 0 );

	rb->activeSegment++;
	if ( rb->activeSegment >= DYN_BUFFER_SEGMENTS ) {
		rb->activeSegment = 0;
	}

	// wait until next segment is ready in 1 sec intervals
	while ( nglClientWaitSync( rb->syncs[ rb->activeSegment ], GL_SYNC_FLUSH_COMMANDS_BIT, 10000000 ) == GL_TIMEOUT_EXPIRED ) {
		ri.Printf( PRINT_WARNING, "long wait for GL buffer" );
	}

	nglDeleteSync( rb->syncs[ rb->activeSegment ] );

	return rb->activeSegment * rb->segmentElements;
}

/*
============
R_ShutdownRingbuffer
============
*/
static void R_ShutdownRingbuffer( GLenum target, glRingbuffer_t *rb ) {
	uint32_t i;

	nglUnmapBuffer( target );
	rb->baseAddr = NULL;

	for ( i = 0; i < DYN_BUFFER_SEGMENTS; i++ ) {
		if ( i == rb->activeSegment ) {
			continue;
		}

		nglDeleteSync( rb->syncs[ i ] );
	}
}
#endif

vertexBuffer_t *R_AllocateBuffer( const char *name, void *vertices, uint32_t verticesSize, void *indices, uint32_t indicesSize,
	bufferType_t type, vertexAttrib_t szAttribs[ ATTRIB_INDEX_COUNT ] )
{
    vertexBuffer_t *buf;
	GLenum vertexUsage, indexUsage;
	uint32_t namelen;
	uint32_t i, j;
	GLuint usedAttribs;
	GLenum err;
	qboolean interleaved;

	switch ( type ) {
	case BUFFER_STATIC:
		vertexUsage = GL_STATIC_DRAW;
		indexUsage = GL_STATIC_DRAW;
		break;
	case BUFFER_FRAME:
	case BUFFER_DYNAMIC:
		vertexUsage = GL_DYNAMIC_DRAW;
		indexUsage = GL_DYNAMIC_DRAW;
		break;
	case BUFFER_STREAM:
		vertexUsage = GL_STREAM_DRAW;
		indexUsage = GL_STREAM_DRAW;
		break;
	default:
		ri.Error( ERR_FATAL, "Bad glUsage %i", type );
	};

	if ( vertices != NULL ) {
		vertexUsage = GL_STATIC_DRAW;
	}
	if ( indices != NULL ) {
		indexUsage = GL_STATIC_DRAW;
	}

	if ( rg.numBuffers == MAX_RENDER_BUFFERS ) {
		ri.Error( ERR_DROP, "R_AllocateBuffer: MAX_RENDER_BUFFERS hit" );
	}

    buf = rg.buffers[ rg.numBuffers ] = ri.Hunk_Alloc( sizeof( *buf ), h_low );
	memset( buf, 0, sizeof( *buf ) );
    rg.numBuffers++;

    buf->type = type;
	N_strncpyz( buf->name, name, sizeof( buf->name ) );
	
	if ( HAVE_DIRECT_STATE_ACCESS ) {
		nglCreateVertexArrays( 1, &buf->vaoId );
		if ( ( err = nglGetError() ) != GL_NO_ERROR ) {
			ri.Error( ERR_DROP, "%s: Error generating OpenGL Vertex Array (0x%04x)", GL_ErrorString( err ), err );
		}
	} else {
		nglGenVertexArrays( 1, &buf->vaoId );
		if ( ( err = nglGetError() ) != GL_NO_ERROR ) {
			ri.Error( ERR_DROP, "%s: Error generating OpenGL Vertex Array (0x%04x)", GL_ErrorString( err ), err );
		}
		nglBindVertexArray( buf->vaoId );
	}

	buf->index.usage = BUF_GL_BUFFER;
	buf->index.size = indicesSize;
	buf->index.glUsage = indexUsage;
	buf->index.target = GL_ELEMENT_ARRAY_BUFFER;

#ifdef _NOMAD_EXPERIMENTAL
	if ( r_drawMode->i == DRAWMODE_MAPPED ) {
		nglGenBuffers( 1, &buf->index.id );
		nglGenBuffers( 1, &buf->vertex.id );

		nglBindBuffer( GL_ARRAY_BUFFER, buf->vertex.id );
		nglBindBuffer( GL_ELEMENT_ARRAY_BUFFER, buf->index.id );

		R_InitRingbuffer( GL_ELEMENT_ARRAY_BUFFER, sizeof( glIndex_t ), buf->index.size / sizeof( glIndex_t ), &buf->index.ringbuffer );
		R_InitRingbuffer( GL_ARRAY_BUFFER, sizeof( drawVert_t ), buf->vertex.size / sizeof( drawVert_t ), &buf->vertex.ringbuffer );
	} else
#endif
	{
		interleaved = qtrue;
		usedAttribs = 0;
		if ( N_stristr( "worldDrawBuffer", name ) ) {
			interleaved = qfalse;
		}
		
		if ( HAVE_DIRECT_STATE_ACCESS && !interleaved ) {
			buf->vertex = (buffer_t *)ri.Hunk_Alloc( sizeof( *buf->vertex ) * ATTRIB_INDEX_COUNT, h_low );
			
			for ( i = 0; i < ATTRIB_INDEX_COUNT; i++ ) {
				if ( szAttribs[i].enabled ) {
					nglCreateBuffers( 1, &buf->vertex[i].id );
					buf->vertex[i].size = verticesSize - szAttribs[i].offset;
					buf->vertex[i].glUsage = vertexUsage;
					buf->vertex[i].usage = BUF_GL_BUFFER;
					buf->vertex[i].offset = szAttribs[i].offset;
					buf->vertex[i].target = GL_ARRAY_BUFFER;

					nglNamedBufferStorage( buf->vertex[i].id, verticesSize, vertices, GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT );
					GL_CheckErrors();

					nglVertexArrayVertexBuffer( buf->vaoId, i, buf->vertex[i].id, szAttribs[i].offset, szAttribs[i].stride );
					
					nglEnableVertexArrayAttrib( buf->vaoId, szAttribs[i].index );
					if ( ( szAttribs[i].type == GL_UNSIGNED_INT || szAttribs[i].type == GL_UNSIGNED_SHORT ) && !szAttribs[i].normalized ) {	
						nglVertexArrayAttribIFormat( buf->vaoId, szAttribs[i].index, szAttribs[i].count, szAttribs[i].type,
							szAttribs[i].offset );
					} else {
						nglVertexArrayAttribFormat( buf->vaoId, szAttribs[i].index, szAttribs[i].count, szAttribs[i].type,
							szAttribs[i].normalized, szAttribs[i].offset );
					}
					nglVertexArrayAttribBinding( buf->vaoId, szAttribs[i].index, i );

					GL_CheckErrors();
				}
			}
			
			nglCreateBuffers( 1, &buf->index.id );
			nglNamedBufferStorage( buf->index.id, indicesSize, indices, GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT );
			
			nglVertexArrayElementBuffer( buf->vaoId, buf->index.id );
		} else {
			buf->vertex = (buffer_t *)ri.Hunk_Alloc( sizeof( *buf->vertex ), h_low );
			
			if ( HAVE_DIRECT_STATE_ACCESS ) {
				nglCreateBuffers( 1, &buf->vertex->id );
				nglCreateBuffers( 1, &buf->index.id );
				
				if ( HAVE_BUFFER_STORAGE && HAVE_MAP_BUFFER_RANGE ) {
					nglNamedBufferStorage( buf->vertex->id, verticesSize, vertices, vertexUsage == GL_STATIC_DRAW ? 0 :
						GL_DYNAMIC_STORAGE_BIT | GL_MAP_WRITE_BIT );
					nglNamedBufferStorage( buf->index.id, indicesSize, indices, indexUsage == GL_STATIC_DRAW ? 0 :
						GL_DYNAMIC_STORAGE_BIT | GL_MAP_WRITE_BIT );
				}
				else {
					nglNamedBufferData( buf->vertex->id, verticesSize, vertices, vertexUsage );
					nglNamedBufferData( buf->index.id, indicesSize, indices, indexUsage );
				}

				nglVertexArrayVertexBuffer( buf->vaoId, 0, buf->vertex->id, 0, sizeof( srfVert_t ) );
				nglVertexArrayElementBuffer( buf->vaoId, buf->index.id );

				for ( i = 0; i < ATTRIB_INDEX_COUNT; i++ ) {
					if ( szAttribs[i].enabled ) {
						nglEnableVertexArrayAttrib( buf->vaoId, szAttribs[i].index );
						if ( ( szAttribs[i].type == GL_UNSIGNED_INT || szAttribs[i].type == GL_UNSIGNED_SHORT ) && !szAttribs[i].normalized ) {	
							nglVertexArrayAttribIFormat( buf->vaoId, szAttribs[i].index, szAttribs[i].count, szAttribs[i].type,
								szAttribs[i].offset );
						} else {
							nglVertexArrayAttribFormat( buf->vaoId, szAttribs[i].index, szAttribs[i].count, szAttribs[i].type,
								szAttribs[i].normalized, szAttribs[i].offset );
						}
						nglVertexArrayAttribBinding( buf->vaoId, szAttribs[i].index, 0 );
					}
				}
				GL_CheckErrors();
			} else {
				buf->vertex->target = GL_ARRAY_BUFFER;

				nglGenBuffers( 1, &buf->vertex->id );
				nglGenBuffers( 1, &buf->index.id );
				
				nglBindBuffer( GL_ARRAY_BUFFER, buf->vertex->id );
				nglBindBuffer( GL_ELEMENT_ARRAY_BUFFER, buf->index.id );
				
				if ( HAVE_BUFFER_STORAGE && HAVE_MAP_BUFFER_RANGE ) {
					nglBufferStorage( GL_ARRAY_BUFFER, verticesSize, vertices, GL_DYNAMIC_STORAGE_BIT | GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT );
					nglBufferStorage( GL_ELEMENT_ARRAY_BUFFER, indicesSize, indices, GL_DYNAMIC_STORAGE_BIT | GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT );
				} else {
					nglBufferData( GL_ARRAY_BUFFER, verticesSize, vertices, vertexUsage );
					nglBufferData( GL_ELEMENT_ARRAY_BUFFER, indicesSize, indices, indexUsage );
				}

				VBO_SetVertexAttribPointers( buf );
				
				nglBindBuffer( GL_ARRAY_BUFFER, 0 );
				nglBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );
			}
		}
	}
	
	GL_SetObjectDebugName( GL_BUFFER, buf->index.id, name, "_ibo" );
	GL_SetObjectDebugName( GL_VERTEX_ARRAY, buf->vaoId, name, "_vao" );
	if ( HAVE_DIRECT_STATE_ACCESS && !interleaved ) {
		usedAttribs = 0;
		for ( i = 0; i < ATTRIB_INDEX_COUNT; i++ ) {
			if ( szAttribs[i].enabled ) {
				GL_SetObjectDebugName( GL_BUFFER, buf->vertex[i].id, name, va( "_vbo%u", usedAttribs ) );
				usedAttribs++;
			}
		}
	} else {
		GL_SetObjectDebugName( GL_BUFFER, buf->vertex->id, name, "_vbo" );
	}

	glState.memstats.estBufferMemUsed += ( indicesSize + verticesSize );
	glState.memstats.estVertexMemUsed += verticesSize;
	glState.memstats.estIndexMemUsed += indicesSize;
	glState.memstats.numIndexBufers++;
	glState.memstats.numVertexBuffers++;
	glState.memstats.numVertexArrays++;
	glState.memstats.numBuffers++;

	memcpy( buf->attribs, szAttribs, sizeof( buf->attribs ) );

    return buf;
}

/*
============
VBO_Bind
============
*/
void VBO_Bind( vertexBuffer_t *vbo )
{
	if ( !vbo ) {
		ri.Error( ERR_DROP, "VBO_Bind: NULL buffer" );
		return;
	}

	if ( glState.currentVao != vbo ) {
		glState.currentVao = vbo;
		glState.vaoId = vbo->vaoId;
		glState.vboId = vbo->vertex->id;
		glState.iboId = vbo->index.id;
		backend.pc.c_bufferBinds++;

		nglBindVertexArray( vbo->vaoId );

		if ( HAVE_DIRECT_STATE_ACCESS ) {
			return;
		}

		nglBindBuffer( GL_ARRAY_BUFFER, vbo->vertex->id );
		if ( vbo->index.id != 0 ) {
			nglBindBuffer( GL_ELEMENT_ARRAY_BUFFER, vbo->index.id );
		}

		// Intel Graphics doesn't save GL_ELEMENT_ARRAY_BUFFER binding with VAO binding.
		// [TheNomad] 6/10/24 you've gotta bind it, nothing saves the binding
//		if ( glContext.intelGraphics ) {
//			nglBindBuffer( GL_ELEMENT_ARRAY_BUFFER, vbo->index.id );
//		}
	}
}

/*
============
VBO_BindNull
============
*/
void VBO_BindNull( void )
{
	if ( glState.currentVao ) {
		glState.currentVao = NULL;
		glState.vaoId = glState.vboId = glState.iboId = 0;

		nglBindVertexArray( 0 );

		if ( HAVE_DIRECT_STATE_ACCESS ) {
			return;
		}

		nglBindBuffer( GL_ARRAY_BUFFER, 0 );
		nglBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );

	    // why you no save GL_ELEMENT_ARRAY_BUFFER binding, Intel?
//	       if ( glContext.intelGraphics ) {
//			nglBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );
//		}
	}

	GL_CheckErrors();
}

void R_ShutdownBuffer( vertexBuffer_t *vbo )
{
	uint32_t i;

	VBO_Bind( vbo );

	ri.GLimp_LogComment( "R_ShutdownBuffer()\n" );

	if ( HAVE_DIRECT_STATE_ACCESS ) {
		for ( i = 0; i < ATTRIB_INDEX_COUNT; i++ ) {
			if ( vbo->attribs[i].enabled ) {
				nglDeleteBuffers( 1, &vbo->vertex[i].id );
				GL_CheckErrors();
			}
		}
	} else {
		if ( vbo->vertex->id ) {
			nglDeleteBuffers( 1, &vbo->vertex->id );
			GL_CheckErrors();
		}
	}

	if ( vbo->index.id ) {
		nglDeleteBuffers( 1, &vbo->index.id );
		GL_CheckErrors();
	}

	if ( vbo->vaoId ) {
		nglDeleteVertexArrays( 1, &vbo->vaoId );
		GL_CheckErrors();
	}

	rg.numBuffers--;

	glState.memstats.numBuffers--;
	glState.memstats.estBufferMemUsed -= ( vbo->vertex->size + vbo->index.size );
	memset( vbo, 0, sizeof( *vbo ) );

	nglBindBuffer( GL_ARRAY_BUFFER, 0 );
	nglBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );
	nglBindVertexArray( 0 );
	glState.currentVao = NULL;
}

void VBO_MapBuffers( buffer_t *buf )
{
	uint32_t i;

	if ( !glContext.ARB_buffer_storage ) {
		buf->data = ri.Hunk_Alloc( buf->size, h_low );

		return;
	}

	ri.GLimp_LogComment( "Mapping vertex and index buffer into CPU DMA...\n" );

	if ( HAVE_DIRECT_STATE_ACCESS ) {
		buf->data = nglMapNamedBufferRange( buf->id, 0, buf->size, GL_MAP_WRITE_BIT | GL_MAP_UNSYNCHRONIZED_BIT
			| GL_MAP_FLUSH_EXPLICIT_BIT | GL_MAP_PERSISTENT_BIT );
	} else {
		nglBindBuffer( buf->target, buf->id );
		buf->data = nglMapBufferRange( buf->target, 0, buf->size, GL_MAP_WRITE_BIT | GL_MAP_UNSYNCHRONIZED_BIT
			| GL_MAP_FLUSH_EXPLICIT_BIT | GL_MAP_PERSISTENT_BIT );
		nglBindBuffer( buf->target, 0 );
	}
	
	GL_CheckErrors();
}

void RB_SetBatchBuffer( vertexBuffer_t *buffer, void *vertexBuffer, uintptr_t vtxSize, void *indexBuffer, uintptr_t idxSize )
{
	uint32_t attribBits, i;

    // clear anything currently queued
	if ( backend.drawBatch.buffer && ( backend.drawBatch.vtxOffset || backend.drawBatch.idxOffset ) ) {
		RB_FlushBatchBuffer();
	}

	// is it already bound?
    if ( backend.drawBatch.buffer != buffer ) {
		VBO_BindNull();
	} else if ( backend.drawBatch.buffer == buffer ) {
		VBO_Bind( buffer );
		return;
	}

    backend.drawBatch.buffer = buffer;

    backend.drawBatch.vtxDataSize = vtxSize;
    backend.drawBatch.idxDataSize = idxSize;
	
	backend.drawBatch.maxVertices = 4*1024*1024;
	backend.drawBatch.maxIndices = 4*1024*1024;

    backend.drawBatch.vertices = vertexBuffer;
    backend.drawBatch.indices = indexBuffer;

    // bind the new cache
	VBO_Bind( buffer );

	attribBits = 0;
	if ( buffer->attribs[ ATTRIB_INDEX_POSITION ].enabled ) {
		attribBits |= ATTRIB_POSITION;
	}
	if ( buffer->attribs[ ATTRIB_INDEX_TEXCOORD ].enabled ) {
		attribBits |= ATTRIB_TEXCOORD;
	}
	if ( buffer->attribs[ ATTRIB_INDEX_COLOR ].enabled ) {
		attribBits |= ATTRIB_COLOR;
	}
	if ( buffer->attribs[ ATTRIB_INDEX_WORLDPOS ].enabled ) {
		attribBits |= ATTRIB_WORLDPOS;
	}

	if ( !( glState.vertexAttribsEnabled & attribBits ) ) {
		R_ClearVertexPointers();
		R_SetVertexPointers( buffer->attribs );
	}
}

/*
* RB_FlushBatchBuffer
* contains platform and driver specific optimizations
* from https://www.reddit.com/r/opengl/comments/1461fzc/vertex_buffer_streaming_techniques_comparision/
*/
void RB_FlushBatchBuffer( void )
{
	vertexBuffer_t *buf;
	uint32_t i;
	void *data;

    if ( !backend.drawBatch.buffer ) {
        return;
    }

    // do we actually have something there?
    if ( backend.drawBatch.vtxOffset == 0 && backend.drawBatch.idxOffset == 0 ) {
        return;
    }

    backend.pc.c_dynamicBufferDraws++;

    //
    // upload the data to the gpu
    //

	buf = backend.drawBatch.buffer;
	
	if ( rg.world && rg.world->buffer == buf ) {
		if ( HAVE_DIRECT_STATE_ACCESS ) {
			nglFlushMappedNamedBufferRange(
				rg.world->buffer->vertex[ ATTRIB_INDEX_POSITION ].id,
				rg.world->buffer->vertex[ ATTRIB_INDEX_POSITION ].offset,
				backend.drawBatch.vtxDataSize * backend.drawBatch.vtxOffset
			);
		} else {
			nglFlushMappedBufferRange(
				GL_ARRAY_BUFFER,
				sizeof( worldPos_t ) * rg.world->numVertices + ( rg.world->buffer->vertex->offset ),
				backend.drawBatch.vtxDataSize * backend.drawBatch.vtxOffset
			);
		}
		GL_CheckErrors();
	}
	else {
		void *data;

		if ( HAVE_DIRECT_STATE_ACCESS ) {
			data = nglMapNamedBufferRange( buf->index.id, 0, backend.drawBatch.idxDataSize * backend.drawBatch.idxOffset,
				GL_MAP_WRITE_BIT | GL_MAP_UNSYNCHRONIZED_BIT | GL_MAP_INVALIDATE_RANGE_BIT );
			if ( data ) {
				memcpy( data, backend.drawBatch.indices, backend.drawBatch.idxOffset * backend.drawBatch.idxDataSize );
			}
			nglUnmapNamedBuffer( buf->index.id );

			data = nglMapNamedBufferRange( buf->vertex->id, 0, backend.drawBatch.vtxDataSize * backend.drawBatch.vtxOffset,
				GL_MAP_WRITE_BIT | GL_MAP_UNSYNCHRONIZED_BIT | GL_MAP_INVALIDATE_BUFFER_BIT );
			if ( data ) {
				memcpy( data, backend.drawBatch.vertices, backend.drawBatch.vtxOffset * backend.drawBatch.vtxDataSize );
			}
			nglUnmapNamedBuffer( buf->vertex->id );
		}
		else {
			data = nglMapBufferRange( GL_ELEMENT_ARRAY_BUFFER, 0, backend.drawBatch.idxDataSize * backend.drawBatch.idxOffset,
				GL_MAP_WRITE_BIT | GL_MAP_UNSYNCHRONIZED_BIT | GL_MAP_INVALIDATE_RANGE_BIT );
			if ( data ) {
				memcpy( data, backend.drawBatch.indices, backend.drawBatch.idxOffset * backend.drawBatch.idxDataSize );
			}
			nglUnmapBuffer( GL_ELEMENT_ARRAY_BUFFER );

			data = nglMapBufferRange( GL_ARRAY_BUFFER, 0, backend.drawBatch.vtxDataSize * backend.drawBatch.vtxOffset,
				GL_MAP_WRITE_BIT | GL_MAP_UNSYNCHRONIZED_BIT | GL_MAP_INVALIDATE_BUFFER_BIT );
			if ( data ) {
				memcpy( data, backend.drawBatch.vertices, backend.drawBatch.vtxOffset * backend.drawBatch.vtxDataSize );
			}
			nglUnmapBuffer( GL_ARRAY_BUFFER );
		}
	}

	RB_IterateShaderStages( backend.drawBatch.shader );

	backend.pc.c_bufferIndices += backend.drawBatch.idxOffset;
	backend.pc.c_bufferVertices += backend.drawBatch.vtxOffset;

	backend.drawBatch.vtxOffset = 0;
	backend.drawBatch.idxOffset = 0;

	backend.drawBuffer->index.offset += backend.drawBatch.idxOffset * backend.drawBatch.idxDataSize;
}

void RB_CommitDrawData( const void *verts, uint32_t numVerts, const void *indices, uint32_t numIndices )
{
	byte *data;

    //
    // copy the data into the client side buffer
    //

    // we could be submitting either indices or vertices
    if ( verts && backend.drawBatch.buffer == backend.drawBuffer ) {
		data = (byte *)( backend.drawBuffer->vertex->data ) + ( backend.drawBatch.vtxOffset * backend.drawBatch.vtxDataSize );
        memcpy( data, verts, numVerts * backend.drawBatch.vtxDataSize );
    }
    if ( indices && backend.drawBatch.buffer == backend.drawBuffer ) {
		data = (byte *)( backend.drawBuffer->index.data ) + ( backend.drawBatch.idxOffset * backend.drawBatch.idxDataSize );
        memcpy( data, indices, numIndices * backend.drawBatch.idxDataSize );
    }

    backend.drawBatch.vtxOffset += numVerts;
    backend.drawBatch.idxOffset += numIndices;
}

/*
==============
RB_UpdateTessVao

Adapted from Tess_UpdateVBOs from xreal

Update the default VAO to replace the client side vertex arrays
==============
*/
/*
void RB_UpdateTessVao( unsigned int attribBits )
{
//	GLimp_LogComment("--- RB_UpdateTessVao ---\n");

	backend.pc.c_dynamicBufferDraws++;

	// update the default VAO
	if ( tess.numVertexes > 0 && tess.numVertexes <= SHADER_MAX_VERTEXES && tess.numIndexes > 0 && tess.numIndexes <= SHADER_MAX_INDEXES ) {
		int attribIndex;
		int attribUpload;

		VBO_Bind( tess.vao );

		// orphan old vertex buffer so we don't stall on it
		nglBufferData( GL_ARRAY_BUFFER, tess.vao->vertex.size, NULL, GL_DYNAMIC_DRAW );

		// if nothing to set, set everything
		if ( !( attribBits & ATTRIB_BITS ) ) {
			attribBits = ATTRIB_BITS;
		}

		attribUpload = attribBits;

		for ( attribIndex = 0; attribIndex < ATTRIB_INDEX_COUNT; attribIndex++ ) {
			uint32_t attribBit = 1 << attribIndex;
			vertexAttrib_t *vAtb = &tess.vao->attribs[attribIndex];

			if ( attribUpload & attribBit ) {
				// note: tess has a VBO where stride == size
				nglBufferSubData( GL_ARRAY_BUFFER, vAtb->offset, tess.numVertexes * vAtb->stride, tess.attribPointers[attribIndex] );
			}

			if ( attribBits & attribBit ) {
				if ( !glContext.ARB_vertex_array_object ) {
					nglVertexAttribPointer( attribIndex, vAtb->count, vAtb->type, vAtb->normalized, vAtb->stride, BUFFER_OFFSET( vAtb->offset ) );
				}

				if ( !( glState.vertexAttribsEnabled & attribBit ) ) {
					nglEnableVertexAttribArray( attribIndex );
					glState.vertexAttribsEnabled |= attribBit;
				}
			}
			else {
				if ( ( glState.vertexAttribsEnabled & attribBit ) ) {
					nglDisableVertexAttribArray( attribIndex );
					glState.vertexAttribsEnabled &= ~attribBit;
				}
			}
		}

		// orphan old index buffer so we don't stall on it
		nglBufferData( GL_ELEMENT_ARRAY_BUFFER, tess.vao->index.size, NULL, GL_DYNAMIC_DRAW );

		nglBufferSubData( GL_ELEMENT_ARRAY_BUFFER, 0, tess.numIndexes * sizeof( tess.indexes[0] ), tess.indexes );
	}
}
*/