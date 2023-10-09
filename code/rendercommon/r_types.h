#ifndef __R_TYPES__
#define __R_TYPES__

#pragma once

typedef struct {
    vec3_t xyz;
    vec3_t color;
    vec2_t uv;
} polyVert_t;

typedef enum {
	STEREO_CENTER,
	STEREO_LEFT,
	STEREO_RIGHT
} stereoFrame_t;

/*
** glconfig_t
**
** Contains variables specific to the OpenGL configuration
** being run right now.  These are constant once the OpenGL
** subsystem is initialized.
*/
typedef enum {
	TC_NONE,
	TC_S3TC,  // this is for the GL_S3_s3tc extension.
	TC_S3TC_ARB  // this is for the GL_EXT_texture_compression_s3tc extension.
} textureCompression_t;

typedef enum {
	GLDRV_ICD,					// driver is integrated with window system
								// WARNING: there are tests that check for
								// > GLDRV_ICD for minidriverness, so this
								// should always be the lowest value in this
								// enum set
	GLDRV_STANDALONE,			// driver is a non-3Dfx standalone driver
	GLDRV_VOODOO				// driver is a 3Dfx standalone driver
} glDriverType_t;

typedef enum {
	GLHW_GENERIC,			// where everything works the way it should
	GLHW_3DFX_2D3D,			// Voodoo Banshee or Voodoo3, relevant since if this is
							// the hardware type then there can NOT exist a secondary
							// display adapter
	GLHW_RIVA128,			// where you can't interpolate alpha
	GLHW_RAGEPRO,			// where you can't modulate alpha on alpha textures
	GLHW_PERMEDIA2			// where you don't have src*dst
} glHardwareType_t;

typedef struct {
    int maxTextureSize; // queried from GL
    int maxTextureUnits; // can only be 16 for the forseeable future

    glDriverType_t driverType;
    glHardwareType_t hardwareType;

    float windowAspect;
    int vidWidth, vidHeight;
    int colorBits, depthBits, stencilBits;
    int displayFrequency;;

    qboolean isFullscreen;
    qboolean deviceSupportsGamma;
    qboolean stereoEnabled;
    qboolean usingRenderThread;
} gpuConfig_t;

#if defined(_WIN32)
#define OPENGL_DRIVER_NAME	"opengl32"
#elif defined(MACOS_X)
#define OPENGL_DRIVER_NAME	"/System/Library/Frameworks/OpenGL.framework/Libraries/libGL.dylib"
#else
#define OPENGL_DRIVER_NAME	"libGL.so.1"
#endif

#endif