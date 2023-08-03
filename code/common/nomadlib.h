#ifndef _NOMADLIB_
#define _NOMADLIB_

#pragma once

void VM_Com_Printf();

// ONLY MEANT FOR TEMPORARY (scope-based) ALLOCATIONS
void G_InitMem(void);
void* G_AllocMem(int size);
void G_FreeMem(void *ptr);
void G_ClearMem(void);

void N_memcpy(void *dst, const void *src, size_t count);
void* N_memset(void *dst, int fill, size_t count);

float Q_root(float x);
float Q_rsqrt(float number);

// quake vm only stuff, because its x86 (32-bit)
typedef union {
    int i[2];
    short s[4];
    char b[8];
} qint64;

typedef union {
    float f[2];
    char b[8];
} qdouble;

qint64 Q64_AddInt();
qint64 Q64_

#endif
