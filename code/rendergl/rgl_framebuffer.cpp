#include "rgl_local.h"
#include "rgl_framebuffer.h"

enum {
    FBO_COLOR_MULTISAMPLE,
    FBO_COLOR_GAMMA,
    FBO_COLOR_BLOOM,
    FBO_COLOR_HDR,

    NUM_COLOR_ATTACHMENTS
};

framebuffer_t *fbo, *intermediate;
shader_t *pintShader;

GO_AWAY_MANGLE void R_InitIntermediateFramebuffer(void);
GO_AWAY_MANGLE const char *R_FramebufferFailureReason(GLenum ret);
GO_AWAY_MANGLE void R_ValidateFramebuffer(const char *name);

GO_AWAY_MANGLE uint32_t R_HDRFormat(void);
GO_AWAY_MANGLE uint32_t R_BloomFormat(void);

GO_AWAY_MANGLE void R_InitColorBuffer(uint32_t format, uint32_t *buffer, uint32_t attachment)
{
    // initialize it again
    if (!*buffer)
        nglGenRenderbuffers(1, (GLuint *)buffer);

    nglBindRenderbuffer(GL_RENDERBUFFER, *buffer);
    
    // multisampling?
    if (!N_stricmpn("MSAA", r_multisampleType->s, sizeof("MSAA")) && r_multisampleAmount->i)
        nglRenderbufferStorageMultisample(GL_RENDERBUFFER, r_multisampleAmount->i, format, fbo->width, fbo->height);
    else
        nglRenderbufferStorage(GL_RENDERBUFFER, format, fbo->width, fbo->height);

    nglBindRenderbuffer(GL_RENDERBUFFER, 0);
    nglFramebufferRenderbuffer(GL_FRAMEBUFFER, attachment, GL_RENDERBUFFER, *buffer);
}

GO_AWAY_MANGLE void R_InitDepthBuffer(uint32_t *buffer)
{
    // initialize it again
    if (!*buffer)
        nglGenRenderbuffers(1, (GLuint *)buffer);
    
    nglBindRenderbuffer(GL_RENDERBUFFER, *buffer);

    // multisampling?
    if (!N_stricmpn("MSAA", r_multisampleType->s, sizeof("MSAA")) && r_multisampleAmount->i)
        nglRenderbufferStorageMultisample(GL_RENDERBUFFER, r_multisampleAmount->i, GL_DEPTH24_STENCIL8, fbo->width, fbo->height);
    else
        nglRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, fbo->width, fbo->height);

    nglBindRenderbuffer(GL_RENDERBUFFER, 0);
    nglFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, *buffer);
}

