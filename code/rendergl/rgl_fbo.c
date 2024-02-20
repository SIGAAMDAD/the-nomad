#include "rgl_local.h"
#include "rgl_fbo.h"


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

    fbo = rg.fbos[rg.numFBOs] = ri.Hunk_Alloc( sizeof(*fbo), h_low );
    N_strncpyz( fbo->name, name, sizeof(fbo->name) );
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
		}
		else {
            nglFramebufferRenderbuffer( GL_FRAMEBUFFER, attachment, GL_RENDERBUFFER, *pRenderBuffer );
		}
	}
    nglBindRenderbuffer( GL_RENDERBUFFER, 0 );
}

void FBO_AttachImage( fbo_t *fbo, texture_t *image, GLenum attachment )
{
    int32_t index;

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
    rg.msaaResolveFbo = FBO_Create( "msaaResolve", glConfig.vidWidth, glConfig.vidHeight );

    rg.hdrDepthFbo = FBO_Create( "hdrDepth", glConfig.vidWidth, glConfig.vidHeight );
    FBO_CreateBuffer( rg.hdrDepthFbo, GL_RGBA16F, GL_COLOR_ATTACHMENT0, 0 );
}

void FBO_Shutdown( void )
{
    ri.Printf( PRINT_INFO, "FBO_Shutdown: deallocating framebuffer objects...\n" );


}
