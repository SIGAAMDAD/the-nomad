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

static vertexBuffer_t *hashTable[MAX_RENDER_BUFFERS];

static qboolean R_BufferExists(const char *name)
{
    return hashTable[Com_GenerateHashValue(name, MAX_RENDER_BUFFERS)] != NULL;
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
			if (glState.currentVao == drawBuf.buf)
				glState.vertexAttribsEnabled |= attribBit;
        }
        else {
			if ((glState.vertexAttribsEnabled & attribBit))
	            nglDisableVertexAttribArray(vAtb->index);
			if (glState.currentVao == drawBuf.buf)
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

	R_SetVertexPointers(vbo->attribs);
}

/*
R_ClearVertexPointers: clears all vertex pointers in the current GL state
*/
static void R_ClearVertexPointers(void)
{
    for (uint64_t i = 0; i < ATTRIB_INDEX_COUNT; i++) {
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

	verticesSize  = sizeof(drawBuf.xyz[0])			* MAX_BATCH_VERTICES;
	verticesSize += sizeof(drawBuf.normal[0])			* MAX_BATCH_VERTICES;
	verticesSize += sizeof(drawBuf.color[0])			* MAX_BATCH_VERTICES;
	verticesSize += sizeof(drawBuf.texCoords[0])		* MAX_BATCH_VERTICES;

	indicesSize = sizeof(drawBuf.indices[0]) * MAX_BATCH_INDICES;

	drawBuf.buf = R_AllocateBuffer("drawBuffer_VBO", NULL, verticesSize, NULL, indicesSize, BUFFER_FRAME);

	drawBuf.buf->attribs[ATTRIB_INDEX_POSITION].index		= ATTRIB_INDEX_POSITION;
	drawBuf.buf->attribs[ATTRIB_INDEX_NORMAL].index			= ATTRIB_INDEX_NORMAL;
	drawBuf.buf->attribs[ATTRIB_INDEX_TEXCOORD].index		= ATTRIB_INDEX_TEXCOORD;
	drawBuf.buf->attribs[ATTRIB_INDEX_COLOR].index			= ATTRIB_INDEX_COLOR;

	drawBuf.buf->attribs[ATTRIB_INDEX_POSITION].enabled		= qtrue;
	drawBuf.buf->attribs[ATTRIB_INDEX_TEXCOORD].enabled		= qtrue;
	drawBuf.buf->attribs[ATTRIB_INDEX_COLOR].enabled		= qtrue;
	drawBuf.buf->attribs[ATTRIB_INDEX_NORMAL].enabled		= qfalse;

	drawBuf.buf->attribs[ATTRIB_INDEX_POSITION].count		= 3;
	drawBuf.buf->attribs[ATTRIB_INDEX_TEXCOORD].count		= 2;
	drawBuf.buf->attribs[ATTRIB_INDEX_COLOR].count			= 4;
	drawBuf.buf->attribs[ATTRIB_INDEX_NORMAL].count			= 4;

	drawBuf.buf->attribs[ATTRIB_INDEX_POSITION].stride		= sizeof(drawBuf.xyz[0]);
	drawBuf.buf->attribs[ATTRIB_INDEX_NORMAL].stride		= sizeof(drawBuf.normal[0]);
	drawBuf.buf->attribs[ATTRIB_INDEX_TEXCOORD].stride		= sizeof(drawBuf.texCoords[0]);
	drawBuf.buf->attribs[ATTRIB_INDEX_COLOR].stride			= sizeof(drawBuf.color[0]);

	drawBuf.buf->attribs[ATTRIB_INDEX_POSITION].type		= GL_FLOAT;
	drawBuf.buf->attribs[ATTRIB_INDEX_NORMAL].type			= GL_SHORT;
	drawBuf.buf->attribs[ATTRIB_INDEX_TEXCOORD].type		= GL_FLOAT;
	drawBuf.buf->attribs[ATTRIB_INDEX_COLOR].type			= GL_UNSIGNED_SHORT;

	drawBuf.buf->attribs[ATTRIB_INDEX_POSITION].normalized	= GL_FALSE;
	drawBuf.buf->attribs[ATTRIB_INDEX_NORMAL].normalized	= GL_TRUE;
	drawBuf.buf->attribs[ATTRIB_INDEX_TEXCOORD].normalized	= GL_FALSE;
	drawBuf.buf->attribs[ATTRIB_INDEX_COLOR].normalized		= GL_TRUE;

	offset = 0;
	drawBuf.buf->attribs[ATTRIB_INDEX_POSITION].offset		= offset; offset += sizeof(drawBuf.xyz[0])		* MAX_BATCH_VERTICES;
	drawBuf.buf->attribs[ATTRIB_INDEX_NORMAL].offset		= offset; offset += sizeof(drawBuf.normal[0])		* MAX_BATCH_VERTICES;
	drawBuf.buf->attribs[ATTRIB_INDEX_TEXCOORD].offset		= offset; offset += sizeof(drawBuf.texCoords[0])	* MAX_BATCH_VERTICES;
	drawBuf.buf->attribs[ATTRIB_INDEX_COLOR].offset			= offset;

	drawBuf.attribPointers[ATTRIB_INDEX_POSITION]			= drawBuf.xyz;
	drawBuf.attribPointers[ATTRIB_INDEX_TEXCOORD]			= drawBuf.texCoords;
	drawBuf.attribPointers[ATTRIB_INDEX_COLOR]				= drawBuf.color;
	drawBuf.attribPointers[ATTRIB_INDEX_NORMAL]				= drawBuf.normal;

	R_SetVertexPointers(drawBuf.buf->attribs);

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

		if (vbo->vaoId)
			nglDeleteVertexArrays(1, (const GLuint *)&vbo->vaoId);

		if (vbo->vertex.id) {
			nglDeleteBuffers(1, (const GLuint *)&vbo->vertex.id);
		}
		
		if (vbo->index.id) {
			nglDeleteBuffers(1, (const GLuint *)&vbo->index.id);
		}
	}

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

vertexBuffer_t *R_AllocateBuffer(const char *name, void *vertices, uint32_t verticesSize, void *indices, uint32_t indicesSize, bufferType_t type)
{
    vertexBuffer_t *buf;
    uint64_t hash;
	GLenum usage;
	uint32_t namelen;
	char newName[1024];

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

    if (R_BufferExists(name)) {
        ri.Error(ERR_DROP, "R_AllocateBuffer: buffer '%s' already exists", name);
    }

	if (rg.numBuffers == MAX_RENDER_BUFFERS) {
		ri.Error(ERR_DROP, "R_AllocateBuffer: MAX_RENDER_BUFFERS hit");
	}

    hash = Com_GenerateHashValue(name, MAX_RENDER_BUFFERS);

    // these buffers are only allocated on a per vm
    // and map basis, so use the hunk
    buf = rg.buffers[rg.numBuffers] = ri.Hunk_Alloc(sizeof(*buf), h_low);
    hashTable[hash] = buf;
    rg.numBuffers++;

    buf->type = type;

	nglGenVertexArrays(1, (GLuint *)&buf->vaoId);
	nglBindVertexArray(buf->vaoId);

	buf->vertex.usage = BUF_GL_BUFFER;
	buf->vertex.usage = BUF_GL_BUFFER;
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
		nglBindBuffer(GL_ARRAY_BUFFER, vbo->vertex.id);
		nglBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo->index.id);

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
		nglBindBuffer(GL_ARRAY_BUFFER, 0);
		nglBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

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
}

