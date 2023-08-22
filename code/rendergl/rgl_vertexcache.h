#ifndef _RGL_VERTEXCACHE_
#define _RGL_VERTEXCACHE_

#pragma once

#pragma pack(push, 1)
typedef struct vertex_s
{
    vec4_t color;
    vec3_t pos;
    vec2_t texcoords;
} vertex_t;

typedef struct 
{
    uint64_t index;
    uint64_t count;
    uint64_t type;
    uintptr_t offset;
} vertexAttrib_t;
#pragma pack(pop)

#define FRAME_RESERVE_MEMORY 10*1024

typedef enum {
    VERTCACHE_FRAME = 0,        // assumed to be update on a per-frame basis
    VERTCACHE_STATIC,           // size and data is constant
    VERTCACHE_FULLSCREEN,       // a fullscreen buffer, most used to render fullscreen textures
} vertexCacheType_t;

typedef struct
{
    uint32_t *indices;
    drawVert_t *vertices;

    vertexAttrib_t *attribs;
    uint64_t numAttribs;
    uintptr_t attribStride;

    uint32_t numVertices;
    uint32_t numIndices;

    // unused by static caches
    uint32_t usedVertices;
    uint32_t usedIndices;

    uint32_t vaoId;
    uint32_t vboId;
    uint32_t iboId;

    vertexCacheType_t cacheType;
} vertexCache_t;

extern "C" vertexCache_t *R_InitFrameCache(void);
extern "C" vertexCache_t *R_InitStaticCache(uint64_t numVertices, const drawVert_t *vertices, qboolean stackAllocated);
extern "C" void R_DrawFrameCache(vertexCache_t *cache);
extern "C" void R_DrawCache(vertexCache_t *cache);
extern "C" void R_PushVertices(vertexCache_t *cache, const drawVert_t *vertices, uint64_t numVertices);
extern "C" void R_ShutdownCache(vertexCache_t *cache);
extern "C" void R_BindCache(const vertexCache_t *cache);
extern "C" void R_ReserveFrameMemory(vertexCache_t *cache, uint64_t numVertices);
extern "C" void R_UnbindCache(void);
extern "C" void R_BindVertexBuffer(const vertexCache_t *cache);
extern "C" void R_UnbindVertexBuffer(void);
extern "C" void R_BindIndexBuffer(const vertexCache_t *cache);
extern "C" void R_UnbindIndexBuffer(void);
extern "C" void R_BindVertexArray(const vertexCache_t *cache);
extern "C" void R_UnbindVertexArray(void);

#endif