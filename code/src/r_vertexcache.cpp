#include "n_shared.h"
#include "m_renderer.h"

VertexCache* RGL_InitCache(const vertex_t *vertices, uint64_t numVertices,
	const void *indices, uint64_t numIndices, uint32_t indexType)
{
	VertexCache *cache;
	uint32_t dataSize;
	
	RENDER_ASSERT(numVertices && numVertices < RENDER_MAX_VERTICES, "bad vertex count");
	RENDER_ASSERT(numIndices && numIndices < RENDER_MAX_INDICES, "bad index count");
	
	switch (indexType) {
	case GL_UNSIGNED_BYTE:
		dataSize = sizeof(GLubyte);
		break;
	case GL_UNSIGNED_SHORT:
		dataSize = sizeof(GLushort);
		break;
	case GL_UNSIGNED_INT:
		dataSize = sizeof(GLuint);
		break;
	default:
		RENDER_ASSERT(false, "invalid index buffer type");
	};
	
	cache = (VertexCache *)Hunk_Alloc(sizeof(VertexCache), "vcache", h_low);
	memset(cache, 0, sizeof(cache));
	
//	cache->attribs = Hunk_Alloc(sizeof(vertexAttrib_t) * 3, "vertexAttribs", h_low);
//	vertexAttrib_t *attribs = (vertexAttrib_t *)cache->attribs;
//	
//	attribs[0].index = 0;
//	attribs[0].type = GL_FLOAT;
//	attribs[0].count = 3;
//	attribs[0].offset = offsetof(vertex_t, pos);
//	
//	attribs[1].index = 1;
//	attribs[1].type = GL_FLOAT;
//	attribs[1].count = 2;
//	attribs[1].offset = offsetof(vertex_t, texcoords);
//	
//	attribs[2].index = 2;
//	attribs[2].type = GL_FLOAT;
//	attribs[2].count = 4;
//	attribs[2].offset = offsetof(vertex_t, color);
//	
//	cache->numVertices = numVertices;
//	cache->numIndices = numIndices;
//	cache->indexSize = dataSize;
//	cache->indexType = indexType;

    cache->numVertices = numVertices;
    cache->numIndices = numIndices;
    cache->indexSize = dataSize;
    cache->indexType = indexType;

    if (vertices)
        memcpy(cache->vertices, vertices, sizeof(vertex_t) * numVertices);
    if (indices)
        memcpy(cache->indices, indices, dataSize * numIndices);
	
	RGL_UnbindCache();
	glGenVertexArrays(1, (GLuint *)&cache->vaoId);
	glGenBuffersARB(1, (GLuint *)&cache->vbo.glObj);
	glGenBuffersARB(1, (GLuint *)&cache->ibo.glObj);
	
	glBindVertexArray(cache->vaoId);
	
	glBindBufferARB(GL_ARRAY_BUFFER_ARB, cache->vbo.glObj);
	glBufferDataARB(GL_ARRAY_BUFFER_ARB, sizeof(vertex_t) * RENDER_MAX_VERTICES, NULL, GL_DYNAMIC_DRAW);
    glBufferSubDataARB(GL_ARRAY_BUFFER_ARB, 0, sizeof(vertex_t) * numVertices, vertices);
	
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vertex_t), (const void *)offsetof(vertex_t, pos));
	
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(vertex_t), (const void *)offsetof(vertex_t, texcoords));
	
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(vertex_t), (const void *)offsetof(vertex_t, color));
	
	glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, cache->ibo.glObj);
	glBufferDataARB(GL_ELEMENT_ARRAY_BUFFER_ARB, dataSize * RENDER_MAX_INDICES, NULL, GL_DYNAMIC_DRAW);
    glBufferSubDataARB(GL_ELEMENT_ARRAY_BUFFER_ARB, 0, dataSize * numIndices, indices);
	glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, 0);
    glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
	
	glBindVertexArray(0);

    renderer->vertexCaches[renderer->numVertexCaches] = cache;
    renderer->numVertexCaches++;
	
	return cache;
}

void RGL_ShutdownCache(VertexCache* cache)
{
	RENDER_ASSERT(cache, "NULL cache");
	glDeleteVertexArrays(1, (GLuint *)&cache->vaoId);
	glDeleteBuffersARB(1, (GLuint *)&cache->vbo.glObj);
	glDeleteBuffersARB(1, (GLuint *)&cache->vbo.glObj);
}

