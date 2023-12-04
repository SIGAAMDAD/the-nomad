#include "rgl_local.h"

GDR_EXPORT void GL_SetObjectDebugName(GLenum target, GLuint id, const char *name, const char *add)
{
	if (r_glDebug->i) {
		static char newName[1024];
		Com_snprintf(newName, sizeof(newName), "%s%s", name, add);
		nglObjectLabel(target, id, strlen(newName), newName);
	}
}

static GLuint textureStack[MAX_TEXTURE_UNITS];
static GLint textureStackP;

GDR_EXPORT void GL_PushTexture(CTexture *image)
{
	uint32_t i;
	GLuint id = image ? image->GetId() : 0;

	// do we need to pop one?
	if (textureStackP == MAX_TEXTURE_UNITS - 1) {
		GL_PopTexture();
	}

	// check if it's already bound
	for (i = 0; i < textureStackP; i++) {
		if (textureStack[i] == id) {
			textureStackP = i;
			break;
		}
	}

	if (i == textureStackP) {
		if (textureStackP >= MAX_TEXTURE_UNITS)
			ri.Error(ERR_DROP, "GL_PushTexture: texture stack overflow");

		textureStack[textureStackP++] = id;
		
		nglActiveTexture(GL_TEXTURE0 + textureStackP);
		nglBindTexture(GL_TEXTURE_2D, id);
	}
}

GDR_EXPORT void GL_PopTexture(void)
{
	textureStackP--;

	if (textureStackP < 0)
		ri.Error(ERR_DROP, "GL_PopTexture: texture stack underflow");

	if (textureStackP) {
		nglActiveTexture(GL_TEXTURE + textureStackP);
		nglBindTexture(GL_TEXTURE_2D, textureStack[textureStackP - 1]);
	}
}

GDR_EXPORT void GL_BindTexture(int32_t tmu, CTexture *image)
{
	GLuint texunit = GL_TEXTURE0 + tmu;
	GLuint id = image ? image->GetId() : 0;

	if (glState.currenttextures[tmu] == id) {
		return;
	}

	if (glState.currenttmu != texunit) {
		nglActiveTexture(texunit);
		glState.currenttmu = texunit;
	}

	nglBindTexture(GL_TEXTURE_2D, id);
}

GDR_EXPORT void GL_BindNullTextures(void)
{
	for (uint32_t i = 0; i < MAX_TEXTURE_UNITS; i++) {
		nglActiveTexture(GL_TEXTURE0 + i);
		nglBindTexture(GL_TEXTURE_2D, 0);
	}

	memset(textureStack, 0, sizeof(GLuint) * textureStackP);
	textureStackP = 0;

#if 0
    for (uint32_t i = 0; i < NUM_TEXTURE_BINDINGS; i++) {
        if (glState.currenttextures[i] != 0) {
            nglActiveTexture(GL_TEXTURE0 + i);
            nglBindTexture(GL_TEXTURE_2D, 0);
        }
    }
#endif
    nglActiveTexture(GL_TEXTURE0);
    glState.currenttmu = GL_TEXTURE0;
	glState.currentTexture = NULL;
}

GDR_EXPORT bool GL_UseProgram(GLuint program)
{
    if (glState.shaderId == program)
        return false;
    
    nglUseProgram(program);
    glState.shaderId = program;
	return true;
}

GDR_EXPORT void GL_BindNullProgram(void)
{
    nglUseProgram((unsigned)0);
    glState.shaderId = 0;
}

GDR_EXPORT void GL_BindFramebuffer(GLenum target, GLuint fbo)
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

GDR_EXPORT void GL_BindNullFramebuffers(void)
{
    nglBindFramebuffer(GL_FRAMEBUFFER, 0);
    nglBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
    nglBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
//	glState.currentFbo = NULL;
}

