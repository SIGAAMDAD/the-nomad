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
