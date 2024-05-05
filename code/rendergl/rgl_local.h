#ifndef __RGL_LOCAL__
#define __RGL_LOCAL__

#pragma once

#include "../engine/n_shared.h"
#include "../rendercommon/r_public.h"
#include "../rendercommon/r_types.h"
#include "../engine/gln_files.h"
#include "../engine/n_cvar.h"
#include "ngl.h"

// cglm -- because moi am stupid
/*
#include <cglm/types.h>
#include <cglm/simd/sse2/mat4.h>
#include <cglm/cglm.h>
#include <cglm/affine-mat.h>
#include <cglm/affine.h>
*/

#define SHADER_MAX_VERTEXES 1000
#define SHADER_MAX_INDEXES (6*SHADER_MAX_VERTEXES)

#define MAX_RENDER_BUFFERS 2048
#define MAX_RENDER_PROGRAMS 2048
#define MAX_RENDER_TEXTURES 2048
#define MAX_RENDER_SHADERS 2048
#define MAX_RENDER_FBOs 64

#define	FOG_TABLE_SIZE		256
#define FUNCTABLE_SIZE		1024
#define FUNCTABLE_SIZE2		10
#define FUNCTABLE_MASK		(FUNCTABLE_SIZE-1)

#define DYN_BUFFER_SIZE ( 4 * 1024 * 1024 )
#define DYN_BUFFER_SEGMENTS 4

#define GLN_INDEX_TYPE GL_UNSIGNED_INT
typedef uint32_t glIndex_t;

// set a hard limit of 2 MiB per uniform buffer
#define MAX_UNIFORM_BUFFER_SIZE ( 2*1024*1024 )
#define MAX_UNIFORM_BUFFERS 512
#define MAX_VERTEX_BUFFERS 1024
#define MAX_FRAMEBUFFERS 64
#define MAX_GLSL_OBJECTS 128
#define MAX_SHADERS 1024
#define MAX_TEXTURES 1024
#define MAX_RENDER_SPRITESHEETS 1024
#define MAX_SHADER_STAGES 8

#define BUFFER_OFFSET(x) ((char *)NULL + (x))

#define MAX_DLIGHTS 1024
#define MAX_RENDER_ENTITIES 1024

#define MAX_TEXTURE_UNITS 16

#define PRINT_INFO 0
#define PRINT_DEVELOPER 1
#define PRINT_WARNING 2
#define PRINT_ERROR 3

#define MAX_CALC_PSHADOWS    64
#define MAX_DRAWN_PSHADOWS    16 // do not increase past 32, because bit flags are used on surfaces
#define PSHADOW_MAP_SIZE      512

#define NGL_VERSION_ATLEAST(major,minor) (glContext.versionMajor > major || (glContext.versionMajor == major && glContext.versionMinor >= minor))

#define MAX_DRAW_VERTICES (MAX_INT/sizeof(drawVert_t))
#define MAX_DRAW_INDICES (MAX_INT/sizeof(glIndex_t))

// per drawcall batch
#define MAX_BATCH_QUADS (1024*1024)
#define MAX_BATCH_VERTICES (MAX_BATCH_QUADS*4)
#define MAX_BATCH_INDICES (MAX_BATCH_QUADS*6)

// any change in the LIGHTMAP_* defines here MUST be reflected in
// R_FindShader()
#define LIGHTMAP_2D         -4	// shader is for 2D rendering
#define LIGHTMAP_BY_VERTEX  -3	// pre-lit triangle models
#define LIGHTMAP_WHITEIMAGE -2
#define LIGHTMAP_NONE       -1

typedef enum {
    MI_NONE,
    MI_NVX,
    MI_ATI
} gpuMemInfo_t;

typedef enum {
	TCR_NONE = 0x0000,
	TCR_RGTC = 0x0001,
	TCR_BPTC = 0x0002,
} textureCompressionRef_t;

typedef enum {
    GL_DBG_NONE = 0,

    GL_DBG_KHR,
    GL_DBG_AMD,
    GL_DBG_ARB
} glDebugType_t;

typedef struct
{
    char vendor[1024];
    char renderer[1024];
    char version_str[1024];
    char glsl_version_str[1024];
    char extensions[8192];
    float version_f;
    int versionMajor, versionMinor;
    int glslVersionMajor, glslVersionMinor;
    int numExtensions;
    glTextureCompression_t textureCompression;
    textureCompressionRef_t textureCompressionRef;
    gpuMemInfo_t memInfo;
    glDebugType_t debugType;
    qboolean nonPowerOfTwoTextures;
    qboolean stereo;
    qboolean intelGraphics;
    qboolean swizzleNormalmap;

    int maxTextureUnits;
    int maxTextureSize;
    int vboTarget;
    int iboTarget;
    int maxSamples;
    int maxColorAttachments;
    int maxRenderBufferSize;
    float maxAnisotropy;

    qboolean ARB_gl_spirv;
    qboolean ARB_texture_filter_anisotropic;
    qboolean ARB_vertex_buffer_object;
    qboolean ARB_buffer_storage;
    qboolean ARB_map_buffer_range;
    qboolean ARB_texture_float;
    qboolean ARB_vertex_array_object;
    qboolean ARB_framebuffer_object;
    qboolean ARB_framebuffer_sRGB;
    qboolean ARB_framebuffer_blit;
    qboolean ARB_framebuffer_multisample;
    qboolean ARB_vertex_shader;
    qboolean ARB_texture_compression;
    qboolean ARB_sync;
    qboolean ARB_shader_storage_buffer_object;
} glContext_t;

typedef struct {
    renderEntityRef_t e;

    qboolean needDlights; // true for anything submitted throught RE_AddEntityToScene
    qboolean lightingCalculated;
    int ambientLightInt;  // 32 bit rgba packed
    vec3_t ambientLight;  // color normalized to 0-255
    vec3_t lightDir;      // normalized direction towards light, in world space
    vec3_t directedLight;
    qboolean intShaderTime;
} renderEntityDef_t;

typedef enum
{
	IMGTYPE_COLORALPHA, // for color, lightmap, diffuse, and specular
	IMGTYPE_NORMAL,
	IMGTYPE_NORMALHEIGHT,
	IMGTYPE_DELUXE, // normals are swizzled, deluxe are not
} imgType_t;

typedef enum
{
	IMGFLAG_NONE           = 0x0000,
	IMGFLAG_MIPMAP         = 0x0001,
	IMGFLAG_PICMIP         = 0x0002,
	IMGFLAG_CUBEMAP        = 0x0004,
	IMGFLAG_NO_COMPRESSION = 0x0008,
	IMGFLAG_NOLIGHTSCALE   = 0x0010,
	IMGFLAG_CLAMPTOEDGE    = 0x0020,
	IMGFLAG_GENNORMALMAP   = 0x0040,
	IMGFLAG_LIGHTMAP       = 0x0080,
	IMGFLAG_NOSCALE        = 0x0100,
	IMGFLAG_CLAMPTOBORDER  = 0x0200,
    IMGFLAG_NOWRAP         = 0x0400
} imgFlags_t;

typedef struct {
    GLuint id;
    GLuint texunit;
} sampler_t;

typedef struct texture_s
{
    char *imgName;              // image path, including extension
    struct texture_s *next;     // for hash search
    struct texture_s *list;     // for listing

    int width, height;          // source image
    int uploadWidth;            // after power of two but not including clamp to MAX_TEXTURE_SIZE
    int uploadHeight;
    GLuint id;                  // GL texture binding

    uint64_t frameUsed;

    GLint internalFormat;
    imgType_t type;
    imgFlags_t flags;
} texture_t;

#include "rgl_fbo.h"

