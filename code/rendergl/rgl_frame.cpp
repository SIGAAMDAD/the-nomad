#include "rgl_local.h"
#include "../src/g_bff.h"

typedef struct {
	uint32_t c_alloc, c_free;       // total bytes allocated/freed from R_StaticAlloc and R_StaticFree
    uint32_t c_frameMemCalls;       // total calls to R_FrameAlloc
    uint32_t c_frameMemUsed;        // total temporary frame memory used
    uint32_t c_frameMemAllocated;   // total temporary frame memory allocated
    uint32_t c_frameMemFreed;       // total temporary frame memory freed
    uint32_t c_mapRenderTime;       // total amount of time it took to render the tilemap
    uint32_t c_numPints;            // total amount of pints rendered
    uint32_t c_numQuads;            // total amount of quads rendered
    uint32_t c_drawVerts;           // total amount of vertices used, only matters when r_enableBuffers == 1
} framePerformance_t;

typedef struct
{
    uint32_t *indices;

    framePerformance_t frameStats;
    vertexCache_t *pintCache;

    const nmap_t *currentMap;
} frameData_t;

static frameData_t frame;
static qboolean console_open = qfalse;

extern "C" void RE_SetDefaultState(void)
{
    nglEnable(GL_DEPTH_TEST);
    nglEnable(GL_STENCIL_TEST);
    nglEnable(GL_TEXTURE_2D);
    nglEnable(GL_BLEND);
    
    nglDisable(GL_CULL_FACE);

    nglBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    nglDepthMask(GL_FALSE);
    nglDepthFunc(GL_LESS);
}

#define MAX_CMD_LINE 1024

extern "C" void RE_CommandConsoleFrame(void)
{
    if (console_open) {
        R_UnbindShader();
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }
}

extern "C" void R_UpdateEvents(void)
{
    if (ri.Key_IsDown(KEY_UP))
        RB_MoveUp();
    if (ri.Key_IsDown(KEY_DOWN))
        RB_MoveDown();
    if (ri.Key_IsDown(KEY_RIGHT))
        RB_MoveRight();
    if (ri.Key_IsDown(KEY_LEFT))
        RB_MoveLeft();
    if (ri.Key_IsDown(KEY_M))
        RB_ZoomOut();
    if (ri.Key_IsDown(KEY_N))
        RB_ZoomIn();
}

extern "C" void RE_ProcessConsoleEvents(SDL_Event *event)
{
    if (!console_open)
        return;
    
    ImGui_ImplSDL2_ProcessEvent(event);
}

extern "C" void R_BeginImGui(void)
{
    ImGui_ImplSDL2_NewFrame();
    ImGui_ImplOpenGL3_NewFrame();
    ImGui::NewFrame();
}

extern "C" qboolean RE_ConsoleIsOpen(void)
{
    return console_open;
}

static void R_FrameStats_f(void)
{
    const framePerformance_t *p;
    
    p = &frame.frameStats;
    Con_Printf("%8i static frame bytes allocated", p->c_alloc);
    Con_Printf("%8i static frame bytes freed", p->c_free);
    Con_Printf("%8i calls to temp frame allocator", p->c_frameMemCalls);
    Con_Printf("%8i temp frame bytes used", p->c_frameMemUsed);
    Con_Printf("%8i temp frame bytes allocated", p->c_frameMemAllocated);
    Con_Printf("%8i temp frame bytes freed", p->c_frameMemFreed);
    Con_Printf("%8i total drawVert_t vertices used", p->c_drawVerts);
    Con_Printf("%8i total quads rendered", p->c_numQuads);
    Con_Printf("%8i tilemap rendering time (in ms)", p->c_mapRenderTime);
}

#define FRAME_QUADS 0x2000

extern "C" void RE_InitFrameData(void)
{
    uint32_t i, offset;

    // if we're reinitializing, reallocate
    if (frame.indices)
        ri.Z_Free(frame.indices);
    if (backend.frameCache)
        R_ShutdownCache(backend.frameCache);
    if (backend.frameShader)
        R_ShutdownShader(backend.frameShader);

    frame.indices = (uint32_t *)ri.Z_Malloc(sizeof(uint32_t) * (FRAME_QUADS + 128) * 6, TAG_RENDERER, &frame.indices, "GLindices");
    offset = 0;

    for (i = 0; i < (FRAME_QUADS * 6); i += 6) {
        frame.indices[i + 0] = offset + 0;
        frame.indices[i + 1] = offset + 1;
        frame.indices[i + 2] = offset + 2;

        frame.indices[i + 3] = offset + 2;
        frame.indices[i + 4] = offset + 3;
        frame.indices[i + 5] = offset + 0;

        offset += 4;
    }

    backend.indices = frame.indices;
    backend.usedIndices = 0;
    backend.usedVertices = 0;
    backend.numIndices = FRAME_QUADS * 6;
    backend.numVertices = FRAME_QUADS * 4;

    backend.frameShader = R_InitShader("pint.glsl.vert", "pint.glsl.frag");
    backend.frameCache = R_InitFrameCache();

    backend.frameCache->indices =  frame.indices;
    backend.frameCache->numIndices = backend.numIndices;
    
    ri.Cmd_AddCommand("framestats", R_FrameStats_f);
}

