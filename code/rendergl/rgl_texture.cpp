#include "rgl_local.h"
#include "stb_image.h"

void *Malloc(uint32_t size)
{
    return ri.Z_Malloc(size, TAG_RENDERER, NULL, "stbimage");
}
void Free(void *p)
{
    ri.Z_Free(p);
}
void *Realloc(void *p, uint64_t nsize)
{
    return ri.Z_Realloc(nsize, TAG_RENDERER, NULL, p, "stbimage");
}

#define MAX_FILE_HASH 1024
static texture_t* textures[MAX_FILE_HASH];

typedef struct
{
    const char *glstr, *filter;
    int magFilter, minFilter, num;
} texmode_t;

static const texmode_t filters[] = {
    {"GL_NEAREST", "Nearest", GL_NEAREST, GL_NEAREST, TEXTURE_FILTER_NEAREST},
    {"GL_LINEAR", "Linear", GL_LINEAR, GL_LINEAR, TEXTURE_FILTER_LINEAR},
    {"GL_NEAREST_MIPMAP_NEAREST", "Billenear", GL_NEAREST_MIPMAP_NEAREST, GL_NEAREST, TEXTURE_FILTER_BILLINEAR},
    {"GL_LINEAR_MIPMAP_LINEAR", "Trillenear", GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR, TEXTURE_FILTER_TRILINEAR},
    {"GL_NEAREST_MIPMAP_LINEAR", "Billinear", GL_NEAREST_MIPMAP_LINEAR, GL_NEAREST, TEXTURE_FILTER_BILLINEAR},
    {"GL_LINEAR_MIPMAP_NEAREST", "Billinear", GL_LINEAR_MIPMAP_NEAREST, GL_LINEAR, TEXTURE_FILTER_BILLINEAR}
};

GO_AWAY_MANGLE uint32_t StringToFilter(const char *str)
{
    if (!N_stricmpn("GL_NEAREST", str, sizeof("GL_NEAREST"))) return GL_NEAREST;
    else if (!N_stricmpn("GL_LINEAR", str, sizeof("GL_LINEAR"))) return GL_LINEAR;
    else if (!N_stricmpn("GL_NEAREST_MIPMAP_NEAREST", str, sizeof("GL_NEAREST_MIMAP_NEAREST"))) return GL_NEAREST_MIPMAP_NEAREST;
    else if (!N_stricmpn("GL_LINEAR_MIPMAP_LINEAR", str, sizeof("GL_LINEAR_MIPMAP_LINEAR"))) return GL_LINEAR_MIPMAP_LINEAR;
    else if (!N_stricmpn("GL_NEAREST_MIPMAP_LINEAR", str, sizeof("GL_NEAREST_MIPMAP_LINEAR"))) return GL_NEAREST_MIPMAP_LINEAR;
    else if (!N_stricmpn("GL_LINEAR_MIPMAP_NEAREST", str, sizeof("GL_LINEAR_MIPMAP_NEAREST"))) return GL_LINEAR_MIPMAP_NEAREST;

    ri.N_Error("StringToFilter: invalid filter string");
}

GO_AWAY_MANGLE uint32_t R_TexFormat(void)
{
    switch (R_GetTextureDetail()) {
    case GPUvsGod: return GL_RGBA32F;
    case extreme: return GL_RGBA16F;
    case high: return GL_RGBA12;
    case medium: return GL_RGBA8;
    case low: return GL_RGBA4;
    case msdos: return GL_RGBA2;
    };
    return GL_RGBA8;
}

GO_AWAY_MANGLE uint32_t R_TexMagFilter(void)
{
    const int magFilter = StringToFilter(r_textureMagFilter->s);
    for (uint32_t i = 0; i < arraylen(filters); ++i) {
        if (filters[i].magFilter == magFilter)
            return magFilter;
    }
    ri.Con_Printf(WARNING, "r_textureMagFilter is invalid, setting to GL_NEAREST");
    N_strncpyz(r_textureMagFilter->s, "GL_NEAREST", sizeof("GL_NEAREST"));
    return GL_NEAREST;
}

GO_AWAY_MANGLE uint32_t R_TexMinFilter(void)
{
    const int minFilter = StringToFilter(r_textureMinFilter->s);
    for (uint32_t i = 0; i < arraylen(filters); ++i) {
        if (filters[i].minFilter == minFilter)
            return minFilter;
    }
    ri.Con_Printf(INFO, "WARNING: r_textureMinFilter is invalid, setting to GL_NEAREST");
    N_strncpyz(r_textureMinFilter->s, "GL_NEAREST", sizeof("GL_NEAREST"));
    return GL_NEAREST;
}

GO_AWAY_MANGLE uint32_t R_TexFilter(void)
{
    for (uint32_t i = 0; i < arraylen(filters); ++i) {
        if (!N_stricmpn(filters[i].filter, r_textureFiltering->s, strlen(filters[i].filter)))
            return filters[i].num;
    }

    ri.Con_Printf(INFO, "WARNING: r_textureFiltering is invalid, setting to Nearest");
    N_strncpyz(r_textureFiltering->s, "Nearest", sizeof("Nearest"));
    N_strncpyz(r_textureMagFilter->s, "GL_NEAREST", sizeof("GL_NEAREST"));
    N_strncpyz(r_textureMinFilter->s, "GL_NEAREST", sizeof("GL_NEAREST"));
    return TEXTURE_FILTER_NEAREST;
}