enum {
	ATTRIB_INDEX_POSITION       = 0,
	ATTRIB_INDEX_TEXCOORD       = 1,
    ATTRIB_INDEX_COLOR          = 2,
    ATTRIB_INDEX_WORLDPOS       = 3,
    ATTRIB_INDEX_LIGHTCOORD     = 4,
    ATTRIB_INDEX_NORMAL         = 5,
    ATTRIB_INDEX_TANGENT        = 6,
    ATTRIB_INDEX_BITANGENT      = 7,
	
	ATTRIB_INDEX_COUNT
};

enum
{
    ATTRIB_POSITION             = BIT( ATTRIB_INDEX_POSITION ),
    ATTRIB_NORMAL               = BIT( ATTRIB_INDEX_NORMAL ),
	ATTRIB_TEXCOORD             = BIT( ATTRIB_INDEX_TEXCOORD ),
    ATTRIB_COLOR                = BIT( ATTRIB_INDEX_COLOR ),
    ATTRIB_WORLDPOS             = BIT( ATTRIB_INDEX_WORLDPOS ),
    ATTRIB_LIGHTCOORD           = BIT( ATTRIB_INDEX_LIGHTCOORD ),
    ATTRIB_TANGENT              = BIT( ATTRIB_INDEX_TANGENT ),
    ATTRIB_BITANGENT            = BIT( ATTRIB_INDEX_BITANGENT ),

	ATTRIB_BITS =
        ATTRIB_POSITION |
        ATTRIB_TEXCOORD | 
        ATTRIB_COLOR |
        ATTRIB_NORMAL |
        ATTRIB_WORLDPOS |
        ATTRIB_LIGHTCOORD
};

typedef enum {
    GLSL_INT = 0,
    GLSL_FLOAT,
    GLSL_VEC2,
    GLSL_VEC3,
    GLSL_VEC4,
    GLSL_VEC5,
    GLSL_MAT16,
    GLSL_BUFFER, // uniform buffer -- special case
} glslType_t;

typedef enum {
    UNIFORM_DIFFUSE_MAP = 0,
	UNIFORM_LIGHT_MAP,
	UNIFORM_NORMAL_MAP,
	UNIFORM_DELUXE_MAP,
	UNIFORM_SPECULAR_MAP,

	UNIFORM_TEXTURE_MAP,
	UNIFORM_LEVELS_MAP,

	UNIFORM_SCREENIMAGE_MAP,
	UNIFORM_SCREENDEPTH_MAP,

	UNIFORM_SHADOW_MAP,
	UNIFORM_SHADOW_MAP2,
	UNIFORM_SHADOW_MAP3,
	UNIFORM_SHADOW_MAP4,

	UNIFORM_SHADOW_MVP,
	UNIFORM_SHADOW_MVP2,
	UNIFORM_SHADOW_MVP3,
	UNIFORM_SHADOW_MVP4,

	UNIFORM_ENABLE_TEXTURES,

	UNIFORM_DIFFUSE_TEXMATRIX,
	UNIFORM_DIFFUSE_TEXOFFTURB,

	UNIFORM_TCGEN0,
	UNIFORM_TCGEN0VECTOR0,
	UNIFORM_TCGEN0VECTOR1,

	UNIFORM_DEFORMGEN,
	UNIFORM_DEFORMPARAMS,

	UNIFORM_COLORGEN,
	UNIFORM_ALPHAGEN,
	UNIFORM_COLOR,
	UNIFORM_BASECOLOR,
	UNIFORM_VERTCOLOR,

	UNIFORM_DLIGHTINFO,
	UNIFORM_LIGHTFORWARD,
	UNIFORM_LIGHTUP,
	UNIFORM_LIGHTRIGHT,
	UNIFORM_LIGHTORIGIN,
	UNIFORM_MODELLIGHTDIR,
	UNIFORM_LIGHTRADIUS,
	UNIFORM_AMBIENTLIGHT,
	UNIFORM_DIRECTEDLIGHT,

	UNIFORM_MODELVIEWPROJECTION,

	UNIFORM_TIME,
	UNIFORM_VERTEXLERP,
	UNIFORM_NORMAL_SCALE,
	UNIFORM_SPECULAR_SCALE,

	UNIFORM_VIEWINFO, // znear, zfar, width/2, height/2
	UNIFORM_VIEWORIGIN,
	UNIFORM_LOCALVIEWORIGIN,
	UNIFORM_VIEWFORWARD,
	UNIFORM_VIEWLEFT,
	UNIFORM_VIEWUP,

	UNIFORM_INVTEXRES,
	UNIFORM_AUTOEXPOSUREMINMAX,
	UNIFORM_TONEMINAVGMAXLINEAR,

	UNIFORM_PRIMARYLIGHTORIGIN,
	UNIFORM_PRIMARYLIGHTCOLOR,
	UNIFORM_PRIMARYLIGHTAMBIENT,
	UNIFORM_PRIMARYLIGHTRADIUS,

	UNIFORM_ALPHATEST,

    UNIFORM_GAMMA,

    UNIFORM_NUM_LIGHTS,

    UNIFORM_EXPOSURE,
    UNIFORM_SCREEN_SIZE,
    UNIFORM_SHARPENING,

    UNIFORM_LIGHTDATA,

    UNIFORM_COUNT
} uniform_t;

typedef struct shaderProgram_s
{
    char name[MAX_NPATH];

    char *compressedVSCode;
    char *compressedFSCode;

    uint32_t vertexBufLen;
    uint32_t fragmentBufLen;

    uint32_t numBuffers;

    uint32_t programId;
    uint32_t vertexId;
    uint32_t fragmentId;
    uint32_t attribBits; // vertex array attribute flags

    // uniforms
    GLint uniforms[UNIFORM_COUNT];
    int16_t uniformBufferOffsets[UNIFORM_COUNT]; // max 32767/64=511 uniforms
    char *uniformBuffer;
} shaderProgram_t;

/*
* GLSL uniform buffer alignment doesn't play well with
* maplight_t or dlight_t
*/
typedef struct {
    vec4_t color;
    uvec2_t origin;
    float brightness;
    float range;
    float linear;
    float quadratic;
    float constant;
    int type;
} shaderLight_t;

typedef struct {
    char *name;
    shaderProgram_t *shader;
    byte *data;
    qboolean externalBuffer;
    uint32_t id;
    uint32_t binding;
    uint64_t size;
} uniformBuffer_t;

typedef enum {
    DL_POINT,       // point light
    DL_SPOT,        // spot light
    DL_SKY,         // sky light
    DL_DIR,         // directional light
} dlightType_t; 

typedef struct dlight_s
{
    vec4_t color;
    vec3_t pos;

    float brightness;
    float diffuse;
    float specular;
    float ambient;

    dlightType_t ltype;

    struct dlight_s *next;
    struct dlight_s *prev;
} dlight_t;

// normal is unused
typedef struct
{
    vec3_t          xyz;
    vec3_t          worldPos;
	vec2_t          uv;
	vec2_t          lightmap;
	int16_t         normal[4];
	int16_t         tangent[4];
	int16_t         lightdir[4];
	uint16_t        color[4];
} drawVert_t;

// when sgame directly specifies a polygon, it becomes a srfPoly_t
// as soon as it is called
typedef struct {
    nhandle_t       hShader;
    uint32_t        numVerts;
    polyVert_t      *verts; // later becomes a drawVert_t
} srfPoly_t;

typedef struct {
    vec3_t          xyz;
    vec3_t          worldPos;
	vec2_t          st;
	vec2_t          lightmap;
	int16_t         normal[4];
	int16_t         tangent[4];
	int16_t         lightdir[4];
	uint16_t        color[4];
#ifdef DEBUG_OPTIMIZEVERTICES
//	unsigned int    id;
#endif
} srfVert_t;

