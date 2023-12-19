#include "../engine/n_shared.h"
#include "sg_local.h"

void SG_Init( void );
void SG_Shutdown( void );
int32_t SG_RunLoop( int32_t msec );
int32_t SG_DrawFrame( void );

/*
vmMain

this is the only way control passes into the module.
this must be the very first function compiled into the .qvm file
*/
int32_t vmMain( int32_t command, int32_t arg0, int32_t arg1, int32_t arg2, int32_t arg3, int32_t arg4, int32_t arg5, int32_t arg6, int32_t arg7,
    int32_t arg8, int32_t arg9, int32_t arg10 )
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
    case SGAME_SEND_USER_CMD:
        SG_SendUserCmd( arg0, arg1, arg2, arg3 );
        return 0;
    case SGAME_EVENT_HANDLING:
    case SGAME_EVENT_NONE:
        return 0;
    case SGAME_LOADLEVEL:
        return SG_InitLevel( arg0 );
    case SGAME_CONSOLE_COMMAND:
        return 0;
    case SGAME_RUNTIC:
        return SG_RunLoop( arg0 );
    default:
        break;
    };

    SG_Error( "vmMain: unrecognized command %i", command );
    return -1;
}

sgGlobals_t sg;

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

vmCvar_t sg_debugPrint;
vmCvar_t sg_paused;
vmCvar_t sg_mouseInvert;
vmCvar_t sg_mouseAcceleration;
vmCvar_t sg_printLevelStats;
vmCvar_t sg_decalDetail;
vmCvar_t sg_gibs;
vmCvar_t sg_levelInfoFile;
vmCvar_t sg_levelIndex;
vmCvar_t sg_levelDataFile;
vmCvar_t sg_savename;
vmCvar_t sg_numSaves;
vmCvar_t pm_waterAccel;
vmCvar_t pm_baseAccel;
vmCvar_t pm_baseSpeed;
vmCvar_t pm_airAccel;
vmCvar_t pm_wallrunAccelVertical;
vmCvar_t pm_wallrunAccelMove;
vmCvar_t pm_wallTime;

typedef struct {
    const char *name;
    const char *defaultValue;
    vmCvar_t *cvar;
    uint32_t flags;
} cvarTable_t;

static cvarTable_t cvarTable[] = {
    { "sg_debugPrint",                  "0",            &sg_debugPrint,             CVAR_LATCH },
    { "g_paused",                       "1",            &sg_paused,                 CVAR_LATCH | CVAR_TEMP },
    { "pm_airAccel",                    "1.5",          &pm_airAccel,               CVAR_LATCH | CVAR_SAVE },
    { "pm_waterAccel",                  "0.5",          &pm_waterAccel,             CVAR_LATCH | CVAR_SAVE },
    { "pm_baseAccel",                   "1.2",          &pm_baseAccel,              CVAR_LATCH | CVAR_SAVE },
    { "pm_baseSpeed",                   "1.0",          &pm_baseSpeed,              CVAR_LATCH | CVAR_SAVE },
    { "g_mouseInvert",                  "0",            &sg_mouseInvert,            CVAR_LATCH | CVAR_SAVE },
    { "g_mouseAcceleration",            "0",            &sg_mouseAcceleration,      CVAR_LATCH | CVAR_SAVE },
    { "sg_printLevelStats",             "1",            &sg_printLevelStats,        CVAR_LATCH | CVAR_TEMP },
    { "sg_decalDetail",                 "3",            &sg_decalDetail,            CVAR_LATCH | CVAR_SAVE },
    { "sg_gibs",                        "0",            &sg_gibs,                   CVAR_LATCH | CVAR_SAVE },
    { "sg_levelInfoFile",               "levels.txt",   &sg_levelInfoFile,          CVAR_INIT | CVAR_LATCH },
    { "sg_savename",                    "savedata.ngd", &sg_savename,               CVAR_LATCH | CVAR_TEMP },
    { "sg_numSaves",                    "0",            &sg_numSaves,               CVAR_LATCH | CVAR_TEMP },
};

static uint32_t cvarTableSize = arraylen(cvarTable);

static void SG_RegisterCvars( void )
{
    uint32_t i;
    cvarTable_t *cv;

    for (i = 0, cv = cvarTable; i < cvarTableSize; i++, cv++) {
       Cvar_Register( cv->cvar, cv->name, cv->defaultValue, cv->flags );
    }
}

void SG_UpdateCvars( void )
{
    uint32_t i;
    cvarTable_t *cv;

    Cvar_Update( &sg_paused );
}

