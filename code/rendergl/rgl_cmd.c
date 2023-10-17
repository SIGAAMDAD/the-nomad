#include "rgl_local.h"

renderBackendData_t *backendData;

static void R_PerformanceCounters(void)
{
	if (!r_speeds->i) {
		// clear the counters even if we aren't printing
		memset(&rg.pc, 0, sizeof(rg.pc));
		memset(&backend->pc, 0, sizeof(backend->pc));
		return;
	}

	if (r_speeds->i == 1) {
		ri.Printf(PRINT_INFO, "%u/%u/%u binds/indices/vertices %u dynamic buffers %u static buffers %u IBOs %u VBOs %u VAOs\n",
			backend->pc.c_bufferBinds, backend->pc.c_bufferIndices, backend->pc.c_bufferVertices, backend->pc.c_dynamicBufferDraws,
			backend->pc.c_staticBufferDraws, backend->pc.c_iboBinds, backend->pc.c_vboBinds, backend->pc.c_vaoBinds);
	}
	else if (r_speeds->i == 2) {
	}
}

void R_IssueRenderCommands(qboolean runPerformanceCounters)
{
	renderCommandList_t *cmdList;

	cmdList = &backendData->commandList;
	assert(cmdList);

	// add an end-of-list command
	*(renderCmdType_t *)(cmdList->buffer + cmdList->usedBytes) = RC_END_LIST;

	// clear it out, in case this is a sync and not a buffer flip
	cmdList->usedBytes = 0;

	if (runPerformanceCounters) {
		R_PerformanceCounters();
	}

	// actually start the commands going
	if (!r_skipBackEnd->i) {
		// let it start on the new batch
		RB_ExecuteRenderCommands(cmdList->buffer);
	}
}

//
// R_IssuePendingRenderCommands: issue any pending commands and wait for them to complete
//
void R_IssuePendingRenderCommands(void)
{
    if (!rg.registered) {
        return;
    }

    R_IssueRenderCommands(qfalse);
}

/*
============
R_GetCommandBufferReserved

make sure there is enough command space
============
*/
void *R_GetCommandBufferReserved( uint32_t bytes, uint32_t reservedBytes )
{
	renderCommandList_t	*cmdList;

	cmdList = &backendData->commandList;
	bytes = PAD(bytes, sizeof(void *));

	// always leave room for the end of list command
	if ( cmdList->usedBytes + bytes + sizeof( renderCmdType_t ) + reservedBytes > MAX_RC_BUFFER ) {
		if ( bytes > MAX_RC_BUFFER - sizeof( renderCmdType_t ) ) {
			ri.Error( ERR_FATAL, "R_GetCommandBuffer: bad size %i", bytes );
		}
		// if we run out of room, just start dropping commands
		return NULL;
	}

	cmdList->usedBytes += bytes;

	return cmdList->buffer + cmdList->usedBytes - bytes;
}

/*
=============
R_GetCommandBuffer

returns NULL if there is not enough space for important commands
=============
*/
void *R_GetCommandBuffer( uint32_t bytes ) {
	return R_GetCommandBufferReserved( bytes, PAD( sizeof( swapBuffersCmd_t ), sizeof(void *) ) );
}

GDR_EXPORT void RE_SetColor(const float *rgba)
{
	setColorCmd_t *cmd;
	
	if (!rg.registered) {
		return;
	}
	cmd = R_GetCommandBuffer(sizeof(*cmd));
	if (!cmd) {
		return;
	}
	cmd->commandId = RC_SET_COLOR;

	if (!rgba) {
		static float colorWhite[4] = { 1, 1, 1, 1 };

		rgba = colorWhite;
	}
	cmd->color[0] = rgba[0];
	cmd->color[1] = rgba[1];
	cmd->color[2] = rgba[2];
	cmd->color[3] = rgba[3];
}

void RE_AddPostProcessCmd(void)
{
	postProcessCmd_t *cmd;

	cmd = R_GetCommandBuffer(sizeof(*cmd));
	if (!cmd)
		return;
	
	cmd->commandId = RC_POSTPROCESS;
	cmd->viewData = rg.viewData;
}

GDR_EXPORT void RE_DrawImage( uint32_t x, uint32_t y, uint32_t w, uint32_t h, float u1, float v1, float u2, float v2, nhandle_t hShader )
{
	drawImageCmd_t *cmd;

	if (!rg.registered) {
		return;
	}
	cmd = R_GetCommandBuffer(sizeof(*cmd));
	if (!cmd)
		return;
	
	cmd->commandId = RC_DRAW_IMAGE;
	cmd->w = w;
	cmd->h = h;
	cmd->x = x;
	cmd->y = y;
	cmd->shader = R_GetShaderByHandle(hShader);
	cmd->u1 = u1;
	cmd->v1 = v1;
	cmd->u2 = u2;
	cmd->v2 = v2;
}

