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

enum
{
    CVAR_ROM, // read-only access from console
    CVAR_USER_CREATED, // created from the console
    CVAR_VM_CREATED, // initialized from the vm
    CVAR_DEV, // can only be modified in dev mode (c_devmode == true)
    CVAR_SAVE, // will be written to upon app exit and read from upon initialization
    CVAR_CHEAT, // is it a cheat?
};

typedef struct cvar_s
{
#ifndef Q3_VM
    char name[64]={0};
    char s[64]={0};
#else
    char name[64];
    char s[64];
#endif
    float f;
    int32_t i;
    qboolean b;
    cvartype_t type;
    int32_t flags;

    struct cvar_s* next;
} cvar_t;

cvar_t* Cvar_Find(const char *name);
void Cvar_RegisterName(const char *name, const char *value, cvartype_t type, int32_t flags);
void Cvar_ChangeValue(const char *name, const char *value);
void Cvar_Register(cvar_t *cvar, const char *value);
qboolean Cvar_Command(void);
const char* Cvar_GetValue(const char *name);

#ifndef Q3_VM

typedef enum {
    DEV = 0,
    DEBUG,
    INFO,
    
    NONE // reserved for Con_Printf without the level specified, don't use
} loglevel_t;

#define MAX_MSG_SIZE (4*1024)
#define MAX_BUFFER_SIZE (10*1024)

extern bool imgui_window_open;

void Con_Init(void);
void Con_EndFrame(void);
void GDR_DECL Con_Printf(loglevel_t level, const char *fmt, ...);
void GDR_DECL Con_Printf(const char *fmt, ...);
void GDR_DECL Con_Error(const char *fmt, ...);
#endif

#endif

