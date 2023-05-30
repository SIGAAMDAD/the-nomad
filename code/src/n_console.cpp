#include "n_shared.h"

static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void Con_Printf(loglevel_t level, const char *fmt, ...)
{
    // code error if this is thrown
    if (strlen(fmt) >= MAX_MSG_SIZE) {
        N_Error("Con_Printf: strlen(fmt) >= MAX_MSG_SIZE (%i)", MAX_MSG_SIZE);
    }

    va_list argptr;
    int length;
    char msg[MAX_MSG_SIZE];
    memset(msg, 0, sizeof(msg));

    va_start(argptr, fmt);
    length = vsnprintf(msg, sizeof(msg), fmt, argptr);
    va_end(argptr);

    if (length >= MAX_MSG_SIZE) {
        N_Error("Con_Printf: buffer overrun");
    }

    msg[length] = '\n';

    // dont buffer
    Sys_Print(msg);
}

void Con_Printf(const char *fmt, ...)
{
    if (strlen(fmt) >= MAX_MSG_SIZE) {
        N_Error("Con_Printf: strlen(fmt) >= MAX_MSG_SIZE (%i)", MAX_MSG_SIZE);
    }

    va_list argptr;
    int length;
    char msg[MAX_MSG_SIZE];

    va_start(argptr, fmt);
    length = vsnprintf(msg, sizeof(msg), fmt, argptr);
    va_end(argptr);

    Con_Printf(NONE, "%s", msg);
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