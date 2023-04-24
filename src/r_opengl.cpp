#include "n_shared.h"
#include "g_game.h"

VertexBuffer::VertexBuffer(const size_t reserve, const std::shared_ptr<VertexArray>& vao)
    : vertices(reserve)
{
    glCall(glGenBuffers(1, &id));
    Bind();

    glCall(glBufferData(GL_ARRAY_BUFFER, reserve * sizeof(Vertex), NULL, GL_DYNAMIC_DRAW));

    glCall(glEnableVertexAttribArray(0));
    glCall(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void *)offsetof(Vertex, pos)));

    glCall(glEnableVertexAttribArray(1));
    glCall(glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void *)offsetof(Vertex, color)));
}

VertexBuffer::VertexBuffer(const std::vector<Vertex>& _vertices, const VertexArray* vao)
    : vertices(_vertices)
{
    glCall(glGenBuffers(1, &id));
    Bind();

    glCall(glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * vertices.size(), vertices.data(), GL_DYNAMIC_DRAW));

    glCall(glEnableVertexAttribArray(0));
    glCall(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void *)offsetof(Vertex, pos)));

    glCall(glEnableVertexAttribArray(1));
    glCall(glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void *)offsetof(Vertex, color)));
}

VertexBuffer::VertexBuffer(const Vertex* _vertices, const size_t count, const VertexArray* vao)
    : vertices(count)
{
    glCall(glGenBuffers(1, &id));
    Bind();

    memmove(vertices.data(), _vertices, count * sizeof(Vertex));
    glCall(glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * count, _vertices, GL_DYNAMIC_DRAW));

    glCall(glEnableVertexAttribArray(0));
    glCall(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void *)offsetof(Vertex, pos)));

    glCall(glEnableVertexAttribArray(1));
    glCall(glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void *)offsetof(Vertex, color)));
}

VertexBuffer::VertexBuffer(const Vertex* _vertices, const size_t count, const std::shared_ptr<VertexArray>& vao)
    : vertices(count)
{
    glCall(glGenBuffers(1, &id));
    Bind();

    memmove(vertices.data(), _vertices, sizeof(Vertex) * count);
    glCall(glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * vertices.size(), _vertices, GL_DYNAMIC_DRAW));
    
    glCall(glEnableVertexAttribArray(0));
    glCall(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void *)offsetof(Vertex, pos)));

    glCall(glEnableVertexAttribArray(1));
    glCall(glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void *)offsetof(Vertex, color)));
}

VertexBuffer::VertexBuffer(const std::vector<Vertex>& _vertices)
    : vertices(_vertices)
{
    glCall(glGenBuffers(1, &id));
    Bind();

    glCall(glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * vertices.size(), vertices.data(), GL_DYNAMIC_DRAW));

    glCall(glEnableVertexArrayAttrib(id, 0));
    glCall(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void *)offsetof(Vertex, pos)));

    glCall(glEnableVertexArrayAttrib(id, 1));
    glCall(glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void *)offsetof(Vertex, color)));
}

VertexBuffer::VertexBuffer(const Vertex* _vertices, const size_t count)
    : vertices(count)
{
    glCall(glGenBuffers(1, &id));
    Bind();
    
    memmove(vertices.data(), _vertices, sizeof(Vertex) * count);
    glCall(glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * vertices.size(), vertices.data(), GL_DYNAMIC_DRAW));

    glCall(glEnableVertexArrayAttrib(id, 0));
    glCall(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void *)offsetof(Vertex, pos)));

    glCall(glEnableVertexArrayAttrib(id, 1));
    glCall(glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void *)offsetof(Vertex, color)));
}

VertexBuffer::~VertexBuffer()
{
    Unbind();
    glCall(glDeleteBuffers(1, &id));
}

void VertexBuffer::SwapBuffer(const Vertex* _vertices, const size_t offset, const size_t count)
{
    Bind();
    glCall(glBufferSubData(GL_ARRAY_BUFFER, offset, count * sizeof(Vertex), _vertices));
}

void VertexBuffer::SwapBuffer(const std::vector<Vertex>& _vertices, bool newbuffer)
{
    if (_vertices.size() != vertices.size())
        vertices.resize(_vertices.size());
    
    memmove(vertices.data(), _vertices.data(), sizeof(Vertex) * _vertices.size());
    Bind();
    if (newbuffer) {
        glCall(glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_DYNAMIC_DRAW));
    }
    else {
        glCall(glBufferSubData(GL_ARRAY_BUFFER, 0, vertices.size() * sizeof(Vertex), vertices.data()));
    }
}

