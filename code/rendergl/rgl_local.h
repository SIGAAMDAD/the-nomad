#ifndef _RGL_LOCAL_
#define _RGL_LOCAL_

#pragma once

#include "ngl.h"
#include "rgl_public.h"
#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_opengl3.h"

#define RENDER_MAX_UNIFORMS 1024

#define TEXTURE_FILTER_NEAREST 0
#define TEXTURE_FILTER_LINEAR 1
#define TEXTURE_FILTER_BILLINEAR 2
#define TEXTURE_FILTER_TRILINEAR 3

#ifdef _NOMAD_DEBUG
#define RENDER_ASSERT(x,...) if (!(x)) ri.N_Error("%s: %s",__func__,va(__VA_ARGS__))
#else
#define RENDER_ASSERT(x,...)
#endif

#include <SDL2/SDL_opengl.h>

typedef enum {
    TC_NONE,
    TC_S3TC, // for the GL_S3_s3tc extension
    TC_S3TC_ARB // for the GL_EXT_texture_compression_s3tc extension
} glTextureCompression_t;

typedef enum {
    RES_SQUARE = 0,
    RES_CIRCLE
} renderEntityShape_t;

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

    int maxViewportDims[2];
    float maxAnisotropy;
    int maxTextureUnits;
    int maxTextureSize;
    int maxBufferSize;
    int maxVertexAttribs;
    int maxTextureSamples;
    int maxVertexAttribStride;

    int maxVertexShaderUniforms;
    int maxVertexShaderVectors;
    int maxVertexShaderAtomicCounters;
    int maxVertexShaderAtomicBuffers;
    
    int maxFragmentShaderUniforms;
    int maxFragmentShaderTextures;
    int maxFragmentShaderVectors;
    int maxFragmentShaderAtomicCounters;
    int maxFragmentShaderAtomicBuffers;

    int maxMultisampleSamples;

    int maxFramebufferColorAttachments;    
    int maxFramebufferWidth;
    int maxFramebufferHeight;
    int maxFramebufferSamples;

    int maxRenderbufferSize;
    int maxProgramTexelOffset;

    int maxTextureLOD;

    int maxVaryingVectors;
    int maxVaryingVars;
    int maxUniformLocations;
    int maxUniformBufferBindings;
} glContext_t;

typedef struct
{
    uint32_t magFilter;
    uint32_t minFilter;
    uint32_t wrapS;
    uint32_t wrapT;
    uint32_t slot;
    
    uint32_t width;
    uint32_t height;
    uint32_t channels;
    uint32_t id;

    byte* data;
} texture_t;

typedef struct
{
    int32_t uniformCache[RENDER_MAX_UNIFORMS];

    char *vertexBuf;
    char *fragmentBuf;

    char *vertexFile;
    char *fragmentFile;

    uint32_t vertexBufLen;
    uint32_t fragmentBufLen;

    uint32_t programId;
} shader_t;

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

struct renderEntityRef_s
{
    vec4_t color;
    vec3_t worldPos;
    vec3_t screenPos;

    renderEntityShape_t shape;

    float size;
    float rotation;

    texture_t *sprite;
};

#include "rgl_vertexcache.h"

typedef struct
{
    uint32_t fboId;
    uint32_t colorIds[32];
    uint32_t depthId;

    uint32_t width;
    uint32_t height;
} framebuffer_t;

#pragma pack(push, 1)
typedef struct
{
    uint32_t bufferSize;
    uint32_t target;
    uint32_t bindingId;
    uint32_t index;
    uint32_t bufferId;
} shaderBuffer_t;
#pragma pack(pop)

typedef enum
{
    RC_SET_COLOR = 0,
    RC_DRAW_RECT,
    RC_DRAW_REF,
    RC_DRAW_TILE,

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
    renderCommandList_t commandList;

    drawVert_t *vertices;
    uint32_t usedVertices;
    uint32_t numVertices;

    uint32_t *indices;
    uint32_t usedIndices;
    uint32_t numIndices;

    uint32_t numDLights;

    uint32_t texId;
    uint32_t vaoId;
    uint32_t vboId;
    uint32_t iboId;
    uint32_t shaderId;

    shader_t *frameShader;
    vertexCache_t *frameCache;

    boost::thread renderThread;
    boost::mutex renderMutex;
    boost::condition_variable renderCondVar;
    eastl::atomic<bool> renderReady;
    eastl::atomic<bool> renderDone;
} renderBackend_t;

extern renderBackend_t backend;

typedef struct
{
    renderCmdType_t id;
    vec4_t color;
} setColorCmd_t;

typedef struct
{
    renderCmdType_t id;
    vec4_t color;
    vec3_t pos;
    float rotation;
    float size;
    qboolean filled;
    nhandle_t texture;
} drawRectCmd_t;

typedef struct
{
    renderCmdType_t id;
    drawVert_t vertices[6];
} drawTileCmd_t;

typedef struct
{
    renderCmdType_t id;
    vec3_t pos;
    float length;
    float width;
} drawLineCmd_t;

typedef struct
{
    renderCmdType_t id;
    renderEntityRef_t *ref;
} drawRefCmd_t;


typedef struct
{
    glm::mat4 projection;
    glm::mat4 viewMatrix;
    glm::mat4 vpm;

    glm::vec3 cameraPos;
    float rotation;
    float zoomLevel;
} camera_t;

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
extern cvar_t *r_useFramebuffer;        // use a framebuffer?
extern cvar_t *r_hdr;                   // is high-dynamic-range enabled?
extern cvar_t *r_texShaderLod;          // the lod integer passed to the shader
extern cvar_t *r_ssao;                  // screen-space ambient occlusion (SSAO) enabled?

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

GDR_INLINE uint32_t R_GetTextureDetail(void)
{
    uint32_t i;

    for (i = 0; i < arraylen(textureDetails); ++i) {
        if (!N_stricmpn(textureDetails[i].string, r_textureDetail->s, textureDetails[i].string_len))
            return textureDetails[i].num;
    }
    ri.Con_Printf(WARNING, "r_textureDetail is invalid, setting to medium");
    ri.Cvar_Set("r_textureDetail", va("%i", TEX_medium));
    return medium;
}

extern const mat4_t mat4_identity;

#include "rgl_framebuffer.h"

#endif