typedef struct {
    nhandle_t hSpriteSheet;
    nhandle_t hSprite;
    vec3_t origin;
} srfQuad_t;

//==================================================================

#ifdef GL_ARB_sync
typedef struct
{
	// the BO is logically split into DYN_BUFFER_SEGMENTS
	// segments, the active segment is the one the CPU may write
	// into, while the GPU may read from the inactive segments.
	void           *baseAddr;
	uint32_t        elementSize;
	uint32_t        segmentElements;
	uint32_t        activeSegment;
	// all syncs except the active segment's should be
	// always defined and waitable, the active segment's
	// sync is always undefined
	GLsync         syncs[ DYN_BUFFER_SEGMENTS ];
} glRingbuffer_t;
#endif

typedef enum
{
    BUF_GL_MAPPED,
    BUF_GL_SYNCED,
    BUF_GL_BUFFER,
    BUF_GL_CLIENT,
} bufferMemType_t;

// not meant to be used by anything other than the vbo backend

typedef enum {
    BUFFER_STATIC,      // data is constant throughout buffer lifetime
    BUFFER_DYNAMIC,     // expected to be updated once in a while, but not every frame
    BUFFER_FRAME,       // expected to be update on a per-frame basis
    BUFFER_STREAM,      // use GL_STREAM_DRAW -- only really used by the imgui backend
} bufferType_t;

typedef struct {
    void *data;
    uint64_t size;
    uintptr_t offset;
    bufferMemType_t usage;
    uint32_t id;
    uint32_t target;
    uint32_t glUsage;
} buffer_t;

typedef struct {
    uint32_t index;
    uint32_t count;
    uint32_t type;
    uint32_t enabled;
    uint32_t normalized;
    uintptr_t stride;
    uintptr_t offset;
} vertexAttrib_t;

typedef struct {
    char name[MAX_GDR_PATH];

    uint32_t vaoId;
    bufferType_t type;

    buffer_t vertex;
    buffer_t index;

    vertexAttrib_t attribs[ATTRIB_INDEX_COUNT];
} vertexBuffer_t;


#include "rgl_fbo.h"



//==================================================================

/*
* Shaders
*/

typedef enum {
    SS_BAD,             // throw error

    SS_OPAQUE,          // opaque stuff
    SS_DECAL,           // scorch marks, blood splats, etc.
    SS_SEE_THROUGH,     // ladders, grates, grills that may have small blended edges

    SS_BLEND,           // the standard
} shaderSort_t;

typedef enum {
	GF_NONE,

	GF_SIN,
	GF_SQUARE,
	GF_TRIANGLE,
	GF_SAWTOOTH, 
	GF_INVERSE_SAWTOOTH, 

	GF_NOISE

} genFunc_t;


typedef enum {
	DEFORM_NONE,
	DEFORM_WAVE,
	DEFORM_NORMALS,
	DEFORM_BULGE,
	DEFORM_MOVE,
	DEFORM_PROJECTION_SHADOW,
	DEFORM_AUTOSPRITE,
	DEFORM_AUTOSPRITE2,
	DEFORM_TEXT0,
	DEFORM_TEXT1,
	DEFORM_TEXT2,
	DEFORM_TEXT3,
	DEFORM_TEXT4,
	DEFORM_TEXT5,
	DEFORM_TEXT6,
	DEFORM_TEXT7
} deform_t;

// deformVertexes types that can be handled by the GPU
typedef enum
{
	// do not edit: same as genFunc_t

	DGEN_NONE,
	DGEN_WAVE_SIN,
	DGEN_WAVE_SQUARE,
	DGEN_WAVE_TRIANGLE,
	DGEN_WAVE_SAWTOOTH,
	DGEN_WAVE_INVERSE_SAWTOOTH,
	DGEN_WAVE_NOISE,

	// do not edit until this line

	DGEN_BULGE,
	DGEN_MOVE
} deformGen_t;

typedef enum {
	AGEN_IDENTITY,
	AGEN_SKIP,
	AGEN_ENTITY,
	AGEN_ONE_MINUS_ENTITY,
	AGEN_VERTEX,
	AGEN_ONE_MINUS_VERTEX,
	AGEN_LIGHTING_SPECULAR,
	AGEN_WAVEFORM,
	AGEN_PORTAL,
	AGEN_CONST,
} alphaGen_t;

typedef enum {
	CGEN_BAD,
	CGEN_IDENTITY_LIGHTING,	// tr.identityLight
	CGEN_IDENTITY,			// always (1,1,1,1)
	CGEN_ENTITY,			// grabbed from entity's modulate field
	CGEN_ONE_MINUS_ENTITY,	// grabbed from 1 - entity.modulate
	CGEN_EXACT_VERTEX,		// tess.vertexColors
	CGEN_VERTEX,			// tess.vertexColors * tr.identityLight
	CGEN_EXACT_VERTEX_LIT,	// like CGEN_EXACT_VERTEX but takes a light direction from the lightgrid
	CGEN_VERTEX_LIT,		// like CGEN_VERTEX but takes a light direction from the lightgrid
	CGEN_ONE_MINUS_VERTEX,
	CGEN_WAVEFORM,			// programmatically generated
	CGEN_LIGHTING_DIFFUSE,
	CGEN_FOG,				// standard fog
	CGEN_CONST				// fixed color
} colorGen_t;

typedef enum {
	TCGEN_BAD,
	TCGEN_IDENTITY,			// clear to 0,0
	TCGEN_LIGHTMAP,
	TCGEN_TEXTURE,
	TCGEN_ENVIRONMENT_MAPPED,
	TCGEN_ENVIRONMENT_MAPPED_FP, // with correct first-person mapping
	TCGEN_FOG,
	TCGEN_VECTOR			// S and T from world coordinates
} texCoordGen_t;

typedef enum {
	ACFF_NONE,
	ACFF_MODULATE_RGB,
	ACFF_MODULATE_RGBA,
	ACFF_MODULATE_ALPHA
} acff_t;

typedef struct {
	float base;
	float amplitude;
	float phase;
	float frequency;

	genFunc_t	func;
} waveForm_t;

#define TR_MAX_TEXMODS 4

typedef enum {
	TMOD_NONE,
	TMOD_TRANSFORM,
	TMOD_TURBULENT,
	TMOD_SCROLL,
	TMOD_SCALE,
	TMOD_STRETCH,
	TMOD_ROTATE,
	TMOD_ENTITY_TRANSLATE,
	TMOD_OFFSET,
	TMOD_SCALE_OFFSET,
} texMod_t;

#define	MAX_SHADER_DEFORMS	3
typedef struct {
	deform_t	deformation;			// vertex coordinate modification type

	vec3_t		moveVector;
	waveForm_t	deformationWave;
	float		deformationSpread;

	float		bulgeWidth;
	float		bulgeHeight;
	float		bulgeSpeed;
} deformStage_t;

typedef struct {
	texMod_t		type;

	union {

		// used for TMOD_TURBULENT and TMOD_STRETCH
		waveForm_t		wave;

		// used for TMOD_TRANSFORM
		struct {
			float		matrix[2][2];	// s' = s * m[0][0] + t * m[1][0] + trans[0]
			float		translate[2];	// t' = s * m[0][1] + t * m[0][1] + trans[1]
		};

		// used for TMOD_SCALE, TMOD_OFFSET, TMOD_SCALE_OFFSET
		struct {
			float		scale[2];		// s' = s * scale[0] + offset[0]
			float		offset[2];		// t' = t * scale[1] + offset[1]
		};

		// used for TMOD_SCROLL
		float			scroll[2];		// s' = s + scroll[0] * time
										// t' = t + scroll[1] * time
		// used for TMOD_ROTATE
		// + = clockwise
		// - = counterclockwise
		float			rotateSpeed;

	};

} texModInfo_t;


