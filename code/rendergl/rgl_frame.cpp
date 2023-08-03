#include "rgl_local.h"
#include "../src/g_bff.h"
#include "r_framevector.h"

typedef struct {
	uint64_t c_alloc, c_free; // total bytes allocated/freed from R_StaticAlloc and R_StaticFree
    uint64_t c_frameMemCalls; // total calls to R_FrameAlloc
    uint64_t c_frameMemUsed;
    uint64_t c_mapRenderTime; // total amount of time it took to render the tilemap
    uint64_t c_numPints; // total amount of pints rendered
    uint64_t c_numQuads; // total amount of quads rendered
    uint64_t c_drawVerts;
} framePerformance_t;

typedef struct
{
    const uint32_t maxQuads = RENDER_MAX_QUADS;
    const uint32_t maxVertices = RENDER_MAX_VERTICES;
    const uint32_t maxIndices = RENDER_MAX_INDICES;

    uint32_t *indices;

    framePerformance_t frameStats;

//    emptyCmd_t *cmd_head, *cmd_tail;
    vertexCache_t *pintCache;

    const nmap_t *currentMap;
} frameData_t;

static frameData_t frame;

#if 0
static void* R_AllocCommandBuffer(uint32_t size)
{
    emptyCmd_t *cmd;

    cmd = (emptyCmd_t *)R_FrameAlloc(size);
    cmd->next = NULL;
    frame.cmd_tail->next = &cmd->cmdId;
    frame.cmd_tail = cmd;

    return (void *)cmd;
}

static void R_ClearCommands(void)
{
    frame.cmd_head = frame.cmd_tail = (emptyCmd_t *)R_FrameAlloc(sizeof(*frame.cmd_head));
    frame.cmd_head->cmdId = RC_NOP;
    frame.cmd_head->next = NULL;
}

static void R_PushDrawVertsCmd(vertex_t *verts, uint64_t nVerts)
{
    drawVertsCmd_t *cmd;

    cmd = (drawVertsCmd_t *)R_AllocCommandBuffer(sizeof(*cmd));
    cmd->cmdId = RC_DRAW_VERTS;
    cmd->verts = verts;

    frame.frameStats.c_drawVerts++;
}

static void R_ExecuteCommands(void)
{
    if (frame.cmd_head->cmdId == RC_NOP && !frame.cmd_head->next) {
        // nothing to execute
        return;
    }

    R_BindCache(frame.pintCache);
    for (emptyCmd_t *cmd = frame.cmd_head;; cmd = cmd->next) {
        drawVerts_t *v = ((drawVertsCmd_t *)cmd)->verts;

        nglBufferSubDataARB(GL_ARRAY_BUFFER_ARB, 0, sizeof(vertex_t) * v->verts.size() * 4, v->verts.data());
        nglBufferSubDataARB(GL_ELEMENT_ARRAY_BUFFER_ARB, 0, sizeof(uint32_t) * v->verts.size() * 6, frame.indices);

        nglDrawElements(GL_TRIANGLES, v->verts.size() * 6, GL_UNSIGNED_INT, NULL);
        
        if (cmd == frame.cmd_tail)
            break;
    }
    R_UnbindCache();

    R_ClearCommands();
}
#endif

static qboolean console_open;

GO_AWAY_MANGLE void RE_SetDefaultState(void)
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

GO_AWAY_MANGLE void RE_RenderMap(void);

GO_AWAY_MANGLE void RE_SubmitMapTilesheet(const char *chunkname, const bffinfo_t *info)
{
    const uint64_t hash = Com_GenerateHashValue(chunkname, MAX_TEXTURE_CHUNKS);
    
    // does it exist?
    if (!info->textures[hash].fileBuffer)
        ri.N_Error("RE_SubmitMapTileset: invalid texture chunk");
    
    // check if its already been loaded
    if (!R_GetTexture(chunkname))
        R_InitTexture(&info->textures[hash]);
}

