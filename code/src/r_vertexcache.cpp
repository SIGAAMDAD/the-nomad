#include "n_shared.h"
#include "m_renderer.h"

void R_PushVertexAttrib(vertexCache_t* cache, uint32_t index, uint32_t glType, uint32_t size, uint32_t stride, const void *offset)
{
    if (!cache) {
        N_Error("R_PushVertexAttrib: null cache");
    }
    if (!stride) {
        N_Error("R_PushVertexAttrib: bad stride");
    }
    if (size > 4) {
        N_Error("R_PushVertexAttrib: bad size: %i", size);
    }

    glBindVertexArray(0);
    glBindVertexArray(cache->vaoId);

    glBindBufferARB(GL_ARRAY_BUFFER_ARB, cache->vboId);

    glEnableVertexAttribArray(index);
    glVertexAttribPointer(index, size, glType, GL_FALSE, stride, offset);

    glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);

    glBindVertexArray(0);
}

void R_SetIndexData(vertexCache_t* cache, const uint32_t* indices, uint32_t size)
{
    if (!cache) {
        N_Error("R_SetIndexData: null cache");
    }
    if (!size) {
        N_Error("R_SetIndexData: bad size");
    }
    if (!indices) {
        N_Error("R_SetIndexData: null indices");
    }
    if (!cache->dynamicIBO) {
        Con_Printf("WARNING: call to R_SetIndexData on non-dynamic index buffer");
    }

    // do need to bind a vao if its an element array buffer
    R_BindVertexArray(cache);

    R_BindIndexBuffer(cache);
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER_ARB, 0, size, (const void *)indices);
    R_UnbindIndexBuffer();

    R_UnbindVertexArray();
}

void R_SetVertexData(vertexCache_t* cache, const void* vertices, uint32_t size)
{
    if (!cache) {
        N_Error("R_SetVertexData: null cache");
    }
    if (!size) {
        N_Error("R_SetVertexData: bad size");
    }
    if (!vertices) {
        N_Error("R_SetVertexData: null vertices");
    }
    if (!cache->dynamicVBO) {
        Con_Printf("WARNING: call to R_SetVertexData on non-dynamic vertex buffer");
    }

    // don't need to bind a vao if its an array buffer
    R_BindVertexBuffer(cache);
    glBufferSubData(GL_ARRAY_BUFFER_ARB, 0, size, vertices);
    R_UnbindVertexBuffer();
}

vertexCache_t* R_CreateCache(const void* vertices, uint32_t verticesSize, const uint32_t* indices, uint32_t indicesSize, const char *name)
{
    if (!verticesSize) {
        N_Error("R_CreatCache: bad vertices count");
    }
    if (!indicesSize) {
        N_Error("R_CreateCache: bad indices count");
    }

    vertexCache_t* cache = (vertexCache_t *)Hunk_Alloc(sizeof(vertexCache_t), name, h_low);

    cache->dynamicVBO = qfalse;
    cache->dynamicIBO = qfalse;
    
    // initialize the opengl objects
    glGenVertexArrays(1, &cache->vaoId);
    glGenBuffersARB(1, &cache->vboId);
    glGenBuffersARB(1, &cache->iboId);

    glBindVertexArray(cache->vaoId);
    
    glBindBufferARB(GL_ARRAY_BUFFER_ARB, cache->vboId);
    glBufferDataARB(GL_ARRAY_BUFFER_ARB, verticesSize, vertices, GL_DYNAMIC_DRAW);
    glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);

    glBindBufferARB(GL_ARRAY_BUFFER_ARB, cache->iboId);
    glBufferDataARB(GL_ARRAY_BUFFER_ARB, indicesSize, indices, GL_DYNAMIC_DRAW);
    glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);

    glBindVertexArray(0);

    renderer->vertexCaches[renderer->numVertexCaches] = cache;
    renderer->numVertexCaches++;

    return cache;
}