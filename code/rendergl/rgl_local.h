#ifndef _RGL_LOCAL_
#define _RGL_LOCAL_

#pragma once

#include "ngl.h"
#include "rgl_public.h"
#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_opengl3.h"
#include <SDL2/SDL_opengl.h>

#define GLN_INDEX_TYPE GL_UNSIGNED_INT
typedef uint32_t glIndex_t;

#define MAX_UNIFORM_BUFFERS 512
#define MAX_VERTEX_BUFFERS 1024
#define MAX_FRAMEBUFFERS 64
#define MAX_GLSL_OBJECTS 128
#define MAX_SHADERS 1024
#define MAX_TEXTURES 1024

// maximum amount of quads per batch buffer
#define MAX_FRAME_QUADS 8192
#define MAX_FRAME_VERTICES (MAX_FRAME_QUADS*4)
#define MAX_FRAME_INDICES (MAX_FRAME_QUADS*6)

typedef enum {
    TC_NONE,
    TC_S3TC, // for the GL_S3_s3tc extension
    TC_S3TC_ARB // for the GL_EXT_texture_compression_s3tc extension
} glTextureCompression_t;

typedef struct
{
    int GL_ARB_vertex_buffer_object;
} gpuInfo_t;

typedef struct
{
    char vendor[1024];
    char renderer[1024];
    char version_str[1024];
    char extensions[8192];
    float version_f;
    int version_i;
    int numExtensions;
    glTextureCompression_t textureCompression;
    qboolean nonPowerOfTwoTextures;
    qboolean stereo;
} glContext_t;

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
	IMGFLAG_NO_COMPRESSION = 0x0010,
	IMGFLAG_NOLIGHTSCALE   = 0x0020,
	IMGFLAG_CLAMPTOEDGE    = 0x0040,
	IMGFLAG_GENNORMALMAP   = 0x0080,
	IMGFLAG_LIGHTMAP       = 0x0100,
	IMGFLAG_NOSCALE        = 0x0200,
	IMGFLAG_CLAMPTOBORDER  = 0x0400,
} imgFlags_t;

typedef struct texture_s
{
    char *imgName;              // image path, including extension
    struct texture_s *next;     // for hash search
    struct texture_s *list;     // for listing

    uint32_t width, height;     // source image
    uint32_t uploadWidth;       // adter power of two but not including clamp to MAX_TEXTURE_SIZE
    uint32_t uploadHeight;
    uint32_t id;                // GL texture binding

    uint32_t internalFormat;
    imgType_t type;
    imgFlags_t flags;
} texture_t;

enum {
	ATTRIB_INDEX_POSITION       = 0,
	ATTRIB_INDEX_TEXCOORD       = 1,
    ATTRIB_INDEX_COLOR          = 2,
    ATTRIB_INDEX_ALPHA          = 3,
    ATTRIB_INDEX_NORMAL         = 4,
    ATTRIB_INDEX_TANGENT        = 5,
	
	ATTRIB_INDEX_COUNT
};

enum
{
    ATTRIB_POSITION             = BIT(ATTRIB_INDEX_POSITION),
	ATTRIB_TEXCOORD             = BIT(ATTRIB_INDEX_TEXCOORD),
    ATTRIB_COLOR                = BIT(ATTRIB_INDEX_COLOR),
    ATTRIB_ALPHA                = BIT(ATTRIB_INDEX_ALPHA),
    ATTRIB_NORMAL               = BIT(ATTRIB_INDEX_NORMAL),
    ATTRIB_TANGENT              = BIT(ATTRIB_INDEX_TANGENT),

	ATTRIB_BITS =
        ATTRIB_POSITION |
        ATTRIB_TEXCOORD | 
        ATTRIB_COLOR |
        ATTRIB_ALPHA |
        ATTRIB_NORMAL |
        ATTRIB_TANGENT
};

typedef enum {
    GLSL_INT = 0,
    GLSL_FLOAT,
    GLSL_VEC2,
    GLSL_VEC3,
    GLSL_VEC4,
    GLSL_MAT16,
} glslType_t;

typedef enum {
    UNIFORM_MODELVIEWPROJECTION = 0,

    UNIFORM_DIFFUSE_MAP,

    UNIFORM_BASECOLOR,
    UNIFORM_VERTCOLOR,
    UNIFORM_COLOR,

    UNIFORM_AMBIENT_LIGHT,
    UNIFORM_NUM_LIGHTS,
    UNIFORM_LIGHTS,

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

    renderEntityRef_t *ref;

    struct dlight_s *next;
    struct dlight_s *prev;
} dlight_t;

