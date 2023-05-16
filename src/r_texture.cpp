#include "n_shared.h"
#include "g_game.h"

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

Texture2D::Texture2D(const Texture2DSetup& setup, const eastl::string& filepath)
{
    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, setup.minFilter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, setup.magFilter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, setup.wrapS);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, setup.wrapT);
    glTexParameteri(GL_TEXTURE_2D, GL_AUTO_GENERATE_MIPMAP, (setup.genMipmap ? GL_TRUE : GL_FALSE));

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

    LOG_INFO("successfully loaded texture file {}", filepath.c_str());
}

Texture2D::~Texture2D()
{
    glDeleteTextures(1, &id);
    Z_Free(buffer);
}

Texture2D* Texture2D::Create(const Texture2DSetup& setup, const eastl::string& filepath, const eastl::string& name)
{
    LOG_INFO("loading texture file {}", filepath.c_str());
    return CONSTRUCT(Texture2D, name.c_str(), setup, filepath);
}