#ifndef _N_CONSOLE_
#define _N_CONSOLE_

#pragma once

/*
==========================================================

CVARS (console variables)

Many variables can be used for cheating purposes, so when
cheats is zero, force all unspecified variables to their
default values.
==========================================================
*/

typedef int cvartype_t;

enum
{
    CVT_NONE = 0,
    CVT_INT,
    CVT_STRING,
    CVT_FLOAT,
    CVT_BOOL,
	CVT_FSPATH,

    CVT_MAX
};

typedef enum {
	CVG_VM,
	CVG_ENGINE,
	CVG_RENDERER,
	CVG_ALLOCATOR,
	CVG_SOUND,
	CVG_FILESYSTEM,
    CVG_NONE,

    CVG_MAX
} cvarGroup_t;

// cvar flags
#define	CVAR_SAVE		0x0001	// set to cause it to be saved to default.cfg
					// used for system variables, not for player
					// specific configurations
#define	CVAR_USERINFO		0x0002	// sent to server on connect or change
#define	CVAR_SERVERINFO		0x0004	// sent in response to front end requests
#define	CVAR_SYSTEMINFO		0x0008	// these cvars will be duplicated on all clients
#define	CVAR_INIT			0x0010	// don't allow change from console at all,
					// but can be set from the command line
#define	CVAR_LATCH			0x0020	// will only change when C code next does
					// a Cvar_Get(), so it can't be changed
					// without proper initialization.  modified
					// will be set, even though the value hasn't
					// changed yet
#define	CVAR_ROM			0x0040	// display only, cannot be set by user at all
#define	CVAR_USER_CREATED	0x0080	// created by a set command
#define	CVAR_TEMP			0x0100	// can be set even when cheats are disabled, but is not archived
#define CVAR_CHEAT			0x0200	// can not be changed if cheats are disabled
#define CVAR_NORESTART		0x0400	// do not clear when a cvar_restart is issued

#define CVAR_SERVER_CREATED	0x0800	// cvar was created by a server the client connected to.
#define CVAR_VM_CREATED		0x1000	// cvar was created exclusively in one of the VMs.
#define CVAR_PROTECTED		0x2000	// prevent modifying this var from VMs or the server

#define CVAR_NODEFAULT		0x4000	// do not write to config if matching with default value

#define CVAR_PRIVATE		0x8000	// can't be read from VM

#define CVAR_DEV		0x10000 // can be set only in developer mode
#define CVAR_NOTABCOMPLETE	0x20000 // no tab completion in console

#define CVAR_ARCHIVE_ND		(CVAR_SAVE | CVAR_NODEFAULT)

// These flags are only returned by the Cvar_Flags() function
#define CVAR_MODIFIED		0x40000000	// Cvar was modified
#define CVAR_NONEXISTENT	0x80000000	// Cvar doesn't exist.

typedef int cvarHandle_t;

#define MAX_CVAR_NAME 64
#define MAX_CVAR_VALUE 256

#define CVAR_INVALID_HANDLE -1

typedef struct cvar_s
{
    char		*name;
	char		*s;
	char		*resetString;		// cvar_restart will reset to this value
	char		*latchedString;		// for CVAR_LATCH vars
	uint32_t	flags;
	qboolean	modified;			// set each time the cvar is changed
	uint32_t	modificationCount;	// incremented each time the cvar is changed
	float		f;  				// Q_atof( string )
	int32_t		i;      			// atoi( string )
	cvartype_t  type;
	char		*mins;
	char		*maxs;
	char		*description;

	struct cvar_s *next;
	struct cvar_s *prev;
	struct cvar_s *hashNext;
	struct cvar_s *hashPrev;
	uint64_t	hashIndex;
	cvarGroup_t	group;				// to track changes
} cvar_t;

typedef struct {
    char s[MAX_CVAR_VALUE];
    float f;
    int i;
    qboolean b;

    unsigned int modificationCount;
    cvarHandle_t handle;
} vmCvar_t;

