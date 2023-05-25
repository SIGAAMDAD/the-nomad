#include "n_shared.h"
#include "g_zone.h"
#include "m_renderer.h"

VertexArray::VertexArray()
{
    glGenVertexArrays(1, &id);
}

VertexArray::~VertexArray()
{
    glDeleteVertexArrays(1, &id);
}

void VertexArray::Bind(void) const
{
    glBindVertexArray(id);
}

void VertexArray::Unbind(void) const
{
    glBindVertexArray(0);
}

void VertexArray::SetIndexBuffer(const std::shared_ptr<IndexBuffer>& _ibo)
{
    indexBuffer = _ibo;
}

VertexArray* VertexArray::Create(const eastl::string& name)
{
    return CONSTRUCT(VertexArray, name.c_str());
}