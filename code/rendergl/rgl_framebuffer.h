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

extern "C" void RE_InitFramebuffers(void);
extern "C" void RE_ShutdownFramebuffers(void);
extern "C" void RE_UpdateFramebuffers(void);
extern "C" void RE_BeginFramebuffer(void);
extern "C" void RE_EndFramebuffer(void);

#endif