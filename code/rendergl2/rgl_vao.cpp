#include "rgl_local.h"

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

static CVertexCache *hashTable[MAX_RENDER_BUFFERS];

static qboolean R_BufferExists(const char *name)
{
    return hashTable[Com_GenerateHashValue(name, MAX_RENDER_BUFFERS)] != NULL;
}

GDR_EXPORT void CVertexCache::SetVertexPointers( void ) const
{
	uint32_t attribBit;
	const vertexAttrib_t *vAtb;

    for (uint64_t i = 0; i < ATTRIB_INDEX_COUNT; i++) {
		attribBit = 1 << i;
		vAtb = &m_Attribs[i];

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

GDR_EXPORT void CVertexCache::Init( const char *name, uint64_t verticesSize, void *vertices, uint64_t indicesSize, void *indices, bufferType_t type )
{
    GLenum usage;

    switch (type) {
    case BUFFER_FRAME:
    case BUFFER_DYNAMIC:
        usage = GL_DYNAMIC_DRAW;
        break;
    case BUFFER_STATIC:
        usage = GL_STATIC_DRAW;
        break;
    default:
        ri.Error( ERR_FATAL, "R_AllocateBuffer: invalid buffer usage" );
    };

    // if we hint GL_DYNAMIC_DRAW or GL_STREAM_DRAW and we give it non-null data, it'll segfault
    if (usage == GL_DYNAMIC_DRAW && (vertices || indices)) {
        ri.Error( ERR_FATAL, "R_AllocateBuffer: don't use BUFFER_FRAME and give vertices or indices, it'll segfault" );
    }

    m_Type = type;

	m_hVertex.m_Usage = BUF_GL_BUFFER;
	m_hIndex.m_Usage = BUF_GL_BUFFER;
	m_hVertex.m_nSize = verticesSize;
	m_hIndex.m_nSize = indicesSize;

    N_strncpyz( m_szName, name, sizeof(m_szName) );

    nglGenVertexArrays( 1, &m_VaoId );
    nglGenBuffers( 1, &m_hVertex.m_Id );
    nglGenBuffers( 1, &m_hIndex.m_Id );

    nglBindVertexArray( m_VaoId );
    nglBindBuffer( GL_ARRAY_BUFFER, m_hVertex.m_Id );
    nglBindBuffer( GL_ELEMENT_ARRAY_BUFFER, m_hIndex.m_Id );

    nglBufferData( GL_ARRAY_BUFFER, verticesSize, vertices, usage );
    nglBufferData( GL_ELEMENT_ARRAY_BUFFER, indicesSize, indices, usage );

    nglBindVertexArray( 0 );
    nglBindBuffer( GL_ARRAY_BUFFER, 0 );
    nglBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );

    GL_SetObjectDebugName(GL_VERTEX_ARRAY, m_VaoId, name, "_vao");
	GL_SetObjectDebugName(GL_BUFFER, m_hVertex.m_Id, name, "_vbo");
	GL_SetObjectDebugName(GL_BUFFER, m_hIndex.m_Id, name, "_ibo");
}

GDR_EXPORT void CVertexCache::Shutdown( void )
{
    if (m_VaoId) {
        nglDeleteVertexArrays( 1, &m_VaoId );
        m_VaoId = 0;
    }
    if (m_hVertex.m_Id) {
        nglDeleteBuffers( 1, &m_hVertex.m_Id );
        m_hVertex.m_Id = 0;
    }
    if (m_hIndex.m_Id) {
        nglDeleteBuffers( 1, &m_hIndex.m_Id );
        m_hIndex.m_Id = 0;
    }
}

GDR_EXPORT CVertexCache *R_AllocateBuffer( const char *name, uint64_t verticesSize, void *vertices, uint64_t indicesSize, void *indices, bufferType_t type )
{
    CVertexCache *buf;
    uint64_t hash;

    if (R_BufferExists( name )) {
        ri.Error( ERR_FATAL, "R_AllocateBuffer: created the same buffer twice" );
    }
    if (strlen( name ) >= MAX_GDR_PATH) {
        ri.Error( ERR_FATAL, "R_AllocateBuffer: name \"%s\" too long", name );
    }
    if (rg.numBuffers == MAX_RENDER_BUFFERS) {
        ri.Error( ERR_DROP, "R_AllocateBuffer: MAX_RENDER_BUFFERS hit" );
    }

    hash = Com_GenerateHashValue(name, MAX_RENDER_BUFFERS);

    // these buffers are only allocated on a per vm
    // and map basis, so use the hunk
    buf = rg.buffers[rg.numBuffers] = (CVertexCache *)ri.Hunk_Alloc(sizeof(*buf), h_low);
    hashTable[hash] = buf;
    rg.numBuffers++;

    buf->Init( name, verticesSize, vertices, indicesSize, indices, type );

    return buf;
}

GDR_EXPORT void VBO_Bind( CVertexCache *vbo )
{
    if (!vbo) {
        VBO_BindNull();
        return;
    }
    if (glState.currentVao != vbo) {
        vbo->Bind();
        glState.currentVao = vbo;
        backend.pc.c_bufferBinds++;
    }
}

GDR_EXPORT void VBO_BindNull( void )
{
    if (glState.currentVao) {
        nglBindVertexArray( 0 );
        if (glContext.intelGraphics) {
            nglBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );
        }
        glState.currentVao = NULL;
    }
}

