#include "n_shared.h"

void Con_Printf(const char *fmt, ...)
{
    va_list argptr;
    va_start(argptr, fmt);
    vfprintf(stdout, fmt, argptr);
    va_end(argptr);
}

void Con_Error(const char *fmt, ...)
{
    fprintf(stderr, "Error: ");
    va_list argptr;
    va_start(argptr, fmt);
    vfprintf(stderr, fmt, argptr);
    va_end(argptr);
}

void Con_Flush(void)
{
    fflush(stdout);
    fflush(stderr);
}