#include "../engine/n_shared.h"
#include "sg_local.h"

int32_t SG_Init(void);
int32_t SG_Shutdown(void);
int32_t SG_RunLoop( int32_t msec );
int32_t SG_DrawFrame( void );

sgGlobals_t sg;

uint32_t sg_numLevels;

vmCvar_t sg_debugPrint;
vmCvar_t sg_paused;
vmCvar_t sg_pmAirAcceleration;
vmCvar_t sg_pmWaterAcceleration;
vmCvar_t sg_pmBaseAcceleration;
vmCvar_t sg_pmBaseSpeed;
vmCvar_t sg_mouseInvert;
vmCvar_t sg_mouseAcceleration;
vmCvar_t sg_printLevelStats;
vmCvar_t sg_decalDetail;
vmCvar_t sg_gibs;
vmCvar_t sg_drawFPS;
vmCvar_t sg_levelInfoFile;
vmCvar_t sg_levelIndex;
vmCvar_t sg_levelDataFile;
vmCvar_t sg_savename;

static const char *bindnames[] = {
    "forward",
    "back",
    "left",
    "right",
    "up",
    "down",
    "button0",
    "button1",
    "button2",
    "button3",
    "button4",
    "button5",
    "button6",
    "button7",
    "button8",
    "button9"
};

typedef struct {
    const char *name;
    const char *defaultValue;
    vmCvar_t *cvar;
    uint32_t flags;
} cvarTable_t;

static cvarTable_t cvarTable[] = {
    { "sg_debugPrint",                  "0",            &sg_debugPrint,             CVAR_LATCH },
    { "sg_paused",                      "0",            &sg_paused,                 CVAR_LATCH | CVAR_TEMP },
    { "sg_pmAirAcceleration",           "1.5",          &sg_pmAirAcceleration,      CVAR_LATCH | CVAR_SAVE },
    { "sg_pmWaterAcceleratino",         "0.5",          &sg_pmWaterAcceleration,    CVAR_LATCH | CVAR_SAVE },
    { "sg_pmBaseAcceleration",          "1.2",          &sg_pmBaseAcceleration,     CVAR_LATCH | CVAR_SAVE },
    { "sg_pmBaseSpeed",                 "1.0",          &sg_pmBaseSpeed,            CVAR_LATCH | CVAR_SAVE },
    { "g_mouseInvert",                  "0",            &sg_mouseInvert,            CVAR_LATCH | CVAR_SAVE },
    { "g_mouseAcceleration",            "0",            &sg_mouseAcceleration,      CVAR_LATCH | CVAR_SAVE },
    { "sg_printLevelStats",             "1",            &sg_printLevelStats,        CVAR_LATCH | CVAR_TEMP },
    { "sg_decalDetail",                 "3",            &sg_decalDetail,            CVAR_LATCH | CVAR_SAVE },
    { "sg_gibs",                        "0",            &sg_gibs,                   CVAR_LATCH | CVAR_SAVE },
    { "sg_drawFPS",                     "0",            &sg_drawFPS,                CVAR_LATCH | CVAR_SAVE },
    { "sg_levelInfoFile",               "levels.txt",   &sg_levelInfoFile,          CVAR_INIT | CVAR_ROM },
    { "sg_savename",                    "savedata.ngd", &sg_savename,               CVAR_LATCH | CVAR_TEMP },
};

/*
vmMain

this is the only way control passes into the module.
this must be the very first function compiled into the .qvm file
*/
int32_t vmMain(int32_t command, int32_t arg0, int32_t arg1, int32_t arg2, int32_t arg3, int32_t arg4, int32_t arg5, int32_t arg6, int32_t arg7,
    int32_t arg8, int32_t arg9, int32_t arg10)
{
    switch (command) {
    case SGAME_INIT:
        return SG_Init();
    case SGAME_SHUTDOWN:
        return SG_Shutdown();
    case SGAME_RUNTIC:
        return SG_RunLoop( arg0 );
    case SGAME_FINISH_FRAME:
        return SG_DrawFrame();
    case SGAME_STARTLEVEL:
        return SG_InitLevel( arg0 );
    case SGAME_ENDLEVEL:
        return SG_EndLevel();
    case SGAME_REWIND_TO_LAST_CHECKPOINT:
        return 1;
    default:
        SG_Error("vmMain: invalid command id: %i", command);
        break;
    };
    return -1;
}

static uint32_t cvarTableSize = arraylen(cvarTable);

static void SG_RegisterCvars( void )
{
    uint32_t i;
    cvarTable_t *cv;

    for (i = 0, cv = cvarTable; i < cvarTableSize; i++, cv++) {
        trap_Cvar_Register( cv->cvar, cv->name, cv->defaultValue, cv->flags );
    }
}

void SG_UpdateCvars( void )
{
    uint32_t i;
    cvarTable_t *cv;

    for (i = 0, cv = cvarTable; i < cvarTableSize; i++, cv++) {
        trap_Cvar_Update( cv->cvar );
    }
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

int32_t SG_RunLoop( int32_t msec )
{
    // increment the ticker
    sg.leveltime++;

    Ent_RunTic();
}


static qboolean SG_LoadMedia( void )
{
    G_Printf( "Loading sgame sfx...\n" );

    if ((sg.media.player_pain0 = trap_Snd_RegisterSfx( "sfx/player/pain0.wav" )) == FS_INVALID_HANDLE)
        return qfalse;
    
    if ((sg.media.player_pain1 = trap_Snd_RegisterSfx( "sfx/player/pain1.wav" )) == FS_INVALID_HANDLE)
        return qfalse;
    
    if ((sg.media.player_pain2 = trap_Snd_RegisterSfx( "sfx/player/pain2.wav" )) == FS_INVALID_HANDLE)
        return qfalse;
    
    if ((sg.media.player_death0 = trap_Snd_RegisterSfx( "sfx/player/death1.wav" )) == FS_INVALID_HANDLE)
        return qfalse;
    
    if ((sg.media.player_death1 = trap_Snd_RegisterSfx( "sfx/player/death2.wav" )) == FS_INVALID_HANDLE)
        return qfalse;
    
    if ((sg.media.player_death2 = trap_Snd_RegisterSfx( "sfx/player/death3.wav" )) == FS_INVALID_HANDLE)
        return qfalse;
    
    G_Printf( "Finished loading sfx.\n" );

    return qtrue;
}

int32_t SG_Init(void)
{
    if (!trap_Key_GetCatcher() & KEYCATCH_SGAME) {
        trap_Key_SetCatcher(trap_Key_GetCatcher() & KEYCATCH_SGAME);
    }

    // clear sgame state
    memset( &sg, 0, sizeof(sg) );
    
    // cache redundant calculations
    trap_GetGPUConfig( &sg.gpuConfig );

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

    SG_RegisterCvars();

    if (!SG_LoadMedia()) {
        G_Printf( COLOR_RED "SG_LoadMedia: failed!\n" );
        return -1; // did we fail to load the required resources?
    }

    SG_MemInit();

    sg.state = SGAME_INACTIVE;

    SG_InitEntities();

    // give the keybinding info to the engine
    G_SetBindNames( bindnames, arraylen(bindnames) );

    return 1;
}

int32_t SG_Shutdown(void)
{

    if (trap_Key_GetCatcher() & KEYCATCH_SGAME) {
        trap_Key_SetCatcher(trap_Key_GetCatcher() & ~KEYCATCH_SGAME);
    }

    return 0;
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