GDR_EXPORT void R_InitGPUBuffers( void )
{
    backend.drawBuffer = R_AllocateBuffer( "drawBuffer", MAX_BATCH_VERTICES, NULL, MAX_BATCH_INDICES, NULL, BUFFER_DYNAMIC );

    backend.drawBuffer->SetAttributes( {
        vertexAttrib_t(
            ATTRIB_INDEX_POSITION,
            3,
            GL_FLOAT,
            qtrue,
            GL_FALSE,
            sizeof(polyVert_t),
            offsetof(polyVert_t, xyz)
        ),
        vertexAttrib_t(
            ATTRIB_INDEX_TEXCOORD,
            2,
            GL_FLOAT,
            qtrue,
            GL_FALSE,
            sizeof(polyVert_t),
            offsetof(polyVert_t, uv)
        ),
        vertexAttrib_t(
            ATTRIB_INDEX_COLOR,
            4,
            GL_UNSIGNED_INT,
            qtrue,
            GL_TRUE,
            sizeof(polyVert_t),
            offsetof(polyVert_t, modulate)
        )
    } );
}

GDR_EXPORT void R_ShutdownGPUBuffers( void )
{
    uint64_t i;
    CVertexCache *vbo;

	ri.Printf(PRINT_INFO, "---------- R_ShutdownGPUBuffers -----------\n");

	VBO_BindNull();

	for (i = 0; i < rg.numBuffers; i++) {
		vbo = rg.buffers[i];
        vbo->Shutdown();
	}

	memset( rg.buffers, 0, sizeof(rg.buffers) );
	memset( hashTable, 0, sizeof(hashTable) );
	rg.numBuffers = 0;
}

GDR_EXPORT void RB_SetBatchBuffer( CVertexCache *buffer, void *vertexBuffer, uintptr_t vtxSize, void *indexBuffer, uintptr_t idxSize )
{
    // is it already bound?
    if (backend.drawBatch.buffer == buffer) {
        return;
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

    backend.drawBatch.maxVertices = buffer->GetVertexObject()->m_nSize;
    backend.drawBatch.maxIndices = buffer->GetIndexObject()->m_nSize;

    backend.drawBatch.vertices = vertexBuffer;
    backend.drawBatch.indices = indexBuffer;

    // bind the new cache
    VBO_Bind( buffer );

    // set the new vertex attrib array state
    buffer->SetVertexPointers();
}

GDR_EXPORT void RB_FlushBatchBuffer( void )
{
    if (!backend.drawBatch.buffer) {
        ri.Printf( PRINT_WARNING, "RB_FlushBatchBuffer: called without a buffer attached.\n" );
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
    nglBufferData( GL_ARRAY_BUFFER, backend.drawBatch.maxVertices, NULL, backend.drawBatch.buffer->GetVertexObject()->m_Usage );
    nglBufferSubData( GL_ARRAY_BUFFER, 0, backend.drawBatch.vtxOffset * backend.drawBatch.vtxDataSize, backend.drawBatch.vertices );

    // orphan the old index buffer so that we don't stall on it
    nglBufferData( GL_ELEMENT_ARRAY_BUFFER, backend.drawBatch.maxIndices, NULL, backend.drawBatch.buffer->GetIndexObject()->m_Usage );
    nglBufferSubData( GL_ELEMENT_ARRAY_BUFFER, 0, backend.drawBatch.idxOffset * backend.drawBatch.idxDataSize, backend.drawBatch.indices );

    R_DrawElements( backend.drawBatch.idxOffset, 0 );

    backend.drawBatch.idxOffset = 0;
    backend.drawBatch.vtxOffset = 0;
}

GDR_EXPORT void RB_CommitDrawData( const void *verts, uint32_t numVerts, const void *indices, uint32_t numIndices )
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
