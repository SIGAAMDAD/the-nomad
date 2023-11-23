#include "../engine/n_shared.h"
#include "sg_local.h"

int32_t SG_Init(void);
int32_t SG_Shutdown(void);
int32_t SG_RunLoop( int32_t msec );
int32_t SG_DrawFrame( void );

sgGlobals_t sg;

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

typedef struct {
    const char *name;
    const char *defaultValue;
    vmCvar_t *cvar;
    uint32_t flags;
} cvarTable_t;

static cvarTable_t cvarTable[] = {
    { "sg_debugPrint",                  "0",    &sg_debugPrint,             CVAR_LATCH },
    { "sg_paused",                      "0",    &sg_paused,                 CVAR_LATCH | CVAR_TEMP },
    { "sg_pmAirAcceleration",           "1.5",  &sg_pmAirAcceleration,      CVAR_LATCH | CVAR_SAVE },
    { "sg_pmWaterAcceleratino",         "0.5",  &sg_pmWaterAcceleration,    CVAR_LATCH | CVAR_SAVE },
    { "sg_pmBaseAcceleration",          "1.2",  &sg_pmBaseAcceleration,     CVAR_LATCH | CVAR_SAVE },
    { "sg_pmBaseSpeed",                 "1.0",  &sg_pmBaseSpeed,            CVAR_LATCH | CVAR_SAVE },
    { "g_mouseInvert",                  "0",    &sg_mouseInvert,            CVAR_LATCH | CVAR_SAVE },
    { "g_mouseAcceleration",            "0",    &sg_mouseAcceleration,      CVAR_LATCH | CVAR_SAVE },
    { "sg_printLevelStats",             "0",    &sg_printLevelStats,        CVAR_LATCH | CVAR_TEMP },
    { "sg_decalDetail",                 "3",    &sg_decalDetail,            CVAR_LATCH | CVAR_SAVE },
    { "sg_gibs",                        "0",    &sg_gibs,                   CVAR_LATCH | CVAR_SAVE },
    { "sg_drawFPS",                     "0",    &sg_drawFPS,                CVAR_LATCH | CVAR_SAVE },
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
    default:
        SG_Error("vmMain: invalid command id: %i", command);
        break;
    };
    return -1;
}

static void SG_RegisterCvars( void )
{
    uint32_t i;

    for (i = 0; i < arraylen(cvarTable); i++) {
        trap_Cvar_Register( cvarTable[i].cvar, cvarTable[i].name, cvarTable[i].defaultValue, cvarTable[i].flags );
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

    return qtrue;
}

int32_t SG_Init(void)
{
    if (!trap_Key_GetCatcher() & KEYCATCH_SGAME) {
        trap_Key_SetCatcher(trap_Key_GetCatcher() & KEYCATCH_SGAME);
    }
    
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

    if (!SG_LoadMedia()) {
        G_Printf( COLOR_RED "SG_LoadMedia: failed!\n" );
        return -1; // did we fail to load the required resources?
    }

    SG_MemInit();

    sg.state = SGAME_INACTIVE;

    sg.mobs = SG_MemAlloc( sizeof(mobj_t) * MAXMOBS );
    sg.items = SG_MemAlloc( sizeof(item_t) * MAXITEMS );
    sg.weapons = SG_MemAlloc( sizeof(weapon_t) * MAXWEAPONS );

    memset( &sg.playr, 0, sizeof(sg.playr) );

    SG_InitEntities();

    return 1;
}

int32_t SG_Shutdown(void)
{

    if (trap_Key_GetCatcher() & KEYCATCH_SGAME) {
        trap_Key_SetCatcher(trap_Key_GetCatcher() & ~KEYCATCH_SGAME);
    }

    return 0;
}
