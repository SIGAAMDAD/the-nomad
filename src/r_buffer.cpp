#include "n_shared.h"
#include "g_game.h"

VertexBuffer::VertexBuffer(const void *data, size_t count)
{
    glGenBuffersARB(1, &id);
    glBindBufferARB(GL_ARRAY_BUFFER_ARB, id);
    glBufferDataARB(GL_ARRAY_BUFFER_ARB, count, data, GL_STATIC_DRAW_ARB);
    glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
}

VertexBuffer::VertexBuffer(size_t reserve)
{
    glGenBuffersARB(1, &id);
    glBindBufferARB(GL_ARRAY_BUFFER_ARB, id);
    glBufferDataARB(GL_ARRAY_BUFFER_ARB, reserve, NULL, GL_DYNAMIC_DRAW_ARB);
    glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
}

VertexBuffer::~VertexBuffer()
{
    glDeleteBuffers(1, &id);
}

void VertexBuffer::SetData(const void *data, size_t count)
{
    glBindBufferARB(GL_ARRAY_BUFFER_ARB, id);
    glBufferSubDataARB(GL_ARRAY_BUFFER_ARB, 0, count, data);
    glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
}

void VertexBuffer::PushVertexAttrib(GLint index, GLsizei count, GLenum type, GLboolean normalized, GLsizei stride, const void *offset)
{
    glBindBufferARB(GL_ARRAY_BUFFER_ARB, id);
    glEnableVertexArrayAttrib(id, index);
    glVertexAttribPointerARB(index, count, type, normalized, stride, offset);
    glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
}

VertexBuffer* VertexBuffer::Create(const void* data, size_t size, const eastl::string& name)
{
    VertexBuffer* ptr = (VertexBuffer *)Z_Malloc(sizeof(VertexBuffer), TAG_STATIC, &ptr, name.c_str());
    new (ptr) VertexBuffer(data, size);
    return ptr;
}
VertexBuffer* VertexBuffer::Create(size_t reserve, const eastl::string& name)
{
    VertexBuffer* ptr = (VertexBuffer *)Z_Malloc(sizeof(VertexBuffer), TAG_STATIC, &ptr, name.c_str());
    new (ptr) VertexBuffer(reserve);
    return ptr;
}

/*
* Index Buffer Objects
*/

IndexBuffer* IndexBuffer::Create(const void *indices, size_t count, GLenum type, const eastl::string& name)
{
    IndexBuffer* ptr = (IndexBuffer *)Z_Malloc(sizeof(IndexBuffer), TAG_STATIC, &ptr, name.c_str());
    new (ptr) IndexBuffer(indices, count, type);
    return ptr;
}

IndexBuffer::IndexBuffer(const void* data, size_t count, GLenum _type)
    : NumIndices(count), type(_type)
{
    switch (_type) {
    case GL_UNSIGNED_BYTE:
        count *= sizeof(GLubyte);
        break;
    case GL_UNSIGNED_SHORT:
        count *= sizeof(GLushort);
        break;
    case GL_UNSIGNED_INT:
        count *= sizeof(GLuint);
        break;
    case GL_UNSIGNED_INT64_ARB:
        count *= sizeof(GLuint64);
        break;
    };

    glGenBuffersARB(1, &id);
    glBindBufferARB(GL_ARRAY_BUFFER_ARB, id);
    glBufferDataARB(GL_ARRAY_BUFFER_ARB, count, data, GL_STATIC_DRAW_ARB);
    glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
}

IndexBuffer::~IndexBuffer()
{
    glDeleteBuffers(1, &id);
}


/*
* Shader Storage Buffer Objects
*/

ShaderStorageBuffer* ShaderStorageBuffer::Create(const void *data, size_t count, GLuint binding, const std::string& name)
{
    ShaderStorageBuffer* ptr = (ShaderStorageBuffer *)Z_Malloc(sizeof(ShaderStorageBuffer), TAG_STATIC, &ptr, name.c_str());
    new (ptr) ShaderStorageBuffer(data, count, binding);
    return ptr;
}
ShaderStorageBuffer* ShaderStorageBuffer::Create(size_t reserve, GLuint binding, const std::string& name)
{
    ShaderStorageBuffer* ptr = (ShaderStorageBuffer *)Z_Malloc(sizeof(ShaderStorageBuffer), TAG_STATIC, &ptr, name.c_str());
    new (ptr) ShaderStorageBuffer(reserve, binding);
    return ptr;
}

ShaderStorageBuffer::ShaderStorageBuffer(const void *data, size_t count, GLuint binding)
{
    glGenBuffersARB(1, &id);
    glBindBufferARB(GL_SHADER_STORAGE_BUFFER, id);
    glBufferDataARB(GL_SHADER_STORAGE_BUFFER, count, data, GL_STATIC_DRAW_ARB);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, binding, id);
    glBindBufferARB(GL_SHADER_STORAGE_BUFFER, 0);
}

ShaderStorageBuffer::ShaderStorageBuffer(size_t reserve, GLuint binding)
{
    glGenBuffersARB(1, &id);
    glBindBufferARB(GL_SHADER_STORAGE_BUFFER, id);
    glBufferDataARB(GL_SHADER_STORAGE_BUFFER, reserve, NULL, GL_DYNAMIC_DRAW_ARB);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, binding, id);
    glBindBufferARB(GL_SHADER_STORAGE_BUFFER, 0);
}

ShaderStorageBuffer::~ShaderStorageBuffer()
{
    glDeleteBuffersARB(1, &id);
}

void ShaderStorageBuffer::SetBufferBinding(GLuint binding)
{
    Bind();
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, binding, id);
    Unbind();
}

void ShaderStorageBuffer::SetData(const void *data, size_t count)
{
    Bind();
    glBufferSubDataARB(GL_SHADER_STORAGE_BUFFER, 0, count, data);
    Unbind();
}