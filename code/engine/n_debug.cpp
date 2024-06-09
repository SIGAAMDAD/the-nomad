#include "n_shared.h"
#include "n_common.h"

#ifdef POSIX
#include <dlfcn.h>
#include <execinfo.h>
#include <backtrace.h>
#include <cxxabi.h>
#include <malloc.h>
#elif defined(_WIN32)
#include <dbghelp.h>
#include <windows.h>
#include <excpt.h>
#endif
#include "../system/sys_thread.h"
#include "../system/sys_thread.inl"
#include <SDL2/SDL.h>

#define MEMHEADER 0xff1daf022

typedef struct {
	uint64_t m_Header;
	uint64_t m_nBytes;
	const char *m_pFilename;
	const char *m_pFunction;
	uint64_t m_nLine;
} DbgMemHeader_t;

void *InternalMalloc( size_t nBytes, const char *pFilename, const char *pFunction, uint32_t nLine )
{
	void *buf;
	DbgMemHeader_t *pHeader;

#ifdef OSX
#else
#if defined(POSIX) || !defined(_DEBUG)
	pHeader = (DbgMemHeader_t *)(malloc)( nBytes + sizeof(DbgMemHeader_t) );
	if (!pHeader) {
		return NULL;
	}

	pHeader->m_pFilename = pFilename;
	pHeader->m_pFunction = pFunction;
	pHeader->m_nLine = nLine;
	pHeader->m_Header = MEMHEADER;
#else
	pHeader = (DbgMemHeader_t *)_malloc_dbg( nBytes + sizeof(DbgMemHeader_t), _NORMAL_BLOCK, pFilename, nLine );
#endif
	pHeader->m_nBytes = nBytes;
	return pHeader + 1;
#endif
}


class CDebugSession
{
public:
    CDebugSession( void );
    ~CDebugSession();

#ifdef POSIX
    char *m_pStacktraceBuffer;
#elif defined(_WIN32)
	HANDLE m_hProcess;
	IMAGEHLP_SYMBOL64 *m_pSymbolBuffer;
	CONTEXT m_Context;
#endif
    void **m_pSymbolArray;
    FILE *m_pBacktraceFile;

#ifdef POSIX
    struct backtrace_state *m_pBTState;
	bool m_bBacktraceError;
	FILE *m_hBackTraceOutput;
#endif

    const char *m_pExePath;
    errorReason_t m_iErrorReason;
    bool m_bDoneErrorStackdump;
};

static CDebugSession g_debugSession;

#define MAX_SYMBOL_LENGTH 4096

#ifdef POSIX
#define MAX_STACKTRACE_FILESIZE 8192

static void bt_error_callback( void *data, const char *msg, int errnum )
{
    fprintf(stdout, "libbacktrace ERROR: %d - %s\n", errnum, msg);
    g_debugSession.m_bBacktraceError = true;
}

extern fileHandle_t logfile;

static void bt_syminfo_callback( void *data, uintptr_t pc, const char *symname,
								 uintptr_t symval, uintptr_t symsize )
{
    if (g_debugSession.m_bBacktraceError) {
        return;
    }

	if (symname != NULL) {
		int status;
		// [glnomad] 10/6/2023: fixed buffer instead of malloc'd buffer, risky however
		char name[MAX_SYMBOL_LENGTH];
		memset( name, 0, sizeof(name) );
		size_t length = sizeof(name);
		abi::__cxa_demangle(symname, name, &length, &status);
		if (name[0]) {
			symname = name;
		}

		if ( logfile != FS_INVALID_HANDLE && FS_Initialized() ) {
			FS_Printf( logfile, "  %-8zu %s\n", pc, symname );
		}
		fprintf( g_debugSession.m_hBackTraceOutput, "  %-8zu %s\n", pc, symname );
		fprintf( stdout, "  %-8zu %s\n", pc, symname );
	} else {
		if ( logfile != FS_INVALID_HANDLE && FS_Initialized() ) {
			FS_Printf( logfile, "%-8zu (unknown symbol)\n", pc );
		}
		fprintf( g_debugSession.m_hBackTraceOutput, "%-8zu (unknown symbol)\n", pc );
		fprintf( stdout, "  %-8zu (unknown symbol)\n", pc );
	}
}

