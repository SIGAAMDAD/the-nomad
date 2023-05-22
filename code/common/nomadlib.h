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

float Q_root(float x);
float Q_rsqrt(float number);

#endif
