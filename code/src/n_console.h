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

typedef enum {
	CVG_VM,
	CVG_ENGINE,
	CVG_RENDERER,
	CVG_ALLOCATOR
} cvarGroup_t;

// cvar flags
enum
{
    CVAR_ROM            = 0x100, // read-only access from console
    CVAR_USER_CREATED   = 0x200, // created from the console
    CVAR_VM_CREATED     = 0x300, // initialized from the vm
    CVAR_DEV            = 0x400, // can only be modified in dev mode (c_devmode == true)
    CVAR_SAVE           = 0x500, // will be written to upon app exit and read from upon initialization
    CVAR_CHEAT          = 0x600, // is it a cheat?
    CVAR_PRIVATE        = 0x700, // cannot be accessed by the vm
};

typedef int cvarHandle_t;

#define MAX_CVAR_NAME 64
#define MAX_CVAR_VALUE 256

#define CVAR_INVALID_HANDLE -1

#ifndef Q3_VM
typedef struct cvar_s
{
	char name[MAX_CVAR_NAME]={0};
	
	// cvar's data (can be modified outside of cvar functions)
	char s[MAX_CVAR_VALUE]={0};
    float f;
    int32_t i;
	qboolean b;
    
    cvartype_t type;
    cvarGroup_t group; // for tracking changes
    int32_t flags;

	char *description;

    qboolean modified;
    cvarHandle_t id;

	struct cvar_s *prev = NULL;
    struct cvar_s *next = NULL;
//	struct cvar_s *hashNext;
//	struct cvar_s *hashPrev;
} cvar_t;
#endif

typedef struct {
    char s[MAX_CVAR_VALUE];
    float f;
    int i;
    qboolean b;

    int modificationCount;
    cvarHandle_t handle;
} vmCvar_t;

#ifndef Q3_VM
cvar_t* Cvar_Find(const char *name);
void Cvar_SetGroup(cvar_t *cvar, cvarGroup_t group);
void Cvar_Register(cvar_t *cvar, const char *value);
qboolean Cvar_Command(void);
const char* Cvar_GetValue(const char *name);
const char* Cvar_Description(const char *name);
void Cvar_ChangeValue(cvar_t *cvar, const char *value);
#endif

void Cvar_Find(vmCvar_t *cvar, const char *name);
void Cvar_Register(vmCvar_t *cvar, const char *name, const char *value, cvartype_t type);
void Cvar_RegisterName(const char *name, const char *value, cvartype_t type, int32_t flags);
void Cvar_ChangeValue(const char *name, const char *value);

#ifndef Q3_VM

typedef enum {
    DEV = 0,
    INFO,
    DEBUG,
    ERROR,
    WARNING,
    
    NONE // reserved for Con_Printf without the level specified, don't use
} loglevel_t;

#define MAX_MSG_SIZE (1*1024*1024)
#define MAX_BUFFER_SIZE (3*1024*1024)

extern bool imgui_window_open;

void Cvar_Init(void);
void Con_Init(void);
void Con_Shutdown(void);
void GDR_DECL Con_Printf(loglevel_t level, const char *fmt, ...) GDR_ATTRIBUTE((format(printf, 2, 3)));
void GDR_DECL Con_Printf(const char *fmt, ...) GDR_ATTRIBUTE((format(printf, 1, 2)));
void GDR_DECL Con_Error(bool exit, const char *fmt, ...) GDR_ATTRIBUTE((format(printf, 2, 3)));
eastl::vector<char>& Con_GetBuffer(void);
#endif

#ifndef Q3_VM

// credits to michaelfm1211 for writing this
// use a SGR parameter not supported by colors.h. ex: alternate fonts
#define SGR_N(n) "\x1b["#n"m"

