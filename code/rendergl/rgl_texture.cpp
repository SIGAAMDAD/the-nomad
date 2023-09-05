#include "rgl_local.h"

extern "C" {
#include "stb_image.h"
}

void *Malloc(uint64_t size)
{
    return malloc(size);
}
void Free(void *p)
{
    free(p);
}
void *Realloc(void *p, uint64_t nsize)
{
    return realloc(p, nsize);
}

#define MAX_FILE_HASH 1024
static texture_t* textures[MAX_FILE_HASH];
static spritesheet_t* spritesheets[MAX_FILE_HASH];

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

extern "C" uint32_t StringToFilter(const char *str)
{
    if (!N_stricmp("GL_NEAREST", str)) return GL_NEAREST;
    else if (!N_stricmp("GL_LINEAR", str)) return GL_LINEAR;
    else if (!N_stricmp("GL_NEAREST_MIPMAP_NEAREST", str)) return GL_NEAREST_MIPMAP_NEAREST;
    else if (!N_stricmp("GL_LINEAR_MIPMAP_LINEAR", str)) return GL_LINEAR_MIPMAP_LINEAR;
    else if (!N_stricmp("GL_NEAREST_MIPMAP_LINEAR", str)) return GL_NEAREST_MIPMAP_LINEAR;
    else if (!N_stricmp("GL_LINEAR_MIPMAP_NEAREST", str)) return GL_LINEAR_MIPMAP_NEAREST;

    ri.N_Error("StringToFilter: invalid filter string: %s", str);
}

extern "C" uint32_t R_TexFormat(void)
{
    switch (r_textureDetail->i) {
    case TEX_GPUvsGod: return GL_RGBA16;
    case TEX_xtreme: return GL_RGBA12;
    case TEX_high: return GL_RGBA8;
    case TEX_medium: return GL_RGBA4;
    case TEX_low: return GL_RGB8;
    case TEX_msdos: return GL_RGB4;
    };
    return GL_RGBA4;
}

extern "C" uint32_t R_TexMagFilter(void)
{
    const int magFilter = StringToFilter(r_textureMagFilter->s);
    for (uint32_t i = 0; i < arraylen(filters); ++i) {
        if (filters[i].magFilter == magFilter)
            return magFilter;
    }
    ri.Con_Printf(WARNING, "r_textureMagFilter is invalid, setting to GL_NEAREST");
    ri.Cvar_Set("r_textureMagFilter", "GL_NEAREST");
    return GL_NEAREST;
}

extern "C" uint32_t R_TexMinFilter(void)
{
    const int minFilter = StringToFilter(r_textureMinFilter->s);
    for (uint32_t i = 0; i < arraylen(filters); ++i) {
        if (filters[i].minFilter == minFilter)
            return minFilter;
    }
    ri.Con_Printf(INFO, "WARNING: r_textureMinFilter is invalid, setting to GL_NEAREST");
    ri.Cvar_Set("r_textureMinFilter", "GL_NEAREST");
    return GL_NEAREST;
}

extern "C" uint32_t R_TexFilter(void)
{
    for (uint32_t i = 0; i < arraylen(filters); ++i) {
        if (!N_stricmpn(filters[i].filter, r_textureFiltering->s, strlen(filters[i].filter)))
            return filters[i].num;
    }

    ri.Con_Printf(INFO, "WARNING: r_textureFiltering is invalid, setting to Nearest");
    ri.Cvar_Set("r_textureFiltering", "Nearest");
    ri.Cvar_Set("r_textureMagFilter", "GL_NEAREST");
    ri.Cvar_Set("r_textureMinFilter", "GL_NEAREST");
    return TEXTURE_FILTER_NEAREST;
}

extern "C" void R_UpdateTextures(void)
{
    // clear the texture bound, if there is any
    R_UnbindTexture();

    const int minFilter = R_TexMinFilter();
    const int magFilter = R_TexMagFilter();
    const int wrapT = GL_REPEAT;
    const int wrapS = GL_REPEAT;

    for (uint32_t i = 0; i < rg.numTextures; ++i) {
        nglBindTexture(GL_TEXTURE_2D, rg.textures[i]->id);

        texture_t* t = rg.textures[i];

        nglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);
        nglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter);
        nglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapT);
        nglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapS);
        if (r_ARB_texture_filter_anisotropic->i)
            nglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY, glContext.maxAnisotropy);

        nglBindTexture(GL_TEXTURE_2D, 0);
    }
}

