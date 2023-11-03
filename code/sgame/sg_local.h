#ifndef _SG_LOCAL_
#define _SG_LOCAL_

#pragma once

#include "../engine/n_shared.h"
#include "../game/g_game.h"
#include "../rendercommon/r_types.h"
#include "../rendercommon/r_public.h"
#include "sg_public.h"

// everything is globally or statically allocated within the vm, unless its using the G_AllocMem stuff, but the vm doesn't like it
// (reasonably, raw pointers + vm bytecode = exploits) when you pass pointers back and forth from the vm and native bytecode, so non of that'll happen
#define MAX_PLAYR_COUNT 10
#define MAX_MOBS_ACTIVE 150
#define MAX_PLAYR_INVENTORY 20

void GDR_DECL SG_Error(const char *fmt, ...) GDR_ATTRIBUTE((format(printf, 1, 2)));
void GDR_DECL SG_Printf(const char *fmt, ...) GDR_ATTRIBUTE((format(printf, 1, 2)));

void* SG_AllocMem(int size);
void SG_ClearMem(void);
void SG_FreeMem(void *ptr);
void SG_InitMem(void);

enum
{
    // ignore the EF_SOLID flag if entity has this one, cheat code
    EF_NOCLIP = 1,
    // merc/mob specific, deaf flag, cheat code
    EF_DEAF = 2,
    // merc/mob specific, blind flag, cheat code
    EF_BLIND = 4,
    // if it can and WILL be blocked, this applies to all attacks, yes ALL ATTACKS
    EF_PROJECTILE = 8,
    // self explanatory
    EF_DEAD = 16,
    // static and/or destructable environment piece
    EF_SOLID = 32,
    // mob/merc specific, entity cannot move
    EF_SENTRY = 64,
    // iframes.
    EF_INVUL = 128,
    // can be false for anything that we don't want dead just yet
    EF_KILLABLE = 256,
    // mob/merc specific, buffed stats
    EF_LEADER = 528,
};

typedef enum
{
    ST_NULL,
} statenum_t;

typedef struct sgentity_s sgentity_t;
typedef void (*actionp_t)(sgentity_t *);
typedef struct
{
    statenum_t id;
    statenum_t next, prev;
    int duration; // number of 'frames' it takes for the state to complete
    int sprite; // used for bff texture lookups
    actionp_t think; // if null, just wait until ticcount == 0
} state_t;

struct sgentity_s
{
    void *e; // entity-specific data

    vec2_t pos;
    vec2_t thrust;
    dirtype_t facing;
    state_t state;

    int flags; // general flags
    int ticker; // the count of how many frames into a state the entity is

    struct sgentity_s* target; // only really applies to mobs and homing attacks
};

//===============================================

//
// system traps
// These functions are how the cgame communicates with the main game system
//

// print a message to the console
void trap_Print(const char *fmt);

// abort the vm
void trap_Error(const char *fmt);


// milliseconds should only be used for performance tuning, never
// for anything game related
uint32_t trap_Milliseconds( void );

// console variable interaction
void trap_Cvar_Register( vmCvar_t *vmCvar, const char *varName, const char *defaultValue, uint32_t flags );
void trap_Cvar_Update( vmCvar_t *vmCvar );
void trap_Cvar_Set( const char *var_name, const char *value );
void trap_Cvar_VariableStringBuffer( const char *var_name, char *buffer, uint32_t bufsize );

// ConsoleCommand parameter access
uint32_t trap_Argc( void );
void trap_Argv( uint32_t n, char *buffer, uint32_t bufferLength );
void trap_Args( char *buffer, uint32_t bufferLength );

// filesystem access
file_t trap_FS_FOpenRead(const char *npath, file_t *f);
file_t trap_FS_FOpenWrite(const char *npath, file_t *f);
uint32_t trap_FS_FileLength(file_t f);
uint32_t trap_FS_Write(const void *buffer, uint32_t len, file_t f);
uint32_t trap_FS_Read(void *buffer, uint32_t len, file_t f);
fileOffset_t trap_FS_FileSeek(file_t f, fileOffset_t offset, uint32_t whence);
fileOffset_t trap_FS_FileTell(file_t f);
void trap_FS_FClose(file_t f);

// register a command name so the console can perform command completion.
void trap_AddCommand( const char *cmdName );

// register a sound effect
sfxHandle_t trap_Snd_RegisterSfx(const char *npath);

// queue a sound effect
void trap_Snd_PlaySfx(sfxHandle_t sfx);

// force stop an sfx
void trap_Snd_StopSfx(sfxHandle_t sfx);

// register a shader (technically a texture)
nhandle_t trap_RE_RegisterShader(const char *npath);

uint32_t trap_Key_GetCatcher(void);
void trap_Key_SetCatcher(uint32_t catcher);
uint32_t trap_Key_GetKey(const char *binding);
qboolean trap_Key_IsDown(uint32_t keynum);

// drawing functions
void trap_RE_AddPolyToScene( nhandle_t hShader, const polyVert_t *verts, uint32_t numVerts );
void trap_RE_AddPolyListToScene( const poly_t *polys, uint32_t numPolys );
void trap_RE_ClearScene( void );
void trap_RE_RenderScene( const renderSceneRef_t *fd );

// set the rendering color
void trap_RE_SetColor(const float *rgba);

// attains to the current gamestate
void trap_GetGameState(gamestate_t *state);

#endif
