#ifndef _R_BUFFER_
#define _R_BUFFER_

typedef struct
{
    uint32_t size;
    uint32_t stride;
    uint32_t numAttribs;
    GLuint id;
    bool dynamicBuffer;
} vertexBuffer_t;

typedef struct
{
    uint32_t glType;
    uint32_t count;
    GLuint id;
} indexBuffer_t;

typedef uint32_t glbuffer_t;

typedef struct
{
    void* vertexData;
    uint32_t* indexData;
    uint32_t numAttribs;

    uint32_t numVertices;
    uint32_t vboDrawHint;

    uint32_t numIndices;
    uint32_t iboDrawHint;
    
    GLuint vaoId;
    GLuint vboId;
    GLuint iboId;

    qboolean dynamicVBO;
    qboolean dynamicIBO;
} vertexCache_t;

void R_PushVertexAttrib(vertexCache_t* cache, uint32_t index, uint32_t glType, uint32_t size, uint32_t stride, const void *offset);
void R_SetIndexData(vertexCache_t* cache, const uint32_t* indices, uint32_t size);
void R_SetVertexData(vertexCache_t* cache, const void* vertices, uint32_t size);
vertexCache_t* R_CreateCache(const void* vertices, uint32_t verticesSize, const uint32_t* indices, uint32_t indicesSize, const char *name);

#endif