#include "rgl_local.h"

static boost::shared_mutex cmdLock;
static boost::mutex allocLock;

extern "C" void *R_GetCommandBufferReserved(uint32_t size, uint32_t reserved)
{
    boost::unique_lock<boost::mutex> lock{allocLock};
    renderCommandList_t *list;

    list = &backend.commandList;
    size = PAD(size, sizeof(void *));

    // always leave room for the end of the list command
    if (list->usedBytes + size + sizeof(renderCmdType_t) + reserved > MAX_RC_BUFFER) {
        if (size > MAX_RC_BUFFER - sizeof(renderCmdType_t)) {
            ri.N_Error("R_GetCommandBuffer: bad cmd size: %i", size);
        }
        // drop commands when out of room
        return NULL;
    }

    list->usedBytes += size;
    return list->buffer + list->usedBytes - size;
}

extern "C" void *R_GetCommandBuffer(uint32_t size)
{
    return R_GetCommandBufferReserved(size, PAD(sizeof(renderCmdType_t), sizeof(void *)));
}

extern "C" void RE_DrawRect(renderRect_t *rect)
{
    drawRectCmd_t *cmd;

    cmd = (drawRectCmd_t *)R_GetCommandBuffer(sizeof(*cmd));
    if (!cmd)
        return;

    boost::unique_lock<boost::shared_mutex> lock{cmdLock};
    cmd->id = RC_DRAW_RECT;
    VectorCopy(cmd->pos, rect->pos);
    memcpy(cmd->color, rect->color, sizeof(vec4_t));
    cmd->rotation = rect->rotation;
    cmd->size = rect->size;
    cmd->filled = rect->filled;
    cmd->texture = rect->texture;
}

extern "C" void RE_SetColor(const float *color, uint32_t count)
{
    setColorCmd_t *cmd;
    uint32_t i;

    cmd = (setColorCmd_t *)R_GetCommandBuffer(sizeof(*cmd));
    if (!cmd)
        return;
    
    boost::unique_lock<boost::shared_mutex> lock{cmdLock};
    cmd->id = RC_SET_COLOR;
    memcpy(cmd->color, colorWhite, sizeof(cmd->color));
    if (color) {
        for (i = 0; i < count; ++i)
            cmd->color[i] = color[i];
    }
}

extern "C" void RE_AddTile(const drawVert_t *vertices)
{
    drawTileCmd_t *cmd;
    const uint32_t count = r_enableBuffers->i ? 4 : 6;

    cmd = (drawTileCmd_t *)R_GetCommandBuffer(sizeof(*cmd));
    if (!cmd)
        return;
    
    boost::unique_lock<boost::shared_mutex> lock{cmdLock};
    cmd->id = RC_DRAW_TILE;
#if 1
    for (uint32_t i = 0; i < count; ++i) {
        memcpy(cmd->vertices[i].pos, vertices[i].pos, sizeof(*vertices) * sizeof(vec3_t));
        memcpy(cmd->vertices[i].color, vertices[i].color, sizeof(*vertices) * sizeof(vec4_t));
        memcpy(cmd->vertices[i].light, vertices[i].light, sizeof(*vertices) * sizeof(vec2_t));
        memcpy(cmd->vertices[i].texcoords, vertices[i].texcoords, sizeof(*vertices) * sizeof(vec2_t));
    }
#else
    memcpy(cmd->vertices, vertices, sizeof(*vertices) * count);
#endif
}

extern "C" void RE_AddDrawEntity(renderEntityRef_t *ref)
{
    drawRefCmd_t *cmd;
    
    cmd = (drawRefCmd_t *)R_GetCommandBuffer(sizeof(*cmd));
    if (!cmd)
        return;
    
    boost::unique_lock<boost::shared_mutex> lock{cmdLock};
    cmd->id = RC_DRAW_REF;
    cmd->ref = ref;
}

extern "C" void RE_IssueRenderCommands(void)
{
    boost::unique_lock<boost::mutex> lock{allocLock};
    renderCommandList_t *cmdList;

    cmdList = &backend.commandList;
    *(renderCmdType_t *)(cmdList->buffer + cmdList->usedBytes) = RC_END_LIST;

    // clear the stack
    cmdList->usedBytes = 0;

    RB_ExecuteCommands();
}

extern "C" void RB_ExecuteCommands(void)
{
    renderCommandList_t *cmdList;
    const void *cmdBuf;

    cmdList = &backend.commandList;
    cmdBuf = cmdList->buffer;

    while (1) {
        cmdBuf = (const void *)PADP(cmdBuf, sizeof(void *));

        switch (*(const renderCmdType_t *)cmdBuf) {
        case RC_SET_COLOR: {
            const setColorCmd_t *cmd = (const setColorCmd_t *)cmdBuf;
            nglColor4fv(cmd->color);
            cmdBuf = (const void *)(cmd + 1);
            break; }
        case RC_DRAW_RECT: {
            const drawRectCmd_t *cmd = (const drawRectCmd_t *)cmdBuf;
            RB_DrawRect(cmd);
            cmdBuf = (const void *)(cmd + 1);
            break; }
        case RC_DRAW_REF: {
            const drawRefCmd_t *cmd = (const drawRefCmd_t *)cmdBuf;
            RB_DrawEntity(cmd);
            cmdBuf = (const void *)(cmd + 1);
            break; }
        case RC_DRAW_TILE: {
            const drawTileCmd_t *cmd = (const drawTileCmd_t *)cmdBuf;
            RB_DrawTile(cmd);
            cmdBuf = (const void *)(cmd + 1);
            break; }
        case RC_END_LIST:
            return;
        default:
            return;
        };
    }
}
