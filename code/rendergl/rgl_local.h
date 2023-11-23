#ifndef __RGL_LOCAL__
#define __RGL_LOCAL__

#pragma once

#include "../engine/n_shared.h"
#include "../rendercommon/r_public.h"
#include "../rendercommon/r_types.h"
#include "../engine/gln_files.h"
#include "ngl.h"

#define MAX_RENDER_BUFFERS 2048
#define MAX_RENDER_PROGRAMS 2048
#define MAX_RENDER_TEXTURES 2048
#define MAX_RENDER_SHADERS 2048
#define MAX_RENDER_FBOs 64

typedef struct ImDrawData ImDrawData;
#define GLN_INDEX_TYPE GL_UNSIGNED_INT
typedef uint32_t glIndex_t;

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

#define PSHADOW_MAP_SIZE 512

#define NGL_VERSION_ATLEAST(major,minor) (glContext.versionMajor > major || (glContext.versionMajor == major && glContext.versionMinor >= minor))

#define MAX_DRAW_VERTICES (MAX_INT/sizeof(drawVert_t))
#define MAX_DRAW_INDICES (MAX_INT/sizeof(glIndex_t))

// per drawcall batch
#define MAX_BATCH_QUADS 4096
#define MAX_BATCH_VERTICES (MAX_BATCH_QUADS*4)
#define MAX_BATCH_INDICES (MAX_BATCH_QUADS*6)

// any change in the LIGHTMAP_* defines here MUST be reflected in
// R_FindShader() in tr_bsp.c
#define LIGHTMAP_2D         -4	// shader is for 2D rendering
#define LIGHTMAP_BY_VERTEX  -3	// pre-lit triangle models
#define LIGHTMAP_WHITEIMAGE -2
#define LIGHTMAP_NONE       -1

enum
{
    TB_COLORMAP = 0,
    TB_DIFFUSEMAP,
    TB_NORMALMAP,
    TB_SPECULARMAP,
    TB_LIGHTMAP,

    NUM_TEXTURE_BINDINGS
};

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
} glContext_t;

typedef struct {
    renderEntityRef_t e;
    vec3_t ambientLight;
    vec3_t lightDir;
    vec3_t directedLight;
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

    uint32_t width, height;     // source image
    uint32_t uploadWidth;       // adter power of two but not including clamp to MAX_TEXTURE_SIZE
    uint32_t uploadHeight;
    uint32_t id;                // GL texture binding

    uint64_t frameUsed;

    uint32_t internalFormat;
    imgType_t type;
    imgFlags_t flags;
} texture_t;

enum {
	ATTRIB_INDEX_POSITION       = 0,
	ATTRIB_INDEX_TEXCOORD       = 1,
    ATTRIB_INDEX_COLOR          = 2,
    ATTRIB_INDEX_NORMAL         = 3,
	
	ATTRIB_INDEX_COUNT
};

enum
{
    ATTRIB_POSITION             = BIT(ATTRIB_INDEX_POSITION),
    ATTRIB_NORMAL               = BIT(ATTRIB_INDEX_NORMAL),
	ATTRIB_TEXCOORD             = BIT(ATTRIB_INDEX_TEXCOORD),
    ATTRIB_COLOR                = BIT(ATTRIB_INDEX_COLOR),

	ATTRIB_BITS =
        ATTRIB_POSITION |
        ATTRIB_TEXCOORD | 
        ATTRIB_COLOR |
        ATTRIB_NORMAL
};

typedef enum {
    GLSL_INT = 0,
    GLSL_FLOAT,
    GLSL_VEC2,
    GLSL_VEC3,
    GLSL_VEC4,
    GLSL_MAT16,
    GLSL_BUFFER, // uniform buffer -- special case
} glslType_t;

typedef enum {
    UNIFORM_DIFFUSE_MAP = 0,
    UNIFORM_LIGHT_MAP,
    UNIFORM_NORMAL_MAP,
    UNIFORM_SPECULAR_MAP,

    UNIFORM_NUM_LIGHTS,
    UNIFORM_LIGHT_INFO,
    UNIFORM_AMBIENT_LIGHT,

    UNIFORM_MODELVIEWPROJECTION,
    UNIFORM_MODELMATRIX,

    UNIFORM_SPECULAR_SCALE,
    UNIFORM_NORMAL_SCALE,

    UNIFORM_COLOR_GEN,
    UNIFORM_ALPHA_GEN,
    UNIFORM_COLOR,
    UNIFORM_BASE_COLOR,
    UNIFORM_VERT_COLOR,

    // random stuff
    UNIFORM_ALPHA_TEST,
    UNIFORM_TCGEN,
	UNIFORM_VIEWINFO, // znear, zfar, width/2, height/2

    UNIFORM_COUNT
} uniform_t;