#define MAX_IMAGE_ANIMATIONS		24

#define LIGHTMAP_INDEX_NONE			0
#define LIGHTMAP_INDEX_SHADER		1
#define LIGHTMAP_INDEX_OFFSET		2

typedef struct {
	texture_t		*image;
	uint32_t		numImageAnimations;
	double			imageAnimationSpeed;	// -EC- set to double

	texCoordGen_t	tcGen;
	vec3_t			tcGenVectors[2];

	uint32_t		numTexMods;
	texModInfo_t	*texMods;

    qboolean isLightmap;

    textureFilter_t filter;


	int32_t			videoMapHandle;
	qboolean		isVideoMap;
	qboolean		isScreenMap;
} textureBundle_t;

#define TESS_ST0	1<<0
#define TESS_ST1	1<<1
#define TESS_ENV0	1<<2
#define TESS_ENV1	1<<3

enum
{
	TB_COLORMAP    = 0,
	TB_DIFFUSEMAP  = 0,
	TB_LIGHTMAP    = 1,
	TB_LEVELSMAP   = 1,
	TB_SHADOWMAP3  = 1,
	TB_NORMALMAP   = 2,
	TB_DELUXEMAP   = 3,
	TB_SHADOWMAP2  = 3,
	TB_SPECULARMAP = 4,
	TB_SHADOWMAP   = 5,
	TB_CUBEMAP     = 6,
	TB_SHADOWMAP4  = 6,

    NUM_TEXTURE_BUNDLES
};

typedef enum
{
	// material shader stage types
	ST_COLORMAP = 0,			// vanilla Q3A style shader treatening
	ST_DIFFUSEMAP = 0,          // treat color and diffusemap the same
	ST_NORMALMAP,
	ST_NORMALPARALLAXMAP,
	ST_SPECULARMAP,
	ST_GLSL
} stageType_t;

typedef struct {
    qboolean active;

    textureBundle_t	bundle[NUM_TEXTURE_BUNDLES];

    waveForm_t rgbWave;
    colorGen_t rgbGen;

    waveForm_t alphaWave;
    alphaGen_t alphaGen;

    byte constantColor[4];      // for CGEN_CONST and AGEN_CONST

    uint32_t stateBits;         // GLS_xxxx mask

    qboolean isDetail;

    stageType_t type;
    struct shaderProgram_s *glslShaderGroup;
    int32_t glslShaderIndex;

    vec4_t normalScale;
    vec4_t specularScale;
} shaderStage_t;

typedef struct shader_s {
    char name[MAX_NPATH];

    uint32_t		numDeforms;
	deformStage_t	deforms[MAX_SHADER_DEFORMS];

    shaderStage_t *stages[MAX_SHADER_STAGES];

    uint32_t sortedIndex;           // this shader == rg.sortedShaders[sortedIndex]
    uint32_t index;                 // this shader == rg.shaders[index]
    shaderSort_t sort;

    int32_t lightmapIndex;

    qboolean explicitlyDefined;     // found in a .shader file

    uint32_t surfaceFlags;          // if explicitly defined this will have SURFACEPARM_* flags

    uint32_t vertexAttribs;         // not all shaders will need all data to be gathered
    
    qboolean noLightScale;
    qboolean polygonOffset;		    // set for decals and other items that must be offset 
	qboolean noMipMaps;			    // for console fonts, 2D elements, etc.
	qboolean noPicMip;			    // for images that must always be full resolution

    int32_t lightingStage;

    qboolean defaultShader;		    // we want to return index 0 if the shader failed to
								    // load for some reason, but R_FindShader should
								    // still keep a name allocated for it, so if
								    // something calls RE_RegisterShader again with
								    // the same name, we don't try looking for it again

    double	clampTime;                                  // time this shader is clamped to - set to double for frameloss fix -EC-
	double	timeOffset;                                 // current time offset for this shader - set to double for frameloss fix -EC-

    struct shader_s *next;
} shader_t;

//==================================================================

/*
* Sprite Sheets
*/

typedef vec2_t spriteCoord_t[4];

typedef struct {
    spriteCoord_t texCoords;
} sprite_t;

typedef struct {
    char *name;
    sprite_t *sprites;
    nhandle_t hShader;
    uint32_t sheetWidth;
    uint32_t sheetHeight;
    uint32_t spriteWidth;
    uint32_t spriteHeight;
    uint32_t numSprites;
} spriteSheet_t;


//==================================================================

// renderSceneDef_t holds everything that comes in renderSceneRef_t,
// as well as the locally generated scene information
typedef struct {
    uint32_t    x, y, width, height;

    stereoFrame_t stereoFrame;

    qboolean    drawn;

    int64_t     time;
    uint64_t    flags;

    double      floatTime; // rg.refdef.time / 1000.0
    float       blurFactor;

    uint64_t    numEntities;
    renderEntityDef_t *entities;

    uint64_t    numDLights;
    dlight_t    *dlights;

    uint64_t    numPolys;
    srfPoly_t   *polys;

    float       sunShadowMvp[4][16];
	float       sunDir[4];
	float       sunCol[4];
	float       sunAmbCol[4];

    float       autoExposureMinMax[2];
	float       toneMinAvgMaxLinear[3];
} renderSceneDef_t;

typedef struct {
    vec3_t origin;
    vec3_t axis[3];
    vec3_t viewOrigin;
    float angle;
    float zoom;
    float aspect;
    mat4_t modelMatrix;
    mat4_t projectionMatrix;
    mat4_t viewMatrix;
    mat4_t viewProjectionMatrix;
} cameraData_t;

typedef struct {
	vec3_t		origin;			// in world coordinates
	vec3_t		axis[3];		// orientation in world
	vec3_t		viewOrigin;		// viewParms->or.origin in local coordinates
	float		modelMatrix[16];
	float		transformMatrix[16];
} orientationr_t;

typedef struct {
    cameraData_t camera;

    uint32_t viewportX, viewportY;
    uint32_t viewportWidth, viewportHeight;

    float zFar;
    float zNear;
    float fovX;
    float fovY;

    stereoFrame_t stereoFrame;
    cplane_t frustum[5];

    uint32_t flags;

    uint64_t frameSceneNum;
    uint64_t frameCount;
} viewData_t;

typedef struct {
    char baseName[MAX_GDR_PATH];
    char name[MAX_GDR_PATH];

    vec3_t ambientLightColor;

    uint32_t width;
    uint32_t height;

    maplight_t *lights;
    uint32_t numLights;

    maptile_t *tiles;
    uint32_t numTiles;

    uint64_t numIndices;
    uint64_t numVertices;

    spriteCoord_t *sprites;
    uint32_t numSprites;

    // frame based draw data
    shader_t *shader;
    drawVert_t *vertices;
    glIndex_t *indices;
    vertexBuffer_t *buffer;
    nhandle_t tileset;
} world_t;

typedef struct {
    uint64_t msec;

    uint32_t c_glslShaderBinds;
    uint32_t c_bufferIndices, c_bufferVertices;
    uint32_t c_iboBinds, c_vboBinds, c_vaoBinds;
    uint32_t c_dynamicBufferDraws;
    uint32_t c_staticBufferDraws;
    uint32_t c_bufferBinds;
    uint32_t c_surfaces;
    uint32_t c_overDraw;
    uint32_t c_lightallDraws;
    uint32_t c_genericDraws;
} backendCounters_t;


