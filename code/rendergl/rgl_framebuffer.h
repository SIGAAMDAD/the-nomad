#ifndef _RGL_FRAMEBUFFER_
#define _RGL_FRAMEBUFFER_

#pragma once

typedef enum {
    SSAA, // super sampling (FSAA)
    MSAA, // simple multisampling
    FXAA, // fast approximate
    MLAA, // morphological
    SMAA, // subpixel morphological
    TSAA // temporal sample
} multisamplingType_t;

void RE_InitFramebuffers(void);
void RE_ShutdownFramebuffers(void);
void RE_UpdateFramebuffers(void);
void RE_BeginFramebuffer(void);
void RE_EndFramebuffer(void);

#endif