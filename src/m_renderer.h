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
int R_DrawMenu(const char* fontfile, const std::vector<std::string>& choices, const char* title);


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
    inline Vertex() = default;
    inline Vertex(const Vertex &) = default;
    inline Vertex(Vertex &&) = default;
    inline ~Vertex() = default;

    inline Vertex& operator=(const Vertex &v) {
        memmove(this, &v, sizeof(Vertex));
        return *this;
    }
};

typedef enum : uint8_t
{
    Null = 0,
    Float,
    Vec2,
    Vec3,
    Vec4,
    Int,
    Int2,
    Int3,
    Int4,
    UInt,
    UInt2,
    UInt3,
    UInt4,
    Mat2,
    Mat3,
    Mat4
} attribtype_t;

class VertexArray;

class VertexBuffer
{
private:
    mutable GLuint id;
    mutable std::vector<Vertex> vertices;
    bool has_been_allocated;
public:
    VertexBuffer(const size_t reserve)
        : vertices(reserve)
    {
        glCall(glGenBuffers(1, &id));
        Bind();
        glCall(glBufferData(GL_ARRAY_BUFFER, reserve * sizeof(Vertex), NULL, GL_DYNAMIC_DRAW));
        Unbind();
    }
    VertexBuffer(const size_t reserve, const std::shared_ptr<VertexArray>& vao);
    VertexBuffer() {
        glCall(glGenBuffers(1, &id));
    }
    VertexBuffer(const std::vector<Vertex>& _vertices, const VertexArray* vao);
    VertexBuffer(const Vertex* _vertices, const size_t count, const VertexArray* vao);
    VertexBuffer(const Vertex* _vertices, const size_t count, const std::shared_ptr<VertexArray>& vao);
    VertexBuffer(const std::vector<Vertex>& _vertices);
    VertexBuffer(const Vertex* _vertices, const size_t count);
    VertexBuffer(const VertexBuffer &vb)
        : vertices(vb.vertices.size()) { SwapBuffer(vb.vertices); }
    VertexBuffer(VertexBuffer &&) = default;
    ~VertexBuffer();

    inline void Draw(GLenum mode = GL_TRIANGLES, GLint first = 0, GLsizei count = 0) const {
        if (count == 0) count = vertices.size();
        glCall(glDrawArrays(mode, first, count));
    }
    inline void Bind() const
    { glCall(glBindBuffer(GL_ARRAY_BUFFER, id)); }
    inline void Unbind() const
    { glCall(glBindBuffer(GL_ARRAY_BUFFER, 0)); }
    inline size_t numvertices() const
    { return vertices.size(); }
    inline Vertex* data(void) const
    { return vertices.data(); }

    inline Vertex& front() const { return vertices.front(); }
    inline Vertex& back() const { return vertices.back(); }
    inline std::vector<Vertex>::iterator begin() { return vertices.begin(); }
    inline std::vector<Vertex>::iterator end() { return vertices.end(); }
    inline std::vector<Vertex>::const_iterator begin() const { return vertices.begin(); }
    inline std::vector<Vertex>::const_iterator end() const { return vertices.end(); }
    inline bool is_allocated() const { return has_been_allocated; }
    inline void GenBuffer() {
        glCall(glGenBuffers(1, &id));
        has_been_allocated = true;
    }

    void SwapBuffer(const std::vector<Vertex>& _vertices, bool newbuffer = false);
    void SwapBuffer(const Vertex* _vertices, const size_t offset, const size_t count);
    void SwapBuffer(const Vertex* _vertices, const size_t count, bool newbuffer = false);
    void SwapBuffer(std::initializer_list<Vertex> _vertices, bool newbuffer = false);
    void SwapBuffer(const VertexBuffer& vbo, bool newbuffer = false);

    static inline std::shared_ptr<VertexBuffer> Create(const size_t reserve, const std::shared_ptr<VertexArray>& vao) {
        return std::make_shared<VertexBuffer>(reserve, vao);
    }
    static inline std::shared_ptr<VertexBuffer> Create(const Vertex* vertices, const size_t count, const std::shared_ptr<VertexArray>& vao) {
        return std::make_shared<VertexBuffer>(vertices, count, vao);
    }
    static inline std::shared_ptr<VertexBuffer> Create(const std::vector<Vertex>& vertices) {
        return std::make_shared<VertexBuffer>(vertices);
    }
    static inline std::shared_ptr<VertexBuffer> Create(const Vertex* vertices, const size_t count) {
        return std::make_shared<VertexBuffer>(vertices, count);
    }
    static inline std::shared_ptr<VertexBuffer> Create(const size_t reserve) {
        return std::make_shared<VertexBuffer>(reserve);
    }