GO_AWAY_MANGLE void R_UpdateTextures(void)
{
    // clear the texture bound, if there is any
    R_UnbindTexture();

    const int minFilter = R_TexMinFilter();
    const int magFilter = R_TexMagFilter();
    const int wrapT = GL_REPEAT;
    const int wrapS = GL_REPEAT;

    for (uint32_t i = 0; i < renderer->numTextures; ++i) {
        nglBindTexture(GL_TEXTURE_2D, renderer->textures[i]->id);

        texture_t* t = renderer->textures[i];

        nglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);
        nglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter);
        nglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapT);
        nglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapS);
        if (r_EXT_anisotropicFiltering->b)
            nglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 1);

        nglBindTexture(GL_TEXTURE_2D, 0);
    }
}

GO_AWAY_MANGLE void R_InitTexBuffer(texture_t *tex, bool withFramebuffer)
{
    // is there compression allowed/available, no framebuffer attachments should be compressed
    if (r_textureCompression->b && !withFramebuffer) {
        nglCompressedTexImage2D(GL_TEXTURE_2D, 0, R_TexFormat(), tex->width, tex->height, 0, tex->width * tex->height * tex->channels, tex->data);
    }
    else {
        // multisampling?
        if (!N_stricmpn("MSAA", r_multisampleType->s, sizeof("MSAA")) && r_multisampleAmount->i) {
            nglTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, r_multisampleAmount->i, R_TexFormat(), tex->width, tex->height, GL_TRUE);
        }
        else {
            nglTexImage2D(GL_TEXTURE_2D, 0, R_TexFormat(), tex->width, tex->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, tex->data);
        }
    }
}

GO_AWAY_MANGLE texture_t *R_GetTexture(const char *name)
{
    return textures[Com_GenerateHashValue(name, MAX_FILE_HASH)];
}

/*
NOTE TO SELF: for some reason, bffs don't work well with any textures that have very low (16x16 ish) resolution, just keep that in mind,
otherwise, it'll cause a crash.
*/
GO_AWAY_MANGLE texture_t *R_InitTexture(const char *filename)
{
    texture_t *t;
    char *buffer;
    uint64_t bufferLen, hash;

    hash = Com_GenerateHashValue(filename, MAX_TEXTURES);
    bufferLen = ri.FS_LoadFile(filename, &buffer);
    if (!buffer) {
        ri.N_Error("R_InitTexture: failed to load texture file '%s'", filename);
    }
    if (textures[hash]) {
        return NULL;
    }

    t = (texture_t *)ri.Z_Malloc(sizeof(texture_t), TAG_RENDERER, &t, "GLtexture");
    t->minFilter = R_TexMinFilter();
    t->magFilter = R_TexMagFilter();
    t->wrapS = GL_REPEAT;
    t->wrapT = GL_REPEAT;

    nglGenTextures(1, (GLuint *)&t->id);
    nglBindTexture(GL_TEXTURE_2D, t->id);

    nglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, t->minFilter);
    nglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, t->magFilter);
    nglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, t->wrapS);
    nglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, t->wrapT);
    if (r_EXT_anisotropicFiltering->b)
        nglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 1);

    stbi_uc *image = stbi_load_from_memory((const stbi_uc *)buffer, bufferLen, (int *)&t->width, (int *)&t->height, (int *)&t->channels, 4);
    if (!image)
        ri.N_Error("R_CreateTexture: stbi_load_from_memory failed to load file %s, error string: %s", filename, stbi_failure_reason());

    t->data = (byte *)ri.Z_Malloc(t->width * t->height * t->channels, TAG_RENDERER, &t->data, "GLtexbuffer");
    memcpy(t->data, image, t->width * t->height * t->channels);
    ri.Mem_Free(image);

    nglTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, t->width, t->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);

    nglBindTexture(GL_TEXTURE_2D, 0);

    ri.Con_Printf(DEBUG, "Loaded texture file %s", filename);
    textures[hash] = t;

    return t;
}

GO_AWAY_MANGLE void RE_ShutdownTextures(void)
{
    for (uint32_t i = 0; i < MAX_FILE_HASH; i++) {
        if (textures[i]) {
            nglDeleteTextures(1, (const GLuint *)&textures[i]->id);
            memset(textures[i]->data, 0, textures[i]->width * textures[i]->height * textures[i]->channels);
            textures[i]->width = 0;
            textures[i]->height = 0;
            textures[i]->channels = 0;
        }
    }
}

GO_AWAY_MANGLE void R_ShutdownTexture(texture_t *texture)
{
    nglDeleteTextures(1, (const GLuint *)&texture->id);
    ri.Z_Free(texture->data);
    ri.Z_Free(texture);
}

GO_AWAY_MANGLE void R_BindTexture(const texture_t* texture, uint32_t slot)
{
    if (renderer->textureid == texture->id)
        return;
    else if (renderer->textureid)
        nglBindTexture(GL_TEXTURE_2D, 0);
    
    renderer->textureid = texture->id;
    nglActiveTexture(GL_TEXTURE0+slot);
    nglBindTexture(GL_TEXTURE_2D, texture->id);
}

GO_AWAY_MANGLE void R_UnbindTexture(void)
{
    if (renderer->textureid == 0) {
        return;
    }
    renderer->textureid = 0;
    nglBindTexture(GL_TEXTURE_2D, 0);
}
