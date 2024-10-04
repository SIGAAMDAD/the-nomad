#include "rgl_local.h"

/*
=============
R_CheckFBO
=============
*/
static qboolean R_CheckFBO( const fbo_t * fbo )
{
	GLenum code = nglCheckFramebufferStatus( GL_FRAMEBUFFER );

	if ( code == GL_FRAMEBUFFER_COMPLETE ) {
		return qtrue;
	}

	// an error occurred
	switch ( code ) {
	case GL_FRAMEBUFFER_UNSUPPORTED:
		ri.Printf( PRINT_WARNING, "R_CheckFBO: (%s) Unsupported framebuffer format\n", fbo->name );
		break;
	case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
		ri.Printf( PRINT_WARNING, "R_CheckFBO: (%s) Framebuffer incomplete attachment\n", fbo->name );
		break;
	case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
		ri.Printf( PRINT_WARNING, "R_CheckFBO: (%s) Framebuffer incomplete, missing attachment\n", fbo->name );
		break;
	case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
		ri.Printf( PRINT_WARNING, "R_CheckFBO: (%s) Framebuffer incomplete, missing draw buffer\n", fbo->name );
		break;
	case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
		ri.Printf( PRINT_WARNING, "R_CheckFBO: (%s) Framebuffer incomplete, missing read buffer\n", fbo->name );
		break;
	case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
		ri.Printf( PRINT_WARNING, "R_CheckFBO: (%s) Framebuffer incomplete multisample\n", fbo->name );
		break;
	default:
		ri.Printf( PRINT_WARNING, "R_CheckFBO: (%s) unknown error 0x%X\n", fbo->name, code );
		break;
	};

	return qfalse;
}

static void FBO_Create( fbo_t *fbo, const char *name, int width, int height )
{
	int i;

	if ( strlen( name ) >= MAX_NPATH ) {
		ri.Error( ERR_DROP, "FBO_Create: \"%s\" too long", name );
	}
	if ( width <= 0 || width > glContext.maxRenderBufferSize ) {
		ri.Error( ERR_DROP, "FBO_Create: bad width %i", width );
	}
	if ( height <= 0 || height > glContext.maxRenderBufferSize ) {
		ri.Error( ERR_DROP, "FBO_Create: bad height %i", height );
	}

	if ( rg.numFBOs == MAX_RENDER_FBOs ) {
		ri.Error( ERR_DROP, "FBO_Create: MAX_RENDER_FBOs hit" );
	}

	memset( fbo, 0, sizeof( *fbo ) );	
	rg.fbos[ rg.numFBOs ] = fbo;
	rg.numFBOs++;

	N_strncpyz( fbo->name, name, sizeof( fbo->name ) );
	fbo->width = width;
	fbo->height = height;

	nglGenFramebuffers( 1, &fbo->frameBuffer );
}

static void FBO_CreateBuffer( fbo_t *fbo, int format, int32_t index, int multisample )
{
	uint32_t *pRenderBuffer;
	GLenum attachment;
	qboolean absent;

	switch ( format ) {
	case GL_RGB8:
	case GL_RGB:
		glState.memstats.estRenderbufferMemUsed += fbo->width * fbo->height * 3;
		break;
	case GL_RGBA8:
	case GL_RGBA:
		glState.memstats.estRenderbufferMemUsed += fbo->width * fbo->height * 4;
		break;
	case GL_RGB16F_ARB:
		glState.memstats.estRenderbufferMemUsed += fbo->width * fbo->height * 12;
		break;
	case GL_RGBA16F_ARB:
		glState.memstats.estRenderbufferMemUsed += fbo->width * fbo->height * 16;
		break;
	case GL_RGB32F_ARB:
		glState.memstats.estRenderbufferMemUsed += fbo->width * fbo->height * 24;
		break;
	case GL_RGBA32F_ARB:
		glState.memstats.estRenderbufferMemUsed += fbo->width * fbo->height * 32;
		break;
	default:
		// best guess
		glState.memstats.estRenderbufferMemUsed += fbo->width * fbo->height * 2;
		break;
	};

	switch ( format ) {
	case GL_RGB:
	case GL_RGBA:
	case GL_RGB8:
	case GL_RGBA8:
	case GL_RGB16F_ARB:
	case GL_RGBA16F_ARB:
	case GL_RGB32F_ARB:
	case GL_RGBA32F_ARB:
		fbo->colorFormat = format;
		pRenderBuffer = &fbo->colorBuffers[index];
		attachment = GL_COLOR_ATTACHMENT0 + index;
		break;
	case GL_DEPTH_COMPONENT:
	case GL_DEPTH_COMPONENT16_ARB:
	case GL_DEPTH_COMPONENT24_ARB:
	case GL_DEPTH_COMPONENT32_ARB:
		fbo->depthFormat = format;
		pRenderBuffer = &fbo->depthBuffer;
		attachment = GL_DEPTH_ATTACHMENT;
		break;
	case GL_STENCIL_INDEX:
	case GL_STENCIL_INDEX1:
	case GL_STENCIL_INDEX4:
	case GL_STENCIL_INDEX8:
	case GL_STENCIL_INDEX16:
		fbo->stencilFormat = format;
		pRenderBuffer = &fbo->stencilBuffer;
		attachment = GL_STENCIL_ATTACHMENT;
		break;
	case GL_DEPTH_STENCIL:
	case GL_DEPTH24_STENCIL8:
		fbo->packedDepthStencilFormat = format;
		pRenderBuffer = &fbo->packedDepthStencilBuffer;
		attachment = 0; // special for stencil and depth
		break;
	default:
		ri.Printf( PRINT_WARNING, "FBO_CreateBuffer: invalid format %d\n", format );
		return;
	};

	absent = *pRenderBuffer == 0;
	if ( absent ) {
		nglGenRenderbuffers( 1, pRenderBuffer );
	}

	GL_BindFramebuffer( GL_FRAMEBUFFER, fbo->frameBuffer );
	nglBindRenderbuffer( GL_RENDERBUFFER, *pRenderBuffer );
	if ( multisample && glContext.ARB_framebuffer_multisample ) {
//		if ( glContext.NV_framebuffer_multisample_coverage && r_multisampleType->i == AntiAlias_CSAA ) {
//			nglRenderBufferStorageMultisampleCoverageNV( GL_RENDERBUFFER, multisample, 8, format, fbo->width, fbo->height );
//		} else {
			nglRenderbufferStorageMultisample( GL_RENDERBUFFER, multisample, format, fbo->width, fbo->height );
//		}
	} else {
		nglRenderbufferStorage( GL_RENDERBUFFER, format, fbo->width, fbo->height );
	}

	if ( absent ) {
		if ( attachment == 0 ) {
			nglFramebufferRenderbuffer( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, *pRenderBuffer );
			nglFramebufferRenderbuffer( GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, *pRenderBuffer );
		} else {
			nglFramebufferRenderbuffer( GL_FRAMEBUFFER, attachment, GL_RENDERBUFFER, *pRenderBuffer );
		}
	}
	nglBindRenderbuffer( GL_RENDERBUFFER, 0 );
}

void FBO_AttachImage( fbo_t *fbo, texture_t *image, GLenum attachment )
{
	int32_t index;

	GL_BindFramebuffer( GL_FRAMEBUFFER, fbo->frameBuffer );
	GL_BindTexture( TB_DIFFUSEMAP, image );
	nglFramebufferTexture2D( GL_FRAMEBUFFER, attachment, GL_TEXTURE_2D, image->id, 0 );
	index = attachment - GL_COLOR_ATTACHMENT0;
	if ( index >= 0 && index <= 15 ) {
		fbo->colorImage[index] = image;
	}
}

