#ifndef _R_FRAMEBUFFER_
#define _R_FRAMEBUFFER_

#pragma once

#include "n_scf.h"
#include "r_texture.h"

#define GL_NO_DEPTHATTACHMENT 3
#define GL_NO_COLORATTACHMENT 3

#define GL_TEXTURE_ATTACHMENT 0
#define GL_RENDERBUFFER_ATTACHMENT 1

typedef struct
{
    vertexCache_t* cache;

    uint32_t height;
    uint32_t width;

    GLuint fboId;
    GLuint colorId;
    GLuint depthId;
} framebuffer_t;

void RE_InitFramebuffers(void);
void R_BeginFramebuffer(void);
void R_EndFramebuffer(void);
void RE_ShutdownFramebuffers(void);

#endif