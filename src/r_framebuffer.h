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
    typedef void(*glfn_t)(GLenum target, GLuint id);

    const FramebufferSetup setup;
    GLuint fboId;
    GLuint depthBufferId;
    GLuint colorBufferId;

    VertexArray* vao;
    VertexBuffer* vbo;
    Shader* shader;

    void DeleteColorBuffer(void)
    {
        if (setup.colorBufferTarget == GL_NO_COLORATTACHMENT)
            return;
        if (setup.colorBufferTarget == GL_TEXTURE_2D)
            glDeleteTextures(1, &colorBufferId);
        else if (setup.colorBufferTarget == GL_RENDERBUFFER)
            glDeleteRenderbuffers(1, &colorBufferId);
    }
    void DeleteDepthBuffer(void)
    {
        if (setup.depthBufferTarget == GL_NO_DEPTHATTACHMENT)
            return;
        if (setup.depthBufferTarget == GL_TEXTURE_2D)
            glDeleteTextures(1, &depthBufferId);
        else if (setup.depthBufferTarget == GL_RENDERBUFFER)
            glDeleteRenderbuffers(1, &depthBufferId);
    }
    void BindColorBuffer(void)
    {
        if (setup.colorBufferTarget == GL_NO_COLORATTACHMENT)
            return;
        if (setup.colorBufferTarget == GL_TEXTURE_2D) {
            if (setup.colorBufferMultisample)
                glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, colorBufferId);
            else
                glBindTexture(GL_TEXTURE_2D, colorBufferId);
        }
        else if (setup.colorBufferTarget == GL_RENDERBUFFER)
            glBindRenderbuffer(GL_RENDERBUFFER, colorBufferId);
    }
    void BindDepthBuffer(void)
    {
        if (setup.depthBufferTarget == GL_NO_DEPTHATTACHMENT)
            return;
        if (setup.depthBufferTarget == GL_TEXTURE_2D) {
            if (setup.depthBufferMultisample)
                glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, depthBufferId);
            else
                glBindTexture(GL_TEXTURE_2D, depthBufferId);
        }
        else if (setup.depthBufferTarget == GL_RENDERBUFFER)
            glBindRenderbuffer(GL_RENDERBUFFER, depthBufferId);
    }
    void UnbindDepthBuffer(void)
    {
        if (setup.depthBufferTarget == GL_NO_DEPTHATTACHMENT)
            return;
        if (setup.depthBufferTarget == GL_TEXTURE_2D) {
            if (setup.depthBufferMultisample)
                glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
            else
                glBindTexture(GL_TEXTURE_2D, 0);
        }
        else if (setup.depthBufferTarget == GL_RENDERBUFFER)
            glBindRenderbuffer(GL_RENDERBUFFER, 0);
    }
    void UnbindColorBuffer(void)
    {
        if (setup.colorBufferTarget == GL_NO_COLORATTACHMENT)
            return;
        if (setup.colorBufferTarget == GL_TEXTURE_2D) {
            if (setup.colorBufferMultisample)
                glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
            else
                glBindTexture(GL_TEXTURE_2D, 0);
        }
        else if (setup.colorBufferTarget == GL_RENDERBUFFER)
            glBindRenderbuffer(GL_RENDERBUFFER, 0);
    }
    void GenColorBuffer(void)
    {
        if (setup.colorBufferTarget == GL_NO_COLORATTACHMENT)
            return;
        if (setup.colorBufferTarget == GL_TEXTURE_2D)
            glGenTextures(1, &colorBufferId);
        else if (setup.colorBufferTarget == GL_RENDERBUFFER)
            glGenRenderbuffers(1, &colorBufferId);
    }
    void GenDepthBuffer(void)
    {
        if (setup.depthBufferTarget == GL_NO_DEPTHATTACHMENT)
            return;
        if (setup.depthBufferTarget == GL_TEXTURE_2D)
            glGenTextures(1, &depthBufferId);
        else if (setup.depthBufferTarget == GL_RENDERBUFFER)
            glGenRenderbuffers(1, &depthBufferId);
    }
    void InitColorBuffer(void);
    void InitDepthBuffer(void);
public:
    Framebuffer(const FramebufferSetup& _setup);
    ~Framebuffer();

    void SetBuffer();
    void SetDefault(void);
    // fbo is dest buffer
    void Blit(const Framebuffer* const fbo);
    void Draw(VertexArray* const vao, IndexBuffer* const ibo, Shader* const shader);

    inline GLuint GetID(void) const { return fboId; }
    inline const FramebufferSetup& GetSetup(void) const { return setup; }

    static Framebuffer* Create(const FramebufferSetup& setup, const eastl::string& filepath);
};

#endif