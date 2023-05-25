#include "n_shared.h"
#include "n_scf.h"
#include "g_zone.h"
#include "m_renderer.h"

static byte* R_LoadImage(const eastl::string& filepath)
{
    const char* file = filepath.c_str();
    if (!strcasestr(file, ".jpg")
    ||  !strcasestr(file, ".png")
    ||  !strcasestr(file, ".bmp")
    ||  !strcasestr(file, ".tga")
    ||  !strcasestr(file, ".pcx")) {
        N_Error("R_LoadImage: unsupported image file type: %s", file);
    }
    byte *buffer;
    return buffer;
}

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

}

Texture2D::Texture2D(const Texture2DSetup& setup, const eastl::string& filepath)
{
    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, R_TexMinFilter());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, R_TexMagFilter());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    byte* image = SOIL_load_image(filepath.c_str(), &width, &height, 0, SOIL_LOAD_RGBA);
    if (!image)
        N_Error("Texture2D::Texture2D: SOIL_load_image failed for texture file %s, error string: %s", filepath.c_str(), SOIL_last_result());
    
    assert(image);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
    
    if (setup.genMipmap)
        glGenerateMipmap(GL_TEXTURE_2D);

    glBindTexture(GL_TEXTURE_2D, 0);

    buffer = (byte *)Z_Malloc(width * height * 4, TAG_STATIC, &buffer, "texbuffer");
    memcpy(buffer, image, width * height * 4);
    (free)(image);

    Con_Printf("Texture2D::Texture2D: successfully loaded texture file %s", filepath.c_str());
}

Texture2D::~Texture2D()
{
    glDeleteTextures(1, &id);
    Z_Free(buffer);
}

Texture2D* Texture2D::Create(const Texture2DSetup& setup, const eastl::string& filepath, const eastl::string& name)
{
    Con_Printf("Texture2D::Create: loading texture file %s", filepath.c_str());
    return CONSTRUCT(Texture2D, name.c_str(), setup, filepath);
}