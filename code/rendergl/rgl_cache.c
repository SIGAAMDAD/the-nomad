#include "rgl_local.h"

static void R_InitRingbuffer( GLenum target, uint32_t elementSize, uint32_t segmentElements, glRingbuffer_t *rb );
static uint32_t R_RotateRingbuffer( glRingbuffer_t *rb );
static void R_ShutdownRingbuffer( GLenum target, glRingbuffer_t *rb );

void R_VaoPackTangent(int16_t *out, vec4_t v)
{
	out[0] = v[0] * 32767.0f + (v[0] > 0.0f ? 0.5f : -0.5f);
	out[1] = v[1] * 32767.0f + (v[1] > 0.0f ? 0.5f : -0.5f);
	out[2] = v[2] * 32767.0f + (v[2] > 0.0f ? 0.5f : -0.5f);
	out[3] = v[3] * 32767.0f + (v[3] > 0.0f ? 0.5f : -0.5f);
}

void R_VaoPackNormal(int16_t *out, vec3_t v)
{
	out[0] = v[0] * 32767.0f + (v[0] > 0.0f ? 0.5f : -0.5f);
	out[1] = v[1] * 32767.0f + (v[1] > 0.0f ? 0.5f : -0.5f);
	out[2] = v[2] * 32767.0f + (v[2] > 0.0f ? 0.5f : -0.5f);
	out[3] = 0;
}

void R_VaoPackColor(uint16_t *out, const vec4_t c)
{
	out[0] = c[0] * 65535.0f + 0.5f;
	out[1] = c[1] * 65535.0f + 0.5f;
	out[2] = c[2] * 65535.0f + 0.5f;
	out[3] = c[3] * 65535.0f + 0.5f;
}

void R_VaoUnpackTangent(vec4_t v, int16_t *pack)
{
	v[0] = pack[0] / 32767.0f;
	v[1] = pack[1] / 32767.0f;
	v[2] = pack[2] / 32767.0f;
	v[3] = pack[3] / 32767.0f;
}

void R_VaoUnpackNormal(vec3_t v, int16_t *pack)
{
	v[0] = pack[0] / 32767.0f;
	v[1] = pack[1] / 32767.0f;
	v[2] = pack[2] / 32767.0f;
}


static void R_SetVertexPointers(const vertexAttrib_t attribs[ATTRIB_INDEX_COUNT])
{
	uint32_t attribBit;
	const vertexAttrib_t *vAtb;

    for (uint64_t i = 0; i < ATTRIB_INDEX_COUNT; i++) {
		attribBit = 1 << i;
		vAtb = &attribs[i];

        if (vAtb->enabled) {
            nglVertexAttribPointer(vAtb->index, vAtb->count, vAtb->type, vAtb->normalized, vAtb->stride, (const void *)vAtb->offset);
			if (!(glState.vertexAttribsEnabled & attribBit))
				nglEnableVertexAttribArray(vAtb->index);
			
			glState.vertexAttribsEnabled |= attribBit;
        }
        else {
			if ((glState.vertexAttribsEnabled & attribBit))
	            nglDisableVertexAttribArray(vAtb->index);
			
			glState.vertexAttribsEnabled &= ~attribBit;
        }
    }
}

void VBO_SetVertexPointers(vertexBuffer_t *vbo, uint32_t attribBits)
{
	// if nothing is set, set everything
	if (!(attribBits & ATTRIB_BITS))
		attribBits = ATTRIB_BITS;
	
	if (attribBits & ATTRIB_POSITION) {
		vbo->attribs[ATTRIB_INDEX_POSITION].enabled = qtrue;
	}
	if (attribBits & ATTRIB_TEXCOORD) {
		vbo->attribs[ATTRIB_INDEX_TEXCOORD].enabled = qtrue;
	}
	if (attribBits & ATTRIB_COLOR) {
		vbo->attribs[ATTRIB_INDEX_COLOR].enabled = qtrue;
	}
	if ( attribBits & ATTRIB_LIGHTCOORD ) {
		vbo->attribs[ATTRIB_INDEX_LIGHTCOORD].enabled = qtrue;
	}

	R_SetVertexPointers(vbo->attribs);
}

/*
R_ClearVertexPointers: clears all vertex pointers in the current GL state
*/
static void R_ClearVertexPointers(void)
{
	uint32_t attribBit;
    for (uint32_t i = 0; i < ATTRIB_INDEX_COUNT; i++) {
		attribBit = 1 << i;
		if ((glState.vertexAttribsEnabled & attribBit))
	        nglDisableVertexAttribArray(i);
    }
	glState.vertexAttribsEnabled = 0;
}

