#include "rgl_local.h"

void GL_PopTexture(void)
{
	GL_BindTexture(GL_TEXTURE0+(uintptr_t)(glState.textureStackPtr-glState.textureStack), 0);
	*glState.textureStackPtr = 0;
	if (glState.textureStackPtr != &glState.textureStack[0])
		glState.textureStackPtr--;
}

void GL_PushTexture(texture_t *texture)
{
	if (glState.textureStackPtr > &glState.textureStack[MAX_TEXTURE_UNITS - 1]) {
		ri.Error(ERR_DROP, "Texture stack overflow");
	}

	// do we already have it?
	for (int i = 0; i < MAX_TEXTURE_UNITS; i++) {
		if (glState.textureStack[i] == texture->id)
			return;
	}

	// pop a texture
	if (glState.textureStackPtr == &glState.textureStack[MAX_TEXTURE_UNITS - 1]) {
		GL_BindTexture(GL_TEXTURE0+(uintptr_t)(glState.textureStackPtr-glState.textureStack), NULL);
		glState.textureStack[MAX_TEXTURE_UNITS - 1] = 0;
		glState.textureStackPtr--;
	}
	*glState.textureStackPtr = texture->id;
	glState.textureStackPtr++;
	GL_BindTexture(GL_TEXTURE0+(uintptr_t)(glState.textureStackPtr-glState.textureStack), texture);
}

/*
** GL_State
**
** This routine is responsible for setting the most commonly changed state
** in GLN.
*/
void GL_State( unsigned stateBits )
{
	unsigned diff = stateBits ^ glState.glStateBits;

	if ( !diff ) {
		return;
	}

	//
	// check depthFunc bits
	//
	if ( diff & GLS_DEPTHFUNC_EQUAL ) {
		if ( stateBits & GLS_DEPTHFUNC_EQUAL ) {
			nglDepthFunc( GL_EQUAL );
		}
		else {
			nglDepthFunc( GL_LEQUAL );
		}
	}

	//
	// check blend bits
	//
	if ( diff & GLS_BLEND_BITS ) {
		GLenum srcFactor = GL_ONE, dstFactor = GL_ONE;

		if ( stateBits & GLS_BLEND_BITS ) {
			switch ( stateBits & GLS_SRCBLEND_BITS ) {
			case GLS_SRCBLEND_ZERO:
				srcFactor = GL_ZERO;
				break;
			case GLS_SRCBLEND_ONE:
				srcFactor = GL_ONE;
				break;
			case GLS_SRCBLEND_DST_COLOR:
				srcFactor = GL_DST_COLOR;
				break;
			case GLS_SRCBLEND_ONE_MINUS_DST_COLOR:
				srcFactor = GL_ONE_MINUS_DST_COLOR;
				break;
			case GLS_SRCBLEND_SRC_ALPHA:
				srcFactor = GL_SRC_ALPHA;
				break;
			case GLS_SRCBLEND_ONE_MINUS_SRC_ALPHA:
				srcFactor = GL_ONE_MINUS_SRC_ALPHA;
				break;
			case GLS_SRCBLEND_DST_ALPHA:
				srcFactor = GL_DST_ALPHA;
				break;
			case GLS_SRCBLEND_ONE_MINUS_DST_ALPHA:
				srcFactor = GL_ONE_MINUS_DST_ALPHA;
				break;
			case GLS_SRCBLEND_ALPHA_SATURATE:
				srcFactor = GL_SRC_ALPHA_SATURATE;
				break;
			default:
				ri.Error( ERR_DROP, "GL_State: invalid src blend state bits" );
				break;
			};

			switch ( stateBits & GLS_DSTBLEND_BITS ) {
			case GLS_DSTBLEND_ZERO:
				dstFactor = GL_ZERO;
				break;
			case GLS_DSTBLEND_ONE:
				dstFactor = GL_ONE;
				break;
			case GLS_DSTBLEND_SRC_COLOR:
				dstFactor = GL_SRC_COLOR;
				break;
			case GLS_DSTBLEND_ONE_MINUS_SRC_COLOR:
				dstFactor = GL_ONE_MINUS_SRC_COLOR;
				break;
			case GLS_DSTBLEND_SRC_ALPHA:
				dstFactor = GL_SRC_ALPHA;
				break;
			case GLS_DSTBLEND_ONE_MINUS_SRC_ALPHA:
				dstFactor = GL_ONE_MINUS_SRC_ALPHA;
				break;
			case GLS_DSTBLEND_DST_ALPHA:
				dstFactor = GL_DST_ALPHA;
				break;
			case GLS_DSTBLEND_ONE_MINUS_DST_ALPHA:
				dstFactor = GL_ONE_MINUS_DST_ALPHA;
				break;
			default:
				ri.Error( ERR_DROP, "GL_State: invalid dst blend state bits" );
				break;
			};

			nglEnable( GL_BLEND );
			nglBlendFunc( srcFactor, dstFactor );
		}
		else {
			nglDisable( GL_BLEND );
		}
	}

	//
	// check depthmask
	//
	if ( diff & GLS_DEPTHMASK_TRUE ) {
		if ( stateBits & GLS_DEPTHMASK_TRUE ) {
			nglDepthMask( GL_TRUE );
		}
		else {
			nglDepthMask( GL_FALSE );
		}
	}

	//
	// fill/line mode
	//
	if ( diff & GLS_POLYMODE_LINE ) {
		if ( stateBits & GLS_POLYMODE_LINE ) {
			nglPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
		}
		else {
			nglPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
		}
	}

	//
	// depthtest
	//
	if ( diff & GLS_DEPTHTEST_DISABLE ) {
		if ( stateBits & GLS_DEPTHTEST_DISABLE ) {
			nglDisable( GL_DEPTH_TEST );
		}
		else {
			nglEnable( GL_DEPTH_TEST );
		}
	}

	//
	// alpha test
	//
	if ( diff & GLS_ATEST_BITS ) {
		switch ( stateBits & GLS_ATEST_BITS ) {
		case 0:
			nglDisable( GL_ALPHA_TEST );
			break;
		case GLS_ATEST_GT_0:
			nglEnable( GL_ALPHA_TEST );
			nglAlphaFunc( GL_GREATER, 0.0f );
			break;
		case GLS_ATEST_LT_80:
			nglEnable( GL_ALPHA_TEST );
			nglAlphaFunc( GL_LESS, 0.5f );
			break;
		case GLS_ATEST_GE_80:
			nglEnable( GL_ALPHA_TEST );
			nglAlphaFunc( GL_GEQUAL, 0.5f );
			break;
		default:
			ri.Error( ERR_DROP, "GL_State: invalid alpha test bits" );
			break;
		};
	}

	glState.glStateBits = stateBits;
}

