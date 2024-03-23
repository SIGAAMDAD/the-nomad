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

static fbo_t *FBO_Create( const char *name, int width, int height )
{
    fbo_t *fbo;

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

    fbo = rg.fbos[rg.numFBOs] = ri.Hunk_Alloc( sizeof( *fbo ), h_low );
    N_strncpyz( fbo->name, name, sizeof( fbo->name ) );
    fbo->width = width;
    fbo->height = height;

    nglGenFramebuffers( 1, &fbo->frameBuffer );

    return fbo;
}

static void FBO_CreateBuffer( fbo_t *fbo, int format, int32_t index, int multisample )
{
	uint32_t *pRenderBuffer;
	GLenum attachment;
	qboolean absent;

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
        nglRenderbufferStorageMultisample( GL_RENDERBUFFER, multisample, format, fbo->width, fbo->height );
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
}

void FBO_Init( void )
{
	int hdrFormat, multisample;

	ri.Printf( PRINT_INFO, "------- FBO_Init -------\n" );

	if ( !glContext.ARB_framebuffer_object || !r_arb_framebuffer_object->i || !r_multisampleType->i ) {
		return;
	}

	rg.numFBOs = 0;
	multisample = 0;

	GL_CheckErrors();

	R_IssuePendingRenderCommands();

	hdrFormat = GL_RGBA8;
	if ( r_hdr->i && glContext.ARB_texture_float && r_arb_texture_float->i ) {
		hdrFormat = GL_RGBA16F_ARB;
	}

	if ( glContext.ARB_framebuffer_multisample ) {
		nglGetIntegerv( GL_MAX_SAMPLES, &multisample );
	}
	
	if ( r_multisampleAmount->i < multisample ) {
		multisample = r_multisampleAmount->i;
	}

	if ( multisample < 2 || !glContext.ARB_framebuffer_blit ) {
		multisample = 0;
	}

	if ( multisample != r_multisampleAmount->i ) {
		ri.Cvar_Set( "r_multisampleAmount", va( "%i", multisample ) );
	}

	if ( multisample && glContext.ARB_framebuffer_multisample ) {
		rg.renderFbo = FBO_Create( "render", rg.renderDepthImage->width, rg.renderDepthImage->height );
		FBO_CreateBuffer( rg.renderFbo, hdrFormat, 0, multisample );
		FBO_CreateBuffer( rg.renderFbo, GL_DEPTH_COMPONENT24, 0, multisample );
		R_CheckFBO( rg.renderFbo );

	    rg.msaaResolveFbo = FBO_Create( "msaaResolve", rg.renderDepthImage->width, rg.renderDepthImage->height );
		FBO_AttachImage( rg.msaaResolveFbo, rg.renderImage, GL_COLOR_ATTACHMENT0 );
		FBO_AttachImage( rg.msaaResolveFbo, rg.renderDepthImage, GL_DEPTH_ATTACHMENT );
	}

	// clear render buffer
	// this fixes the corrupt screen bug with r_hdr 1 on older hardware
	if ( rg.renderFbo ) {
		GL_BindFramebuffer( GL_FRAMEBUFFER, rg.renderFbo->frameBuffer );
		nglClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	}

	if ( rg.hdrDepthImage ) {
		rg.hdrDepthFbo = FBO_Create( "_hdrDepth", rg.hdrDepthImage->width, rg.hdrDepthImage->height );
		FBO_AttachImage( rg.hdrDepthFbo, rg.hdrDepthImage, GL_COLOR_ATTACHMENT0 );
		R_CheckFBO( rg.hdrDepthFbo );
	}

	if ( rg.screenSsaoImage ) {
		rg.screenSsaoFbo = FBO_Create( "_screenSsao", rg.screenSsaoImage->width, rg.screenSsaoImage->height );
		FBO_AttachImage( rg.screenSsaoFbo, rg.screenSsaoImage, GL_COLOR_ATTACHMENT0 );
	}

	GL_CheckErrors();

	GL_BindFramebuffer( GL_FRAMEBUFFER, 0 );
	glState.currentFbo = NULL;
}

void FBO_Shutdown( void )
{
	uint64_t i, j;
	fbo_t *fbo;

    ri.Printf( PRINT_INFO, "------- FBO_Shutdown -------\n" );

	if ( !glContext.ARB_framebuffer_object ) {
		return;
	}

	FBO_Bind( NULL );

	for ( i = 0; i < rg.numFBOs; i++ ) {
		fbo = rg.fbos[i];

		for ( j = 0; j < glContext.maxColorAttachments; j++ ) {
			if ( fbo->colorBuffers[i] ) {
				nglDeleteRenderbuffers( 1, &fbo->colorBuffers[i] );
			}
		}

		if ( fbo->depthBuffer ) {
			nglDeleteRenderbuffers( 1, &fbo->depthBuffer );
		}

		if ( fbo->stencilBuffer ) {
			nglDeleteRenderbuffers( 1, &fbo->stencilBuffer );
		}

		if ( fbo->frameBuffer ) {
			nglDeleteFramebuffers( 1, &fbo->frameBuffer );
		}
	}
}

void FBO_BlitFromTexture( struct texture_s *src, vec4_t inSrcTexCorners, vec2_t inSrcTexScale, fbo_t *dst, ivec4_t inDstBox, struct shaderProgram_s *shaderProgram, const vec4_t inColor, int blend )
{
	return;
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
		ri.Printf( PRINT_WARNING, "Tried to blit from a NULL texture!\n" );
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
		shaderProgram = &rg.basicShader;
	}

	FBO_Bind( dst );

	nglViewport( 0, 0, width, height );
	nglScissor( 0, 0, width, height );

	Mat4Ortho( 0, width, height, 0, 0, 1, projection );

	GL_BindTexture( TB_COLORMAP, src );

	VectorSet4(quadVerts[0], dstBox[0], dstBox[1], 0.0f, 1.0f);
	VectorSet4(quadVerts[1], dstBox[2], dstBox[1], 0.0f, 1.0f);
	VectorSet4(quadVerts[2], dstBox[2], dstBox[3], 0.0f, 1.0f);
	VectorSet4(quadVerts[3], dstBox[0], dstBox[3], 0.0f, 1.0f);

	invTexRes[0] /= src->width;
	invTexRes[1] /= src->height;

	GL_State( blend );

	GLSL_UseProgram(shaderProgram );
	
	GLSL_SetUniformMatrix4(shaderProgram, UNIFORM_MODELVIEWPROJECTION, projection);
	GLSL_SetUniformVec4(shaderProgram, UNIFORM_COLOR, color);
//	GLSL_SetUniformVec2(shaderProgram, UNIFORM_INVTEXRES, invTexRes);
//	GLSL_SetUniformVec2(shaderProgram, UNIFORM_AUTOEXPOSUREMINMAX, tr.refdef.autoExposureMinMax);
//	GLSL_SetUniformVec3(shaderProgram, UNIFORM_TONEMINAVGMAXLINEAR, tr.refdef.toneMinAvgMaxLinear);

	RB_InstantQuad2(quadVerts, texCoords);

	FBO_Bind(oldFbo);
}

void FBO_Blit( fbo_t *src, ivec4_t inSrcBox, vec2_t srcTexScale, fbo_t *dst, ivec4_t dstBox, struct shaderProgram_s *shaderProgram, const vec4_t color, int blend )
{
	return;
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

	FBO_BlitFromTexture( src->colorImage[0], srcTexCorners, srcTexScale, dst, dstBox, shaderProgram, color, blend | GLS_DEPTHTEST_DISABLE );
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
