#include "../engine/n_shared.h"
#include "sg_local.h"

void SG_Init( void );
void SG_Shutdown( void );
int SG_RunLoop( int levelTime, int frameTime );
int SG_DrawFrame( void );

void SaveGame( void );
void LoadGame( void );

/*
vmMain

this is the only way control passes into the module.
this must be the very first function compiled into the .qvm file
*/
int vmMain( int command, int arg0, int arg1, int arg2, int arg3, int arg4, int arg5, int arg6, int arg7,
    int arg8, int arg9, int arg10 )
{
    switch ( command ) {
    case SGAME_INIT:
        SG_Init();
        return 0;
    case SGAME_SHUTDOWN:
        SG_Shutdown();
        return 0;
    case SGAME_GET_STATE:
        return sg.state;
    case SGAME_ENDLEVEL:
        return SG_EndLevel();
    case SGAME_LOAD_GAME:
        LoadGame();
        return 0;
    case SGAME_SAVE_GAME:
        SaveGame();
        return 0;
    case SGAME_MOUSE_EVENT:
        return 0;
    case SGAME_KEY_EVENT:
        SG_KeyEvent( arg0, arg1 );
        return 0;
    case SGAME_EVENT_HANDLING:
    case SGAME_EVENT_NONE:
        return 0;
    case SGAME_LOADLEVEL:
        return SG_StartLevel();
    case SGAME_CONSOLE_COMMAND:
        SGameCommand();
        return 0;
    case SGAME_RUNTIC:
        return SG_RunLoop( arg0, arg1 );
    default:
        break;
    };

    SG_Error( "vmMain: unrecognized command %i", command );
    return -1;
}

sgGlobals_t sg;

vmCvar_t sg_printEntities;
vmCvar_t sg_debugPrint;
vmCvar_t sg_paused;
vmCvar_t sg_mouseInvert;
vmCvar_t sg_mouseAcceleration;
vmCvar_t sg_printLevelStats;
vmCvar_t sg_decalDetail;
vmCvar_t sg_gibs;
vmCvar_t sg_levelIndex;
vmCvar_t sg_savename;
vmCvar_t sg_numSaves;
vmCvar_t sg_memoryDebug;

vmCvar_t pm_groundFriction;
vmCvar_t pm_waterFriction;
vmCvar_t pm_airFriction;
vmCvar_t pm_waterAccel;
vmCvar_t pm_baseAccel;
vmCvar_t pm_baseSpeed;
vmCvar_t pm_airAccel;
vmCvar_t pm_wallrunAccelVertical;
vmCvar_t pm_wallrunAccelMove;
vmCvar_t pm_wallTime;

typedef struct {
    vmCvar_t *vmCvar;
    const char *cvarName;
    const char *defaultValue;
    int cvarFlags;
    int modificationCount; // for tracking changes
    qboolean trackChange;       // track this variable, and announce if changed
} cvarTable_t;