void GDR_ATTRIBUTE((format(printf, 1, 2))) GDR_DECL G_Printf(const char *fmt, ...)
{
    va_list argptr;
    char msg[4096];
    int32_t length;

    va_start(argptr, fmt);
    length = vsprintf(msg, fmt, argptr);
    va_end(argptr);

    trap_Print(msg);
}

void GDR_ATTRIBUTE((format(printf, 1, 2))) GDR_DECL G_Error(const char *err, ...)
{
    va_list argptr;
    char msg[4096];
    int32_t length;

    va_start(argptr, err);
    length = vsprintf(msg, err, argptr);
    va_end(argptr);

    trap_Error(msg);
}

void GDR_ATTRIBUTE((format(printf, 1, 2))) GDR_DECL SG_Printf(const char *fmt, ...)
{
    va_list argptr;
    char msg[4096];
    int32_t length;

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
    int32_t length;

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
    int32_t length;

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
    int32_t length;

    va_start(argptr, fmt);
    length = vsprintf(msg, fmt, argptr);
    va_end(argptr);

    if (length >= sizeof(msg)) {
        trap_Error( "SG_Printf: buffer overflow" );
    }

    SG_Printf("%s", msg);
}

//#endif

int32_t SG_RunLoop( int32_t levelTime )
{
    int32_t i;
    int32_t start, end;
    int32_t msec;
    sgentity_t *ent;

    if ( sg.state == SG_INACTIVE ) {
        return 0;
    }

    // get any cvar changes
    SG_UpdateCvars();

    // even if the game is paused, we still render everything in the background
    if ( sg.state == SG_SHOW_LEVEL_STATS ) {
        SG_DrawLevelStats();
        return 1; // we don't draw the level if we're ending it
    }

    SG_DrawFrame();

    if ( sg_paused.i ) {
        return 0;
    }

    sg.framenum++;
    sg.previousTime = sg.framenum;
    sg.levelTime = levelTime;
    msec = sg.levelTime - sg.previousTime;

    //
    // go through all allocated entities
    //
    start = trap_Milliseconds();
    ent = &sg_entities[0];
    for ( i = 0; i < sg.numEntities; i++) {
        if ( !ent->health ) {
            continue;
        }

        
    }
    end = trap_Milliseconds();

    return 1;
}


static void SG_LoadMedia( void )
{
    G_Printf( "Loading sgame sfx...\n" );

    sg.media.player_death0 = trap_Snd_RegisterSfx( "sfx/player/death1.wav" );
    sg.media.player_death1 = trap_Snd_RegisterSfx( "sfx/player/death2.wav" );
    sg.media.player_death2 = trap_Snd_RegisterSfx( "sfx/player/death3.wav" ); 
    sg.media.player_pain0 = trap_Snd_RegisterSfx( "sfx/player/pain0.wav" );
    sg.media.player_pain1 = trap_Snd_RegisterSfx( "sfx/player/pain1.wav" );
    sg.media.player_pain2 = trap_Snd_RegisterSfx( "sfx/player/pain2.wav" );
    sg.media.revolver_fire = trap_Snd_RegisterSfx( "sfx/weapons/revolver_fire.wav" );
    sg.media.revolver_rld = trap_Snd_RegisterSfx( "sfx/weapons/revolver_rld.wav" );

    G_Printf( "Finished loading sfx.\n" );

    G_Printf( "Loading sgame sprites...\n" );

    sg.media.raio_shader = RE_RegisterShader( "textures/sprites/glnomad_raio_base.png" );
    sg.media.grunt_shader = RE_RegisterShader( "textures/sprites/glnomad_grunt.png" );

    sg.media.raio_sprites = RE_RegisterSpriteSheet( "textures/sprites/glnomad_raio_base.png", 512, 512, 32, 32 );

    G_Printf( "Finished loading sprites.\n" );
}

void SG_Init( void )
{
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
}

void SG_Shutdown( void )
{
    G_Printf( "Shutting down sgame...\n" );

    trap_Key_SetCatcher( trap_Key_GetCatcher() & ~KEYCATCH_SGAME );

    memset( &sg, 0, sizeof(sg) );

    sg.state = SG_INACTIVE;
}

void GDR_ATTRIBUTE((format(printf, 2, 3))) GDR_DECL trap_FS_Printf( file_t f, const char *fmt, ... )
{
    va_list argptr;
    char msg[MAXPRINTMSG];

    va_start( argptr, fmt );
    vsprintf( msg, fmt, argptr );
    va_end( argptr );

    trap_FS_Write( msg, strlen(msg), f );
}
