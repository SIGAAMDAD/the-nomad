#include "../engine/n_shared.h"
#include "sg_local.h"

int SG_Init(void);
int SG_Shutdown(void);
int SG_RunLoop(void);

/*
vmMain

this is the only way control passes into the module.
this must be the very first function compiled into the .qvm file
*/
int vmMain(int command, int arg0, int arg1, int arg2, int arg3, int arg4, int arg5, int arg6, int arg7,
    int arg8, int arg9, int arg10)
{
    switch (command) {
    case SGAME_INIT:
        return SG_Init();
    case SGAME_SHUTDOWN:
        return SG_Shutdown();
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

    va_start(argptr, fmt);
    vsprintf(msg, fmt, argptr);
    va_end(argptr);

    trap_Print(msg);
}

void GDR_DECL SG_Error(const char *fmt, ...)
{
    va_list argptr;
    char msg[4096];

    memset(msg, 0, sizeof(msg));
    va_start(argptr, fmt);
    vsprintf(msg, fmt, argptr);
    va_end(argptr);

    trap_Error(msg);
}

void GDR_DECL GDR_ATTRIBUTE((format(printf, 2, 3))) N_Error(errorCode_t code, const char *err, ...)
{
    va_list argptr;
    char msg[4096];

    va_start(argptr, err);
    vsprintf(msg, err, argptr);
    va_end(argptr);

    SG_Error("%s", msg);
}

//#ifndef SGAME_HARD_LINKED
// this is only here so the functions in n_shared.c and bg_*.c can link

void GDR_DECL GDR_ATTRIBUTE((format(printf, 1, 2))) Con_Printf(const char *fmt, ...)
{
    va_list argptr;
    char msg[4096];

    va_start(argptr, fmt);
    vsprintf(msg, fmt, argptr);
    va_end(argptr);

    SG_Printf("%s", msg);
}

//#endif

int SG_Init(void)
{
//    if (!trap_Key_GetCatcher() & KEYCATCH_SGAME) {
//        trap_Key_SetCatcher(trap_Key_GetCatcher() & KEYCATCH_SGAME);
//    }

    return 0;
}

int SG_Shutdown(void)
{
    SG_ClearMem();

    if (trap_Key_GetCatcher() & KEYCATCH_SGAME) {
        trap_Key_SetCatcher(trap_Key_GetCatcher() & ~KEYCATCH_SGAME);
    }

    return 0;
}