typedef struct {
    uintptr_t vtxOffset;        // current offset in vertex buffer
    uintptr_t idxOffset;        // current offset in index buffer
    
    uintptr_t vtxDataSize;      // size in bytes of each vertex
    uintptr_t idxDataSize;      // size in bytes of each index

    uint32_t maxVertices;       // total allocated vertices with glBufferData
    uint32_t maxIndices;        // total allocated indices with glBufferData

    void *vertices;             // address of the client vertices
    void *indices;              // address of the client indices

    vertexBuffer_t *buffer;     // the buffer handle we're using for this batch

    glRingbuffer_t vertexRB;
    glRingbuffer_t indexRB;

    shader_t *shader;

    double shaderTime;
} batch_t;

typedef struct {
//    byte color2D[4];
    float color2D[4];

    qboolean depthFill;
    qboolean framePostProcessed;
    qboolean colorMask[4];

    backendCounters_t pc;

    vertexBuffer_t *drawBuffer;

    renderSceneDef_t refdef;
    renderEntityDef_t *currentEntity;

    batch_t drawBatch;
} renderBackend_t;

// the renderer front end should never modify glstate_t
typedef struct {
	GLuint		currenttextures[ MAX_TEXTURE_UNITS ];
	int			currenttmu;
	qboolean	finishCalled;
	GLint		texEnv[2];
	unsigned	glStateBits;
    unsigned    storedGlState;
    int         currentArray;
	unsigned	glClientStateBits[ MAX_TEXTURE_UNITS ];

    GLuint      textureStack[MAX_TEXTURE_UNITS];
    GLuint      *textureStackPtr;

    viewData_t viewData;

    uint32_t vertexAttribsEnabled;

    uint32_t    vaoId;
    uint32_t    vboId;
    uint32_t    iboId;
    uint32_t    defFboId;
    uint32_t    readFboId;
    uint32_t    writeFboId;
    uint32_t    rboId;
    uint32_t    shaderId;
    uint32_t    uboId;

    fbo_t *currentFbo;
    vertexBuffer_t *currentVao;
    texture_t *currentTexture;
    shaderProgram_t *currentShader;
    uniformBuffer_t *currentUbo;
} glstate_t;

typedef enum {
    TIME_QUERY,
    SAMPLES_QUERY,
    PRIMTIVES_QUERY,

    NUMQUERIES
} query_t;

enum
{
    GENERICDEF_USE_DEFORM_VERTEXES  = 0x0001,
	GENERICDEF_USE_TCGEN_AND_TCMOD  = 0x0002,
	GENERICDEF_USE_RGBAGEN          = 0x0004,
	GENERICDEF_ALL                  = 0x000F,
	GENERICDEF_COUNT                = 0x0010,
};

enum
{
	LIGHTDEF_USE_LIGHTMAP        = 0x0001,
	LIGHTDEF_USE_LIGHT_VECTOR    = 0x0002,
	LIGHTDEF_USE_LIGHT_VERTEX    = 0x0003,
	LIGHTDEF_LIGHTTYPE_MASK      = 0x0003,
	LIGHTDEF_USE_TCGEN_AND_TCMOD = 0x0004,
    LIGHTDEF_USE_PARALLAXMAP     = 0x0008,
	LIGHTDEF_USE_SHADOWMAP       = 0x0010,
	LIGHTDEF_ALL                 = 0x002F,
	LIGHTDEF_COUNT               = 0x0030
};


typedef struct
{
    qboolean				registered;		// cleared at shutdown, set at beginRegistration

	uint32_t				frameCount;		// incremented every frame
	uint32_t				sceneCount;		// incremented every scene
	uint32_t				viewCount;		// incremented every view (twice a scene if portaled)
											// and every R_MarkFragments call

	uint32_t	   			frameSceneNum;	// zeroed at RE_BeginFrame

    qboolean                needScreenMap;

	qboolean				worldMapLoaded;
	qboolean				worldDeluxeMapping;
	vec3_t                  toneMinAvgMaxLevel;
    renderEntityDef_t       worldEntity;
	world_t					*world;

	texture_t				*defaultImage;
	texture_t				*scratchImage[ MAX_VIDEO_HANDLES ];
//	texture_t				*fogImage;
	texture_t				*dlightImage;	// inverse-quare highlight for projective adding
	texture_t				*flareImage;
	texture_t				*whiteImage;			// full of 0xff
	texture_t				*identityLightImage;	// full of tr.identityLightByte	

    texture_t               *bloomImage;
	texture_t				*renderImage;
	texture_t				*sunRaysImage;
	texture_t				*renderDepthImage;
	texture_t				*pshadowMaps[MAX_DRAWN_PSHADOWS];
	texture_t				*screenScratchImage;
	texture_t				*textureScratchImage[2];
	texture_t               *quarterImage[2];
	texture_t				*calcLevelsImage;
	texture_t				*targetLevelsImage;
	texture_t				*fixedLevelsImage;
	texture_t				*sunShadowDepthImage[4];
	texture_t               *screenShadowImage;
	texture_t               *screenSsaoImage;
	texture_t				*hdrDepthImage;
	
	texture_t				*textureDepthImage;

	fbo_t					*renderFbo;
	fbo_t					*msaaResolveFbo;
	fbo_t					*sunRaysFbo;
	fbo_t					*depthFbo;
	fbo_t					*pshadowFbos[MAX_DRAWN_PSHADOWS];
	fbo_t					*screenScratchFbo;
	fbo_t					*textureScratchFbo[2];
	fbo_t                   *quarterFbo[2];
	fbo_t					*calcLevelsFbo;
	fbo_t					*targetLevelsFbo;
	fbo_t					*sunShadowFbo[4];
	fbo_t					*screenShadowFbo;
	fbo_t					*screenSsaoFbo;
	fbo_t					*hdrDepthFbo;
//	fbo_t                   *renderCubeFbo;

	shader_t				*defaultShader;
	shader_t				*shadowShader;
	shader_t				*projectionShadowShader;

	shader_t				*flareShader;
	shader_t				*sunShader;
	shader_t				*sunFlareShader;

	uint32_t				numLightmaps;
	uint32_t				lightmapSize;
	texture_t				**lightmaps;
	texture_t				**deluxemaps;

    uniformBuffer_t         *lightData;

    uniformBuffer_t *uniformBuffers[MAX_UNIFORM_BUFFERS];
    uint64_t numUniformBuffers;

    spriteSheet_t *sheets[MAX_RENDER_SPRITESHEETS];
    uint32_t numSpriteSheets;

    vertexBuffer_t *buffers[MAX_RENDER_BUFFERS];
    uint32_t numBuffers;

    shader_t *shaders[MAX_RENDER_SHADERS];
    shader_t *sortedShaders[MAX_RENDER_SHADERS];
    uint32_t numShaders;

    shaderProgram_t *programs[MAX_RENDER_PROGRAMS];
    uint32_t numPrograms;

    texture_t *textures[MAX_RENDER_TEXTURES];
    uint32_t numTextures;

    fbo_t *fbos[MAX_RENDER_FBOs];
    uint32_t numFBOs;

    float identityLight;
    uint32_t identityLightByte;
    uint32_t overbrightBits;

    uint64_t frontEndMsec;

    shaderProgram_t genericShader[GENERICDEF_COUNT];
    shaderProgram_t lightallShader[LIGHTDEF_COUNT];
    shaderProgram_t imguiShader;
    shaderProgram_t tileShader;
    shaderProgram_t ssaoShader;
    shaderProgram_t depthBlurShader[4];
    shaderProgram_t calclevels4xShader[2];
    shaderProgram_t down4xShader;
	shaderProgram_t bokehShader;
	shaderProgram_t tonemapShader;
    shaderProgram_t textureColorShader;

    qboolean beganQuery;

    uint32_t samplers[MAX_TEXTURE_UNITS];

    uint32_t queries[NUMQUERIES];
    uint32_t queryCounts[NUMQUERIES];

    float sinTable[FUNCTABLE_SIZE];
	float squareTable[FUNCTABLE_SIZE];
	float triangleTable[FUNCTABLE_SIZE];
	float sawToothTable[FUNCTABLE_SIZE];
	float inverseSawToothTable[FUNCTABLE_SIZE];
    qboolean vertexLightingAllowed;
} renderGlobals_t;

