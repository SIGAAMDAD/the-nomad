#include "n_shared.h"
#include <backtrace.h>
#include <cxxabi.h>

#ifdef POSIX
#include <dlfcn.h>
#include <execinfo.h>
#elif defined(_WIN32)
#include <dbghelp.h>
#endif
#include <malloc.h>

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
	SYMBOL_INFO *m_pSymbolBuffer;
#endif
    void **m_pSymbolArray;
    FILE *m_pBacktraceFile;

#ifdef POSIX
    struct backtrace_state *m_pBTState;
	bool m_bBacktraceError;
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

extern file_t logfile;

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

		if (logfile != FS_INVALID_HANDLE) {
			FS_Printf( logfile, "  %-8zu %s\n", pc, symname );
		}
		fprintf( stdout, "  %-8zu %s\n", pc, symname );
	} else {
		if (logfile != FS_INVALID_HANDLE) {
			FS_Printf( logfile, "%-8zu (unknown symbol)\n", pc );
		}
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
	backtrace_pcinfo(g_debugSession.m_pBTState, pc, bt_pcinfo_callback, bt_error_dummy, &pcInfoWorked);
	if (!pcInfoWorked) { // no debug info? use normal symbols instead
		// yes, it would be easier to call backtrace_syminfo() in bt_pcinfo_callback() if function == NULL,
		// but some libbacktrace versions (e.g. in Ubuntu 18.04's g++-7) don't call bt_pcinfo_callback
		// at all if no debug info was available - which is also the reason backtrace_full() can't be used..
		backtrace_syminfo(g_debugSession.m_pBTState, pc, bt_syminfo_callback, bt_error_callback, NULL);
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
    m_pExePath = "glnomad";

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

	SymInitialize( m_hProcess, NULL, TRUE );

	m_pSymbolBuffer = (SYMBOL_INFO *)malloc( sizeof(SYMBOL_INFO) + MAX_SYMBOL_LENGTH );
	if (!m_pSymbolBuffer) {
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
#ifdef _WIN32
	FreeConsole();
#endif
    if (m_iErrorReason != ERR_NONE) {
        if (!m_bDoneErrorStackdump) {
            Sys_DebugStacktrace( MAX_STACKTRACE_FRAMES );
        }
    }

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

#ifdef _WIN32
#pragma auto_inline( off )
int GetCallStack_Fast( void **pReturnAddressesOut, int iArrayCount, int iSkipCount )
{
	//Only tested in windows. This function won't work with frame pointer omission enabled. "vpc /nofpo" all projects
#if (defined( TIER0_FPO_DISABLED ) || defined( _DEBUG )) &&\
	(defined( WIN32 ) && !defined( _X360 ))
	void *pStackCrawlEBP;
	__asm
	{
		mov [pStackCrawlEBP], ebp;
	}

	/*
	With frame pointer omission disabled, this should be the pattern all the way up the stack
	[ebp+00]   Old ebp value
	[ebp+04]   Return address
	*/

	void *pNoLessThan = pStackCrawlEBP; //impossible for a valid stack to traverse before this address
	int i;

	CStackTop_FriendFuncs *pTop = (CStackTop_FriendFuncs *)(CStackTop_Base *)g_StackTop;
	if( pTop != NULL ) //we can do fewer error checks if we have a valid reference point for the top of the stack
	{		
		void *pNoGreaterThan = pTop->m_pStackBase;

		//skips
		for( i = 0; i != iSkipCount; ++i )
		{
			if( (pStackCrawlEBP < pNoLessThan) || (pStackCrawlEBP > pNoGreaterThan) )
				return AppendParentStackTrace( pReturnAddressesOut, iArrayCount, 0 );

			pNoLessThan = pStackCrawlEBP;
			pStackCrawlEBP = *(void **)pStackCrawlEBP; //should be pointing at old ebp value
		}

		//store
		for( i = 0; i != iArrayCount; ++i ) {
			if( (pStackCrawlEBP < pNoLessThan) || (pStackCrawlEBP > pNoGreaterThan) )
				break;

			pReturnAddressesOut[i] = *((void **)pStackCrawlEBP + 1);

			pNoLessThan = pStackCrawlEBP;
			pStackCrawlEBP = *(void **)pStackCrawlEBP; //should be pointing at old ebp value
		}

		return AppendParentStackTrace( pReturnAddressesOut, iArrayCount, i );
	}
	else {
		void *pNoGreaterThan = ((unsigned char *)pNoLessThan) + (1024 * 1024); //standard stack is 1MB. TODO: Get actual stack end address if available since this check isn't foolproof	

		//skips
		for( i = 0; i != iSkipCount; ++i )
		{
			if( !ValidStackAddress( pStackCrawlEBP, pNoLessThan, pNoGreaterThan ) )
				return AppendParentStackTrace( pReturnAddressesOut, iArrayCount, 0 );

			pNoLessThan = pStackCrawlEBP;
			pStackCrawlEBP = *(void **)pStackCrawlEBP; //should be pointing at old ebp value
		}

		//store
		for( i = 0; i != iArrayCount; ++i ) {
			if( !ValidStackAddress( pStackCrawlEBP, pNoLessThan, pNoGreaterThan ) )
				break;

			pReturnAddressesOut[i] = *((void **)pStackCrawlEBP + 1);

			pNoLessThan = pStackCrawlEBP;
			pStackCrawlEBP = *(void **)pStackCrawlEBP; //should be pointing at old ebp value
		}

		return AppendParentStackTrace( pReturnAddressesOut, iArrayCount, i );
	}
#endif

	return 0;
}
#pragma auto_inline( on )
#endif


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

    N_Error( ERR_FATAL,
            "Assertion '%s' failed:\n"
            "\tFile: %s\n"
            "\tFunction: %s\n"
            "\tLine: %u\n"
            "\tMessage: %s\n"
    , expr, file, func, line, text );
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

#include <iostream>

void Sys_DebugStacktrace( uint32_t frames )
{
#ifdef _WIN32
	if (com_errorEntered && g_debugSession.m_iErrorReason != ERR_NONE) {
        g_debugSession.m_bDoneErrorStackdump = true;
    }

	std::cout << boost::stacktrace::stacktrace() << std::endl;
#elif 0
	uint32_t i;
	uint32_t numFrames;

	if (com_errorEntered && g_debugSession.m_iErrorReason != ERR_NONE) {
        g_debugSession.m_bDoneErrorStackdump = true;
    }

	numFrames = CaptureStackBackTrace( 1, frames, g_debugSession.m_pSymbolArray, NULL );
	g_debugSession.m_pSymbolBuffer->MaxNameLen = MAX_SYMBOL_LENGTH - 1;
	g_debugSession.m_pSymbolBuffer->SizeOfStruct = sizeof(SYMBOL_INFO) + (MAX_SYMBOL_LENGTH - 1) * sizeof(TCHAR);

	for (i = 0; i < numFrames; i++) {
		SymFromAddr( g_debugSession.m_hProcess, (DWORD64)(g_debugSession.m_pSymbolArray[i]), 0,  g_debugSession.m_pSymbolBuffer );

		Sys_Print( va("  %i: 0x%04x %s\n", numFrames - i - 1, g_debugSession.m_pSymbolBuffer->Address,  g_debugSession.m_pSymbolBuffer->Name) );
	}
#elif defined(POSIX)
	char *buffer;
    int numframes;
    uint64_t fileLength;

	if (com_errorEntered && g_debugSession.m_iErrorReason != ERR_NONE) {
        g_debugSession.m_bDoneErrorStackdump = true;
    }

    if (g_debugSession.m_pBTState) {
        int skip = 3; // skip this function in backtrace
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

        if (fileLength >= MAX_STACKTRACE_FILESIZE) {
			if (!FS_Initialized()) {
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
    }
#endif
}