/*
** GL_State
**
** This routine is responsible for setting the most commonly changed state
** in Q3.
*/
GDR_EXPORT void GL_State( uint32_t stateBits )
{
	uint32_t diff = stateBits ^ glState.glStateBits;

	if ( !diff )
	{
		return;
	}

	//
	// check depthFunc bits
	//
	if ( diff & GLS_DEPTHFUNC_BITS )
	{
		if ( stateBits & GLS_DEPTHFUNC_EQUAL )
		{
			nglDepthFunc( GL_EQUAL );
		}
		else if ( stateBits & GLS_DEPTHFUNC_GREATER)
		{
			nglDepthFunc( GL_GREATER );
		}
		else
		{
			nglDepthFunc( GL_LEQUAL );
		}
	}

	//
	// check blend bits
	//
	if ( diff & ( GLS_SRCBLEND_BITS | GLS_DSTBLEND_BITS ) )
	{
		uint32_t oldState = glState.glStateBits & ( GLS_SRCBLEND_BITS | GLS_DSTBLEND_BITS );
		uint32_t newState = stateBits & ( GLS_SRCBLEND_BITS | GLS_DSTBLEND_BITS );
		uint32_t storedState = glState.storedGlState & ( GLS_SRCBLEND_BITS | GLS_DSTBLEND_BITS );

		if (oldState == 0)
		{
			nglEnable( GL_BLEND );
		}
		else if (newState == 0)
		{
			nglDisable( GL_BLEND );
		}

		if (newState != 0 && storedState != newState)
		{
			GLenum srcFactor = GL_ONE, dstFactor = GL_ONE;

			glState.storedGlState &= ~( GLS_SRCBLEND_BITS | GLS_DSTBLEND_BITS );
			glState.storedGlState |= newState;

			switch ( stateBits & GLS_SRCBLEND_BITS )
			{
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
			}

			switch ( stateBits & GLS_DSTBLEND_BITS )
			{
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
			}

			nglBlendFunc( srcFactor, dstFactor );
		}
	}

	//
	// check depthmask
	//
	if ( diff & GLS_DEPTHMASK_TRUE )
	{
		if ( stateBits & GLS_DEPTHMASK_TRUE )
		{
			nglDepthMask( GL_TRUE );
		}
		else
		{
			nglDepthMask( GL_FALSE );
		}
	}

	//
	// fill/line mode
	//
	if ( diff & GLS_POLYMODE_LINE )
	{
		if ( stateBits & GLS_POLYMODE_LINE )
		{
			nglPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
		}
		else
		{
			nglPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
		}
	}

	//
	// depthtest
	//
	if ( diff & GLS_DEPTHTEST_DISABLE )
	{
		if ( stateBits & GLS_DEPTHTEST_DISABLE )
		{
			nglDisable( GL_DEPTH_TEST );
		}
		else
		{
			nglEnable( GL_DEPTH_TEST );
		}
	}

	glState.glStateBits = stateBits;
}

GDR_EXPORT const char *GL_ErrorString(GLenum error)
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

GDR_EXPORT void GL_CheckErrors(void)
{
    GLenum error = nglGetError();

    if (error == GL_NO_ERROR)
        return;
    
    if (!r_ignoreGLErrors->i) {
		ri.Printf(PRINT_INFO, COLOR_RED "GL_CheckErrors: 0x%04x -- %s\n", error, GL_ErrorString(error));
        ri.Error(ERR_FATAL, "GL_CheckErrors: OpenGL error occured (0x%x): %s", error, GL_ErrorString(error));
    }
}

GDR_EXPORT void GDR_ATTRIBUTE((format(printf, 1, 2))) GL_LogComment(const char *fmt, ...)
{
	va_list argptr;
	char msg[MAXPRINTMSG];
	int length;

	va_start(argptr, fmt);
	length = N_vsnprintf(msg, sizeof(msg), fmt, argptr);
	va_end(argptr);

	nglDebugMessageInsertARB(GL_DEBUG_SOURCE_APPLICATION_ARB, GL_DEBUG_TYPE_OTHER_ARB, 0, GL_DEBUG_SEVERITY_NOTIFICATION, length, msg);
}

GDR_EXPORT void GDR_ATTRIBUTE((format(printf, 1, 2))) GL_LogError(const char *fmt, ...)
{
	va_list argptr;
	char msg[MAXPRINTMSG];
	int length;

	va_start(argptr, fmt);
	length = N_vsnprintf(msg, sizeof(msg), fmt, argptr);
	va_end(argptr);

	nglDebugMessageInsertARB(GL_DEBUG_SOURCE_APPLICATION_ARB, GL_DEBUG_TYPE_ERROR_ARB, 0, GL_DEBUG_SEVERITY_HIGH_ARB, length, msg);
}