typedef struct
{
    char name[MAX_GDR_PATH];

    char *compressedVSCode;
    char *compressedFSCode;

    uint32_t vertexBufLen;
    uint32_t fragmentBufLen;

    uint32_t programId;
    uint32_t vertexId;
    uint32_t fragmentId;
    uint32_t attribBits; // vertex array attribute flags

    // uniforms
    GLint uniforms[UNIFORM_COUNT];
    int16_t uniformBufferOffsets[UNIFORM_COUNT]; // max 32767/64=511 uniforms
    char *uniformBuffer;
} shaderProgram_t;

typedef struct
{
    shaderProgram_t *shader;
    uint32_t id;
    uint32_t binding;
    uint64_t size;
} uniformBuffer_t;

typedef enum
{
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

// normal and light are unused
typedef struct
{
    vec3_t xyz;
    vec2_t uv;
    vec2_t lightmap;
    int16_t normal[4];
    int16_t tangent[4];
    int16_t lightdir[4];
    uint16_t color[4];
} drawVert_t;

/*
==============================================================================

SURFACES

==============================================================================
*/

#define MAX_DRAWSURFS 0x10000
#define DRAWSURF_MASK (0x20000-1)

// any changes in surfaceType must be mirrored in rb_surfaceTable[]
typedef enum {
    SF_BAD,
    SF_SKIP,    // ignore
    SF_POLY,
    SF_TILE,

    SF_NUM_SURFACE_TYPES,
    SF_MAX = 0x7fffffff			// ensures that sizeof( surfaceType_t ) == sizeof( int )
} surfaceType_t;

typedef struct {
    uint32_t sort;
    surfaceType_t *surface;
} drawSurf_t;

// when sgame directly specifies a polygon, it becomes a srfPoly_t
// as soon as it is called
typedef struct {
    surfaceType_t   surfaceType;
    nhandle_t       hShader;
    uint32_t        numVerts;
    polyVert_t      *verts; // later becomes a drawVert_t
} srfPoly_t;

typedef struct {
    surfaceType_t surfaceType;
    uint32_t numTiles;

    drawVert_t *vertices;
    glIndex_t *indices;
} srfTile_t;

//==================================================================

typedef enum
{
    BUF_GL_MAPPED,
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

typedef struct {
    qboolean active;
    qboolean isLightmap;

    texture_t *image;

    colorGen_t rgbGen;
    alphaGen_t alphaGen;

    uint32_t stateBits;         // GLS_xxxx mask

    byte constantColor[4];      // for CGEN_CONST and AGEN_CONST

    vec4_t normalScale;
    vec4_t specularScale;
} shaderStage_t;

typedef struct shader_s {
    char name[MAX_GDR_PATH];

    shaderStage_t *stages[MAX_SHADER_STAGES];

    uint32_t sortedIndex;       // this shader == rg.sortedShaders[sortedIndex]
    uint32_t index;             // this shader == rg.shaders[index]
    shaderSort_t sort;

    uint32_t surfaceFlags;      // if explicitly defined this will have SURFPARM_* flags

    uint32_t vertexAttribs;

    qboolean polygonOffset;     // set for decals and other items that must be offset

    qboolean defaultShader;		// we want to return index 0 if the shader failed to
								// load for some reason, but R_FindShader should
								// still keep a name allocated for it, so if
								// something calls RE_RegisterShader again with
								// the same name, we don't try looking for it again

    qboolean isLightmap;

    struct shader_s *next;
} shader_t;

//==================================================================


typedef struct {
    uint32_t x, y, width, height;
    stereoFrame_t stereoFrame;
    mat4_t transformMatrix;

    qboolean drawn;

    uint64_t time;
    uint64_t flags;

    double floatTime;

    uint64_t numDrawSurfs;
    drawSurf_t *drawSurfs;

    uint64_t numEntities;
    renderEntityDef_t *entities;

    uint64_t numDLights;
    dlight_t *dlights;

    uint64_t numPolys;
    srfPoly_t *polys;

    renderCameraDef_t *camData;
} renderSceneDef_t;

typedef struct {
    vec3_t origin;
    float angle;
    float zoom;
    float aspect;
    mat4_t projectionMatrix;
    mat4_t modelMatrix;
    mat4_t transformMatrix;
} cameraData_t;

typedef struct {
    cameraData_t camera;

    uint32_t viewportX, viewportY, viewportWidth, viewportHeight;

    float zFar;
    float zNear;

    stereoFrame_t stereoFrame;

    uint64_t frameSceneNum;
    uint64_t frameCount;
} viewData_t;

typedef vec2_t refSprite_t[4];

typedef struct {
    char name[MAX_GDR_PATH];

    shader_t *shader;
    nhandle_t hShader;

    uint32_t numSprites;
    refSprite_t *texCoords;
    uint32_t spriteWidth, spriteHeight;
    uint32_t spriteCountX, spriteCountY;
} refSpriteSheet_t;

typedef struct {
    char baseName[MAX_GDR_PATH];
    char name[MAX_GDR_PATH];

    uint32_t width;
    uint32_t height;

    maplight_t *lights;
    uint32_t numLights;

    maptile_t *tiles;
    uint32_t numTiles;

    mapcheckpoint_t *checkpoints;
    uint32_t numCheckpoints;

    mapspawn_t *spawns;
    uint32_t numSpawns;

    uint64_t numIndices;
    uint64_t numVertices;

    tile2d_sprite_t *sprites;
    uint32_t numTilesetSprites;

    // frame based draw data
    srfTile_t *surface;
    shader_t *shader;
    drawVert_t *vertices;
    glIndex_t *indices;
    nhandle_t tileset;
} world_t;

typedef struct stageVars
{
//	color4ub_t	colors[MAX_BATCH_VERTICES];
//	vec2_t		texcoords[NUM_TEXTURE_BUNDLES][MAX_BATCH_VERTICES];
} stageVars_t;

typedef struct
{
    // silence the compiler warning
#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wattributes"
#endif
    glIndex_t indices[MAX_BATCH_INDICES] GDR_ALIGN(16);
    vec3_t xyz[MAX_BATCH_VERTICES] GDR_ALIGN(16);
    int16_t normal[MAX_BATCH_VERTICES] GDR_ALIGN(16);
    vec2_t texCoords[MAX_BATCH_VERTICES] GDR_ALIGN(16);
    uint16_t color[MAX_BATCH_VERTICES][4] GDR_ALIGN(16);

//    stageVars_t svars GDR_ALIGN(16);
#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

    void *attribPointers[ATTRIB_INDEX_COUNT];

    uint32_t numVertices;
    uint32_t numIndices;
    uint32_t firstIndex;

    shader_t *shader;
//    shaderStage_t **stages;
    vertexBuffer_t *buf;

    double shaderTime;

    qboolean useInternalVao;
    qboolean useCacheVao;
    qboolean updated;
} shaderDrawCommands_t;

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
} backendCounters_t;

typedef struct {
    viewData_t viewData;
    renderSceneDef_t refdef;

    byte color2D[4];

    qboolean depthFill;
    qboolean colorMask[4];

    backendCounters_t pc;
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

    mat4_t modelviewProjection;
    mat4_t projection;
    mat4_t modelview;

    uint32_t vertexAttribsEnabled;

    uint32_t    vaoId;
    uint32_t    vboId;
    uint32_t    iboId;
    uint32_t    defFboId;
    uint32_t    readFboId;
    uint32_t    writeFboId;
    uint32_t    rboId;
    uint32_t    shaderId;

    void *currentFbo; // unused
    vertexBuffer_t *currentVao;
    texture_t *currentTexture;
    shaderProgram_t *currentShader;
} glstate_t;

typedef struct
{
    texture_t *defaultImage;
    texture_t *whiteImage;

    shader_t *defaultShader;

    refSpriteSheet_t *spriteSheets[MAX_RENDER_SPRITESHEETS];
    uint64_t numSpriteSheets;

    vertexBuffer_t *buffers[MAX_RENDER_BUFFERS];
    uint64_t numBuffers;

    shader_t *shaders[MAX_RENDER_SHADERS];
    shader_t *sortedShaders[MAX_RENDER_SHADERS];
    uint64_t numShaders;

    shaderProgram_t *programs[MAX_RENDER_PROGRAMS];
    uint64_t numPrograms;

    texture_t *textures[MAX_RENDER_TEXTURES];
    uint64_t numTextures;

    viewData_t viewData;
    renderSceneDef_t refdef;

    world_t *world;
    qboolean worldLoaded;

    uint64_t viewCount;

    qboolean registered;

    uint64_t frameSceneNum;
    uint64_t frameCount;

    float identityLight;
    uint32_t identityLightByte;

    uint64_t frontEndMsec;

    GLuint samplers[MAX_TEXTURE_UNITS];

    shaderProgram_t basicShader;
    shaderProgram_t imguiShader;
} renderGlobals_t;

extern shaderDrawCommands_t drawBuf;
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

extern cvar_t *r_useExtensions;
extern cvar_t *r_allowLegacy;
extern cvar_t *r_allowShaders;
extern cvar_t *r_multisample;
extern cvar_t *r_overBrightBits;
extern cvar_t *r_ignorehwgamma;
extern cvar_t *r_normalMapping;
extern cvar_t *r_specularMapping;
extern cvar_t *r_genNormalMaps;
extern cvar_t *r_drawMode;
extern cvar_t *r_hdr;
extern cvar_t *r_ssao;
extern cvar_t *r_greyscale;
extern cvar_t *r_externalGLSL;
extern cvar_t *r_ignoreDstAlpha;
extern cvar_t *r_fullbright;
extern cvar_t *r_intensity;
extern cvar_t *r_singleShader;
extern cvar_t *r_glDebug;
extern cvar_t *r_textureBits;
extern cvar_t *r_stencilBits;
extern cvar_t *r_finish;
extern cvar_t *r_picmip;
extern cvar_t *r_roundImagesDown;
extern cvar_t *r_gammaAmount;
extern cvar_t *r_printShaders;
extern cvar_t *r_textureDetail;
extern cvar_t *r_textureFiltering;
extern cvar_t *r_speeds;
extern cvar_t *r_showImages;
extern cvar_t *r_skipBackEnd;
extern cvar_t *r_znear;
extern cvar_t *r_measureOverdraw;
extern cvar_t *r_ignoreGLErrors;
extern cvar_t *r_clear;
extern cvar_t *r_drawBuffer;
extern cvar_t *r_maxPolys;
extern cvar_t *r_maxPolyVerts;

extern cvar_t *r_imageUpsampleType;
extern cvar_t *r_imageUpsample;
extern cvar_t *r_imageUpsampleMaxSize;
extern cvar_t *r_colorMipLevels;

// OpenGL extensions
extern cvar_t *r_arb_texture_compression;
extern cvar_t *r_arb_framebuffer_object;
extern cvar_t *r_arb_vertex_array_object;
extern cvar_t *r_arb_vertex_buffer_object;
extern cvar_t *r_arb_texture_filter_anisotropic;
extern cvar_t *r_arb_texture_float;

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
void GL_BIndNullFramebuffers(void);
void GL_BindRenderbuffer(GLuint rbo);
void GL_BindNullRenderbuffer(void);
void GL_State(unsigned stateBits);
void GL_ClientState( int unit, unsigned stateBits );
void GL_SetDefaultState(void);
void RE_BeginFrame(stereoFrame_t stereoFrame);
void RE_EndFrame(uint64_t *frontEndMsec, uint64_t *backEndMsec);
void RB_ExecuteRenderCommands(const void *data);

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

//
// rgl_texture.c
//
void R_ImageList_f(void);
void R_DeleteTextures(void);
texture_t *R_FindImageFile( const char *name, imgType_t type, imgFlags_t flags );
void R_GammaCorrect( byte *buffer, uint64_t bufSize );
void R_UpdateTextures( void );
void R_InitTextures(void);
texture_t *R_CreateImage(  const char *name, byte *pic, uint32_t width, uint32_t height, imgType_t type, imgFlags_t flags, int internalFormat, GLenum picFormat );

extern int gl_filter_min, gl_filter_max;

//
// rgl_math.c
//
void Mat4Scale( float scale, const mat4_t in, mat4_t out );
void Mat4Rotate( const vec3_t v, float angle, const mat4_t in, mat4_t out );
void Mat4Zero( mat4_t out );
void Mat4Identity( mat4_t out );
void Mat4Copy( const mat4_t in, mat4_t out );
void Mat4Multiply( const mat4_t in1, const mat4_t in2, mat4_t out );
void Mat4Transform( const mat4_t in1, const vec4_t in2, vec4_t out );
qboolean Mat4Compare( const mat4_t a, const mat4_t b );
void Mat4Dump( const mat4_t in );
void Mat4Translation( vec3_t vec, mat4_t out );
void Mat4Ortho( float left, float right, float bottom, float top, float znear, float zfar, mat4_t out );
void Mat4View(vec3_t axes[3], vec3_t origin, mat4_t out);
void Mat4SimpleInverse( const mat4_t in, mat4_t out);
int NextPowerOfTwo(int in);
unsigned short FloatToHalf(float in);
float HalfToFloat(unsigned short in);

//
// rgl_main.c
//
GDR_EXPORT nhandle_t RE_RegisterSpriteSheet(const char *shaderName, uint32_t numSprites, uint32_t spriteWidth, uint32_t spriteHeight,
    uint32_t sheetWidth, uint32_t sheetHeight);
void R_SortDrawSurfs(drawSurf_t *drawSurfs, uint32_t numDrawSurfs);
void RB_MakeViewMatrix( void );
void R_RenderView(const viewData_t *parms);
void GL_CameraResize(void);
qboolean R_HasExtension(const char *ext);
void R_SortDrawSurfs(drawSurf_t *drawSurfs, uint32_t numDrawSurfs);
void R_AddDrawSurf(surfaceType_t *surface, shader_t *shader);
GDR_EXPORT void RE_BeginRegistration(gpuConfig_t *config);
void R_InitSamplers(void);

//
// rgl_scene.c
//
void RB_InstantQuad(vec4_t quadVerts[4]);
GDR_EXPORT void RE_AddPolyToScene( nhandle_t hShader, const polyVert_t *verts, uint32_t numVerts );
GDR_EXPORT void RE_AddPolyListToScene( const poly_t *polys, uint32_t numPolys );
GDR_EXPORT void RE_RenderScene( const renderSceneRef_t *fd );
GDR_EXPORT void RE_AddEntityToScene( const renderEntityRef_t *ent );
GDR_EXPORT void RE_BeginScene( const renderSceneRef_t *fd );
GDR_EXPORT void RE_EndScene( void );
GDR_EXPORT void RE_ClearScene( void );
void R_InitNextFrame( void );
void RB_CheckOverflow( uint32_t verts, uint32_t indexes );

extern void (*rb_surfaceTable[SF_NUM_SURFACE_TYPES])(void *);

//
// rgl_shader.c
//
void R_ShaderList_f(void);
nhandle_t RE_RegisterShaderFromTexture(const char *name, texture_t *image);
nhandle_t RE_RegisterShader(const char *name);
shader_t *R_GetShaderByHandle(nhandle_t hShader);
shader_t *R_FindShaderByName( const char *name );
shader_t *R_FindShader(const char *name);
void R_InitShaders( void );

//
// rgl_shade.c
//
void R_DrawElements(uint32_t numIndices, glIndex_t firstIndex);
void RB_BeginSurface(shader_t *shader);
void RB_EndSurface(void);

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
void RB_UpdateCache( uint32_t attribBits );
void R_ShutdownBuffer( vertexBuffer_t *vbo );

void VaoCache_Init(void);
void VaoCache_AddSurface(drawVert_t *verts, uint64_t numVerts, glIndex_t *indexes, uint64_t numIndexes);
void VaoCache_RecycleIndexBuffer(void);
void VaoCache_RecycleVertexBuffer(void);
void VaoCache_InitQueue(void);
void VaoCache_Commit(void);
void VaoCache_BindVao(void);
void VaoCache_CheckAdd(qboolean *endSurface, qboolean *recycleVertexBuffer, qboolean *recycleIndexBuffer, uint32_t numVerts, uint32_t numIndexes);

GDR_EXPORT void RE_BeginFrame(stereoFrame_t stereoFrame);
GDR_EXPORT void RE_EndFrame(uint64_t *frontEndMsec, uint64_t *backEndMsec);

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
    RC_DRAW_SURFS,
    RC_COLORMASK,

    // mainly called from the vm
    RC_DRAW_IMAGE,
    RC_SET_COLOR,

    RC_END_OF_LIST
} renderCmdType_t;

typedef struct {
    renderCmdType_t commandId;
    drawSurf_t *drawSurfs;
    uint32_t numDrawSurfs;
    viewData_t viewData;
    renderSceneDef_t refdef;
} drawSurfsCmd_t;

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
    viewData_t viewData;
} postProcessCmd_t;

typedef struct {
    drawSurf_t drawSurfs[MAX_DRAWSURFS];
    dlight_t dlights[MAX_DLIGHTS];
    renderEntityDef_t entities[MAX_RENDER_ENTITIES];
    polyVert_t *polyVerts;
    srfPoly_t *polys;

    uint64_t numPolys;
    uint64_t numDrawSurfs;

    renderCommandList_t commandList;
} renderBackendData_t;

extern renderBackendData_t *backendData;

GDR_EXPORT void RE_DrawImage( float x, float y, float w, float h, float u1, float v1, float u2, float v2, nhandle_t hShader );
GDR_EXPORT void RE_LoadWorldMap(const char *filename);
GDR_EXPORT void RE_SetColor(const float *rgba);
void R_AddDrawSurfCmd( drawSurf_t *drawSurfs, uint32_t numDrawSurfs );
void R_IssuePendingRenderCommands(void);

#endif