void GL_ClientState( int unit, unsigned stateBits )
{
	unsigned diff = stateBits ^ glState.glClientStateBits[ unit ];

	if ( diff & CLS_COLOR_ARRAY ) {
		if ( stateBits & CLS_COLOR_ARRAY )
			nglEnableClientState( GL_COLOR_ARRAY );
		else
			nglDisableClientState( GL_COLOR_ARRAY );
	}

	if ( diff & CLS_NORMAL_ARRAY ) {
		if ( stateBits & CLS_NORMAL_ARRAY )
			nglEnableClientState( GL_NORMAL_ARRAY );
		else
			nglDisableClientState( GL_NORMAL_ARRAY );
	}

	if ( diff & CLS_TEXCOORD_ARRAY ) {
		if ( stateBits & CLS_TEXCOORD_ARRAY )
			nglEnableClientState( GL_TEXTURE_COORD_ARRAY );
		else
			nglDisableClientState( GL_TEXTURE_COORD_ARRAY );
	}

	glState.glClientStateBits[ unit ] = stateBits;
}


void GL_BindNullTextures(void)
{
    for (uint32_t i = 0; i < MAX_TEXTURE_UNITS; i++) {
        nglActiveTexture(GL_TEXTURE0 + i);
        nglBindTexture(GL_TEXTURE_2D, 0);
        glState.currenttextures[i] = 0;
    }

    nglActiveTexture(GL_TEXTURE0);
    glState.currenttmu = GL_TEXTURE0;
}

void GL_BindTexture(GLenum unit, texture_t *texture)
{
    GLuint tmu = unit - GL_TEXTURE0;
	if (!texture) {
		glState.currenttextures[tmu] = 0;
		glState.currenttmu = unit;
		nglActiveTexture(unit);
		nglBindTexture(GL_TEXTURE_2D, 0);
		return;
	}
    if (glState.currenttextures[tmu] == texture->id)
        return;
    
    nglActiveTexture(unit);
    nglBindTexture(GL_TEXTURE_2D, texture->id);
    glState.currenttmu = unit;
}

int GL_UseProgram(GLuint program)
{
    if (glState.shaderId == program)
        return 0;
    
    nglUseProgram(program);
    glState.shaderId = program;
	return 1;
}

void GL_BindNullProgram(void)
{
    nglUseProgram((unsigned)0);
    glState.shaderId = 0;
	glState.currentShader = &rg.basicShader;
}