#define MAX_CMD_LINE 1024
#define MAX_CMD_BUFFER 8192

GO_AWAY_MANGLE void R_ConsoleGetInput(void)
{
}

GO_AWAY_MANGLE void RE_CommandConsoleFrame(void)
{
    if (console_open) {
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }
}

GO_AWAY_MANGLE void R_UpdateState(void)
{
    const qboolean *keys = ri.Com_GetKeyboard();
    const uint32_t window = ri.Com_GetWindowEvents();

    if (keys[KEY_BACKQUOTE]) {
        if (console_open)
            console_open = qfalse;
        else
            console_open = qtrue;
    }
    if (window & SDL_WINDOWEVENT_RESIZED) {
        // redo the settings if the window has been resized
        RE_InitSettings_f();
    }
    if (keys[KEY_UP])
        renderer->camera.MoveUp();
    if (keys[KEY_DOWN])
        renderer->camera.MoveDown();
    if (keys[KEY_LEFT])
        renderer->camera.MoveLeft();
    if (keys[KEY_RIGHT])
        renderer->camera.MoveRight();
    if (keys[KEY_M])
        renderer->camera.ZoomIn();
    if (keys[KEY_N])
        renderer->camera.ZoomOut();
}

GO_AWAY_MANGLE void RE_ProcessConsoleEvents(SDL_Event *events)
{
    if (!console_open)
        return;
    
    ImGui_ImplSDL2_ProcessEvent(events);
}

GO_AWAY_MANGLE void R_BeginImGui(void)
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();
}

GO_AWAY_MANGLE qboolean RE_ConsoleIsOpen(void)
{
    return console_open;
}

#define FRAME_QUADS 0x20000

GO_AWAY_MANGLE void RE_BeginFrame(void)
{
    EASY_FUNCTION();

    nglClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    nglViewport(0, 0, r_screenwidth->i, r_screenheight->i);
    nglClearColor(0.1f, 0.1f, 0.1f, 1.0f);

//    RE_BeginFramebuffer();
    R_UpdateState();
    R_BeginImGui();

    R_InitFrameMemory();
    frame.currentMap = ri.G_GetCurrentMap();
    
    R_ReserveFrameMemory(frame.pintCache, FRAME_QUADS, FRAME_QUADS);
//    RE_SetDefaultState();

    renderer->camera.CalculateViewMatrix();

    R_BindShader(pintShader);
    R_SetMatrix4(pintShader, "u_ViewProjection", renderer->camera.GetVPM());
//    R_SetBool(pintShader, "u_UseGamma", (bool)r_gammaAmount->f);
//    R_SetFloat(pintShader, "u_Gamma", r_gammaAmount->f);
}

GO_AWAY_MANGLE void RE_EndFrame(void)
{
    EASY_FUNCTION();

    RE_RenderMap();
    RE_CommandConsoleFrame();

    // flush it if there's anything in there
    R_DrawCache(frame.pintCache);

    R_BindTexture(R_GetTexture(frame.currentMap->tilesetName));
    nglBegin(GL_TRIANGLES);
    nglVertex2f( 0.5f,  0.5f);
    nglVertex2f( 0.5f, -0.5f);
    nglVertex2f(-0.5f, -0.5f);
    nglVertex2f(-0.5f,  0.5f);
    nglVertex2f(-0.5f, -0.5f);
    nglVertex2f(-0.5f,  0.5f);
    nglVertex2f( 0.5f,  0.5f);
    nglEnd();
    R_UnbindTexture();

    R_UnbindShader();
    
//    RE_EndFramebuffer();

    ri.SDL_GL_SwapWindow(renderer->window);
}


