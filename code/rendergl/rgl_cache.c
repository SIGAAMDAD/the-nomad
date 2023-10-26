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
			if (glState.currentVao == backend->dbuf.buf)
				glState.vertexAttribsEnabled |= attribBit;
        }
        else {
			if ((glState.vertexAttribsEnabled & attribBit))
	            nglDisableVertexAttribArray(vAtb->index);
			if (glState.currentVao == backend->dbuf.buf)
				glState.vertexAttribsEnabled &= ~attribBit;
        }
    }
}

/*
R_ClearVertexPointers: clears all vertex pointers in the current GL state
*/
static void R_ClearVertexPointers(void)
{
    for (uint64_t i = 0; i < ATTRIB_INDEX_COUNT; i++) {
        nglDisableVertexAttribArray(i);
    }
}

void R_InitGPUBuffers(void)
{
	uint64_t verticesSize, indicesSize;
	uintptr_t offset;

	ri.Printf(PRINT_INFO, "---------- R_InitGPUBuffers ----------\n");

	rg.numBuffers = 0;

	verticesSize  = sizeof(backend->dbuf.xyz[0])			* MAX_BATCH_VERTICES;
	verticesSize += sizeof(backend->dbuf.normal[0])			* MAX_BATCH_VERTICES;
	verticesSize += sizeof(backend->dbuf.color[0])			* MAX_BATCH_VERTICES;
	verticesSize += sizeof(backend->dbuf.texCoords[0])		* MAX_BATCH_VERTICES;

	indicesSize = sizeof(backend->dbuf.indices[0]) * MAX_BATCH_INDICES;

	backend->dbuf.buf = R_AllocateBuffer("drawBuffer_VBO", NULL, verticesSize, NULL, indicesSize, BUFFER_FRAME);

	backend->dbuf.buf->attribs[ATTRIB_INDEX_POSITION].index			= ATTRIB_INDEX_POSITION;
	backend->dbuf.buf->attribs[ATTRIB_INDEX_NORMAL].index			= ATTRIB_INDEX_NORMAL;
	backend->dbuf.buf->attribs[ATTRIB_INDEX_TEXCOORD].index			= ATTRIB_INDEX_TEXCOORD;
	backend->dbuf.buf->attribs[ATTRIB_INDEX_COLOR].index			= ATTRIB_INDEX_COLOR;

	backend->dbuf.buf->attribs[ATTRIB_INDEX_POSITION].enabled		= qtrue;
	backend->dbuf.buf->attribs[ATTRIB_INDEX_TEXCOORD].enabled		= qtrue;
	backend->dbuf.buf->attribs[ATTRIB_INDEX_COLOR].enabled			= qtrue;
	backend->dbuf.buf->attribs[ATTRIB_INDEX_NORMAL].enabled			= qtrue;

	backend->dbuf.buf->attribs[ATTRIB_INDEX_POSITION].count			= 3;
	backend->dbuf.buf->attribs[ATTRIB_INDEX_TEXCOORD].count			= 2;
	backend->dbuf.buf->attribs[ATTRIB_INDEX_COLOR].count			= 4;
	backend->dbuf.buf->attribs[ATTRIB_INDEX_NORMAL].count			= 4;

	backend->dbuf.buf->attribs[ATTRIB_INDEX_POSITION].stride		= sizeof(backend->dbuf.xyz[0]);
	backend->dbuf.buf->attribs[ATTRIB_INDEX_NORMAL].stride			= sizeof(backend->dbuf.normal[0]);
	backend->dbuf.buf->attribs[ATTRIB_INDEX_TEXCOORD].stride		= sizeof(backend->dbuf.texCoords[0]);
	backend->dbuf.buf->attribs[ATTRIB_INDEX_COLOR].stride			= sizeof(backend->dbuf.color[0]);

	backend->dbuf.buf->attribs[ATTRIB_INDEX_POSITION].type			= GL_FLOAT;
	backend->dbuf.buf->attribs[ATTRIB_INDEX_NORMAL].type			= GL_SHORT;
	backend->dbuf.buf->attribs[ATTRIB_INDEX_TEXCOORD].type			= GL_FLOAT;
	backend->dbuf.buf->attribs[ATTRIB_INDEX_COLOR].type				= GL_UNSIGNED_SHORT;

	backend->dbuf.buf->attribs[ATTRIB_INDEX_POSITION].normalized	= GL_FALSE;
	backend->dbuf.buf->attribs[ATTRIB_INDEX_NORMAL].normalized		= GL_TRUE;
	backend->dbuf.buf->attribs[ATTRIB_INDEX_TEXCOORD].normalized	= GL_FALSE;
	backend->dbuf.buf->attribs[ATTRIB_INDEX_COLOR].normalized		= GL_TRUE;

	offset = 0;
	backend->dbuf.buf->attribs[ATTRIB_INDEX_POSITION].offset		= offset; offset += sizeof(backend->dbuf.xyz[0])		* MAX_BATCH_VERTICES;
	backend->dbuf.buf->attribs[ATTRIB_INDEX_NORMAL].offset			= offset; offset += sizeof(backend->dbuf.normal[0])		* MAX_BATCH_VERTICES;
	backend->dbuf.buf->attribs[ATTRIB_INDEX_TEXCOORD].offset		= offset; offset += sizeof(backend->dbuf.texCoords[0])	* MAX_BATCH_VERTICES;
	backend->dbuf.buf->attribs[ATTRIB_INDEX_COLOR].offset			= offset;

	backend->dbuf.attribPointers[ATTRIB_INDEX_POSITION]		= backend->dbuf.xyz;
	backend->dbuf.attribPointers[ATTRIB_INDEX_TEXCOORD]		= backend->dbuf.texCoords;
	backend->dbuf.attribPointers[ATTRIB_INDEX_COLOR]		= backend->dbuf.color;
	backend->dbuf.attribPointers[ATTRIB_INDEX_NORMAL]		= backend->dbuf.normal;

	R_SetVertexPointers(backend->dbuf.buf->attribs);

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

	// zero clue how well this'll work
	if (r_experimental->i) {
		if (glContext->ARB_map_buffer_range) {
			bits = GL_MAP_READ_BIT;

			if (glUsage == GL_DYNAMIC_DRAW) {
				bits |= GL_MAP_WRITE_BIT;
				if (usage == BUFFER_FRAME) {
					bits |= GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;
				}
			}

			if (glContext->ARB_buffer_storage || NGL_VERSION_ATLEAST(4, 5)) {
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
		backend->pc.c_bufferBinds++;

		nglBindVertexArray(vbo->vaoId);
		nglBindBuffer(GL_ARRAY_BUFFER, vbo->vertex.id);
		nglBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo->index.id);

		// Intel Graphics doesn't save GL_ELEMENT_ARRAY_BUFFER binding with VAO binding.
		if (glContext->intelGraphics)
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
        if (glContext->intelGraphics) nglBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}

	GL_CheckErrors();
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

	backend->pc.c_dynamicBufferDraws++;

	// update the default VAO
	if (backend->dbuf.numVertices > 0 && backend->dbuf.numVertices <= MAX_BATCH_VERTICES
	&& backend->dbuf.numIndices > 0 && backend->dbuf.numIndices <= MAX_BATCH_INDICES) {
		uint32_t attribIndex;
		uint32_t attribUpload;

		VBO_Bind(backend->dbuf.buf);

		// orphan old vertex buffer so we don't stall on it
		nglBufferData(GL_ARRAY_BUFFER, backend->dbuf.buf->vertex.size, NULL, GL_DYNAMIC_DRAW);

		// if nothing to set, set everything
		if (!(attribBits & ATTRIB_BITS))
			attribBits = ATTRIB_BITS;

		attribUpload = attribBits;

		for (attribIndex = 0; attribIndex < ATTRIB_INDEX_COUNT; attribIndex++) {
			uint32_t attribBit = 1 << attribIndex;
			vertexAttrib_t *vAtb = &backend->dbuf.buf->attribs[attribIndex];

			if (attribUpload & attribBit) {
				// note: tess has a VBO where stride == size
				nglBufferSubData(GL_ARRAY_BUFFER, vAtb->offset, backend->dbuf.numVertices * vAtb->stride, backend->dbuf.attribPointers[attribIndex]);
			}
			if (attribBits & attribBit) {
				if (glContext->ARB_vertex_array_object || NGL_VERSION_ATLEAST(3, 0))
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
		nglBufferData(GL_ELEMENT_ARRAY_BUFFER, backend->dbuf.buf->index.size, NULL, GL_DYNAMIC_DRAW);

		nglBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, backend->dbuf.numIndices * sizeof(backend->dbuf.indices[0]), backend->dbuf.indices);
	}
}

#if 0

// FIXME: This sets a limit of 65536 verts/262144 indexes per static surface
// This is higher than the old vq3 limits but is worth noting
#define VAOCACHE_QUEUE_MAX_SURFACES (1 << 10)
#define VAOCACHE_QUEUE_MAX_VERTEXES (1 << 16)
#define VAOCACHE_QUEUE_MAX_INDEXES (VAOCACHE_QUEUE_MAX_VERTEXES * 4)

typedef struct queuedSurface_s
{
	drawVert_t *vertexes;
	int numVerts;
	glIndex_t *indexes;
	int numIndices;
} queuedSurface_t;

static struct
{
	queuedSurface_t surfaces[VAOCACHE_QUEUE_MAX_SURFACES];
	int numSurfaces;

	drawVert_t vertexes[VAOCACHE_QUEUE_MAX_VERTEXES];
	int vertexCommitSize;

	glIndex_t indexes[VAOCACHE_QUEUE_MAX_INDEXES];
	int indexCommitSize;
} vcq;

#define VAOCACHE_MAX_SURFACES (1 << 16)
#define VAOCACHE_MAX_BATCHES (1 << 10)

// drawVert_t is 60 bytes
// assuming each vert is referenced 4 times, need 16 bytes (4 glIndex_t) per vert
// -> need about 4/15ths the space for indexes as vertexes
#if GL_INDEX_TYPE == GL_UNSIGNED_SHORT
#define VAOCACHE_VERTEX_BUFFER_SIZE (sizeof(drawVert_t) * USHRT_MAX)
#define VAOCACHE_INDEX_BUFFER_SIZE (sizeof(glIndex_t) * USHRT_MAX * 4)
#else // GL_UNSIGNED_INT
#define VAOCACHE_VERTEX_BUFFER_SIZE (16 * 1024 * 1024)
#define VAOCACHE_INDEX_BUFFER_SIZE (5 * 1024 * 1024)
#endif

typedef struct buffered_s
{
	void *data;
    uint64_t size;
	uintptr_t bufferOffset;
} buffered_t;

static struct
{
	vertexBuffer_t *vao;
	buffered_t surfaceIndexSets[VAOCACHE_MAX_SURFACES];
	uint64_t numSurfaces;

	uint64_t batchLengths[VAOCACHE_MAX_BATCHES];
	uint64_t numBatches;

	uintptr_t vertexOffset;
	uintptr_t indexOffset;
} vc;

void VaoCache_Commit(void)
{
	buffered_t *indexSet;
	uint64_t *batchLength;
	queuedSurface_t *surf, *end = vcq.surfaces + vcq.numSurfaces;

    VBO_Bind(vc.vao);

	// Search for a matching batch
	// FIXME: Use faster search
	indexSet = vc.surfaceIndexSets;
	batchLength = vc.batchLengths;
	for (; batchLength < vc.batchLengths + vc.numBatches; batchLength++) {
		if (*batchLength == vcq.numSurfaces) {
			buffered_t *indexSet2 = indexSet;
			for (surf = vcq.surfaces; surf < end; surf++, indexSet2++) {
				if (surf->indexes != indexSet2->data || (surf->numIndices * sizeof(glIndex_t)) != indexSet2->size)
					break;
			}

			if (surf == end)
				break;
		}

		indexSet += *batchLength;
	}

	// If found, use it
	if (indexSet < vc.surfaceIndexSets + vc.numSurfaces) {
		backend->dbuf.firstIndex = indexSet->bufferOffset / sizeof(glIndex_t);
		//ri.Printf(PRINT_ALL, "firstIndex %d numIndices %d as %d\n", backend->dbuf.firstIndex, backend->dbuf.numIndices, (int)(batchLength - vc.batchLengths));
		//ri.Printf(PRINT_ALL, "vc.numSurfaces %d vc.numBatches %d\n", vc.numSurfaces, vc.numBatches);
	}
	// If not, rebuffer the batch
	// FIXME: keep track of the vertexes so we don't have to reupload them every time
	else {
		drawVert_t *dstVertex = vcq.vertexes;
		glIndex_t *dstIndex = vcq.indexes;

		batchLength = vc.batchLengths + vc.numBatches;
		*batchLength = vcq.numSurfaces;
		vc.numBatches++;

		backend->dbuf.firstIndex = vc.indexOffset / sizeof(glIndex_t);
		vcq.vertexCommitSize = 0;
		vcq.indexCommitSize = 0;

		for (surf = vcq.surfaces; surf < end; surf++) {
			glIndex_t *srcIndex = surf->indexes;
			uint32_t vertexesSize = surf->numVerts * sizeof(drawVert_t);
			uint32_t indexesSize = surf->numIndices * sizeof(glIndex_t);
			uint32_t i, indexOffset = (vc.vertexOffset + vcq.vertexCommitSize) / sizeof(drawVert_t);

			memcpy(dstVertex, surf->vertexes, vertexesSize);
			dstVertex += surf->numVerts;

			vcq.vertexCommitSize += vertexesSize;

			indexSet = vc.surfaceIndexSets + vc.numSurfaces;
			indexSet->data = surf->indexes;
			indexSet->size = indexesSize;
			indexSet->bufferOffset = vc.indexOffset + vcq.indexCommitSize;
			vc.numSurfaces++;

			for (i = 0; i < surf->numIndices; i++)
				*dstIndex++ = *srcIndex++ + indexOffset;

			vcq.indexCommitSize += indexesSize;
		}

		//ri.Printf(PRINT_ALL, "committing %d to %d, %d to %d as %d\n", vcq.vertexCommitSize, vc.vertexOffset, vcq.indexCommitSize, vc.indexOffset, (int)(batchLength - vc.batchLengths));

		if (vcq.vertexCommitSize) {
			nglBindBuffer(GL_ARRAY_BUFFER, vc.vao->vertex.id);
			nglBufferSubData(GL_ARRAY_BUFFER, vc.vertexOffset, vcq.vertexCommitSize, vcq.vertexes);
			vc.vertexOffset += vcq.vertexCommitSize;
		}

		if (vcq.indexCommitSize) {
			nglBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vc.vao->index.id);
			nglBufferSubData(GL_ELEMENT_ARRAY_BUFFER, vc.indexOffset, vcq.indexCommitSize, vcq.indexes);
			vc.indexOffset += vcq.indexCommitSize;
		}
	}
}

void VaoCache_Init(void)
{
	vc.vao = R_AllocateBuffer("VaoCache",  NULL, VAOCACHE_VERTEX_BUFFER_SIZE, NULL, VAOCACHE_INDEX_BUFFER_SIZE, BUFFER_FRAME);

	vc.vao->attribs[ATTRIB_INDEX_POSITION].index		= ATTRIB_INDEX_POSITION;
	vc.vao->attribs[ATTRIB_INDEX_NORMAL].index			= ATTRIB_INDEX_NORMAL;
	vc.vao->attribs[ATTRIB_INDEX_TEXCOORD].index		= ATTRIB_INDEX_TEXCOORD;
	vc.vao->attribs[ATTRIB_INDEX_COLOR].index			= ATTRIB_INDEX_COLOR;

	vc.vao->attribs[ATTRIB_INDEX_POSITION].enabled       = qtrue;
	vc.vao->attribs[ATTRIB_INDEX_TEXCOORD].enabled       = qtrue;
	vc.vao->attribs[ATTRIB_INDEX_NORMAL].enabled         = qtrue;
	vc.vao->attribs[ATTRIB_INDEX_COLOR].enabled          = qtrue;

	vc.vao->attribs[ATTRIB_INDEX_POSITION].count       = 3;
	vc.vao->attribs[ATTRIB_INDEX_TEXCOORD].count       = 2;
	vc.vao->attribs[ATTRIB_INDEX_NORMAL].count         = 4;
	vc.vao->attribs[ATTRIB_INDEX_COLOR].count          = 4;

	vc.vao->attribs[ATTRIB_INDEX_POSITION].type             = GL_FLOAT;
	vc.vao->attribs[ATTRIB_INDEX_TEXCOORD].type             = GL_FLOAT;
	vc.vao->attribs[ATTRIB_INDEX_NORMAL].type               = GL_SHORT;
	vc.vao->attribs[ATTRIB_INDEX_COLOR].type                = GL_UNSIGNED_SHORT;

	vc.vao->attribs[ATTRIB_INDEX_POSITION].normalized       = GL_FALSE;
	vc.vao->attribs[ATTRIB_INDEX_TEXCOORD].normalized       = GL_FALSE;
	vc.vao->attribs[ATTRIB_INDEX_NORMAL].normalized         = GL_TRUE;
	vc.vao->attribs[ATTRIB_INDEX_COLOR].normalized          = GL_TRUE;

	vc.vao->attribs[ATTRIB_INDEX_POSITION].offset       = offsetof(drawVert_t, xyz);
	vc.vao->attribs[ATTRIB_INDEX_TEXCOORD].offset       = offsetof(drawVert_t, uv);
	vc.vao->attribs[ATTRIB_INDEX_NORMAL].offset         = offsetof(drawVert_t, normal);
	vc.vao->attribs[ATTRIB_INDEX_COLOR].offset          = offsetof(drawVert_t, color);

	vc.vao->attribs[ATTRIB_INDEX_POSITION].stride		= sizeof(drawVert_t);
	vc.vao->attribs[ATTRIB_INDEX_TEXCOORD].stride		= sizeof(drawVert_t);
	vc.vao->attribs[ATTRIB_INDEX_COLOR].stride			= sizeof(drawVert_t);
	vc.vao->attribs[ATTRIB_INDEX_NORMAL].stride			= sizeof(drawVert_t);

	R_SetVertexPointers(vc.vao->attribs);

	VBO_BindNull();

	vc.numSurfaces = 0;
	vc.numBatches = 0;
	vc.vertexOffset = 0;
	vc.indexOffset = 0;
	vcq.vertexCommitSize = 0;
	vcq.indexCommitSize = 0;
	vcq.numSurfaces = 0;
}

void VaoCache_BindVao(void)
{
    VBO_Bind(vc.vao);
}

void VaoCache_CheckAdd(qboolean *endSurface, qboolean *recycleVertexBuffer, qboolean *recycleIndexBuffer, uint32_t numVerts, uint32_t numIndices)
{
	uint64_t vertexesSize = sizeof(drawVert_t) * numVerts;
	uint64_t indexesSize = sizeof(glIndex_t) * numIndices;

	if (vc.vao->vertices.size < vc.vertexOffset + vcq.vertexCommitSize + vertexesSize) {
		//ri.Printf(PRINT_ALL, "out of space in vertex cache: %d < %d + %d + %d\n", vc.vao->vertexesSize, vc.vertexOffset, vcq.vertexCommitSize, vertexesSize);
		*recycleVertexBuffer = qtrue;
		*recycleIndexBuffer = qtrue;
		*endSurface = qtrue;
	}

	if (vc.vao->indices.size < vc.indexOffset + vcq.indexCommitSize + indexesSize) {
		//ri.Printf(PRINT_ALL, "out of space in index cache\n");
		*recycleIndexBuffer = qtrue;
		*endSurface = qtrue;
	}

	if (vc.numSurfaces + vcq.numSurfaces >= VAOCACHE_MAX_SURFACES) {
		//ri.Printf(PRINT_ALL, "out of surfaces in index cache\n");
		*recycleIndexBuffer = qtrue;
		*endSurface = qtrue;
	}

	if (vc.numBatches >= VAOCACHE_MAX_BATCHES) {
		//ri.Printf(PRINT_ALL, "out of batches in index cache\n");
		*recycleIndexBuffer = qtrue;
		*endSurface = qtrue;
	}

	if (vcq.numSurfaces >= VAOCACHE_QUEUE_MAX_SURFACES) {
		//ri.Printf(PRINT_ALL, "out of queued surfaces\n");
		*endSurface = qtrue;
	}

	if (VAOCACHE_QUEUE_MAX_VERTEXES * sizeof(drawVert_t) < vcq.vertexCommitSize + vertexesSize) {
		//ri.Printf(PRINT_ALL, "out of queued vertexes\n");
		*endSurface = qtrue;
	}

	if (VAOCACHE_QUEUE_MAX_INDEXES * sizeof(glIndex_t) < vcq.indexCommitSize + indexesSize) {
		//ri.Printf(PRINT_ALL, "out of queued indexes\n");
		*endSurface = qtrue;
	}
}

void VaoCache_RecycleVertexBuffer(void)
{
	nglBindBuffer(GL_ARRAY_BUFFER, vc.vao->vboId);
	nglBufferData(GL_ARRAY_BUFFER, vc.vao->vertices.size, NULL, GL_DYNAMIC_DRAW);
	vc.vertexOffset = 0;
}

void VaoCache_RecycleIndexBuffer(void)
{
	nglBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vc.vao->iboId);
	nglBufferData(GL_ELEMENT_ARRAY_BUFFER, vc.vao->indices.size, NULL, GL_DYNAMIC_DRAW);
	vc.indexOffset = 0;
	vc.numSurfaces = 0;
	vc.numBatches = 0;
}

void VaoCache_InitQueue(void)
{
	vcq.vertexCommitSize = 0;
	vcq.indexCommitSize = 0;
	vcq.numSurfaces = 0;
}

void VaoCache_AddSurface(drawVert_t *verts, uint32_t numVerts, glIndex_t *indexes, uint32_t numIndices)
{
	queuedSurface_t *queueEntry = vcq.surfaces + vcq.numSurfaces;
	queueEntry->vertexes = verts;
	queueEntry->numVerts = numVerts;
	queueEntry->indexes = indexes;
	queueEntry->numIndices = numIndices;
	vcq.numSurfaces++;

	vcq.vertexCommitSize += sizeof(drawVert_t) * numVerts;
	vcq.indexCommitSize += sizeof(glIndex_t) * numIndices;
}

#endif