static int bt_pcinfo_callback( void *data, uintptr_t pc, const char *filename, int lineno, const char *function )
{
    if (g_debugSession.m_bBacktraceError) {
        return 0;
    }

	if (data != NULL) {
		int* hadInfo = (int*)data;
		*hadInfo = (function != NULL);
	}

	if (function != NULL) {
		int status;
		// [glnomad] 10/6/2023: fixed buffer instead of malloc'd buffer, risky however
		char name[MAX_SYMBOL_LENGTH];
		memset( name, 0, sizeof(name) );
		size_t length = sizeof(name);
		abi::__cxa_demangle(function, name, &length, &status);
		if (name[0]) {
			function = name;
		}

		const char* fileNameSrc = strstr(filename, "/src/");
		if (fileNameSrc != NULL) {
			filename = fileNameSrc+1; // I want "src/bla/blub.cpp:42"
		}
		if (logfile != FS_INVALID_HANDLE) {
			FS_Printf( logfile,  "  %-8zu %-16s:%-8d %s\n", pc, filename, lineno, function );
		}
		fprintf( g_debugSession.m_hBackTraceOutput, "  %-8zu %-16s:%-8d %s\n", pc, filename, lineno, function );
		fprintf( stdout, "  %-8zu %-16s:%-8d %s\n", pc, filename, lineno, function );
	}

	return 0;
}

static void bt_error_dummy( void *data, const char *msg, int errnum )
{
	//CrashPrintf("ERROR-DUMMY: %d - %s\n", errnum, msg);
}

static int bt_simple_callback(void *data, uintptr_t pc)
{
	int pcInfoWorked;

    pcInfoWorked = 0;
	// if this fails, the executable doesn't have debug info, that's ok (=> use bt_error_dummy())
	backtrace_pcinfo( g_debugSession.m_pBTState, pc, bt_pcinfo_callback, bt_error_dummy, &pcInfoWorked );
	if ( !pcInfoWorked ) { // no debug info? use normal symbols instead
		// yes, it would be easier to call backtrace_syminfo() in bt_pcinfo_callback() if function == NULL,
		// but some libbacktrace versions (e.g. in Ubuntu 18.04's g++-7) don't call bt_pcinfo_callback
		// at all if no debug info was available - which is also the reason backtrace_full() can't be used..
		backtrace_syminfo( g_debugSession.m_pBTState, pc, bt_syminfo_callback, bt_error_callback, NULL );
	}

	return 0;
}
#endif


static GDR_INLINE size_t MemSize( void *ptr )
{
#ifdef _WIN32
    return _msize( ptr );
#elif defined(OSX) || defined(__APPLE__)
    return malloc_size( ptr );
#else
    return malloc_usable_size( ptr );
#endif
}

CDebugSession::CDebugSession( void )
{
    m_pExePath = "TheNomad.x64";

#ifdef POSIX
    m_pBTState = backtrace_create_state( m_pExePath, 0, bt_error_callback, NULL );

    if (!m_pBTState) {
        Con_DPrintf( COLOR_YELLOW "WARNING: failed to init libbacktrace, the symbols outputted will look very weird...\n" );
    }
#endif

    m_pBacktraceFile = Sys_FOpen( "backtrace.dat", "w+" );
    if (!m_pBacktraceFile) {
        Sys_Error( "Failed to open a backtrace file" );
    }

#ifdef POSIX
    m_pStacktraceBuffer = (char *)malloc( MAX_STACKTRACE_FILESIZE );
    if (!m_pStacktraceBuffer) {
        Sys_Error( "Failed to allocate sufficient memory for stacktrace buffer" );
    }

    m_bBacktraceError = false;
#elif defined(_WIN32)
	m_hProcess = GetCurrentProcess();

	memset( &m_Context, 0, sizeof( m_Context ) );
	m_Context.ContextFlags = CONTEXT_FULL;
	RtlCaptureContext( &m_Context );

	SymInitialize( m_hProcess, NULL, TRUE );

	m_pSymbolBuffer = (IMAGEHLP_SYMBOL64 *)calloc( sizeof( *m_pSymbolBuffer ) + MAX_SYMBOL_LENGTH, 1 );
	if ( !m_pSymbolBuffer ) {
		Sys_Error( "Failed to allocate sufficient memory for stacktrace buffer" );
	}

	AllocConsole();
#endif
	m_pSymbolArray = (void **)malloc( sizeof(void *) * MAX_STACKTRACE_FRAMES );
    if (!m_pSymbolArray) {
        Sys_Error( "Failed to allocate sufficient memory for stacktrace frames" );
    }

	m_bDoneErrorStackdump = false;
	m_iErrorReason = ERR_NONE;
}