GO_AWAY_MANGLE void R_InitFancyFramebuffer(void)
{
    uint32_t numColorBuffers;

    nglGenFramebuffers(1, (GLuint *)&fbo->fboId);

    fbo->width = r_screenwidth->i;
    fbo->height = r_screenheight->i;
    memset(fbo->colorIds, 0, sizeof(fbo->colorIds));

    nglBindFramebuffer(GL_FRAMEBUFFER, fbo->fboId);

    numColorBuffers = 0;
    if (r_multisampleAmount->i) {
        // super-sampling?
        if (!N_stricmpn("SSAA", r_multisampleType->s, sizeof("SSAA"))) {
            fbo->width *= r_multisampleAmount->i;
            fbo->height *= r_multisampleAmount->i;
        }
        ++numColorBuffers;
        R_InitColorBuffer(R_TexFormat(), &fbo->colorIds[FBO_COLOR_MULTISAMPLE], GL_COLOR_ATTACHMENT0);
    }
    if (r_gammaAmount->f > 0.0f) {
        R_InitColorBuffer(GL_SRGB8, &fbo->colorIds[FBO_COLOR_GAMMA], GL_COLOR_ATTACHMENT0 + numColorBuffers);
        ++numColorBuffers;
    }
    if (r_bloomOn->b) {
        R_InitColorBuffer(GL_RGBA16F, &fbo->colorIds[FBO_COLOR_BLOOM], GL_COLOR_ATTACHMENT0 + numColorBuffers);
        ++numColorBuffers;
    }
    if (R_HDRFormat() != 0) {
        R_InitColorBuffer(R_HDRFormat(), &fbo->colorIds[FBO_COLOR_HDR], GL_COLOR_ATTACHMENT0 + numColorBuffers);
        ++numColorBuffers;
    }
    R_InitDepthBuffer(&fbo->depthId);

    if (!pintShader)
        pintShader = R_InitShader("pint.glsl.vert", "pint.glsl.frag");
    else
        R_RecompileShader(pintShader, NULL, NULL, 0, 0);

//    GLenum dBuffers[numColorBuffers];
//    for (uint32_t i = 0; i < numColorBuffers; ++i)
//        dBuffers[i] = GL_COLOR_ATTACHMENT0 + i;
//
//    if (numColorBuffers)
//        nglDrawBuffers(numColorBuffers, dBuffers);
    nglDrawBuffer(GL_COLOR_ATTACHMENT0);

    R_ValidateFramebuffer("fancy");

    nglBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void RE_InitFramebuffers(void)
{
    fbo = (framebuffer_t *)ri.Hunk_Alloc(sizeof(framebuffer_t), "GLfbo", h_low);
    intermediate = (framebuffer_t *)ri.Hunk_Alloc(sizeof(framebuffer_t), "GLinterFbo", h_low);

    R_InitFancyFramebuffer();
    R_InitIntermediateFramebuffer();
}

void RE_ShutdownFramebuffers(void)
{
    if (!fbo || !intermediate)
        return;

    if (fbo->fboId) {
        nglDeleteFramebuffers(1, (GLuint *)&fbo->fboId);
        nglDeleteRenderbuffers(1, (GLuint *)&fbo->depthId);
        for (uint32_t i = 0; i < NUM_COLOR_ATTACHMENTS; ++i) {
            if (fbo->colorIds[i])
                nglDeleteRenderbuffers(1, (GLuint *)&fbo->colorIds[i]);
        }

        fbo->fboId = 0;
        fbo->depthId = 0;
        memset(fbo->colorIds, 0, sizeof(fbo->colorIds));
    }
    if (intermediate->fboId) {
        nglDeleteFramebuffers(1, (GLuint *)&intermediate->fboId);
        nglDeleteTextures(1, (GLuint *)&intermediate->colorIds[0]);

        intermediate->colorIds[0] = 0;
        intermediate->fboId = 0;
    }
}

GO_AWAY_MANGLE void R_InvalidateFramebuffers(void)
{
    if (fbo->fboId) {
        nglDeleteFramebuffers(1, (GLuint *)&fbo->fboId);
        nglDeleteRenderbuffers(1, (GLuint *)&fbo->depthId);
        for (uint32_t i = 0; i < NUM_COLOR_ATTACHMENTS; ++i)
            nglDeleteRenderbuffers(1, (GLuint *)&fbo->colorIds[i]);

        fbo->fboId = 0;
        fbo->depthId = 0;
        memset(fbo->colorIds, 0, sizeof(fbo->colorIds));
    }

    R_InitFancyFramebuffer();

    if (intermediate->fboId) {
        nglDeleteFramebuffers(1, (GLuint *)&intermediate->fboId);
        nglDeleteTextures(1, (GLuint *)&intermediate->colorIds[0]);

        intermediate->colorIds[0] = 0;
        intermediate->fboId = 0;
    }

    R_InitIntermediateFramebuffer();
}

/*
RE_UpdateFramebuffer: if graphics settings are changed in any way during runtime, this is called
to update and make a new valid set of framebuffers
*/
void RE_UpdateFramebuffers(void)
{
    R_InvalidateFramebuffers();
}

void RE_BeginFramebuffer(void)
{
    nglBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo->fboId);
    nglClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    nglViewport(0, 0, fbo->width, fbo->height);
    R_BindShader(pintShader);
    R_SetMatrix4(pintShader, "u_ViewProjection", renderer->camera.GetVPM());
    R_SetBool(pintShader, "u_UseGamma", (bool)r_gammaAmount->f);
    R_SetFloat(pintShader, "u_Gamma", r_gammaAmount->f);
}

