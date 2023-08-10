#ifndef _RGL_LOCAL_
#define _RGL_LOCAL_

#pragma once

#include "ngl.h"
#include "rgl_public.h"
#include "imgui.h"
#include "imgui_internal.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_sdl2.h"
#include <cglm/cglm.h>
#include <cglm/vec3.h>
#include <cglm/vec2.h>
#include <cglm/mat4.h>

#define RENDER_MAX_UNIFORMS 1024

#define TEXTURE_FILTER_NEAREST 0
#define TEXTURE_FILTER_LINEAR 1
#define TEXTURE_FILTER_BILLINEAR 2
#define TEXTURE_FILTER_TRILINEAR 3
#define RENDER_MAX_QUADS 0x80000
#define RENDER_MAX_VERTICES (RENDER_MAX_QUADS*4)
#define RENDER_MAX_INDICES (RENDER_MAX_QUADS*6)

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
    int maxAnisotropy;
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

typedef enum : uint32_t
{
    RC_SET_COLOR = 0,
    RC_DRAW_RECT,
    RC_DRAW_SPRITE,

    RC_END_LIST
} renderCmdType_t;

#define MAX_RC_BUFFER 64
#define MAX_RC_LIST 1024

#pragma pack(push, 1)
typedef struct
{
    char buffer[MAX_RC_BUFFER];
    renderCmdType_t id;
} renderCmd_t;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct
{
    renderCmd_t buffers[MAX_RC_LIST];
    uint32_t usedCommands;
} renderCommandList_t;
#pragma pack(pop)

typedef struct
{
    glm::vec2 pos;
} drawSprite_t;

typedef struct
{
    renderCommandList_t list;

    vertex_t *vertices;
    uint32_t usedVertices;
    uint32_t numVertices;

    uint32_t *indices;
    uint32_t usedIndices;
    uint32_t numIndices;
} renderBackend_t;

extern renderBackend_t backend;

typedef struct
{
    glm::vec4 color;
} setColorCmd_t;

typedef struct
{
    glm::vec4 color;
    glm::vec2 pos;
    float rotation;
    float size;
    qboolean filled;
} drawRectCmd_t;

typedef struct
{
    glm::vec2 pos;
    float length;
    float width;
} drawLineCmd_t;

typedef struct
{
    glm::vec2 pos;
    drawSprite_t *sprite;
    float rotation;
} drawSpriteCmd_t;

#if 0
typedef struct
{
    mat4 projection;
    mat4 viewMatrix;
    mat4 vpm;

    vec3 cameraPos;
    float rotation;
    float zoomLevel;
    float aspectRatio;
    float rotationSpeed;
    float moveSpeed;
} camera_t;

void RC_CalculateView(camera_t *camera);
#endif

class Camera
{
private:
    mutable glm::mat4 m_ProjectionMatrix;
    mutable glm::mat4 m_ViewMatrix;
    mutable glm::mat4 m_ViewProjectionMatrix;
    mutable glm::mat4 m_ModelViewMatrix;
    mutable glm::vec3 m_CameraPos;
    mutable float m_Rotation = 0.0f;
    mutable float m_ZoomLevel = 3.0f;
    float m_AspectRatio;
    float m_CameraRotationSpeed = 0.20f;
    float m_CameraSpeed = 0.25f;

    // only reason why it isn't a singleton is because MAYBE co-op, MAYBE...
public:
    Camera(float left, float right, float bottom, float top);
    Camera(const Camera &) = delete;
    Camera(Camera &&) = default;
    ~Camera() = default;