CDebugSession::~CDebugSession()
{
    if (m_iErrorReason != ERR_NONE) {
        if (!m_bDoneErrorStackdump) {
            Sys_DebugStacktrace( MAX_STACKTRACE_FRAMES );
        }
    }
#ifdef _WIN32
	FreeConsole();
	SymCleanup( m_hProcess );
#endif

    if (m_iErrorReason == ERR_NONE) { // could lead to more recursive errors
        if (m_pBacktraceFile) {
            fclose( m_pBacktraceFile );
        }
        if (m_pSymbolArray) {
            free( m_pSymbolArray );
        }
#ifdef POSIX
        if (m_pStacktraceBuffer) {
            free( m_pStacktraceBuffer );
		}
#elif defined(_WIN32)
		if (m_pSymbolBuffer) {
			free( m_pSymbolBuffer );
		}
#endif
    }
}

GDR_INLINE bool ValidStackAddress( void *pAddress, const void *pNoLessThan, const void *pNoGreaterThan )
{
    // [glnomad] fixup, source engine uses a uint64_t, while that is the current pointer type, it still
    // means less portability
    if ((uintptr_t)pAddress & 3) {
        return false;
    }
    if (pAddress < pNoLessThan) { // frame pointer traversal should always increase the pointer
		return false;
    }
	if (pAddress > pNoGreaterThan) { // never traverse outside the stack (Oh 0xCCCCCCCC, how I hate you)
		return false;
    }

#if defined( WIN32 ) && !defined( _X360 )
	if (IsBadReadPtr( pAddress, (sizeof( void * ) * 2) )) { // safety net, but also throws an exception (handled internally) to stop bad access
		return false;
    }
#endif

	return true;
}

void Sys_DebugMessageBox( const char *title, const char *message )
{
#ifdef _WIN32
    ::MessageBox( NULL, message, title, MB_OK );
#else
    int buttonid = 0;
    SDL_MessageBoxData boxData = { 0 };
    SDL_MessageBoxButtonData buttonData[] = {
        { SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT, 1, "OK"      },
//        { SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT, 0, "Cancel"  },
    };

    boxData.window = SDL_GL_GetCurrentWindow();
    boxData.title = title;
    boxData.message = message;
    boxData.numbuttons = 1;
    boxData.buttons = buttonData;

    SDL_ShowMessageBox( &boxData, &buttonid );
#endif
}

void Sys_SetError( errorReason_t reason ) {
    g_debugSession.m_iErrorReason = reason;
}

void Sys_AssertionFailure( const char *expr, const char *file, const char *func, unsigned line )
{
    g_debugSession.m_iErrorReason = ERR_ASSERTION;

    N_Error( ERR_FATAL,
            "Assertion '%s' failed:\n"
            "\tFile: %s\n"
            "\tFunction: %s\n"
            "\tLine: %u\n"
    , expr, file, func, line );
}

void GDR_ATTRIBUTE((format(printf, 5, 6))) Sys_AssertionFailureMsg( const char *expr, const char *file, const char *func, unsigned line, const char *msg, ... )
{
    va_list argptr;
    char text[MAXPRINTMSG];

    g_debugSession.m_iErrorReason = ERR_ASSERTION;

    va_start( argptr, msg );
    N_vsnprintf( text, sizeof(text), msg, argptr );
    va_end( argptr );

    Con_Printf( COLOR_RED
            "Assertion '%s' failed:\n"
            "\tFile: %s\n"
            "\tFunction: %s\n"
            "\tLine: %u\n"
            "\tMessage: %s\n"
    , expr, file, func, line, text );

	abort();
}

void Sys_DebugString( const char *str )
{
#ifdef _CERT
    return; // do nothing
#endif

#ifdef _WIN32
    ::OutputDebugStringA( str );
#else
#ifdef _NOMAD_DEBUG
    Con_Printf( COLOR_GREEN "[Debug]: %s\n", str );
#else
    Con_DPrintf( COLOR_GREEN "%s\n", str );
#endif
#endif
}

