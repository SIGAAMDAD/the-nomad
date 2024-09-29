#ifndef __RGL_FBO__
#define __RGL_FBO__

#pragma once

struct texture_s;
struct shaderProgram_s;

typedef struct fbo_s
{
	char            name[MAX_NPATH];

	int32_t         index;

	uint32_t        frameBuffer;

	uint32_t        colorBuffers[16];
	int32_t         colorFormat;
	texture_t       *colorImage[16];

	uint32_t        depthBuffer;
	int32_t         depthFormat;

	uint32_t        stencilBuffer;
	int32_t         stencilFormat;

	uint32_t        packedDepthStencilBuffer;
	int32_t         packedDepthStencilFormat;

    int32_t         width;
    int32_t         height;
} fbo_t;

void FBO_Init( void );
void FBO_Shutdown( void );
void FBO_FastBlit( fbo_t *src, ivec4_t srcBox, fbo_t *dst, ivec4_t dstBox, int buffers, int filter );
void FBO_Blit( fbo_t *src, ivec4_t inSrcBox, vec2_t srcTexScale, fbo_t *dst, ivec4_t dstBox, struct shaderProgram_s *shaderProgram, const vec4_t color, int blend );
void FBO_Bind( fbo_t *fbo );
void FBO_BlitFromTexture( fbo_t *srcFbo, struct texture_s *src, vec4_t inSrcTexCorners, vec2_t inSrcTexScale, fbo_t *dst,
	ivec4_t inDstBox, struct shaderProgram_s *shaderProgram, const vec4_t inColor, int blend );

void RB_ToneMap( fbo_t *hdrFbo, ivec4_t hdrBox, fbo_t *ldrFbo, ivec4_t ldrBox, int autoExposure );
void RB_BokehBlur( fbo_t *src, ivec4_t srcBox, fbo_t *dst, ivec4_t dstBox, float blur );
void RB_SunRays( fbo_t *srcFbo, ivec4_t srcBox, fbo_t *dstFbo, ivec4_t dstBox );
void RB_GaussianBlur( float blur );
void RB_PostProcessSMAA( fbo_t *srcFbo );
void RB_BloomPass( fbo_t *srcFbo, fbo_t *dstFbo );
void RB_FinishPostProcess( fbo_t *srcFbo );

#endif