/*
==============
RB_UpdateCache

Adapted from Tess_UpdateVBOs from xreal

Update the default VAO to replace the client side vertex arrays
==============
*/
void RB_UpdateCache(uint32_t attribBits)
{
//	GLimp_LogComment("--- RB_UpdateTessVao ---\n");
	uint32_t size;

	backend.pc.c_dynamicBufferDraws++;

	// update the default VAO
	if (drawBuf.numVertices > 0 && drawBuf.numVertices <= MAX_BATCH_VERTICES
	&& drawBuf.numIndices > 0 && drawBuf.numIndices <= MAX_BATCH_INDICES) {
		uint32_t attribIndex;
		uint32_t attribUpload;

		VBO_Bind(drawBuf.buf);

		// orphan old vertex buffer so we don't stall on it
		nglBufferData(GL_ARRAY_BUFFER, drawBuf.buf->vertex.size, NULL, GL_DYNAMIC_DRAW);

		// if nothing to set, set everything
		if (!(attribBits & ATTRIB_BITS))
			attribBits = ATTRIB_BITS;

		attribUpload = attribBits;

		for (attribIndex = 0; attribIndex < ATTRIB_INDEX_COUNT; attribIndex++) {
			uint32_t attribBit = 1 << attribIndex;
			vertexAttrib_t *vAtb = &drawBuf.buf->attribs[attribIndex];

			if (attribUpload & attribBit) {
				// note: tess has a VBO where stride == size
				nglBufferSubData(GL_ARRAY_BUFFER, vAtb->offset, drawBuf.numVertices * vAtb->stride, drawBuf.attribPointers[attribIndex]);
			}
			if (attribBits & attribBit) {
				if (glContext.ARB_vertex_array_object || NGL_VERSION_ATLEAST(3, 0))
					nglVertexAttribPointer(attribIndex, vAtb->count, vAtb->type, vAtb->normalized, vAtb->stride, (const void *)vAtb->offset);
				if (!(glState.vertexAttribsEnabled & attribBit)) {
					nglEnableVertexAttribArray(attribIndex);
					glState.vertexAttribsEnabled |= attribBit;
				}
			}
			else {
				if ((glState.vertexAttribsEnabled & attribBit)) {
					nglDisableVertexAttribArray(attribIndex);
					glState.vertexAttribsEnabled &= ~attribBit;
				}
			}
		}

		// orphan old index buffer so we don't stall on it
		nglBufferData(GL_ELEMENT_ARRAY_BUFFER, drawBuf.buf->index.size, NULL, GL_DYNAMIC_DRAW);

		nglBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, drawBuf.numIndices * sizeof(drawBuf.indices[0]), drawBuf.indices);
	}
}