void GL_BindFramebuffer(GLenum target, GLuint fbo)
{
    switch (target) {
    case GL_FRAMEBUFFER:
        if (glState.defFboId == fbo)
            return;
        
        glState.defFboId = fbo;
        break;
    case GL_READ_FRAMEBUFFER:
        if (glState.readFboId == fbo)
            return;
        
        glState.readFboId = fbo;
        break;
    case GL_DRAW_FRAMEBUFFER:
        if (glState.writeFboId == fbo)
            return;
        
        glState.writeFboId = fbo;
        break;
    default: // should never happen, if it does, then skill issue from the dev
        ri.Error(ERR_FATAL, "GL_BindFramebuffer: Invalid fbo target: %i", target);
    };

    nglBindFramebuffer(target, fbo);
}

void GL_BindNullFramebuffers(void)
{
    nglBindFramebuffer(GL_FRAMEBUFFER, 0);
    nglBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
    nglBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	glState.currentFbo = rg.renderFbo;
}

void GL_SetModelViewMatrix(const mat4_t m)
{
	Mat4Copy(m, glState.modelView);
	Mat4Multiply(glState.projection, glState.modelView, glState.modelViewProjectionMatrix);
}

void GL_SetProjectionMatrix(const mat4_t m)
{
	Mat4Copy(m, glState.projection);
	Mat4Multiply(glState.projection, glState.modelView, glState.modelViewProjectionMatrix);
}

void GL_BindNullFramebuffer(GLenum target)
{
    switch (target) {
    case GL_FRAMEBUFFER:
        if (glState.defFboId == 0)
            return;
        
        glState.defFboId = 0;
        break;
    case GL_READ_FRAMEBUFFER:
        if (glState.readFboId == 0)
            return;
        
        glState.readFboId = 0;
        break;
    case GL_DRAW_FRAMEBUFFER:
        if (glState.writeFboId == 0)
            return;
        
        glState.writeFboId = 0;
        break;
    default:
        ri.Error(ERR_FATAL, "GL_BindNullFramebuffer: Invalid fbo target: %i", target);
    };

    nglBindFramebuffer(target, 0);
}

void GL_BindRenderbuffer(GLuint rbo)
{
    if (glState.rboId == rbo)
        return;
    
    glState.rboId = rbo;
    nglBindRenderbuffer(GL_RENDERBUFFER, rbo);
}

void GL_BindNullRenderbuffer(void)
{
    if (glState.rboId == 0)
        return;
    
    glState.rboId = 0;
    nglBindRenderbuffer(GL_RENDERBUFFER, 0);
}

/*
RB_BeginSurface:
called whenever we start rendering through a new texture binding
*/
void RB_BeginSurface(shader_t *shader)
{
	// if we are still rendering to the
	// shader given, don't switch up
	if (backend->dbuf.shader == shader) {
		return;
	}
	// finish up anything previously rendering
	else if (backend->dbuf.numIndices) {
		RB_EndSurface();
	}

	backend->dbuf.shader = shader;
}

/*
RB_EndSurface:
called whenever we must switch texture binding by shader sort
*/
void RB_EndSurface(void)
{
	// flush if there's anything there
	R_FinishBatch(&backend->dbuf);

	// clear out the old data
	backend->dbuf.numIndices = 0;
	backend->dbuf.numVertices = 0;
	backend->dbuf.firstIndex = 0;
}

static void RB_RenderDrawSurfList(const drawSurf_t *surfList, uint32_t numSurfs)
{
	shader_t *shader, *oldShader;
	const drawSurf_t *surf;
	uint32_t i;
	uint32_t oldSort;
	fbo_t *fbo = NULL;
	qboolean depthRange, oldDepthRange;

	// start drawing
	fbo = glState.currentFbo;
	oldShader = NULL;
	shader = NULL;
	oldSort = ~0U;
	depthRange = qfalse;
	oldDepthRange = qfalse;

	for (i = 0, surf = surfList; i < numSurfs; i++, surf++) {
		if (surf->sort == oldSort) {
			if (shader && shader->sort != SS_OPAQUE)
				continue;
			
			// fast path, same as previous sort
			rb_surfaceTable[*surf->surface](surf->surface);
			continue;
		}
		oldSort = surf->sort;

		if (shader != NULL && (shader != oldShader)) {
			if (oldShader != NULL) {
				RB_EndSurface();
			}
			RB_BeginSurface(shader);
			oldShader = shader;
		}

		// add triangles to this surface
		rb_surfaceTable[*surf->surface](surf->surface);
	}

	// draw the contents of the last shader batch
	if (oldShader != NULL)
		RB_EndSurface();
	
	if (glContext->ARB_framebuffer_object)
		FBO_Bind(fbo);

	nglDepthRange(0, 1);
}