GO_AWAY_MANGLE void RE_InitFrameData(void)
{
    frame.indices = (uint32_t *)ri.Hunk_Alloc(sizeof(uint32_t) * FRAME_QUADS * 6, "frameIndices", h_low);
    uint32_t offset = 0;
    for (uint32_t i = 0; i < (FRAME_QUADS * 6); i += 6) {
        frame.indices[i + 0] = offset + 0;
        frame.indices[i + 1] = offset + 1;
        frame.indices[i + 2] = offset + 2;

        frame.indices[i + 3] = offset + 2;
        frame.indices[i + 4] = offset + 3;
        frame.indices[i + 5] = offset + 0;

        offset += 4;
    }

    frame.pintCache = R_InitFrameCache();
}

/*
RE_SubmitPint: frameVerts should always be stack-based
*/
GO_AWAY_MANGLE void RE_SubmitPint(const glm::vec2& pos, const glm::vec2& dims, uint32_t gid, vertex_t *frameVerts)
{
    glm::mat4 model, mvp;
    vertex_t *vert;
    const glm::vec2* coords = ri.Map_GetSpriteCoords(gid);
    if (!coords) { // oh no
        Con_Printf(ERROR, "bad GID (%i)", gid);
        return;
    }

    constexpr glm::vec4 positions[] = {
        glm::vec4( 0.5f,  0.5f, 0.0f, 1.0f),
        glm::vec4( 0.5f, -0.5f, 0.0f, 1.0f),
        glm::vec4(-0.5f, -0.5f, 0.0f, 1.0f),
        glm::vec4(-0.5f,  0.5f, 0.0f, 1.0f),
        glm::vec4(-0.5f, -0.5f, 0.0f, 1.0f), // extra
        glm::vec4(-0.5f,  0.5f, 0.0f, 1.0f), // extra
        glm::vec4( 0.5f,  0.5f, 0.0f, 1.0f) // extra
    };

    model = glm::translate(glm::mat4(1.0f), glm::vec3(pos.x - (dims.x * 0.5f), dims.y - pos.y, 0.0f));
//        * glm::scale(glm::mat(1.0f), glm::vec3(sheet->getTileWidth(), sheet->getTileHeight(), 0.0f));
    mvp = renderer->camera.GetVPM() * model;

    vert = frameVerts;
    for (uint32_t i = 0; i < 6; ++i) {
        vert->pos = mvp * positions[i];
        vert->texcoords = coords[i];
        vert->color = glm::vec4(0.0f);
        vert++;
    }
}

GO_AWAY_MANGLE void RE_RenderMap(void)
{
    EASY_FUNCTION();

    vertex_t pintVerts[6];
    uint32_t numVerts;
    const uint32_t maxVerts = (FRAME_QUADS / 2) * 4;
    const uint32_t maxIndices = (FRAME_QUADS / 2) * 6;
    uint64_t renderStartTime, renderEndTime;

    // initialize and allocate the temp frame data
    numVerts = 0;

    // begin the profiling
    renderStartTime = clock();
    uint32_t gid = 0;
    R_SetInt(pintShader, "u_Texture", 0);
    R_BindTexture(R_GetTexture(frame.currentMap->tilesetName));
    nglBegin(GL_TRIANGLES);
    for (uint64_t y = 0; y < frame.currentMap->mapHeight; y++) {
        for (uint64_t x = 0; x < frame.currentMap->mapWidth; x++) {
            gid = frame.currentMap->tilemapData[y * frame.currentMap->mapWidth + x][0];
            RE_SubmitPint({ x, y }, { frame.currentMap->tileWidth, frame.currentMap->tileHeight }, gid, pintVerts);

            for (uint32_t i = 0; i < arraylen(pintVerts); i++) {
                nglVertex2f(pintVerts[i].pos.x, pintVerts[i].pos.y);
                nglTexCoord2f(pintVerts[i].texcoords.x, pintVerts[i].texcoords.y);
            }

            if (numVerts + 4 >= (FRAME_QUADS / 2) * 4) {
//                R_DrawCache(frame.pintCache);
//                numVerts = 0;
            }

//            R_PushVertices(frame.pintCache, pintVerts, arraylen(pintVerts));
            numVerts += 4;
        }
    }
    nglEnd();
    R_UnbindTexture();

//    R_PushVertices(frame.pintCache, pintVerts, numVerts);
//    R_PushIndices(frame.pintCache, frame.indices, maxIndices);
//    R_DrawCache(frame.pintCache);

    renderEndTime = clock();

    frame.frameStats.c_mapRenderTime = renderEndTime - renderStartTime;

    memset(&frame.frameStats, 0, sizeof(frame.frameStats));
}

