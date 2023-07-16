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

GO_AWAY_MANGLE void RE_InitFramebuffers(void);
GO_AWAY_MANGLE void RE_ShutdownFramebuffers(void);
GO_AWAY_MANGLE void RE_UpdateFramebuffers(void);
GO_AWAY_MANGLE void RE_BeginFramebuffer(void);
GO_AWAY_MANGLE void RE_EndFramebuffer(void);

#endif