void R_InitGPUBuffers(void)
{
	uint64_t verticesSize, indicesSize;
	uintptr_t offset;

	ri.Printf(PRINT_INFO, "---------- R_InitGPUBuffers ----------\n");

	rg.numBuffers = 0;

	backend.drawBuffer = R_AllocateBuffer( "batchBuffer", NULL, sizeof(srfVert_t) * MAX_BATCH_VERTICES, NULL, sizeof(glIndex_t) * MAX_BATCH_INDICES, BUFFER_FRAME );

	backend.drawBuffer->attribs[ATTRIB_INDEX_POSITION].enabled		= qtrue;
	backend.drawBuffer->attribs[ATTRIB_INDEX_TEXCOORD].enabled		= qtrue;
	backend.drawBuffer->attribs[ATTRIB_INDEX_COLOR].enabled			= qtrue;
	backend.drawBuffer->attribs[ATTRIB_INDEX_NORMAL].enabled		= qtrue;
	backend.drawBuffer->attribs[ATTRIB_INDEX_WORLDPOS].enabled		= qtrue;
	backend.drawBuffer->attribs[ATTRIB_INDEX_LIGHTCOORD].enabled	= qtrue;

	backend.drawBuffer->attribs[ATTRIB_INDEX_POSITION].count		= 3;
	backend.drawBuffer->attribs[ATTRIB_INDEX_TEXCOORD].count		= 2;
	backend.drawBuffer->attribs[ATTRIB_INDEX_COLOR].count			= 4;
	backend.drawBuffer->attribs[ATTRIB_INDEX_NORMAL].count			= 4;
	backend.drawBuffer->attribs[ATTRIB_INDEX_WORLDPOS].count		= 3;
	backend.drawBuffer->attribs[ATTRIB_INDEX_LIGHTCOORD].count		= 4;

	backend.drawBuffer->attribs[ATTRIB_INDEX_POSITION].type			= GL_FLOAT;
	backend.drawBuffer->attribs[ATTRIB_INDEX_TEXCOORD].type			= GL_FLOAT;
	backend.drawBuffer->attribs[ATTRIB_INDEX_COLOR].type			= GL_UNSIGNED_SHORT;
	backend.drawBuffer->attribs[ATTRIB_INDEX_NORMAL].type			= GL_SHORT;
	backend.drawBuffer->attribs[ATTRIB_INDEX_WORLDPOS].type			= GL_FLOAT;
	backend.drawBuffer->attribs[ATTRIB_INDEX_LIGHTCOORD].type		= GL_FLOAT;

	backend.drawBuffer->attribs[ATTRIB_INDEX_POSITION].index		= ATTRIB_INDEX_POSITION;
	backend.drawBuffer->attribs[ATTRIB_INDEX_TEXCOORD].index		= ATTRIB_INDEX_TEXCOORD;
	backend.drawBuffer->attribs[ATTRIB_INDEX_COLOR].index			= ATTRIB_INDEX_COLOR;
	backend.drawBuffer->attribs[ATTRIB_INDEX_NORMAL].index			= ATTRIB_INDEX_NORMAL;
	backend.drawBuffer->attribs[ATTRIB_INDEX_WORLDPOS].index		= ATTRIB_INDEX_WORLDPOS;
	backend.drawBuffer->attribs[ATTRIB_INDEX_LIGHTCOORD].index		= ATTRIB_INDEX_LIGHTCOORD;

	backend.drawBuffer->attribs[ATTRIB_INDEX_POSITION].normalized	= GL_FALSE;
	backend.drawBuffer->attribs[ATTRIB_INDEX_TEXCOORD].normalized	= GL_FALSE;
	backend.drawBuffer->attribs[ATTRIB_INDEX_COLOR].normalized		= GL_TRUE;
	backend.drawBuffer->attribs[ATTRIB_INDEX_NORMAL].normalized		= GL_TRUE;
	backend.drawBuffer->attribs[ATTRIB_INDEX_WORLDPOS].normalized	= GL_FALSE;
	backend.drawBuffer->attribs[ATTRIB_INDEX_LIGHTCOORD].normalized = GL_FALSE;

	backend.drawBuffer->attribs[ATTRIB_INDEX_POSITION].offset		= offsetof( srfVert_t, xyz );
	backend.drawBuffer->attribs[ATTRIB_INDEX_TEXCOORD].offset		= offsetof( srfVert_t, st );
	backend.drawBuffer->attribs[ATTRIB_INDEX_COLOR].offset			= offsetof( srfVert_t, color );
	backend.drawBuffer->attribs[ATTRIB_INDEX_NORMAL].offset			= offsetof( srfVert_t, normal );
	backend.drawBuffer->attribs[ATTRIB_INDEX_WORLDPOS].offset		= offsetof( srfVert_t, worldPos );
	backend.drawBuffer->attribs[ATTRIB_INDEX_LIGHTCOORD].offset     = offsetof( srfVert_t, lightmap );

	backend.drawBuffer->attribs[ATTRIB_INDEX_POSITION].stride		= sizeof(srfVert_t);
	backend.drawBuffer->attribs[ATTRIB_INDEX_TEXCOORD].stride		= sizeof(srfVert_t);
	backend.drawBuffer->attribs[ATTRIB_INDEX_COLOR].stride			= sizeof(srfVert_t);
	backend.drawBuffer->attribs[ATTRIB_INDEX_NORMAL].stride			= sizeof(srfVert_t);
	backend.drawBuffer->attribs[ATTRIB_INDEX_WORLDPOS].stride		= sizeof(srfVert_t);

	VBO_BindNull();

	GL_CheckErrors();
}

