#include "n_shared.h"
#include "g_game.h"

Framebuffer* Framebuffer::Create(const eastl::string& name)
{
    return CONSTRUCT(Framebuffer, name.c_str());
}

Framebuffer::Framebuffer()
{
    glGenFramebuffers(1, &fboId);
    glGenFramebuffers(1, &fboMsaaId);
    glGenRenderbuffers(1, &rboColorId);
    glGenRenderbuffers(1, &rboDepthId);
    glGenTextures(1, &texColorId);

    glBindFramebuffer(GL_FRAMEBUFFER, fboMsaaId);

    glBindRenderbuffer(GL_RENDERBUFFER, rboColorId);
    glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_RGBA8, scf::renderer::width, scf::renderer::height);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);

    glBindRenderbuffer(GL_RENDERBUFFER, rboDepthId);
    glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_DEPTH24_STENCIL8, scf::renderer::width, scf::renderer::height);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);

    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, rboColorId);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rboDepthId);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        N_Error("Framebuffer::Framebuffer: (fboMsaaId) glCheckFramebufferStatus != GL_FRAMEBUFFER_COMPLETE");

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glBindFramebuffer(GL_FRAMEBUFFER, fboId);

    glBindTexture(GL_TEXTURE_2D, texColorId);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, scf::renderer::width, scf::renderer::height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_2D, 0);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texColorId, 0);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        N_Error("Framebuffer::Framebuffer: (fboId) glCheckFramebufferStatus != GL_FRAMEBUFFER_COMPLETE");

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    Vertex vertices[] = {
        Vertex(glm::vec3( 1.0f,  1.0f, 0.0f), glm::vec2(0.0f, 0.0f)), // top right
        Vertex(glm::vec3( 1.0f, -1.0f, 0.0f), glm::vec2(0.0f, 1.0f)), // bottom right
        Vertex(glm::vec3(-1.0f, -1.0f, 0.0f), glm::vec2(1.0f, 1.0f)), // bottom left
        Vertex(glm::vec3(-1.0f,  1.0f, 0.0f), glm::vec2(1.0f, 0.0f)), // top left
    };
    uint8_t indices[] = {
        0, 1, 2,
        0, 2, 3
    };

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glGenBuffersARB(1, &vbo);
    glGenBuffersARB(1, &ibo);

    glBindBufferARB(GL_ARRAY_BUFFER_ARB, vbo);

    glBufferDataARB(GL_ARRAY_BUFFER_ARB, sizeof(vertices), vertices, GL_STATIC_DRAW_ARB);
    glEnableVertexAttribArrayARB(0);
    glVertexAttribPointerARB(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void *)offsetof(Vertex, pos));
    glEnableVertexAttribArrayARB(1);
    glVertexAttribPointerARB(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void *)offsetof(Vertex, texcoords));

    glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
    
    glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, ibo);
    glBufferDataARB(GL_ELEMENT_ARRAY_BUFFER_ARB, sizeof(indices), indices, GL_STATIC_DRAW_ARB);
    glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, 0);

    shader = Shader::Create("framebuffer.glsl", "fbo_Shader");
}

Framebuffer::~Framebuffer()
{
    glDeleteFramebuffers(1, &fboId);
    glDeleteFramebuffers(1, &fboMsaaId);
    glDeleteRenderbuffers(1, &rboColorId);
    glDeleteRenderbuffers(1, &rboDepthId);
    glDeleteTextures(1, &texColorId);
}

void Framebuffer::SetBuffer(void)
{
    glBindFramebuffer(GL_FRAMEBUFFER, fboMsaaId);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glViewport(0, 0, scf::renderer::width, scf::renderer::height);
}

void Framebuffer::SetDefault(void)
{
    glBindFramebuffer(GL_READ_FRAMEBUFFER, fboMsaaId);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fboId);

    glBlitFramebuffer(0, 0, scf::renderer::width, scf::renderer::height,
                      0, 0, scf::renderer::width, scf::renderer::height,
                      GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT,
                      GL_NEAREST);
    
    glBindFramebuffer(GL_READ_FRAMEBUFFER, fboId);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

    glBlitFramebuffer(0, 0, scf::renderer::width, scf::renderer::height,
                      0, 0, scf::renderer::width, scf::renderer::height,
                      GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT,
                      GL_NEAREST);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    
    shader->Bind();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texColorId);
    shader->SetInt("u_ColorTexture", 0);
    screenTexture->Bind(1);
    shader->SetInt("u_ScreenTexture", 1);

    glBindVertexArray(vao);
    glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, ibo);

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, NULL);

    glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, 0);
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
    shader->Unbind();
}

void Framebuffer::SetScreenTexture(const Texture2D* texture)
{
    screenTexture = texture;
}