// a request for frame memory will never fail
// (until malloc fails), but it may force the
// allocation of a new memory block that will
// be discontinuous with the existing memory
typedef struct frameMemoryBlock_s {
	struct frameMemoryBlock_s *next;
	uint32_t size;
	uint32_t used;
	uint32_t poop;			// so that base is 16 byte aligned
	byte base[4];	// dynamically allocated as [size]
} frameMemoryBlock_t;

// all of the information needed by the back end must be
// contained in a frameData_t.  This entire structure is
// duplicated so the front and back end can run in parallel
// on an SMP machine (OBSOLETE: this capability has been removed)
typedef struct {
	// one or more blocks of memory for all frame
	// temporary allocations
	frameMemoryBlock_t	*memory;

	// alloc will point somewhere into the memory chain
	frameMemoryBlock_t	*alloc;

	uint64_t memoryHighwater;	// max used on any frame
} frameMemory_t;

frameMemory_t *frameData;

#if 1
#define	MEMORY_BLOCK_SIZE   0x9000000
#else
#define MEMORY_BLOCK_SIZE   0x1000
#endif

/*
R_CountFrameMemory
*/
GO_AWAY_MANGLE uint64_t R_CountFrameMemory(void)
{
	frameMemory_t *frameMem;
	frameMemoryBlock_t *block;
	uint64_t count;

	count = 0;
	frameMem = frameData;
	for ( block = frameMem->memory ; block ; block=block->next ) {
		count += block->used;
		if ( block == frameMem->alloc ) {
			break;
		}
	}

	// note if this is a new highwater mark
	if ( count > frameMem->memoryHighwater )
		frameMem->memoryHighwater = count;

	return count;
}

GO_AWAY_MANGLE void R_ToggleFrame(void)
{
    // clear frame-temporary data
	frameMemory_t *frameMem;
	frameMemoryBlock_t *block;

	// update the highwater mark
	R_CountFrameMemory();

	frameMem = frameData;

	// reset the memory allocation to the first block
	frameMem->alloc = frameMem->memory;

	// clear all the blocks
	for (block = frameMem->memory; block; block = block->next)
		block->used = 0;
}

/*
R_ShutdownFrameData
*/
GO_AWAY_MANGLE void R_ShutdownFrameMemory(void)
{
	frameMemory_t *frameMem;
	frameMemoryBlock_t *block;

	// free any current data
	frameMem = frameData;
	if (!frameMem)
		return;

	frameMemoryBlock_t *nextBlock;
	for (block = frameMem->memory; block; block = nextBlock) {
		nextBlock = block->next;
#if 1
		ri.Mem_Free(block);
#else
        ri.Z_ChangeTag(frameMem, TAG_PURGELEVEL);
#endif
	}
#if 1
	ri.Mem_Free(frameMem);
#else
    ri.Z_ChangeTag(frameMem, TAG_PURGELEVEL);
#endif
	frameData = NULL;
}