static cvarTable_t cvarTable[] = {
    // noset vars
    { NULL,                     "gamename",             GLN_VERSION,    CVAR_ROM,                   0, qfalse },
    { NULL,                     "gamedate",             __DATE__,       CVAR_ROM,                   0, qfalse },
    { &sg_printEntities,        "sg_printEntities",     "0",            0,                          0, qfalse },
    { &sg_debugPrint,           "sg_debugPrint",        "1",            CVAR_TEMP,                  0, qfalse },
    { &sg_paused,               "g_paused",             "1",            CVAR_TEMP | CVAR_LATCH,     0, qfalse },
    { &pm_groundFriction,       "pm_groundFriction",    "0.6f",         CVAR_LATCH | CVAR_SAVE,     0, qtrue },
    { &pm_waterFriction,        "pm_waterFriction",     "0.06f",        CVAR_LATCH | CVAR_SAVE,     0, qtrue },
    { &pm_airFriction,          "pm_airFriction",       "0.01f",        CVAR_LATCH | CVAR_SAVE,     0, qtrue },
    { &pm_airAccel,             "pm_airAccel",          "1.5f",         CVAR_LATCH | CVAR_SAVE,     0, qfalse },
    { &pm_waterAccel,           "pm_waterAccel",        "0.5f",         CVAR_LATCH | CVAR_SAVE,     0, qfalse },
    { &pm_baseAccel,            "pm_baseAccel",         "1.0f",         CVAR_LATCH | CVAR_SAVE,     0, qfalse },
    { &pm_baseSpeed,            "pm_baseSpeed",         "0.02f",        CVAR_LATCH | CVAR_SAVE,     0, qfalse },
    { &sg_mouseInvert,          "g_mouseInvert",        "0",            CVAR_LATCH | CVAR_SAVE,     0, qtrue },
    { &sg_mouseAcceleration,    "g_mouseAcceleration",  "0",            CVAR_LATCH | CVAR_SAVE,     0, qtrue },
    { &sg_printLevelStats,      "sg_printLevelStats",   "1",            CVAR_LATCH | CVAR_SAVE,     0, qfalse },
    { &sg_decalDetail,          "sg_decalDetail",       "3",            CVAR_LATCH | CVAR_SAVE,     0, qtrue },
    { &sg_gibs,                 "sg_gibs",              "0",            CVAR_LATCH | CVAR_SAVE,     0, qtrue },
    { &sg_savename,             "sg_savename",          "savedata",     CVAR_LATCH | CVAR_SAVE,     0, qtrue },
    { &sg_numSaves,             "sg_numSaves",          "0",            CVAR_LATCH | CVAR_SAVE,     0, qfalse },
#ifdef _NOMAD_DEBUG
    { &sg_memoryDebug,          "sg_memoryDebug",       "1",            CVAR_LATCH | CVAR_TEMP,     0, qfalse },
#else
    { &sg_memoryDebug,          "sg_memoryDebug",       "0",            CVAR_LATCH | CVAR_TEMP,     0, qfalse },
#endif
};

static const int cvarTableSize = arraylen(cvarTable);

static void SG_RegisterCvars( void )
{
    int i;
    cvarTable_t *cv;

    for ( i = 0, cv = cvarTable; i < cvarTableSize; i++, cv++ ) {
        Cvar_Register( cv->vmCvar, cv->cvarName, cv->defaultValue, cv->cvarFlags );
        if ( cv->vmCvar ) {
            cv->modificationCount = cv->vmCvar->modificationCount;
        }
    }
}

void SG_UpdateCvars( void )
{
    int i;
    cvarTable_t *cv;

    for ( i = 0, cv = cvarTable; i < cvarTableSize; i++, cv++ ) {
        if ( cv->vmCvar ) {
            Cvar_Update( cv->vmCvar );

            if ( cv->modificationCount != cv->vmCvar->modificationCount ) {
                cv->modificationCount = cv->vmCvar->modificationCount;

                if ( cv->trackChange ) {
                    trap_SendConsoleCommand( va( "Changed \"%s\" to \"%s\"", cv->cvarName, cv->vmCvar->s ) );
                }
            }
        }
    }
}

void GDR_ATTRIBUTE((format(printf, 1, 2))) GDR_DECL G_Printf(const char *fmt, ...)
{
    va_list argptr;
    char msg[4096];
    int length;

    va_start(argptr, fmt);
    length = vsprintf(msg, fmt, argptr);
    va_end(argptr);

    trap_Print(msg);
}

void GDR_ATTRIBUTE((format(printf, 1, 2))) GDR_DECL G_Error(const char *err, ...)
{
    va_list argptr;
    char msg[4096];
    int length;

    va_start(argptr, err);
    length = vsprintf(msg, err, argptr);
    va_end(argptr);

    trap_Error(msg);
}