extern renderGlobals_t rg;
extern glContext_t glContext;
extern glstate_t glState;
extern gpuConfig_t glConfig;
extern renderBackend_t backend;

#define OffsetByteToFloat(a)    ((float)(a) * 1.0f/127.5f - 1.0f)
#define FloatToOffsetByte(a)    (byte)((a) * 127.5f + 128.0f)
#define ByteToFloat(a)          ((float)(a) * 1.0f/255.0f)
#define FloatToByte(a)          (byte)((a) * 255.0f)

#define GLS_SRCBLEND_ZERO						0x00000001
#define GLS_SRCBLEND_ONE						0x00000002
#define GLS_SRCBLEND_DST_COLOR					0x00000003
#define GLS_SRCBLEND_ONE_MINUS_DST_COLOR		0x00000004
#define GLS_SRCBLEND_SRC_ALPHA					0x00000005
#define GLS_SRCBLEND_ONE_MINUS_SRC_ALPHA		0x00000006
#define GLS_SRCBLEND_DST_ALPHA					0x00000007
#define GLS_SRCBLEND_ONE_MINUS_DST_ALPHA		0x00000008
#define GLS_SRCBLEND_ALPHA_SATURATE				0x00000009
#define GLS_SRCBLEND_BITS						0x0000000f

#define GLS_DSTBLEND_ZERO						0x00000010
#define GLS_DSTBLEND_ONE						0x00000020
#define GLS_DSTBLEND_SRC_COLOR					0x00000030
#define GLS_DSTBLEND_ONE_MINUS_SRC_COLOR		0x00000040
#define GLS_DSTBLEND_SRC_ALPHA					0x00000050
#define GLS_DSTBLEND_ONE_MINUS_SRC_ALPHA		0x00000060
#define GLS_DSTBLEND_DST_ALPHA					0x00000070
#define GLS_DSTBLEND_ONE_MINUS_DST_ALPHA		0x00000080
#define GLS_DSTBLEND_BITS						0x000000f0

#define GLS_BLEND_BITS							(GLS_SRCBLEND_BITS | GLS_DSTBLEND_BITS)

#define GLS_DEPTHMASK_TRUE						0x00000100

#define GLS_POLYMODE_LINE						0x00001000

#define GLS_DEPTHTEST_DISABLE					0x00010000
#define GLS_DEPTHFUNC_EQUAL						0x00020000
#define GLS_DEPTHFUNC_GREATER                   0x00040000
#define GLS_DEPTHFUNC_BITS                      0x00060000

#define GLS_ATEST_GT_0							0x10000000
#define GLS_ATEST_LT_80							0x20000000
#define GLS_ATEST_GE_80							0x40000000
#define GLS_ATEST_BITS							0x70000000

#define GLS_DEFAULT                             (GLS_DEPTHMASK_TRUE | GLS_SRCBLEND_SRC_ALPHA | GLS_DSTBLEND_ONE_MINUS_SRC_ALPHA)

// vertex array states

#define CLS_NONE								0x00000000
#define CLS_COLOR_ARRAY							0x00000001
#define CLS_TEXCOORD_ARRAY						0x00000002
#define CLS_NORMAL_ARRAY						0x00000004

#define DRAW_IMMEDIATE 0
#define DRAW_CLIENT_BUFFERED 1
#define DRAW_GPU_BUFFERED 2

#ifndef SGN
#define SGN(x) (((x) >= 0) ? !!(x) : -1)
#endif

#ifndef MAX
#define MAX(a,b) ((a) > (b) ? (a) : (b))
#endif

#ifndef MIN
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#endif

#ifndef CLAMP
#define CLAMP(a,b,c) MIN(MAX((a),(b)),(c))
#endif


extern cvar_t *r_measureOverdraw;		// enables stencil buffer overdraw measurement

extern cvar_t *r_fastsky;				// controls whether sky should be cleared or drawn
extern cvar_t *r_drawSun;				// controls drawing of sun quad
extern cvar_t *r_dynamiclight;		    // dynamic lights enabled/disabled
extern cvar_t *r_dlightBacks;			// dlight non-facing surfaces for continuity

extern cvar_t *r_norefresh;			    // bypasses the ref rendering
extern cvar_t *r_drawentities;		    // disable/enable entity rendering
extern cvar_t *r_drawworld;			    // disable/enable world rendering
extern cvar_t *r_speeds;				// various levels of information display
extern cvar_t *r_detailTextures;		// enables/disables detail texturing stages

extern cvar_t *r_cameraExposure;

extern cvar_t *r_gammaAmount;

extern cvar_t *r_singleShader;			// make most world faces use default shader
extern cvar_t *r_roundImagesDown;
extern cvar_t *r_picmip;				// controls picmip values
extern cvar_t *r_finish;
extern cvar_t *r_textureMode;

extern cvar_t *r_fullbright;			// avoid lightmap pass
extern cvar_t *r_lightmap;			    // render lightmaps only
extern cvar_t *r_vertexLight;			// vertex lighting mode for better performance

extern cvar_t *r_showSky;			    // forces sky in front of all surfaces
extern cvar_t *r_clear;			        // force screen clear every frame

extern cvar_t *r_shadows;				// controls shadows: 0 = none, 1 = blur, 2 = stencil, 3 = black planar projection
extern cvar_t *r_flares;				// light flares

extern cvar_t *r_intensity;

extern cvar_t *r_skipBackEnd;

extern cvar_t *r_externalGLSL;

extern cvar_t *r_imageSharpenAmount;

extern cvar_t *r_hdr;
extern cvar_t *r_floatLightmap;
extern cvar_t *r_postProcess;
extern cvar_t *r_lightmap;

extern cvar_t *r_toneMapType;
extern cvar_t *r_toneMap;
extern cvar_t *r_forceToneMap;
extern cvar_t *r_forceToneMapMin;
extern cvar_t *r_forceToneMapAvg;
extern cvar_t *r_forceToneMapMax;

extern cvar_t  *r_autoExposure;
extern cvar_t  *r_forceAutoExposure;
extern cvar_t  *r_forceAutoExposureMin;
extern cvar_t  *r_forceAutoExposureMax;

extern cvar_t *r_depthPrepass;
extern cvar_t *r_ssao;
extern cvar_t *r_bloom;

