#ifndef _M_RENDERER_
#define _M_RENDERER_

#include <glm/gtc/type_ptr.hpp>

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

    inline Vertex(const glm::vec3& _pos, const glm::vec4& _color)
        : pos(_pos), color(_color)
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
class VertexBuffer
{
public:
    GLuint id;
    size_t NumVertices;
public:
    VertexBuffer(size_t reserve);
    VertexBuffer(const void* data, size_t size);
    ~VertexBuffer();

    void Bind() const {
        glBindBuffer(GL_ARRAY_BUFFER, id);
    }
    void Unbind() const{ 
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    void PushVertexAttrib(GLint index, GLsizei count, GLenum type, GLboolean normalized, GLsizei stride, const void *offset) {
        Bind();
        glEnableVertexArrayAttrib(id, index);
        glVertexAttribPointer(index, count, type, normalized, stride, offset);
        Unbind();
    }
    void SetData(const void *data, size_t size);
    inline size_t GetCount() const { return NumVertices; }

    static std::shared_ptr<VertexBuffer> Create(const void *data, size_t count)
    { return std::make_shared<VertexBuffer>(data, count); }
    static std::shared_ptr<VertexBuffer> Create(size_t reserve)
    { return std::make_shared<VertexBuffer>(reserve); }
};
// Currently Hazel only supports 32-bit index buffers
class IndexBuffer
{
private:
    GLuint id;
    size_t NumIndices;
public:
    IndexBuffer(uint32_t* indices, size_t count);
    ~IndexBuffer();

    void Bind() const {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, id);
    }
    void Unbind() const {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }

    size_t GetCount() const { return NumIndices; }

    static std::shared_ptr<IndexBuffer> Create(uint32_t* indices, size_t count) {
        return std::make_shared<IndexBuffer>(indices, count);
    }
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
        glEnableVertexAttribArray(index);
        glVertexAttribPointer(index, count, type, normalized, stride, offset);
        Unbind();
    }
    void AddVertexBuffer(VertexBuffer* const vertexBuffer);
    void SetIndexBuffer(const std::shared_ptr<IndexBuffer>& _indexBuffer) {
        indexBuffer = _indexBuffer;
    }
    
    const nomadvector<std::shared_ptr<VertexBuffer>> GetVertexBuffers() const { return vertexBuffers; }
    const std::shared_ptr<IndexBuffer> GetIndexBuffer() const { return indexBuffer; }


    static std::shared_ptr<VertexArray> Create()
    { return std::make_shared<VertexArray>(); }
};

class Shader
{
private:
    GLuint id;
    mutable std::unordered_map<std::string, GLint> uniformCache;
    mutable std::unordered_map<GLenum, std::string> GLSL_Src;
    GLint GetUniformLocation(const std::string& name) const
    {
        if (uniformCache.find(name) != uniformCache.end())
            return uniformCache[name];
        
        GLint location = glGetUniformLocation(id, name.c_str());
        if (location == -1) {
            LOG_WARN("failed to get location of uniform {}", name);
            return 0;
        }
        uniformCache[name] = location;
        return location;
    }
    GLuint Compile(const std::string& src, GLenum type);
    GLuint CreateProgram(const std::string& filepath);
public:
    Shader(const std::string& filepath);
    ~Shader();

    void Bind() const;
    void Unbind() const;

    void SetInt(const std::string& name, int value)
    { Uniform1i(name, value); }
    void SetIntArray(const std::string& name, int* values, uint32_t count)
    { Uniformiv(name, values, count); }
	void SetFloat(const std::string& name, float value)
    { Uniform1f(name, value); }
	void SetFloat2(const std::string& name, const glm::vec2& value)
    { Uniform2f(name, value); }
	void SetFloat3(const std::string& name, const glm::vec3& value)
    { Uniform3f(name, value); }
	void SetFloat4(const std::string& name, const glm::vec4& value)
    { Uniform4f(name, value); }
	void SetMat4(const std::string& name, const glm::mat4& value)
    { UniformMat4(name, value); }

    inline void Uniform1i(const std::string& name, int value) const
    { glUniform1i(GetUniformLocation(name), value); }
    inline void Uniformiv(const std::string& name, int *values, uint32_t count) const
    { glUniform1iv(GetUniformLocation(name), count, values); }
    inline void Uniform1f(const std::string& name, float value) const
    { glUniform1f(GetUniformLocation(name), value); }
    inline void Uniform2f(const std::string& name, const glm::vec2& value) const
    { glUniform2f(GetUniformLocation(name), value.x, value.y); }
    inline void Uniform3f(const std::string& name, const glm::vec3& value) const
    { glUniform3f(GetUniformLocation(name), value.x, value.y, value.z); }
    inline void Uniform4f(const std::string& name, const glm::vec4& value) const
    { glUniform4f(GetUniformLocation(name), value.r, value.g, value.b, value.a); }
    inline void UniformMat4(const std::string& name, const glm::mat4& value) const
    { glUniformMatrix4fv(GetUniformLocation(name), 1, GL_FALSE, glm::value_ptr(value)); }

	static std::shared_ptr<Shader> Create(const std::string& filepath) {
        return std::make_shared<Shader>(filepath);
    }
};


class SpriteRenderer
{
public:
	SpriteRenderer(Shader* const shader);
	~SpriteRenderer();

	void drawSprite(const glm::vec2& position,
		const glm::vec2& size = glm::vec2(10, 10), GLfloat rotate = 0.0f, const glm::vec4& color = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));

private:
	mutable Shader* shader;
	mutable VertexArray* vertexArray;
};

class Camera
{
private:
    mutable glm::mat4 m_ProjectionMatrix;
    mutable glm::mat4 m_ViewMatrix;
    mutable glm::mat4 m_ViewProjectionMatrix;
    mutable glm::vec3 m_CameraPos;
    mutable float m_Rotation = 0.0f;
    mutable float m_ZoomLevel = 1.0f;
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

    void RotateRight() {
        m_Rotation += m_CameraRotationSpeed;
        if (m_Rotation > 180.0f)
            m_Rotation -= 360.0f;
        else if (m_Rotation <= -180.0f)
            m_Rotation += 360.0f;
    }
    void RotateLeft() {
        m_Rotation -= m_CameraRotationSpeed;
        if (m_Rotation > 180.0f)
            m_Rotation -= 360.0f;
        else if (m_Rotation <= -180.0f)
            m_Rotation += 360.0f;
    }
    void MoveUp() {
        m_CameraPos.x += -sin(glm::radians(m_Rotation)) * m_CameraSpeed;
        m_CameraPos.y += cos(glm::radians(m_Rotation)) * m_CameraSpeed;
    }
    void MoveDown() {
        m_CameraPos.x -= -sin(glm::radians(m_Rotation)) * m_CameraSpeed;
        m_CameraPos.y -= cos(glm::radians(m_Rotation)) * m_CameraSpeed;
    }
    void MoveLeft() {
        m_CameraPos.x -= cos(glm::radians(m_Rotation)) * m_CameraSpeed;
        m_CameraPos.y -= sin(glm::radians(m_Rotation)) * m_CameraSpeed;
    }
    void MoveRight() {
        m_CameraPos.x += cos(glm::radians(m_Rotation)) * m_CameraSpeed;
        m_CameraPos.y += sin(glm::radians(m_Rotation)) * m_CameraSpeed;
    }

    void CalculateViewMatrix();
};

class Renderer;

extern Renderer* renderer;

class Renderer
{
public:
    Camera*camera;
    SDL_Window* window;
    SDL_GLContext context;
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