void GDR_ATTRIBUTE((format(printf, 1, 2))) GDR_DECL SG_Printf(const char *fmt, ...)
{
    va_list argptr;
    char msg[4096];
    int length;

    va_start(argptr, fmt);
    length = vsprintf(msg, fmt, argptr);
    va_end(argptr);

    if (length >= sizeof(msg)) {
        trap_Error( "SG_Printf: buffer overflow" );
    }

    trap_Print(msg);
}

void GDR_ATTRIBUTE((format(printf, 1, 2))) GDR_DECL SG_Error(const char *err, ...)
{
    va_list argptr;
    char msg[4096];
    int length;

    va_start(argptr, err);
    length = vsprintf(msg, err, argptr);
    va_end(argptr);

    if (length >= sizeof(msg)) {
        trap_Error( "SG_Printf: buffer overflow" );
    }

    trap_Error(msg);
}

void GDR_DECL GDR_ATTRIBUTE((format(printf, 2, 3))) N_Error(errorCode_t code, const char *err, ...)
{
    va_list argptr;
    char msg[4096];
    int length;

    va_start(argptr, err);
    length = vsprintf(msg, err, argptr);
    va_end(argptr);

    if (length >= sizeof(msg)) {
        trap_Error( "SG_Printf: buffer overflow" );
    }

    SG_Error("%s", msg);
}

//#ifndef SGAME_HARD_LINKED
// this is only here so the functions in n_shared.c and bg_*.c can link

void GDR_DECL GDR_ATTRIBUTE((format(printf, 1, 2))) Con_Printf(const char *fmt, ...)
{
    va_list argptr;
    char msg[4096];
    int length;

    va_start(argptr, fmt);
    length = vsprintf(msg, fmt, argptr);
    va_end(argptr);

    if (length >= sizeof(msg)) {
        trap_Error( "SG_Printf: buffer overflow" );
    }

    SG_Printf("%s", msg);
}

//#endif

int SG_RunLoop( int levelTime, int frameTime )
{
    int i;
    int start, end;
    int msec;
    sgentity_t *ent;

    if ( sg.state == SG_INACTIVE ) {
        return 0;
    }

    // get any cvar changes
    SG_UpdateCvars();

    if ( sg_paused.i ) {
        return 0;
    }

    sg.framenum++;
    sg.previousTime = sg.framenum;
    sg.levelTime = levelTime;
    msec = sg.levelTime - sg.previousTime;

    // build player's movement command
//    SG_BuildMoveCommand();

    //
    // go through all allocated entities
    //
    start = trap_Milliseconds();
    ent = &sg_entities[0];
    for ( i = 0; i < sg.numEntities; i++) {
        if ( !ent->health ) {
            continue;
        }

        ent->ticker--;

        if ( ent->ticker <= -1 ) {
            Ent_SetState( ent, ent->state->nextstate );
            continue;
        }

        // update the current entity's animation frame
        if ( ent->state->frames > 0 ) {
            if ( ent->frame == ent->state->frames ) {
                ent->frame = 0; // reset animation
            }
            else if ( ent->ticker % ent->state->frames ) {
                ent->frame++;
            }
        }

        ent->state->action.acp1( ent );
    }
    end = trap_Milliseconds();

    SG_DrawFrame();

    if ( sg_printEntities.i ) {
        for ( i = 0; i < sg.numEntities; i++ ) {
            G_Printf( "%4i: %s\n", i, sg_entities[i].classname );
        }
        Cvar_Set( "sg_printEntities", "0" );
    }

    return 1;
}


