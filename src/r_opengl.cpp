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

Framebuffer* Framebuffer::Create(const FramebufferSetup& setup, const eastl::string& name)
{
    Framebuffer* ptr = (Framebuffer *)Z_Malloc(sizeof(Framebuffer), TAG_STATIC, &ptr, name.c_str());
    new (ptr) Framebuffer(setup);
    return ptr;
}

void Framebuffer::InitDepthBuffer(void)
{
    if (setup.depthBufferTarget == GL_NO_DEPTHATTACHMENT)
        return;

    GenDepthBuffer();
    BindDepthBuffer();

    if (setup.colorBufferTarget == GL_TEXTURE_2D) {
        glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        if (setup.colorBufferMultisample)
            glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_DEPTH24_STENCIL8, setup.width, setup.height, GL_TRUE);
        else {
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, setup.width, setup.height, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_BYTE, NULL);
        }
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, (setup.depthBufferMultisample ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D),
            depthBufferId, 0);
    }
    else if (setup.colorBufferTarget == GL_RENDERBUFFER) {
        if (setup.colorBufferMultisample)
            glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_DEPTH24_STENCIL8, setup.width, setup.height);
        else
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, setup.width, setup.height);
        
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, depthBufferId);
    }

    UnbindDepthBuffer();
}

void Framebuffer::InitColorBuffer(void)
{
    if (setup.colorBufferTarget == GL_NO_COLORATTACHMENT)
        return;

    GenColorBuffer();
    BindColorBuffer();

    if (setup.colorBufferTarget == GL_TEXTURE_2D) {
        glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        if (setup.colorBufferMultisample)
            glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGB, setup.width, setup.height, GL_TRUE);
        else {
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, setup.width, setup.height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
        }
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, (setup.colorBufferMultisample ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D),
            colorBufferId, 0);
    }
    else if (setup.colorBufferTarget == GL_RENDERBUFFER) {
        if (setup.colorBufferMultisample)
            glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_RGB, setup.width, setup.height);
        else
            glRenderbufferStorage(GL_RENDERBUFFER, GL_RGB, setup.width, setup.height);
        
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, colorBufferId);
    }

    UnbindColorBuffer();
}

Framebuffer::Framebuffer(const FramebufferSetup& _setup)
    : setup(_setup)
{
    glGenFramebuffers(1, &fboId);

    glBindFramebuffer(GL_FRAMEBUFFER, fboId);
    
    InitColorBuffer();
    InitDepthBuffer();

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        N_Error("Framebuffer::Framebuffer: glCheckFramebufferStatus != GL_FRAMEBUFFER_COMPLETE");

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

Framebuffer::~Framebuffer()
{
    glDeleteFramebuffers(1, &fboId);
    UnbindColorBuffer();
    UnbindDepthBuffer();
}

void Framebuffer::SetBuffer(void)
{
    glBindFramebuffer(GL_FRAMEBUFFER, fboId);
    glViewport(0, 0, setup.width, setup.height);
    glClear(setup.clear);
}

void Framebuffer::SetDefault(void)
{
    glBindFramebuffer(GL_READ_FRAMEBUFFER, fboId);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

    glBlitFramebuffer(0, 0, setup.width, setup.height,
                      0, 0, scf::renderer::width, scf::renderer::height,
                      setup.blit,
                      GL_NEAREST);
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Framebuffer::Draw(VertexArray* const vao, IndexBuffer* const ibo, Shader* const shader)
{
    vao->Bind();
    ibo->Bind();
    shader->Bind();

    if (setup.colorBufferTarget == GL_TEXTURE_2D) {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture((setup.colorBufferMultisample ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D), colorBufferId);
        shader->SetInt("u_ColorTexture", 0);
    }
//    if (setup.depthBufferTarget == GL_TEXTURE_2D) {
//        glActiveTexture(GL_TEXTURE1);
//        glBindTexture((setup.colorBufferMultisample ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D), colorBufferId);
//        shader->SetFloat("u_DepthTexture", 1);
//    }
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, NULL);

    shader->Unbind();
    ibo->Unbind();
    vao->Unbind();
}

void Framebuffer::Blit(const Framebuffer* const fbo)
{
    glBindFramebuffer(GL_READ_FRAMEBUFFER, fboId);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo->GetID());
    
    glBlitFramebuffer(0, 0, setup.width, setup.height,
                      0, 0, fbo->setup.width, fbo->setup.height,
                      setup.blit,
                      GL_NEAREST);
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

GPUContext::GPUContext()
{
    glGetIntegerv(GL_GPU_MEMORY_INFO_DEDICATED_VIDMEM_NVX, &gpu_memory_total);
    glGetIntegerv(GL_GPU_MEMORY_INFO_CURRENT_AVAILABLE_VIDMEM_NVX, &gpu_memory_available);

    renderer = (const char*)glGetString(GL_RENDERER);
    version = (const char*)glGetString(GL_VERSION);
    vendor = (const char*)glGetString(GL_VENDOR);

    glGetIntegerv(GL_NUM_SPIR_V_EXTENSIONS, &num_glsl_extensions);
    glsl_extensions = (const char*)glGetString(GL_SPIR_V_EXTENSIONS);

    glGetIntegerv(GL_NUM_EXTENSIONS, &num_extensions);
    extensions = (char **)Z_Malloc(sizeof(char *) * num_extensions, TAG_STATIC, &extensions, "OpenGL_EXT");
    for (int i = 0; i < num_extensions; i++) {
        const char* str = (const char*)glGetStringi(GL_EXTENSIONS, i);
        extensions[i] = (char *)Z_Malloc(strlen(str)+1, TAG_STATIC, &extensions[i], "extensionStr");
        strncpy(extensions[i], str, strlen(str));
    }
}

GPUContext::~GPUContext()
{
}