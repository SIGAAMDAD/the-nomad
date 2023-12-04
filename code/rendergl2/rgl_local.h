#ifndef __RGL_LOCAL__
#define __RGL_LOCAL__

#pragma once

#include "../engine/n_shared.h"
#include "../engine/gln_files.h"
#include "../engine/n_cvar.h"
#include "ngl.h"
#include "../rendercommon/r_public.h"
#include "../rendercommon/r_types.h"

#define MAX_RENDER_BUFFERS 2048
#define MAX_RENDER_PROGRAMS 2048
#define MAX_RENDER_TEXTURES 2048
#define MAX_RENDER_SHADERS 2048
#define MAX_RENDER_FBOs 64

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
#define MAX_BATCH_QUADS (1024*1024)
#define MAX_BATCH_VERTICES (MAX_BATCH_QUADS*4)
#define MAX_BATCH_INDICES (MAX_BATCH_QUADS*6)

// any change in the LIGHTMAP_* defines here MUST be reflected in
// R_FindShader()
#define LIGHTMAP_2D         -4	// shader is for 2D rendering
#define LIGHTMAP_BY_VERTEX  -3	// pre-lit triangle models
#define LIGHTMAP_WHITEIMAGE -2
#define LIGHTMAP_NONE       -1

// normal is unused for now
typedef struct
{
    glm::vec3 xyz;
    glm::vec2 uv;
    int16_t normal[4];
    uint16_t color[4];
} drawVert_t;

// when sgame directly specifies a polygon, it becomes a srfPoly_t
// as soon as it is called
typedef struct {
    nhandle_t       hShader;
    uint32_t        numVerts;
    polyVert_t      *verts;
} srfPoly_t;

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

//=========================================================================

class CVertexCache;
class CTexture;
class CShaderProgram;


typedef struct {
    glm::mat4 projectionMatrix;
    glm::mat4 viewMatrix;
    glm::mat4 modelViewProjection;
    
    glm::vec3 origin;

    float zoom;
    float rotation;
    float aspect;
} viewData_t;

typedef struct {
	GLuint		currenttextures[ MAX_TEXTURE_UNITS ];
	GLint	    currenttmu;
	qboolean	finishCalled;
	GLint		texEnv[2];
	uint32_t	glStateBits;
    uint32_t    storedGlState;
    uint32_t    currentArray;

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

//    void *currentFbo; // unused
    CVertexCache *currentVao;
    CTexture *currentTexture;
    CShaderProgram *currentShader;
} glstate_t;

extern glstate_t glState;

typedef enum {
    MI_NONE,
    MI_NVX,
    MI_ATI
} gpuMemInfo_t;

enum {
	TCR_NONE = 0x0000,
	TCR_RGTC = 0x0001,
	TCR_BPTC = 0x0002,
};

typedef enum {
    GL_DBG_NONE = 0,

    GL_DBG_KHR,
    GL_DBG_AMD,
    GL_DBG_ARB
} glDebugType_t;

typedef uint32_t textureCompressionRef_t;

