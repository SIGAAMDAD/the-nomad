#include "n_shared.h"
#include "n_scf.h"
#include "m_renderer.h"

framebuffer_t *bloom;
framebuffer_t *msaa;
framebuffer_t *endbuffer;

void RE_InitMsaa(void)
{
    msaa->width = r_screenwidth.i;
    msaa->height = r_screenheight.i;

    glGenFramebuffers(1, &msaa->fboId);
    glGenRenderbuffers(1, &msaa->colorId);
    glGenRenderbuffers(1, &msaa->depthId);

    glBindFramebuffer(GL_FRAMEBUFFER, msaa->fboId);

    glBindRenderbuffer(GL_RENDERBUFFER, msaa->colorId);
    glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_RGBA12, msaa->width, msaa->height);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);

    glBindRenderbuffer(GL_RENDERBUFFER, msaa->depthId);
    glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_DEPTH24_STENCIL8, msaa->width, msaa->height);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);

    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, msaa->colorId);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, msaa->depthId);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        N_Error("RE_InitMsaa: framebuffer is incomplete");

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void RE_InitDefault(void)
{
    endbuffer->width = r_screenwidth.i;
    endbuffer->height = r_screenheight.i;

    glGenFramebuffers(1, &endbuffer->fboId);
    glGenTextures(1, &endbuffer->colorId);

    glBindFramebuffer(GL_FRAMEBUFFER, endbuffer->fboId);

    glBindTexture(GL_TEXTURE_2D, endbuffer->colorId);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, endbuffer->width, endbuffer->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glBindTexture(GL_TEXTURE_2D, 0);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, endbuffer->colorId, 0);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        N_Error("RE_InitDefault: framebuffer is incomplete");

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void RE_InitFramebuffers(void)
{
    bloom = (framebuffer_t *)Hunk_Alloc(sizeof(framebuffer_t), "bloomFBO", h_low);
    msaa = (framebuffer_t *)Hunk_Alloc(sizeof(framebuffer_t), "msaaFBO", h_low);
    endbuffer = (framebuffer_t *)Hunk_Alloc(sizeof(framebuffer_t), "endbufFBO", h_low);

//    RE_InitMsaa();
//    RE_InitDefault();
}

void RE_ShutdownFramebuffers(void)
{
//    glDeleteFramebuffers(1, &msaa->fboId);
//    glDeleteFramebuffers(1, &endbuffer->fboId);
//    glDeleteTextures(1, &endbuffer->colorId);
//    glDeleteRenderbuffers(1, &msaa->colorId);
//    glDeleteRenderbuffers(1, &msaa->depthId);
}

void R_BeginBuffer(void)
{
    glBindFramebuffer(GL_FRAMEBUFFER, msaa->fboId);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glViewport(0, 0, msaa->width, msaa->height);
}

void R_EndBuffer(void)
{
    glBindFramebuffer(GL_READ_FRAMEBUFFER, msaa->fboId);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

    glBlitFramebuffer(0, 0, msaa->width, msaa->height,
                      0, 0, msaa->width, msaa->height,
                      GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT,
                      GL_NEAREST);
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
