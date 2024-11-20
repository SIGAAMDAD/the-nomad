#ifndef __N_DEBUG__
#define __N_DEBUG__

#pragma once

#include "n_shared.h"
#include "n_common.h"

typedef enum {
    ERR_OUT_OF_MEMORY,
    ERR_SEGGY,
    ERR_BUS,
    ERR_ASSERTION,

    ERR_NONE
} errorReason_t;

#define MAX_STACKTRACE_FRAMES 1024
extern void Sys_DebugStacktrace( uint32_t frames );
extern void Sys_DebugStacktraceFile( uint32_t frames );
extern bool Sys_IsInDebugSession( void );
extern void Sys_SetError( errorReason_t reason );
extern void Sys_AssertionFailure( const char *expr, const char *file, const char *func, unsigned line );
extern void GDR_ATTRIBUTE((format(printf, 5, 6))) Sys_AssertionFailureMsg( const char *expr, const char *file, const char *func, unsigned line, const char *msg, ... );
extern void Sys_DebugString( const char *str );
void Sys_DebugMessageBox( const char *title, const char *message );

#ifdef _NOMAD_DEBUG
#define AssertMsg( x, msg ) ((bool)(x) ? (void)0 : Sys_AssertionFailureMsg( #x, __FILE__, __func__, __LINE__, msg ) )
#define Assert( x ) ((bool)(x) ? (void)0 : Sys_AssertionFailure( #x, __FILE__, __func__, __LINE__ ) )
#define AssertMsg1( x, msg, ... ) ((bool)(x) ? (void)0 : Sys_AssertionFailureMsg( #x, __FILE__, __func__, __LINE__, msg, __VA_ARGS__ ) )
#else
// simply print it to the log if in dist or release mode
#define AssertMsg( x, msg ) if (!(x)) Con_Printf( COLOR_RED "[ASSERTION FAILURE - %s:%s:%u] " #msg "\n", __FILE__, FUNC_SIG, __LINE__ )
#define Assert( x ) if (!(x)) Con_Printf( COLOR_RED "[ASSERTION FAILURE] %s:%s:%u\n", __FILE__, FUNC_SIG, __LINE__ )
#define AssertMsg1( x, msg, ... ) if (!(x)) Con_Printf( COLOR_RED "[ASSERTION FAILURE - " __FILE__ ":%s:%u] " #msg  " ", FUNC_SIG, __LINE__, __VA_ARGS__ )
#endif

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
DWORD DumpStackTraceWindows( EXCEPTION_POINTERS *pException );
#endif

// NOTE: if you see Illegal Instruction (Core Dumped), that's __builtin_trap()'s fault, its not very pretty

// Used to step into the debugger
#if defined( _WIN32 )
    #define DebuggerBreak()  __debugbreak()
#else
	// On OSX, SIGTRAP doesn't really stop the thread cold when debugging.
	// So if being debugged, use INT3 which is precise.
    #if defined(OSX) || defined(PLATFORM_BSD)
        #if defined(arm32) || defined(arm64)
            #ifdef __clang__
                #define DebuggerBreak()  do { if ( Sys_IsInDebugSession() ) { __builtin_debugtrap(); } else { raise(SIGTRAP); } } while(0)
            #elif defined(__GNUC__)
                #define DebuggerBreak()  do { if ( Sys_IsInDebugSession() ) { __builtin_trap(); } else { raise(SIGTRAP); } } while(0)
            #else
                #define DebuggerBreak()  raise(SIGTRAP)
            #endif
        #else
            #define DebuggerBreak()  do { if ( Sys_IsInDebugSession() ) { __asm ( "int $3" ); } else { raise(SIGTRAP); } } while(0)
        #endif
    #else
        #define DebuggerBreak()  raise(SIGTRAP)
    #endif
#endif
#define	DebuggerBreakIfDebugging() if ( !Sys_IsInDebugSession() ) ; else DebuggerBreak()

#endif