/*
used for rendering chunks of the tilemap into the player's view
*/
typedef struct
{
    shader_t *shader;

    tile_t **tiles;
    uint32_t numTilesX;
    uint32_t numTilesY;

    dlight_t *lights;
    uint32_t numLights;
} tileSurf_t;

typedef struct
{
    vec4_t color;
    vec3_t pos;
    vec2_t texcoords;
    vec2_t light;
} drawVert_t;

#include "rgl_vertexcache.h"

typedef struct
{
    uint32_t fboId;
    uint32_t colorBuffers[32];
    uint32_t depthBuffer;
    uint32_t packedStencilDepthBuffer;

    uint32_t width;
    uint32_t height;
} framebuffer_t;

typedef enum
{
    RC_SET_COLOR = 0,
    RC_SWAP_BUFFERS,
    RC_DRAW_REF,

    RC_END_LIST
} renderCmdType_t;

#define MAX_RC_BUFFER 0x80000

typedef struct
{
    byte buffer[MAX_RC_BUFFER];
    uint32_t usedBytes;
} renderCommandList_t;

typedef struct
{
    vec4_t color[MAX_FRAME_VERTICES] GDR_ALIGN(16);
    vec3_t xyz[MAX_FRAME_VERTICES] GDR_ALIGN(16);
    vec2_t texCoords[MAX_FRAME_VERTICES] GDR_ALIGN(16);
    glIndex_t indices[MAX_FRAME_INDICES] GDR_ALIGN(16);

    void *attribPointers[ATTRIB_INDEX_COUNT];

    uint64_t numVertices;
    uint64_t numIndices;
} drawBuffer_t;

typedef struct
{
    renderCommandList_t commandList;
    drawBuffer_t dbuf;

    drawVert_t *vertices;
    uint32_t usedVertices;
    uint32_t numVertices;

    uint32_t *indices;
    uint32_t usedIndices;
    uint32_t numIndices;

    uint32_t numDLights;

    uint32_t boundTexId;
    uint32_t vaoId;
    uint32_t vboId;
    uint32_t iboId;
    uint32_t shaderId;

    shader_t *frameShader;
    vertexCache_t *frameCache;

    nhandle_t cmdThread;
} renderBackend_t;

extern renderBackend_t *backend;

typedef struct
{
    mat4_t projection;
    mat4_t viewMatrix;
    mat4_t modelViewProjection;

    vec3_t cameraPos;
    float rotation;
    float zoomLevel;
} camera_t;

typedef struct
{
    texture_t *image;
    vec2_t sheetDims;
    vec2_t spriteDims;
    uint32_t numSprites;
} spritesheet_t;

#define RS_DRAWING          0x0000
#define RS_IN_FRAMBUFFER    0x2000

#define MAX_RENDER_ENTITIES 1024
#define MAX_RENDER_TEXTURES 1024
#define MAX_RENDER_SHADERS 1024
#define MAX_RENDER_CACHES 256

typedef struct renderGlobals_s
{
    camera_t camera;

    SDL_Window *window;
    SDL_GLContext context;

    shader_t *shaders[MAX_RENDER_SHADERS];
    uint32_t numShaders;

    // index 0 is always the local player entity
    renderEntityRef_t *entityDefs[MAX_RENDER_ENTITIES];
    uint32_t numEntityDefs;

    renderEntityRef_t *plRef;

    texture_t *textures[MAX_RENDER_TEXTURES];
    uint32_t numTextures;

    vertexCache_t *vertexCaches[MAX_RENDER_CACHES];
    uint32_t numVertexCaches;

    uint32_t stateBits;
    uint32_t drawMode;

    tileSurf_t *tileSurf;

    const nmap_t *mapData;
} renderGlobals_t;

#define VEC3_TO_GLM(x) glm::vec3((x)[0],(x)[1],(x)[2])
#define VEC4_TO_GLM(x) glm::vec4((x)[0],(x)[1],(x)[2],(x)[3])

extern "C" void RB_SetProjection(float left, float right, float bottom, float top);
extern "C" void RB_ZoomIn(void);
extern "C" void RB_ZoomOut(void);
extern "C" void RB_RotateLeft(void);
extern "C" void RB_RotateRight(void);
extern "C" void RB_MakeViewMatrix(void);
extern "C" void RB_MoveUp(void);
extern "C" void RB_MoveLeft(void);
extern "C" void RB_MoveRight(void);
extern "C" void RB_MoveDown(void);
extern "C" void RB_CameraInit(void);
extern "C" void RB_CameraResize(void);

