#include "n_shared.h"

static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void Con_Printf(loglevel_t level, const char *fmt, ...)
{
    if (level == DEV) {
        fprintf(stdout, "DEV: ");
    }
    else if (level == DEBUG) {
#ifdef _NOMAD_DEBUG
        fprintf(stdout, "DEBUG: ");
#else
        return;
#endif
    }

    va_list argptr;
    
    va_start(argptr, fmt);
    vfprintf(stdout, fmt, argptr);
    va_end(argptr);
    fprintf(stdout, "\n");
}

void Con_Printf(const char *fmt, ...)
{
    va_list argptr;

    va_start(argptr, fmt);
    vfprintf(stdout, fmt, argptr);
    va_end(argptr);

    fprintf(stdout, "\n");
    fflush(stdout);
}

void Con_Error(const char *fmt, ...)
{
    va_list argptr;

    fprintf(stderr,"ERROR: ");
    va_start(argptr, fmt);
    vfprintf(stderr, fmt, argptr);
    va_end(argptr);
    fprintf(stderr, "\n");
    fflush(stderr);
}