void VertexBuffer::SwapBuffer(const Vertex* _vertices, const size_t count, bool newbuffer)
{
    if (count != vertices.size())
        vertices.resize(count);
    
    memmove(vertices.data(), _vertices, sizeof(Vertex) * count);
    Bind();
    if (newbuffer) {
        glCall(glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_DYNAMIC_DRAW));
    }
    else {
        glCall(glBufferSubData(GL_ARRAY_BUFFER, 0, vertices.size() * sizeof(Vertex), vertices.data()));
    }
}

Shader::Shader(const std::string& filepath)
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

Shader::~Shader()
{
    Unbind();
    glCall(glDeleteProgram(id));
}

IndexBuffer::IndexBuffer(const uint32_t* _indices, const size_t count)
    : indices_count(count)
{
    glCall(glGenBuffers(1, &id));
    Bind();

    glCall(glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(uint32_t), NULL, GL_DYNAMIC_DRAW));
}

IndexBuffer::IndexBuffer(const std::vector<uint32_t>& _indices)
    : indices_count(_indices.size())
{
    glCall(glGenBuffers(1, &id));
    Bind();

    glCall(glBufferData(GL_ELEMENT_ARRAY_BUFFER, _indices.size() * sizeof(uint32_t), _indices.data(), GL_STATIC_DRAW));
}

IndexBuffer::IndexBuffer(const size_t count)
    : indices_count(count)
{
    glCall(glGenBuffers(1, &id));
    Bind();

    glCall(glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(uint32_t), NULL, GL_DYNAMIC_DRAW));
}

IndexBuffer::~IndexBuffer()
{
    Unbind();
    glCall(glDeleteBuffers(1, &id));
}

void IndexBuffer::SwapBuffer(const uint32_t* _indices, const size_t count, bool newbuffer)
{
    indices_count = count;
    Bind();
    if (newbuffer) {
        glCall(glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(uint32_t), _indices, GL_DYNAMIC_DRAW));
    }
    else {
        glCall(glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, count * sizeof(uint32_t), _indices));
    }
}

void IndexBuffer::SwapBuffer(const std::vector<uint32_t>& _indices, bool newbuffer)
{
    indices_count = _indices.size();
    Bind();
    if (newbuffer) {
        glCall(glBufferData(GL_ELEMENT_ARRAY_BUFFER, _indices.size() * sizeof(uint32_t), _indices.data(), GL_STATIC_DRAW));
    }
    else {
        glCall(glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, _indices.size() * sizeof(uint32_t), _indices.data()));
    }
}

VertexArray::VertexArray()
{
    glCall(glGenVertexArrays(1, &id));
}
VertexArray::VertexArray(const std::vector<Vertex>& _vertices)
{
    glCall(glGenVertexArrays(1, &id));
    Bind();

    vbo.emplace_back(std::make_shared<VertexBuffer>(_vertices, this));
}
VertexArray::VertexArray(const Vertex* _vertices, const size_t count)
{
    glCall(glGenVertexArrays(1, &id));
    Bind();

    vbo.emplace_back(std::make_shared<VertexBuffer>(_vertices, count, this));
}

VertexArray::VertexArray(const std::vector<Vertex>& _vertices, const std::vector<uint32_t>& _indices)
{
    glCall(glGenVertexArrays(1, &id));
    Bind();

    vbo.emplace_back(std::make_shared<VertexBuffer>(_vertices, this));
    ibo = std::make_shared<IndexBuffer>(_indices);
}
VertexArray::VertexArray(const Vertex* _vertices, const size_t vertices_count, const uint32_t* _indices, const size_t indices_count)
{
    glCall(glGenVertexArrays(1, &id));
    Bind();

    vbo.emplace_back(std::make_shared<VertexBuffer>(_vertices, vertices_count, this));
    ibo = std::make_shared<IndexBuffer>(_indices, indices_count);
}

VertexArray::~VertexArray()
{
    Unbind();
    glCall(glDeleteVertexArrays(1, &id));
}

void VertexArray::DrawVBO(GLenum mode, GLint first, GLsizei count) const
{
    for (const auto& i : vbo) {
        i->Bind();
        i->Draw(mode, first, count);
        i->Unbind();
    }
}
void VertexArray::DrawIBO(GLenum mode) const
{
    ibo->Bind();
    ibo->Draw(mode, ibo->numindices());
    ibo->Unbind();
}

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>
#include <stb/stb_image_resize.h>

Texture::Texture(const std::string& filepath)
{
    glCall(glGenTextures(1, &id));
    Bind();

//    buffer = (uint8_t *)stbi_load();

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, buffer);
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapS);
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapT); // GL_CLAMP_TO_EDGE
//	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filterMin);
//    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filterMax);
	glBindTexture(GL_TEXTURE_2D, 0);
}