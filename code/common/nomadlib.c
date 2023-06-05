#include "../src/n_shared.h"
#include "qvmstdlib.h"
#include "nomadlib.h"

void Sys_Con_Printf(const char *fmt);
void Sys_Con_Error(const char *fmt);

void Con_Printf(const char *fmt, ...)
{
    va_list argptr;
    char buffer[1024];

    va_start(argptr, fmt);
    vsprintf(buffer, fmt, argptr);
    va_end(argptr);

    Sys_Con_Printf(buffer);
}

void Con_Error(const char *fmt, ...)
{
    va_list argptr;
    char buffer[1024];
    strcpy(buffer, "WARNING: ");

    va_start(argptr, fmt);
    vsprintf(buffer+9, fmt, argptr);
    va_end(argptr);

    Sys_Con_Error(buffer);
}

#if 0
// optimized versions of the qvmstdlib stuff, may break with specific stuff, unknown tho
void N_memcpy(void *dst, const void *src, size_t count)
{
    size_t i;
    if ((( (long)dst | (long)src | count) & 7) == 0) {
        count >>= 3;
        for (i = 0; i < count; i++)
            ((long *)dst)[i] = ((long *)src)[i];
    }
    else if ((( (long)dst | (long)src | count) & 3) == 0) {
        count >>= 2;
        for (i = 0; i < count; i++)
            ((int *)dst)[i] = ((int *)src)[i];
    }
    else {
        for (i = 0; i < count; i++)
            ((char *)dst)[i] = ((char *)src)[i];
    }
}
#endif

void N_memcpy(void *dst, const void *src, size_t count)
{
    size_t chunks, offset, i;

    // small copies
    switch (count) {
    case 8:
        *(long *restrict)dst = *(const long *restrict)src;
        return;
    case 4:
        *(int *restrict)dst = *(const int *restrict)src;
        return;
    case 2:
        *(short *restrict)dst = *(const short *restrict)src;
        return;
    case 1:
        *(char *restrict)dst = *(const char *restrict)src;
        return;
    };

    if ((( (long)dst | (long)src | count) & 7) == 0) {
        chunks = count >> 3;
        offset = count - (chunks << 3);
        char *d = dst;
        const char *s = src;

        while (chunks--) {
            *(long *restrict)d++ = *(const long *restrict)s++;
            *(long *restrict)d++ = *(const long *restrict)s++;
            *(long *restrict)d++ = *(const long *restrict)s++;
            *(long *restrict)d++ = *(const long *restrict)s++;
            *(long *restrict)d++ = *(const long *restrict)s++;
            *(long *restrict)d++ = *(const long *restrict)s++;
            *(long *restrict)d++ = *(const long *restrict)s++;
            *(long *restrict)d++ = *(const long *restrict)s++;
        }
        while (offset--)
            *dst++ = *src++;
    }
    else if ((( (long)dst | (long)src | count) & 3) == 0) {
        count >>= 2;
        for (i = 0; i < count; i++)
            ((int *restrict)dst)[i] = ((const int *restrict)src)[i];
    }
    else {
        for (i = 0; i < count; i++)
            ((char *restrict)dst)[i] = ((const char *restrict)src)[i];
    }
}

void* N_memset(void *dst, int fill, size_t count)
{
    size_t i;
    if ((( (long)dst | (long)src | count) & 7) == 0) {
        count >>= 3;
        fill = fill | (fill<<8) | (fill<<16) | (fill<<24) | (fill<<32) | (fill<<40) | (fill<<48) | (fill<<56);
        for (i = 0; i < count; i++)
            ((long *)dst)[i] = fill;
    }
    else if ((( (long)dst | (long)src | count) & 3) == 0) {
        count >>= 2;
        fill = fill | (fill<<8) | (fill<<16) | (fill<<24);
        for (i = 0; i < count; i++)
            ((int *)dst)[i] = fill;
    }
    else {
        for (i = 0; i < count; i++)
            ((char *)dst)[i] = fill;
    }
    return dst;
}

int N_memcmp(const void *p1, const void *p2, size_t count)
{
    while (count--) {
        if (((const *)p1)[i] != ((const byte *)p2)[i])
            return 0;
    }
    return (int)(p1 - p2);
}