#include "unix_local.h"
#include "../game/g_game.h"

static qboolean signalcaught = qfalse;

extern void GDR_NORETURN Sys_Exit( int code );

static void SignalHandle( int sig )
{
    char msg[32];
    const char *str;

    if ( signalcaught == qtrue ) {
        str = va( "DOUBLE SIGNAL FAULT: Recieved signal %i, exiting...\n", sig );
        write( STDERR_FILENO, str, strlen( str ) );
        Sys_Exit( 1 ); // abstraction
    }

    str = va( "Recieved signal %i, exiting...\n", sig );
    write( STDERR_FILENO, str, strlen( str ) );

    switch ( sig ) {
    case SIGSEGV:
        exit_type = &signals[0];
        Sys_SetError( ERR_SEGGY );
        Sys_Error( "An unrecoverable error has occured within the engine, please click OK and report this error.\n\n(FOR DEVELOPERS) Memory Access Violation (SIGSEGV)" );
        break;
    case SIGABRT:
        exit_type = &signals[2];
        Sys_SetError( ERR_ASSERTION );
        Sys_Error( "An unrecoverable error has occured within the engine, please click OK and report this error.\n\n(FOR DEVELOPERS) Debug Assertion (SIGABRT)" );
        break;
    case SIGBUS:
        exit_type = &signals[1];
        Sys_SetError( ERR_BUS );
        Sys_Error( "An unrecoverable error has occured within the engine, please click OK and report this error.\n\n(FOR DEVELOPERS) Memory Bus Error (SIGBUS)" );
        break;
    case SIGILL:
        break;
    case SIGTRAP: {
        static qboolean recursive = qfalse;
        Con_DPrintf( "DebugBreak Triggered...\n" );
        Sys_DebugStacktrace( 1024 ); // do a stack dump

        // debug traps woun't happen unless we have a dedicated debug binary
        if ( !recursive ) {
            Sys_MessageBox( "DebugBreak Triggered", "A DebugBreak was triggered", false );
            recursive = qtrue;
            signal( SIGTRAP, SignalHandle );
        }
        else {
            Sys_Error( "DebugBreak triggered twice!" );
        }
        return; }
    case SIGTERM:
        Con_Printf( "SIGTERM, exiting...\n" );
        Sys_Exit( 1 );
        break;
    case SIGFPE: {
        static qboolean recursive = qfalse;
        Con_DPrintf( "FPE Triggered...\n" );
        Sys_DebugStacktrace( 1024 ); // do a stack dump

        if ( !recursive ) {
            Sys_MessageBox( "Engine Warning", "A Floating Point Exception was caught", false );
            recursive = qtrue;
            signal( SIGFPE, SignalHandle );
        }
        else {
            Sys_Error( "An unrecoverable error has occured within the engine, please click OK and report this error.\n\n(FOR DEVELOPERS) Divide by zero or somethin'?? (SIGFPE)" );
        }
        return; }
    case SIGHUP:
        return;
    default:
        Con_Printf( "Unknown signal (%i)... Wtf?\n", sig );
        return;
    };
    Sys_Exit( -1 );
}

void InitSig( void )
{
    signal( SIGTERM, SignalHandle );
    signal( SIGSEGV, SignalHandle );
    signal( SIGFPE, SignalHandle );
    signal( SIGABRT, SignalHandle );
    signal( SIGBUS, SignalHandle );
    signal( SIGTRAP, SignalHandle );
    signal( SIGILL, SignalHandle );
    signal( SIGINT, SIG_IGN );
    signal( SIGHUP, SignalHandle );
}