static void SG_LoadMedia( void )
{
    sg.media.player_death0 = trap_Snd_RegisterSfx( "sfx/player/death1.wav" );
    sg.media.player_death1 = trap_Snd_RegisterSfx( "sfx/player/death2.wav" );
    sg.media.player_death2 = trap_Snd_RegisterSfx( "sfx/player/death3.wav" ); 
    sg.media.player_pain0 = trap_Snd_RegisterSfx( "sfx/player/pain0.wav" );
    sg.media.player_pain1 = trap_Snd_RegisterSfx( "sfx/player/pain1.wav" );
    sg.media.player_pain2 = trap_Snd_RegisterSfx( "sfx/player/pain2.wav" );
    sg.media.revolver_fire = trap_Snd_RegisterSfx( "sfx/weapons/revolver_fire.wav" );
    sg.media.revolver_rld = trap_Snd_RegisterSfx( "sfx/weapons/revolver_rld.wav" );

    sg.media.raio_shader = RE_RegisterShader( "sprites/glnomad_raio_base" );
    sg.media.grunt_shader = RE_RegisterShader( "sprites/glnomad_grunt" );

    sg.media.raio_sprites = RE_RegisterSpriteSheet( "sprites/glnomad_raio_base", 512, 512, 32, 32 );
}

void SG_Init( void )
{
    G_Printf( "---------- Game Initialization ----------\n" );
    G_Printf( "gamename: %s\n", GLN_VERSION );
    G_Printf( "gamedate: %s\n", __DATE__ );

    trap_Key_SetCatcher( trap_Key_GetCatcher() | KEYCATCH_SGAME );

    // clear sgame state
    memset( &sg, 0, sizeof(sg) );
    
    // cache redundant calculations
    Sys_GetGPUConfig( &sg.gpuConfig );

    // for 1024x768 virtualized screen
	sg.scale = sg.gpuConfig.vidHeight * (1.0/768.0);
	if ( sg.gpuConfig.vidWidth * 768 > sg.gpuConfig.vidHeight * 1024 ) {
		// wide screen
		sg.bias = 0.5 * ( sg.gpuConfig.vidWidth - ( sg.gpuConfig.vidHeight * (1024.0/768.0) ) );
	}
	else {
		// no wide screen
		sg.bias = 0;
	}

    // register sgame cvars
    SG_RegisterCvars();

    // load assets/resources
    SG_LoadMedia();

    SG_MemInit();

    sg.state = SG_INACTIVE;

    G_Printf( "-----------------------------------\n" );
}

void SG_Shutdown( void )
{
    G_Printf( "Shutting down sgame...\n" );

    memset( &sg, 0, sizeof(sg) );

    sg.state = SG_INACTIVE;
}

void SaveGame( void )
{
    G_Printf( "Saving game...\n" );

    SG_SaveLevelData();

    G_Printf( "Done" );
}

void LoadGame( void )
{
    char savename[MAX_NPATH];

    Cvar_VariableStringBuffer( "sg_savename", savename, sizeof(savename) );

    G_Printf( "Loading save file '%s'...\n", savename );
}

void GDR_ATTRIBUTE((format(printf, 2, 3))) GDR_DECL trap_FS_Printf( file_t f, const char *fmt, ... )
{
    va_list argptr;
    char msg[MAXPRINTMSG];

    va_start( argptr, fmt );
    vsprintf( msg, fmt, argptr );
    va_end( argptr );

    trap_FS_Write( msg, strlen( msg ), f );
}


const vec3_t dirvectors[NUMDIRS] = {
    { -1.0f, -1.0f, 0.0f },
    {  0.0f, -1.0f, 0.0f },
    {  1.0f, -1.0f, 0.0f },
    {  1.0f,  0.0f, 0.0f },
    {  1.0f,  1.0f, 0.0f },
    {  0.0f,  1.0f, 0.0f },
    { -1.0f,  1.0f, 0.0f },
    { -1.0f,  0.0f, 0.0f }
};

const dirtype_t inversedirs[NUMDIRS] = {
    DIR_SOUTH_EAST,
    DIR_SOUTH,
    DIR_SOUTH_WEST,
    DIR_WEST,
    DIR_NORTH_WEST,
    DIR_NORTH,
    DIR_NORTH_EAST,
    DIR_EAST
};
