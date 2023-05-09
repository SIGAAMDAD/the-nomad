#ifndef _R_BUFFER_
#define _R_BUFFER_

class UniformBuffer
{
private:
    GLuint id;
    size_t size;
public:
    UniformBuffer(const void *data, size_t count);
    UniformBuffer(size_t reserve);
    UniformBuffer(const UniformBuffer& other);
    UniformBuffer(UniformBuffer &&) = delete;
    ~UniformBuffer();

    UniformBuffer& operator=(const UniformBuffer& other);

    inline void Bind() const
    { glBindBufferARB(GL_UNIFORM_BUFFER, id); }
    inline void Unbind() const
    { glBindBufferARB(GL_UNIFORM_BUFFER, 0); }

    void SetData(const void *data, size_t size);
    
    static UniformBuffer* Create(const void *data, size_t count, const eastl::string& name);
    static UniformBuffer* Create(size_t reserve, const eastl::string& name);
};

class VertexBuffer
{
public:
    GLuint id;
    size_t NumVertices;
public:
    VertexBuffer(size_t reserve);
    VertexBuffer(const void* data, size_t size);
    VertexBuffer(const VertexBuffer &) = delete;
    VertexBuffer(VertexBuffer &&) = delete;
    ~VertexBuffer();

    inline void Bind() const
    { glBindBufferARB(GL_ARRAY_BUFFER_ARB, id); }
    inline void Unbind() const
    { glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0); }

    void PushVertexAttrib(GLint index, GLsizei count, GLenum type, GLboolean normalized, GLsizei stride, const void *offset);
    void SetData(const void *data, size_t size);
    inline size_t GetCount() const { return NumVertices; }

    static VertexBuffer* Create(const void* data, size_t size, const eastl::string& name);
    static VertexBuffer* Create(size_t reserve, const eastl::string& name);
};

class IndexBuffer
{
private:
    GLuint id;
    size_t NumIndices;
    GLenum type;
public:
    IndexBuffer(const void* indices, size_t count, GLenum _type);
    ~IndexBuffer();

    inline void Bind() const
    { glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, id); }
    inline void Unbind() const
    { glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, 0); }
    inline size_t GetCount() const { return NumIndices; }

    static IndexBuffer* Create(const void *indices, size_t count, GLenum type, const eastl::string& name);
};

class ShaderStorageBuffer
{
private:
    GLuint id;
public:
    ShaderStorageBuffer(const void *data, size_t count, GLuint binding);
    ShaderStorageBuffer(size_t reserve, GLuint binding);
    ~ShaderStorageBuffer();

    inline void Bind() const
    { glBindBufferARB(GL_SHADER_STORAGE_BUFFER, id); }
    inline void Unbind() const
    { glBindBufferARB(GL_SHADER_STORAGE_BUFFER, 0); }

    void SetBufferBinding(GLuint binding);
    void SetData(const void *data, size_t count);

    static ShaderStorageBuffer* Create(const void *data, size_t count, GLuint binding, const std::string& name);
    static ShaderStorageBuffer* Create(size_t reserve, GLuint binding, const std::string& name);
};

typedef ShaderStorageBuffer SSBO;
typedef UniformBuffer UBO;
typedef VertexBuffer VBO;
typedef IndexBuffer IBO;

#endif