#include <stdlib.h>
#include <stdint.h>

void *Malloc(uint32_t size);
void Free(void *p);
uint32_t Msize(void *p);

#define STB_IMAGE_IMPLEMENTATION
#define STBI_MALLOC(size) Malloc(size)
#define STBI_FREE(p) Free(p)
#define STBI_REALLOC(p,nsize) ({void *ptr=Malloc(nsize);if(p){memcpy(ptr,p,Msize(p));Free(p);}ptr;})
#define STBI_REALLOC_SIZED(p,oldsize,nsize) ({void *ptr = Malloc(nsize);if(p){memcpy(ptr, p, oldsize);Free(p);}ptr;})
#include "stb_image.h"