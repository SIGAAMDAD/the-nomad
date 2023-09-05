#include "rgl_local.h"

static uint32_t r_numCaches = 0;
static uint32_t r_numVAOs = 0;
static uint32_t r_numVBOs = 0;
static uint32_t r_numIBOs = 0;
static uint64_t r_totalVertices = 0;
static uint64_t r_totalIndices = 0;
static uint64_t r_totalMemUsed = 0;

extern "C" void R_BufferInfo_f(void)
{
	Con_Printf("%8i total vertex cache objects", r_numCaches);
	Con_Printf("%8i total vertex array objects", r_numVAOs);
	Con_Printf("%8i total vertex buffer objects", r_numVBOs);
	Con_Printf("%8i total index buffer objects", r_numIBOs);
	Con_Printf("%8lu total memory allocated to buffer objects", r_totalMemUsed);
}

extern "C" void R_PushVertexAttrib(vertexAttrib_t *attribs, uint32_t i, uint64_t index, uint64_t type, uint64_t count, uintptr_t offset)
{
	attribs[i].index = index;
	attribs[i].type = type;
	attribs[i].count = count;
	attribs[i].offset = offset;
}

/*
R_InitCacheBase: initializes a cache's basic features
*/
extern "C" void R_InitCacheBase(vertexCache_t *cache)
{
	if (!r_enableBuffers->i)
		return;

	cache->attribs = (vertexAttrib_t *)(cache + 1);
	cache->attribStride = sizeof(drawVert_t);

	R_PushVertexAttrib(cache->attribs, 0, 0, GL_FLOAT, 3, offsetof(drawVert_t, pos));
	R_PushVertexAttrib(cache->attribs, 1, 1, GL_FLOAT, 2, offsetof(drawVert_t, texcoords));
	R_PushVertexAttrib(cache->attribs, 2, 2, GL_FLOAT, 4, offsetof(drawVert_t, color));

	nglGenVertexArrays(1, (GLuint *)&cache->vaoId);
	nglGenBuffersARB(1, (GLuint *)&cache->vboId);
	nglGenBuffersARB(1, (GLuint *)&cache->iboId);

	// unbind any bound caches
	R_UnbindCache();

	nglBindVertexArray(cache->vaoId);
	nglBindBufferARB(GL_ARRAY_BUFFER_ARB, cache->vboId);
	nglBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, cache->iboId);

	nglEnableVertexAttribArray(0);
	nglVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(drawVert_t), (const void *)offsetof(drawVert_t, pos));
	
	nglEnableVertexAttribArray(1);
	nglVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(drawVert_t), (const void *)offsetof(drawVert_t, texcoords));

	nglBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, 0);
    nglBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
	nglBindVertexArray(0);

    rg.vertexCaches[rg.numVertexCaches] = cache;
    rg.numVertexCaches++;
}

#define RENDER_MAX_INDICES 0x80000
#define RENDER_MAX_VERTICES 0x80000

/*
R_InitFrameCache: initializes a frame-based dynamically sized cache buffer
*/
extern "C" vertexCache_t *R_InitFrameCache(void)
{
	vertexCache_t *cache;

	cache = (vertexCache_t *)ri.Hunk_Alloc(sizeof(*cache) + (sizeof(*cache->attribs) * 4), "GLvcache", h_low);
	r_totalMemUsed += sizeof(*cache) + (sizeof(*cache->attribs) * 4);

	R_InitCacheBase(cache);

	R_BindCache(cache);

	if (r_enableBuffers->i) {
		nglBufferDataARB(GL_ARRAY_BUFFER_ARB, sizeof(drawVert_t) * RENDER_MAX_VERTICES, NULL, GL_DYNAMIC_DRAW);
		nglBufferDataARB(GL_ELEMENT_ARRAY_BUFFER_ARB, sizeof(uint32_t) * RENDER_MAX_INDICES, NULL, GL_DYNAMIC_DRAW);
	}

	r_totalMemUsed += sizeof(drawVert_t) * RENDER_MAX_VERTICES;

	R_UnbindCache();

	cache->cacheType = VERTCACHE_FRAME;
	cache->vertices = NULL;
	cache->numVertices = 0;
	cache->usedVertices = 0;

	return cache;
}