/*
===============
RB_ShowImages

Draw all the images to the screen, on top of whatever
was there.  This is used to test for texture thrashing.

Also called by RE_EndRegistration
===============
*/
GDR_EXPORT void RB_ShowImages( void ) {
	uint32_t    i;
	CTexture   *image;
	float	    x, y, w, h;
	uint64_t    start, end;

	nglClear( GL_COLOR_BUFFER_BIT );

	nglFinish();

	start = ri.Milliseconds();

	for ( i=0 ; i<rg.numTextures ; i++ ) {
		image = rg.textures[i];

		w = glConfig.vidWidth / 20;
		h = glConfig.vidHeight / 15;
		x = i % 20 * w;
		y = i / 20 * h;

		// show in proportional size in mode 2
		if ( r_showImages->i == 2 ) {
			w *= image->GetWidth() / 512.0f;
			h *= image->GetHeight() / 512.0f;
		}

		{
			glm::vec4 quadVerts[4];

            GL_BindTexture(0, image);

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

GDR_EXPORT const void *RB_SwapBuffers(const void *data)
{
    const swapBuffersCmd_t *cmd;
    
    // texture swapping test
    if (r_showImages->i)
        RB_ShowImages();
    
    cmd = (const swapBuffersCmd_t *)data;

	// only draw imgui data after everything else has finished
	ri.ImGui_Draw();

#if 0
    // we measure overdraw by reading back the stencil buffer and
	// counting up the number of increments that have happened
	if ( r_measureOverdraw->i ) {
		uint32_t i;
		uint64_t sum = 0;
		byte *stencilReadback;

		stencilReadback = (byte *)ri.Hunk_AllocateTempMemory( glConfig.vidWidth * glConfig.vidHeight );
		nglReadPixels( 0, 0, glConfig.vidWidth, glConfig.vidHeight, GL_STENCIL_INDEX, GL_UNSIGNED_BYTE, stencilReadback );

		for ( i = 0; i < glConfig.vidWidth * glConfig.vidHeight; i++ ) {
			sum += stencilReadback[i];
		}

		backend.pc.c_overDraw += sum;
		ri.Hunk_FreeTempMemory( stencilReadback );
	}
#endif

    if (!glState.finishCalled) {
        nglFinish();
    }

    ri.GLimp_EndFrame();

    return (const void *)(cmd + 1);
}

/*
=============
RB_DrawBuffer

=============
*/
GDR_EXPORT const void *RB_DrawBuffer( const void *data ) {
	const drawBufferCmd_t *cmd;

	cmd = (const drawBufferCmd_t *)data;

	// finish any 2D drawing if needed
//	if(drawBuf.numIndices)
//		RB_EndSurface();

//	if (glConfig.ARB_framebuffer_object)
//		FBO_Bind(NULL);

	nglDrawBuffer( cmd->buffer );

	// clear screen for debugging
#if 0
	if ( r_clear->integer ) {
		nglClearColor( 1, 0, 0.5, 1 );
		nglClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	}
#endif

	return (const void *)(cmd + 1);
}

GDR_EXPORT const void *RB_SetColor(const void *data) {
	const setColorCmd_t *cmd;

	cmd = (const setColorCmd_t *)data;

	backend.color2D[0] = cmd->color[0] * 255;
	backend.color2D[1] = cmd->color[1] * 255;
	backend.color2D[2] = cmd->color[2] * 255;
	backend.color2D[3] = cmd->color[3] * 255;

	return (const void *)(cmd + 1);
}

GDR_EXPORT const void *RB_ColorMask(const void *data) {
	const colorMaskCmd_t *cmd;

	cmd = (const colorMaskCmd_t *)data;
	
#if 0
	if (glConfig.ARB_framebuffer_object) {
		// reverse color mask, so 0 0 0 0 is the default
		backend.colorMask[0] = !cmd->rgba[0];
		backend.colorMask[1] = !cmd->rgba[1];
		backend.colorMask[2] = !cmd->rgba[2];
		backend.colorMask[3] = !cmd->rgba[3];
	}
#endif

	nglColorMask(cmd->rgba[0], cmd->rgba[1], cmd->rgba[2], cmd->rgba[3]);

	return (const void *)(cmd + 1);
}

GDR_EXPORT const void *RB_DrawImage(const void *data) {
	const drawImageCmd_t *cmd;
	CShader *shader;
	uint32_t numVerts, numIndices;

#if 0
	cmd = (const drawImageCmd_t *)data;

	shader = cmd->shader;
	if ( shader != drawBuf.shader ) {
		if ( drawBuf.numIndices ) {
			RB_EndSurface();
		}
		RB_BeginSurface( shader );
	}

	RB_CheckOverflow( 4, 6 );
	numVerts = drawBuf.numVertices;
	numIndices = drawBuf.numIndices;

	drawBuf.numVertices += 4;
	drawBuf.numIndices += 6;

	drawBuf.indices[ numIndices ] = numVerts + 3;
	drawBuf.indices[ numIndices + 1 ] = numVerts + 0;
	drawBuf.indices[ numIndices + 2 ] = numVerts + 2;
	drawBuf.indices[ numIndices + 3 ] = numVerts + 2;
	drawBuf.indices[ numIndices + 4 ] = numVerts + 0;
	drawBuf.indices[ numIndices + 5 ] = numVerts + 1;

	{
		uint16_t color[4];

		VectorScale4(backend.color2D, 257, color);

		VectorCopy4(color, drawBuf.color[ numVerts ]);
		VectorCopy4(color, drawBuf.color[ numVerts + 1]);
		VectorCopy4(color, drawBuf.color[ numVerts + 2]);
		VectorCopy4(color, drawBuf.color[ numVerts + 3 ]);
	}

	drawBuf.xyz[ numVerts ][0] = cmd->x;
	drawBuf.xyz[ numVerts ][1] = cmd->y;
	drawBuf.xyz[ numVerts ][2] = 0;

	drawBuf.texCoords[ numVerts ][0] = cmd->u1;
	drawBuf.texCoords[ numVerts ][1] = cmd->v1;

	drawBuf.xyz[ numVerts + 1 ][0] = cmd->x + cmd->w;
	drawBuf.xyz[ numVerts + 1 ][1] = cmd->y;
	drawBuf.xyz[ numVerts + 1 ][2] = 0;

	drawBuf.texCoords[ numVerts + 1 ][0] = cmd->u2;
	drawBuf.texCoords[ numVerts + 1 ][1] = cmd->v1;

	drawBuf.xyz[ numVerts + 2 ][0] = cmd->x + cmd->w;
	drawBuf.xyz[ numVerts + 2 ][1] = cmd->y + cmd->h;
	drawBuf.xyz[ numVerts + 2 ][2] = 0;

	drawBuf.texCoords[ numVerts + 2 ][0] = cmd->u2;
	drawBuf.texCoords[ numVerts + 2 ][1] = cmd->v2;

	drawBuf.xyz[ numVerts + 3 ][0] = cmd->x;
	drawBuf.xyz[ numVerts + 3 ][1] = cmd->y + cmd->h;
	drawBuf.xyz[ numVerts + 3 ][2] = 0;

	drawBuf.texCoords[ numVerts + 3 ][0] = cmd->u1;
	drawBuf.texCoords[ numVerts + 3 ][1] = cmd->v2;
#endif

	return (const void *)(cmd + 1);
}


void RB_ExecuteRenderCommands( const void *data )
{
	uint64_t t1, t2;

	t1 = ri.Milliseconds();

	while ( 1 ) {
		data = PADP(data, sizeof(void *));

		switch ( *(const int *)data ) {
		case RC_SET_COLOR:
			data = RB_SetColor( data );
			break;
		case RC_DRAW_IMAGE:
			data = RB_DrawImage( data );
			break;
		case RC_DRAW_BUFFER:
			data = RB_DrawBuffer( data );
			break;
		case RC_SWAP_BUFFERS:
			data = RB_SwapBuffers( data );
			break;
//		case RC_SCREENSHOT:
//			data = RB_TakeScreenshotCmd( data );
//			break;
		case RC_COLORMASK:
			data = RB_ColorMask(data);
			break;
//		case RC_CLEARDEPTH:
//			data = RB_ClearDepth(data);
//			break;
//		case RC_POSTPROCESS:
//			data = RB_PostProcess(data);
//			break;
		case RC_END_OF_LIST:
		default:
			// finish any drawing if needed
//			if (drawBuf.numIndices)
//				RB_EndSurface();

			// stop rendering
			t2 = ri.Milliseconds();
			backend.pc.msec = t2 - t1;
			return;
		}
	}

}