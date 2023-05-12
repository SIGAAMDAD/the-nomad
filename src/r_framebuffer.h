#ifndef _R_FRAMEBUFFER_
#define _R_FRAMEBUFFER_

#pragma once

#define GL_NO_DEPTHATTACHMENT 0
#define GL_NO_COLORATTACHMENT 0

struct FramebufferSetup
{
    uint32_t width;
    uint32_t height;
    GLenum blit; // GL_COLOR_BUFFER_BIT, GL_DEPTH_BUFFER_BIT, GL_STENCIL_BUFFER_BIT
    GLenum clear;
    GLenum depthBufferTarget;
    GLenum colorBufferTarget;
    bool depthBufferMultisample;
    bool colorBufferMultisample;

    FramebufferSetup(uint32_t _width, uint32_t _height, GLenum _blit, GLenum _clear, GLenum _depthBufferTarget, GLenum _colorBufferTarget,
        bool _db_ms, bool _cb_ms)
        : width(_width), height(_height), blit(_blit), clear(_clear), depthBufferTarget(_depthBufferTarget), colorBufferTarget(_colorBufferTarget),
        depthBufferMultisample(_db_ms), colorBufferMultisample(_cb_ms)
    {
    }
#if 0
    FramebufferSetup(const uint32_t& _weight, const uint32_t& _height, GLenum _blit, GLenum _clear, GLenum _depthBufferTarget, GLenum _colorBufferTarget,
        bool _db_ms, bool _cb_ms)
        : width(_width), height(_height), blit(_blit), clear(_clear), depthBufferTarget(_depthBufferTarget), colorBufferTarget(_colorBufferTarget),
        depthBufferMultisample(_db_ms), colorBufferMultisample(_cb_ms)
    {
    }
#endif
    ~FramebufferSetup(void) = default;
};

inline FramebufferSetup DEFAULT_FRAMEBUFFER_SETUP = FramebufferSetup(
    scf::renderer::width, scf::renderer::height, GL_COLOR_BUFFER_BIT, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT, GL_RENDERBUFFER, GL_RENDERBUFFER, true, true
);

class VertexArray;
class VertexBuffer;
class Shader;

class Framebuffer
{
private:
    GLuint fboId, fboMsaaId;
    GLuint rboDepthId;
    GLuint texColorId, texMsaaColorId;

    Shader* shader;
    VertexArray* vao;
    VertexBuffer* vbo;
    IndexBuffer* ibo;
public:
    Framebuffer();
    ~Framebuffer();

    void SetBuffer();
    void SetDefault(void);
    // fbo is dest buffer
    void Blit(const Framebuffer* const fbo);
    void Draw(VertexArray* const vao, IndexBuffer* const ibo, Shader* const shader);

    inline GLuint GetID(void) const { return fboId; }
//    inline const FramebufferSetup& GetSetup(void) const { return setup; }

    static Framebuffer* Create(const eastl::string& filepath);
};

#endif