void RGL_SwapVertexData(const vertex_t *vertices, uint64_t numVertices, VertexCache* cache)
{
	RENDER_ASSERT(cache, "NULL cache");
	RENDER_ASSERT(vertices, "NULL vertices");
	
    vertex_t *verts = &cache->vertices[cache->numVertices];
    for (uint32_t i = 0; i < numVertices; ++i) {
        verts[i].pos = vertices[i].pos;
        verts[i].texcoords = vertices[i].texcoords;
        verts[i].color = vertices[i].color;
    }

	cache->numVertices += numVertices;
}

void RGL_SwapIndexData(const void *indices, uint64_t numIndices, VertexCache* cache)
{
	RENDER_ASSERT(cache, "NULL cache");
	RENDER_ASSERT(indices, "NULL indices");

    switch (cache->indexType) {
    case GL_UNSIGNED_BYTE:
        memcpy(&((GLubyte *)cache->indices)[cache->numIndices], indices, numIndices * cache->indexSize);
        break;
    case GL_UNSIGNED_SHORT:
        memcpy(&((GLushort *)cache->indices)[cache->numIndices], indices, numIndices * cache->indexSize);
        break;
    case GL_UNSIGNED_INT:
        memcpy(&((GLuint *)cache->indices)[cache->numIndices], indices, numIndices * cache->indexSize);
        break;
    };

    cache->numIndices += numIndices;
}

void RGL_ResetIndexData(VertexCache *cache)
{
    cache->numIndices = 0;
}

void RGL_ResetVertexData(VertexCache *cache)
{
    cache->numVertices = 0;
}

void RGL_DrawCache(VertexCache *cache)
{
	RENDER_ASSERT(cache, "NULL cache");
	
	RGL_BindCache(cache);

    glBufferSubDataARB(GL_ARRAY_BUFFER_ARB, 0, sizeof(vertex_t) * cache->numVertices, cache->vertices);
    glBufferSubDataARB(GL_ELEMENT_ARRAY_BUFFER_ARB, 0, cache->indexSize * cache->numIndices, cache->indices);

	glDrawElements(GL_TRIANGLES, cache->numIndices, cache->indexType, NULL);

    RGL_ResetIndexData(cache);
    RGL_ResetVertexData(cache);
	
	RGL_UnbindCache();
}

void RGL_BindCache(const VertexCache *cache)
{
	RGL_BindVertexArray(cache);
	RGL_BindVertexBuffer(cache);
	RGL_BindIndexBuffer(cache);
}

void RGL_BindVertexArray(const VertexCache *cache)
{
	if (renderer->vaoid == cache->vaoId)
		return; // already bound
	else if (renderer->vaoid)
		glBindVertexArray(0);
	
	renderer->vaoid = cache->vaoId;
	glBindVertexArray(cache->vaoId);
}

void RGL_BindVertexBuffer(const VertexCache* cache)
{
	if (renderer->vboid == cache->vbo.glObj)
		return; // already bound
	else if (renderer->vboid)
		glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
	
	renderer->vboid = cache->vbo.glObj;
	glBindBufferARB(GL_ARRAY_BUFFER_ARB, cache->vbo.glObj);
}

void RGL_BindIndexBuffer(const VertexCache* cache)
{
	if (renderer->iboid == cache->ibo.glObj)
		return; // already bound
	else if (renderer->iboid)
		glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, 0);
	
	renderer->iboid = cache->ibo.glObj;
	glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, cache->ibo.glObj);
}

void RGL_UnbindCache(void)
{
	RGL_UnbindVertexBuffer();
	RGL_UnbindIndexBuffer();
	RGL_UnbindVertexArray();
}
void RGL_UnbindVertexArray(void)
{
	if (!renderer->vaoid)
		return; // already unbound
	
	renderer->vaoid = 0;
	glBindVertexArray(0);
}

void RGL_UnbindVertexBuffer(void)
{
	if (!renderer->vboid)
		return; // already unbound
	
	renderer->vboid = 0;
	glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
}

void RGL_UnbindIndexBuffer(void)
{
	if (!renderer->iboid)
		return; // already unbound
	
	renderer->iboid = 0;
	glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, 0);
}