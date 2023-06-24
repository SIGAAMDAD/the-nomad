#ifndef _M_RENDERER_
#define _M_RENDERER_

#include <glm/gtc/type_ptr.hpp>
#include "r_vertexcache.h"
#include "r_shader.h"
#include "r_framebuffer.h"
#include "r_texture.h"
#include "r_spritesheet.h"

#pragma once

void R_Init();
void R_ShutDown();

#define RENDER_ASSERT(x,...) \
{\
    if (!(x)) {\
        char bigbuffer[1024];\
        stbsp_snprintf(bigbuffer, sizeof(bigbuffer), __VA_ARGS__);\
        N_Error("%s: %s",__func__,bigbuffer);\
    }\
}

class GPUContext
{
public:
    GLuint memObj;
    int gpu_memory_total;
    int gpu_memory_available;

    const char* renderer;
    const char* version;
    const char* vendor;

    int version_major;
    int version_minor;

    int num_extensions;
    char** extensions;

    int num_glsl_extensions;
    const char* glsl_extensions;

    static GPUContext* gpuContext;
public:
    GPUContext();
    ~GPUContext();
};

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

    GDR_INLINE glm::mat4& GetProjection() const { return m_ProjectionMatrix; }
    GDR_INLINE glm::mat4& GetViewMatrix() const { return m_ViewMatrix; }
    GDR_INLINE glm::mat4& GetVPM() const { return m_ViewProjectionMatrix; }
    GDR_INLINE glm::vec3& GetPos() const { return m_CameraPos; }
    GDR_INLINE float& GetRotation() const { return m_Rotation; }
    GDR_INLINE float GetRotationSpeed() const { return m_CameraRotationSpeed; }
    GDR_INLINE float GetSpeed() const { return m_CameraSpeed; }
    GDR_INLINE glm::mat4& CalcMVP(const glm::vec3& translation) const
    {
        glm::mat4 model = glm::translate(m_ViewProjectionMatrix, translation);
        static glm::mat4 mvp = m_ProjectionMatrix * m_ViewProjectionMatrix * model;
        return mvp;
    }
    GDR_INLINE glm::mat4 CalcVPM() const { return m_ProjectionMatrix * m_ViewMatrix; }
    constexpr GDR_INLINE Camera& operator=(const Camera& other)
    {
        m_ProjectionMatrix = other.m_ProjectionMatrix;
        m_ViewMatrix = other.m_ViewMatrix;
        m_CameraPos = other.m_CameraPos;
        m_Rotation = other.m_Rotation;
        m_ZoomLevel = other.m_ZoomLevel;
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

    void CalculateViewMatrix();
};

class Renderer;

extern Renderer* renderer;

struct VKContext
{
    VkInstance instance;
    VkSurfaceKHR surface;
    VkApplicationInfo appInfo;
    VkInstanceCreateInfo createInfo;
    VkPhysicalDevice device;
    VkXcbSurfaceCreateInfoKHR surfaceInfo;
};

typedef union gpuContext_u
{
    SDL_GLContext context;
    VKContext* instance;
} gpuContext_t;

void WorldToScreen(const glm::vec3& in, glm::vec3& out);
void RE_DrawPints(SpriteSheet* sheet, vertexCache_t *cache);
bool R_CullVertex(const glm::vec2& pos);

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
    gpuContext_t gpuContext;
    Camera camera;
    SDL_Window* window;

    shader_t* shaders[MAX_SHADERS];
    texture_t* textures[MAX_TEXTURES];
    framebuffer_t* fbos[MAX_FBOS];
    vertexCache_t* vertexCaches[MAX_VERTEXCACHES];

    uint32_t numVertexCaches, numTextures, numFBOs, numShaders;
    uint32_t shaderid, vaoid, vboid, iboid, textureid, fboid;
public:
    Renderer() = default;
    Renderer(const Renderer &) = delete;
    Renderer(Renderer &&) = delete;
    ~Renderer();

    Renderer& operator=(const Renderer &) = delete;
    Renderer& operator=(Renderer &&) = delete;
};

void R_BeginFramebuffer(framebuffer_t* fbo);
void R_EndFramebuffer(void);
void R_BindTexture(const texture_t* texture, uint32_t slot = 0);
void R_UnbindTexture(void);
void R_UnbindShader(void);
void R_BindShader(const shader_t* shader);
void R_UnbindShader(void);

extern bool imgui_on;

#ifdef _NOMAD_DEBUG

const char *DBG_GL_SourceToStr(GLenum source);
const char *DBG_GL_TypeToStr(GLenum type);
const char *DBG_GL_SeverityToStr(GLenum severity);
void DBG_GL_ErrorCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message, const GLvoid *userParam);

#endif

typedef struct renderEntity_s renderEntity_t;

renderEntity_t RE_InitEntity(const char *texchunk, const char *name, vec2_t worldpos);
void RE_DestroyHandle(renderEntity_t id);
void RE_CmdDrawEntity(renderEntity_t id);

#endif