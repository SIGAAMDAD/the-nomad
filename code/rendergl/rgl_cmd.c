#include "rgl_local.h"

renderBackendData_t *backendData[ SMP_FRAMES ];

volatile renderCommandList_t *renderCommandList;
volatile qboolean renderThreadActive;

static void R_PerformanceCounters( void )
{
	if ( !r_speeds->i ) {
		// clear the counters even if we aren't printing
		memset( &backend.pc, 0, sizeof( backend.pc ) );
		return;
	}

	if ( r_speeds->i == 1 ) {
		ri.Printf( PRINT_INFO, "%u/%u/%u binds/indices/vertices %u dynamic buffers %u static buffers %u IBOs %u VBOs %u VAOs\n",
			backend.pc.c_bufferBinds, backend.pc.c_bufferIndices, backend.pc.c_bufferVertices, backend.pc.c_dynamicBufferDraws,
			backend.pc.c_staticBufferDraws, backend.pc.c_iboBinds, backend.pc.c_vboBinds, backend.pc.c_vaoBinds );
	}
	else if ( r_speeds->i == 2 ) {
	}
}

void R_InitCommandBuffers( void )
{
	glContext.smpActive = qfalse;
	if ( !sys_forceSingleThreading->i ) {
		ri.Printf( PRINT_INFO, "Trying SMP acceleration...\n" );
		if ( ri.GLimp_SpawnRenderThread( RB_RenderThread ) ) {
			ri.Printf( PRINT_INFO, "...succeded.\n" );
			glContext.smpActive = qtrue;
		} else {
			ri.Printf( PRINT_INFO, "...failed.\n" );
		}
	}
}

void R_ShutdownCommandBuffers( void ) {
	// kill the rendering thread
	if ( glContext.smpActive ) {
		ri.GLimp_WakeRenderer( NULL );
		glContext.smpActive = qfalse;
	}
}

void R_IssueRenderCommands( qboolean runPerformanceCounters, qboolean finalCommand )
{
	renderCommandList_t *cmdList;

	cmdList = &backendData[ rg.smpFrame ]->commandList;
	assert( cmdList );

	// add an end-of-list command
	*(renderCmdType_t *)( cmdList->buffer + cmdList->usedBytes ) = RC_END_OF_LIST;

	// clear it out, in case this is a sync and not a buffer flip
	cmdList->usedBytes = 0;

	if ( glContext.smpActive ) {
		// if the render thread is not idle, wait for it
		// sleep until the renderer has completed
		ri.GLimp_FrontEndSleep();
	}

	if ( runPerformanceCounters ) {
		R_PerformanceCounters();
	}

	// actually start the commands going
	if ( !r_skipBackEnd->i ) {
		// let it start on the new batch
		if ( sys_forceSingleThreading->i || finalCommand ) {
			RB_ExecuteRenderCommands( cmdList->buffer );
		} else {
			ri.GLimp_WakeRenderer( cmdList );
		}
	}
}

