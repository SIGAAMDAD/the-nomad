#include "rgl_public.h"
#include "rgl_local.h"

vertexCache_t* R_CreateCache(const Vertex* vertices, uint32_t numVertices, const void* indices, uint32_t numIndices, uint32_t indexType)
{
    vertexCache_t *cache;
    uint32_t dataSize;

    if (!numVertices || numVertices >= RENDER_MAX_VERTICES)
        ri.Error("R_CreateCache: bad vertex count");
    if (!numIndices || numIndices >= RENDER_MAX_INDICES)
        ri.Error("R_CreateCache: bad index count");

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
        ri.Error("R_CreateCache: invalid index type");
    };

    cache = (vertexCache_t *)ri.Hunk_Alloc(sizeof(vertexCache_t), "vcache", h_low);

    cache->vertexCache = (Vertex *)ri.Hunk_Alloc(sizeof(Vertex) * RENDER_MAX_VERTICES, "vertexCache", h_low);
    cache->indexCache = ri.Hunk_Alloc(dataSize * RENDER_MAX_INDICES, "indexCache", h_low);

    cache->numVertices = numVertices;
    cache->numIndices = numIndices;

    cache->indexType = indexType;
    cache->indexSize = dataSize;

    memcpy(cache->vertexCache, vertices, sizeof(Vertex) * numVertices);
    memcpy(cache->indexCache, indices, dataSize * numIndices);

    return cache;
}

void R_BindVertexArray(const vertexCache_t *cache)
{
    if (glState.vaoId == cache->vaoId)
        return; // already bound
    if (glState.vaoId)
        nglBindVertexArray(0);
    
    glState.vaoId = cache->vaoId;
    nglBindVertexArray(cache->vaoId);
}

void R_BindVertexBuffer(const vertexCache_t *cache)
{
    if (glState.vboId == cache->vboId)
        return; // already bound
    if (glState.vboId)
        nglBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
    
    glState.vboId = cache->vboId;
    nglBindBufferARB(GL_ARRAY_BUFFER_ARB, cache->vboId);
}

void R_BindIndexBuffer(const vertexCache_t *cache)
{
    if (glState.iboId == cache->iboId)
        return; // already bound
    if (glState.iboId)
        nglBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, 0);
    
    glState.iboId = cache->iboId;
    nglBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, cache->iboId);
}

void R_UnbindVertexArray(void)
{
    if (!glState.vaoId)
        return; // already unbound
    
    glState.vaoId = 0;
    nglBindVertexArray(0);
}

void R_UnbindVertexBuffer(void)
{
    if (!glState.vboId)
        return; // already unbound
    
    glState.vboId = 0;
    nglBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
}

void R_UnbindIndexBuffer(void)
{
    if (!glState.iboId)
        return; // already unbound
    
    glState.iboId = 0;
    nglBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, 0);
}

void R_DrawIndices(const vertexCache_t *cache)
{
    R_BindVertexArray(cache);
    R_BindVertexBuffer(cache);
    R_BindIndexBuffer(cache);

    nglDrawElements(GL_TRIANGLES, cache->numIndices, cache->indexType, NULL);

    R_UnbindIndexBuffer();
    R_UnbindVertexBuffer();
    R_UnbindVertexArray();
}