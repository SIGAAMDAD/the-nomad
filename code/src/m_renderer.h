#ifndef _M_RENDERER_
#define _M_RENDERER_

#include <glm/gtc/type_ptr.hpp>
#include "r_buffer.h"
#include "r_framebuffer.h"
#include ""
#include "r_texture.h"
#include "r_spritesheet.h"

#pragma once

void R_Init();
void R_ShutDown();

#ifdef PARANOID
#   ifdef __GNUC__
#       define debugbreak() __builtin_trap()
#   elif defined(_MSVC_VER)
#       define debugbreak() __debugbreak()
#   endif
#else
#   define debugbreak()
#endif

#define glCall(x) while (glGetError() != GL_NO_ERROR); x; GL_ASSERT(#x, glGetError() == GL_NO_ERROR)

struct Vertex
{
    glm::vec4 color;
    glm::vec3 pos;
    glm::vec2 texcoords;
    float texindex;

    inline Vertex(const glm::vec3& _pos, const glm::vec4& _color, const glm::vec2& _texcoords)
        : pos(_pos), color(_color), texcoords(_texcoords)
    {
    }
    inline Vertex(const glm::vec3& _pos, const glm::vec4& _color, const glm::vec2& _texcoords, float _texindex)
        : pos(_pos), color(_color), texcoords(_texcoords), texindex(_texindex)
    {
    }
    inline Vertex(const glm::vec3& _pos, const glm::vec4& _color)
        : pos(_pos), color(_color)
    {
    }
    inline Vertex(const glm::vec3& _pos, const glm::vec2& _texcoords)
        : pos(_pos), texcoords(_texcoords)
    {
    }
    inline Vertex(const glm::vec3& _pos, const glm::vec2& _texcoords, float _texindex)
        : pos(_pos), texcoords(_texcoords), texindex(_texindex)
    {
    }
    inline Vertex(const glm::vec3& _pos)
        : pos(_pos), color(0.0f)
    {
    }
    inline Vertex() = default;
    inline Vertex(const Vertex &) = default;
    inline Vertex(Vertex &&) = default;
    inline ~Vertex() = default;

    inline Vertex& operator=(const Vertex &v) {
        memmove(this, &v, sizeof(Vertex));
        return *this;
    }
};

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

class Renderer
{
public:
    gpuContext_t gpuContext;
    Camera* camera;
    SDL_Window* window;
public:
    Renderer() = default;
    Renderer(const Renderer &) = delete;
    Renderer(Renderer &&) = delete;
    ~Renderer() = default;

    Renderer& operator=(const Renderer &) = delete;
    Renderer& operator=(Renderer &&) = delete;
};

#ifdef _NOMAD_DEBUG

const char *DBG_GL_SourceToStr(GLenum source);
const char *DBG_GL_TypeToStr(GLenum type);
const char *DBG_GL_SeverityToStr(GLenum severity);
void DBG_GL_ErrorCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message, const GLvoid *userParam);

#endif

#endif