    inline float GetZoom(void) const { return m_ZoomLevel; }
    inline glm::mat4& GetProjection() const { return m_ProjectionMatrix; }
    inline glm::mat4& GetViewMatrix() const { return m_ViewMatrix; }
    inline glm::mat4& GetVPM() const { return m_ViewProjectionMatrix; }
    inline glm::vec3& GetPos() const { return m_CameraPos; }
    inline glm::mat4& GetModelViewMatrix() const { return m_ModelViewMatrix; }
    inline float& GetRotation() const { return m_Rotation; }
    inline float GetRotationSpeed() const { return m_CameraRotationSpeed; }
    inline float GetSpeed() const { return m_CameraSpeed; }
    inline glm::mat4& CalcMVP(const glm::vec3& translation) const
    {
        glm::mat4 model = glm::translate(m_ViewProjectionMatrix, translation);
        static glm::mat4 mvp = m_ProjectionMatrix * m_ViewProjectionMatrix * model;
        return mvp;
    }
    inline glm::mat4 CalcVPM() const { return m_ProjectionMatrix * m_ViewMatrix; }
    constexpr inline Camera& operator=(const Camera& other)
    {
        m_ProjectionMatrix = m_ProjectionMatrix;
        m_ViewMatrix = m_ViewMatrix;
        m_ViewProjectionMatrix = m_ViewProjectionMatrix;
        m_CameraPos = m_CameraPos;
        m_Rotation = m_Rotation;
        m_ZoomLevel = m_ZoomLevel;
        m_AspectRatio = m_AspectRatio;
        m_CameraRotationSpeed = m_CameraRotationSpeed;
        m_CameraSpeed = m_CameraSpeed;
        return *this;
    }

    void ZoomIn(void);
    void ZoomOut(void);
    void RotateRight(void);
    void RotateLeft(void);
    void MoveUp(void);
    void MoveDown(void);
    void MoveLeft(void);
    void MoveRight(void);

    void Resize(void);
    void SetProjection(float left, float right, float bottom, float top);
    void CalculateViewMatrix();
};


#if 0
struct VKContext
{
    VkInstance instance;
    VkSurfaceKHR surface;
    VkApplicationInfo appInfo;
    VkInstanceCreateInfo createInfo;
    VkPhysicalDevice device;
    VkXcbSurfaceCreateInfoKHR surfaceInfo;
};
#endif

#define RS_DRAWING 0x0000

typedef struct
{
    Camera camera;

    SDL_Window *window;
    SDL_GLcontext context;

    shader_t *shaders;
    uint64_t numShaders;

    drawSprite_t *sprites;
    uint64_t numSprites;

    uint32_t state;
    uint32_t drawMode;
} renderGlobals_t;

#define VEC3_TO_GLM(x) glm::vec3((x)[0],(x)[1],(x)[2])
#define VEC4_TO_GLM(x) glm::vec4((x)[0],(x)[1],(x)[2],(x)[3])

GO_AWAY_MANGLE void RE_InitSettings_f(void);

GO_AWAY_MANGLE shaderBuffer_t *R_InitShaderBuffer(uint32_t GLtarget, uint32_t bufSize, shader_t *shader, const char *block);
GO_AWAY_MANGLE void R_UpdateShaderBuffer(const void *data, shaderBuffer_t *buf);
GO_AWAY_MANGLE void R_ShutdownShaderBuffer(shaderBuffer_t *buf);
GO_AWAY_MANGLE void R_BindShaderBuffer(const shaderBuffer_t *buf);
GO_AWAY_MANGLE void R_UnbindShaderBuffer(void);
GO_AWAY_MANGLE void RE_ShutdownShaderBuffers(void);

GO_AWAY_MANGLE texture_t *R_InitTexture(const char *filename);
GO_AWAY_MANGLE void RE_ShutdownTextures(void);
GO_AWAY_MANGLE void R_UpdateTextures(void);
GO_AWAY_MANGLE void R_BindTexture(const texture_t *texture, uint32_t slot = 0);
GO_AWAY_MANGLE void R_UnbindTexture(void);
GO_AWAY_MANGLE void R_InitTexBuffer(texture_t *tex, bool withFramebuffer = false);
GO_AWAY_MANGLE uint32_t R_TexFormat(void);
GO_AWAY_MANGLE uint32_t R_TexMagFilter(void);
GO_AWAY_MANGLE uint32_t R_TexMinFilter(void);
GO_AWAY_MANGLE uint32_t R_TexFilter(void);
GO_AWAY_MANGLE texture_t *R_GetTexture(const char *name);

GO_AWAY_MANGLE void R_RecompileShader(shader_t *shader, const char **vertexMacros, const char **fragmentMacros,
                                                        uint32_t numVertexMacros, uint32_t numFragmentMacros);
