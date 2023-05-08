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

Texture2D::Texture2D(const eastl::string& filepath, bool _multisample)
    : multisampled(_multisample)
{    
    // texture
    glGenTextures(1, &id);
    glBindTexture((_multisample ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D), id);

    if (!_multisample) {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    }

    byte* image = SOIL_load_image(filepath.c_str(), &width, &height, 0, SOIL_LOAD_RGBA);
    if (!image)
        N_Error("Texture2D::Texture2D: SOIL_load_image failed for texture file %s, error string: %s", filepath.c_str(), SOIL_last_result());
    
    assert(image);
    if (!_multisample)
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
    else
        glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGBA, width, height, GL_TRUE);
    
    if (!_multisample)
        glGenerateMipmapEXT(GL_TEXTURE_2D);
    
    buffer = (byte *)Z_Malloc((width * height) * 4, TAG_STATIC, &buffer, "texbuffer");
    memcpy(buffer, image, (width * height) * 4);
    SOIL_free_image_data(image);
    glBindTexture((_multisample ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D), 0);
    LOG_INFO("successfully loaded texture file {}", filepath.c_str());
}

Texture2D::~Texture2D()
{
    glDeleteTextures(1, &id);
    Z_Free(buffer);
}

Texture2D* Texture2D::Create(const eastl::string& filepath, const eastl::string& name, bool multisample)
{
    LOG_INFO("loading texture file {}", filepath.c_str());
    Texture2D* ptr = (Texture2D *)Z_Malloc(sizeof(Texture2D), TAG_STATIC, &ptr, name.c_str());
    new (ptr) Texture2D(filepath, multisample);
    return ptr;
}