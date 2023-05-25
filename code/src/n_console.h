#ifndef _N_CONSOLE_
#define _N_CONSOLE_

#pragma once

typedef enum
{
    TYPE_INT,
    TYPE_STRING,
    TYPE_FLOAT,
    TYPE_BOOL
} cvartype_t;

typedef struct cvar_s
{
    char *name;
    char *value;
    cvartype_t type;
    qboolean save; // whether this should be saved to the config file

    struct cvar_s* next = NULL;
} cvar_t;

void G_Printf(const char *fmt);
void G_Error(const char *fmt);
void Con_Printf(const char *fmt, ...);
void Con_Error(const char *fmt, ...);
void Con_Flush(void);

#endif