void RE_EndFramebuffer(void)
{
    nglBindFramebuffer(GL_DRAW_FRAMEBUFFER, intermediate->fboId);
    nglBindFramebuffer(GL_READ_FRAMEBUFFER, fbo->fboId);

    GLenum blit = GL_NEAREST;
    
    // check if ssaa is on (intermediate's dimensions will always be the viewport's dimensions)
    if (fbo->width != intermediate->width || fbo->height != intermediate->height)
        blit = GL_LINEAR;

    nglBlitFramebuffer(0, 0, fbo->width, fbo->height,
                       0, 0, intermediate->width, intermediate->height,
                       GL_COLOR_BUFFER_BIT,
                       blit);
    
    nglBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    nglClear(GL_COLOR_BUFFER_BIT);
    nglDisable(GL_DEPTH_TEST);
    R_UnbindShader();

    // the intermediate fbo's texture isn't an engine texture, so binding it manually is the only option
    R_UnbindTexture();
    nglBindTexture(GL_TEXTURE_2D, intermediate->colorIds[0]);

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
    R_UnbindTexture();

    nglBindFramebuffer(GL_FRAMEBUFFER, 0);
}

GO_AWAY_MANGLE const char *R_FramebufferFailureReason(GLenum ret)
{
    switch (ret) {
    case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
        return "(GL_FRAMBUFFER_INCOMPLETE_MULTISAMPLE) Framebuffer's Renderbuffer attachments don't all have the same amount of samples";
    case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
        return "(GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT) Framebuffer attachment is invalid";
    case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
        return "(GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT) Framebuffer doesn't have anything attached to it";
    case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS:
        return "(GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS) Framebuffer's dimensions are invalid";
    case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS:
        return "(GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS) Framebuffer attachment is layered, but improperly done config";
    case GL_FRAMEBUFFER_UNSUPPORTED:
        return "(GL_FRAMEBUFFER_UNSUPPORTED) OpenGL implementation on current hardware doesn't support framebuffer config";
    case GL_FRAMEBUFFER_UNDEFINED:
        return "(GL_FRAMEBUFFER_UNDEFINED) Framebuffer is bound to either draw or read, but the default framebuffer doesn't exist";
    case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
        return "(GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER) There is no color buffer attached to the framebuffer's glDrawBuffer";
    case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
        return "(GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER) There is no color buffer attached to the framebuffer's glReadBuffer";
    };
}

GO_AWAY_MANGLE void R_ValidateFramebuffer(const char *name)
{
    GLenum ret = nglCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (ret != GL_FRAMEBUFFER_COMPLETE)
        ri.Con_Printf(INFO, "WARNIING: OpenGL %s framebuffer is incomplete, reason: %s", name, R_FramebufferFailureReason(ret));
}

GO_AWAY_MANGLE void R_InitIntermediateFramebuffer(void)
{
    nglGenFramebuffers(1, (GLuint *)&intermediate->fboId);
    nglGenTextures(1, (GLuint *)&intermediate->colorIds[0]);

    nglBindFramebuffer(GL_FRAMEBUFFER, intermediate->fboId);

    nglBindTexture(GL_TEXTURE_2D, intermediate->colorIds[0]);
    nglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    nglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    nglTexImage2D(GL_TEXTURE_2D, 0, R_TexFormat(), intermediate->width, intermediate->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    nglBindTexture(GL_TEXTURE_2D, 0);

    nglFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, intermediate->colorIds[0], 0);

    R_ValidateFramebuffer("intermediate");

    nglBindFramebuffer(GL_FRAMEBUFFER, 0);
}

GO_AWAY_MANGLE uint32_t R_HDRFormat(void)
{
    switch (R_GetTextureDetail()) {
    case GPUvsGod:
    case extreme:
    case high:
        return GL_RGBA32F;
    case medium:
    case low:
        return GL_RGBA16F;
    case msdos: // not allowed when using msdos level
        return 0;
    };
}

GO_AWAY_MANGLE uint32_t R_BloomFormat(void);