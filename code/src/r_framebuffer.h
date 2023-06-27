#ifndef _R_FRAMEBUFFER_
#define _R_FRAMEBUFFER_

#pragma once

#include "n_scf.h"
#include "r_texture.h"

typedef struct
{
    vertexCache_t* cache;

    uint32_t height;
    uint32_t width;

    uint32_t fboId;
    uint32_t usedColors;
    uint32_t colorIds[32];
    uint32_t depthId;
} framebuffer_t;

void RE_InitFramebuffers(void);
void RE_BeginFramebuffer(void);
void RE_EndFramebuffer(void);
void RE_SetScreenTexture(const texture_t *tex);
void RE_ShutdownFramebuffers(void);

#endif