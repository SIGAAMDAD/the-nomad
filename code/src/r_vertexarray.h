#ifndef _R_VERTEXARRAY_
#define _R_VERTEXARRAY_

#pragma once

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

    void Bind(void) const;
    void Unbind(void) const;

    void SetIndexBuffer(const std::shared_ptr<IndexBuffer>& _indexBuffer);
    const nomadvector<std::shared_ptr<VertexBuffer>> GetVertexBuffers() const { return vertexBuffers; }
    const std::shared_ptr<IndexBuffer> GetIndexBuffer() const { return indexBuffer; }

    static VertexArray* Create(const eastl::string& name);
};


#endif