#define SGR_RESET "\x1b[m"
#define SGR_BOLD "\x1b[1m"
#define SGR_FAINT "\x1b[2m"
#define SGR_ITALIC "\x1b[3m"
#define SGR_UNDER "\x1b[4m"
#define SGR_BLINK "\x1b[5m"
#define SGR_BLINK_RAPID "\x1b[6m"
#define SGR_INVERT "\x1b[7m"
#define SGR_HIDE "\x1b[8m"
#define SGR_STRIKE "\x1b[9m"
#define SGR_DEF_FONT "\x1b[10m"
#define SGR_GOTHIC "\x1b[20m"
#define SGR_2UNDER "\x1b[21m"
#define SGR_DEF_WEIGHT "\x1b[22m"
#define SGR_NO_ITALIC "\x1b[23m"
#define SGR_NO_GOTHIC "\x1b[23m"
#define SGR_NO_UNDER "\x1b[24m"
#define SGR_NO_BLINK "\x1b[25m"
#define SGR_MONOSPACE "\x1b[26m"
#define SGR_NO_INVERT "\x1b[27m"
#define SGR_NO_HIDE "\x1b[28m"
#define SGR_NO_STRIKE "\x1b[29m"
// there's a bunch of color related SGRs. they're under the C_ instead, but
// everyting under here is already obscure anyway
#define SGR_NO_MONOSPACE "\x1b[50m"
#define SGR_FRAME "\x1b[51m"
#define SGR_CIRCLE "\x1b[52m"
#define SGR_OVER "\x1b[53m"
#define SGR_NO_FRAME "\x1b[54m"
#define SGR_NO_CIRCLE "\x1b[54m"
#define SGR_NO_OVER "\x1b[55m"
#define SGR_NO_FRAME "\x1b[54m"
#define SGR_IDEO_UNDER "\x1b[60m"
#define SGR_R_SIDE "\x1b[60m"
#define SGR_IDEO_2UNDER "\x1b[61m"
#define SGR_R_2SIDE "\x1b[61m"
#define SGR_IDEO_OVER "\x1b[62m"
#define SGR_L_SIDE "\x1b[62m"
#define SGR_IDEO_2OVER "\x1b[63m"
#define SGR_L_2SIDE "\x1b[63m"
#define SGR_IDEO_STRESS "\x1b[64m"
#define SGR_NO_IDEO "\x1b[65m"
#define SGR_NO_SIDE "\x1b[65m"
#define SGR_SUPER "\x1b[73m"
#define SGR_SUB "\x1b[74m"
#define SGR_NO_SUPER "\x1b[75m"
#define SGR_NO_SUB "\x1b[75m"


/*
 * Colors
 * Key:
 * C_{color} = text color
 * C_BR_{color} = bright text color
 * C_BG_{color} = background color
 * C_BG_BR_{color} = bright background color
 */

#define C_RESET "\x1b[0m"

#define C_DEF "\x1b[39m"
#define C_BG_DEF "\x1b[49m"

#define C_BLACK "\x1b[30m"
#define C_RED "\x1b[31m"
#define C_GREEN "\x1b[32m"
#define C_YELLOW "\x1b[33m"
#define C_BLUE "\x1b[34m"
#define C_MAGENTA "\x1b[35m"
#define C_CYAN "\x1b[36m"
#define C_WHITE "\x1b[37m"

#define C_BR_BLACK "\x1b[90m"
#define C_GRAY "\x1b[90m"
#define C_BR_RED "\x1b[91m"
#define C_BR_GREEN "\x1b[92m"
#define C_BR_YELLOW "\x1b[93m"
#define C_BR_BLUE "\x1b[94m"
#define C_BR_MAGENTA "\x1b[95m"
#define C_BR_CYAN "\x1b[96m"
#define C_BR_WHITE "\x1b[97m"

#define C_BG_BLACK "\x1b[40m"
#define C_BG_RED "\x1b[41m"
#define C_BG_GREEN "\x1b[42m"
#define C_BG_YELLOW "\x1b[43m"
#define C_BG_BLUE "\x1b[44m"
#define C_BG_MAGENTA "\x1b[45m"
#define C_BG_CYAN "\x1b[46m"
#define C_BG_WHITE "\x1b[47m"

#define C_BG_BR_BLACK "\x1b[100m"
#define C_BG_GRAY "\x1b[100m"
#define C_BG_BR_RED "\x1b[101m"
#define C_BG_BR_GREEN "\x1b[102m"
#define C_BG_BR_YELLOW "\x1b[103m"
#define C_BG_BR_BLUE "\x1b[104m"
#define C_BG_BR_MAGENTA "\x1b[105m"
#define C_BG_BR_CYAN "\x1b[106m"
#define C_BG_BR_WHITE "\x1b[107m"

// select from the 256-color table. see table at:
// https://en.wikipedia.org/wiki/ANSI_escape_code#8-bit
#define C_256(n) "\x1b[38;5;"#n"m"
#define C_BG_256(n) "\x1b[48;5;"#n"m"

// enter a RGB value
#define C_RGB(r, g, b) "\x1b[38;2;"#r";"#g";"#b"m"
#define C_BG_RGB(r, g, b) "\x1b[48;2;"#r";"#g";"#b"m"
#endif

#endif

