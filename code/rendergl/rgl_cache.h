#ifndef _RGL_CACHE_
#define _RGL_CACHE_

#pragma once

typedef struct
{
    GLuint vaoId;
    GLuint vboId;
    GLuint iboId;

    uint32_t numVertices;
    uint32_t numIndices;

    uint32_t indexSize;
    uint32_t indexType;

    Vertex *vertexCache;
    void *indexCache;
} vertexCache_t;

vertexCache_t* R_CreateCache(const Vertex *vertices, uint32_t numVertices, const void *indices, uint32_t numIndices, uint32_t indexType);
void R_BindVertexArray(const vertexCache_t *cache);
void R_BindVertexBuffer(const vertexCache_t *cache);
void R_BindIndexBuffer(const vertexCache_t *cache);
void R_UnbindVertexArray(void);
void R_UnbindVertexBuffer(void);
void R_UnbindIndexBuffer(void);
void R_DrawIndices(const vertexCache_t *cache);

#endif