typedef struct
{
    char vendor[1024];
    char renderer[1024];
    char version_str[1024];
    char glsl_version_str[1024];
    char extensions[8192];
    GLfloat version_f;
    GLint versionMajor, versionMinor;
    GLint glslVersionMajor, glslVersionMinor;
    GLint numExtensions;
    glTextureCompression_t textureCompression;
    textureCompressionRef_t textureCompressionRef;
    gpuMemInfo_t memInfo;
    glDebugType_t debugType;
    qboolean nonPowerOfTwoTextures;
    qboolean stereo;
    qboolean intelGraphics;
    qboolean swizzleNormalmap;

    GLint maxTextureUnits;
    GLint maxTextureSize;
    GLint vboTarget;
    GLint iboTarget;
    GLint maxSamples;
    GLint maxColorAttachments;
    GLint maxRenderBufferSize;

    GLfloat maxAnisotropy;

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

extern glContext_t glContext;

//=========================================================================

#include "rgl_vao.h"
#include "rgl_texture.h"
#include "rgl_program.h"
#include "rgl_shader.h"
#include "rgl_world.h"

//
// rgl_main.cpp
//
GDR_EXPORT void GDR_ATTRIBUTE((format(printf, 1, 2))) GL_LogComment(const char *fmt, ...);
GDR_EXPORT void GDR_ATTRIBUTE((format(printf, 1, 2))) GL_LogError(const char *fmt, ...);
GDR_EXPORT void GL_SetObjectDebugName(GLenum target, GLuint id, const char *name, const char *add);
GDR_EXPORT void RB_ShowImages(void);
GDR_EXPORT void GL_SetModelViewMatrix(const mat4_t m);
GDR_EXPORT void GL_SetProjectionMatrix(const mat4_t m);
GDR_EXPORT void GL_CheckErrors(void);
GDR_EXPORT void GL_BindNullTextures(void);
GDR_EXPORT void GL_PushTexture(CTexture *texture);
GDR_EXPORT void GL_PopTexture(void);
GDR_EXPORT void GL_BindTexture(int32_t tmu, CTexture *texture);
GDR_EXPORT void GL_BindNullRenderbuffer(void);
GDR_EXPORT bool GL_UseProgram(GLuint program);
GDR_EXPORT void GL_BindNullProgram(void);
GDR_EXPORT void GL_BindFramebuffer(GLenum target, GLuint fbo);
GDR_EXPORT void GL_BindNullFramebuffer(GLenum target);
GDR_EXPORT void GL_BindNullFramebuffers(void);
GDR_EXPORT void GL_BindRenderbuffer(GLuint rbo);
GDR_EXPORT void GL_BindNullRenderbuffer(void);
GDR_EXPORT void GL_State(uint32_t stateBits);
GDR_EXPORT void RE_BeginFrame(stereoFrame_t stereoFrame);
GDR_EXPORT void RE_EndFrame(uint64_t *frontEndMsec, uint64_t *backEndMsec);
GDR_EXPORT void RB_ExecuteRenderCommands(const void *data);
GDR_EXPORT void R_WorldToGL( drawVert_t *verts, const glm::vec3& position, float width, float height );
GDR_EXPORT void R_RenderView( void );

//
// rgl_extensions.c
//
GDR_EXPORT void R_InitExtensions(void);

//
// rgl_main.c
//
GDR_EXPORT void R_MakeViewMatrix( void );
GDR_EXPORT bool R_HasExtension( const char *ext );
GDR_EXPORT void RE_BeginRegistration( gpuConfig_t *config );

//
// rgl_scene.c
//
GDR_EXPORT void RB_InstantQuad(glm::vec4 quadVerts[4]);
GDR_EXPORT void RE_AddPolyToScene( nhandle_t hShader, const polyVert_t *verts, uint32_t numVerts );
GDR_EXPORT void RE_AddPolyListToScene( const poly_t *polys, uint32_t numPolys );
GDR_EXPORT void RE_RenderScene( const renderSceneRef_t *fd );
//GDR_EXPORT void RE_AddEntityToScene( const renderEntityRef_t *ent );
GDR_EXPORT void RE_BeginScene( const renderSceneRef_t *fd );
GDR_EXPORT void RE_EndScene( void );
//GDR_EXPORT void RE_ClearScene( void );
//GDR_EXPORT void R_InitNextFrame( void );

//
// rgl_shader.c
//
GDR_EXPORT void R_ShaderList_f( void );
GDR_EXPORT nhandle_t RE_RegisterShaderFromTexture( const char *name, CTexture *image);
GDR_EXPORT nhandle_t RE_RegisterShader( const char *name);
GDR_EXPORT CShader *R_GetShaderByHandle( nhandle_t hShader);
GDR_EXPORT CShader *R_FindShaderByName( const char *name );
GDR_EXPORT CShader *R_FindShader( const char *name);
GDR_EXPORT void R_InitShaders( void );

//
// rgl_draw.cpp
//
extern uint64_t r_numPolys, r_numPolyVerts;

GDR_EXPORT void R_DrawElements(uint32_t numIndices, uintptr_t firstIndex);
GDR_EXPORT void R_DrawWorld( void );
GDR_EXPORT void RE_DrawImage( float x, float y, float w, float h, float u1, float v1, float u2, float v2, nhandle_t hShader );
GDR_EXPORT void RE_SetColor(const float *rgba);
GDR_EXPORT void R_IssuePendingRenderCommands(void);

GDR_EXPORT void RE_LoadWorldMap( const char *filename );

GDR_EXPORT void RE_BeginFrame(stereoFrame_t stereoFrame);
GDR_EXPORT void RE_EndFrame(uint64_t *frontEndMsec, uint64_t *backEndMsec);

//==================================================================


typedef struct {
    uint32_t x, y, width, height;
    stereoFrame_t stereoFrame;

    qboolean drawn;

    uint64_t time;
    uint64_t flags;

    double floatTime;

//    uint64_t numEntities;
//    renderEntityDef_t *entities;

//    uint64_t numDLights;
//    dlight_t *dlights;

    uint64_t numPolys;
    srfPoly_t *polys;

//    renderCameraDef_t *camData;
} renderSceneDef_t;

//=========================================================================

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
    uintptr_t vtxOffset;        // current offset in vertex buffer
    uintptr_t idxOffset;        // current offset in index buffer
    
    uintptr_t vtxDataSize;      // size in bytes of each vertex
    uintptr_t idxDataSize;      // size in bytes of each index

    uint32_t maxVertices;       // total allocated vertices with glBufferData
    uint32_t maxIndices;        // total allocated indices with glBufferData

    void *vertices;             // address of the client vertices
    void *indices;              // address of the client indices

    CVertexCache *buffer;       // the buffer handle we're using for this batch
} batch_t;