extern "C" void *RB_BeginRenderThread(void *unused);
extern "C" void RB_EndRenderThread(void);

extern "C" void RE_InitPLRef(void);

extern "C" qboolean R_IsInView(const glm::vec3& pos);

extern "C" void RE_InitSettings_f(void);

extern "C" shaderBuffer_t *R_InitShaderBuffer(uint32_t GLtarget, uint32_t bufSize, shader_t *shader, const char *block);
extern "C" void R_UpdateShaderBuffer(const void *data, shaderBuffer_t *buf);
extern "C" void R_ShutdownShaderBuffer(shaderBuffer_t *buf);
extern "C" void R_BindShaderBuffer(const shaderBuffer_t *buf);
extern "C" void R_UnbindShaderBuffer(void);
extern "C" void RE_ShutdownShaderBuffers(void);

extern "C" texture_t *R_InitTexture(const char *filename);
extern "C" void RE_ShutdownTextures(void);
extern "C" void R_UpdateTextures(void);
extern "C" void R_BindTexture(const texture_t *texture);
extern "C" void R_UnbindTexture(void);
extern "C" void R_InitTexBuffer(texture_t *tex, qboolean withFramebuffer);
extern "C" uint32_t R_TexFormat(void);
extern "C" uint32_t R_TexMagFilter(void);
extern "C" uint32_t R_TexMinFilter(void);
extern "C" uint32_t R_TexFilter(void);
extern "C" texture_t *R_GetTexture(const char *name);

extern "C" void R_RecompileShader(shader_t *shader);
extern "C" void R_ShutdownShader(shader_t *shader);
extern "C" shader_t *R_InitShader(const char *vertexFile, const char *fragmentFile);
extern "C" void RE_ShutdownShaders(void);
extern "C" void R_BindShader(const shader_t *shader);
extern "C" void R_UnbindShader(void);
extern "C" int32_t R_GetUniformLocation(shader_t *shader, const char *name);

GDR_INLINE void R_SetBool(shader_t *shader, const char *name, qboolean value)
{ nglUniform1iARB(R_GetUniformLocation(shader, name), (GLint)value); }
GDR_INLINE void R_SetInt(shader_t *shader, const char *name, int32_t value)
{ nglUniform1iARB(R_GetUniformLocation(shader, name), value); }
GDR_INLINE void R_SetIntArray(shader_t *shader, const char *name, int32_t *values, uint32_t count)
{ nglUniform1ivARB(R_GetUniformLocation(shader, name), count, values); }
GDR_INLINE void R_SetFloat(shader_t *shader, const char *name, float value)
{ nglUniform1fARB(R_GetUniformLocation(shader, name), value); }
GDR_INLINE void R_SetFloat2(shader_t *shader, const char *name, const glm::vec2& value)
{ nglUniform2fARB(R_GetUniformLocation(shader, name), value[0], value[1]); }
GDR_INLINE void R_SetFloat3(shader_t *shader, const char *name, const glm::vec3& value)
{ nglUniform3fARB(R_GetUniformLocation(shader, name), value[0], value[1], value[2]); }
GDR_INLINE void R_SetFloat4(shader_t *shader, const char *name, const glm::vec4& value)
{ nglUniform4fARB(R_GetUniformLocation(shader, name), value[0], value[1], value[2], value[3]); }
GDR_INLINE void R_SetFloat2(shader_t *shader, const char *name, const float *value)
{ nglUniform2fARB(R_GetUniformLocation(shader, name), value[0], value[1]); }
GDR_INLINE void R_SetFloat3(shader_t *shader, const char *name, const float *value)
{ nglUniform3fARB(R_GetUniformLocation(shader, name), value[0], value[1], value[2]); }
GDR_INLINE void R_SetFloat4(shader_t *shader, const char *name, const float *value)
{ nglUniform4fARB(R_GetUniformLocation(shader, name), value[0], value[1], value[2], value[3]); }
GDR_INLINE void R_SetMatrix4(shader_t *shader, const char *name, const glm::mat4& value)
{ nglUniformMatrix4fvARB(R_GetUniformLocation(shader, name), 1, GL_FALSE, glm::value_ptr(value)); }

extern "C" void RB_ExecuteCommands(void);
extern "C" void RE_IssueRenderCommands(void);

