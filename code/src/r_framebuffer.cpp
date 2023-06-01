#include "n_shared.h"
#include "n_scf.h"
#include "m_renderer.h"
#if 0

framebuffer_t* R_CreateFramebuffer(const char *name)
{
    framebuffer_t* fbo = (framebuffer_t *)Hunk_Alloc(sizeof(framebuffer_t), name, h_low);

    fbo->width = N_atoi(r_screenwidth.value);
    fbo->height = N_atoi(r_screenheight.value);

    glGenFramebuffers(1, &fbo->fboId);
    glGenRenderbuffers(1, &fbo->colorId);
    glGenRenderbuffers(1, &fbo->depthId);

    glBindFramebuffer(GL_FRAMEBUFFER, fbo->fboId);

    glBindRenderbuffer(GL_RENDERBUFFER, fbo->colorId);
    glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_RGBA8, fbo->width, fbo->height);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);

    glBindRenderbuffer(GL_RENDERBUFFER, fbo->depthId);
    glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_DEPTH24_STENCIL8, fbo->width, fbo->height);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);

    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, fbo->colorId);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, fbo->depthId);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        N_Error("R_CreateFramebuffer: (fboId) glCheckFramebufferStatus != GL_FRAMEBUFFER_COMPLETE");

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glGenFramebuffers(1, &fbo->defId);
    glGenTextures(1, &fbo->defTex);

    glBindFramebuffer(GL_FRAMEBUFFER, fbo->defId);

    glBindTexture(GL_TEXTURE_2D, fbo->defTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, fbo->width, fbo->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_2D, 0);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        N_Error("R_CreateFramebuffer: (defId) glCheckFramebufferStatus != GL_FRAMEBUFFER_COMPLETE");

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    Vertex vertices[] = {
        Vertex(glm::vec3( 1.0f,  1.0f, 0.0f), glm::vec2(0.0f, 0.0f)), // top right
        Vertex(glm::vec3( 1.0f, -1.0f, 0.0f), glm::vec2(0.0f, 1.0f)), // bottom right
        Vertex(glm::vec3(-1.0f, -1.0f, 0.0f), glm::vec2(1.0f, 1.0f)), // bottom left
        Vertex(glm::vec3(-1.0f,  1.0f, 0.0f), glm::vec2(1.0f, 0.0f)), // top left
    };
    uint32_t indices[] = {
        0, 1, 2,
        0, 2, 3
    };
//    fbo->cache = R_CreateCache(vertices, sizeof(vertices), indices, sizeof(indices), "fboVAO");
//    fbo->shader = R_CreateShader("gamedata/framebuffer.glsl", "fboShader");

    uint64_t hash = Com_GenerateHashValue(name, MAX_FBO_HASH);
    renderer->fbos[hash] = fbo;
    renderer->numFBOs++;

    return fbo;
}

static void R_SetDefaultFramebuffer(void)
{
    framebuffer_t* fbo = renderer->fbos[0];

    glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo->fboId);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo->defId);

    glBlitFramebuffer(0, 0, fbo->width, fbo->height,
                      0, 0, fbo->width, fbo->height,
                      GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT,
                      GL_NEAREST);
    
    glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo->defId);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

    glBlitFramebuffer(0, 0, fbo->width, fbo->height,
                      0, 0, fbo->width, fbo->height,
                      GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT,
                      GL_NEAREST);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    
    glUseProgram(fbo->shader->id);
    glBindTexture(GL_TEXTURE_2D, 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, fbo->defTex);
    renderer->textureid = fbo->defTex;

    R_SetInt(fbo->shader, "u_ColorTexture", 0);
    R_SetInt(fbo->shader, "u_ScreenTexture", 1);
    R_BindTexture(fbo->screenTexture, 1);

    R_DrawIndexed(fbo->cache, 6);

    glBindTexture(GL_TEXTURE_2D, 0);
    glUseProgram(0);
}

void R_SetFramebuffer(framebuffer_t* fbo)
{
    if (fbo == NULL) {
        R_SetDefaultFramebuffer();
    }
    else {
        glBindFramebuffer(GL_FRAMEBUFFER, fbo->fboId);
        glViewport(0, 0, fbo->width, fbo->height);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }
}
#endif