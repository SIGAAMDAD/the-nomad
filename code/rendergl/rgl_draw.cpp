#include "rgl_public.h"
#include "rgl_local.h"

typedef struct
{
    vertexCache_t *cache;
    Vertex* vertexPtr;
} renderCommand_t;

#define RENDER_MAX_COMMANDS 128

static renderCommand_t commands[RENDER_MAX_COMMANDS];
static renderCommand_t *cPtr;

static void R_ResetCommands(void)
{
    cPtr = &commands[0];
    cPtr->vertexPtr = cPtr->cache->vertexCache;
}

static void R_FlushCommands(void)
{
    const uint32_t count = (uint32_t)(commands - cPtr);
    for (uint32_t i = 0; i < count; ++i) {
        R_DrawIndices(commands[i].cache);
    }
    R_ResetCommands();
}

GDR_EXPORT void RE_DrawCommand(const Quad *quad)
{
    vertexCache_t *cache = cPtr->cache;
    
    if (cPtr + 1 >= &commands[RENDER_MAX_COMMANDS]) {
        R_FlushCommands();
    }
    if (cache->numVertices + 4 >= RENDER_MAX_VERTICES || cache->numIndices + 4 >= RENDER_MAX_INDICES) {
        cPtr++;
        cPtr->cache->numVertices = 0;
        cPtr->cache->numIndices = 0;
        cPtr->vertexPtr = cPtr->cache->vertexCache;
        cache = cPtr->cache;
    }

    cache->numVertices += 4;
    cache->numIndices += 6;
    
    memcpy(cPtr->vertexPtr, quad->vertices, sizeof(quad->vertices));
}

/*
RE_BeginFrame: called at the beginning of every single frame to reinitialize all the buffers
*/
GDR_EXPORT void RE_BeginFrame(void)
{
    R_ApplyViewMatrix();
    
    nglClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    nglViewport(0, 0, glState.width, glState.height);
}

/*
RE_EndFrame: buffer all vertices and rendering data until frame needs to be drawn, unless buffer overflow
*/
GDR_EXPORT void RE_EndFrame(void)
{
    R_FlushCommands();
    R_UnbindAll();

    // swap the buffers and finish up
    SDL_GL_SwapBuffers();
}