void FBO_Bind( fbo_t *fbo )
{
	if ( !glContext.ARB_framebuffer_object ) {
		ri.Printf( PRINT_WARNING, "FBO_Bind() called without framebuffers enabled!\n" );
		return;
	}

	if ( glState.currentFbo == fbo ) {
		return;
	}

	GL_BindFramebuffer( GL_FRAMEBUFFER, fbo ? fbo->frameBuffer : 0 );
	glState.currentFbo = fbo;
	GL_CheckErrors();
}

static void FBO_Clear( fbo_t *fbo )
{
	int i;

	if ( !fbo ) {
		return;
	}

	for ( i = 0; i < glContext.maxColorAttachments; i++ ) {
		if ( fbo->colorBuffers[i] ) {
			nglDeleteRenderbuffers( 1, &fbo->colorBuffers[i] );
			fbo->colorBuffers[i] = 0;
		}
	}

	if ( fbo->depthBuffer ) {
		nglDeleteRenderbuffers( 1, &fbo->depthBuffer );
		fbo->depthBuffer = 0;
	}

	if ( fbo->stencilBuffer ) {
		nglDeleteRenderbuffers( 1, &fbo->stencilBuffer );
		fbo->stencilBuffer = 0;
	}

	if ( fbo->frameBuffer ) {
		nglDeleteFramebuffers( 1, &fbo->frameBuffer );
		fbo->frameBuffer = 0;
	}
}

static void FBO_List_f( void )
{
	int i, j;
	uint64_t renderBufferMemoryUsed;
	GLint type;

	renderBufferMemoryUsed = 0;

	ri.Printf( PRINT_INFO, " name             width      height\n" );
	ri.Printf( PRINT_INFO, "----------------------------------------------------------\n" );
	for ( i = 0; i < rg.numFBOs; i++ ) {
		for ( j = 0; j < glContext.maxColorAttachments; j++ ) {
			if ( rg.fbos[i]->colorBuffers[j] ) {
				GL_BindFramebuffer( GL_FRAMEBUFFER, rg.fbos[i]->frameBuffer );
	
				nglGetFramebufferAttachmentParameteriv( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + j, GL_FRAMEBUFFER_ATTACHMENT_COMPONENT_TYPE,
					&type );

				renderBufferMemoryUsed = 4 * rg.fbos[i]->width * rg.fbos[i]->height;
			}
		}
		ri.Printf( PRINT_INFO, "%s %i %i\n", rg.fbos[i]->name, rg.fbos[i]->width, rg.fbos[i]->height );
	}
	ri.Printf( PRINT_INFO, " %u total FBOs\n", rg.numFBOs );
	ri.Printf( PRINT_INFO, " %0.02lf MB render buffer memory\n", (double)( renderBufferMemoryUsed / ( 1024 * 1024 ) ) );
}

static void FBO_CreateMultisampleTexture( fbo_t *fbo, texture_t *image, GLint index, GLenum hdrFormat )
{
	int sampleCount;

	if ( image->id != 0 ) {
		nglDeleteTextures( 1, &image->id );
	}

	sampleCount = 0;
	if ( r_antialiasQuality->i == 0 ) {
		sampleCount = 2;
	} else if ( r_antialiasQuality->i == 1 ) {
		sampleCount = 4;
	} else if ( r_antialiasQuality->i == 2 ) {
		sampleCount = 8;
	}

	nglGenTextures( 1, &image->id );

	nglActiveTexture( GL_TEXTURE0 );
	nglBindTexture( GL_TEXTURE_2D_MULTISAMPLE, image->id );
//	nglTexParameteri( GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
//	nglTexParameteri( GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
//	nglTexParameteri( GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_BASE_LEVEL, 0 );
	nglTexImage2DMultisample( GL_TEXTURE_2D_MULTISAMPLE, sampleCount, hdrFormat, fbo->width, fbo->height, GL_TRUE );
	nglBindTexture( GL_TEXTURE_2D_MULTISAMPLE, 0 );

	image->evicted = qtrue;

	if ( r_loadTexturesOnDemand->i ) {
		image->handle = nglGetTextureHandleARB( image->id );
		nglMakeTextureHandleResidentARB( image->handle );
	}

	GL_BindFramebuffer( GL_FRAMEBUFFER, fbo->frameBuffer );
	nglFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + index, GL_TEXTURE_2D_MULTISAMPLE, image->id, 0 );
	GL_BindFramebuffer( GL_FRAMEBUFFER, 0 );
}