extern cvar_t *r_normalMapping;
extern cvar_t *r_specularMapping;
extern cvar_t *r_deluxeMapping;
extern cvar_t *r_parallaxMapping;
extern cvar_t *r_parallaxMapOffset;
extern cvar_t *r_parallaxMapShadows;
extern cvar_t *r_cubeMapping;
extern cvar_t *r_cubemapSize;
extern cvar_t *r_deluxeSpecular;
extern cvar_t *r_pbr;
extern cvar_t *r_baseNormalX;
extern cvar_t *r_baseNormalY;
extern cvar_t *r_baseParallax;
extern cvar_t *r_baseSpecular;
extern cvar_t *r_baseGloss;
extern cvar_t *r_glossType;
extern cvar_t *r_dlightMode;
extern cvar_t *r_pshadowDist;
extern cvar_t *r_mergeLightmaps;
extern cvar_t *r_imageUpsample;
extern cvar_t *r_imageUpsampleMaxSize;
extern cvar_t *r_imageUpsampleType;
extern cvar_t *r_genNormalMaps;
extern cvar_t *r_forceSun;
extern cvar_t *r_forceSunLightScale;
extern cvar_t *r_forceSunAmbientScale;
extern cvar_t *r_sunlightMode;
extern cvar_t *r_drawSunRays;
extern cvar_t *r_sunShadows;
extern cvar_t *r_shadowFilter;
extern cvar_t *r_shadowBlur;
extern cvar_t *r_shadowMapSize;
extern cvar_t *r_shadowCascadeZNear;
extern cvar_t *r_shadowCascadeZFar;
extern cvar_t *r_shadowCascadeZBias;
extern cvar_t *r_ignoreDstAlpha;

extern cvar_t *r_greyscale;

extern cvar_t *r_ignoreGLErrors;

extern cvar_t *r_overBrightBits;
extern cvar_t *r_mapOverBrightBits;

extern cvar_t *r_showImages;

extern cvar_t *r_printShaders;

extern cvar_t *r_useExtensions;
extern cvar_t *r_allowLegacy;
extern cvar_t *r_allowShaders;
extern cvar_t *r_multisampleAmount;
extern cvar_t *r_multisampleType;
extern cvar_t *r_ignorehwgamma;
extern cvar_t *r_drawMode;
extern cvar_t *r_glDebug;
extern cvar_t *r_textureBits;
extern cvar_t *r_stencilBits;
extern cvar_t *r_textureDetail;
extern cvar_t *r_drawBuffer;
extern cvar_t *r_customWidth;
extern cvar_t *r_customHeight;
extern cvar_t *r_mappedBuffers;
extern cvar_t *r_glDiagnostics;
extern cvar_t *r_colorMipLevels;

extern cvar_t *r_maxPolys;
extern cvar_t *r_maxEntities;
extern cvar_t *r_maxDLights;

extern cvar_t *r_imageUpsampleType;
extern cvar_t *r_imageUpsample;
extern cvar_t *r_imageUpsampleMaxSize;

// OpenGL extensions
extern cvar_t *r_arb_texture_compression;
extern cvar_t *r_arb_framebuffer_object;
extern cvar_t *r_arb_vertex_array_object;
extern cvar_t *r_arb_vertex_buffer_object;
extern cvar_t *r_arb_texture_filter_anisotropic;
extern cvar_t *r_arb_texture_max_anisotropy;
extern cvar_t *r_arb_texture_float;
extern cvar_t *r_arb_sync;
extern cvar_t *r_arb_shader_storage_buffer_object;
extern cvar_t *r_arb_map_buffer_range;

//====================================================================

static GDR_INLINE qboolean ShaderRequiresCPUDeforms(const shader_t * shader)
{
	if(shader->numDeforms) {
		const deformStage_t *ds = &shader->deforms[0];

		if (shader->numDeforms > 1)
			return qtrue;

		switch (ds->deformation)
		{
			case DEFORM_WAVE:
			case DEFORM_BULGE:
				// need CPU deforms at high level-times to avoid floating point precision loss
				return ( backend.refdef.floatTime != (float)backend.refdef.floatTime );

			default:
				return qtrue;
		}
	}

	return qfalse;
}

//====================================================================

static GDR_INLINE int VectorCompare4(const vec4_t v1, const vec4_t v2)
{
	if(v1[0] != v2[0] || v1[1] != v2[1] || v1[2] != v2[2] || v1[3] != v2[3])
	{
		return 0;
	}
	return 1;
}

//
// rgl_backend.c
//
void GDR_ATTRIBUTE((format(printf, 1, 2))) GL_LogComment(const char *fmt, ...);
void GDR_ATTRIBUTE((format(printf, 1, 2))) GL_LogError(const char *fmt, ...);
void GL_SetObjectDebugName(GLenum target, GLuint id, const char *name, const char *add);
void RB_ShowImages(void);
void GL_SetModelViewMatrix(const mat4_t m);
void GL_SetProjectionMatrix(const mat4_t m);
void GL_CheckErrors(void);
void GL_BindNullTextures(void);
void GL_PushTexture(texture_t *texture);
void GL_PopTexture(void);
void GL_BindTexture(int tmu, texture_t *texture);
void GL_BindNullRenderbuffer(void);
int GL_UseProgram(GLuint program);
void GL_BindNullProgram(void);
void GL_BindFramebuffer(GLenum target, GLuint fbo);
void GL_BindNullFramebuffer(GLenum target);
void GL_BindNullFramebuffers(void);
void GL_BindRenderbuffer(GLuint rbo);
void GL_BindNullRenderbuffer(void);
void GL_State(unsigned stateBits);
void GL_ClientState( int unit, unsigned stateBits );
void RE_BeginFrame(stereoFrame_t stereoFrame);
void RE_EndFrame(uint64_t *frontEndMsec, uint64_t *backEndMsec);
void RB_ExecuteRenderCommands(const void *data);

void R_AddPostProcessCmd( void );

//
// rgl_extensions.c
//
void R_InitExtensions(void);

//
// rgl_program.c
//
void GLSL_UseProgram(shaderProgram_t *program);
void GLSL_InitGPUShaders(void);
void GLSL_ShutdownGPUShaders(void);
void GLSL_SetUniformInt(shaderProgram_t *program, uint32_t uniformNum, GLint value);
void GLSL_SetUniformFloat(shaderProgram_t *program, uint32_t uniformNum, GLfloat value);
void GLSL_SetUniformVec2(shaderProgram_t *program, uint32_t uniformNum, const vec2_t v);
void GLSL_SetUniformVec3(shaderProgram_t *program, uint32_t uniformNum, const vec3_t v);
void GLSL_SetUniformVec4(shaderProgram_t *program, uint32_t uniformNum, const vec4_t v);
void GLSL_SetUniformMatrix4(shaderProgram_t *program, uint32_t uniformNum, const mat4_t m);
void GLSL_ShaderBufferData( shaderProgram_t *shader, uint32_t uniformNum, uniformBuffer_t *buffer );
uniformBuffer_t *GLSL_InitUniformBuffer( const char *name, byte *buffer, uint64_t bufSize );
shaderProgram_t *GLSL_GetGenericShaderProgram(int stage);

//
// rgl_math.c
//
qboolean Mat4Compare( const mat4_t a, const mat4_t b );
void Mat4Ortho( float left, float right, float bottom, float top, float znear, float zfar, mat4_t out );
void Mat4Dump( const mat4_t in );
void VectorLerp( vec3_t a, vec3_t b, float lerp, vec3_t c );
qboolean SpheresIntersect(vec3_t origin1, float radius1, vec3_t origin2, float radius2);
void BoundingSphereOfSpheres(vec3_t origin1, float radius1, vec3_t origin2, float radius2, vec3_t origin3, float *radius3);
int32_t NextPowerOfTwo(int32_t in);
uint16_t FloatToHalf( float in );
float HalfToFloat( uint16_t in );

//
// rgl_texture.c
//
void R_ImageList_f(void);
void R_DeleteTextures(void);
texture_t *R_FindImageFile( const char *name, imgType_t type, imgFlags_t flags );
void R_GammaCorrect( byte *buffer, uint64_t bufSize );
void R_UpdateTextures( void );
void R_InitTextures(void);
texture_t *R_CreateImage(  const char *name, byte *pic, int width, int height, imgType_t type, imgFlags_t flags, int internalFormat );

extern int gl_filter_min, gl_filter_max;