void R_ShutdownGPUBuffers(void)
{
	uint64_t i;
	vertexBuffer_t *vbo;

	ri.Printf(PRINT_INFO, "---------- R_ShutdownGPUBuffers -----------\n");

	VBO_BindNull();

	for (i = 0; i < rg.numBuffers; i++) {
		vbo = rg.buffers[i];

		R_ShutdownBuffer( vbo );
	}

	memset( rg.buffers, 0, sizeof(rg.buffers) );
	rg.numBuffers = 0;
}

static void R_InitGPUMemory(GLenum target, GLuint id, void *data, uint32_t size, bufferType_t usage)
{
	GLbitfield bits;
	GLenum glUsage;

	switch (usage) {
	case BUFFER_DYNAMIC:
	case BUFFER_FRAME:
		glUsage = GL_DYNAMIC_DRAW;
		break;
	case BUFFER_STATIC:
		glUsage = GL_STATIC_DRAW;
		break;
	default:
		ri.Error(ERR_FATAL, "R_AllocateBuffer: invalid buffer usage %i", usage);
	};

#if 0
	// zero clue how well this'll work
	if (r_experimental->i) {
		if (glContext.ARB_map_buffer_range) {
			bits = GL_MAP_READ_BIT;

			if (glUsage == GL_DYNAMIC_DRAW) {
				bits |= GL_MAP_WRITE_BIT;
				if (usage == BUFFER_FRAME) {
					bits |= GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;
				}
			}

			if (glContext.ARB_buffer_storage || NGL_VERSION_ATLEAST(4, 5)) {
				nglBufferStorage(target, size, data, bits | (glUsage == GL_DYNAMIC_DRAW ? GL_DYNAMIC_STORAGE_BIT : 0));
			}
			else {
				nglBufferData(target, size, NULL, glUsage);
			}
		}
	}
	else {
		nglBufferData(target, size, data, glUsage);
	}
#endif
}

#if 0

vertexBuffer_t *R_AllocateBuffer( const char *name, void *vertices, uint32_t verticesSize, void *indices, uint32_t indicesSize, bufferType_t type )
{
    vertexBuffer_t *buf;
	GLenum usage;
	uint32_t namelen;

	switch (type) {
	case BUFFER_STATIC:
		usage = GL_STATIC_DRAW;
		break;
	case BUFFER_FRAME:
	case BUFFER_DYNAMIC:
		usage = GL_DYNAMIC_DRAW;
		break;
	case BUFFER_STREAM:
		usage = GL_STREAM_DRAW;
		break;
	default:
		ri.Error(ERR_FATAL, "Bad glUsage %i", type);
	};

	if (R_BufferExists( name )) {
		ri.Error( ERR_DROP, "R_AllocateBuffer: buffer '%s' already exists", name );
	}
	if (rg.numBuffers == MAX_RENDER_BUFFERS) {
		ri.Error(ERR_DROP, "R_AllocateBuffer: MAX_RENDER_BUFFERS hit");
	}

    // these buffers are only allocated on a per vm
    // and map basis, so use the hunk
    buf = rg.buffers[rg.numBuffers] = (vertexBuffer_t *)ri.Hunk_Alloc( sizeof(*buf), h_low );
	memset( buf, 0, sizeof(*buf) );
    rg.numBuffers++;

    buf->type = type;

	nglGenVertexArrays( 1, &buf->vaoId );
	nglBindVertexArray( buf->vaoId );

	if ( usage != GL_STATIC_DRAW ) {
		if ( glContext.ARB_buffer_storage && glContext.ARB_map_buffer_range && glContext.ARB_sync && r_arb_sync->i && r_mappedBuffers->i ) {
			buf->vertex.usage = BUF_GL_SYNCED;
			buf->index.usage = BUF_GL_SYNCED;
		} else if ( glContext.ARB_buffer_storage && glContext.ARB_map_buffer_range && r_mappedBuffers->i ) {
			buf->vertex.usage = BUF_GL_MAPPED;
			buf->vertex.usage = BUF_GL_MAPPED;
		}
	} else {
		if ( NGL_VERSION_ATLEAST( 2, 1 ) || glContext.ARB_buffer_storage ) {
			buf->vertex.usage = BUF_GL_BUFFER;
			buf->index.usage = BUF_GL_BUFFER;
		} else {
			ri.Printf( PRINT_WARNING, "R_AllocateBuffer: using client-side (CPU) draw buffers, this'll be VERY slow...\n" );
			buf->vertex.usage = BUF_GL_CLIENT;
			buf->index.usage = BUF_GL_CLIENT;

			buf->vertex.data = ri.Hunk_Alloc( verticesSize, h_low );
			buf->index.data = ri.Hunk_Alloc( indicesSize, h_low );
		}

		if ( !vertices ) {
			ri.Error( ERR_FATAL, "R_AllocateBuffer: BUFFER_STATIC usage without any vertices given" );
		}
		if ( !indices ) {
			ri.Error( ERR_FATAL, "R_AllocateBuffer: BUFFER_STATIC usage without any indices given" );
		}
	}

	buf->vertex.glUsage = usage;
	buf->index.glUsage = usage;

	buf->vertex.size = verticesSize;
	buf->index.size = indicesSize;

	nglGenBuffers( 1, &buf->vertex.id );
	nglBindBuffer( GL_ARRAY_BUFFER, buf->vertex.id );

	if ( buf->vertex.usage == BUF_GL_MAPPED && glContext.ARB_buffer_storage && r_mappedBuffers->i ) {
		nglBufferStorage( GL_ARRAY_BUFFER, verticesSize, vertices, GL_MAP_WRITE_BIT | GL_MAP_COHERENT_BIT | GL_MAP_PERSISTENT_BIT );
		buf->vertex.data = nglMapBufferRange( GL_ARRAY_BUFFER, 0, verticesSize, GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT );
	} else {
		nglBufferData( GL_ARRAY_BUFFER, verticesSize, vertices, usage );
	}

	nglGenBuffers( 1, &buf->index.id );
	nglBindBuffer( GL_ELEMENT_ARRAY_BUFFER, buf->index.id );
	if ( buf->index.usage == BUF_GL_MAPPED && glContext.ARB_buffer_storage && r_mappedBuffers->i ) {
		nglBufferStorage( GL_ELEMENT_ARRAY_BUFFER, indicesSize, indices, GL_MAP_WRITE_BIT | GL_MAP_COHERENT_BIT | GL_MAP_PERSISTENT_BIT );
		buf->index.data = nglMapBufferRange( GL_ELEMENT_ARRAY_BUFFER, 0, indicesSize, GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT );
	} else {
		nglBufferData( GL_ELEMENT_ARRAY_BUFFER, indicesSize, indices, usage );
	}

	GL_SetObjectDebugName( GL_VERTEX_ARRAY, buf->vaoId, name, "_vao" );
	GL_SetObjectDebugName( GL_BUFFER, buf->vertex.id, name, "_vbo" );
	GL_SetObjectDebugName( GL_BUFFER, buf->index.id, name, "_ibo" );

    return buf;
}