extern "C" void RB_FlushVertices(void);
extern "C" void RB_PushRect(const drawVert_t *vertices);
extern "C" void RB_DrawRect(const drawRectCmd_t *cmd);
extern "C" void RB_DrawEntity(const drawRefCmd_t *cmd);
extern "C" void RB_DrawTile(const drawTileCmd_t *cmd);
extern "C" void RE_AddTile(const drawVert_t *vertices);
extern "C" void RE_RenderTilemap(void);
extern "C" void RE_InitFrameData(void);

extern "C" texture_t *R_TextureFromHandle(nhandle_t handle);

extern "C" void *R_FrameAlloc(uint32_t size);
extern "C" void R_InitFrameMemory(void);
extern "C" void R_ShutdownFrameMemory(void);

extern "C" void RB_ConvertCoords(drawVert_t *v, const glm::vec3& pos, uint32_t count);

extern "C" void load_gl_procs(NGLloadproc load);

extern renderGlobals_t rg;
extern renderImport_t ri;
extern glContext_t glContext;
extern shader_t *pintShader;

// cvars
extern cvar_t *r_ticrate;
extern cvar_t *r_screenheight;
extern cvar_t *r_screenwidth;
extern cvar_t *r_vsync;
extern cvar_t *r_fullscreen;
extern cvar_t *r_native_fullscreen;
extern cvar_t *r_hidden;
extern cvar_t *r_drawFPS;
extern cvar_t *r_renderapi;
extern cvar_t *r_multisampleAmount;
extern cvar_t *r_multisampleType;
extern cvar_t *r_dither;
extern cvar_t *r_gammaAmount;
extern cvar_t *r_textureMagFilter;
extern cvar_t *r_textureMinFilter;
extern cvar_t *r_textureFiltering;
extern cvar_t *r_textureCompression;
extern cvar_t *r_textureDetail;
extern cvar_t *r_bloomOn;
extern cvar_t *r_useExtensions;
extern cvar_t *r_fovWidth;
extern cvar_t *r_fovHeight;
extern cvar_t *r_aspectRatio;
extern cvar_t *r_enableClientState;     // uses glEnableClientState and stuff like that. Overrides buffers and immediate mode
extern cvar_t *r_useFramebuffer;        // use a framebuffer?
extern cvar_t *r_hdr;                   // is high-dynamic-range enabled?
extern cvar_t *r_texShaderLod;          // the lod integer passed to the shader
extern cvar_t *r_ssao;                  // screen-space ambient occlusion (SSAO) enabled?
extern cvar_t *r_flipTextureVertically; // stbi_set_flip_vertically_on_load(?)

// OpenGL extensions
extern cvar_t *r_EXT_anisotropicFiltering;
extern cvar_t *r_EXT_compiled_vertex_array;
extern cvar_t *r_ARB_texture_float;
extern cvar_t *r_ARB_texture_filter_anisotropic;
extern cvar_t *r_ARB_vertex_attrib_64bit;
extern cvar_t *r_ARB_vertex_buffer_object;
extern cvar_t *r_ARB_sparse_buffer;
extern cvar_t *r_ARB_vertex_attrib_binding;
extern cvar_t *r_ARB_texture_compression_bptc;
extern cvar_t *r_ARB_texture_compression_rgtc;
extern cvar_t *r_ARB_texture_compression;
extern cvar_t *r_ARB_pipeline_statistics_query;


// for the truly old-school peeps
extern cvar_t *r_enableBuffers;     // if 0, forces immediate mode rendering
extern cvar_t *r_enableShaders;     // if 0, forces deprecated matrix operations

#ifdef __cplusplus
// using libraries without using custom allocators is annoying, so this is here
#undef new
#undef delete

inline void *operator new( size_t s ) {
	return ri.Mem_Alloc( s );
}
inline void operator delete( void *p ) {
	ri.Mem_Free( p );
}
inline void *operator new[]( size_t s ) {
	return ri.Mem_Alloc( s );
}
inline void operator delete[]( void *p ) {
	ri.Mem_Free( p );
}
#endif

// texture details
typedef struct {
    const char *string;
    int num;
    int string_len;
} textureDetail_t;

#define TEX_GPUvsGod 0
#define TEX_xtreme 1
#define TEX_high 2
#define TEX_medium 3
#define TEX_low 4
#define TEX_msdos 5

extern const textureDetail_t textureDetails[6];

extern const mat4_t mat4_identity;

#include "rgl_framebuffer.h"

#endif