/*
R_InitStaticCache: initializes a static/immutable vertex cache
*/
extern "C" vertexCache_t *R_InitStaticCache(uint64_t numVertices, const drawVert_t *vertices, qboolean stackAllocated)
{
	vertexCache_t *cache;

	cache = (vertexCache_t *)ri.Hunk_Alloc(sizeof(*cache) + (sizeof(*cache->attribs) * 4), "GLvcache", h_low);
	r_totalMemUsed += sizeof(*cache) + (sizeof(*cache->attribs) * 4);

	R_InitCacheBase(cache);

	cache->numVertices = numVertices;
	cache->cacheType = VERTCACHE_STATIC;
	
	// if the data given to this function is stack allocated, make a heap copy of it
	if (stackAllocated) {
		cache->vertices = (drawVert_t *)ri.Hunk_Alloc(sizeof(drawVert_t) * numVertices, "GLvertices", h_low);
		memcpy(cache->vertices, vertices, sizeof(drawVert_t) * numVertices);

		r_totalMemUsed += sizeof(drawVert_t) * numVertices;
	}
	else {
		cache->vertices = (drawVert_t *)vertices;
	}

	R_BindCache(cache);

	if (r_enableBuffers->i) {
		nglBufferDataARB(GL_ARRAY_BUFFER_ARB, sizeof(drawVert_t) * numVertices, cache->vertices, GL_STATIC_DRAW);
	}

	r_totalMemUsed += sizeof(drawVert_t) * numVertices;

	R_UnbindCache();

	return cache;
}

/*
R_DrawFrameCache: draws a frame-based vertex cache and resets the used buffer count.
NOTE: THIS DOES NOT BIND THE CACHE!
*/
extern "C" void R_DrawFrameCache(vertexCache_t *cache)
{
	// swap the gpu data with the updated cpu data
	if (r_enableClientState->i) {
		nglEnableClientState(GL_VERTEX_ARRAY);
		nglEnableClientState(GL_COLOR_ARRAY);
		nglEnableClientState(GL_TEXTURE_COORD_ARRAY);

		nglVertexPointer(3, GL_FLOAT, sizeof(*cache->vertices), cache->vertices[0].pos);
		nglTexCoordPointer(2, GL_FLOAT, sizeof(*cache->vertices), cache->vertices[0].texcoords);
		nglColorPointer(4, GL_FLOAT, sizeof(*cache->vertices), cache->vertices[0].color);

		nglDrawElements(GL_TRIANGLES, cache->usedIndices, GL_UNSIGNED_INT, cache->indices);

		nglDisableClientState(GL_VERTEX_ARRAY);
		nglDisableClientState(GL_COLOR_ARRAY);
		nglDisableClientState(GL_TEXTURE_COORD_ARRAY);
	}
	else if (r_enableBuffers->i) {
		nglBufferSubDataARB(GL_ARRAY_BUFFER_ARB, 0, sizeof(drawVert_t) * cache->usedVertices, cache->vertices);
		nglBufferSubDataARB(GL_ELEMENT_ARRAY_BUFFER_ARB, 0, sizeof(uint32_t) * cache->usedIndices, cache->indices);
		nglDrawElements(GL_TRIANGLES, cache->usedIndices, GL_UNSIGNED_INT, NULL);
	}
	else {
		nglBegin(GL_TRIANGLE_FAN);
		for (uint32_t i = 0; i < cache->usedVertices; i++) {
			nglVertex2fv(cache->vertices[i].pos);
			nglTexCoord2fv(cache->vertices[i].texcoords);
		}
		nglEnd();
	}
	
	cache->usedIndices = 0;
	cache->usedVertices = 0;
}