//
// rgl_shade_calc.c
//
void RB_DeformTessGeometry( void );
//void RB_CalcFogTexCoords( float *dstTexCoords );
void RB_CalcScaleTexMatrix( const float scale[2], float *matrix );
void RB_CalcScrollTexMatrix( const float scrollSpeed[2], float *matrix );
void RB_CalcRotateTexMatrix( float degsPerSecond, float *matrix );
void RB_CalcTurbulentFactors( const waveForm_t *wf, float *amplitude, float *now );
void RB_CalcTransformTexMatrix( const texModInfo_t *tmi, float *matrix  );
void RB_CalcStretchTexMatrix( const waveForm_t *wf, float *matrix );
void RB_CalcModulateColorsByFog( unsigned char *dstColors );
float RB_CalcWaveAlphaSingle( const waveForm_t *wf );
float RB_CalcWaveColorSingle( const waveForm_t *wf );

//
// rgl_main.c
//
void RB_MakeViewMatrix( void );
void R_RenderView( const viewData_t *parms );
void GL_CameraResize(void);
qboolean R_HasExtension(const char *ext);
void RE_BeginRegistration(gpuConfig_t *config);
nhandle_t RE_RegisterSpriteSheet( const char *npath, uint32_t sheetWidth, uint32_t sheetHeight, uint32_t spriteWidth, uint32_t spriteHeight );
nhandle_t RE_RegisterSprite( nhandle_t hSpriteSheet, uint32_t index );
void R_WorldToGL2( polyVert_t *verts, vec3_t pos );
qboolean R_CalcTangentVectors(drawVert_t dv[3]);

//
// rgl_scene.c
//
void RB_InstantQuad(vec4_t quadVerts[4]);
void RE_AddSpriteToScene( const vec3_t origin, nhandle_t hSpriteSheet, nhandle_t hSprite );
void RE_AddPolyToScene( nhandle_t hShader, const polyVert_t *verts, uint32_t numVerts );
void RE_AddPolyListToScene( const poly_t *polys, uint32_t numPolys );
void RE_RenderScene( const renderSceneRef_t *fd );
void RE_AddEntityToScene( const renderEntityRef_t *ent );
void RE_BeginScene( const renderSceneRef_t *fd );
void RE_EndScene( void );
void RE_ClearScene( void );
void R_InitNextFrame( void );

//
// rgl_shader.c
//
void R_ShaderList_f( void );
nhandle_t RE_RegisterShaderFromTexture(const char *name, int32_t lightmapIndex, texture_t *image);
nhandle_t RE_RegisterShader(const char *name);
shader_t *R_GetShaderByHandle(nhandle_t hShader);
shader_t *R_FindShaderByName( const char *name );
shader_t *R_FindShader(const char *name);
void R_InitShaders( void );

//
// rgl_draw.c
//
void R_DrawElements( uint32_t numElements, uintptr_t nOffset );
void R_DrawPolys( void );
void RB_IterateShaderStages( shader_t *shader );
void RB_InstantQuad(vec4_t quadVerts[4]);
void RB_InstantQuad2(vec4_t quadVerts[4], vec2_t texCoords[4]);
void RB_DrawShaderStages( nhandle_t hShader, uint32_t nElems, uint32_t type, const void *offset, int32_t baseVertex );

//
// rgl_cache.c
//
void R_VaoPackTangent( int16_t *out, vec4_t v );
void R_VaoPackNormal( int16_t *out, vec3_t v );
void R_VaoPackColor( uint16_t *out, const vec4_t c );
void R_VaoUnpackNormal( vec3_t v, int16_t *pack );
void R_VaoUnpackTangent( vec4_t v, int16_t *pack );
vertexBuffer_t *R_AllocateBuffer( const char *name, void *vertices, uint32_t verticesSize, void *indices, uint32_t indicesSize,
    bufferType_t type );
void VBO_BindNull( void );
void R_InitGPUBuffers( void );
void R_ShutdownGPUBuffers( void );
void VBO_Bind( vertexBuffer_t *vbo );
void VBO_SetVertexPointers(vertexBuffer_t *vbo, uint32_t attribBits);
void R_ShutdownBuffer( vertexBuffer_t *vbo );

// for batch drawing
void RB_SetBatchBuffer( vertexBuffer_t *buffer, void *vertexBuffer, uintptr_t vtxSize, void *indexBuffer, uintptr_t idxSize );
void RB_FlushBatchBuffer( void );
void RB_CommitDrawData( const void *verts, uint32_t numVerts, const void *indices, uint32_t numIndices );


void RE_BeginFrame(stereoFrame_t stereoFrame);
void RE_EndFrame(uint64_t *frontEndMsec, uint64_t *backEndMsec);



void R_LoadBMP( const char *name, byte **pic, int *width, int *height, int *channels );
void R_LoadJPG( const char *name, byte **pic, int *width, int *height, int *channels );
void R_LoadPCX( const char *name, byte **pic, int *width, int *height, int *channels );
void R_LoadPNG( const char *name, byte **pic, int *width, int *height, int *channels );
void R_LoadTGA( const char *name, byte **pic, int *width, int *height, int *channels );

/*
=============================================================

RENDERER BACK END COMMAND QUEUE

=============================================================
*/

#define MAX_RC_BUFFER 0x80000

typedef struct
{
    byte buffer[MAX_RC_BUFFER];
    uint32_t usedBytes;
} renderCommandList_t;

typedef enum
{
    RC_POSTPROCESS,
    RC_SWAP_BUFFERS,
    RC_DRAW_BUFFER,
    RC_COLORMASK,
    RC_SCREENSHOT,

    // mainly called from the vm
    RC_DRAW_IMAGE,
    RC_SET_COLOR,

    RC_END_OF_LIST
} renderCmdType_t;

typedef struct {
    renderCmdType_t commandId;
    float color[4];
} setColorCmd_t;

typedef struct {
    renderCmdType_t commandId;
    uint32_t rgba[4];
} colorMaskCmd_t;

typedef struct {
    renderCmdType_t commandId;
} swapBuffersCmd_t;

typedef struct {
    renderCmdType_t commandId;
    uint32_t buffer;
} drawBufferCmd_t;

typedef struct {
    renderCmdType_t commandId;
    shader_t *shader;
    float x, y;
    float w, h;
    float u1, v1;
    float u2, v2;
} drawImageCmd_t;

typedef struct {
	renderCmdType_t commandId;
	int x;
	int y;
	int width;
	int height;
	const char *fileName;
	qboolean jpeg;
} screenshotCommand_t;


typedef struct {
    renderCmdType_t commandId;
    viewData_t viewData;
    renderSceneDef_t refdef;
} postProcessCmd_t;

typedef struct {
    renderCommandList_t commandList;

    dlight_t *dlights;
    renderEntityDef_t *entities;

    glIndex_t *indices;
    srfVert_t *verts;
    polyVert_t *polyVerts;
    srfPoly_t *polys;

    uint64_t numPolys;
    uint64_t numIndices;

    screenshotCommand_t screenshotBuf;
} renderBackendData_t;

extern qboolean screenshotFrame;
extern renderBackendData_t *backendData;

void RE_DrawImage( float x, float y, float w, float h, float u1, float v1, float u2, float v2, nhandle_t hShader );
void RE_LoadWorldMap(const char *filename);
void RE_SetColor(const float *rgba);
void R_IssuePendingRenderCommands(void);

// https://zerowing.idsoftware.com/bugzilla/show_bug.cgi?id=516
void RB_TakeScreenshotCmd( void );
void *R_GetCommandBuffer( uint32_t bytes );

float R_NoiseGet4f( float x, float y, float z, double t );
void R_NoiseInit( void );

#endif