    inline GLuint& GetID() const { return id; }
};

class Shader
{
private:
    mutable std::unordered_map<std::string, GLint> uniformCache;
    GLuint id;

    GLuint Compile(const std::string& src, GLuint type)
    {
        glCall(GLuint id = glCreateShader(type));
        const char *buffer = src.c_str();
        glCall(glShaderSource(id, 1, &buffer, NULL));
        glCall(glCompileShader(id));
        int success;
        char infolog[512];
        glGetShaderiv(id, GL_COMPILE_STATUS, &success);
        if (success == GL_FALSE) {
            glGetShaderInfoLog(id, sizeof(infolog), NULL, infolog);
            LOG_ERROR("Shader::Compile: failed to compile shader: {}", infolog);
            glCall(glDeleteShader(id));
            return 0;
        }
        return id;
    }
    GLint GetUniformLocation(const std::string& name) const
    {
        if (uniformCache.find(name) != uniformCache.end())
            return uniformCache[name];
        
        glCall(GLint location = glGetUniformLocation(id, name.c_str()));
        if (location == -1) {
            LOG_WARN("glGetUniform location returned -1 for uniform named {}", name);
            return -1;
        }
        uniformCache[name] = location;
        return location;
    }
public:
    Shader(const std::string& filepath);
    Shader(const Shader &) = delete;
    Shader(Shader &&) = default;
    ~Shader();

    inline void Bind() const
    { glCall(glUseProgram(id)); }
    inline void Unbind() const
    { glCall(glUseProgram(0)); }

    inline void UniformMat4(const std::string& name, const glm::mat4& m) const
    { glCall(glUniformMatrix4fv(GetUniformLocation(name), 1, GL_FALSE, glm::value_ptr(m))); }
    inline void UniformInt(const std::string& name, GLint value) const
    { glCall(glUniform1i(GetUniformLocation(name), value)); }
    inline void UniformInt2(const std::string& name, GLint v1, GLint v2) const
    { glCall(glUniform2i(GetUniformLocation(name), v1, v2)); }
    inline void UniformFloat(const std::string& name, float value) const
    { glCall(glUniform1f(GetUniformLocation(name), value)); }
    inline void UniformVec2(const std::string& name, const glm::vec2& value) const
    { glCall(glUniform2f(GetUniformLocation(name), value.x, value.y)); }
    inline void UniformVec3(const std::string& name, const glm::vec3& value) const
    { glCall(glUniform3f(GetUniformLocation(name), value.x, value.y, value.z)); }
    inline void UniformVec4(const std::string& name, const glm::vec4& value) const
    { glCall(glUniform4f(GetUniformLocation(name), value.r, value.g, value.b, value.a)); }

    inline GLuint GetID() const { return id; }
};

class IndexBuffer
{
private:
    GLuint id;
    size_t indices_count = 0;
public:
    IndexBuffer(const size_t reserve);
    IndexBuffer() = default;
    IndexBuffer(const std::vector<uint32_t>& indices);
    IndexBuffer(const uint32_t* indices, const size_t count);
    IndexBuffer(IndexBuffer &&) = default;
    ~IndexBuffer();
    
    inline void Draw(GLenum mode, GLsizei count) const {
        Bind();
        glCall(glDrawElements(mode, count, GL_UNSIGNED_INT, NULL));
        Unbind();
    }
    inline void Bind() const
    { glCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, id)); }
    inline void Unbind() const
    { glCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0)); }
    inline size_t numindices() const
    { return indices_count; }

    void SwapBuffer(const uint32_t* _indices, const size_t count, bool newbuffer = false);
    void SwapBuffer(const std::vector<uint32_t>& _indices, bool newbuffer = false);
    void SwapBuffer(const IndexBuffer& ibo, bool newbuffer = false);

    static std::shared_ptr<IndexBuffer> Create(const std::vector<uint32_t>& indices) {
        return std::make_shared<IndexBuffer>(indices);
    }
    static std::shared_ptr<IndexBuffer> Create(const uint32_t* indices, const size_t count) {
        return std::make_shared<IndexBuffer>(indices, count);
    }

    inline GLuint GetID() const { return id; }
};

