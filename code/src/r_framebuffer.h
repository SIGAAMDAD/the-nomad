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
    shader_t* shader;
    texture_t* screenTexture;

    uint32_t height;
    uint32_t width;

    GLuint fboId;
    GLuint colorId;
    GLuint depthId;
    
    GLuint defId;
    GLuint defTex;
} framebuffer_t;

void R_SetFramebuffer(framebuffer_t* fbo);
framebuffer_t* R_CreateFramebuffer(const char *name);

#endif