void Sys_DebugStacktraceFile( uint32_t frames, FILE *fp )
{
#ifdef _WIN32
	DWORD i;
	USHORT nFrames;
	IMAGEHLP_LINE64 SymLine;
	IMAGEHLP_MODULE64 ModuleInfo;
//	PSYMBOL_INFO64 pSymbol;
	PIMAGEHLP_SYMBOL64 pSymbol;
	HMODULE hModule;
	HANDLE hProcess;
	DWORD dwDisplacement;
	DWORD64 dwDisplacement64;
	DWORD64 dwAddr;
	DWORD64 dwModuleBase;
	STACKFRAME64 stackFrame;
	HANDLE hThread;
	DWORD image;

	hModule = GetModuleHandle( NULL );
	hProcess = GetCurrentProcess();
	hThread = GetCurrentThread();

	ZeroMemory( &stackFrame, sizeof( stackFrame ) );
#ifdef _M_IX86
	image = IMAGE_FILE_MACHINE_I386;
	stackFrame.AddrPC.Offset = g_debugSession.m_Context.Eip;
	stackFrame.AddrPC.Mode = AddrModeFlat;
	stackFrame.AddrFrame.Offset = g_debugSession.m_Context.Ebp;
	stackFrame.AddrFrame.Mode = AddrModeFlat;
	stackFrame.AddrStack.Offset = g_debugSession.m_Context.Esp;
	stackFrame.AddrStack.Mode = AddrModeFlat;
#elif defined(_M_X64)
	image = IMAGE_FILE_MACHINE_AMD64;
	stackFrame.AddrPC.Offset = g_debugSession.m_Context.Rip;
	stackFrame.AddrPC.Mode = AddrModeFlat;
	stackFrame.AddrFrame.Offset = g_debugSession.m_Context.Rsp;
	stackFrame.AddrFrame.Mode = AddrModeFlat;
	stackFrame.AddrStack.Offset = g_debugSession.m_Context.Rsp;
	stackFrame.AddrStack.Mode = AddrModeFlat;
#elif defined(_M_IA64)
	image = IMAGE_FILE_MACHINE_IA64;
	stackFrame.AddrPC.Offset = g_debugSession.m_Context.StIIP;
	stackFrame.AddrPC.Mode = AddrModeFlat;
	stackFrame.AddrFrame.Offset = g_debugSession.m_Context.IntSp;
	stackFrame.AddrFrame.Mode = AddrModeFlat;
	stackFrame.AddrBStore.Offset = g_debugSession.m_Context.RsBSP;
	stackFrame.AddrBStore.Mode = AddrModeFlat;
	stackFrame.AddrStack.Offset = g_debugSession.m_Context.IntSp;
	stackFrame.AddrStack.Mode = AddrModeFlat;
#endif

	for ( uint32_t i = 0; i < frames; i++ ) {
		const BOOL result = StackWalk64( image, hProcess, hThread, &stackFrame, &g_debugSession.m_Context, NULL, SymFunctionTableAccess64,
			SymGetModuleBase64, NULL );
		if ( !result ) {
			break;
		}

		char buffer[ sizeof( SYMBOL_INFO ) + ( MAX_SYM_NAME - 1 ) * sizeof( TCHAR ) ];
		PSYMBOL_INFO symbol = (PSYMBOL_INFO)buffer;
		symbol->SizeOfStruct = sizeof( SYMBOL_INFO );
		symbol->MaxNameLen = MAX_SYM_NAME - 1;

		SymLine.SizeOfStruct = sizeof( IMAGEHLP_LINE64 );
		SymGetLineFromAddr64( hProcess, stackFrame.AddrPC.Offset, &dwDisplacement, &SymLine );
		SymGetModuleInfo64( hProcess, symbol->ModBase, &ModuleInfo );

		if ( SymFromAddr( hProcess, stackFrame.AddrPC.Offset, &dwDisplacement64, symbol ) ) {
			Con_Printf( "[Frame %i] %s\n"
						"(0x%04lx) %s:%i:%i -> %s\n"
				, i, ModuleInfo.ModuleName, symbol->Address, SymLine.FileName, SymLine.LineNumber, dwDisplacement, (const char *)symbol->Name );
			
			fprintf( fp, "[Frame %i] %s\n"
						"(0x%04lx) %s:%i:%i -> %s\n"
				, i, ModuleInfo.ModuleName, symbol->Address, SymLine.FileName, SymLine.LineNumber, dwDisplacement, (const char *)symbol->Name );
		} else {
			Con_Printf( "SymFromAddr failed: %s\n", Sys_GetError() );
			Con_Printf( "[Frame %i] (unknown symbol) ???\n", i );

			fprintf( fp, "SymFromAddr failed: %s\n", Sys_GetError() );
			fprintf( fp, "[Frame %i] (unknown symbol) ???\n", i );
		}
	}

/*
	memset( g_debugSession.m_pSymbolArray, 0, sizeof( void * ) * MAX_STACKTRACE_FRAMES );
	nFrames = CaptureStackBackTrace( 1, frames, g_debugSession.m_pSymbolArray, NULL );

	ModuleInfo.SizeOfStruct = sizeof( IMAGEHLP_MODULE64 );
	SymLine.SizeOfStruct = sizeof( IMAGEHLP_LINE64 );

	pSymbol = g_debugSession.m_pSymbolBuffer;
	pSymbol->SizeOfStruct = sizeof( SYMBOL_INFO ) + ( ( MAX_SYMBOL_LENGTH - 1 ) * sizeof( char ) );
	pSymbol->MaxNameLength = MAX_SYMBOL_LENGTH - 1;

	for ( i = 0; i < nFrames; i++ ) {
		dwModuleBase = SymGetModuleBase64( hProcess, (DWORD64)g_debugSession.m_pSymbolArray[i] );
		SymGetSymFromAddr64( hProcess,(DWORD64)( g_debugSession.m_pSymbolArray[i] ), NULL, pSymbol );
		SymGetLineFromAddr64( hProcess, (DWORD64)g_debugSession.m_pSymbolArray[i], &dwDisplacement, &SymLine );
		SymGetModuleInfo64( hProcess, dwModuleBase, &ModuleInfo );

		Con_Printf( "[Frame #%i]\n"
					"  Module Name: %s\n"
					"  Address: 0x%04lx\n"
					"  Line: %i\n"
					"  Name: %s\n"
					"  File: %s\n"
				, nFrames - i - 1, ModuleInfo.ModuleName, pSymbol->Address, SymLine.LineNumber, (char *)pSymbol->Name, SymLine.FileName );
	}
	*/

#elif defined(POSIX)
	char *buffer;
    int numframes;
    uint64_t fileLength;

	g_debugSession.m_hBackTraceOutput = fp;

    if ( g_debugSession.m_pBTState ) {
        int skip = 2; // skip this function in backtrace
		backtrace_simple( g_debugSession.m_pBTState, skip, bt_simple_callback, bt_error_callback, NULL );
    }
    else {
        // use libgcc's backtrace

        numframes = backtrace( g_debugSession.m_pSymbolArray, frames );

        /* write the backtrace */
        backtrace_symbols_fd( g_debugSession.m_pSymbolArray, numframes, fileno(g_debugSession.m_pBacktraceFile) );

        fseek( g_debugSession.m_pBacktraceFile, 0L, SEEK_END );
        fileLength = ftell( g_debugSession.m_pBacktraceFile );
        fseek( g_debugSession.m_pBacktraceFile, 0L, SEEK_SET );

        if ( fileLength >= MAX_STACKTRACE_FILESIZE ) {
			if ( !FS_Initialized() ) {
				fprintf( stderr, "WARNING: stacktrace file size is %lu\n", fileLength );
			}
	        else {
			    Con_Printf( "WARNING: stacktrace file size is %lu\n", fileLength );
			}
            return;
        }

        if (!((int64_t)Sys_StackMemoryRemaining() - (int64_t)fileLength)) {
            buffer = g_debugSession.m_pStacktraceBuffer;
        }
        else {
            // just use a stack buffer
            static char *p = (char *)alloca( fileLength );
            buffer = p;
        }

	    fread( buffer, fileLength, 1, g_debugSession.m_pBacktraceFile );
		fwrite( buffer, fileLength, 1, fp );
    }
#endif
}

void Sys_DebugStacktrace( uint32_t frames )
{
	if ( com_errorEntered && g_debugSession.m_iErrorReason != ERR_NONE ) {
        g_debugSession.m_bDoneErrorStackdump = true;
    }
	Sys_DebugStacktraceFile( frames, FS_Handle( logfile ) );
}
