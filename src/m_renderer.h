#ifndef _M_RENDERER_
#define _M_RENDERER_

#include <glm/gtc/type_ptr.hpp>
#include "r_buffer.h"
#include "r_framebuffer.h"
#include "r_shader.h"
#include "r_texture.h"
#include "r_spritesheet.h"

#pragma once

#define HARDWARE_RENDERER_FLAGS (SDL_RENDERER_ACCELERATED | (scf::renderer::vsync ? SDL_RENDERER_ACCELERATED : SDL_RENDERER_PRESENTVSYNC))
#define SOFTWARE_RENDERER_FLAGS (SDL_RENDERER_SOFTWARE | (scf::renderer::vsync ? SDL_RENDERER_SOFTWARE : SDL_RENDERER_PRESENTVSYNC))

typedef struct model_s
{
    SDL_Rect screen_pos;
    SDL_Rect offset;
    SDL_Color color;
} model_t;

template <auto fn>
struct sdl_deleter {
    template <typename T>
    constexpr void operator()(T* arg) const { fn(arg); }
};

enum : int32_t
{
    MDL_PLAYER,
    MDL_MERC,
    MDL_HEALTH,
    MDL_ARMOR,
    MDL_COMPASS_UP,
    MDL_COMPASS_DOWN,
    MDL_COMPASS_RIGHT,
    MDL_COMPASS_LEFT,
    MDL_DOOR_OPEN,
    MDL_DOOR_CLOSED,
    MDL_FLOOR_OUTSIDE,
    MDL_FLOOR_INSIDE,
    MDL_WALL_VERTICAL,
    MDL_WALL_CORNER_BR,
    MDL_WALL_CORNER_BL,
    MDL_WALL_CORNER_TR,
    MDL_WALL_CORNER_TL,

    NUMMODELS
};

typedef std::unique_ptr<SDL_Texture, sdl_deleter<SDL_DestroyTexture>> texture_ptr;
typedef std::unique_ptr<SDL_Window, sdl_deleter<SDL_DestroyWindow>> window_ptr;
typedef std::unique_ptr<TTF_Font, sdl_deleter<TTF_CloseFont>> font_ptr;
typedef std::unique_ptr<SDL_Surface, sdl_deleter<SDL_FreeSurface>> surface_ptr;
typedef std::unique_ptr<SDL_Renderer, sdl_deleter<SDL_DestroyRenderer>> renderer_ptr;

#define DEFAULT_TEXT_SIZE 50

void R_Init();
void R_ShutDown();
void R_DrawScreen();
int R_DrawMenu(const char* fontfile, const nomadvector<std::string>& choices, const char* title);


#ifdef PARANOID
#   ifdef __GNUC__
#       define debugbreak() __builtin_trap()
#   elif defined(_MSVC_VER)
#       define debugbreak() __debugbreak()
#   endif
#else
#   define debugbreak()
#endif

#define GL_ASSERT_FILLER __FILE__,FUNC_SIG,__LINE__
#ifdef _NOMAD_DEBUG
#   define GL_ASSERT(op,x) \
{ \
    if (!(x)) { \
        fprintf(stderr, \
        "[OpenGL Error Thrown] %s:%s:%u\n" \
        "   operation: %s\n" \
        "   id: %i\n" \
        "   error: %s\n", \
        GL_ASSERT_FILLER,op,glGetError(),glGetString(glGetError())); \
        debugbreak(); \
    } \
}
#else
#   define GL_ASSERT(op,x) \
{ \
    if (!(x)) { \
        fprintf(stderr, \
        "[OpenGL Error Thrown] %s:%s:%u\n" \
        "   operation: %s\n" \
        "   id: %i\n" \
        "   error: %s\n", \
        GL_ASSERT_FILLER,op,glGetError(),glGetString(glGetError())); \
    } \
}
#endif

#define glCall(x) while (glGetError() != GL_NO_ERROR); x; GL_ASSERT(#x, glGetError() == GL_NO_ERROR)