extern "C" void RE_ToggleConsole(void)
{
    if (console_open)
        console_open = qfalse;
    else
        console_open = qtrue;
}

extern "C" void RE_BeginFrame(void)
{
    nglClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    nglViewport(0, 0, r_screenwidth->i, r_screenheight->i);
    nglClearColor(0.1f, 0.1f, 0.1f, 1.0f);

    RE_BeginFramebuffer();
    R_UpdateEvents();
    RE_SetDefaultState();
    R_BeginImGui();
    R_InitFrameMemory();

    if (r_enableBuffers->i) {
        R_ReserveFrameMemory(backend.frameCache, backend.numVertices + 64, backend.numIndices + 64);
        backend.vertices = backend.frameCache->vertices;
        backend.indices = backend.frameCache->indices;
    }
    backend.commandList.usedBytes = 0;

    RB_MakeViewMatrix();

    rg.mapData = ri.G_GetCurrentMap();
    R_BindShader(backend.frameShader);
    R_SetMatrix4(backend.frameShader, "u_ViewProjection", rg.camera.vpm);
}

extern "C" void RE_EndFrame(void)
{
    RE_RenderTilemap();

    // flush it if there's anything in there
    if (r_enableBuffers->i) {
        R_DrawCache(backend.frameCache);
    }

    R_UnbindShader();

    // flush the command buffer
    RE_IssueRenderCommands();
    RE_EndFramebuffer();

    RE_CommandConsoleFrame();

    ri.SDL_GL_SwapWindow(rg.window);
}

typedef struct frameAlloc_s
{
    struct frameAlloc_s *next;
    uint32_t size;
    uint32_t used;
    uint32_t padding;
    byte base[4];
} frameAlloc_t;

typedef struct
{
    frameAlloc_t *memory;
    frameAlloc_t *alloc;

    uint32_t memoryHighwater;
} frameMemoryData_t;

static frameMemoryData_t frameMem;

#define MEMORY_BLOCK_SIZE 0x1000000
#if 1
#define FrameAlloc(size) ri.Z_Malloc((size),TAG_RENDERER,NULL,"")
#define FrameFree(p) ri.Z_Free(p)
#else
#define FrameAlloc(size) ri.Hunk_AllocateTempMemory((size))
#define FrameFree(p) ri.Hunk_FreeTempMemory((p))
#endif

extern "C" void R_ShutdownFrameMemory(void)
{
    frameAlloc_t *alloc, *next;
    frameMemoryData_t *mem;

    mem = &frameMem;

    //
    // free any current data
    //
    
    for (alloc = mem->memory; alloc; alloc = next) {
        next = alloc->next;
        frame.frameStats.c_frameMemFreed += alloc->size;
        FrameFree(alloc);
    }
}

extern "C" void R_InitFrameMemory(void)
{
    frameAlloc_t *alloc;

    R_ShutdownFrameMemory();

    alloc = (frameAlloc_t *)FrameAlloc(MEMORY_BLOCK_SIZE + sizeof(*alloc));
    if (!alloc) { 
        ri.N_Error("R_InitFrameMemory: failed to allocated temp frame memory");
    }
    frame.frameStats.c_frameMemAllocated += MEMORY_BLOCK_SIZE;

    alloc->size = MEMORY_BLOCK_SIZE;
    alloc->used = 0;
    alloc->next = NULL;

    frameMem.memory = alloc;
    frameMem.alloc = alloc;
    frameMem.memoryHighwater = 0;
}

extern "C" void *R_FrameAlloc(uint32_t size)
{
    frameAlloc_t *alloc;
    void *buf;

    size = PAD(size, 16);

    // see if it can be satisfied in the current block
    alloc = frameMem.alloc;
    if (alloc->used + size <= alloc->size) {
        buf = alloc->base + alloc->used;
        alloc->used += size;
        frame.frameStats.c_frameMemUsed += size;
        return buf;
    }

    // check the other blocks for available space
    alloc = alloc->next;
    
    // we're at the end of the chain, allocate a new block
    if (!alloc) {
        uint32_t blocksize;

        blocksize = MEMORY_BLOCK_SIZE;
        alloc = (frameAlloc_t *)FrameAlloc(blocksize + sizeof(*alloc));
        if (!alloc) {
            ri.N_Error("R_FrameAlloc: failed to allocate temp frame memory");
        }
        alloc->size = blocksize;
        alloc->used = 0;
        alloc->next = NULL;
        frameMem.alloc->next = alloc;
        frame.frameStats.c_frameMemAllocated += MEMORY_BLOCK_SIZE;
    }

    if (size > alloc->size) {
        ri.N_Error("R_FrameAlloc: overflow of %i bytes", size);
    }

    frameMem.alloc = alloc;
    alloc->used = size;
    frame.frameStats.c_frameMemUsed += size;

    return alloc->base;
}