static void VBO_UnmapBuffers( vertexBuffer_t *vbo )
{
	if ( vbo->vertex.usage == BUF_GL_MAPPED || vbo->vertex.usage == BUF_GL_SYNCED ) {
		nglUnmapBuffer( GL_ARRAY_BUFFER );
		vbo->vertex.data = NULL; // mark as unmapped
	}
	if ( vbo->index.usage == BUF_GL_MAPPED || vbo->index.usage == BUF_GL_SYNCED ) {
		nglUnmapBuffer( GL_ELEMENT_ARRAY_BUFFER );
		vbo->index.data = NULL; // mark as unmapped
	}
}

/*
============
VBO_Bind
============
*/
void VBO_Bind(vertexBuffer_t *vbo)
{
	if ( !vbo ) {
		ri.Error(ERR_DROP, "VBO_Bind: NULL buffer");
		return;
	}

	if ( glState.currentVao != vbo ) {
		if ( glState.currentVao ) {
			VBO_UnmapBuffers( glState.currentVao );
		}

		glState.currentVao = vbo;
		glState.vaoId = vbo->vaoId;
		glState.vboId = vbo->vertex.id;
		glState.iboId = vbo->index.id;
		backend.pc.c_bufferBinds++;

		nglBindVertexArray( vbo->vaoId );

		// Intel Graphics doesn't save GL_ELEMENT_ARRAY_BUFFER binding with VAO binding.
		if ( glContext.intelGraphics ) {
			nglBindBuffer( GL_ELEMENT_ARRAY_BUFFER, vbo->index.id );
		}

		if ( vbo->vertex.usage == BUF_GL_MAPPED || vbo->vertex.usage == BUF_GL_SYNCED ) {
			vbo->vertex.data = nglMapBufferRange( GL_ARRAY_BUFFER, 0, backend.drawBatch.maxVertices * backend.drawBatch.vtxDataSize,
				GL_MAP_WRITE_BIT | GL_MAP_UNSYNCHRONIZED_BIT );
		}
		if ( vbo->index.usage == BUF_GL_MAPPED || vbo->index.usage == BUF_GL_SYNCED ) {
			vbo->index.data = nglMapBufferRange( GL_ELEMENT_ARRAY_BUFFER, 0, backend.drawBatch.maxIndices * backend.drawBatch.idxDataSize,
				GL_MAP_WRITE_BIT | GL_MAP_UNSYNCHRONIZED_BIT );
		}
	}
}

/*
============
VBO_BindNull
============
*/
void VBO_BindNull(void)
{
	ri.GLimp_LogComment( "--- VBO_BindNull ---\n" );

	if ( glState.currentVao ) {
		VBO_UnmapBuffers( glState.currentVao );

		glState.currentVao = NULL;
		glState.vaoId = glState.vboId = glState.iboId = 0;

        nglBindVertexArray( 0 );

        // why you no save GL_ELEMENT_ARRAY_BUFFER binding, Intel?
        if ( glContext.intelGraphics ) {
			nglBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );
		}
	}

	GL_CheckErrors();
}