class VertexArray
{
private:
    mutable GLuint id;
    mutable std::vector<std::shared_ptr<VertexBuffer>> vbo;
    mutable std::shared_ptr<IndexBuffer> ibo;
public:
    VertexArray();
    VertexArray(std::initializer_list<Vertex> _vertices);
    VertexArray(std::initializer_list<Vertex> _vertices, const std::vector<uint32_t>& _indices);
    VertexArray(const std::vector<Vertex>& _vertices);
    VertexArray(const std::vector<Vertex>& _vertices, const std::vector<uint32_t>& _indices);
    VertexArray(const Vertex* _vertices, const size_t count);
    VertexArray(const Vertex* _vertices, const size_t vertices_count, const uint32_t *_indices, const size_t indices_count);
    VertexArray(const VertexArray &) = delete;
    VertexArray(VertexArray &&) = default;
    ~VertexArray();

    inline void Bind() const
    { glCall(glBindVertexArray(id)); }
    inline void Unbind() const
    { glCall(glBindVertexArray(0)); }
    inline void BindIBO() const
    { ibo->Bind(); }
    inline void UnbindIBO() const
    { ibo->Unbind(); }

    void DrawVBO(GLenum mode = GL_TRIANGLES, GLint first = 0, GLsizei count = 0) const;
    void DrawIBO(GLenum mode = GL_TRIANGLES) const;

    inline void SwapIBO(const std::shared_ptr<IndexBuffer>& _ibo)
    { ibo = _ibo; }

    inline void PushVBO(const Vertex* vertices, const size_t count)
    { vbo.emplace_back(std::make_shared<VertexBuffer>(vertices, count)); }
    inline void PushVBO(const std::vector<Vertex>& vertices)
    { vbo.emplace_back(std::make_shared<VertexBuffer>(vertices)); }
    inline void PushVBO(std::initializer_list<Vertex> vertices)
    { vbo.emplace_back(std::make_shared<VertexBuffer>(vertices)); }
    inline void PushVBO(const std::shared_ptr<VertexBuffer>& vb)
    { vbo.emplace_back(vb); }
    inline void PopVBO()
    { vbo.pop_back(); }
    inline void EraseVBO(std::vector<std::shared_ptr<VertexBuffer>>::iterator it) {
        vbo.erase(it);
    }
    
    inline std::vector<std::shared_ptr<VertexBuffer>>& GetVBO() const { return vbo; }
    inline std::shared_ptr<VertexBuffer>& front_vbo() const { return vbo.front(); }
    inline std::shared_ptr<VertexBuffer>& back_vbo() const { return vbo.back(); }

    inline std::vector<std::shared_ptr<VertexBuffer>>::iterator begin_vbo() { return vbo.begin(); }
    inline std::vector<std::shared_ptr<VertexBuffer>>::iterator end_vbo() { return vbo.end(); }
    inline std::vector<std::shared_ptr<VertexBuffer>>::const_iterator begin_vbo() const { return vbo.begin(); }
    inline std::vector<std::shared_ptr<VertexBuffer>>::const_iterator end_vbo() const { return vbo.end(); }
    inline size_t numvbo() const { return vbo.size(); }

    inline std::shared_ptr<IndexBuffer>& GetIBO() const { return ibo; }
    inline GLuint &GetID() const { return id; }

    static inline std::shared_ptr<VertexArray> Create(std::initializer_list<Vertex> vertices) {
        return std::make_shared<VertexArray>(std::initializer_list<Vertex>(vertices));
    }
    static inline std::shared_ptr<VertexArray> Create(std::initializer_list<Vertex> vertices, const std::vector<uint32_t>& indices) {
        return std::make_shared<VertexArray>(std::initializer_list<Vertex>(vertices), indices);
    }
    static inline std::shared_ptr<VertexArray> Create(const std::vector<Vertex>& vertices) {
        return std::make_shared<VertexArray>(vertices);
    }
    static inline std::shared_ptr<VertexArray> Create(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices) {
        return std::make_shared<VertexArray>(vertices, indices);
    }
    static inline std::shared_ptr<VertexArray> Create(const Vertex* vertices, const size_t count) {
        return std::make_shared<VertexArray>(vertices, count);
    }
    static inline std::shared_ptr<VertexArray> Create(void) {
        return std::make_shared<VertexArray>();
    }
};


class Texture
{
private:
    GLuint id;
    int width, height;
    int n;
    uint8_t* buffer;
public:
    Texture(const std::string& filepath);
    ~Texture();

    inline void Bind(uint8_t slot = 0) const {
        glCall(glActiveTexture(GL_TEXTURE0+slot));
        glCall(glBindTexture(GL_TEXTURE_2D, id));
    }
    inline void Unbind() const
    { glCall(glBindTexture(GL_TEXTURE_2D, 0)); }
};

