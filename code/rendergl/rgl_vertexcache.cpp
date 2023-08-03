#include "rgl_local.h"

/*
R_InitCacheBase: initializes a cache's basic features
*/
GO_AWAY_MANGLE void R_InitCacheBase(vertexCache_t *cache)
{
	cache->attribs = (vertexAttrib_t *)ri.Z_Malloc(sizeof(vertexAttrib_t) * 3, TAG_STATIC, &cache->attribs, "GLvattribs");

	cache->attribs[0].index = 0;
	cache->attribs[0].type = GL_FLOAT;
	cache->attribs[0].count = 3;
	cache->attribs[0].offset = offsetof(vertex_t, pos);
	
	cache->attribs[1].index = 1;
	cache->attribs[1].type = GL_FLOAT;
	cache->attribs[1].count = 2;
	cache->attribs[1].offset = offsetof(vertex_t, texcoords);
	
	cache->attribs[2].index = 2;
	cache->attribs[2].type = GL_FLOAT;
	cache->attribs[2].count = 4;
	cache->attribs[2].offset = offsetof(vertex_t, color);

	nglGenVertexArrays(1, (GLuint *)&cache->vaoId);
	nglGenBuffersARB(1, (GLuint *)&cache->vboId);
	nglGenBuffersARB(1, (GLuint *)&cache->iboId);

	R_UnbindCache();
	nglBindVertexArray(cache->vaoId);
	nglBindBufferARB(GL_ARRAY_BUFFER_ARB, cache->vboId);
	
    for (uint64_t i = 0; i < cache->numAttribs; ++i) {
    	nglEnableVertexAttribArray(cache->attribs[i].index);
	    nglVertexAttribPointer(cache->attribs[i].index, cache->attribs[i].count, cache->attribs[i].type, GL_FALSE,
            cache->attribStride, (const void *)cache->attribs[i].offset);
    }
    nglBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
	nglBindVertexArray(0);

    renderer->vertexCaches[renderer->numVertexCaches] = cache;
    renderer->numVertexCaches++;
}

/*
R_InitFrameCache: initializes a frame-based dynamically sized cache buffer
*/
GO_AWAY_MANGLE vertexCache_t *R_InitFrameCache(void)
{
	vertexCache_t *cache;

	cache = (vertexCache_t *)ri.Z_Malloc(sizeof(vertexCache_t), TAG_STATIC, &cache, "GLvcache");

	R_InitCacheBase(cache);

	R_BindCache(cache);

	nglBufferDataARB(GL_ARRAY_BUFFER_ARB, sizeof(vertex_t) * RENDER_MAX_VERTICES, NULL, GL_STREAM_DRAW);
	nglBufferDataARB(GL_ELEMENT_ARRAY_BUFFER_ARB, sizeof(uint32_t) * RENDER_MAX_INDICES, NULL, GL_STREAM_DRAW);

	R_UnbindCache();

	cache->cacheType = VERTCACHE_FRAME;
	cache->vertices = NULL;
	cache->indices = NULL;
	cache->numIndices = 0;
	cache->numVertices = 0;
	cache->usedIndices = 0;
	cache->usedVertices = 0;

	return cache;
}

