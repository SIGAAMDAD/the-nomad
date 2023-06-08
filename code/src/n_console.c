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

cvar_t* Cvar_Find(const char *name)
{
    cvar_t* cvar;

    for (cvar = cvar_list; cvar; cvar = cvar->next) {
        if (N_strcmp(name, cvar->name)) {
            return cvar;
        }
    }
    return NULL;
}