/*
R_DrawCache: draws a cache based on what type it is, use this only really for development
NOTE: this function binds the cache
*/
extern "C" void R_DrawCache(vertexCache_t *cache)
{
	R_BindCache(cache);
	{
		switch (cache->cacheType) {
		case VERTCACHE_STATIC:
			if (r_enableClientState->i) {
				nglEnableClientState(GL_VERTEX_ARRAY);
				nglEnableClientState(GL_COLOR_ARRAY);
				nglEnableClientState(GL_TEXTURE_COORD_ARRAY);

				nglVertexPointer(3, GL_FLOAT, sizeof(*cache->vertices), cache->vertices[0].pos);
				nglTexCoordPointer(2, GL_FLOAT, sizeof(*cache->vertices), cache->vertices[0].texcoords);
				nglColorPointer(4, GL_FLOAT, sizeof(*cache->vertices), cache->vertices[0].color);

				nglDrawArrays(GL_TRIANGLES, 0, cache->usedVertices);

				nglDisableClientState(GL_VERTEX_ARRAY);
				nglDisableClientState(GL_COLOR_ARRAY);
				nglDisableClientState(GL_TEXTURE_COORD_ARRAY);
			}
			else if (r_enableBuffers->i) {
				nglDrawArrays(GL_TRIANGLES, 0, cache->usedVertices);
			}
			else {
				nglBegin(GL_TRIANGLES);
				for (uint32_t i = 0; i < cache->usedIndices; i++) {
					nglVertex2fv(cache->vertices[i].pos);
					nglTexCoord2fv(cache->vertices[i].texcoords);
				}
				nglEnd();
			}
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
R_ReserveFrameMemory: reserves memory on the zone heap for the current frame. Uses the temporary R_FrameAlloc
*/
extern "C" void R_ReserveFrameMemory(vertexCache_t *cache, uint64_t numVertices)
{
	if (cache->cacheType != VERTCACHE_FRAME)
		ri.N_Error("R_ReserveFrameMemory: cache type isn't VERTCACHE_FRAME");

	cache->vertices = (drawVert_t *)R_FrameAlloc(sizeof(drawVert_t) * numVertices * 4);
	cache->usedVertices = 0;
	cache->numVertices = numVertices * 4;
}

/*
R_ShutdownCache: frees the cache's cpu memory and deallocates the GL buffer objects.
this function does not free the vertices or the indices, you'll have to do that yourself
*/
extern "C" void R_ShutdownCache(vertexCache_t *cache)
{
	// delete the gpu buffers
	if (r_enableBuffers->i) {
		nglDeleteVertexArrays(1, (const GLuint *)&cache->vaoId);
		nglDeleteBuffersARB(1, (const GLuint *)&cache->vboId);
		nglDeleteBuffersARB(1, (const GLuint *)&cache->iboId);
	}
}

/*
R_PushVertices: pushes vertices into the cache's buffer, flushes the buffer if needed
*/
extern "C" void R_PushVertices(vertexCache_t *cache, const drawVert_t *vertices, uint64_t numVertices)
{
	RENDER_ASSERT(cache, "NULL cache");
	RENDER_ASSERT(vertices, "NULL vertices");
//	RENDER_ASSERT(numVertices > cache->numVertices, "Too many vertices");
	RENDER_ASSERT(cache->cacheType == VERTCACHE_FRAME, "cache isn't a frame-based cache");

	drawVert_t *verts;

	// the buffer has been filled, flush it
	if (cache->usedVertices + numVertices >= cache->numVertices) {
		R_BindCache(cache);
		R_DrawFrameCache(cache);
		R_UnbindCache();
	}

	// using memcpy doens't work for some reason with drawVert_t
	verts = &cache->vertices[cache->usedVertices];
	for (uint64_t i = 0; i < numVertices; ++i)
		*verts++ = vertices[i];

	cache->usedVertices += numVertices;
}


extern "C" void R_BindCache(const vertexCache_t *cache)
{
	if (!r_enableBuffers->i)
		return;

	R_BindVertexArray(cache);
	R_BindVertexBuffer(cache);
	R_BindIndexBuffer(cache);
}

extern "C" void R_BindVertexArray(const vertexCache_t *cache)
{
	if (!r_enableBuffers->i)
		return;
	if (!cache) {
		Con_Printf(ERROR, "R_BindVertexArray(NULL)");
		return;
	}
	if (backend.vaoId == cache->vaoId)
		return; // already bound
	else if (backend.vaoId)
		nglBindVertexArray(0);
	
	backend.vaoId = cache->vaoId;
	nglBindVertexArray(cache->vaoId);
}

extern "C" void R_BindIndexBuffer(const vertexCache_t *cache)
{
	if (!r_enableBuffers->i)
		return;
	if (!cache) {
		Con_Printf(ERROR, "R_BindIndexBuffer(NULL)");
		return;
	}
	if (backend.iboId == cache->iboId)
		return; // already bound
	else if (backend.iboId)
		nglBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, 0);
	
	backend.iboId = cache->iboId;
	nglBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, cache->iboId);
}

extern "C" void R_BindVertexBuffer(const vertexCache_t* cache)
{
	if (!r_enableBuffers->i)
		return;
	if (!cache) {
		Con_Printf(ERROR, "R_BindVertexBuffer(NULL)");
		return;
	}
	if (backend.vboId == cache->vboId)
		return; // already bound
	else if (backend.vboId)
		nglBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
	
	backend.vboId = cache->vboId;
	nglBindBufferARB(GL_ARRAY_BUFFER_ARB, cache->vboId);
}

extern "C" void R_UnbindCache(void)
{
	if (!r_enableBuffers->i)
		return;
	
	R_UnbindVertexBuffer();
	R_UnbindIndexBuffer();
	R_UnbindVertexArray();
}

extern "C" void R_UnbindVertexArray(void)
{
	if (!r_enableBuffers->i)
		return;
	if (!backend.vaoId)
		return; // already unbound
	
	backend.vaoId = 0;
	nglBindVertexArray(0);
}

extern "C" void R_UnbindIndexBuffer(void)
{
	if (!r_enableBuffers->i)
		return;
	if (!backend.iboId)
		return; // already unbound
	
	backend.iboId = 0;
	nglBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, 0);
}

extern "C" void R_UnbindVertexBuffer(void)
{
	if (!r_enableBuffers->i)
		return;
	if (!backend.vboId)
		return; // already unbound
	
	backend.vboId = 0;
	nglBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
}
