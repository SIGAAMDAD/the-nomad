#include "rgl_public.h"
#include "rgl_local.h"
#include "../src/n_scf.h"

#define STBI_MALLOC(sz)           ri.Malloc(sz)
#define STBI_REALLOC(p,newsz)     ri.Realloc(p,newsz)
#define STBI_FREE(p)              ri.Free(p)
#define STBI_IMAGE_IMPLEMENTATION
#include "stb_image.h"

static image_t *images[RENDER_MAX_IMAGES];

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
    {"GL_NEAREST_MIPMAP_NEAREST", GL_NEAREST_MIPMAP_NEAREST},
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
        ri.Printf("WARNING: r_texture_magfilter was invalid, using default of GL_NEAREST");
        filter = GL_NEAREST;
        N_strcpy(r_texture_magfilter.value, "GL_NEAREST");
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
        ri.Printf("WARNING: r_texture_minfilter was invalid, using default of GL_LINEAR_MIPMAP_LINEAR");
        filter = GL_LINEAR_MIPMAP_LINEAR;
        N_strcpy(r_texture_minfilter.value, "GL_LINEAR_MIPMAP_LINEAR");
    }
    return filter;
}

void R_UpdateTextures(void)
{
    // clear the texture bound, if there is any
    nglBindTexture(GL_TEXTURE_2D, 0);

    for (uint32_t i = 0; i < arraylen(images); i++) {
        if (!images[i])
            continue;

        nglBindTexture(GL_TEXTURE_2D, images[i]->id);

        image_t *image = images[i];
        image->minFilter = R_TexMinFilter();
        image->magFilter = R_TexMagFilter();
        image->wrapS = GL_REPEAT;
        image->wrapT = GL_REPEAT;

        nglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, image->minFilter);
        nglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, image->magFilter);
        nglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, image->wrapT);
        nglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, image->wrapS);

        nglBindTexture(GL_TEXTURE_2D, 0);
    }
}

image_t* R_GetImage(const char *chunkname)
{
    if (!images[ri.Com_GenerateHashValue(chunkname, RENDER_MAX_IMAGES)])
        ri.Error("R_GetImage: image %s hasn't been loaded", chunkname);
    
    return images[ri.Com_GenerateHashValue(chunkname, RENDER_MAX_IMAGES)];
}

image_t* R_CreateImage(const char *name, const bfftexture_t *tex)
{
    image_t *image;

    if (!tex)
        ri.Error("R_CreateImage: null image");
    
    image = (image_t *)Hunk_Alloc(sizeof(image_t), "GLimage", h_low);
    image->minFilter = R_TexMinFilter();
    image->magFilter = R_TexMagFilter();

    nglGenTextures(1, &image->id);
    nglBindTexture(GL_TEXTURE_2D, image->id);

    nglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, image->magFilter);
    nglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, image->minFilter);
    nglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, image->wrapT);
    nglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, image->wrapS);

    image->format = GL_RGBA8;

    stbi_uc *buffer = stbi_load_from_memory(
        (stbi_uc *const)tex->fileBuffer, tex->fileSize, (int *)&image->width, (int *)&image->height, (int *)&image->channels, 4);
    if (!buffer)
        ri.Error("R_CreateImage: failed to load texture chunk %s, stbi_strerror: %s", tex->name, stbi_failure_reason());

    nglTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, image->width, image->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, buffer);
    nglBindTexture(GL_TEXTURE_2D, 0);

    image->pixbuffer = (byte *)ri.Hunk_Alloc(image->width * image->height * 4, "imagebuffer", h_low);
    memcpy(image->pixbuffer, buffer, image->width * image->height * 4);
    ri.Free(buffer);

    images[ri.Com_GenerateHashValue(tex->name, RENDER_MAX_IMAGES)] = image;

    return image;
}

void R_BindImage(const image_t *image)
{
    if (glState.imageId == image->id)
        return; // already bound
    if (glState.imageId)
        nglBindTexture(GL_TEXTURE_2D, 0);
    
    glState.imageId = image->id;
    nglBindTexture(GL_TEXTURE_2D, image->id);
}

void R_UnbindImage(void)
{
    if (!glState.imageId)
        return; // already unbound
    
    glState.imageId = 0;
    nglBindTexture(GL_TEXTURE_2D, 0);
}

void R_InitImages(void)
{
    bffinfo_t *info = ri.BFF_FetchInfo();

    for (uint32_t i = 0; i < info->numTextures; i++) {
        R_CreateImage(info->textures[i].name, &info->textures[i]);
    }
}