struct Vertex
{
    glm::vec3 pos;
    glm::vec4 color;
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

enum class ShaderDataType
{
	None = 0, Float, Float2, Float3, Float4, Mat3, Mat4, Int, Int2, Int3, Int4, Bool
};

static uint32_t ShaderDataTypeSize(ShaderDataType type)
{
	switch (type) {
	case ShaderDataType::Float:    return 4;
	case ShaderDataType::Float2:   return 4 * 2;
	case ShaderDataType::Float3:   return 4 * 3;
	case ShaderDataType::Float4:   return 4 * 4;
	case ShaderDataType::Mat3:     return 4 * 3 * 3;
	case ShaderDataType::Mat4:     return 4 * 4 * 4;
	case ShaderDataType::Int:      return 4;
	case ShaderDataType::Int2:     return 4 * 2;
	case ShaderDataType::Int3:     return 4 * 3;
	case ShaderDataType::Int4:     return 4 * 4;
	case ShaderDataType::Bool:     return 1;
	};
    assert(false);
	if (!false)
        N_Error("Unknown ShaderDataType!");
	return 0;
}
struct BufferElement
{
    std::string Name;
	ShaderDataType Type;
	uint32_t Size;
	size_t Offset;
	bool Normalized;
	BufferElement() = default;
	BufferElement(ShaderDataType type, const std::string& name, bool normalized = false)
		: Name(name), Type(type), Size(ShaderDataTypeSize(type)), Offset(0), Normalized(normalized)
	{
	}
	uint32_t GetComponentCount() const
	{
		switch (Type) {
		case ShaderDataType::Float:   return 1;
		case ShaderDataType::Float2:  return 2;
		case ShaderDataType::Float3:  return 3;
		case ShaderDataType::Float4:  return 4;
		case ShaderDataType::Mat3:    return 3; // 3* float3
		case ShaderDataType::Mat4:    return 4; // 4* float4
		case ShaderDataType::Int:     return 1;
		case ShaderDataType::Int2:    return 2;
		case ShaderDataType::Int3:    return 3;
		case ShaderDataType::Int4:    return 4;
		case ShaderDataType::Bool:    return 1;
		};
        assert(false);
        if (!false)
            N_Error("invalid ShaderDataType");
        
		return 0;
	}
};
class BufferLayout
{
public:
	BufferLayout() {}
	
    BufferLayout(std::initializer_list<BufferElement> elements)
		: m_Elements(elements)
	{
		CalculateOffsetsAndStride();
	}
	size_t GetStride() const { return m_Stride; }

	const nomadvector<BufferElement>& GetElements() const { return m_Elements; }
	
    nomadvector<BufferElement>::iterator begin() { return m_Elements.begin(); }
	nomadvector<BufferElement>::iterator end() { return m_Elements.end(); }
	nomadvector<BufferElement>::const_iterator begin() const { return m_Elements.begin(); }
	nomadvector<BufferElement>::const_iterator end() const { return m_Elements.end(); }
private:
	void CalculateOffsetsAndStride()
	{
		size_t offset = 0;
		m_Stride = 0;
		for (auto& element : m_Elements)
		{
			element.Offset = offset;
			offset += element.Size;
			m_Stride += element.Size;
		}
	}
private:
	mutable nomadvector<BufferElement> m_Elements;
	size_t m_Stride = 0;
};
class VertexArray
{
private:
    GLuint id;
    size_t vertexBufferIndex = 0;
    mutable nomadvector<std::shared_ptr<VertexBuffer>> vertexBuffers;
    mutable std::shared_ptr<IndexBuffer> indexBuffer;
public:
    VertexArray();
    ~VertexArray();

    void Bind() const {
        glBindVertexArray(id);
    }
    void Unbind() const {
        glBindVertexArray(0);
    }

    void PushVertexAttrib(GLint index, GLsizei count, GLenum type, GLboolean normalized, GLsizei stride, const void *offset) {
        Bind();
        glEnableVertexAttribArrayARB(index);
        glVertexAttribPointerARB(index, count, type, normalized, stride, offset);
        Unbind();
    }
    void AddVertexBuffer(VertexBuffer* const vertexBuffer);
    void SetIndexBuffer(const std::shared_ptr<IndexBuffer>& _indexBuffer) {
        indexBuffer = _indexBuffer;
    }
    const nomadvector<std::shared_ptr<VertexBuffer>> GetVertexBuffers() const { return vertexBuffers; }
    const std::shared_ptr<IndexBuffer> GetIndexBuffer() const { return indexBuffer; }


    static VertexArray* Create(const std::string& name) {
        VertexArray* ptr = (VertexArray *)Z_Malloc(sizeof(VertexArray), TAG_STATIC, &ptr, name.c_str());
        glGenVertexArrays(1, &ptr->id);
        return ptr;
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

struct CameraBuffer
{
    glm::mat4 u_ViewProjection;
    glm::mat4 u_ProjectionMatrix;
    glm::mat4 u_ViewMatrix;
    glm::vec3 u_CameraPos;
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
    void AllocBuffers(const nomadvector<Vertex>& _vertices);

    static void DrawQuad(const glm::vec2& pos, const glm::vec2& size, const glm::vec4& color);
    static void Submit();
    static void Flush();
    static void BeginScene();
    static void EndScene();
    static void StartBatch();
    static void NextBatch();
    static void Draw();
    static void Submit(const nomadvector<Vertex>& vertices);
    static void Submit(const Vertex* vertices, const size_t count);

    static void Clear(void)
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }
};

extern nomadvector<model_t> modelinfo;


void glDrawBatches(GLenum mode, GLsizei count, const Vertex* vertices);
void glDrawDuplicate(GLenum mode, GLsizei amount, GLsizei numvertices, const Vertex* vertices);

#ifdef _NOMAD_DEBUG

const char *DBG_GL_SourceToStr(GLenum source);
const char *DBG_GL_TypeToStr(GLenum type);
const char *DBG_GL_SeverityToStr(GLenum severity);
void DBG_GL_ErrorCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message, const GLvoid *userParam);

#endif

void R_InitScene();

#endif