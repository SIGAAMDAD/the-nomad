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

    const GDRMap *currentMap;
    const GDRMapLayer *currentLayer;
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
    EASY_FUNCTION();

    if (!console_open)
        return; // nothing to process
    
    char buffer[MAX_CMD_BUFFER];
    memset(buffer, 0, sizeof(buffer));
    
    ImGui::Text("> ");
    ImGui::SameLine();
    if (ImGui::InputText(" ", buffer, MAX_CMD_BUFFER, ImGuiInputTextFlags_EnterReturnsTrue)) {
        ri.Con_Printf(INFO, "]%s", buffer); // echo it into the console
        if (*buffer == '/') { // its a command
            ri.Cmd_ExecuteText(buffer);
        }
    }
}

GO_AWAY_MANGLE void RE_CommandConsoleFrame(void)
{
    int flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize;

    ImGui::SetWindowPos(ImVec2(0.0f, 0.0f));
    ImGui::SetWindowSize(ImVec2((float)r_screenwidth->i, (float)(r_screenheight->i / 2)));
    ImGui::Begin("Command Console", NULL, flags);
    Con_GetBuffer().emplace_back('\0');
    ImGui::Text("%s", Con_GetBuffer().data());
    Con_GetBuffer().clear();

    R_ConsoleGetInput();

    ImGui::End();
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
}

GO_AWAY_MANGLE void R_BeginImGui(void)
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();
}

qboolean RE_ConsoleIsOpen(void)
{
    return console_open;
}

#define FRAME_QUADS 0x2000

GO_AWAY_MANGLE void RE_BeginFrame(void)
{
    EASY_FUNCTION();

    nglClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    nglViewport(0, 0, r_screenwidth->i, r_screenheight->i);
    nglClearColor(0.1f, 0.1f, 0.1f, 0.0f);

    RE_BeginFramebuffer();
    R_UpdateState();
    R_BeginImGui();

    R_InitFrameMemory();
    if (!ri.G_GetCurrentMap()->getTilesets()[0].getSpriteData())
        ri.N_Error("DEREFERENCED!");
    
    frame.currentMap = ri.G_GetCurrentMap();

    if (!frame.currentMap->getTilesets()[0].getSpriteData())
        ri.N_Error("DEREFERENCED!");
    
    R_ReserveFrameMemory(frame.pintCache);
    if (!frame.currentMap->getTilesets()[0].getSpriteData())
        ri.N_Error("DEREFERENCED!");

    RE_SetDefaultState();
    if (!frame.currentMap->getTilesets()[0].getSpriteData())
        ri.N_Error("DEREFERENCED!");

    renderer->camera.CalculateViewMatrix();
    if (!frame.currentMap->getTilesets()[0].getSpriteData())
        ri.N_Error("DEREFERENCED!");

    nglClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    nglViewport(0, 0, r_screenwidth->i, r_screenheight->i);
}