GO_AWAY_MANGLE shader_t *R_InitShader(const char *vertexFile, const char *fragmentFile, const char **vertexMacros = NULL,
                                                                                        const char **fragmentMacros = NULL,
                                                                                        uint32_t numVertexMacros = 0,
                                                                                        uint32_t numFragmentMacros = 0);
GO_AWAY_MANGLE void RE_ShutdownShaders(void);
GO_AWAY_MANGLE void R_BindShader(const shader_t *shader);
GO_AWAY_MANGLE void R_UnbindShader(void);
GO_AWAY_MANGLE int32_t R_GetUniformLocation(shader_t *shader, const char *name);

inline void R_SetBool(shader_t *shader, const char *name, bool value)
{ nglUniform1iARB(R_GetUniformLocation(shader, name), (GLint)value); }
inline void R_SetInt(shader_t *shader, const char *name, int32_t value)
{ nglUniform1iARB(R_GetUniformLocation(shader, name), value); }
inline void R_SetIntArray(shader_t *shader, const char *name, int32_t *values, uint32_t count)
{ nglUniform1ivARB(R_GetUniformLocation(shader, name), count, values); }
inline void R_SetFloat(shader_t *shader, const char *name, float value)
{ nglUniform1fARB(R_GetUniformLocation(shader, name), value); }
inline void R_SetFloat2(shader_t *shader, const char *name, const glm::vec2& value)
{ nglUniform2fARB(R_GetUniformLocation(shader, name), value.x, value.y); }
inline void R_SetFloat3(shader_t *shader, const char *name, const glm::vec3& value)
{ nglUniform3fARB(R_GetUniformLocation(shader, name), value.x, value.y, value.z); }
inline void R_SetFloat4(shader_t *shader, const char *name, const glm::vec4& value)
{ nglUniform4fARB(R_GetUniformLocation(shader, name), value.r, value.g, value.b, value.a); }
inline void R_SetMatrix4(shader_t *shader, const char *name, const glm::mat4& value)
{ nglUniformMatrix4fvARB(R_GetUniformLocation(shader, name), 1, GL_FALSE, glm::value_ptr(value)); }

void load_gl_procs(NGLloadproc load);

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
extern cvar_t *r_EXT_anisotropicFiltering;
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

// for the truly old-school peeps
extern cvar_t *r_enableBuffers;     // forces immediate mode rendering
extern cvar_t *r_enableShaders;     // forces stuff like glPushAttrib

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

// texture details
typedef struct {
    const char *string;
    int num;
    int string_len;
} textureDetail_t;
enum {
    GPUvsGod = 0,
    extreme,
    high,
    medium,
    low,
    msdos
};
inline constexpr textureDetail_t textureDetails[] = {
    {"GPUvsGod", GPUvsGod, (int)sizeof("GPUvsGod")},
    {"extreme",  extreme,  (int)sizeof("extreme")},
    {"high",     high,     (int)sizeof("high")},
    {"medium",   medium,   (int)sizeof("medium")},
    {"low",      low,      (int)sizeof("low")},
    {"msdos",    msdos,    (int)sizeof("msdos")}
};

GDR_INLINE uint32_t R_GetTextureDetail(void)
{
    for (const auto& i : textureDetails) {
        if (!N_stricmpn(i.string, r_textureDetail->s, i.string_len))
            return i.num;
    }
    ri.Con_Printf(WARNING, "r_textureDetail is invalid, setting to medium");
    N_strncpyz(r_textureDetail->s, "medium", sizeof("medium"));
    return medium;
}

GO_AWAY_MANGLE void *R_FrameAlloc(uint32_t size);
GO_AWAY_MANGLE void *R_ClearedFrameAlloc(uint32_t size);
GO_AWAY_MANGLE void *R_StaticAlloc(uint32_t size);
GO_AWAY_MANGLE void *R_ClearedFrameAlloc(uint32_t size);
GO_AWAY_MANGLE void R_InitFrameMemory(void);
GO_AWAY_MANGLE void R_ShutdownFrameMemory(void);
GO_AWAY_MANGLE void R_ToggleFrame(void);
GO_AWAY_MANGLE uint64_t R_CountFrameMemory(void);

#include "rgl_framebuffer.h"

#endif