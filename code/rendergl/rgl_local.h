#ifndef _RGL_LOCAL_
#define _RGL_LOCAL_

#pragma once

#include "ngl.h"
#include "rgl_public.h"
#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_sdl2.h"

#define RENDER_MAX_UNIFORMS 1024

#define TEXTURE_FILTER_NEAREST 0
#define TEXTURE_FILTER_LINEAR 1
#define TEXTURE_FILTER_BILLINEAR 2
#define TEXTURE_FILTER_TRILINEAR 3
#define RENDER_MAX_QUADS 0x80000
#define RENDER_MAX_VERTICES (RENDER_MAX_QUADS*4)
#define RENDER_MAX_INDICES (RENDER_MAX_VERTICES*6)

#define RENDER_ASSERT(x,...) if (!(x)) ri.N_Error("%s: %s",__func__,va(__VA_ARGS__))

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

    uint32_t vertexBufLen;
    uint32_t fragmentBufLen;

    uint32_t programId;
} shader_t;

typedef struct vertex_s
{
	glm::vec3 pos;
	glm::vec2 texcoords;
	glm::vec4 color;
	float type;
	
	inline vertex_s(const glm::vec3& _pos, const glm::vec2& _texcoords, const glm::vec4& _color)
		: pos(_pos), texcoords(+texcoords), color(_color) { }
    inline vertex_s(void)
		: pos(0.0f), texcoords(0.0f), color(0.0f) { }
	inline vertex_s(const vertex_s &v)
		: pos(v.pos), texcoords(v.texcoords), color(v.color) { }
	inline vertex_s(vertex_s &&v)
		: pos(v.pos), texcoords(v.texcoords), color(v.color) { }
    inline ~vertex_s() = default;
} vertex_t;

typedef struct
{
	uint32_t index;
	uint32_t count;
	uint32_t type;
	uint32_t offset;
} vertexAttrib_t;

typedef struct
{
    vertex_t *vertices;
    uint32_t *indices;

    uint64_t numVertices;
    uint64_t numIndices;

    vertexAttrib_t *attribs;
    uint64_t numAttribs;
    uint64_t attribStride;

    uint32_t vaoId;
    uint32_t iboId;
    uint32_t vboId;
} vertexCache_t;

typedef struct
{
    uint32_t fboId;
    uint32_t colorIds[32];
    uint32_t depthId;

    uint32_t width;
    uint32_t height;
} framebuffer_t;

class Camera
{
private:
    mutable glm::mat4 m_ProjectionMatrix;
    mutable glm::mat4 m_ViewMatrix;
    mutable glm::mat4 m_ViewProjectionMatrix;
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

#define MAX_FBOS 64
#define MAX_VERTEXCACHES 1024
#define MAX_SHADERS 1024
#define MAX_TEXTURES 1024

/*
the renderer class manages all the opengl objects, initializes them, and then deletes them upon destruction
*/
class Renderer
{
public:
    SDL_GLContext instance;
    Camera camera;
    SDL_Window* window;

    shader_t* shaders[MAX_SHADERS];
    texture_t* textures[MAX_TEXTURES];
    framebuffer_t* fbos[MAX_FBOS];
    vertexCache_t* vertexCaches[MAX_VERTEXCACHES];

    uint32_t numVertexCaches, numTextures, numFBOs, numShaders;
    uint32_t shaderid, vaoid, vboid, iboid, textureid, fboid;
public:
    Renderer();
    Renderer(const Renderer &) = delete;
    Renderer(Renderer &&) = delete;
    ~Renderer();

    Renderer& operator=(const Renderer &) = delete;
    Renderer& operator=(Renderer &&) = delete;
};

vertexCache_t* R_InitCache(const vertex_t *vertices, uint64_t numVertices, const uint32_t *indices, uint64_t numIndices);
void R_ShutdownCache(vertexCache_t *cache);
void R_SwapVertexData(const vertex_t *vertices, uint64_t numVertices, vertexCache_t* cache);
void R_SwapIndexData(const void *indices, uint64_t numIndices, vertexCache_t* cache);
void R_ResetVertexData(vertexCache_t *cache);
void R_ResetIndexData(vertexCache_t *cache);
void R_DrawCache(vertexCache_t *cache);
void R_BindCache(const vertexCache_t *cache);
void R_BindVertexArray(const vertexCache_t *cache);
void R_BindVertexBuffer(const vertexCache_t *cache);
void R_BindIndexBuffer(const vertexCache_t *cache);
void R_UnbindCache(void);
void R_UnbindVertexArray(void);
void R_UnbindVertexBuffer(void);
void R_UnbindIndexBuffer(void);

texture_t *R_InitTexture(const char *chunkname);
void R_ShutdownTexture(texture_t *texture);
void R_UpdateTextures(void);
void R_BindTexture(const texture_t *texture, uint32_t slot = 0);
void R_UnbindTexture(void);
void R_InitTexBuffer(texture_t *tex, bool withFramebuffer = false);
uint32_t R_TexFormat(void);
uint32_t R_TexMagFilter(void);
uint32_t R_TexMinFilter(void);
uint32_t R_TexFilter(void);
texture_t *R_GetTexture(const char *name);

shader_t *R_InitShader(const char *vertexFile, const char *fragmentFile);
void R_ShutdownShader(shader_t *shader);
void R_BindShader(const shader_t *shader);
void R_UnbindShader(void);
int32_t R_GetUniformLocation(shader_t *shader, const char *name);

inline void R_SetBool(shader_t *shader, const char *name, bool value)
{ glUniform1i(R_GetUniformLocation(shader, name), (GLint)value); }
inline void R_SetInt(shader_t *shader, const char *name, int32_t value)
{ glUniform1i(R_GetUniformLocation(shader, name), value); }
inline void R_SetIntArray(shader_t *shader, const char *name, int32_t *values, uint32_t count)
{ glUniform1iv(R_GetUniformLocation(shader, name), count, values); }
inline void R_SetFloat(shader_t *shader, const char *name, float value)
{ glUniform1f(R_GetUniformLocation(shader, name), value); }
inline void R_SetFloat2(shader_t *shader, const char *name, const glm::vec2& value)
{ glUniform2f(R_GetUniformLocation(shader, name), value.x, value.y); }
inline void R_SetFloat3(shader_t *shader, const char *name, const glm::vec3& value)
{ glUniform3f(R_GetUniformLocation(shader, name), value.x, value.y, value.z); }
inline void R_SetFloat4(shader_t *shader, const char *name, const glm::vec4& value)
{ glUniform4f(R_GetUniformLocation(shader, name), value.r, value.g, value.b, value.a); }
inline void R_SetMatrix4(shader_t *shader, const char *name, const glm::mat4& value)
{ glUniformMatrix4fv(R_GetUniformLocation(shader, name), 1, GL_FALSE, glm::value_ptr(value)); }

void load_gl_procs(NGLloadproc load);

extern Renderer *renderer;
extern renderImport_t ri;
extern glContext_t glContext;

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
extern cvar_t *r_anisotropicFiltering;
extern cvar_t *r_gammaAmount;
extern cvar_t *r_textureMagFilter;
extern cvar_t *r_textureMinFilter;
extern cvar_t *r_textureFiltering;
extern cvar_t *r_textureCompression;
extern cvar_t *r_textureDetail;
extern cvar_t *r_bloomOn;
extern cvar_t *r_useExtensions;

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

#include "rgl_framebuffer.h"

#endif