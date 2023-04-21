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
    glm::vec3 offset;
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

class VertexBuffer
{
private:
    GLuint id;
    mutable std::vector<Vertex> vertices;
    bool batch;
public:
    VertexBuffer(const std::initializer_list<Vertex>& _vertices, bool _batch = false)
        : vertices(_vertices), batch(_batch)
    {
        glCall(glGenBuffers(1, &id));
        glCall(glBindBuffer(GL_ARRAY_BUFFER, id));
        glCall(glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * vertices.size(), vertices.data(), GL_STATIC_DRAW));

        glCall(glEnableVertexArrayAttrib(id, 0));
        glCall(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void *)offsetof(Vertex, pos)));

        glCall(glEnableVertexArrayAttrib(id, 1));
        glCall(glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void *)offsetof(Vertex, color)));

        glCall(glBindBuffer(GL_ARRAY_BUFFER, 0));
    }
    VertexBuffer()
    {
        glCall(glGenBuffers(1, &id));
    }
    VertexBuffer(const VertexBuffer &) = delete;
    VertexBuffer(VertexBuffer &&) = default;
    ~VertexBuffer()
    {
        Unbind();
        glCall(glDeleteBuffers(1, &id));
    }

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
    inline void SwapBuffer(const std::vector<Vertex>& _vertices) {
        vertices.resize(_vertices.size());
        memmove(vertices.data(), _vertices.data(), sizeof(Vertex) * _vertices.size());
    }
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
    Shader(const std::string& filepath)
    {
        std::ifstream file(filepath, std::ios::in);
        if (!file)
            N_Error("Shader::Shader: failed to open shader file %s", filepath.c_str());
        
        assert(file.is_open());
        std::string line;
        int index = 0;
        std::stringstream stream[3];
        while (std::getline(file, line)) {
            if (line == "#shader vertex")
                index = 0;
            else if (line == "#shader fragment")
                index = 1;
            else if (line == "#shader geometry")
                index = 2;
            else
                stream[index] << line << '\n';
        }
        const std::string vertsrc = stream[0].str();
        const std::string fragsrc = stream[1].str();
        file.close();
        GLuint vertid = Compile(vertsrc, GL_VERTEX_SHADER);
        GLuint fragid = Compile(fragsrc, GL_FRAGMENT_SHADER);

        glCall(id = glCreateProgram());
        glCall(glAttachShader(id, vertid));
        glCall(glAttachShader(id, fragid));
        glCall(glLinkProgram(id));
        glCall(glValidateProgram(id));
        glCall(glDeleteShader(vertid));
        glCall(glDeleteShader(fragid));
        glCall(glUseProgram(0));
    }
    Shader()
    {
        id = glCreateProgram();
    }
    Shader(const Shader &) = delete;
    Shader(Shader &&) = default;
    ~Shader()
    {
        glCall(glDeleteProgram(id));
    }

    inline void Bind() const
    { glCall(glUseProgram(id)); }
    inline void Unbind() const
    { glCall(glUseProgram(0)); }

    inline void UniformMat4(const std::string& name, const glm::mat4& m) const
    {
        GLint location = GetUniformLocation(name);
        glCall(glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(m)));
    }
    inline void UniformInt(const std::string& name, GLint value) const
    {
        GLint location = GetUniformLocation(name);
        glCall(glUniform1i(location, value));
    }
    inline void UniformInt2(const std::string& name, GLint v1, GLint v2) const
    {
        GLint location = GetUniformLocation(name);
        glCall(glUniform2i(location, v1, v2));
    }
    inline void UniformFloat(const std::string& name, float value) const
    {
        GLint location = GetUniformLocation(name);
        glCall(glUniform1f(location, value));
    }
    inline void UniformVec2(const std::string& name, const glm::vec2& value) const
    {
        GLint location = GetUniformLocation(name);
        glCall(glUniform2f(location, value.x, value.y));
    }
    inline void UniformVec3(const std::string& name, const glm::vec3& value) const
    {
        GLint location = GetUniformLocation(name);
        glCall(glUniform3f(location, value.x, value.y, value.z));
    }
    inline void UniformVec4(const std::string& name, const glm::vec4& value) const
    {
        GLint location = GetUniformLocation(name);
        glCall(glUniform4f(location, value.r, value.g, value.b, value.a));
    }
};

class IndexBuffer
{
private:
    GLuint id;
    mutable std::vector<uint32_t> indices;
    bool batch;
public:
    IndexBuffer(std::initializer_list<uint32_t> _indices, bool _batch = false)
        : indices(_indices), batch(_batch)
    {
        glCall(glGenBuffers(1, &id));
        Bind();

        glCall(glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(uint32_t), indices.data(),
            (batch ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW)));

