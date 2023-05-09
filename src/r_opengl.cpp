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
    glGenFramebuffers(1, &fboId);
    glGenRenderbuffers(1, &rboDepthId);
    glGenTextures(1, &texColorId);

    glBindFramebuffer(GL_FRAMEBUFFER, fboId);

    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, texColorId);
    glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGB, scf::renderer::width, scf::renderer::height, GL_TRUE);
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);

    glBindRenderbuffer(GL_RENDERBUFFER, rboDepthId);
    glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_DEPTH24_STENCIL8, scf::renderer::width, scf::renderer::height);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);

    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rboDepthId);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, texColorId, 0);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        N_Error("Framebuffer::Framebuffer: glCheckFramebufferStatus != GL_FRAMEBUFFER_COMPLETE");

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

Framebuffer::~Framebuffer()
{
    glDeleteFramebuffers(1, &fboId);
    glDeleteRenderbuffers(1, &rboDepthId);
    glDeleteTextures(1, &texColorId);
//    Z_Free(vbo);
//    Z_Free(vao);
//    Z_Free(shader);
}

void Framebuffer::SetBuffer()
{
    glClear(GL_COLOR_BUFFER_BIT);
    glBindFramebuffer(GL_FRAMEBUFFER, fboId);
    glViewport(0, 0, scf::renderer::width, scf::renderer::height);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Framebuffer::SetDefault()
{
    glBindFramebuffer(GL_READ_FRAMEBUFFER, fboId);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

    glBlitFramebuffer(0, 0, scf::renderer::width, scf::renderer::height,
                      0, 0, scf::renderer::width, scf::renderer::height,
                      GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT,
                      GL_NEAREST);
    
    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
}

GPUContext::GPUContext()
{
    glGetIntegerv(GL_GPU_MEMORY_INFO_DEDICATED_VIDMEM_NVX, &gpu_memory_total);
    glGetIntegerv(GL_GPU_MEMORY_INFO_CURRENT_AVAILABLE_VIDMEM_NVX, &gpu_memory_available);
}

GPUContext::~GPUContext()
{
}