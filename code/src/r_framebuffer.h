#ifndef _R_FRAMEBUFFER_
#define _R_FRAMEBUFFER_

#pragma once

class Texture2D;

#define GL_NO_DEPTHATTACHMENT 0
#define GL_NO_COLORATTACHMENT 0

struct FramebufferAttachment
{
    GLenum target;
    GLenum attachmentType;
    uint32_t width;
    uint32_t height;
    bool multisample;
    GLenum internalFormat;

    constexpr FramebufferAttachment(GLenum _target, GLenum _attachmentType, uint32_t _width, uint32_t _height, bool _multisample, GLenum _internalFormat)
        : target(_target), attachmentType(_attachmentType), width(_width), height(_height), multisample(_multisample), internalFormat(_internalFormat)
    {
    }
    ~FramebufferAttachment() = default;
};

struct FramebufferSetup
{
    uint32_t width;
    uint32_t height;
    GLenum blitBit;
    GLenum clearBit;
    uint32_t numAttachments;
    FramebufferAttachment* attachments;

    FramebufferSetup(uint32_t _width, uint32_t _height, GLenum _blitBit, GLenum _clearBit, std::initializer_list<FramebufferAttachment> _attachments)
        : width(_width), height(_height), blitBit(_blitBit), clearBit(_clearBit)
    {
        numAttachments = _attachments.size();
        attachments = (FramebufferAttachment *)xmalloc(sizeof(FramebufferAttachment) * numAttachments);
        size_t index = 0;
        for (std::initializer_list<FramebufferAttachment>::const_iterator it = _attachments.begin(); it != _attachments.end(); it++) {
            memmove(&attachments[index], it, sizeof(FramebufferAttachment));
            index++;
        }
    }
    ~FramebufferSetup() = default;
};

const inline FramebufferSetup DEFAULT_FRAMEBUFFER_SETUP = FramebufferSetup(
    scf::renderer::width, scf::renderer::height, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT,
    {
        FramebufferAttachment(GL_RENDERBUFFER, GL_COLOR_ATTACHMENT0, scf::renderer::width, scf::renderer::height, true, GL_RGBA8),
        FramebufferAttachment(GL_RENDERBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, scf::renderer::width, scf::renderer::height, true, GL_DEPTH24_STENCIL8),
    }
);

class VertexArray;
class VertexBuffer;
class Shader;

class VK_Framebuffer
{
private:
    VkFramebuffer fboId;
    VkFramebufferAttachmentsCreateInfoKHR attachmentsCreateInfo;
    VkFramebufferAttachmentImageInfoKHR imageInfo;
};
class Framebuffer
{
private:
    GLuint fboId, fboMsaaId;
    GLuint rboColorId, rboDepthId;
    GLuint texColorId;

    Shader* shader;
    GLuint vao, vbo, ibo;    
    const Texture2D* screenTexture;
public:
    Framebuffer();
    ~Framebuffer();

    void SetBuffer(void);
    void SetDefault(void);
    void SetScreenTexture(const Texture2D* texture);

    inline GLuint GetVAO(void) const { return vao; }
    inline GLuint GetVBO(void) const { return vbo; }
    inline GLuint GetIBO(void) const { return ibo; }
    inline const Shader* GetShader(void) const { return shader; }
    inline GLuint GetID(void) const { return fboId; }

    static Framebuffer* Create(const eastl::string& filepath);
};

#endif