typedef struct {
    byte color2D[4];

    qboolean depthFill;
    qboolean colorMask[4];

    backendCounters_t pc;

    CVertexCache *drawBuffer;

    renderSceneDef_t refdef;

    batch_t drawBatch;
} renderBackend_t;


typedef struct
{
//    CTexture *defaultImage;
//    CTexture *whiteImage;

    CShader *defaultShader;

    CVertexCache *buffers[MAX_RENDER_BUFFERS];
    uint64_t numBuffers;

    CShader *shaders[MAX_RENDER_SHADERS];
    uint64_t numShaders;

    CShaderProgram *programs[MAX_RENDER_PROGRAMS];
    uint64_t numPrograms;

    CTexture *textures[MAX_RENDER_TEXTURES];
    uint64_t numTextures;

    CRenderWorld *world;
    qboolean worldLoaded;

    uint64_t viewCount;

    qboolean registered;

    uint64_t frameSceneNum;
    uint64_t frameCount;

    float identityLight;
    uint32_t identityLightByte;

    uint64_t frontEndMsec;

    GLuint samplers[MAX_TEXTURE_UNITS];

    CShaderProgram basicShader;
    CShaderProgram imguiShader;
} renderGlobals_t;

extern renderGlobals_t rg;
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
extern cvar_t *r_customWidth;
extern cvar_t *r_customHeight;

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
    CShader *shader;
    float x, y;
    float w, h;
    float u1, v1;
    float u2, v2;
} drawImageCmd_t;

typedef struct {
    renderCmdType_t commandId;
//    viewData_t viewData;
} postProcessCmd_t;

typedef struct {
//    drawSurf_t drawSurfs[MAX_DRAWSURFS];
 //   dlight_t dlights[MAX_DLIGHTS];
 //   renderEntityDef_t entities[MAX_RENDER_ENTITIES];
    polyVert_t *polyVerts;
    srfPoly_t *polys;
    glIndex_t *indices;

    uint64_t numPolys;
//    uint64_t numDrawSurfs;

    renderCommandList_t commandList;
} renderBackendData_t;

extern renderBackendData_t *backendData;

#endif