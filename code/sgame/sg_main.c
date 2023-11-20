#include "../engine/n_shared.h"
#include "sg_local.h"

int32_t SG_Init(void);
int32_t SG_Shutdown(void);
int32_t SG_RunLoop(void);

sgGlobals_t sg;

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

void GDR_DECL SG_Printf(const char *fmt, ...)
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

void GDR_DECL SG_Error(const char *fmt, ...)
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

static int32_t SG_LoadSfx( void )
{
    sg.media.player_hurt0 = trap_Snd_RegisterSfx( "sfx/player/hurt0.wav" );
    sg.media.player_hurt1 = trap_Snd_RegisterSfx( "sfx/player/hurt1.wav" );
    sg.media.player_hurt2 = trap_Snd_RegisterSfx( "sfx/player/hurt2.wav" );

    return 1;
}

int32_t SG_Init(void)
{
    if (!trap_Key_GetCatcher() & KEYCATCH_SGAME) {
        trap_Key_SetCatcher(trap_Key_GetCatcher() & KEYCATCH_SGAME);
    }

    if (!SG_LoadSfx()) {
        return -1;
    }

    return 1;
}

int32_t SG_Shutdown(void)
{
    SG_ClearMem();

    if (trap_Key_GetCatcher() & KEYCATCH_SGAME) {
        trap_Key_SetCatcher(trap_Key_GetCatcher() & ~KEYCATCH_SGAME);
    }

    return 0;
}