void Cvar_ResetGroup( cvarGroup_t group, qboolean resetModifiedFlags );
int Cvar_CheckGroup(cvarGroup_t group);
void Cvar_ForceReset(const char *name);
void Cvar_Init(void);
void Cvar_Restart(qboolean unsetVM);
void Cvar_Register(vmCvar_t *vmCvar, const char *varName, const char *defaultValue, uint32_t flags, uint32_t privateFlag);
void Cvar_CompleteCvarName(const char *args, uint32_t argNum);
void Cvar_CommandCompletion( void (*callback)(const char *s) );
cvar_t *Cvar_Set2(const char *var_name, const char *value, qboolean force);
void Cvar_VariableStringBuffer(const char *name, char *buffer, uint64_t bufferSize);
void Cvar_VariableStringBufferSafe(const char *name, char *buffer, uint64_t bufferSize, uint32_t flag);
int32_t Cvar_VariableInteger(const char *name);
float Cvar_VariableFloat(const char *name);
qboolean Cvar_VariableBoolean(const char *name);
const char *Cvar_VariableString(const char *name);
void Cvar_CheckRange(cvar_t *var, const char *mins, const char *maxs, cvartype_t type);
uint32_t Cvar_Flags(const char *name);
void Cvar_Update(vmCvar_t *vmCvar, uint32_t privateFlag);
cvar_t *Cvar_Get(const char *name, const char *value, uint32_t flags);
qboolean Cvar_Command(void);
void Cvar_Reset(const char *name);
void Cvar_SetGroup(cvar_t *cv, cvarGroup_t group);
void Cvar_SetDescription(cvar_t *cv, const char *description);
void Cvar_SetSafe(const char *name, const char *value);
void Cvar_Set(const char *name, const char *value);
void Cvar_SetValueSafe(const char *name, float value);
qboolean Cvar_SetModified(const char *name, qboolean modified);
void Cvar_SetIntegerValue(const char *name, int32_t value);
void Cvar_SetFloatValue(const char *name, float value);
void Cvar_SetStringValue(const char *name, const char *value);
void Cvar_SetBooleanValue(const char *name, qboolean value);

#define Q_COLOR_ESCAPE	'^'
#define Q_IsColorString(p) ( *(p) == Q_COLOR_ESCAPE && *((p)+1) && *((p)+1) != Q_COLOR_ESCAPE )

#define ColorIndex(c)	( ( (c) - '0' ) & 7 )

#define S_COLOR_BLACK		'0'
#define S_COLOR_RED			'1'
#define S_COLOR_GREEN		'2'
#define S_COLOR_YELLOW		'3'
#define S_COLOR_BLUE		'4'
#define S_COLOR_CYAN		'5'
#define S_COLOR_MAGENTA		'6'
#define S_COLOR_WHITE		'7'
#define S_COLOR_RESET		'8'

#define COLOR_BLACK		"^0"
#define COLOR_RED		"^1"
#define COLOR_GREEN		"^2"
#define COLOR_YELLOW	"^3"
#define COLOR_BLUE		"^4"
#define COLOR_CYAN		"^5"
#define COLOR_MAGENTA	"^6"
#define COLOR_WHITE		"^7"
#define COLOR_RESET		"^8"

extern const vec4_t	g_color_table[ 64 ];
extern int ColorIndexFromChar( char ccode );

#define	MAKERGB( v, r, g, b ) v[0]=r;v[1]=g;v[2]=b
#define	MAKERGBA( v, r, g, b, a ) v[0]=r;v[1]=g;v[2]=b;v[3]=a

void Con_AddText(const char *msg);
void Con_DrawConsole(void);
void Con_Init(void);
void Con_Shutdown(void);

void GDR_DECL Con_Printf(const char *fmt, ...) GDR_ATTRIBUTE((format(printf, 1, 2)));
void GDR_DECL Con_DPrintf(const char *fmt, ...) GDR_ATTRIBUTE((format(printf, 1, 2)));

#endif

