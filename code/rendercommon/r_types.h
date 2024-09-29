#ifndef __R_TYPES__
#define __R_TYPES__

#pragma once

#include "../engine/n_common.h"

#define MAX_VIDEO_HANDLES	16

#define RSF_ORTHO_TYPE_WORLD			0x0001 // range = [mapWidth, mapHeight]
#define RSF_ORTHO_TYPE_SCREENSPACE		0x0002 // range = [vidWidth, vidHeight]
#define RSF_ORTHO_TYPE_CORDESIAN		0x0004 // range = [1.0, 1.0] (OpenGL screen coordinates)
#define RSF_ORTHO_BITS					0x000f

#define RSF_NOWORLDMODEL				0x0010 // used when rendering the ui, but can be used with sgame
#define RSF_USE_SUNLIGHT				0x0020
#define RSF_CONFIG_BITS					0x00f0

#define MAX_VERTS_ON_POLY				1024

typedef struct {
    vec3_t xyz;
	vec3_t worldPos;
    vec2_t uv;
	color4ub_t modulate;
} polyVert_t;

typedef struct {
	nhandle_t hShader;
	uint32_t numVerts;
	polyVert_t *verts;
} poly_t;

typedef enum {
	RT_LIGHTNING,
	RT_SPRITE,			// from a sprite sheet (a mob or player)
	RT_POLY,			// used for fx

	RT_MAX_REF_ENTITY_TYPE
} renderEntityType_t;

typedef struct {
	// texturing
	nhandle_t sheetNum;  // sprite sheet index
	nhandle_t spriteId;  // sprite id

	int renderfx;

	vec3_t lightingOrigin; // RF_LIGHTING_ORIGIN
	vec3_t origin;
	uint64_t frame;

	uint32_t flags;

	// misc
	color4ub_t	shader;
	float		shaderTexCoord[2];	// texture coordinates used by tcMod entity modifiers

	// subtracted from refdef time to control effect start times
	floatint_t	shaderTime;			// -EC- set to union

	// extra sprite information
	float		radius;
	float		rotation;
	float		scale;
} renderEntityRef_t;

typedef struct {
	uint32_t x, y;
	uint32_t width, height;

	float fovX;
	float fovY;

	uint32_t flags;
	uint32_t time;
} renderSceneRef_t;

// for quake3 enthusiasts
typedef renderEntityType_t refEntityType_t;
typedef renderEntityRef_t refEntity_t;
typedef renderSceneRef_t refdef_t;

#define LIGHTING_STATIC 0
#define LIGHTING_DYNAMIC 1

typedef enum {
	AntiAlias_None,
	AntiAlias_MSAA,
	AntiAlias_SSAA,
	AntiAlias_FXAA,

	AntiAlias_TAA,
	AntiAlias_SMAA,

	AntiAlias_CSAA
} antialiasType_t;

typedef enum {
    TexDetail_MSDOS,
    TexDetail_IntegratedGPU,
    TexDetail_Normie,
    TexDetail_ExpensiveShitWeveGotHere,
    TexDetail_GPUvsGod,

	NumTexDetails
} textureDetail_t;

typedef enum {
	GeomDetail_VeryLow,
	GeomDetail_Low,
	GeomDetail_Normal,
	GeomDetail_High,
	GeomDetail_VeryHigh,

	NumGeomDetails
} geometryDetail_t;

typedef enum
{							 // [min, mag]
    TexFilter_Bilinear,      // GL_LINEAR GL_LINEAR
    TexFilter_Nearest,       // GL_NEAREST GL_NEAREST
    TexFilter_LinearNearest, // GL_LINEAR GL_NEAREST
    TexFilter_NearestLinear, // GL_NEAREST GL_LINEAR

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
	char vendor_string[MAX_STRING_CHARS];
    char renderer_string[MAX_STRING_CHARS];
    char version_string[MAX_STRING_CHARS];
    char shader_version_str[MAX_STRING_CHARS];
    char extensions_string[BIG_INFO_STRING];

    int maxTextureSize;		// queried from GL
    int maxTextureUnits;	// can only be 16 for the forseeable future

	int colorBits, depthBits, stencilBits;

    glDriverType_t driverType;
    glHardwareType_t hardwareType;

    float windowAspect;
	// aspect is the screen's physical width / height, which may be different
	// than scrWidth / scrHeight if the pixels are non-square
	// normal screens should be 4/3, but wide aspect monitors may be 16/9
    int vidWidth, vidHeight;

    int displayFrequency;

    qboolean isFullscreen;
    qboolean deviceSupportsGamma;
    qboolean stereoEnabled;
    qboolean usingRenderThread;		// dedicated render thread (not yet implemented)
} gpuConfig_t;

#if defined(_WIN32)
#define OPENGL_DRIVER_NAME	"opengl32"
#elif defined(MACOS_X)
#define OPENGL_DRIVER_NAME	"/System/Library/Frameworks/OpenGL.framework/Libraries/libGL.dylib"
#else
#define OPENGL_DRIVER_NAME	"libGL.so.1"
#endif

#endif
