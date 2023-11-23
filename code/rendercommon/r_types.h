#ifndef __R_TYPES__
#define __R_TYPES__

#pragma once

#define MAX_VIDEO_HANDLES	16

#define RSF_USE_ORTHO_ASPECT	0x0001 // rendering to the tilemap?
#define RSF_USE_ORTHO_UI		0x0002 // rendering to the ui?
#define RSF_NOWORLDMODEL		0x0004 // used when rendering the ui, but can be used with sgame

typedef struct {
    vec3_t xyz;
    vec2_t uv;
	color4ub_t modulate;
} polyVert_t;

typedef struct {
	nhandle_t hShader;
	uint32_t numVerts;
	polyVert_t *verts;
} poly_t;

typedef struct {
	vec3_t origin;
	vec2_t texCoords[4];
	color4ub_t modulate;

	nhandle_t hShader;

	uint32_t flags;
} renderEntityRef_t;

typedef struct {
	vec3_t origin;

	float width;
	float height;

	float realWidth;
	float realHeight;
} renderCameraDef_t;

typedef struct {
	uint32_t x, y;
	uint32_t width, height;

	uint32_t flags;
	uint32_t time;

	renderCameraDef_t *camera; // only used when rendering with tilemaps
} renderSceneRef_t;

typedef renderSceneRef_t refdef_t;

#ifdef __cplusplus
typedef enum : uint32_t
#else
typedef enum
#endif
{
	AntiAlias_2xMSAA,
	AntiAlias_4xMSAA,
	AntiAlias_8xMSAA,
	AntiAlias_16xMSAA,
	AntiAlias_32xMSAA,
	AntiAlias_2xSSAA,
	AntiAlias_4xSSAA,
	AntiAlias_DSSAA
} antialiasType_t;

#ifdef __cplusplus
typedef enum : uint32_t
#else
typedef enum
#endif
{
    TexDetail_MSDOS,
    TexDetail_IntegratedGPU,
    TexDetail_Normie,
    TexDetail_ExpensiveShitWeveGotHere,
    TexDetail_GPUvsGod,

	NumTexDetails
} textureDetail_t;

#ifdef __cplusplus
typedef enum : uint32_t
#else
typedef enum
#endif
{							// [min, mag]
    TexFilter_Linear,       // GL_LINEAR GL_LINEAR
    TexFilter_Nearest,      // GL_NEAREST GL_NEAREST
    TexFilter_Bilinear,     // GL_NEAREST GL_LINEAR
    TexFilter_Trilinear,    // GL_LINEAR GL_NEAREST

	NumTexFilters
} textureFilter_t;

/*
** gpuConfig_t
**
** Contains variables specific to the OpenGL configuration
** being run right now.  These are constant once the OpenGL
** subsystem is initialized.
*/
typedef enum {
	TC_NONE,
	TC_S3TC,  // this is for the GL_S3_s3tc extension.
	TC_S3TC_ARB  // this is for the GL_EXT_texture_compression_s3tc extension.
} glTextureCompression_t;

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

typedef enum {
	STEREO_CENTER,
	STEREO_LEFT,
	STEREO_RIGHT
} stereoFrame_t;

typedef struct {
    int maxTextureSize; // queried from GL
    int maxTextureUnits; // can only be 16 for the forseeable future

    glDriverType_t driverType;
    glHardwareType_t hardwareType;

    float windowAspect;
    int vidWidth, vidHeight;
    int colorBits, depthBits, stencilBits;
    int displayFrequency;

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

enum {
	RF_LIGHTING_ORIGIN,
};

typedef struct {
	vec3_t origin;
} refEntity_t;

#endif