// interface with the vao's shader
#define CAMERA_RENDER_MVP 0
#define CAMERA_RENDER_VPM 1
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

    inline void MoveUp() {
        m_CameraPos.x += -sin(glm::radians(m_Rotation)) * m_CameraSpeed;
        m_CameraPos.y += cos(glm::radians(m_Rotation)) * m_CameraSpeed;
    }
    inline void MoveDown() {
        m_CameraPos.x -= -sin(glm::radians(m_Rotation)) * m_CameraSpeed;
        m_CameraPos.y -= cos(glm::radians(m_Rotation)) * m_CameraSpeed;
    }
    inline void MoveLeft() {
        m_CameraPos.x -= cos(glm::radians(m_Rotation)) * m_CameraSpeed;
        m_CameraPos.y -= sin(glm::radians(m_Rotation)) * m_CameraSpeed;
    }
    inline void MoveRight() {
        m_CameraPos.x += cos(glm::radians(m_Rotation)) * m_CameraSpeed;
        m_CameraPos.y += sin(glm::radians(m_Rotation)) * m_CameraSpeed;
    }

    void CalculateViewMatrix();
};

class Renderer;

extern std::unique_ptr<Renderer> renderer;

class Renderer
{
private:
    mutable std::vector<std::shared_ptr<VertexArray>> vertexarrays;
    mutable std::vector<std::shared_ptr<Shader>> shaders;

    mutable std::unordered_map<GLuint, std::shared_ptr<IndexBuffer>> bound_ibo;
    mutable std::unordered_map<GLuint, std::shared_ptr<Shader>> bound_shaders;
    mutable std::unordered_map<GLuint, std::shared_ptr<VertexArray>> bound_vao;
    mutable std::unordered_map<GLuint, std::shared_ptr<VertexBuffer>> bound_vbo;
public:
    SDL_Window* window;
    SDL_GLContext context;
public:
    void AllocBuffers(const std::vector<Vertex>& _vertices);

    static void DrawSquare();
    static void Submit();
    static void Flush();
    static void BeginScene();
    static void EndScene();
    static void StartBatch();
    static void NextBatch();
    static void Draw();
    static void Submit(const std::vector<Vertex>& vertices);
    static void Submit(const Vertex* vertices, const size_t count);

    static inline std::vector<std::shared_ptr<Shader>>& GetShaders(void)
    { return renderer->shaders; }
    static inline std::vector<std::shared_ptr<VertexArray>>& GetVertexArrays(void)
    { return renderer->vertexarrays; }
    static inline std::shared_ptr<Shader> CreateShader(const std::string& filepath) {
        return renderer->shaders.emplace_back(std::make_shared<Shader>(filepath));
    }
    static inline std::shared_ptr<VertexArray> CreateVertexArray(std::initializer_list<Vertex> vertices) {
        return renderer->vertexarrays.emplace_back(std::make_shared<VertexArray>(vertices));
    }
    static inline std::shared_ptr<VertexArray> CreateVertexArray(const Vertex* vertices, const size_t count) {
        return renderer->vertexarrays.emplace_back(std::make_shared<VertexArray>(vertices, count));
    }

    static inline void Bind(const std::shared_ptr<VertexArray>& vao) {
        if (renderer->bound_vao.find(vao->GetID()) != renderer->bound_vao.end())
            return;
        
        renderer->bound_vao[vao->GetID()] = vao;
        vao->Bind();
    }
    static inline void Bind(const std::shared_ptr<VertexBuffer>& vbo) {
        if (renderer->bound_vbo.find(vbo->GetID()) != renderer->bound_vbo.end())
            return;
        
        renderer->bound_vbo[vbo->GetID()] = vbo;
        vbo->Bind();
    }
    static inline void Bind(const std::shared_ptr<IndexBuffer>& ibo) {
        if (renderer->bound_ibo.find(ibo->GetID()) != renderer->bound_ibo.end())
            return;
        
        renderer->bound_ibo[ibo->GetID()] = ibo;
        ibo->Bind();
    }
    static inline void Bind(const std::shared_ptr<Shader>& shader) {
        if (renderer->bound_shaders.find(shader->GetID()) != renderer->bound_shaders.end())
            return;
        
        renderer->bound_shaders[shader->GetID()] = shader;
        shader->Bind();
    }
};

extern std::vector<model_t> modelinfo;

void glDrawBatches(GLenum mode, GLsizei count, const Vertex* vertices);

#ifdef _NOMAD_DEBUG

const char *DBG_GL_SourceToStr(GLenum source);
const char *DBG_GL_TypeToStr(GLenum type);
const char *DBG_GL_SeverityToStr(GLenum severity);
void DBG_GL_ErrorCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message, const GLvoid *userParam);

#endif

void R_InitScene();

#endif