static const char *GL_ErrorString(GLenum error)
{
    switch (error) {
    case GL_INVALID_ENUM:
        return "GL_INVALID_ENUM";
    case GL_INVALID_FRAMEBUFFER_OPERATION:
        return "GL_INVALID_FRAMEBUFFER_OPERATION";
    case GL_INVALID_OPERATION:
        return "GL_INVALID_OPERATION";
    case GL_INVALID_INDEX:
        return "GL_INVALID_INDEX";
    case GL_INVALID_VALUE:
        return "GL_INVALID_VALUE";
    case GL_OUT_OF_MEMORY:
        return "GL_OUT_OF_MEMORY";
    default:
        break;
    };
    return "Unknown Error Code";
}

void GL_CheckErrors(void)
{
    GLenum error = nglGetError();

    if (error == GL_NO_ERROR)
        return;
    
    if (!r_ignoreGLErrors->i) {
        ri.Error(ERR_FATAL, "GL_CheckErrors: OpenGL error occured (0x%x): %s", error, GL_ErrorString(error));
    }
}

static void R_SetDefaultState(void)
{
	nglClear(GL_COLOR_BUFFER_BIT);

	//
    // do overdraw measurement
    //
    if (r_measureOverdraw->i) {
        if (glConfig.stencilBits < 4) {
            ri.Printf(PRINT_INFO, "Warning not enough stencil bits to measure overdraw: %i\n", glConfig.stencilBits);
            ri.Cvar_Set("r_measureOverdraw", "0");
            r_measureOverdraw->modified = qfalse;
        }
        else {
            R_IssuePendingRenderCommands();
			nglEnable(GL_STENCIL_TEST);
			nglClearStencil(0U);
            nglStencilMask(~0U);
            nglStencilFunc(GL_ALWAYS, 0U, ~0U);
			nglClear(GL_STENCIL_BUFFER_BIT);
            nglStencilOp(GL_KEEP, GL_INCR, GL_INCR);
        }
        r_measureOverdraw->modified = qfalse;
    }
    else {
        // this is only reached if it was on and is now off
        if (r_measureOverdraw->modified) {
            R_IssuePendingRenderCommands();
            nglDisable(GL_STENCIL_TEST);
        }
        r_measureOverdraw->modified = qfalse;
    }

	// clear all textures
	GL_BindNullTextures();
	glState.textureStackPtr = glState.textureStack;
	memset(glState.textureStack, 0, sizeof(glState.textureStack));

	// calculate matrices
	RB_MakeModelViewProjection();
}

void RE_BeginFrame(stereoFrame_t stereoFrame)
{
    drawBufferCmd_t *cmd = NULL;
	colorMaskCmd_t *colorCmd = NULL;

    if (!rg.registered) {
        return;
    }

    // check for errors
    GL_CheckErrors();

    if ((cmd = R_GetCommandBuffer(sizeof(*cmd))) == NULL)
        return;
    
    cmd->commandId = RC_DRAW_BUFFER;

	//
	// setup gl state for frame
	//
	R_SetDefaultState();

    //
    // texture filtering
    //
    if (r_textureFiltering->modified) {
        R_IssuePendingRenderCommands();
        R_UpdateTextures();
    }

    // check for errors
    if (!r_ignoreGLErrors->i) {
        GLenum error;

        R_IssuePendingRenderCommands();
        if ((error = nglGetError()) != GL_NO_ERROR)
            ri.Error(ERR_FATAL, "RE_BeginFrame() - glGetError() failed (0x%x)!", error);
    }

    if (glConfig.stereoEnabled) {
		if (!(cmd = R_GetCommandBuffer(sizeof(*cmd))))
			return;
		
		cmd->commandId = RC_DRAW_BUFFER;

		if (stereoFrame == STEREO_LEFT) {
			cmd->buffer = GL_BACK_LEFT;
		}
		else if (stereoFrame == STEREO_RIGHT) {
			cmd->buffer = GL_BACK_RIGHT;
		}
		else {
			ri.Error(ERR_FATAL, "RE_BeginFrame: Stereo is enabled, but stereoFrame was %i", stereoFrame);
		}
    }
    else {
		if (stereoFrame != STEREO_CENTER)
			ri.Error(ERR_FATAL, "RE_BeginFrame: Stereo is disabled, but stereoFrame was %i", stereoFrame);
		if (!(cmd = R_GetCommandBuffer(sizeof(*cmd))))
			return;
		
		if (cmd) {
			cmd->commandId = RC_DRAW_BUFFER;

			if (!N_stricmp(r_drawBuffer->s, "GL_FRONT"))
				cmd->buffer = GL_FRONT;
			else
				cmd->buffer = GL_BACK;
		}
    }
}

