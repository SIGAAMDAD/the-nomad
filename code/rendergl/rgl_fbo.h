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

#endif