GO_AWAY_MANGLE void RE_EndFrame(void)
{
    EASY_FUNCTION();

    if (!frame.currentMap->getTilesets()[0].getSpriteData())
        ri.N_Error("DEREFERENCED!");

    RE_RenderMap();
    RE_CommandConsoleFrame();

    // flush it if there's anything in there
    R_DrawCache(frame.pintCache);
    
    RE_EndFramebuffer();

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
GO_AWAY_MANGLE void RE_SubmitPint(const glm::vec2& pos, const glm::vec2& dims, uint32_t gid, const GDRTileSheet *sheet,
    vertex_t *frameVerts)
{
    glm::mat4 model, mvp;
    vertex_t *vert;
    const glm::vec2* coords = sheet->getSpriteCoords(gid);
    constexpr glm::vec4 positions[] = {
        glm::vec4( 0.5f,  0.5f, 0.0f, 1.0f),
        glm::vec4( 0.5f, -0.5f, 0.0f, 1.0f),
        glm::vec4(-0.5f, -0.5f, 0.0f, 1.0f),
        glm::vec4(-0.5f,  0.5f, 0.0f, 1.0f)
    };

    model = glm::translate(glm::mat4(1.0f), glm::vec3(pos.x - (dims.x * 0.5f), dims.y - pos.y, 0.0f));
//        * glm::scale(glm::mat(1.0f), glm::vec3(sheet->getTileWidth(), sheet->getTileHeight(), 0.0f));
    mvp = renderer->camera.GetVPM() * model;

    vert = frameVerts;
    for (uint32_t i = 0; i < 4; ++i) {
        vert->pos = mvp * positions[i];
        vert->texcoords = coords[i];
        vert->color = glm::vec4(0.0f);
        vert++;
    }
}

GO_AWAY_MANGLE void R_RenderImageLayer(const GDRImageLayer *layer)
{
    if (!frame.currentMap->getTilesets()[0].getSpriteData())
        ri.N_Error("DEREFERENCED!");
    
    const texture_t *tex = R_GetTexture(layer->getImage());

    R_BindTexture(tex);

#if 0
#else
    // using immediate mode until it becomes necessary to use a vbo to render the stuff (more than one fullscreen texture per frame)
    nglBegin(GL_TRIANGLE_FAN);

    nglTexCoord2f(0.0f, 0.0f);
    nglVertex2f(1.0f,  1.0f);

    nglTexCoord2f(0.0f, 1.0f);
    nglVertex2f(1.0f, -1.0f);
    
    nglTexCoord2f(1.0f, 1.0f);
    nglVertex2f(-1.0f, -1.0f);
    
    nglTexCoord2f(1.0f, 0.0f);
    nglVertex2f(-1.0f,  1.0f);
    
    nglEnd();
#endif

    R_UnbindTexture();
}

GO_AWAY_MANGLE void R_RenderTileLayer(const GDRMapLayer *layer)
{
    const GDRTile *tile;
    vertex_t *pintVertices, *vertPtr;
    uint32_t numVerts;
    const uint32_t maxVerts = FRAME_QUADS * 4;
    const GDRTileLayer *tileLayer;

    if (!frame.currentMap->getTilesets()[0].getSpriteData())
        ri.N_Error("DEREFERENCED!");

    numVerts = 0;
    pintVertices = (vertex_t *)R_FrameAlloc(sizeof(vertex_t) * maxVerts);
    vertPtr = pintVertices;
    tileLayer = (const GDRTileLayer *)layer->data();

    for (uint64_t y = 0; y < layer->getHeight(); ++y) {
        for (uint64_t x = 0; x < layer->getWidth(); ++x) {
            tile = &tileLayer->getTiles()[y * layer->getWidth() + x];

            // invalid tile, skip it
            if (tile->tilesetIndex >= frame.currentMap->numTilesets())
                continue;
            
            Con_Printf(DEBUG, "tilesetIndex: %li", tile->tilesetIndex);
            Con_Printf(DEBUG, "tileset.m_name: %s", frame.currentMap->getTilesets()[tile->tilesetIndex].getName());
            RE_SubmitPint({ x, y }, { layer->getWidth(), layer->getHeight() }, tile->gid,
                frame.currentMap->getTilesets()[tile->tilesetIndex].getSpriteData(), vertPtr);
            
            if (numVerts + 4 >= maxVerts) {
                R_PushVertices(frame.pintCache, pintVertices, numVerts);
                vertPtr = pintVertices;
                numVerts = 0;
            }
            numVerts += 4;
        }
    }
    // submit it
    R_DrawCache(frame.pintCache);
}

GO_AWAY_MANGLE void R_LayerIterate(const GDRGroupLayer *layer)
{
    if (!frame.currentMap->getTilesets()[0].getSpriteData())
        ri.N_Error("DEREFERENCED!");

    const GDRMapLayer *layerBase = layer->getChildren();

    for (uint64_t i = 0; i < layer->getNumChildren(); ++i) {
        switch (layerBase->getType()) {
//        case MAP_LAYER_OBJECT: {
//            const GDRObjectGroup *object = dynamic_cast<const GDRObjectGroup *>(i);
//            R_RenderLayer(object);
//            break; }
        case MAP_LAYER_IMAGE: {
            const GDRImageLayer *image = (const GDRImageLayer *)layerBase->data();
            R_RenderImageLayer(image);
            break; }
        case MAP_LAYER_TILE: {
            R_RenderTileLayer(layerBase);
            break; }
        case MAP_LAYER_GROUP: {
            const GDRGroupLayer *group = (const GDRGroupLayer *)layerBase->data();
            R_LayerIterate(group); // hopefully this doesn't end up in an infinite loop
            break; }
        };
        layerBase++;
    }
}

GO_AWAY_MANGLE void RE_RenderMap(void)
{
    EASY_FUNCTION();

    if (!frame.currentMap->getTilesets()[0].getSpriteData())
        ri.N_Error("DEREFERENCED!");

    uint64_t renderStartTime, renderEndTime;
    const GDRTileset *tileset = frame.currentMap->getTilesets();
    const GDRMapLayer *layerBase = frame.currentMap->getLayers();

    // begin the profiling
    renderStartTime = clock();
    for (uint64_t i = 0; i < frame.currentMap->numLayers(); ++i) {
        switch (layerBase->getType()) {
        case MAP_LAYER_TILE: {
            R_RenderTileLayer(layerBase);
            break; }
        case MAP_LAYER_GROUP: {
            const GDRGroupLayer *layer = (const GDRGroupLayer *)layerBase->data();
            R_LayerIterate(layer);
            break; }
        case MAP_LAYER_IMAGE: {
            const GDRImageLayer *layer = (const GDRImageLayer *)layerBase->data();
            R_RenderImageLayer(layer);
            break; }
//        case MAP_LAYER_OBJECT: {
//            const GDRObjectGroup *layer = dynamic_cast<GDRObjectGroup *>(layerBase);
//            R_RenderLayer(eastl::dynamic_pointer_cast<GDRObjectGroup>(layerBase), mapData);
//            break; }
        };
        layerBase++;
    }
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
#define	MEMORY_BLOCK_SIZE	0x600000
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