//
// RE_EndFrame: returns the number of msec spent in the back end
//
void RE_EndFrame(uint64_t *frontEndMsec, uint64_t *backEndMsec)
{
	swapBuffersCmd_t *cmd;

	if (!rg.registered) {
		return;
	}

	cmd = R_GetCommandBufferReserved(sizeof(*cmd), 0);
	if (!cmd)
		return;
	
	cmd->commandId = RC_SWAP_BUFFERS;

	R_IssueRenderCommands(qtrue);

	R_InitNextFrame();

	if (frontEndMsec) {
	}
	if (backEndMsec) {
		*backEndMsec = backend->pc.msec;
	}
	backend->pc.msec = 0;
}

static const void *RB_DrawBuffer(const void *data)
{
    const drawBufferCmd_t *cmd;

    cmd = (const drawBufferCmd_t *)data;

    nglDrawBuffer(cmd->buffer);

    // clear screen for debugging
    if (r_clear->i) {
        nglClearColor(1, 0, 0.5, 1);
        nglClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    return (const void *)(cmd + 1);
}

static const void *RB_SwapBuffers(const void *data)
{
    const swapBuffersCmd_t *cmd;

    cmd = (const swapBuffersCmd_t *)data;

    // we measure overdraw by reading back the stencil buffer and
	// counting up the number of increments that have happened
	if ( r_measureOverdraw->i ) {
		uint32_t i;
		uint64_t sum = 0;
		unsigned char *stencilReadback;

		stencilReadback = ri.Hunk_AllocateTempMemory( glConfig.vidWidth * glConfig.vidHeight );
		nglReadPixels( 0, 0, glConfig.vidWidth, glConfig.vidHeight, GL_STENCIL_INDEX, GL_UNSIGNED_BYTE, stencilReadback );

		for ( i = 0; i < glConfig.vidWidth * glConfig.vidHeight; i++ ) {
			sum += stencilReadback[i];
		}

		backend->pc.c_overDraw += sum;
		ri.Hunk_FreeTempMemory( stencilReadback );
	}

    if (glContext->ARB_framebuffer_object) {
		if (!backend->framePostProcessed) {
			if (rg.msaaResolveFbo && r_hdr->i) {
				// Resolving an RGB16F MSAA FBO to the screen messes with the brightness, so resolve to an RGB16F FBO first
				FBO_FastBlit(rg.renderFbo, NULL, rg.msaaResolveFbo, NULL, GL_COLOR_BUFFER_BIT, GL_NEAREST);
				FBO_FastBlit(rg.msaaResolveFbo, NULL, NULL, NULL, GL_COLOR_BUFFER_BIT, GL_NEAREST);
			}
			else if (rg.renderFbo) {
				FBO_FastBlit(rg.renderFbo, NULL, NULL, NULL, GL_COLOR_BUFFER_BIT, GL_NEAREST);
			}
		}
	}

	if ( !glState.finishCalled ) {
		nglFinish();
	}

	ri.GLimp_LogComment( "***************** RB_SwapBuffers *****************\n\n\n" );

	ri.GLimp_EndFrame();

    return (const void *)(cmd + 1);
}

static void SetViewportAndScissor(void)
{
	int width, height;
	int x, y;

	if (glState.currentFbo) {
		x = glState.currentFbo->x;
		y = glState.currentFbo->y;
		width = glState.currentFbo->width;
		height = glState.currentFbo->height;
	}
	else {
		x = rg.viewData.viewportX;
		y = rg.viewData.viewportY;
		width = glConfig.vidWidth;
		height = glConfig.vidHeight;
	}

	nglViewport(x, y, width, height);
	nglScissor(x, y, width, height);
}

static void RB_BeginDrawingView(void)
{
	unsigned clearBits = 0;

	// sync with gl if needed
	if (r_finish->i && !glState.finishCalled) {
		nglFinish();
		glState.finishCalled = qtrue;
	}
	if (!r_finish->i) {
		glState.finishCalled = qtrue;
	}

	if (glContext->ARB_framebuffer_object) {
		fbo_t *fbo = rg.viewData.targetFbo;

		if (fbo == NULL && !(backend->framePostProcessed))
			fbo = rg.renderFbo;
		
		FBO_Bind(fbo);
	}

	//
	// update
	//
	SetViewportAndScissor();

	// ensures that depth writes are enabled for the depth clear
	GL_State(GLS_DEFAULT);
	// clear relevant buffers
	clearBits = GL_DEPTH_BUFFER_BIT;

	if (r_measureOverdraw->i) {
		clearBits |= GL_STENCIL_BUFFER_BIT;
	}

	nglClear(clearBits);
}

//
// RB_SetGLRaw2D: only really for drawing
// straight up textures, disables depth buffering
//
static void RB_SetGLRaw2D(void)
{
	int width, height;

	if (glState.currentFbo) {
		width = glState.currentFbo->width;
		height = glState.currentFbo->height;
	}
	else {
		width = glConfig.vidWidth;
		height = glConfig.vidHeight;
	}

	nglViewport(0, 0, width, height);
	nglScissor(0, 0, width, height);

	GL_State(GLS_DEPTHTEST_DISABLE |
			 GLS_SRCBLEND_SRC_ALPHA |
			 GLS_DSTBLEND_ONE_MINUS_SRC_ALPHA);
}

static const void *RB_DrawSurfaces(const void *data)
{
	const drawSurfCmd_t *cmd;

	cmd = (const drawSurfCmd_t *)data;

	// finish up any old surface
	if (backend->dbuf.numIndices) {
		RB_EndSurface();
	}

	rg.viewData = cmd->viewData;

	// clear the z bufffer
	RB_BeginDrawingView();
	RB_RenderDrawSurfList(cmd->drawSurfs, cmd->numDrawSurfs);

	if (glContext->ARB_framebuffer_object) {
		nglEnable(GL_DEPTH_CLAMP);
	}

#if 0
	if (glContext->ARB_framebuffer_object && r_depthPrepass->i) {
		fbo_t *oldFbo = glState.currentFbo;
		vec4_t viewInfo;

		VectorSet4(viewInfo, rg.viewData.zFar / r_znear->f, rg.viewData.zFar, 0.0f, 0.0f);

		backend->depthFill = qtrue;
		nglColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
		RB_RenderDrawSurfList(cmd->drawSurfs, cmd->numDrawSurfs);
		nglColorMask(!backend->colorMask[0], !backend->colorMask[1], !backend->colorMask[2], !backend->colorMask[3]);
		backend->depthFill = qfalse;

		// reset viewport and scissor
		FBO_Bind(oldFbo);
		rg.viewData.viewportX = 0;
		rg.viewData.viewportY = 0;
		SetViewportAndScissor();
	}
#endif

#if 0
	if (0) {
		if (rg.msaaResolveFbo) {
			// if we're using multisampling, resolve the depth first
			FBO_FastBlit(rg.renderFbo, NULL, rg.msaaResolveFbo, NULL, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
		}
		if (r_ssao->i) {
			vec4_t quadVerts[4];
			vec2_t texCoords[4];

			viewInfo[2] = 1.0f / ((float)(tr.quarterImage[0]->width)  * tan(backEnd.viewParms.fovX * M_PI / 360.0f) * 2.0f);
			viewInfo[3] = 1.0f / ((float)(tr.quarterImage[0]->height) * tan(backEnd.viewParms.fovY * M_PI / 360.0f) * 2.0f);
			viewInfo[3] *= (float)backEnd.viewParms.viewportHeight / (float)backEnd.viewParms.viewportWidth;

			FBO_Bind(tr.quarterFbo[0]);

			qglViewport(0, 0, tr.quarterFbo[0]->width, tr.quarterFbo[0]->height);
			qglScissor(0, 0, tr.quarterFbo[0]->width, tr.quarterFbo[0]->height);

			VectorSet4(quadVerts[0], -1,  1, 0, 1);
			VectorSet4(quadVerts[1],  1,  1, 0, 1);
			VectorSet4(quadVerts[2],  1, -1, 0, 1);
			VectorSet4(quadVerts[3], -1, -1, 0, 1);

			texCoords[0][0] = 0; texCoords[0][1] = 1;
			texCoords[1][0] = 1; texCoords[1][1] = 1;
			texCoords[2][0] = 1; texCoords[2][1] = 0;
			texCoords[3][0] = 0; texCoords[3][1] = 0;

			GL_State( GLS_DEPTHTEST_DISABLE );

			GLSL_BindProgram(&tr.ssaoShader);

			GL_BindToTMU(tr.hdrDepthImage, TB_COLORMAP);

			GLSL_SetUniformVec4(&tr.ssaoShader, UNIFORM_VIEWINFO, viewInfo);

			RB_InstantQuad2(quadVerts, texCoords); //, color, shaderProgram, invTexRes);


			viewInfo[2] = 1.0f / (float)(tr.quarterImage[0]->width);
			viewInfo[3] = 1.0f / (float)(tr.quarterImage[0]->height);

				FBO_Bind(tr.quarterFbo[1]);

				qglViewport(0, 0, tr.quarterFbo[1]->width, tr.quarterFbo[1]->height);
				qglScissor(0, 0, tr.quarterFbo[1]->width, tr.quarterFbo[1]->height);

				GLSL_BindProgram(&tr.depthBlurShader[0]);

				GL_BindToTMU(tr.quarterImage[0],  TB_COLORMAP);
				GL_BindToTMU(tr.hdrDepthImage, TB_LIGHTMAP);

				GLSL_SetUniformVec4(&tr.depthBlurShader[0], UNIFORM_VIEWINFO, viewInfo);

				RB_InstantQuad2(quadVerts, texCoords); //, color, shaderProgram, invTexRes);


				FBO_Bind(tr.screenSsaoFbo);

				qglViewport(0, 0, tr.screenSsaoFbo->width, tr.screenSsaoFbo->height);
				qglScissor(0, 0, tr.screenSsaoFbo->width, tr.screenSsaoFbo->height);

				GLSL_BindProgram(&tr.depthBlurShader[1]);

				GL_BindToTMU(tr.quarterImage[1],  TB_COLORMAP);
				GL_BindToTMU(tr.hdrDepthImage, TB_LIGHTMAP);

				GLSL_SetUniformVec4(&tr.depthBlurShader[1], UNIFORM_VIEWINFO, viewInfo);


				RB_InstantQuad2(quadVerts, texCoords); //, color, shaderProgram, invTexRes);
		}
	}
	#endif

	if (glContext->ARB_framebuffer_object) {
		nglDisable(GL_DEPTH_CLAMP);
	}

	return (const void *)(cmd + 1);
}

/*
===============
RB_ShowImages

Draw all the images to the screen, on top of whatever
was there.  This is used to test for texture thrashing.

Also called by RE_EndRegistration
===============
*/
void RB_ShowImages(void)
{
	uint64_t i;
	texture_t *image;
	float x, y, w, h;
	uint64_t start, end;

	RB_SetGLRaw2D();

	nglClear(GL_COLOR_BUFFER_BIT);

	nglFinish();

	start = ri.Milliseconds();

	for (uint64_t i = 0; i < rg.numTextures; i++) {
		image = rg.textures[i];

		w = glConfig.vidWidth / 20;
		h = glConfig.vidHeight / 15;
		x = i % 20 * w;
		h = i / 20 * h;

		// show in proportional size in mode 2
		if ( r_showImages->i == 2 ) {
			w *= image->uploadWidth / 512.0f;
			h *= image->uploadHeight / 512.0f;
		}

		{
			vec4_t quadVerts[4];

			GL_BindTexture(GL_TEXTURE0, image);

			VectorSet4(quadVerts[0], x, y, 0, 1);
			VectorSet4(quadVerts[1], x + w, y, 0, 1);
			VectorSet4(quadVerts[2], x + w, y + h, 0, 1);
			VectorSet4(quadVerts[3], x, y + h, 0, 1);

			RB_InstantQuad(quadVerts);
		}
	}

	nglFinish();

	end = ri.Milliseconds();

	ri.Printf( PRINT_INFO, "%lu msec to draw all images\n", end - start );
}

static const void *RB_DrawImage(const void *data)
{
	const drawImageCmd_t *cmd;
	shader_t *shader;
	uint32_t numVerts, numIndices;

	cmd = (const drawImageCmd_t *)data;

	RB_SetGLRaw2D();
	
	shader = cmd->shader;
	if (shader != backend->dbuf.shader) {
		if (backend->dbuf.numIndices) {
			RB_EndSurface();
		}
		RB_BeginSurface(shader);
	}

	RB_CheckOverflow(4, 6);
	numVerts = backend->dbuf.numVertices;
	numIndices = backend->dbuf.numIndices;

	backend->dbuf.numVertices += 4;
	backend->dbuf.numIndices += 6;

	backend->dbuf.indices[ numIndices ] = numVerts + 3;
	backend->dbuf.indices[ numIndices + 1 ] = numVerts + 0;
	backend->dbuf.indices[ numIndices + 2 ] = numVerts + 2;
	backend->dbuf.indices[ numIndices + 3 ] = numVerts + 2;
	backend->dbuf.indices[ numIndices + 4 ] = numVerts + 0;
	backend->dbuf.indices[ numIndices + 5 ] = numVerts + 1;

	{
		uint16_t color[4];

		VectorScale4(backend->color2D, 257, color);

		VectorCopy4(color, backend->dbuf.color[ numVerts ]);
		VectorCopy4(color, backend->dbuf.color[ numVerts + 1]);
		VectorCopy4(color, backend->dbuf.color[ numVerts + 2]);
		VectorCopy4(color, backend->dbuf.color[ numVerts + 3 ]);
	}

	backend->dbuf.xyz[ numVerts ][0] = cmd->x;
	backend->dbuf.xyz[ numVerts ][1] = cmd->y;
	backend->dbuf.xyz[ numVerts ][2] = 0;

	backend->dbuf.texCoords[ numVerts ][0] = cmd->u1;
	backend->dbuf.texCoords[ numVerts ][1] = cmd->v1;

	backend->dbuf.xyz[ numVerts + 1 ][0] = cmd->x + cmd->w;
	backend->dbuf.xyz[ numVerts + 1 ][1] = cmd->y;
	backend->dbuf.xyz[ numVerts + 1 ][2] = 0;

	backend->dbuf.texCoords[ numVerts + 1 ][0] = cmd->u2;
	backend->dbuf.texCoords[ numVerts + 1 ][1] = cmd->v1;

	backend->dbuf.xyz[ numVerts + 2 ][0] = cmd->x + cmd->w;
	backend->dbuf.xyz[ numVerts + 2 ][1] = cmd->y + cmd->h;
	backend->dbuf.xyz[ numVerts + 2 ][2] = 0;

	backend->dbuf.texCoords[ numVerts + 2 ][0] = cmd->u2;
	backend->dbuf.texCoords[ numVerts + 2 ][1] = cmd->v1;

	backend->dbuf.xyz[ numVerts + 3 ][0] = cmd->x;
	backend->dbuf.xyz[ numVerts + 3 ][1] = cmd->y + cmd->h;
	backend->dbuf.xyz[ numVerts + 3 ][2] = 0;

	backend->dbuf.texCoords[ numVerts + 3 ][0] = cmd->u1;
	backend->dbuf.texCoords[ numVerts + 3 ][1] = cmd->v2;

	return (const void *)(cmd + 1);
}

static const void *RB_ColorMask(const void *data)
{
	const colorMaskCmd_t *cmd;

	cmd = (const colorMaskCmd_t *)data;

	// finish any current batch
	if (backend->dbuf.numIndices)
		RB_EndSurface();

	if (glContext->ARB_framebuffer_object) {
		// reverse color mask, so 0 0 0 0 is the default
		backend->colorMask[0] = !cmd->rgba[0];
		backend->colorMask[1] = !cmd->rgba[1];
		backend->colorMask[2] = !cmd->rgba[2];
		backend->colorMask[3] = !cmd->rgba[3];
	}

	nglColorMask(cmd->rgba[0], cmd->rgba[1], cmd->rgba[2], cmd->rgba[3]);

	return (const void *)(cmd + 1);
}

static const void *RB_SetColor(const void *data)
{
	const setColorCmd_t *cmd;

	cmd = (const setColorCmd_t *)data;

	backend->color2D[0] = cmd->color[0] * 255;
	backend->color2D[1] = cmd->color[1] * 255;
	backend->color2D[2] = cmd->color[2] * 255;
	backend->color2D[3] = cmd->color[3] * 255;

	return (const void *)(cmd + 1);
}

void RB_ExecuteRenderCommands(const void *data)
{
    uint64_t start, end;

    start = ri.Milliseconds();

    while (1) {
        data = PADP(data, sizeof(void *));

        switch (*(const renderCmdType_t *)data) {
		case RC_DRAW_BUFFER:
			data = RB_DrawBuffer(data);
			break;
		case RC_DRAW_IMAGE:
			data = RB_DrawImage(data);
			break;
		case RC_SET_COLOR:
			data = RB_SetColor(data);
			break;
		case RC_DRAW_SURFS:
			data = RB_DrawSurfaces(data);
			break;
		case RC_SWAP_BUFFERS:
			data = RB_SwapBuffers(data);
			break;
//        case RC_POSTPROCESS:
//            data = RB_PostProcess(data);
//            break;
        case RC_END_LIST:
        default:
            // stop rendering
            end = ri.Milliseconds();
            return;
        };
    }
}
