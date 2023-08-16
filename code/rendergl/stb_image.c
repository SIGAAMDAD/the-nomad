#include <stdlib.h>
#include <stdint.h>

void *Malloc(uint64_t size);
void Free(void *p);
void *Realloc(void *p, uint64_t nsize);

#ifdef __cplusplus
extern "C" {
#endif
#define STB_IMAGE_IMPLEMENTATION
#define STBI_MALLOC(size) Malloc(size)
#define STBI_FREE(p) Free(p)
#define STBI_REALLOC(p,nsize) Realloc(p,nsize)
#define STBI_REALLOC_SIZED(p,oldsize,nsize) Realloc(p,nsize)
#include "stb_image.h"
#ifdef __cplusplus
}
#endif