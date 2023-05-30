#include "n_shared.h"
#include "n_scf.h"
#include "m_renderer.h"

typedef struct
{
    const char *str;
    GLint value;
} texmode_t;

static const texmode_t modes[] = {
    {"GL_NEAREST", GL_NEAREST},
    {"GL_LINEAR", GL_LINEAR},
    {"GL_LINEAR_MIPMAP_LINEAR", GL_LINEAR_MIPMAP_LINEAR},
    {"GL_LINEAR_MIPMAP_NEAREST", GL_LINEAR_MIPMAP_NEAREST},
    {"GL_NEAREST_MIPMAP_LINEAR", GL_NEAREST_MIPMAP_LINEAR},
    {"GL_NEAREST_MIPMAP_NEAREST", GL_NEAREST_MIPMAP_NEAREST}
};

static GLint R_TexMagFilter(void)
{
    GLint filter = -1;
    for (uint32_t i = 0; i < arraylen(modes); i++) {
        if (N_strcmp(r_texture_magfilter.value, modes[i].str)) {
            filter = modes[i].value;
            break;
        }
    }
    if (filter == -1) {
        Con_Printf("WARNING: r_texture_magfilter was invalid, using default of GL_NEAREST");
        filter = GL_NEAREST;
    }
    return filter;
}
static GLint R_TexMinFilter(void)
{
    GLint filter = -1;
    for (uint32_t i = 0; i < arraylen(modes); i++) {
        if (N_strcmp(r_texture_minfilter.value, modes[i].str)) {
            filter = modes[i].value;
            break;
        }
    }
    if (filter == -1) {
        Con_Printf("WARNING: r_texture_minfilter was invalid, using default of GL_LINEAR_MIPMAP_LINEAR");
        filter = GL_LINEAR_MIPMAP_LINEAR;
    }
    return filter;
}

void R_UpdateTextures(void)
{
    // clear the texture binded, if there is any
    R_UnbindTexture();

    for (uint32_t i = 0; i < renderer->numTextures; i++) {
        glBindTexture(GL_TEXTURE_2D, renderer->textures[i]->id);

        texture_t* tex = renderer->textures[i];
        tex->minFilter = R_TexMinFilter();
        tex->magFilter = R_TexMagFilter();
        tex->wrapS = GL_REPEAT;
        tex->wrapT = GL_REPEAT;

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, tex->minFilter);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, tex->magFilter);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, tex->wrapT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, tex->wrapS);

        glBindTexture(GL_TEXTURE_2D, 0);
    }
}

texture_t* R_CreateTexture(const char *filepath, const char *name)
{
    texture_t* tex = (texture_t *)Hunk_Alloc(sizeof(texture_t), name, h_low);

    tex->minFilter = R_TexMinFilter();
    tex->magFilter = R_TexMagFilter();
    tex->wrapS = GL_REPEAT;
    tex->wrapT = GL_REPEAT;

    glGenTextures(1, &tex->id);
    glBindTexture(GL_TEXTURE_2D, tex->id);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, tex->minFilter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, tex->magFilter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, tex->wrapS);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, tex->wrapT);
    
    byte* image = SOIL_load_image(filepath, &tex->width, &tex->height, &tex->channels, SOIL_LOAD_RGBA);
    if (!image) {
        N_Error("R_CreateTexture: SOIL_load_image failed to load file %s, error string: %s", filepath, SOIL_last_result());
    }
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, tex->width, tex->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
    glBindTexture(GL_TEXTURE_2D, 0);

    tex->data = (byte *)Hunk_Alloc(tex->width * tex->height * 4, "texbuffer", h_low);
    memcpy(tex->data, image, tex->width * tex->height * 4);
    free(image);

    Con_Printf("Loaded texture file %s", filepath);

    renderer->textures[renderer->numTextures] = tex;
    renderer->numTextures++;

    return tex;
}