void R_ShutdownBuffer( vertexBuffer_t *vbo )
{
	if ( vbo->vaoId ) {
		nglDeleteVertexArrays( 1, &vbo->vaoId );
	}

	if ( vbo->vertex.id ) {
		if ( vbo->vertex.usage == BUF_GL_MAPPED && vbo->vertex.data != NULL ) {
			nglBindBuffer( GL_ARRAY_BUFFER, vbo->vertex.id );
			nglUnmapBuffer( GL_ARRAY_BUFFER );
			nglBindBuffer( GL_ARRAY_BUFFER, 0 );
			vbo->vertex.data = NULL;
		}
		nglDeleteBuffers( 1, &vbo->vertex.id );
	}
	
	if ( vbo->index.id ) {
		if ( vbo->index.usage == BUF_GL_MAPPED && vbo->index.data != NULL ) {
			nglBindBuffer( GL_ELEMENT_ARRAY_BUFFER, vbo->index.id );
			nglUnmapBuffer( GL_ELEMENT_ARRAY_BUFFER );
			nglBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );
			vbo->index.data = NULL;
		}
		nglDeleteBuffers( 1, &vbo->index.id );
	}

	rg.numBuffers--;

	memset( vbo, 0, sizeof(*vbo) );
}

static void VBO_OrphanBuffer( qboolean forceCPU )
{
	uint32_t segmentEnd;

	if ( forceCPU || !glContext.ARB_map_buffer_range || !glContext.ARB_sync ) {
		// use client-side buffers
		backend.drawBatch.vertices = backend.drawBuffer->vertex.data;
		backend.drawBatch.indices = backend.drawBuffer->index.data;
		return;
	}

#ifdef GL_ARB_sync
	if ( glContext.ARB_sync && glContext.ARB_buffer_storage && glContext.ARB_map_buffer_range && backend.drawBuffer->vertex.usage == BUF_GL_SYNCED && r_arb_sync->i ) {
		segmentEnd = ( backend.drawBatch.vertexRB.activeSegment + 1 ) * backend.drawBatch.vertexRB.segmentElements;
		if ( backend.drawBatch.vtxOffset + backend.drawBatch.maxVertices > segmentEnd ) {
			backend.drawBatch.vtxOffset = R_RotateRingbuffer( &backend.drawBatch.vertexRB );
		}
		backend.drawBuffer->vertex.data = (void *)((byte *)backend.drawBatch.vertexRB.baseAddr +
			( backend.drawBatch.vtxOffset * backend.drawBatch.vtxDataSize ) );
	} else
#endif
	{
		if ( backend.drawBatch.vtxOffset > 0 ) {
			// orphan the old vertex buffer so that we don't stall on it
			nglBufferData( GL_ARRAY_BUFFER, backend.drawBatch.maxVertices * backend.drawBatch.vtxDataSize, NULL,
				backend.drawBatch.buffer->vertex.glUsage );
#ifdef GL_ARB_map_buffer_range
			if ( backend.drawBuffer->vertex.usage == BUF_GL_MAPPED && glContext.ARB_buffer_storage && glContext.ARB_map_buffer_range ) {
				backend.drawBatch.vertices = nglMapBufferRange( GL_ARRAY_BUFFER, 0, backend.drawBatch.maxVertices * backend.drawBatch.vtxDataSize,
													GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT | GL_MAP_UNSYNCHRONIZED_BIT );
			}
#endif
		}
	}

#ifdef GL_ARB_sync
	if ( glContext.ARB_sync && glContext.ARB_buffer_storage && glContext.ARB_map_buffer_range && backend.drawBuffer->index.usage == BUF_GL_SYNCED && r_arb_sync->i ) {
		segmentEnd = ( backend.drawBatch.indexRB.activeSegment + 1 ) * backend.drawBatch.indexRB.segmentElements;
		if ( backend.drawBatch.idxOffset + backend.drawBatch.maxIndices > segmentEnd ) {
			backend.drawBatch.idxOffset = R_RotateRingbuffer( &backend.drawBatch.indexRB );
		}
		backend.drawBuffer->index.data = (void *)((byte *)backend.drawBatch.indexRB.baseAddr +
			( backend.drawBatch.idxOffset * backend.drawBatch.idxDataSize ) );
	} else
#endif
	{
		if ( backend.drawBatch.idxOffset > 0 ) {
			// orphan the old vertex buffer so that we don't stall on it
			nglBufferData( GL_ELEMENT_ARRAY_BUFFER, backend.drawBatch.maxIndices * backend.drawBatch.idxDataSize, NULL,
				backend.drawBatch.buffer->index.glUsage );
#ifdef GL_ARB_map_buffer_range
			if ( backend.drawBuffer->index.usage == BUF_GL_MAPPED && glContext.ARB_buffer_storage && glContext.ARB_map_buffer_range ) {
				nglBufferStorage( GL_ELEMENT_ARRAY_BUFFER, backend.drawBatch.maxIndices * backend.drawBatch.idxDataSize, NULL,
					GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT );
				backend.drawBatch.indices = nglMapBufferRange( GL_ELEMENT_ARRAY_BUFFER, 0, backend.drawBatch.maxIndices * backend.drawBatch.idxDataSize,
													GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT | GL_MAP_UNSYNCHRONIZED_BIT );
			}
#endif
		}
	}
}