/*
R_InitStaticCache: initializes a static/immutable vertex cache
*/
GO_AWAY_MANGLE vertexCache_t *R_InitStaticCache(uint64_t numVertices, const vertex_t *vertices, uint64_t numIndices, const uint32_t *indices,
	bool stackAllocated)
{
	vertexCache_t *cache;

	cache = (vertexCache_t *)ri.Z_Malloc(sizeof(vertexCache_t), TAG_STATIC, &cache, "GLvcache");

	R_InitCacheBase(cache);

	cache->numIndices = numIndices;
	cache->numVertices = numVertices;
	cache->cacheType = VERTCACHE_STATIC;
	
	// if the data given to this function is stack allocated, make a heap copy of it
	if (stackAllocated) {
		cache->vertices = (vertex_t *)ri.Z_Malloc(sizeof(vertex_t) * numVertices, TAG_RENDERER, &cache->vertices, "GLvertices");
		cache->indices = (uint32_t *)ri.Z_Malloc(sizeof(uint32_t) * numIndices, TAG_RENDERER, &cache->indices, "GLindices");
		memcpy(cache->vertices, vertices, sizeof(vertex_t) * numVertices);
		memcpy(cache->indices, indices, sizeof(uint32_t) * numIndices);
	}
	else {
		cache->vertices = (vertex_t *)vertices;
		cache->indices = (uint32_t *)indices;
	}

	R_BindCache(cache);

	nglBufferDataARB(GL_ARRAY_BUFFER_ARB, sizeof(vertex_t) * numVertices, cache->vertices, GL_STATIC_DRAW);
	nglBufferDataARB(GL_ELEMENT_ARRAY_BUFFER_ARB, sizeof(uint32_t) * numIndices, cache->indices, GL_STATIC_DRAW);

	R_UnbindCache();

	return cache;
}

/*
R_DrawFrameCache: draws a frame-based vertex cache and resets the used buffer count.
NOTE: THIS DOES NOT BIND THE CACHE!
*/
GO_AWAY_MANGLE void R_DrawFrameCache(vertexCache_t *cache)
{
	if (!cache->usedIndices || !cache->usedVertices)
		return; // don't draw it if its already been flushed

	// swap the gpu data with the updated cpu data
	nglBufferSubDataARB(GL_ARRAY_BUFFER_ARB, 0, sizeof(vertex_t) * cache->usedVertices, cache->vertices);
	nglBufferSubDataARB(GL_ELEMENT_ARRAY_BUFFER_ARB, 0, sizeof(uint32_t) * cache->usedIndices, cache->indices);
	
	nglDrawElements(GL_TRIANGLES, cache->numIndices, GL_UNSIGNED_INT, NULL);
	
	cache->usedIndices = 0;
	cache->usedVertices = 0;
}

/*
R_DrawCache: draws a cache based on what type it is, use this only really for development
NOTE: this function binds the cache
*/
GO_AWAY_MANGLE void R_DrawCache(vertexCache_t *cache)
{
	R_BindCache(cache);
	{
		switch (cache->cacheType) {
		case VERTCACHE_STATIC:
			nglDrawElements(GL_TRIANGLES, cache->numIndices, GL_UNSIGNED_INT, NULL);
			break;
		case VERTCACHE_FRAME:
			R_DrawFrameCache(cache);
			break;
		default: ri.N_Error("R_DrawCache: invalid cache type!");
		};
	}
	R_UnbindCache();
}

/*
R_ReserveFrameMemory: reserves memory on the zone heap for the current frame based on FRAME_RESERVE_MEMORY
*/
GO_AWAY_MANGLE void R_ReserveFrameMemory(vertexCache_t *cache, uint64_t numVertices, uint64_t numIndices)
{
	if (cache->cacheType != VERTCACHE_FRAME)
		ri.N_Error("R_ReserveFrameMemory: cache type isn't VERTCACHE_FRAME");

	cache->vertices = (vertex_t *)R_FrameAlloc(sizeof(vertex_t) * numVertices * 4);
	cache->indices = (uint32_t *)R_FrameAlloc(sizeof(uint32_t) * numIndices * 6);

	cache->usedIndices = 0;
	cache->usedVertices = 0;

	cache->numIndices = numIndices * 6;
	cache->numVertices = numVertices * 4;
}

/*
R_ShutdownCache: frees the cache's cpu memory and deallocates the GL buffer objects.
this function does not free the vertices or the indices, you'll have to do that yourself
*/
GO_AWAY_MANGLE void R_ShutdownCache(vertexCache_t *cache)
{
	// delete the gpu buffers
	nglDeleteVertexArrays(1, (const GLuint *)&cache->vaoId);
	nglDeleteBuffersARB(1, (const GLuint *)&cache->vboId);
	nglDeleteBuffersARB(1, (const GLuint *)&cache->iboId);

	// free the cpu memory back into the zone
	ri.Z_Free(cache);
}