/*
============
R_IssuePendingRenderCommands

issue any pending commands and wait for them to complete
============
*/
void R_IssuePendingRenderCommands( void )
{
    if ( !rg.registered ) {
        return;
    }

    R_IssueRenderCommands( qfalse, qfalse );

	if ( !glContext.smpActive ) {
		return;
	}
	ri.GLimp_FrontEndSleep();
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

	cmdList = &backendData[ rg.smpFrame ]->commandList;
	bytes = PAD( bytes, sizeof( void * ) );

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

void RE_AddDrawWorldCmd( void )
{
	drawWorldView_t *cmd;

	if ( !rg.registered ) {
		return;
	}
	cmd = R_GetCommandBuffer( sizeof( *cmd ) );
	if ( !cmd ) {
		return;
	}
	cmd->commandID = RC_DRAW_WORLDVIEW;
}

/*
=============
RE_SetColor

Passing NULL will set the color to white
=============
*/
void RE_SetColor( const float *rgba )
{
	setColorCmd_t *cmd;

    if ( !rg.registered ) {
        return;
    }
	cmd = R_GetCommandBuffer( sizeof( *cmd ) );
	if ( !cmd ) {
		return;
	}
	cmd->commandId = RC_SET_COLOR;
	if ( !rgba ) {
		static float colorWhite[4] = { 1, 1, 1, 1 };

		rgba = colorWhite;
	}

	cmd->color[0] = rgba[0];
	cmd->color[1] = rgba[1];
	cmd->color[2] = rgba[2];
	cmd->color[3] = rgba[3];
}

void RE_DrawImage( float x, float y, float w, float h, float u1, float v1, float u2, float v2,
	nhandle_t hShader )
{
	drawImageCmd_t *cmd;

	if (!rg.registered) {
		return;
	}
	cmd = R_GetCommandBuffer(sizeof(*cmd));
	if (!cmd) {
		return;
	}

	cmd->commandId = RC_DRAW_IMAGE;
	cmd->shader = R_GetShaderByHandle(hShader);
	cmd->x = x;
	cmd->y = y;
	cmd->w = w;
	cmd->h = h;
	cmd->u1 = u1;
	cmd->v1 = v1;
	cmd->u2 = u2;
	cmd->v2 = v2;
}

/*
=============
R_AddPostProcessCmd

=============
*/
void R_AddPostProcessCmd( void ) {
	postProcessCmd_t	*cmd;

	cmd = R_GetCommandBuffer( sizeof( *cmd ) );
	if ( !cmd ) {
		return;
	}
	cmd->commandId = RC_POSTPROCESS;

	cmd->refdef = backend.refdef;
	cmd->viewData = glState.viewData;
}

void RE_BeginFrame( stereoFrame_t stereoFrame )
{
    int width, height;
    unsigned clearBits;
	int i;
	char buf[ MAX_CVAR_VALUE ], *v[4];
    mat4_t matrix;
	drawBufferCmd_t *cmd = NULL;

	ri.ProfileFunctionBegin( "BeginFrame" );

	if ( !rg.registered ) {
		return;
	}

	if ( glContext.ARB_framebuffer_object && r_arb_framebuffer_object->i && rg.renderFbo ) {
		GL_BindFramebuffer( GL_FRAMEBUFFER, rg.renderFbo->frameBuffer );
	}

    if ( glState.currentFbo ) {
        width = glState.currentFbo->width;
        height = glState.currentFbo->height;
    } else {
        width = glConfig.vidWidth;
        height = glConfig.vidHeight;
    }

	clearBits = GL_COLOR_BUFFER_BIT;

	if ( r_glDiagnostics->i ) {
		if ( !rg.beganQuery ) {
			nglBeginQuery( GL_TIME_ELAPSED, rg.queries[TIME_QUERY] );
			nglBeginQuery( GL_SAMPLES_PASSED, rg.queries[SAMPLES_QUERY] );
			nglBeginQuery( GL_PRIMITIVES_GENERATED, rg.queries[PRIMTIVES_QUERY] );
		}
		rg.beganQuery = qtrue;
	}

    if ( r_measureOverdraw->i ) {
        clearBits |= GL_STENCIL_BUFFER_BIT;
    }

	if ( r_clearColor->s ) {
		// track changes
		if ( strcmp( r_clearColor->s, glState.clearColorString ) )  {
			N_strncpyz( glState.clearColorString, r_clearColor->s, sizeof( glState.clearColorString ) );
			N_strncpyz( buf, r_clearColor->s, sizeof( buf ) );
			Com_Split( buf, v, 4, ' ' );
			for ( i = 0; i < 4 ; i++ ) {
				glState.clearColor[ i ] = N_atof( v[ i ] ) / 255.0f;
				if ( glState.clearColor[ i ] > 1.0f ) {
					glState.clearColor[ i ] = 1.0f;
				} else if ( glState.clearColor[ i ] < 0.0f ) {
					glState.clearColor[ i ] = 0.0f;
				}
			}
		}
	}

	if ( NGL_VERSION_ATLEAST( 4, 3 ) ) {
		/*
		ri.ProfileFunctionBegin( "ComputeShaderDispatch" );
		GLSL_UseProgram( &rg.computeShader );
		nglDispatchCompute( glConfig.vidWidth / 64, glConfig.vidHeight / 16, 1 );
		ri.ProfileFunctionEnd();
		*/
	}

    // clear relevant buffers
    nglClear( clearBits );
	nglActiveTexture( GL_TEXTURE0 );
	nglClearColor( glState.clearColor[0], glState.clearColor[1], glState.clearColor[2], glState.clearColor[3] );

	// setup basic state
	nglEnable( GL_BLEND );
	nglEnable( GL_SCISSOR_TEST );
	nglDisable( GL_STENCIL_TEST );
	nglDisable( GL_CULL_FACE );
	nglDisable( GL_DEPTH_TEST );

	nglBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

    // set 2D virtual screen size
    nglViewport( 0, 0, width, height );
    nglScissor( 0, 0, width, height );

	if ( !sys_forceSingleThreading->i ) {
		R_IssuePendingRenderCommands();
	}

	ri.ImGui_NewFrame();

	R_EvictUnusedTextures();

	rg.frameCount++;
	rg.frameSceneNum = 0;

	glState.finishCalled = qfalse;

	//
	// do overdraw measurement
	//
	if ( r_measureOverdraw->i ) {
		if ( glConfig.stencilBits < 4 ) {
			ri.Printf( PRINT_INFO, "Warning: not enough stencil bits to measure overdraw: %d\n", glConfig.stencilBits );
			ri.Cvar_Set( "r_measureOverdraw", "0" );
			r_measureOverdraw->modified = qfalse;
		}
		else if ( r_shadows->i == 2 ) {
			ri.Printf( PRINT_INFO, "Warning: stencil shadows and overdraw measurement are mutually exclusive\n" );
			ri.Cvar_Set( "r_measureOverdraw", "0" );
			r_measureOverdraw->modified = qfalse;
		}
		else {
			R_IssuePendingRenderCommands();
			nglEnable( GL_STENCIL_TEST );
			nglStencilMask( ~0U );
			nglClearStencil( 0U );
			nglStencilFunc( GL_ALWAYS, 0U, ~0U );
			nglStencilOp( GL_KEEP, GL_INCR, GL_INCR );
		}
		r_measureOverdraw->modified = qfalse;
	}
	else {
		// this is only reached if it was on and is now off
		if ( r_measureOverdraw->modified ) {
			R_IssuePendingRenderCommands();
			nglDisable( GL_STENCIL_TEST );
		}
		r_measureOverdraw->modified = qfalse;
	}

    //
    // texture filtering
    //
    if ( r_textureMode->modified ) {
        R_IssuePendingRenderCommands();
        R_UpdateTextures();
		r_textureMode->modified = qfalse;
    }

    // check for errors
    if ( !r_ignoreGLErrors->i ) {
        GLenum error;

        R_IssuePendingRenderCommands();
        if ( ( error = nglGetError() ) != GL_NO_ERROR ) {
            ri.Error( ERR_FATAL, "RE_BeginFrame() - glGetError() failed (0x%04x)! %s", error, GL_ErrorString( error ) );
		}
    }

	/*
    if ( glConfig.stereoEnabled ) {
		if ( !( cmd = R_GetCommandBuffer( sizeof(*cmd) ) ) )
			return;
		
		cmd->commandId = RC_DRAW_BUFFER;

		if ( stereoFrame == STEREO_LEFT ) {
			cmd->buffer = GL_BACK_LEFT;
		}
		else if ( stereoFrame == STEREO_RIGHT ) {
			cmd->buffer = GL_BACK_RIGHT;
		}
		else {
			ri.Error( ERR_FATAL, "RE_BeginFrame: Stereo is enabled, but stereoFrame was %i", stereoFrame );
		}
    }
    else {
		if ( stereoFrame != STEREO_CENTER )
			ri.Error( ERR_FATAL, "RE_BeginFrame: Stereo is disabled, but stereoFrame was %i", stereoFrame );
		if ( !( cmd = R_GetCommandBuffer( sizeof(*cmd) ) ) )
			return;
		
		if ( cmd ) {
			cmd->commandId = RC_DRAW_BUFFER;

			if ( !N_stricmp( r_drawBuffer->s, "GL_FRONT" ) )
				cmd->buffer = GL_FRONT;
			else
				cmd->buffer = GL_BACK;
		}
    }
	*/

	backend.refdef.stereoFrame = stereoFrame;

	ri.ProfileFunctionEnd();
}
/*
=============
RE_EndFrame

Returns the number of msec spent in the back end
=============
*/
void RE_EndFrame( uint64_t *frontEndMsec, uint64_t *backEndMsec, backendCounters_t *pc )
{
	swapBuffersCmd_t *cmd;

	cmd = R_GetCommandBufferReserved( sizeof( *cmd ), 0 );
	if ( !cmd ) {
		return;
	}
	cmd->commandId = RC_SWAP_BUFFERS;

	if ( pc ) {
		*pc = backend.pc;
	}

	// compute shader
	if ( NGL_VERSION_ATLEAST( 4, 3 ) ) {
		/*
//		nglMemoryBarrier( GL_UNIFORM_BARRIER_BIT | GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT | GL_ELEMENT_ARRAY_BARRIER_BIT
//			| GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_BUFFER_UPDATE_BARRIER_BIT | GL_FRAMEBUFFER_BARRIER_BIT );
		ri.ProfileFunctionBegin( "ComputeShaderFlush" );

		GLSL_UseProgram( &rg.textureColorShader );
		nglActiveTexture( GL_TEXTURE0 );
		nglBindTexture( GL_TEXTURE_2D, rg.computeShaderTexture );
		GLSL_SetUniformInt( &rg.textureColorShader, UNIFORM_DIFFUSE_MAP, TB_DIFFUSEMAP );
		RB_SetBatchBuffer( rg.renderPassVBO, NULL, sizeof( srfVert_t ), NULL, sizeof( uint32_t ) );
		backend.drawBatch.shader = rg.defaultShader;
		RB_FlushBatchBuffer();

		ri.ProfileFunctionEnd();
		*/
	}

	R_IssueRenderCommands( qtrue, qtrue );
	R_InitNextFrame();

	if ( frontEndMsec ) {
		*frontEndMsec = rg.frontEndMsec;
	}
	rg.frontEndMsec = 0;
	if ( backEndMsec ) {
		*backEndMsec = backend.pc.msec;
	}
	backend.pc.msec = 0;
}