void RB_SetBatchBuffer( vertexBuffer_t *buffer, void *vertexBuffer, uintptr_t vtxSize, void *indexBuffer, uintptr_t idxSize )
{
    // is it already bound?
    if (backend.drawBatch.buffer == buffer) {
        return;
    }
	else if (backend.drawBatch.buffer != buffer) {
		VBO_BindNull();
	}

    // clear anything currently queued
    if (backend.drawBatch.buffer && (backend.drawBatch.vtxOffset || backend.drawBatch.idxOffset)) {
        RB_FlushBatchBuffer();
    }

    backend.drawBatch.buffer = buffer;

    backend.drawBatch.vtxOffset = 0;
    backend.drawBatch.idxOffset = 0;

    backend.drawBatch.vtxDataSize = vtxSize;
    backend.drawBatch.idxDataSize = idxSize;

    backend.drawBatch.maxVertices = buffer->vertex.size;
    backend.drawBatch.maxIndices = buffer->index.size;

    backend.drawBatch.vertices = vertexBuffer;
    backend.drawBatch.indices = indexBuffer;

    // bind the new cache
    VBO_Bind( buffer );

	R_ClearVertexPointers();

    // set the new vertex attrib array state
	R_SetVertexPointers( buffer->attribs );
}

void RB_FlushBatchBuffer( void )
{
    if (!backend.drawBatch.buffer) {
        return;
    }

    // do we actually have something there?
    if (backend.drawBatch.vtxOffset == 0 && backend.drawBatch.idxOffset == 0) {
        return;
    }

    backend.pc.c_dynamicBufferDraws++;

    //
    // upload the data to the gpu
    //

	// orphan the old buffers
	VBO_OrphanBuffer( r_drawMode->i == DRAW_CLIENT_BUFFERED );

	if ( ( glContext.ARB_buffer_storage || glContext.ARB_vertex_buffer_object ) && backend.drawBuffer->vertex.usage == BUF_GL_BUFFER && r_drawMode->i != DRAW_CLIENT_BUFFERED  ) {
		nglBufferSubData( GL_ARRAY_BUFFER, 0, backend.drawBatch.vtxOffset * backend.drawBatch.vtxDataSize, backend.drawBatch.vertices );
	}

	if ( ( glContext.ARB_buffer_storage || glContext.ARB_vertex_buffer_object ) && backend.drawBuffer->index.usage == BUF_GL_BUFFER && r_drawMode->i != DRAW_CLIENT_BUFFERED ) {
		nglBufferSubData( GL_ELEMENT_ARRAY_BUFFER, 0, backend.drawBatch.idxOffset * backend.drawBatch.idxDataSize, backend.drawBatch.indices );
	}

	if ( glContext.ARB_map_buffer_range && backend.drawBuffer->vertex.usage == BUF_GL_MAPPED ) {
		//nglFlushMappedBufferRange( GL_ARRAY_BUFFER, 0, backend.drawBatch.vtxOffset * backend.drawBatch.vtxDataSize );
	}
	if ( glContext.ARB_map_buffer_range && backend.drawBuffer->index.usage == BUF_GL_MAPPED ) {
		//nglFlushMappedBufferRange( GL_ELEMENT_ARRAY_BUFFER, 0, backend.drawBatch.idxOffset * backend.drawBatch.idxDataSize );
	}

	// if it's a mapped buffer, then the data's already been copied

    R_DrawElements( backend.drawBatch.idxOffset, 0 );

	backend.pc.c_bufferIndices += backend.drawBatch.idxOffset;
	backend.pc.c_bufferVertices += backend.drawBatch.vtxOffset;

    backend.drawBatch.idxOffset = 0;
    backend.drawBatch.vtxOffset = 0;
}

void RB_CommitDrawData( const void *verts, uint32_t numVerts, const void *indices, uint32_t numIndices )
{
    if (numVerts >= backend.drawBatch.maxVertices) {
        ri.Error( ERR_DROP, "RB_CommitDrawData: numVerts >= backend.drawBatch.maxVertices (%i >= %i)", numVerts, backend.drawBatch.maxVertices );
    }
    if (numIndices >= backend.drawBatch.maxIndices) {
        ri.Error( ERR_DROP, "RB_CommitDrawData: numIndices >= backend.drawBatch.maxIndices (%i >= %i)", numIndices, backend.drawBatch.maxIndices );
    }

    // do we need to flush?
    if (backend.drawBatch.vtxOffset + numVerts >= backend.drawBatch.maxVertices
    || backend.drawBatch.idxOffset + numIndices >= backend.drawBatch.maxIndices) {
        RB_FlushBatchBuffer();
    }

    //
    // copy the data into the client side buffer
    //

    // we could be submitting either indices or vertices
    if ( verts ) {
        memcpy( (byte *)backend.drawBatch.vertices + (backend.drawBatch.vtxOffset * backend.drawBatch.vtxDataSize), verts, numVerts * backend.drawBatch.vtxDataSize );
    }
    if ( indices ) {
        memcpy( (byte *)backend.drawBatch.indices + (backend.drawBatch.idxOffset * backend.drawBatch.idxDataSize), indices, numIndices * backend.drawBatch.idxDataSize );
    }

    backend.drawBatch.vtxOffset += numVerts;
    backend.drawBatch.idxOffset += numIndices;
}

