#ifndef _NOMADLIB_
#define _NOMADLIB_

#pragma once

void Con_Printf(const char *fmt, ...);
void Con_Error(const char *fmt, ...);
void Con_Flush(void);

// ONLY MEANT FOR TEMPORARY (scope-based) ALLOCATIONS
void G_InitMem(void);
void* G_AllocMem(int size);
void G_FreeMem(void *ptr);
void G_ClearMem(void);

void N_memcpy(void *dst, const void *src, size_t count);
void* N_memset(void *dst, int fill, size_t count);

float Q_root(float x);
float Q_rsqrt(float number);

#endif
