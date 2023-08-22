#include "n_shared.h"
#include "n_scf.h"
#include "m_renderer.h"

framebuffer_t *fbo;
framebuffer_t *intermediate;

static const texture_t *screenTexture;
static shader_t *screenShader;
static uint32_t screenVAO, screenVBO, screenIBO;

typedef enum : uint32_t
{
    SSAA,
    MSAA,
    FXAA,
    MLAA,
    SMAA,
    TSAA
} antialiasing_t;

static void R_FinishFramebuffer(framebuffer_t *fbo, const char *name)
{
    glBindFramebuffer(GL_FRAMEBUFFER, fbo->fboId);
    if (fbo->depthId)
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, fbo->depthId);
    for (uint32_t i = 0; i < fbo->usedColors; ++i)
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_RENDERBUFFER, fbo->colorIds[i]);
    
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        N_Error("RE_InitFramebuffers: %s framebuffer is incomplete!", name);
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

static void R_AllocFramebuffer(framebuffer_t *fbo, bool depth, uint32_t numColors)
{
    fbo->usedColors = numColors;

    glGenFramebuffers(1, (GLuint *)&fbo->fboId);

    fbo->depthId = 0;
    if (depth)
        glGenRenderbuffers(1, (GLuint *)&fbo->depthId);
    for (uint32_t i = 0; i < numColors; ++i)
        glGenRenderbuffers(1, (GLuint *)&fbo->colorIds[i]);
}

static void R_DeallocFramebuffer(framebuffer_t *fbo)
{
    glDeleteFramebuffers(1, (GLuint *)&fbo->fboId);

    if (fbo->depthId)
        glDeleteRenderbuffers(1, (GLuint *)&fbo->depthId);
    for (uint32_t i = 0; i < fbo->usedColors; ++i)
        glDeleteRenderbuffers(1, (GLuint *)&fbo->colorIds[i]);
}

static void RE_InitMsaa(void)
{
    fbo = (framebuffer_t *)ri.Z_Malloc(sizeof(framebuffer_t), "GLfbo", h_low);

    fbo->width = 1024;
    fbo->height = 720;

    R_AllocFramebuffer(fbo, false, 0);

    glBindRenderbuffer(GL_RENDERBUFFER, fbo->colorIds[0]);
    glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_RGB, fbo->width, fbo->height);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    
    glBindRenderbuffer(GL_RENDERBUFFER, fbo->depthId);
    glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_DEPTH24_STENCIL8, fbo->width, fbo->height);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);

    R_FinishFramebuffer(fbo, "post-processing");
}

static void RE_InitIntermediate(void)
{
    intermediate = (framebuffer_t *)Hunk_Alloc(sizeof(framebuffer_t), "GLfbo", h_low);

    intermediate->width = 1024;
    intermediate->height = 720;
    
    R_AllocFramebuffer(intermediate, false, 1);

    glBindRenderbuffer(GL_RENDERBUFFER, intermediate->colorIds[0]);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_RGB, intermediate->width, intermediate->height);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);

    R_FinishFramebuffer(intermediate, "intermediate");
}

void RE_InitFramebuffers(void)
{
#if 0
    RE_InitMsaa();
    RE_InitIntermediate();

    screenShader = R_CreateShader("gamedata/framebuffer.glsl", "screenShader");

    // intermediate fbo screen rendering
    RGL_UnbindCache();
    const vertex_t vertices[4] = {
        vertex_t(glm::vec3( 1.0f,  1.0f, 0.0f), glm::vec2(0.0f, 0.0f), glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)), // top right
        vertex_t(glm::vec3( 1.0f, -1.0f, 0.0f), glm::vec2(0.0f, 1.0f), glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)), // bottom right
        vertex_t(glm::vec3(-1.0f, -1.0f, 0.0f), glm::vec2(1.0f, 1.0f), glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)), // bottom left
        vertex_t(glm::vec3(-1.0f,  1.0f, 0.0f), glm::vec2(1.0f, 0.0f), glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)), // top left
    };
    const uint32_t indices[6] = {
        0, 1, 2,
        2, 3, 0
    };

    glGenVertexArrays(1, (GLuint *)&screenVAO);
    glGenBuffersARB(1, (GLuint *)&screenVBO);
    glGenBuffersARB(1, (GLuint *)&screenIBO);

    glBindVertexArray(screenVAO);

    glBindBufferARB(GL_ARRAY_BUFFER_ARB, screenVBO);
    glBufferDataARB(GL_ARRAY_BUFFER_ARB, sizeof(vertex_t) * 4, vertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vertex_t), (const void *)offsetof(vertex_t, pos));

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(vertex_t), (const void *)offsetof(vertex_t, texcoords));

    glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);

    glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, screenIBO);
    glBufferDataARB(GL_ELEMENT_ARRAY_BUFFER_ARB, sizeof(uint32_t) * 6, indices, GL_STATIC_DRAW);
    glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, 0);

    glBindVertexArray(0);
#endif
}

void RE_ShutdownFramebuffers(void)
{
#if 0
    R_DeallocFramebuffer(fbo);
    R_DeallocFramebuffer(intermediate);

    glDeleteVertexArrays(1, (GLuint *)&screenVAO);
    glDeleteBuffersARB(1, (GLuint *)&screenVBO);
    glDeleteBuffersARB(1, (GLuint *)&screenIBO);
#endif
}

void RE_SetScreenTexture(const texture_t *tex)
{
    screenTexture = tex;
}

void RE_BeginFramebuffer(void)
{
#if 0
    EASY_FUNCTION();
    glBindFramebuffer(GL_FRAMEBUFFER, fbo->fboId);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glViewport(0, 0, fbo->width, fbo->height);
#endif
}

void RE_EndFramebuffer(void)
{
#if 0
    EASY_FUNCTION();
    glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo->fboId);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

    glBlitFramebuffer(0, 0, fbo->width, fbo->height,
                      0, 0, fbo->width, fbo->height,
                      GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT,
                      GL_NEAREST);
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
#endif
    
//    glBindFramebuffer(GL_READ_FRAMEBUFFER, intermediate->fboId);
//    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
//
//    RGL_UnbindCache();
//
//    R_BindShader(screenShader);
//    if (screenTexture)
//        R_BindTexture(screenTexture);
//    
//    glBindVertexArray(screenVAO);
//    glBindBufferARB(GL_ARRAY_BUFFER_ARB, screenVBO);
//    glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, screenIBO);
//
//    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, NULL);
//
//    glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
//    glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, 0);
//    glBindVertexArray(0);
//    if (screenTexture)
//        R_UnbindTexture();
//    
//    R_UnbindShader();
//
//    // set it back to default
//    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