extern "C" void R_InitTexBuffer(texture_t *tex, qboolean withFramebuffer)
{
    // is there compression allowed/available, no framebuffer attachments should be compressed
    if (r_textureCompression->i && !withFramebuffer) {
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

extern "C" texture_t *R_GetTexture(const char *name)
{
    return textures[Com_GenerateHashValue(name, MAX_RENDER_TEXTURES)];
}

extern "C" nhandle_t RE_RegisterTexture(const char *name)
{
    uint64_t hash;
    texture_t *tex;

    if (strlen(name) >= MAX_GDR_PATH) {
        ri.Con_Printf(WARNING, "RE_RegisterTexture: name too long");
        return -1;
    }

    hash = Com_GenerateHashValue(name, MAX_RENDER_TEXTURES);
    
    // check if the texture already exists
    if (textures[hash])
        return (nhandle_t)hash;
    
    tex = R_InitTexture(name);
    return hash;
}

/*
NOTE TO SELF: for some reason, bffs don't work well with any textures that have very low (16x16 ish) resolution, just keep that in mind,
otherwise, it'll cause a crash.
*/
extern "C" texture_t *R_InitTexture(const char *filename)
{
    texture_t *t;
    char *buffer;
    stbi_uc *image;
    uint64_t bufferLen, hash;

    // do we already have it?
    hash = Com_GenerateHashValue(filename, MAX_RENDER_TEXTURES);
    if (textures[hash]) {
        return textures[hash];
    }

    t = (texture_t *)ri.Malloc(sizeof(texture_t), &t, "GLtexture");
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
    if (r_ARB_texture_filter_anisotropic->i)
        nglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY, glContext.maxAnisotropy);

    bufferLen = ri.FS_LoadFile(filename, (void **)&buffer);
    if (!buffer) {
        goto error;
    }
    stbi_set_flip_vertically_on_load(r_flipTextureVertically->i);
    image = stbi_load_from_memory((const stbi_uc *)buffer, bufferLen, (int *)&t->width, (int *)&t->height, (int *)&t->channels, 4);
    if (!image) {
        ri.Con_Printf(ERROR, "stbi_load_from_memory failed to load texture %s, error string: %s", filename, stbi_failure_reason());
        goto error;
    }

    nglTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, t->width, t->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);

    t->data = (byte *)ri.Malloc(t->width * t->height * t->channels, &t->data, "GLtexbuffer");
    memcpy(t->data, image, t->width * t->height * t->channels);
    ri.Mem_Free(image);

    nglBindTexture(GL_TEXTURE_2D, 0);

    ri.Con_Printf(DEBUG, "Loaded texture file %s", filename);
    textures[hash] = t;
    rg.textures[hash] = t;
    rg.numTextures++;

    return t;

    // keep the texture around just so that we don't try loading it up again
error:
    ri.Con_Printf(WARNING, "Failed to load texture '%s'", filename);
    textures[hash] = t;
    return NULL;
}

extern "C" void RE_ShutdownTextures(void)
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

const textureDetail_t textureDetails[6] = {
    {"GPUvsGod", TEX_GPUvsGod, (int)sizeof("GPUvsGod")},
    {"xtreme",   TEX_xtreme,   (int)sizeof("xtreme")},
    {"high",     TEX_high,     (int)sizeof("high")},
    {"medium",   TEX_medium,   (int)sizeof("medium")},
    {"low",      TEX_low,      (int)sizeof("low")},
    {"msdos",    TEX_msdos,    (int)sizeof("msdos")}
};

extern "C" texture_t *R_TextureFromHandle(nhandle_t handle)
{
    return textures[handle];
}

extern "C" void R_ShutdownTexture(texture_t *texture)
{
    nglDeleteTextures(1, (const GLuint *)&texture->id);
}

extern "C" void R_BindTexture(const texture_t* texture)
{
    if (backend.texId == texture->id)
        return;
    else if (backend.texId)
        nglBindTexture(GL_TEXTURE_2D, 0);
    
    backend.texId = texture->id;
    nglActiveTexture(GL_TEXTURE0);
    nglBindTexture(GL_TEXTURE_2D, texture->id);
}

extern "C" void R_UnbindTexture(void)
{
    if (backend.texId == 0) {
        return;
    }
    backend.texId = 0;
    nglBindTexture(GL_TEXTURE_2D, 0);
}
