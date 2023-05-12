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

#if 0

Framebuffer* Framebuffer::Create(const eastl::string& name)
{
    Framebuffer* ptr = (Framebuffer *)Z_Malloc(sizeof(Framebuffer), TAG_STATIC, &ptr, name.c_str());
    new (ptr) Framebuffer();
    return ptr;
}

Framebuffer::Framebuffer()
{
    glGenFramebuffers(1, &fboId);
    glGenFramebuffers(1, &fboMsaaId);
    glGenTextures(1, &texMsaaColorId);

    glBindFramebuffer(GL_FRAMEBUFFER, fboId);

    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, texColorId);
    glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGBA8, scf::renderer::width, scf::renderer::height, GL_TRUE);
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);

    glBindRenderbuffer(GL_RENDERBUFFER, rboDepthId);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, scf::renderer::width, scf::renderer::height);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, texColorId, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rboDepthId);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        N_Error("Framebuffer::Framebuffer: glCheckFramebufferStatus != GL_FRAMEBUFFER_COMPLETE");

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    Vertex vertices[] = {
        Vertex(glm::vec3( 0.5f,  0.5f, 0.0f), glm::vec2(1.0f, 1.0f)), // top right
        Vertex(glm::vec3( 0.5f, -0.5f, 0.0f), glm::vec2(1.0f, 0.0f)), // bottom right
        Vertex(glm::vec3(-0.5f, -0.5f, 0.0f), glm::vec2(0.0f, 0.0f)), // bottom left
        Vertex(glm::vec3(-0.5f,  0.5f, 0.0f), glm::vec2(0.0f, 1.0f)), // top left
    };
    uint32_t indices[] = {
        0, 1, 2,
        0, 2, 3
    };

    vao = VertexArray::Create("fbo_VAO");
    glBindVertexArray(0); // extra security
    vao->Bind();
    vbo = VertexBuffer::Create(vertices, sizeof(Vertices), "fbo_VBO");
    vbo->Bind();

    vbo->PushVertexAttrib(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void *)offsetof(Vertex, pos));
    vbo->PushVertexAttrib(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void *)offsetof(Vertex, texcoords));

    ibo = IndexBuffer::Create(indices, 6, GL_UNSIGNED_INT, "fbo_IBO");

    vbo->Unbind();
    vao->Unbind();

    shader = Shader::Create("framebuffer.glsl", "fbo_Shader");
}

Framebuffer::~Framebuffer()
{
    glDeleteFramebuffers(1, &fboId);
    glDeleteTextures(1, &texColorId);
    glDeleteRenderbuffers(1, &rboDepthId);
}

void Framebuffer::SetBuffer(void)
{
    glBindFramebuffer(GL_FRAMEBUFFER, fboId);
    glViewport(0, 0, scf::renderer::width, scf::renderer::height);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Framebuffer::SetDefault(void)
{
    glBindFramebuffer(GL_READ_FRAMEBUFFER, fboId);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

    glBlitFramebuffer(0, 0, scf::renderer::width, scf::renderer::height,
                      0, 0, scf::renderer::width, scf::renderer::height,
                      GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT,
                      GL_NEAREST);
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
#endif

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