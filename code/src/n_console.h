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
    char name[64];
    char *s;
    float f;
    int32_t i;
    qboolean b;
    cvartype_t type;
    qboolean save; // whether this should be saved to the config file

    struct cvar_s* next;
} cvar_t;

cvar_t* Cvar_Find(const char *name);
void Cvar_RegisterName(const char *name, const char *value, cvartype_t type, qboolean save);
void Cvar_ChangeValue(const char *name, const char *value);
void Cvar_Register(cvar_t *cvar);
qboolean Cvar_Command(void);
const char* Cvar_GetValue(const char *name);

#ifndef Q3_VM

typedef enum {
    DEV = 0,
    DEBUG,
    
    NONE // reserved for Con_Printf without the level specified, don't use
} loglevel_t;

#define MAX_MSG_SIZE (4*1024)
#define MAX_BUFFER_SIZE (10*1024)

extern bool imgui_window_open;

void Con_EndFrame(void);
void Con_Printf(loglevel_t level, const char *fmt, ...);
void Con_Printf(const char *fmt, ...);
void Con_Error(const char *fmt, ...);
#endif

#endif