/*
R_PushVertices: pushes vertices into the cache's buffer, flushes the buffer if needed
*/
GO_AWAY_MANGLE void R_PushVertices(vertexCache_t *cache, const vertex_t *vertices, uint64_t numVertices)
{
	RENDER_ASSERT(cache, "NULL cache");
	RENDER_ASSERT(vertices, "NULL vertices");
//	RENDER_ASSERT(numVertices > cache->numVertices, "Too many vertices");
	RENDER_ASSERT(cache->cacheType == VERTCACHE_FRAME, "cache isn't a frame-based cache");

	vertex_t *verts;

	// the buffer has been filled, flush it
	if (cache->usedVertices + numVertices >= cache->numVertices) {
		R_BindCache(cache);
		R_DrawFrameCache(cache);
		R_UnbindCache();
	}

	// using memcpy doens't work for some reason with vertex_t
	verts = &cache->vertices[cache->usedVertices];
	for (uint64_t i = 0; i < numVertices; ++i)
		*verts++ = vertices[i];

	cache->usedVertices += numVertices;
}

/*
R_PushIndices: pushes indices into the cache's buffer, flushes the buffer if needed
*/
GO_AWAY_MANGLE void R_PushIndices(vertexCache_t *cache, const uint32_t *indices, uint64_t numIndices)
{
	RENDER_ASSERT(cache, "NULL cache");
	RENDER_ASSERT(indices, "NULL indices");
//	RENDER_ASSERT(numIndices > cache->numIndices, "Too many indices");
	RENDER_ASSERT(cache->cacheType == VERTCACHE_FRAME, "cache isn't a frame-based cache");

	// the buffer has been filled, flush it
	if (cache->usedVertices + numIndices >= cache->numIndices) {
		R_BindCache(cache);
		R_DrawFrameCache(cache);
		R_UnbindCache();
	}

	// the index type is a primitive, use memcpy instead of a manual copy
	memcpy(cache->indices, indices, sizeof(uint32_t) * numIndices);
	cache->usedIndices += numIndices;
}

GO_AWAY_MANGLE void R_BindCache(const vertexCache_t *cache)
{
	R_BindVertexArray(cache);
	R_BindVertexBuffer(cache);
	R_BindIndexBuffer(cache);
}

GO_AWAY_MANGLE void R_BindVertexArray(const vertexCache_t *cache)
{
	if (renderer->vaoid == cache->vaoId)
		return; // already bound
	else if (renderer->vaoid)
		nglBindVertexArray(0);
	
	renderer->vaoid = cache->vaoId;
	nglBindVertexArray(cache->vaoId);
}

GO_AWAY_MANGLE void R_BindVertexBuffer(const vertexCache_t* cache)
{
	if (renderer->vboid == cache->vboId)
		return; // already bound
	else if (renderer->vboid)
		nglBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
	
	renderer->vboid = cache->vboId;
	nglBindBufferARB(GL_ARRAY_BUFFER_ARB, cache->vboId);
}

GO_AWAY_MANGLE void R_BindIndexBuffer(const vertexCache_t* cache)
{
	if (renderer->iboid == cache->iboId)
		return; // already bound
	else if (renderer->iboid)
		nglBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, 0);
	
	renderer->iboid = cache->iboId;
	nglBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, cache->iboId);
}

GO_AWAY_MANGLE void R_UnbindCache(void)
{
	R_UnbindVertexBuffer();
	R_UnbindIndexBuffer();
	R_UnbindVertexArray();
}

GO_AWAY_MANGLE void R_UnbindVertexArray(void)
{
	if (!renderer->vaoid)
		return; // already unbound
	
	renderer->vaoid = 0;
	nglBindVertexArray(0);
}

GO_AWAY_MANGLE void R_UnbindVertexBuffer(void)
{
	if (!renderer->vboid)
		return; // already unbound
	
	renderer->vboid = 0;
	nglBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
}

GO_AWAY_MANGLE void R_UnbindIndexBuffer(void)
{
	if (!renderer->iboid)
		return; // already unbound
	
	renderer->iboid = 0;
	nglBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, 0);
}