static void FBO_Init_f( void )
{
	int hdrFormat, multisample;
	int width, height;
	int fboWidth, fboHeight;
	int i;
	int sampleCount;
	qboolean restart = rg.numFBOs > 0;

	FBO_Shutdown();

	ri.Printf( PRINT_INFO, "------- FBO_Init -------\n" );

	if ( !glContext.ARB_framebuffer_object || !r_arb_framebuffer_object->i ) {
		return;
	}

	rg.numFBOs = 0;
	multisample = 0;

	GL_BindFramebuffer( GL_FRAMEBUFFER, 0 );

	GL_CheckErrors();

	R_IssuePendingRenderCommands();

	width = glConfig.vidWidth;
	height = glConfig.vidHeight;
	if ( r_multisampleType->i == AntiAlias_SSAA ) {
		if ( r_antialiasQuality->i == 0 ) {
			width *= 2;
			height *= 2;
		} else {
			width *= 4;
			height *= 4;
		}
	}

	if ( r_fixedRendering->i ) {
		fboWidth = SCREEN_WIDTH;
		fboHeight = SCREEN_HEIGHT;
	} else {
		fboWidth = glConfig.vidWidth;
		fboHeight = glConfig.vidHeight;
	}

	hdrFormat = GL_RGBA8;
	if ( r_hdr->i && glContext.ARB_texture_float ) {
		hdrFormat = GL_RGBA16F_ARB;
		ri.Printf( PRINT_DEVELOPER, "Using HDR framebuffer format.\n" );
	}

	if ( r_antialiasQuality->i == 0 ) {
		sampleCount = 2;
	} else if ( r_antialiasQuality->i == 1 ) {
		sampleCount = 4;
	} else if ( r_antialiasQuality->i == 2 ) {
		sampleCount = 8;
	}

	if ( glContext.ARB_framebuffer_multisample ) {
		nglGetIntegerv( GL_MAX_SAMPLES, &multisample );
	}

	if ( sampleCount < multisample ) {
		multisample = sampleCount;
	}

	if ( multisample < 2 || !glContext.ARB_framebuffer_blit ) {
		multisample = 0;
	}
	
	if ( r_hdr->i && r_bloom->i ) {
		for ( i = 0; i < 2; i++ ) {
			FBO_Create( &rg.bloomPingPongFbo[ i ], va( "_bloomPingPong%i", i ), fboWidth, fboHeight );
			FBO_AttachImage( &rg.bloomPingPongFbo[ i ], rg.bloomPingPongImage[ i ], GL_COLOR_ATTACHMENT0 );
			R_CheckFBO( &rg.bloomPingPongFbo[ i ] );
		}
	}

	if ( multisample && r_multisampleType->i == AntiAlias_MSAA ) {
		FBO_Create( &rg.renderFbo, "_render", fboWidth, fboHeight );
		FBO_CreateBuffer( &rg.renderFbo, hdrFormat, 0, multisample );
		if ( r_bloom->i && r_hdr->i ) {
			GL_BindFramebuffer( GL_FRAMEBUFFER, rg.renderFbo.frameBuffer );
			GLuint buffers[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
			FBO_CreateBuffer( &rg.renderFbo, hdrFormat, 1, multisample );
			nglDrawBuffers( 2, buffers );
		}
		FBO_CreateBuffer( &rg.renderFbo, GL_DEPTH24_STENCIL8, 0, multisample );
		R_CheckFBO( &rg.renderFbo );
		GL_CheckErrors();

		FBO_Create( &rg.msaaResolveFbo, "_msaaResolve", fboWidth, fboHeight );
		FBO_AttachImage( &rg.msaaResolveFbo, rg.firstPassImage, GL_COLOR_ATTACHMENT0 );
		if ( r_bloom->i && r_hdr->i ) {
			GL_BindFramebuffer( GL_FRAMEBUFFER, rg.msaaResolveFbo.frameBuffer );
			GLuint buffers[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
			FBO_AttachImage( &rg.msaaResolveFbo, rg.bloomImage, GL_COLOR_ATTACHMENT1 );
			nglDrawBuffers( 2, buffers );
		}
		R_CheckFBO( &rg.msaaResolveFbo );
		GL_CheckErrors();
	}
	else if ( multisample && r_multisampleType->i == AntiAlias_SSAA ) {
		FBO_Create( &rg.renderFbo, "_render", fboWidth, fboHeight );
		if ( r_bloom->i && r_hdr->i ) {
			GL_BindFramebuffer( GL_FRAMEBUFFER, rg.renderFbo.frameBuffer );
			GLuint buffers[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };

			FBO_AttachImage( &rg.renderFbo, rg.firstPassImage, GL_COLOR_ATTACHMENT0 );
			FBO_AttachImage( &rg.renderFbo, rg.bloomImage, GL_COLOR_ATTACHMENT1 );

			nglDrawBuffers( 2, buffers );
		} else {
			FBO_CreateBuffer( &rg.renderFbo, hdrFormat, 0, multisample );
			FBO_CreateBuffer( &rg.renderFbo, GL_DEPTH24_STENCIL8, 0, multisample );
		}
		R_CheckFBO( &rg.renderFbo );

		FBO_Create( &rg.ssaaResolveFbo, "_ssaaResolve", glConfig.vidWidth, glConfig.vidHeight );
		FBO_CreateBuffer( &rg.ssaaResolveFbo, hdrFormat, 0, multisample );
		R_CheckFBO( &rg.ssaaResolveFbo );
	}
	else if ( r_hdr->i ) {
		FBO_Create( &rg.renderFbo, "_render", fboWidth, fboHeight );
		if ( r_bloom->i ) {
			GL_BindFramebuffer( GL_FRAMEBUFFER, rg.renderFbo.frameBuffer );
			GLuint buffers[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };

			FBO_AttachImage( &rg.renderFbo, rg.firstPassImage, GL_COLOR_ATTACHMENT0 );
			FBO_AttachImage( &rg.renderFbo, rg.bloomImage, GL_COLOR_ATTACHMENT1 );

			nglDrawBuffers( 2, buffers );
		}
		else {
			FBO_AttachImage( &rg.renderFbo, rg.firstPassImage, GL_COLOR_ATTACHMENT0 );
		}
		R_CheckFBO( &rg.renderFbo );
	}
	else if ( r_fixedRendering->i ) {
		FBO_Create( &rg.renderFbo, "_render", fboWidth, fboHeight );
		FBO_AttachImage( &rg.renderFbo, rg.firstPassImage, GL_COLOR_ATTACHMENT0 );
		R_CheckFBO( &rg.renderFbo );
	}
	/*
	if ( r_fixedRendering->i ) {
		FBO_Create( &rg.scaleFbo, "_scale", glConfig.vidWidth, glConfig.vidHeight );
		FBO_AttachImage( &rg.scaleFbo, rg.renderImage, GL_COLOR_ATTACHMENT0 );
		R_CheckFBO( &rg.scaleFbo );
	}
	*/
	/*
	if ( r_multisampleType->i == AntiAlias_SMAA ) {
		rg.smaaBlendFbo = FBO_Create( "_smaaBlend", width, height );
		FBO_AttachImage( rg.smaaBlendFbo, rg.smaaBlendImage, GL_COLOR_ATTACHMENT0 );
		R_CheckFBO( rg.smaaBlendFbo );

		rg.smaaEdgesFbo = FBO_Create( "_smaaEdges", width, height );
		FBO_AttachImage( rg.smaaEdgesFbo, rg.smaaEdgesImage, GL_COLOR_ATTACHMENT0 );
		R_CheckFBO( rg.smaaEdgesFbo );

		rg.smaaWeightsFbo = FBO_Create( "_smaaWeights", width, height );
		FBO_AttachImage( rg.smaaWeightsFbo, rg.smaaWeightsImage, GL_COLOR_ATTACHMENT0 );
		R_CheckFBO( rg.smaaWeightsFbo );
	}
	*/

	// clear render buffer
	// this fixes the corrupt screen bug with r_hdr 1 on older hardware
//	if ( rg.renderFbo.frameBuffer ) {
//		GL_BindFramebuffer( GL_FRAMEBUFFER, rg.renderFbo.frameBuffer );
//		nglClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
//	}
	if ( restart ) {
		GL_BindFramebuffer( GL_FRAMEBUFFER, 0 );
		nglClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	}

	GL_CheckErrors();

	GL_BindFramebuffer( GL_FRAMEBUFFER, 0 );
	glState.currentFbo = NULL;
}

void FBO_Init( void )
{
	ri.Cmd_AddCommand( "fbolist", FBO_List_f );
	ri.Cmd_AddCommand( "fbo_restart", FBO_Init_f );

	FBO_Init_f();
}

void FBO_Shutdown( void )
{
	uint64_t i, j;
	fbo_t *fbo;

	if ( !glContext.ARB_framebuffer_object || !rg.numFBOs ) {
		return;
	}

	ri.Printf( PRINT_INFO, "------- FBO_Shutdown -------\n" );

	FBO_Bind( NULL );

	for ( i = 0; i < rg.numFBOs; i++ ) {
		fbo = rg.fbos[i];

		for ( j = 0; j < glContext.maxColorAttachments; j++ ) {
			if ( fbo->colorBuffers[j] ) {
				nglDeleteRenderbuffers( 1, &fbo->colorBuffers[j] );
			}
		}

		if ( fbo->depthBuffer ) {
			nglDeleteRenderbuffers( 1, &fbo->depthBuffer );
			fbo->depthBuffer = 0;
		}

		if ( fbo->stencilBuffer ) {
			nglDeleteRenderbuffers( 1, &fbo->stencilBuffer );
			fbo->stencilBuffer = 0;
		}

		if ( fbo->frameBuffer ) {
			nglDeleteFramebuffers( 1, &fbo->frameBuffer );
			fbo->frameBuffer = 0;
		}
	}
}

void FBO_BlitFromTexture( fbo_t *srcFbo, struct texture_s *src, vec4_t inSrcTexCorners, vec2_t inSrcTexScale, fbo_t *dst,
	ivec4_t inDstBox, struct shaderProgram_s *shaderProgram, const vec4_t inColor, int blend )
{
	ivec4_t dstBox;
	vec4_t color;
	vec4_t quadVerts[4];
	vec2_t texCoords[4];
	vec2_t invTexRes;
	fbo_t *oldFbo = glState.currentFbo;
	mat4_t projection;
	int width, height;
	vec4_t ortho;

	if ( !src ) {
		ri.Printf( PRINT_WARNING, "Tried to blit from a NULL texture! (%s)\n", srcFbo->name );
		return;
	}

	width  = dst ? dst->width  : glConfig.vidWidth;
	height = dst ? dst->height : glConfig.vidHeight;

	if ( inSrcTexCorners ) {
		VectorSet2( texCoords[0], inSrcTexCorners[0], inSrcTexCorners[1] );
		VectorSet2( texCoords[1], inSrcTexCorners[2], inSrcTexCorners[1] );
		VectorSet2( texCoords[2], inSrcTexCorners[2], inSrcTexCorners[3] );
		VectorSet2( texCoords[3], inSrcTexCorners[0], inSrcTexCorners[3] );
	}
	else {
		VectorSet2( texCoords[0], 0.0f, 1.0f );
		VectorSet2( texCoords[1], 1.0f, 1.0f );
		VectorSet2( texCoords[2], 1.0f, 0.0f );
		VectorSet2( texCoords[3], 0.0f, 0.0f );
	}

	// framebuffers are 0 bottom, Y up.
	if ( inDstBox ) {
		dstBox[0] = inDstBox[0];
		dstBox[1] = height - inDstBox[1] - inDstBox[3];
		dstBox[2] = inDstBox[0] + inDstBox[2];
		dstBox[3] = height - inDstBox[1];
	}
	else {
		VectorSet4( dstBox, 0, height, width, 0 );
	}

	if ( inSrcTexScale ) {
		VectorCopy2( invTexRes, inSrcTexScale );
	}
	else {
		VectorSet2( invTexRes, 1.0f, 1.0f );
	}

	if ( inColor ) {
		VectorCopy4( color, invTexRes );
	}
	else {
		VectorCopy4( color, colorWhite );
	}

	if ( !shaderProgram ) {
		shaderProgram = &rg.genericShader[0];
	}

	FBO_Bind( dst );

	nglViewport( 0, 0, width, height );
	nglScissor( 0, 0, width, height );

//	GLSL_UseProgram( shaderProgram );
//	GLSL_SetUniformVec4( shaderProgram, UNIFORM_COLOR, color );

//	Mat4Ortho( 0, width, height, 0, 0, 1, projection );

	GL_BindTexture( TB_COLORMAP, src );

	VectorSet4( quadVerts[0], dstBox[0], dstBox[1], 0.0f, 1.0f );
	VectorSet4( quadVerts[1], dstBox[2], dstBox[1], 0.0f, 1.0f );
	VectorSet4( quadVerts[2], dstBox[2], dstBox[3], 0.0f, 1.0f );
	VectorSet4( quadVerts[3], dstBox[0], dstBox[3], 0.0f, 1.0f );

	invTexRes[0] /= src->width;
	invTexRes[1] /= src->height;

	GL_State( blend );

	RB_RenderPass();

	FBO_Bind( oldFbo );
}

void FBO_Blit( fbo_t *src, ivec4_t inSrcBox, vec2_t srcTexScale, fbo_t *dst, ivec4_t dstBox, struct shaderProgram_s *shaderProgram, const vec4_t color, int blend )
{
	vec4_t srcTexCorners;

	if ( !src ) {
		ri.Printf( PRINT_WARNING, "Tried to blit from a NULL FBO!\n" );
		return;
	}

	if ( inSrcBox ) {
		srcTexCorners[0] =  inSrcBox[0]                / (float)src->width;
		srcTexCorners[1] = (inSrcBox[1] + inSrcBox[3]) / (float)src->height;
		srcTexCorners[2] = (inSrcBox[0] + inSrcBox[2]) / (float)src->width;
		srcTexCorners[3] =  inSrcBox[1]                / (float)src->height;
	}
	else {
		VectorSet4( srcTexCorners, 0.0f, 0.0f, 1.0f, 1.0f );
	}

//	FBO_FastBlit( src, inSrcBox, dst, dstBox, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT, GL_NEAREST );
	FBO_BlitFromTexture( src, src->colorImage[0], srcTexCorners, srcTexScale, dst, dstBox, shaderProgram, color, blend | GLS_DEPTHTEST_DISABLE );
}

void FBO_FastBlit( fbo_t *src, ivec4_t srcBox, fbo_t *dst, ivec4_t dstBox, int buffers, int filter )
{
	ivec4_t srcBoxFinal, dstBoxFinal;
	GLuint srcFb, dstFb;

	if ( !glContext.ARB_framebuffer_blit ) {
		FBO_Blit( src, srcBox, NULL, dst, dstBox, NULL, NULL, 0 );
		return;
	}

	srcFb = src ? src->frameBuffer : 0;
	dstFb = dst ? dst->frameBuffer : 0;

	if ( !srcBox ) {
		int width =  src ? src->width  : glConfig.vidWidth;
		int height = src ? src->height : glConfig.vidHeight;

		VectorSet4( srcBoxFinal, 0, 0, width, height );
	}
	else {
		VectorSet4( srcBoxFinal, srcBox[0], srcBox[1], srcBox[0] + srcBox[2], srcBox[1] + srcBox[3] );
	}

	if ( !dstBox ) {
		int width  = dst ? dst->width  : glConfig.vidWidth;
		int height = dst ? dst->height : glConfig.vidHeight;

		VectorSet4( dstBoxFinal, 0, 0, width, height );
	}
	else {
		VectorSet4( dstBoxFinal, dstBox[0], dstBox[1], dstBox[0] + dstBox[2], dstBox[1] + dstBox[3] );
	}

	GL_BindFramebuffer( GL_READ_FRAMEBUFFER, srcFb );
	GL_BindFramebuffer( GL_DRAW_FRAMEBUFFER, dstFb );
	nglBlitFramebuffer( srcBoxFinal[0], srcBoxFinal[1], srcBoxFinal[2], srcBoxFinal[3],
						  dstBoxFinal[0], dstBoxFinal[1], dstBoxFinal[2], dstBoxFinal[3],
						  buffers, filter );
	GL_BindFramebuffer( GL_FRAMEBUFFER, 0 );
	glState.currentFbo = NULL;
}

/*
void RB_ToneMap( fbo_t *hdrFbo, ivec4_t hdrBox, fbo_t *ldrFbo, ivec4_t ldrBox, int autoExposure )
{
	ivec4_t srcBox, dstBox;
	vec4_t color;
	static int lastFrameCount = 0;

	if ( autoExposure ) {
		if ( lastFrameCount == 0 || rg.frameCount < lastFrameCount || rg.frameCount - lastFrameCount > 5 ) {
			// determine average log luminance
			fbo_t *srcFbo, *dstFbo, *tmp;
			int size = 256;

			lastFrameCount = rg.frameCount;

			VectorSet4( dstBox, 0, 0, size, size );

			FBO_Blit( hdrFbo, hdrBox, NULL, rg.textureScratchFbo[0], dstBox, &rg.calclevels4xShader[0], NULL, 0 );

			srcFbo = rg.textureScratchFbo[0];
			dstFbo = rg.textureScratchFbo[1];

			// downscale to 1x1 texture
			while ( size > 1 ) {
				VectorSet4( srcBox, 0, 0, size, size );
				//size >>= 2;
				size >>= 1;
				VectorSet4( dstBox, 0, 0, size, size );

				if ( size == 1 ) {
					dstFbo = rg.targetLevelsFbo;
				}

				//FBO_Blit(targetFbo, srcBox, NULL, rg.textureScratchFbo[nextScratch], dstBox, &rg.calclevels4xShader[1], NULL, 0);
				FBO_FastBlit( srcFbo, srcBox, dstFbo, dstBox, GL_COLOR_BUFFER_BIT, GL_LINEAR );

				tmp = srcFbo;
				srcFbo = dstFbo;
				dstFbo = tmp;
			}
		}

		// blend with old log luminance for gradual change
		VectorSet4( srcBox, 0, 0, 0, 0 );

		color[0] = 
		color[1] =
		color[2] = 1.0f;
		if ( glContext.ARB_texture_float ) {
			color[3] = 0.03f;
		} else {
			color[3] = 0.1f;
		}

		FBO_Blit( rg.targetLevelsFbo, srcBox, NULL, rg.calcLevelsFbo, NULL,  NULL, color, GLS_SRCBLEND_SRC_ALPHA | GLS_DSTBLEND_ONE_MINUS_SRC_ALPHA );
	}

	// tonemap
	color[0] =
	color[1] =
	color[2] = pow( 2, r_cameraExposure->f - autoExposure ); //exp2(r_cameraExposure->value);
	color[3] = 1.0f;

	if ( autoExposure ) {
		GL_BindTexture( TB_LEVELSMAP, rg.calcLevelsImage );
	} else {
		GL_BindTexture( TB_LEVELSMAP, rg.fixedLevelsImage );
	}

	FBO_FastBlit( hdrFbo, hdrBox, ldrFbo, NULL, GL_COLOR_BUFFER_BIT, GL_LINEAR );
	FBO_Blit( hdrFbo, hdrBox, NULL, ldrFbo, ldrBox, &rg.tonemapShader, color, 0 );
}
*/

/*
=============
RB_BokehBlur


Blurs a part of one framebuffer to another.

Framebuffers can be identical. 
=============
*/
/*
void RB_BokehBlur(fbo_t *src, ivec4_t srcBox, fbo_t *dst, ivec4_t dstBox, float blur)
{
//	ivec4_t srcBox, dstBox;
	vec4_t color;
	
	blur *= 10.0f;

	if (blur < 0.004f)
		return;

	if (glContext.ARB_framebuffer_object)
	{
		// bokeh blur
		if (blur > 0.0f)
		{
			ivec4_t quarterBox;

			quarterBox[0] = 0;
			quarterBox[1] = rg.quarterFbo[0]->height;
			quarterBox[2] = rg.quarterFbo[0]->width;
			quarterBox[3] = -rg.quarterFbo[0]->height;

			// create a quarter texture
			FBO_Blit(NULL, NULL, NULL, rg.quarterFbo[0], NULL, NULL, NULL, 0);
			//FBO_FastBlit(src, srcBox, rg.quarterFbo[0], quarterBox, GL_COLOR_BUFFER_BIT, GL_LINEAR);
		}

#ifndef HQ_BLUR
		if (blur > 1.0f)
		{
			// create a 1/16th texture
			FBO_Blit(rg.quarterFbo[0], NULL, NULL, rg.textureScratchFbo[0], NULL, NULL, NULL, 0);
			//FBO_FastBlit(rg.quarterFbo[0], NULL, rg.textureScratchFbo[0], NULL, GL_COLOR_BUFFER_BIT, GL_LINEAR);
		}
#endif

		if (blur > 0.0f && blur <= 1.0f)
		{
			// Crossfade original with quarter texture
			VectorSet4(color, 1, 1, 1, blur);

			FBO_Blit(rg.quarterFbo[0], NULL, NULL, dst, dstBox, NULL, color, GLS_SRCBLEND_SRC_ALPHA | GLS_DSTBLEND_ONE_MINUS_SRC_ALPHA);
		}
#ifndef HQ_BLUR
		// ok blur, but can see some pixelization
		else if (blur > 1.0f && blur <= 2.0f)
		{
			// crossfade quarter texture with 1/16th texture
			FBO_Blit(rg.quarterFbo[0], NULL, NULL, dst, dstBox, NULL, NULL, 0);

			VectorSet4(color, 1, 1, 1, blur - 1.0f);

			FBO_Blit(rg.textureScratchFbo[0], NULL, NULL, dst, dstBox, NULL, color, GLS_SRCBLEND_SRC_ALPHA | GLS_DSTBLEND_ONE_MINUS_SRC_ALPHA);
		}
		else if (blur > 2.0f)
		{
			// blur 1/16th texture then replace
			int i;

			for (i = 0; i < 2; i++)
			{
				vec2_t blurTexScale;
				float subblur;

				subblur = ((blur - 2.0f) / 2.0f) / 3.0f * (float)(i + 1);

				blurTexScale[0] =
				blurTexScale[1] = subblur;

				color[0] =
				color[1] =
				color[2] = 0.5f;
				color[3] = 1.0f;

				if (i != 0)
					FBO_Blit(rg.textureScratchFbo[0], NULL, blurTexScale, rg.textureScratchFbo[1], NULL, &rg.bokehShader, color, GLS_SRCBLEND_ONE | GLS_DSTBLEND_ONE);
				else
					FBO_Blit(rg.textureScratchFbo[0], NULL, blurTexScale, rg.textureScratchFbo[1], NULL, &rg.bokehShader, color, 0);
			}

			FBO_Blit(rg.textureScratchFbo[1], NULL, NULL, dst, dstBox, NULL, NULL, 0);
		}
#else // higher quality blur, but slower
		else if (blur > 1.0f)
		{
			// blur quarter texture then replace
			int i;

			src = rg.quarterFbo[0];
			dst = rg.quarterFbo[1];

			VectorSet4(color, 0.5f, 0.5f, 0.5f, 1);

			for (i = 0; i < 2; i++)
			{
				vec2_t blurTexScale;
				float subblur;

				subblur = (blur - 1.0f) / 2.0f * (float)(i + 1);

				blurTexScale[0] =
				blurTexScale[1] = subblur;

				color[0] =
				color[1] =
				color[2] = 1.0f;
				if (i != 0)
					color[3] = 1.0f;
				else
					color[3] = 0.5f;

				FBO_Blit(rg.quarterFbo[0], NULL, blurTexScale, rg.quarterFbo[1], NULL, &rg.bokehShader, color, GLS_SRCBLEND_SRC_ALPHA | GLS_DSTBLEND_ONE_MINUS_SRC_ALPHA);
			}

			FBO_Blit(rg.quarterFbo[1], NULL, NULL, dst, dstBox, NULL, NULL, 0);
		}
#endif
	}
}


static void RB_RadialBlur(fbo_t *srcFbo, fbo_t *dstFbo, int passes, float stretch, float x, float y, float w, float h, float xcenter, float ycenter, float alpha)
{
	ivec4_t srcBox, dstBox;
	int srcWidth, srcHeight;
	vec4_t color;
	const float inc = 1.f / passes;
	const float mul = powf(stretch, inc);
	float scale;

	alpha *= inc;
	VectorSet4(color, alpha, alpha, alpha, 1.0f);

	srcWidth  = srcFbo ? srcFbo->width  : glConfig.vidWidth;
	srcHeight = srcFbo ? srcFbo->height : glConfig.vidHeight;

	VectorSet4(srcBox, 0, 0, srcWidth, srcHeight);

	VectorSet4(dstBox, x, y, w, h);
	FBO_Blit(srcFbo, srcBox, NULL, dstFbo, dstBox, NULL, color, 0);

	--passes;
	scale = mul;
	while (passes > 0)
	{
		float iscale = 1.f / scale;
		float s0 = xcenter * (1.f - iscale);
		float t0 = (1.0f - ycenter) * (1.f - iscale);

		srcBox[0] = s0 * srcWidth;
		srcBox[1] = t0 * srcHeight;
		srcBox[2] = iscale * srcWidth;
		srcBox[3] = iscale * srcHeight;
			
		FBO_Blit(srcFbo, srcBox, NULL, dstFbo, dstBox, NULL, color, GLS_SRCBLEND_ONE | GLS_DSTBLEND_ONE );

		scale *= mul;
		--passes;
	}
}

*/

/*
static qboolean RB_UpdateSunFlareVis(void)
{
	GLuint sampleCount = 0;
	if (!glRefConfig.occlusionQuery)
		return qtrue;

	rg.sunFlareQueryIndex ^= 1;
	if (!rg.sunFlareQueryActive[rg.sunFlareQueryIndex])
		return qtrue;

	// debug code
	if (0)
	{
		int iter;
		for (iter=0 ; ; ++iter)
		{
			GLint available = 0;
			qglGetQueryObjectiv(rg.sunFlareQuery[rg.sunFlareQueryIndex], GL_QUERY_RESULT_AVAILABLE, &available);
			if (available)
				break;
		}

		ri.Printf(PRINT_DEVELOPER, "Waited %d iterations\n", iter);
	}
	
	qglGetQueryObjectuiv(rg.sunFlareQuery[rg.sunFlareQueryIndex], GL_QUERY_RESULT, &sampleCount);
	return sampleCount > 0;
}
*/

/*
void RB_SunRays(fbo_t *srcFbo, ivec4_t srcBox, fbo_t *dstFbo, ivec4_t dstBox)
{
	vec4_t color;
	float dot;
	const float cutoff = 0.25f;
	qboolean colorize = qtrue;

//	float w, h, w2, h2;
	mat4_t mvp;
	vec4_t pos, hpos;

//	dot = DotProduct(rg.sunDirection, backend.viewParms.or.axis[0]);
	if (dot < cutoff)
		return;

//	if (!RB_UpdateSunFlareVis())
//		return;

	// From RB_DrawSun()
	{
		float dist;
		mat4_t trans, model;

//		Mat4Translation( backend.viewParms.or.origin, trans );
//		Mat4Multiply( backend.viewParms.world.modelMatrix, trans, model );
//		Mat4Multiply(backend.viewParms.projectionMatrix, model, mvp);

		dist = glState.viewData.zFar / 1.75;		// div sqrt(3)

//		VectorScale( rg.sunDirection, dist, pos );
	}

	// project sun point
	//Mat4Multiply(backend.viewParms.projectionMatrix, backend.viewParms.world.modelMatrix, mvp);
//	Mat4Transform(mvp, pos, hpos);

	// transform to UV coords
	hpos[3] = 0.5f / hpos[3];

	pos[0] = 0.5f + hpos[0] * hpos[3];
	pos[1] = 0.5f + hpos[1] * hpos[3];

	// initialize quarter buffers
	{
		float mul = 1.f;
		ivec4_t rayBox, quarterBox;
		int srcWidth  = srcFbo ? srcFbo->width  : glConfig.vidWidth;
		int srcHeight = srcFbo ? srcFbo->height : glConfig.vidHeight;

		VectorSet4(color, mul, mul, mul, 1);

		rayBox[0] = srcBox[0] * rg.sunRaysFbo->width  / srcWidth;
		rayBox[1] = srcBox[1] * rg.sunRaysFbo->height / srcHeight;
		rayBox[2] = srcBox[2] * rg.sunRaysFbo->width  / srcWidth;
		rayBox[3] = srcBox[3] * rg.sunRaysFbo->height / srcHeight;

		quarterBox[0] = 0;
		quarterBox[1] = rg.quarterFbo[0]->height;
		quarterBox[2] = rg.quarterFbo[0]->width;
		quarterBox[3] = -rg.quarterFbo[0]->height;

		// first, downsample the framebuffer
		if (colorize)
		{
			FBO_FastBlit(srcFbo, srcBox, rg.quarterFbo[0], quarterBox, GL_COLOR_BUFFER_BIT, GL_LINEAR);
			FBO_Blit(rg.sunRaysFbo, rayBox, NULL, rg.quarterFbo[0], quarterBox, NULL, color, GLS_SRCBLEND_DST_COLOR | GLS_DSTBLEND_ZERO);
		}
		else
		{
			FBO_FastBlit(rg.sunRaysFbo, rayBox, rg.quarterFbo[0], quarterBox, GL_COLOR_BUFFER_BIT, GL_LINEAR);
		}
	}

	// radial blur passes, ping-ponging between the two quarter-size buffers
	{
		const float stretch_add = 2.f/3.f;
		float stretch = 1.f + stretch_add;
		int i;
		for (i=0; i<2; ++i)
		{
			RB_RadialBlur(rg.quarterFbo[i&1], rg.quarterFbo[(~i) & 1], 5, stretch, 0.f, 0.f, rg.quarterFbo[0]->width, rg.quarterFbo[0]->height, pos[0], pos[1], 1.125f);
			stretch += stretch_add;
		}
	}
	
	// add result back on top of the main buffer
	{
		float mul = 1.f;

		VectorSet4(color, mul, mul, mul, 1);

		FBO_Blit(rg.quarterFbo[0], NULL, NULL, dstFbo, dstBox, NULL, color, GLS_SRCBLEND_ONE | GLS_DSTBLEND_ONE);
	}
}

static void RB_BlurAxis(fbo_t *srcFbo, fbo_t *dstFbo, float strength, qboolean horizontal)
{
	float dx, dy;
	float xmul, ymul;
	float weights[3] = {
		0.227027027f,
		0.316216216f,
		0.070270270f,
	};
	float offsets[3] = {
		0.f,
		1.3846153846f,
		3.2307692308f,
	};

	xmul = horizontal;
	ymul = 1.f - xmul;

	xmul *= strength;
	ymul *= strength;

	{
		ivec4_t srcBox, dstBox;
		vec4_t color;

		VectorSet4(color, weights[0], weights[0], weights[0], 1.0f);
		VectorSet4(srcBox, 0, 0, srcFbo->width, srcFbo->height);
		VectorSet4(dstBox, 0, 0, dstFbo->width, dstFbo->height);
		FBO_Blit(srcFbo, srcBox, NULL, dstFbo, dstBox, NULL, color, 0);

		VectorSet4(color, weights[1], weights[1], weights[1], 1.0f);
		dx = offsets[1] * xmul;
		dy = offsets[1] * ymul;
		VectorSet4(srcBox, dx, dy, srcFbo->width, srcFbo->height);
		FBO_Blit(srcFbo, srcBox, NULL, dstFbo, dstBox, NULL, color, GLS_SRCBLEND_ONE | GLS_DSTBLEND_ONE);
		VectorSet4(srcBox, -dx, -dy, srcFbo->width, srcFbo->height);
		FBO_Blit(srcFbo, srcBox, NULL, dstFbo, dstBox, NULL, color, GLS_SRCBLEND_ONE | GLS_DSTBLEND_ONE);

		VectorSet4(color, weights[2], weights[2], weights[2], 1.0f);
		dx = offsets[2] * xmul;
		dy = offsets[2] * ymul;
		VectorSet4(srcBox, dx, dy, srcFbo->width, srcFbo->height);
		FBO_Blit(srcFbo, srcBox, NULL, dstFbo, dstBox, NULL, color, GLS_SRCBLEND_ONE | GLS_DSTBLEND_ONE);
		VectorSet4(srcBox, -dx, -dy, srcFbo->width, srcFbo->height);
		FBO_Blit(srcFbo, srcBox, NULL, dstFbo, dstBox, NULL, color, GLS_SRCBLEND_ONE | GLS_DSTBLEND_ONE);
	}
}

static void RB_HBlur(fbo_t *srcFbo, fbo_t *dstFbo, float strength)
{
	RB_BlurAxis(srcFbo, dstFbo, strength, qtrue);
}

static void RB_VBlur(fbo_t *srcFbo, fbo_t *dstFbo, float strength)
{
	RB_BlurAxis(srcFbo, dstFbo, strength, qfalse);
}

void RB_GaussianBlur( float blur )
{
	//float mul = 1.f;
	float factor = Com_Clamp( 0.0f, 1.0f, blur );

	if ( factor <= 0.0f ) {
		return;
	}

	{
		ivec4_t srcBox, dstBox;
		vec4_t color;

		VectorSet4(color, 1, 1, 1, 1);

		// first, downsample the framebuffer
		FBO_FastBlit(NULL, NULL, rg.quarterFbo[0], NULL, GL_COLOR_BUFFER_BIT, GL_LINEAR);
		FBO_FastBlit(rg.quarterFbo[0], NULL, rg.textureScratchFbo[0], NULL, GL_COLOR_BUFFER_BIT, GL_LINEAR);

		// set the alpha channel
		nglColorMask( GL_FALSE, GL_FALSE, GL_FALSE, GL_TRUE );
		FBO_BlitFromTexture(rg.textureScratchFbo[0], rg.whiteImage, NULL, NULL, rg.textureScratchFbo[0], NULL, NULL, color, GLS_DEPTHTEST_DISABLE);
		nglColorMask( GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE );

		// blur the tiny buffer horizontally and vertically
		RB_HBlur(rg.textureScratchFbo[0], rg.textureScratchFbo[1], factor);
		RB_VBlur(rg.textureScratchFbo[1], rg.textureScratchFbo[0], factor);

		// finally, merge back to framebuffer
		VectorSet4(srcBox, 0, 0, rg.textureScratchFbo[0]->width, rg.textureScratchFbo[0]->height);
		VectorSet4(dstBox, 0, 0, glConfig.vidWidth,              glConfig.vidHeight);
		color[3] = factor;
		FBO_Blit(rg.textureScratchFbo[0], srcBox, NULL, NULL, dstBox, NULL, color, GLS_SRCBLEND_SRC_ALPHA | GLS_DSTBLEND_ONE_MINUS_SRC_ALPHA);
	}
}
*/

void RB_PostProcessSMAA( fbo_t *srcFbo )
{
	assert( !"NOPE" );

#if 0
	//
	// edges pass
	//
	GLSL_UseProgram( &rg.smaaEdgesShader );
	{
		vec2_t screenSize;
		VectorSet2( screenSize, glConfig.vidWidth, glConfig.vidHeight );
		GLSL_SetUniformVec2( &rg.smaaEdgesShader, UNIFORM_SCREEN_SIZE, screenSize );
	}
	GL_BindTexture( UNIFORM_DIFFUSE_MAP, srcFbo->colorImage[ 0 ] );
	GLSL_SetUniformTexture( &rg.smaaEdgesShader, UNIFORM_DIFFUSE_MAP, srcFbo->colorImage[ 0 ] );

	GL_BindFramebuffer( GL_FRAMEBUFFER, rg.smaaEdgesFbo.frameBuffer );
	nglClear( GL_COLOR_BUFFER_BIT );
	RB_RenderPass();

	//
	// weights pass
	//
	GLSL_UseProgram( &rg.smaaWeightsShader );
	GL_BindTexture( UNIFORM_EDGES_TEXTURE, rg.smaaEdgesImage );
	GL_BindTexture( UNIFORM_AREA_TEXTURE, rg.smaaAreaImage );
	GL_BindTexture( UNIFORM_SEARCH_TEXTURE, rg.smaaSearchImage );

	GLSL_SetUniformTexture( &rg.smaaWeightsShader, UNIFORM_EDGES_TEXTURE, rg.smaaEdgesImage );
	GLSL_SetUniformTexture( &rg.smaaWeightsShader, UNIFORM_AREA_TEXTURE, rg.smaaAreaImage );
	GLSL_SetUniformTexture( &rg.smaaWeightsShader, UNIFORM_SEARCH_TEXTURE, rg.smaaSearchImage );
	{
		vec2_t screenSize;
		VectorSet2( screenSize, glConfig.vidWidth, glConfig.vidHeight );
		GLSL_SetUniformVec2( &rg.smaaWeightsShader, UNIFORM_SCREEN_SIZE, screenSize );
	}
	GL_BindFramebuffer( GL_FRAMEBUFFER, rg.smaaWeightsFbo.frameBuffer );
	nglClear( GL_COLOR_BUFFER_BIT );
	RB_RenderPass();

	//
	// blending pass
	//
	GLSL_UseProgram( &rg.smaaBlendShader );
	GL_BindTexture( UNIFORM_BLEND_TEXTURE, rg.smaaBlendImage );
	GL_BindTexture( UNIFORM_DIFFUSE_MAP, rg.smaaWeightsImage );

	GLSL_SetUniformTexture( &rg.smaaBlendShader, UNIFORM_BLEND_TEXTURE, rg.smaaBlendImage );
	GLSL_SetUniformTexture( &rg.smaaBlendShader, UNIFORM_DIFFUSE_MAP, rg.smaaWeightsImage );
	{
		vec2_t screenSize;
		VectorSet2( screenSize, glConfig.vidWidth, glConfig.vidHeight );
		GLSL_SetUniformVec2( &rg.smaaBlendShader, UNIFORM_SCREEN_SIZE, screenSize );
	}
	GL_BindFramebuffer( GL_FRAMEBUFFER, rg.smaaBlendFbo.frameBuffer );
	RB_RenderPass();

	GLSL_UseProgram( &rg.textureColorShader );
	GLSL_SetUniformInt( &rg.textureColorShader, UNIFORM_FINALPASS, qtrue );
	GLSL_SetUniformInt( &rg.textureColorShader, UNIFORM_ANTIALIASING, r_multisampleType->i );
	FBO_Blit( &rg.smaaBlendFbo, NULL, NULL, srcFbo, NULL, &rg.textureColorShader, colorWhite, 0 );
#endif
}

#define NUM_BLUR_PASSES 10

/*
static void R_ComputePass( fbo_t *srcFbo, fbo_t *dstFbo )
{
	ri.ProfileFunctionBegin( "ColorMapCompute" );
	GLSL_UseProgram( &rg.colormapShader );
	GLSL_SetUniformInt( &rg.colormapShader, UNIFORM_USE_BLOOM, r_bloom->i );
	GLSL_SetUniformInt( &rg.colormapShader, UNIFORM_USE_HDR, r_hdr->i );
	GLSL_SetUniformInt( &rg.colormapShader, UNIFORM_ANTIALIASING, r_multisampleType->i );
	GLSL_SetUniformFloat( &rg.colormapShader, UNIFORM_EXPOSURE, r_autoExposure->f );
	GLSL_SetUniformFloat( &rg.colormapShader, UNIFORM_GAMMA, r_gammaAmount->f );
	{
		vec2_t screenSize;
		VectorSet2( screenSize, glConfig.vidWidth, glConfig.vidHeight );
		GLSL_SetUniformVec2( &rg.colormapShader, UNIFORM_SCREEN_SIZE, screenSize );
	}
	{
		uvec2_t dispatchComputeSize;
		VectorSet2( dispatchComputeSize, (GLuint)ceil( glConfig.vidWidth / 128 ), (GLuint)ceil( glConfig.vidHeight / 4 ) );
		GLSL_SetUniformUVec2( &rg.colormapShader, UNIFORM_DISPATCH_COMPUTE_SIZE, dispatchComputeSize );
	}
	GL_BindTexture( UNIFORM_DIFFUSE_MAP, rg.firstPassImage );
	GLSL_SetUniformTexture( &rg.colormapShader, UNIFORM_DIFFUSE_MAP, rg.firstPassImage );
	if ( r_bloom->i && r_hdr->i ) {
		GL_BindTexture( UNIFORM_BRIGHT_MAP, rg.bloomImage );
		GLSL_SetUniformTexture( &rg.colormapShader, UNIFORM_BRIGHT_MAP, rg.bloomImage );
	}
	nglDispatchCompute( (GLuint)ceil( glConfig.vidWidth / 128 ), (GLuint)ceil( glConfig.vidHeight / 4 ), 1 );

	GL_BindFramebuffer( GL_READ_FRAMEBUFFER, srcFbo->frameBuffer );
	GL_BindFramebuffer( GL_DRAW_FRAMEBUFFER, dstFbo ? dstFbo->frameBuffer : 0 );
	GLSL_UseProgram( &rg.textureColorShader );
	GL_BindTexture( UNIFORM_DIFFUSE_MAP, rg.computeImage );
	GLSL_SetUniformTexture( &rg.textureColorShader, UNIFORM_DIFFUSE_MAP, rg.computeImage );
	RB_RenderPass();

	ri.ProfileFunctionEnd();

	GL_BindFramebuffer( GL_FRAMEBUFFER, 0 );
}
*/

void RB_BloomPass( fbo_t *srcFbo, fbo_t *dstFbo )
{
	int i;
	int horizontal;

	if ( !rg.bloomPingPongFbo[ 0 ].frameBuffer || !rg.bloomPingPongFbo[ 1 ].frameBuffer ) {
		return;
	}

	horizontal = 0;

	FBO_FastBlit( srcFbo, NULL, &rg.bloomPingPongFbo[ 0 ], NULL, GL_COLOR_BUFFER_BIT, GL_LINEAR );

	for ( i = 0; i < NUM_BLUR_PASSES; i++ ) {
		GL_BindFramebuffer( GL_FRAMEBUFFER, rg.bloomPingPongFbo[ horizontal ].frameBuffer );
		
		// the srcFbo's colorbuffers must be set up specifically for bloom for this to work
		GLSL_UseProgram( &rg.blurShader );
		GLSL_SetUniformInt( &rg.blurShader, UNIFORM_BLUR_HORIZONTAL, horizontal );
		GLSL_SetUniformTexture( &rg.blurShader, UNIFORM_DIFFUSE_MAP, i == 0 ? srcFbo->colorImage[ 1 ] : rg.bloomPingPongImage[ !horizontal ] );
		RB_RenderPass();

		horizontal = !horizontal;
	}

	// combine hdr and bloom pass

	GL_BindFramebuffer( GL_READ_FRAMEBUFFER, rg.bloomPingPongFbo[ 0 ].frameBuffer );
	GL_BindFramebuffer( GL_DRAW_FRAMEBUFFER, dstFbo ? dstFbo->frameBuffer : 0 );

	GLSL_UseProgram( &rg.bloomResolveShader );
	GLSL_SetUniformInt( &rg.bloomResolveShader, UNIFORM_USE_HDR, r_hdr->i );
	GLSL_SetUniformInt( &rg.bloomResolveShader, UNIFORM_USE_BLOOM, r_bloom->i );
	GL_BindTexture( UNIFORM_DIFFUSE_MAP, rg.firstPassImage );
	GL_BindTexture( UNIFORM_BRIGHT_MAP, rg.bloomImage );
	GLSL_SetUniformTexture( &rg.bloomResolveShader, UNIFORM_DIFFUSE_MAP, rg.firstPassImage );
	GLSL_SetUniformTexture( &rg.bloomResolveShader, UNIFORM_BRIGHT_MAP, rg.bloomImage );
	GLSL_SetUniformFloat( &rg.bloomResolveShader, UNIFORM_EXPOSURE, r_autoExposure->f );
	GLSL_SetUniformFloat( &rg.bloomResolveShader, UNIFORM_GAMMA, r_gammaAmount->f );

	RB_RenderPass();

	GL_BindFramebuffer( GL_FRAMEBUFFER, 0 );
}

void RB_FinishPostProcess( fbo_t *srcFbo )
{
	ri.ProfileFunctionBegin( "FinishPostProcess" );
	/*
	if ( r_arb_compute_shader->i ) {
		if ( !r_bloom->i && !r_hdr->i ) {
			// apply the gamma correction here
			R_ComputePass( srcFbo, NULL );
		} else {
			GL_BindFramebuffer( GL_FRAMEBUFFER, 0 );
			nglClear( GL_COLOR_BUFFER_BIT );
			GLSL_UseProgram( &rg.textureColorShader );
			GL_BindTexture( UNIFORM_DIFFUSE_MAP, srcFbo->colorImage[ 0 ] );
			GLSL_SetUniformTexture( &rg.textureColorShader, UNIFORM_DIFFUSE_MAP, srcFbo->colorImage[ 0 ] );
			RB_RenderPass();
		}
	}
	else if ( srcFbo )
	*/
	{
		if ( r_fixedRendering->i && ( glConfig.vidWidth != SCREEN_WIDTH || glConfig.vidHeight != SCREEN_HEIGHT ) ) {
			// dynamically upscale to the real resolution from the virtual screen
			// NOTE: while this might seem slow at first, in most cases it is faster
			// especially when applying post-processing effects because we are
			// only every processing a 1280x720 screen instead of something like a 4K
			// window
			//
			// on some of the higher resolutions, performance can be tanked by about
			// 200-300 frame simply because of the amount of work each drawcall needs

			GL_BindFramebuffer( GL_FRAMEBUFFER, 0 );
			nglScissor( 0, 0, glConfig.vidWidth, glConfig.vidHeight );
			nglViewport( 0, 0, glConfig.vidWidth, glConfig.vidHeight );

			GLSL_UseProgram( &rg.bloomResolveShader );
			GLSL_SetUniformInt( &rg.bloomResolveShader, UNIFORM_USE_HDR, r_hdr->i );
			GL_BindTexture( UNIFORM_DIFFUSE_MAP, srcFbo->colorImage[ 0 ] );
			GLSL_SetUniformTexture( &rg.bloomResolveShader, UNIFORM_DIFFUSE_MAP, srcFbo->colorImage[ 0 ] );
			GLSL_SetUniformFloat( &rg.bloomResolveShader, UNIFORM_EXPOSURE, r_autoExposure->f );
			GLSL_SetUniformFloat( &rg.bloomResolveShader, UNIFORM_GAMMA, r_gammaAmount->f );

			RB_RenderPass();
		} else {
			GL_BindFramebuffer( GL_FRAMEBUFFER, 0 );
			GLSL_UseProgram( &rg.bloomResolveShader );
			GLSL_SetUniformInt( &rg.bloomResolveShader, UNIFORM_USE_HDR, r_hdr->i );
			GL_BindTexture( UNIFORM_DIFFUSE_MAP, srcFbo->colorImage[ 0 ] );
			GLSL_SetUniformTexture( &rg.bloomResolveShader, UNIFORM_DIFFUSE_MAP, srcFbo->colorImage[ 0 ] );
			GLSL_SetUniformFloat( &rg.bloomResolveShader, UNIFORM_EXPOSURE, r_autoExposure->f );
			GLSL_SetUniformFloat( &rg.bloomResolveShader, UNIFORM_GAMMA, r_gammaAmount->f );

			RB_RenderPass();
		}
	}
	ri.ProfileFunctionEnd();
}