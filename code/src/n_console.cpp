#include "n_shared.h"
#include "n_console.h"

static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

typedef enum {
    DEV = 0, // vm shouldn't really be using this...
    INFO,
    WARN,
} loglevel_t;

static thread_local va_list con_argptr;
static thread_local va_list com_argptr;

void Con_Printf(int level, const char *fmt, ...);
void Con_Error(int level, const char *fmt, ...);

void Con_Printf(loglevel_t level, const char *fmt, ...)
{
    switch (level) {
    case DEV:
        fprintf(stdout, "DEV: ");
        break;
    case WARN:
        fprintf(stdout, "WARNING: ");
        break;
    default: break;
    };
    char buffer[1024];

    va_list argptr;
    va_start(argptr, fmt);
    stbsp_vsnprintf(buffer, sizeof(buffer) - 1, fmt, argptr);
    va_end(argptr);

    G_Printf(buffer);
}

void Con_Printf(const char *fmt, ...)
{
    va_start(con_argptr, fmt);
    vfprintf(stdout, fmt, con_argptr);
    va_end(con_argptr);
    fprintf(stdout, "\n");
}
void Con_Error(const char *fmt, ...)
{
    fprintf(stderr, "Error: ");
    va_start(con_argptr, fmt);
    vfprintf(stderr, fmt, con_argptr);
    va_end(con_argptr);
    fprintf(stderr, "\n");
}
void Con_Flush(void)
{
    fflush(stdout);
    fflush(stderr);
}
void G_Printf(const char *fmt)
{
    fprintf(stdout, "%s", fmt);
}
void G_Error(const char *fmt)
{
    fprintf(stderr, "%s", fmt);
}
#if 0
void Com_Printf(const char *fmt, ...)
{
    va_start(com_argptr, fmt);
    vfprintf(stdout, fmt, com_argptr);
    va_end(com_argptr);
}
void Com_Error(const char *fmt, ...)
{
    va_start(com_argptr, fmt);
    vfprintf(stderr, fmt, com_argptr);
    va_end(com_argptr);
}
#endif