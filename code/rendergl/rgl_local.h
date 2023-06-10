#ifndef _RGL_LOCAL_
#define _RGL_LOCAL_

#pragma once

#include "ngl.h"
#include "rgl_image.h"
#include "rgl_cache.h"

#define RENDER_MAX_QUADS 1024
#define RENDER_MAX_VERTICES (RENDER_MAX_QUADS*4)
#define RENDER_MAX_INDICES (RENDER_MAX_VERTICES*6)

#define RENDER_MAX_IMAGES 256
#define RENDER_MAX_SHADERS 256
#define RENDER_MAX_CACHES 64
#define RENDER_MAX_FRAMEBUFFERS 32
#define RENDER_MAX_UNIFORMS 256

typedef struct
{
    GLuint imageId;
    GLuint shaderId;
    GLuint vaoId;
    GLuint vboId;
    GLuint iboId;
    GLuint fboId;

    GLuint images[RENDER_MAX_IMAGES];
    GLuint shaders[RENDER_MAX_SHADERS];
    GLuint caches[RENDER_MAX_CACHES];
    GLuint framebuffers[RENDER_MAX_FRAMEBUFFERS];

    uint32_t numImages;
    uint32_t numShaders;
    uint32_t numCaches;
    uint32_t numFramebuffers;

    uint32_t width;
    uint32_t height;

    bool initialized;
} glstate_t;

extern glstate_t glState;

typedef struct
{
    SDL_Window* window;
    SDL_GLContext context;

    char **extensions;
	uint32_t num_extensions;
	uint32_t version_major;
	uint32_t version_minor;
	
	void *libhandle;
} glcontext_t;

typedef struct
{
    char name[256];
    GLint location;
} uniform_t;

typedef struct
{
    GLuint id;

    uniform_t *uniformCache[RENDER_MAX_UNIFORMS];
} shader_t;

extern glcontext_t glContext;

#endif