#if defined( GL_ARB_buffer_storage ) && defined( GL_ARB_sync )
/*
============
R_InitRingbuffer
============
*/
static void R_InitRingbuffer( GLenum target, uint32_t elementSize, uint32_t segmentElements, glRingbuffer_t *rb ) {
	const uint32_t totalSize = elementSize * segmentElements * DYN_BUFFER_SEGMENTS;
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

#endif


vertexBuffer_t *R_AllocateBuffer(const char *name, void *vertices, uint32_t verticesSize, void *indices, uint32_t indicesSize, bufferType_t type)
{
    vertexBuffer_t *buf;
	GLenum usage;
	uint32_t namelen;
	uint64_t i;

	switch (type) {
	case BUFFER_STATIC:
		usage = GL_STATIC_DRAW;
		break;
	case BUFFER_FRAME:
	case BUFFER_DYNAMIC:
		usage = GL_DYNAMIC_DRAW;
		break;
	case BUFFER_STREAM:
		usage = GL_STREAM_DRAW;
		break;
	default:
		ri.Error(ERR_FATAL, "Bad glUsage %i", type);
	};

	for ( i = 0; i < rg.numBuffers; i++ ) {
		if ( !N_stricmp( rg.buffers[i]->name, name ) ) {
			return rg.buffers[i];
		}
	}

	if ( rg.numBuffers == MAX_RENDER_BUFFERS ) {
		ri.Error(ERR_DROP, "R_AllocateBuffer: MAX_RENDER_BUFFERS hit");
	}

    buf = rg.buffers[rg.numBuffers] = ri.Hunk_Alloc( sizeof(*buf), h_low );
	memset( buf, 0, sizeof(*buf) );
    rg.numBuffers++;

    buf->type = type;
	N_strncpyz( buf->name, name, sizeof( buf->name ) );

	nglGenVertexArrays(1, (GLuint *)&buf->vaoId);
	nglBindVertexArray(buf->vaoId);

	buf->vertex.usage = BUF_GL_BUFFER;
	buf->index.usage = BUF_GL_BUFFER;

	buf->vertex.glUsage = usage;
	buf->index.glUsage = usage;

	buf->vertex.size = verticesSize;
	buf->index.size = indicesSize;

	nglGenBuffers(1, (GLuint *)&buf->vertex.id);
	nglBindBuffer(GL_ARRAY_BUFFER, buf->vertex.id);
	nglBufferData(GL_ARRAY_BUFFER, verticesSize, vertices, usage);

	nglGenBuffers(1, (GLuint *)&buf->index.id);
	nglBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buf->index.id);
	nglBufferData(GL_ELEMENT_ARRAY_BUFFER, indicesSize, indices, usage);

	GL_SetObjectDebugName(GL_VERTEX_ARRAY, buf->vaoId, name, "_vao");
	GL_SetObjectDebugName(GL_BUFFER, buf->vertex.id, name, "_vbo");
	GL_SetObjectDebugName(GL_BUFFER, buf->index.id, name, "_ibo");

    return buf;
}

/*
============
VBO_Bind
============
*/
void VBO_Bind(vertexBuffer_t *vbo)
{
	if (!vbo) {
		ri.Error(ERR_DROP, "VBO_Bind: NULL buffer");
		return;
	}

	if (glState.currentVao != vbo) {
		glState.currentVao = vbo;
		glState.vaoId = vbo->vaoId;
		glState.vboId = vbo->vertex.id;
		glState.iboId = vbo->index.id;
		backend.pc.c_bufferBinds++;

		nglBindVertexArray(vbo->vaoId);
		nglBindBuffer( GL_ARRAY_BUFFER, vbo->vertex.id );
		nglBindBuffer( GL_ELEMENT_ARRAY_BUFFER, vbo->index.id );

		// Intel Graphics doesn't save GL_ELEMENT_ARRAY_BUFFER binding with VAO binding.
		if (glContext.intelGraphics)
			nglBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo->index.id);
	}
}

/*
============
VBO_BindNull
============
*/
void VBO_BindNull(void)
{
	ri.GLimp_LogComment("--- VBO_BindNull ---\n");

	if (glState.currentVao) {
		glState.currentVao = NULL;
		glState.vaoId = glState.vboId = glState.iboId = 0;
        nglBindVertexArray(0);
		nglBindBuffer( GL_ARRAY_BUFFER, 0 );
		nglBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );

        // why you no save GL_ELEMENT_ARRAY_BUFFER binding, Intel?
        if (glContext.intelGraphics) nglBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}

	GL_CheckErrors();
}