/*
R_InitFrameData
*/
GO_AWAY_MANGLE void R_InitFrameMemory(void)
{
	uint64_t size;
	frameMemory_t *frameMem;
	frameMemoryBlock_t *block;

	R_ShutdownFrameMemory();

#if 1
	frameData = (frameMemory_t *)memset(ri.Mem_Alloc(sizeof( *frameData )), 0, sizeof(*frameData));
	frameMem = frameData;
	size = MEMORY_BLOCK_SIZE;
	block = (frameMemoryBlock_t *)ri.Mem_Alloc( size + sizeof( *block ) );
	if ( !block )
		ri.N_Error( "R_InitFrameMemory: Mem_Alloc() failed" );
#else
    frameData = (frameMemory_t *)ri.Z_Calloc(sizeof(*frameData), TAG_RENDERER, &frameData, "frameData");
    frameMem = frameData;
    size = MEMORY_BLOCK_SIZE;
    block = (frameMemoryBlock_t *)ri.Z_Malloc(size + sizeof(*block), TAG_RENDERER, &block, "frameBlock");
#endif

	block->size = size;
	block->used = 0;
	block->next = NULL;
	frameMem->memory = block;
	frameMem->memoryHighwater = 0;

	R_ToggleFrame();
}

/*
R_StaticAlloc
*/
GO_AWAY_MANGLE void *R_StaticAlloc(uint32_t size)
{
	void *buf;

    frame.frameStats.c_alloc += size;

	buf = ri.Mem_Alloc(size);

	// don't exit on failure on zero length allocations since the old code didn't
	if ( !buf && ( size != 0 ) ) {
		ri.N_Error( "R_StaticAlloc failed on %i bytes", size );
	}
	return buf;
}

/*
R_ClearedStaticAlloc
*/
GO_AWAY_MANGLE void *R_ClearedStaticAlloc(uint32_t size)
{
	return memset(R_StaticAlloc(size), 0, size);
}

/*
R_StaticFree
*/
GO_AWAY_MANGLE void R_StaticFree(void *p)
{
    frame.frameStats.c_free += ri.Mem_Msize(p);
	ri.Mem_Free(p);
}

/*
R_FrameAlloc:

This data will be automatically freed when the current frame's back end completes.

This should only be called by the front end. The back end shouldn't need to allocate memory.

If we passed smpFrame in, the back end could alloc memory, because it will always
be a different frameData than the front end is using.

All temporary data, like dynamic tesselations and local spaces are allocated here.

The memory will not move, but it may not be contiguous with previous allocations even from this frame.

The memory is NOT zero filled. Should part of this be inlined in a macro?
*/
GO_AWAY_MANGLE void *R_FrameAlloc(uint32_t size)
{
	frameMemory_t *frameMem;
	frameMemoryBlock_t *block;
	void *buf;

	size = (size+16)&~15;

	// see if it can be satisfied in the current block
	frameMem = frameData;
	block = frameMem->alloc;

	if ( block->size - block->used >= size ) {
		buf = block->base + block->used;
		block->used += size;
		return buf;
	}

	// advance to the next memory block if available
	block = block->next;

	// create a new block if we are at the end of the chain
	if ( !block ) {
		uint32_t size;

		size = MEMORY_BLOCK_SIZE;
#if 1
		block = (frameMemoryBlock_t *)ri.Mem_Alloc( size + sizeof( *block ) );
        if ( !block ) {
			ri.N_Error( "R_FrameAlloc: Mem_Alloc() failed" );
		}
#else
        block = (frameMemoryBlock_t *)ri.Z_Malloc(size + sizeof(*block), TAG_STATIC, &block, "frameBlock");
#endif
		block->size = size;
		block->used = 0;
		block->next = NULL;
		frameMem->alloc->next = block;
	}

	// we could fix this if we needed to...
	if ( size > block->size )
		ri.N_Error( "R_FrameAlloc of %i exceeded MEMORY_BLOCK_SIZE", size);

	frameMem->alloc = block;
	block->used = size;

    frame.frameStats.c_frameMemCalls++;
    frame.frameStats.c_frameMemUsed++;

	return block->base;
}

/*
R_ClearedFrameAlloc: basically R_FrameAlloc but initializes the memory to 0
*/
GO_AWAY_MANGLE void *R_ClearedFrameAlloc(uint32_t size)
{
    return memset(R_FrameAlloc(size), 0, size);
}