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

// optimized versions of the qvmstdlib stuff, may break with specific stuff, unknown tho
void N_memcpy(void *dst, const void *src, size_t count)
{
    size_t i;
    if ((( (long)dst | (long)src | count) & 7) == 0) {
        count >>= 2;
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

void* N_memset(void *dst, int fill, size_t count)
{
    size_t i;
    if ((( (long)dst | (long)src | count) & 7) == 0) {
        count >>= 2;
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
        if (((byte *)p1)[i] != ((byte *)p2)[i])
            return 0;
    }
    return (int)(p1 - p2);
}

qint64 Q64_AddInt(const qint64 *in, int n)
{
    qint64 out;

    out[0] = in[0] + n;
    out[1] = in[1] + n;
    out[2] = in[2] + n;
    out[3] = in[3] + n;
    out[4] = in[4] + n;
    out[5] = in[5] + n;
    out[6] = in[6] + n;
    out[7] = in[7] + n;

    return out;
}

qint64 Q64_DivideInt(const qint64 *in, int n)
{
    qint64 out;

    out[0] = in[0] / n;
    out[1] = in[1] / n;
    out[2] = in[2] / n;
    out[3] = in[3] / n;
    out[4] = in[4] / n;
    out[5] = in[5] / n;
    out[6] = in[6] / n;
    out[7] = in[7] / n;

    return out;
}

qint64 Q64_MultiplyInt(const qint64 *in, int n)
{
    qint64 out;

    out[0] = in[0] * n;
    out[1] = in[1] * n;
    out[2] = in[2] * n;
    out[3] = in[3] * n;
    out[4] = in[4] * n;
    out[5] = in[5] * n;
    out[6] = in[6] * n;
    out[7] = in[7] * n;

    return out;
}

qint64 Q64_SubtractInt(const qint64 *in, int n)
{
    qint64 out;

    out[0] = in[0] - n;
    out[1] = in[1] - n;
    out[2] = in[2] - n;
    out[3] = in[3] - n;
    out[4] = in[4] - n;
    out[5] = in[5] - n;
    out[6] = in[6] - n;
    out[7] = in[7] - n;

    return out;
}

qint64 Q64_ModInt(const qint64 *in, int n)
{
    qint64 out;

    out[0] = in[0] % n;
    out[1] = in[1] % n;
    out[2] = in[2] % n;
    out[3] = in[3] % n;
    out[4] = in[4] % n;
    out[5] = in[5] % n;
    out[6] = in[6] % n;
    out[7] = in[7] % n;

    return out;
}

qint64 Q64_BitandInt(const qint64 *in, int n)
{
    qint64 out;

    out[0] = in[0] & n;
    out[1] = in[1] & n;
    out[2] = in[2] & n;
    out[3] = in[3] & n;
    out[4] = in[4] & n;
    out[5] = in[5] & n;
    out[6] = in[6] & n;
    out[7] = in[7] & n;

    return out;
}

qint64 Q64_BitorInt(const qint64 *in, int n)
{
    qint64 out;

    out.b[0] = in.b[0] | n;
    out.b[1] = in.b[1] | n;
    out.b[2] = in.b[2] | n;
    out.b[3] = in.b[3] | n;
    out.b[4] = in.b[4] | n;
    out.b[5] = in.b[5] | n;
    out.b[6] = in.b[6] | n;
    out.b[7] = in.b[7] | n;

    return out;
}

qint64 Q64_BitxorInt(const qint64 *in, int n)
{
    qint64 out;

    out.b[0] = in.b[0] ^ n;
    out.b[1] = in.b[1] ^ n;
    out.b[2] = in.b[2] ^ n;
    out.b[3] = in.b[3] ^ n;
    out.b[4] = in.b[4] ^ n;
    out.b[5] = in.b[5] ^ n;
    out.b[6] = in.b[6] ^ n;
    out.b[7] = in.b[7] ^ n;

    return out;
}

qint64 Q64_AddShort(const qint64 *in, short n)
{
    qint64 out;

    out[0] = in[0] + n;
    out[1] = in[1] + n;
    out[2] = in[2] + n;
    out[3] = in[3] + n;
    out[4] = in[4] + n;
    out[5] = in[5] + n;
    out[6] = in[6] + n;
    out[7] = in[7] + n;

    return out;
}

qint64 Q64_DivideShort(const qint64 *in, short n)
{
    qint64 out;

    out[0] = in[0] / n;
    out[1] = in[1] / n;
    out[2] = in[2] / n;
    out[3] = in[3] / n;
    out[4] = in[4] / n;
    out[5] = in[5] / n;
    out[6] = in[6] / n;
    out[7] = in[7] / n;
    
    return out;
}

qint64 Q64_MultiplyShort(const qint64 *in, short n)
{
    qint64 out;

    out[0] = in[0] * n;
    out[1] = in[1] * n;
    out[2] = in[2] * n;
    out[3] = in[3] * n;
    out[4] = in[4] * n;
    out[5] = in[5] * n;
    out[6] = in[6] * n;
    out[7] = in[7] * n;

    return out;
}

qint64 Q64_SubtractShort(const qint64 *in, short n)
{
    qint64 out;

    out[0] = in[0] - n;
    out[1] = in[1] - n;
    out[2] = in[2] - n;
    out[3] = in[3] - n;
    out[4] = in[4] - n;
    out[5] = in[5] - n;
    out[6] = in[6] - n;
    out[7] = in[7] - n;

    return out;
}

qint64 Q64_ModShort(const qint64 *in, short n)
{
    qint64 out;

    out[0] = in[0] % n;
    out[1] = in[1] % n;
    out[2] = in[2] % n;
    out[3] = in[3] % n;
    out[4] = in[4] % n;
    out[5] = in[5] % n;
    out[6] = in[6] % n;
    out[7] = in[7] % n;

    return out;
}

qint64 Q64_BitandShort(const qint64 *in, short n)
{
    qint64 out;

    out[0] = in[0] & n;
    out[1] = in[1] & n;
    out[2] = in[2] & n;
    out[3] = in[3] & n;
    out[4] = in[4] & n;
    out[5] = in[5] & n;
    out[6] = in[6] & n;
    out[7] = in[7] & n;

    return out;
}

qint64 Q64_BitorShort(const qint64 *in, short n)
{
    qint64 out;

    out[0] = in[0] | n;
    out[1] = in[1] | n;
    out[2] = in[2] | n;
    out[3] = in[3] | n;
    out[4] = in[4] | n;
    out[5] = in[5] | n;
    out[6] = in[6] | n;
    out[7] = in[7] | n;

    return out;
}

qint64 Q64_BitxorShort(const qint64 *in, short n)
{
    qint64 out;

    out[0] = in[0] ^ n;
    out[1] = in[1] ^ n;
    out[2] = in[2] ^ n;
    out[3] = in[3] ^ n;
    out[4] = in[4] ^ n;
    out[5] = in[5] ^ n;
    out[6] = in[6] ^ n;
    out[7] = in[7] ^ n;

    return out;
}

qint64 Q64_AddShort(const qint64 *in, short n)
{
    qint64 out;

    out[0] = in.s[0] + n;
    out[1] = in.s[1] + n;

    return out;
}

qint64 Q64_DivideQ64(const qint64 *in, const qint64 *n)
{
    qint64 out;

    out.b[0] = in.b[0] / n.b[0];
    out.b[1] = in.b[1] / n.b[1];
    out.b[2] = in.b[2] / n.b[2];
    out.b[3] = in.b[3] / n.b[3];
    out.b[4] = in.b[4] / n.b[4];
    
    return out;
}

qint64 Q64_MultiplyQ64(const qint64 *in, const qint64 *n)
{
    qint64 out;

    out.i[0] = in.i[0] * n.i[0];
    out.i[1] = in.i[1] * n.i[1];

    return out;
}

qint64 Q64_SubtractQ64(const qint64 *in, const qint64 *n)
{
    qint64 out;

    out.i[0] = in.i[0] - n.i[0];
    out.i[1] = in.i[1] - n.i[1];

    return out;
}

qint64 Q64_ModQ64(const qint64 *in, const qint64 *n)
{
    qint64 out;

    out.i[0] = in.i[0] % n.i[0];
    out.i[1] = in.i[1] % n.i[1];

    return out;
}