void R_ShutdownBuffer( vertexBuffer_t *vbo )
{
	if (vbo->vaoId)
		nglDeleteVertexArrays(1, (const GLuint *)&vbo->vaoId);

	if (vbo->vertex.id) {
		if (vbo->vertex.usage == BUF_GL_MAPPED) {
			nglBindBuffer(GL_ARRAY_BUFFER, vbo->vertex.id);
			nglUnmapBuffer(GL_ARRAY_BUFFER);
			nglBindBuffer(GL_ARRAY_BUFFER, 0);
		}
		nglDeleteBuffers(1, (const GLuint *)&vbo->vertex.id);
	}
	
	if (vbo->index.id) {
		if (vbo->index.usage == BUF_GL_MAPPED) {
			nglBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo->index.id);
			nglUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
			nglBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		}
		nglDeleteBuffers(1, (const GLuint *)&vbo->index.id);
	}

	memset( vbo, 0, sizeof(*vbo) );
	rg.numBuffers--;
}

void RB_SetBatchBuffer( vertexBuffer_t *buffer, void *vertexBuffer, uintptr_t vtxSize, void *indexBuffer, uintptr_t idxSize )
{
    // is it already bound?
    if (backend.drawBatch.buffer == buffer) {
        return;
    }
	else if (backend.drawBatch.buffer != buffer) {
		VBO_BindNull();
	}

    // clear anything currently queued
    if (backend.drawBatch.buffer && (backend.drawBatch.vtxOffset || backend.drawBatch.idxOffset)) {
        RB_FlushBatchBuffer();
    }

    backend.drawBatch.buffer = buffer;

    backend.drawBatch.vtxOffset = 0;
    backend.drawBatch.idxOffset = 0;

    backend.drawBatch.vtxDataSize = vtxSize;
    backend.drawBatch.idxDataSize = idxSize;

    backend.drawBatch.maxVertices = buffer->vertex.size;
    backend.drawBatch.maxIndices = buffer->index.size;

    backend.drawBatch.vertices = vertexBuffer;
    backend.drawBatch.indices = indexBuffer;

    // bind the new cache
    VBO_Bind( buffer );

	R_ClearVertexPointers();

    // set the new vertex attrib array state
	R_SetVertexPointers( buffer->attribs );
}

void RB_FlushBatchBuffer( void )
{
    if (!backend.drawBatch.buffer) {
        return;
    }

    // do we actually have something there?
    if (backend.drawBatch.vtxOffset == 0 && backend.drawBatch.idxOffset == 0) {
        return;
    }

    backend.pc.c_dynamicBufferDraws++;

    //
    // upload the data to the gpu
    //

    // orphan the old vertex buffer so that we don't stall on it
    nglBufferData( GL_ARRAY_BUFFER, backend.drawBatch.maxVertices, NULL, backend.drawBatch.buffer->vertex.glUsage );
    nglBufferSubData( GL_ARRAY_BUFFER, 0, backend.drawBatch.vtxOffset * backend.drawBatch.vtxDataSize, backend.drawBatch.vertices );

    // orphan the old index buffer so that we don't stall on it
    nglBufferData( GL_ELEMENT_ARRAY_BUFFER, backend.drawBatch.maxIndices, NULL, backend.drawBatch.buffer->index.glUsage );
    nglBufferSubData( GL_ELEMENT_ARRAY_BUFFER, 0, backend.drawBatch.idxOffset * backend.drawBatch.idxDataSize, backend.drawBatch.indices );

	RB_IterateShaderStages( backend.drawBatch.shader );

	backend.pc.c_bufferIndices += backend.drawBatch.idxOffset;
	backend.pc.c_bufferVertices += backend.drawBatch.vtxOffset;

    backend.drawBatch.idxOffset = 0;
    backend.drawBatch.vtxOffset = 0;

	backend.drawBatch.shader = NULL;
}

void RB_CommitDrawData( const void *verts, uint32_t numVerts, const void *indices, uint32_t numIndices )
{
    if (numVerts >= backend.drawBatch.maxVertices) {
        ri.Error( ERR_DROP, "RB_CommitDrawData: numVerts >= backend.drawBatch.maxVertices (%i >= %i)", numVerts, backend.drawBatch.maxVertices );
    }
    if (numIndices >= backend.drawBatch.maxIndices) {
        ri.Error( ERR_DROP, "RB_CommitDrawData: numIndices >= backend.drawBatch.maxIndices (%i >= %i)", numIndices, backend.drawBatch.maxIndices );
    }

    // do we need to flush?
    if (backend.drawBatch.vtxOffset + numVerts >= backend.drawBatch.maxVertices
    || backend.drawBatch.idxOffset + numIndices >= backend.drawBatch.maxIndices) {
        RB_FlushBatchBuffer();
    }

    //
    // copy the data into the client side buffer
    //

    // we could be submitting either indices or vertices
    if (verts) {
        memcpy( (byte *)backend.drawBatch.vertices + (backend.drawBatch.vtxOffset * backend.drawBatch.vtxDataSize), verts, numVerts * backend.drawBatch.vtxDataSize );
    }
    if (indices) {
        memcpy( (byte *)backend.drawBatch.indices + (backend.drawBatch.idxOffset * backend.drawBatch.idxDataSize), indices, numIndices * backend.drawBatch.idxDataSize );
    }

    backend.drawBatch.vtxOffset += numVerts;
    backend.drawBatch.idxOffset += numIndices;
}


