#ifndef _R_BUFFER_
#define _R_BUFFER_

class VertexArray;

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

class VK_VertexBuffer
{
public:
    VkBufferCreateInfo bufferInfo{};
    VkBuffer buffer{};
    VkMemoryRequirements memReqs{};
public:
    VK_VertexBuffer(const void *data, size_t count);
    ~VK_VertexBuffer();
};

// for the future
#if 0

struct VK_VertexBuffer
{
    VkBuffer buffer;
    VkMemoryRequirements memReqs;
    VkBufferCreateInfo bufferInfo;
    VkMemoryAllocateInfo allocInfo;
    VkDeviceMemory bufferMemory;

    void Bind(void) const
    {
//        vkCmdBindVertexBuffers();
    }
    void Unbind(void) const;
};

struct GL_VertexBuffer
{
    GLuint id;
    void *data;
    size_t numvertices;

    void Bind(void) const
    {
        glBindBufferARB(GL_ARRAY_BUFFER_ARB, id);
    }
    void Unbind(void) const
    {
        glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
    }
};

class VertexBuffer
{
private:
    typedef union {
        GL_VertexBuffer* glPtr;
        VK_VertexBuffer* vkPtr;
    } obj;
public:
    VertexBuffer();
    ~VertexBuffer();

    inline void Bind(void) const;
    inline void Unbind(void) const;
};
#endif

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
    void PushVertexAttrib(const VertexArray* vao, GLint index, GLsizei count, GLenum type, GLboolean normalized, GLsizei stride, const void *offset);
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
    inline size_t TypeSize(void) const
    {
        switch (type) {
        case GL_UNSIGNED_BYTE: return 1;
        case GL_UNSIGNED_SHORT: return 2;
        case GL_UNSIGNED_INT: return 4;
        case GL_UNSIGNED_INT64_ARB: return 8;
        };
        assert(false);
        N_Error("IndexBuffer::TypeSize: invalid type for index buffer");
    }
public:
    IndexBuffer(const void* indices, size_t count, GLenum _type);
    ~IndexBuffer();

    inline void Bind() const
    { glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, id); }
    inline void Unbind() const
    { glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, 0); }
    inline size_t GetCount() const { return NumIndices; }
    inline size_t BufferSize() const { return NumIndices * TypeSize(); }

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