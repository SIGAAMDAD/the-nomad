#include "n_shared.h"
#include "g_game.h"

VertexArray::VertexArray()
{
    glGenVertexArrays(1, &id);
}

VertexArray::~VertexArray()
{
    glDeleteVertexArrays(1, &id);
}

Framebuffer::Framebuffer()
{
    glGenFramebuffers(1, &fboMsaaId);
    glBindFramebuffer(GL_FRAMEBUFFER, fboMsaaId);
    glGenTextures(1, &texMsaaId);
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, texMsaaId);
    glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGB, scf::renderer::width, scf::renderer::height, GL_TRUE);
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, texMsaaId, 0);

    glGenRenderbuffers(1, &rboId);
    glBindRenderbuffer(GL_RENDERBUFFER, rboId);
    glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_DEPTH24_STENCIL8, scf::renderer::width, scf::renderer::height);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rboId);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        N_Error("Framebuffer::Framebuffer: (for fboMsaaId) glCheckFramebufferStatus != GL_FRAMEBUFFER_COMPLETE");
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glGenFramebuffers(1, &fboId);
    glBindFramebuffer(GL_FRAMEBUFFER, fboId);
    glGenTextures(1, &texId);
    glBindTexture(GL_TEXTURE_2D, texId);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, scf::renderer::width, scf::renderer::height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glGenerateMipmap(GL_TEXTURE_2D);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texId, 0);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        N_Error("Framebuffer::Framebuffer: (for fboId) glCheckFramebufferStatus != GL_FRAMEBUFFER_COMPLETE");
    
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Framebuffer::AddTexture(const Texture2D& texture)
{
}

Framebuffer::~Framebuffer()
{
    glDeleteFramebuffers(1, &fboId);
    glDeleteFramebuffers(1, &fboMsaaId);
    glDeleteRenderbuffers(1, &rboId);
    glDeleteTextures(1, &texId);
    glDeleteTextures(1, &texMsaaId);
}

void Framebuffer::Bind() const
{
    glBindFramebuffer(GL_FRAMEBUFFER, fboId);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, fboMsaaId);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fboId);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Framebuffer::Unbind() const
{
    glBlitFramebuffer(0, 0, scf::renderer::width, scf::renderer::height, 0, 0, scf::renderer::width, scf::renderer::height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
}

GPUContext::GPUContext()
{
    glGetIntegerv(GL_GPU_MEMORY_INFO_DEDICATED_VIDMEM_NVX, &gpu_memory_total);
    glGetIntegerv(GL_GPU_MEMORY_INFO_CURRENT_AVAILABLE_VIDMEM_NVX, &gpu_memory_available);
}

GPUContext::~GPUContext()
{
}