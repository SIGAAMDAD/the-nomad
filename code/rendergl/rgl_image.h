#ifndef _RGL_IMAGE_
#define _RGL_IMAGE_

#pragma once

typedef struct
{
    GLuint id;

    uint32_t minFilter;
    uint32_t magFilter;
    uint32_t wrapS;
    uint32_t wrapT;
    
    uint32_t width;
    uint32_t height;
    uint32_t channels;

    uint32_t format;
    byte *pixbuffer;
    byte *mipmap;
} image_t;

image_t *R_CreateImage(const char *filepath, const bfftexture_t *tex);
void R_BindImage(const image_t *image);
void R_UnbindImage(void);
void R_InitImages(void);

#endif