        Unbind();
    }
    IndexBuffer(const IndexBuffer &) = delete;
    IndexBuffer(IndexBuffer &&) = default;
    ~IndexBuffer()
    {
        Unbind();
        glCall(glDeleteBuffers(1, &id));
    }
    
    inline void Draw(GLenum mode = GL_TRIANGLES) const {
        glCall(glDrawElements(mode, indices.size(), GL_UNSIGNED_INT, indices.data()));
    }
    inline void Bind() const
    { glCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, id)); }
    inline void Unbind() const
    { glCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0)); }
    inline uint32_t* data() const { return indices.data(); }
    inline size_t numindices() const { return indices.size(); }
    inline void SwapBuffer(const std::vector<uint32_t>& _indices) {
        indices.resize(_indices.size());
        memmove(indices.data(), _indices.data(), sizeof(uint32_t) * _indices.size());
    }
};

class VertexArray
{
private:
    GLuint id;
    mutable std::shared_ptr<VertexBuffer> vbo;
    mutable std::shared_ptr<IndexBuffer> ibo;
    
    bool has_ibo;
    bool has_vbo;
public:
    VertexArray(std::initializer_list<Vertex> _vertices, bool batch = false)
        : has_ibo(true), has_vbo(false)
    {
        glCall(glGenVertexArrays(1, &id));
        Bind();

        vbo = std::make_shared<VertexBuffer>(_vertices, batch);

        Unbind();
    }
    VertexArray(std::initializer_list<Vertex> _vertices, std::initializer_list<uint32_t> _indices, bool batch = false)
        : has_ibo(true), has_vbo(true)
    {
        glCall(glGenVertexArrays(1, &id));
        Bind();

        vbo = std::make_shared<VertexBuffer>(_vertices, batch);
        ibo = std::make_shared<IndexBuffer>(_indices, batch);

        Unbind();
    }
    VertexArray()
    {
        glCall(glGenVertexArrays(1, &id));
    }
    VertexArray(const VertexArray &) = delete;
    VertexArray(VertexArray &&) = default;
    ~VertexArray()
    {
        Unbind();
        glCall(glDeleteVertexArrays(1, &id));
    }

    inline void Bind() const
    { glCall(glBindVertexArray(id)); }
    inline void Unbind() const
    { glCall(glBindVertexArray(0)); }
    inline void BindVBO() const
    { vbo->Bind(); }
    inline void UnbindVBO() const
    { vbo->Unbind(); }
    inline void BindIBO() const
    { ibo->Bind(); }
    inline void UnbindIBO() const
    { ibo->Unbind(); }
    inline bool HasVBO() const
    { return has_vbo; }
    inline bool HasIBO() const
    { return has_ibo; }
    inline std::shared_ptr<VertexBuffer>& GetVBO() const { return vbo; }
    inline std::shared_ptr<IndexBuffer>& GetIBO() const { return ibo; }
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

    // only reason why it isn't a singleton is because MAYBE co-op, MAYBE...
public:
    Camera(float left, float right, float bottom, float top, float zFar, float zNear);
    Camera(const Camera &) = delete;
    Camera(Camera &&) = default;
    ~Camera() = default;

    inline glm::mat4& GetProjection() const { return m_ProjectionMatrix; }
    inline glm::mat4& GetViewMatix() const { return m_ViewMatrix; }
    inline glm::mat4& GetVPM() const { return m_ViewProjectionMatrix; }
    inline glm::vec3& GetPos() const { return m_CameraPos; }
    inline float& GetRotation() const { return m_Rotation; }
    inline glm::mat4& CalcMVP(const glm::vec3& translation) const
    {
        glm::mat4 model = glm::translate(glm::mat4(1.0f), translation);
        static glm::mat4 mvp = m_ProjectionMatrix * m_ViewMatrix * model;
        return mvp;
    }

    void CalculateViewMatrix();
};

class Renderer
{
private:
    std::vector<std::shared_ptr<VertexArray>> vao;
    std::vector<std::shared_ptr<Shader>> shaders;    
public:
    SDL_Window* window;
    SDL_GLContext context;
public:
    void AllocBuffers(const std::vector<Vertex>& _vertices);
    void Draw(const Shader& shader, const VertexArray& va) const;

    static void LoadShader(const std::string& filepath);
};

extern std::vector<model_t> modelinfo;
extern std::unique_ptr<Renderer> renderer;

void I_CacheModels();

#endif