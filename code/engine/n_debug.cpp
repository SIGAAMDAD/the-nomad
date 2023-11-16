#include "n_shared.h"
#include <backtrace.h>
#include <cxxabi.h>
#include <execinfo.h>
#ifdef POSIX
#include <dlfcn.h>
#endif
#include <malloc.h>

class CDebugSession
{
public:
    CDebugSession( void );
    ~CDebugSession();

    char *m_pStacktraceBuffer;
    void **m_pSymbolArray;
    FILE *m_pBacktraceFile;

    struct backtrace_state *m_pBTState;
    const char *m_pExePath;
    unsigned char m_pAssertBuffer[1024];
    errorReason_t m_iErrorReason;
    bool m_bBacktraceError;
    bool m_bDoneErrorStackdump;
};

#ifdef POSIX
int register_crash_handler(const char * process_name, unsigned char * assert_buf_ptr);
#endif
static CDebugSession g_debugSession;

#define CXXDEMANGLE_MAX_NAME 4096
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
		char name[CXXDEMANGLE_MAX_NAME];
		size_t length = sizeof(name);
		abi::__cxa_demangle(symname, name, &length, &status);
		if (name[0]) {
			symname = name;
		}
		fprintf( stdout, "  %zu %s\n", pc, symname );
	} else {
		fprintf( stdout, "  %zu (unknown symbol)\n", pc );
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
		char name[CXXDEMANGLE_MAX_NAME];
		size_t length = sizeof(name);
		abi::__cxa_demangle(function, name, &length, &status);
		if (name[0]) {
			function = name;
		}

		const char* fileNameSrc = strstr(filename, "/src/");
		if (fileNameSrc != NULL) {
			filename = fileNameSrc+1; // I want "src/bla/blub.cpp:42"
		}
		fprintf( stdout, "  %zu %s:%d %s\n", pc, filename, lineno, function );
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
    m_pBTState = backtrace_create_state( m_pExePath, 0, bt_error_callback, NULL );

    if (m_pBTState) {
        int skip = 1; // skip this function in backtrace
		backtrace_simple(m_pBTState, skip, bt_simple_callback, bt_error_callback, NULL);
    }
    else {
        Con_DPrintf( COLOR_YELLOW "WARNING: failed to init libbacktrace, the symbols outputted will look very weird...\n" );
    }

    m_pBacktraceFile = Sys_FOpen( "backtrace.dat", "w+" );
    if (!m_pBacktraceFile) {
        Sys_Error( "Failed to open a backtrace file" );
    }

    m_pSymbolArray = (void **)malloc( sizeof(void *) * MAX_STACKTRACE_FRAMES );
    if (!m_pSymbolArray) {
        Sys_Error( "Failed to allocate sufficient memory for stacktrace frames" );
    }

    m_pStacktraceBuffer = (char *)malloc( MAX_STACKTRACE_FILESIZE );
    if (!m_pStacktraceBuffer) {
        Sys_Error( "Failed to allocate sufficient memory for stacktrace buffer" );
    }

    m_bBacktraceError = false;
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

    if (m_iErrorReason == ERR_NONE) { // could lead to more recursive errors
        if (m_pBacktraceFile) {
            fclose( m_pBacktraceFile );
        }
        if (m_pSymbolArray) {
            free( m_pSymbolArray );
        }
        if (m_pStacktraceBuffer) {
            free( m_pStacktraceBuffer );
        }
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

void Sys_DebugStacktrace( uint32_t frames )
{
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
}

#if 0

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/syscall.h>

struct crash_message_struct;

#define CRASH_MAX_PROCESS_NAME_SIZE (12)
#define CRASH_MAX_BACKTRACE_DEPTH (25)
#define CRASH_ASSERT_BUFFER_SIZE (128)
#define CRASH_MAX_MSG_SIZE (4096)
#define CRASH_MSG_MAGIC (0xdeadbeefUL)
#define CRASH_ANCILLARY_DATA_SIZE (CRASH_MAX_MSG_SIZE - sizeof(struct crash_message_struct))

/* Dead man switch: we wait this many seconds since first being tickled
 * until we continue processing crash, whether full crash information
 * has been received or not 
 * */
#define READ_TIMEOUT (10) 

void crashd_main(char daemonise_flag, const char * progname, int pfd[]);


struct crash_message_struct {
	/* Magic number */
	unsigned long magic;
	/* Process name as passed during registration */
	char process_name[CRASH_MAX_PROCESS_NAME_SIZE];
	/* The process PID */
	pid_t process_id;
	/* The process kernel thread id (struct task -> pid), NOT pthread_self() */
	pid_t thread_id; 
	/* The number of the exception signal */ 
	unsigned int signal_number;
	/* The signal code from siginfo_t. Provides exception reason */
	unsigned int signal_code;
	/* Fault address, if relevant */
	void * fault_address;
	/* The last error as reported via siginfo_t. Seems to be always 0 */
	unsigned int signal_errno;
	/* The last error in errno when the exception handler got called. */
	unsigned int handler_errno;
	/* Number of stack frames we got */
	size_t num_backtrace_frames;
	/* A time stamp */
	struct timespec timestamp;
	/* Buffer for assert data */
	unsigned char assert_buffer[CRASH_ASSERT_BUFFER_SIZE];
	/* Stack backtrace */
	void *backtrace[CRASH_MAX_BACKTRACE_DEPTH];
	/* Place holder for ancillary data, such as symbol trace sent as seperate message */
	char ancillary_data[1];
};


/* Asks this thread to dump. You can use this for asserts. */
static GDR_INLINE int crash_dump(void) {
		asm volatile ("" : : : "memory");
        return raise(SIGQUIT);
}

#ifdef USE_THREADS 

/* Ask some other thread to dump. You can use this for asserts. */
static GDR_INLINE int crash_dump_thread(pthread_t thread) {
	asm volatile ("" : : : "memory");
	return pthread_kill(thread, SIGUSR1);
}
#endif /* USE_THREADS */

/* The buffer holds the message + ancillary data, such as symbol stack traced
 * The pointer is used to cash the buffer to the header */
static char crash_msg_buf[CRASH_MAX_MSG_SIZE];
static struct crash_message_struct * crash_msg = (struct crash_message_struct *)&crash_msg_buf;

/* A simple compiler only memory barrier, both read and write */
#define mb(x) asm volatile ("" : : : "memory")

/* When this is set from SIGTERM signal handler it's 
 * time to terminate.
 * 
 * NOTE: It's a very good idea to kill crashd before any process 
 * relying on it for exception handling. 
 */
static char terminate_flag = 0;

/* When this is set from the SIGALRM signal handler 
 * it means our time to read crash details is out 
 */

static char timeout_flag = 0;

/* This translates a signal code into a readable string */
static GDR_INLINE const char * code2str(int code, int signal) {

	switch(code) {
		case SI_USER:
			return "kill, sigsend or raise ";
		case SI_KERNEL:
			return "kernel";
		case SI_QUEUE:
			return "sigqueue";
	}

	if(SIGILL==signal) switch(code) {
		case ILL_ILLOPC:
			return "illegal opcode";
		case ILL_ILLOPN:
			return "illegal operand";
		case ILL_ILLADR:
			return "illegal addressing mode";
		case ILL_ILLTRP:
			return "illegal trap";
		case ILL_PRVOPC:
			return "privileged register";
		case ILL_COPROC:
			return "coprocessor error";
		case ILL_BADSTK:
			return "internal stack error";
	}

	if(SIGFPE==signal) switch(code) {
		case FPE_INTDIV:
			return "integer divide by zero";
		case FPE_INTOVF:
			return "integer overflow";
		case FPE_FLTDIV:
			return "floating point divide by zero";
		case FPE_FLTOVF:
			return "floating point overflow";
		case FPE_FLTUND:
			return "floating point underflow";
		case FPE_FLTRES:
			return "floating point inexact result";
		case FPE_FLTINV:
			return "floating point invalid operation";
		case FPE_FLTSUB:
			return "subscript out of range";
	}

	if(SIGSEGV==signal) switch(code) {
		case SEGV_MAPERR:
			return "address not mapped to object";
		case SEGV_ACCERR:
			return "invalid permissions for mapped object";
	}

	if(SIGBUS==signal) switch(code) {
		case BUS_ADRALN:
			return "invalid address alignment";
		case BUS_ADRERR:
			return "non-existent physical address";
		case BUS_OBJERR:
			return "object specific hardware error";
	}

	if(SIGTRAP==signal) switch(code) {
		case TRAP_BRKPT:
			return "process breakpoint";
		case TRAP_TRACE:
			return "process trace trap";
	}

	return "Unhandled signal handler";
}

/* Call this to reboot. Production version must be asaync-signal safe */
static void GDR_INLINE do_reboot(void) {

#ifdef NDEBUG
	
	char * reboot_argv[] = { "reboot", NULL};
	char * reboot_env[] = {NULL};
	
	execve("/sbin/reboot", reboot_argv, reboot_env);

#else /* NDEBUG */

	Con_Printf("Would have rebooted but running in debug mode. Have a nice day.\n");
	exit(3);
	
#endif /* NDEBUG */

	/* NOT REACHED */
	return;
}

/* Handle the crash data
 * This is just an example: it speqs the entire message to stderr in human readable form
 */
static void handle_crash(void)
{
	int i;

	Assert(crash_msg != NULL);
	Assert(sizeof(crash_msg->assert_buffer[0])==sizeof(unsigned char));

	
	Con_Printf(
		"\n********************************"
		"\n*      EXCEPTION CAUGHT        *"
		"\n********************************\n"
		"Process name: %s\n"
		"Process ID: %d\n"
		"Thread ID: %d\n"
		"Exception: %s\n"
		"Reason: %s\n"
		"Fault Address: %p\n"
		"Signal error: %s\n"
		"Last error: %s\n"
		"Time stamp: %s\n"
		"Assert buffer: %s",
		crash_msg->process_name,
		crash_msg->process_id,
		crash_msg->thread_id,
		strsignal(crash_msg->signal_number),
		code2str(crash_msg->signal_code, crash_msg->signal_number),
		crash_msg->fault_address,
		strerror(crash_msg->signal_errno),
		strerror(crash_msg->handler_errno),
		ctime(&(crash_msg->timestamp.tv_sec)),
		crash_msg->assert_buffer
	);

	Sys_DebugStacktrace(MAX_STACKTRACE_FRAMES);
}

static void term_sig_handler(int signal) {
	terminate_flag = 1;
	mb();
}

/* Timeout reading crash data */
static void alarm_sig_handler(int signal) {
	timeout_flag = 1;
	mb();
}

/* Our very own fault handler.
 * If we ever got it it means something is very very wrong.
 * Trying to save debug info is useless. We probably got here
 * because of a fault when processing some crash. The chances
 * are very slim that we'll be able to save any meaningfull
 * data and we risk getting stuck instead of resetting the system,
 * so we just reboot 
 */
static void fault_sig_handler(int signal) {
	do_reboot();
}


/* Utility function to register a simple signal handler with no flags 
 * (as opposed to signal(2))
 */
static int register_signal(int signo, sighandler_t handler) {
	struct sigaction act;
	
	memset(&act, 0, sizeof (act));
	act.sa_handler = handler;
	sigemptyset (&act.sa_mask);
	act.sa_flags = 0;

	return sigaction (signo, &act, NULL);
}

/* The main deal */
void crashd_main(char daemonise_flag, const char * progname, int pfd[])
{
	int ret, fd;
	char *p = crash_msg_buf;
	int remaining_bytes = CRASH_MAX_MSG_SIZE;
	fd_set rfds;

	ret = fork();
	
	if (ret) {
		return;
	} else {
		close(pfd[1]);
		fd = pfd[0];
	}
	
	/* This forks again, closing stdin/out/err and loose our TTY, if asked to. */
	if (daemonise_flag) {
		ret = daemon(0, 1);
		if (ret == -1)
			goto bail_out;
	}	
	
	/* Register all signal handlers for timeout, kill and fault */
	ret = register_signal(SIGTERM, term_sig_handler);
	if(ret == -1)
		goto bail_out;
	
	ret = register_signal(SIGALRM, alarm_sig_handler);
	if(ret == -1)
		goto bail_out;
	
	ret = register_signal(SIGSEGV, fault_sig_handler);
	if(ret == -1)
		goto bail_out;
		
	ret = register_signal(SIGILL, fault_sig_handler);
	if(ret == -1)
		goto bail_out;
		
	ret = register_signal(SIGFPE, fault_sig_handler);
	if(ret == -1)
		goto bail_out;
	
	ret = register_signal(SIGBUS, fault_sig_handler);
	if(ret == -1)
		goto bail_out;
		

	/* OK, wait for someone to tickle us */
	
	FD_ZERO(&rfds);
	FD_SET(fd, &rfds);
	
	ret = select(fd+1, &rfds, NULL, NULL, NULL);
	
	/* Deal correctly with random harmless signals
	 * Especially useful for when we run under debugger */
	while(ret == -1 && EINTR == errno) {
		mb();
		if(terminate_flag)
			exit(0);

		ret = select(fd+1, &rfds, NULL, NULL, NULL);
	}
	
	if(ret == -1)
		goto bail_out;
	
	/* OK, we have action. First thing arm the timer */
	(void)alarm(READ_TIMEOUT);
	
	/* This crazy loop reads the message in, possbly in several parts.
	 * We continue when we're done or when it's time to leave. 
	 */
	
	do {
		ret = read(fd, p, remaining_bytes);

		/* We need to exit if the end closed the pipe or if we asked to terminate */
		if((0 == ret) || terminate_flag) {
			break;
		}

		/* Oh oh... we're late. Time out. */
		if(timeout_flag)
			break;
		
		/* Handle random signals nicely */ 
		if(ret == -1 && errno == EINTR)
			continue;
		
		/* Read errors make us nervous. log and bail out */
		if(ret == -1)
			break;
		
		p += ret;		
		remaining_bytes -= ret;
	} while (ret && (remaining_bytes > 0));
	
//	Assert(CRASH_MSG_MAGIC == crash_msg->magic);
	
	/* Make sure the process name has an ending NULL */
	crash_msg->process_name[CRASH_MAX_PROCESS_NAME_SIZE-1] = '\0';

	close(fd);

	/* Go process the crash */
	handle_crash();
	
	exit(0);
	
bail_out:
	/* Oy very... if we got here it means that the crash daemon has itself
	 * encountered some error. We simply record it in the usuall format and
	 * initaite a crash as normal. 
	 * 
	 * We don't bother with the backtrace symbols since there is only main here
	 * but we do put a meaningful error message as ancillary data.
	 * 
	 * Hope this never happens...
	 */

	strncpy(crash_msg->process_name, progname, CRASH_MAX_PROCESS_NAME_SIZE-1);
	crash_msg->process_id = getpid();
	crash_msg->thread_id = getpid();
	crash_msg->handler_errno = errno;
	clock_gettime(CLOCK_REALTIME, &crash_msg->timestamp);
	crash_msg->num_backtrace_frames=backtrace(crash_msg->backtrace, CRASH_MAX_BACKTRACE_DEPTH);
	snprintf(crash_msg->ancillary_data, CRASH_ANCILLARY_DATA_SIZE-1,
			"crashd bailing out due to %s\n",strerror(errno)); 
	
	handle_crash();

	// done
}

#ifdef USE_THREADS
#include <pthread.h>
#define _XOPEN_SOURCE 600
#include <sys/select.h>
#endif /* USE_THREADS */

/* Global static crash message buffer */
static struct crash_message_struct g_crash_msg;

/* Pipe file descriptor to crashd */
static int g_logfd = -1;

/* Pointer to global assert info, supplied during registration */
static unsigned char * g_assert_buf_ptr = NULL;

#ifdef USE_THREADS
/* Spinlock protecting access to the fault handler in multi-threaded setups */
static pthread_spinlock_t g_thread_lock;
#endif /* USE_THREADS */

/* gettid in non offical so not in glibc headers. This works though */
inline pid_t gettid (void)
{
    return syscall(__NR_gettid);
}

/* Get a backtrace from a signal handler.
 * array is place to put array
 * size is it's size
 * context is a pointer to the mysterious signal ahndler 3rd parameter with the registers 
 * distance is the distance is calls from the signal handler
 * 
 */
GDR_INLINE unsigned int signal_backtrace(void ** array, unsigned int  size, ucontext_t * context, unsigned int distance)
{
	/* WARNING: If you ever remove the inline from the function prototype, 
	 * adjust this to match!!!
	 */
#define IP_STACK_FRAME_NUMBER (3)
	
	unsigned int ret = backtrace(array, size);
	distance += IP_STACK_FRAME_NUMBER;
	
	Assert(distance <= size);
	
	/* OK, here is the tricky part:
	 * 
	 * Linux signal handling on some archs works by the kernel replacing, in situ, the 
	 * return address of the faulting function on the faulting thread user space stack with  
	 * that of the Glibc signal unwind handling routine and coercing user space to just to 
	 * glibc signal handler preamble. Later the signal unwind handling routine undo this.
	 * 
	 * What this means for us is that the backtrace we get is missing the single most important
	 * bit of information: the addres of the faulting function.
	 * 
	 * We get it back using the undocumented 3rs parameter to the signal handler call back
	 * with used in it's SA_SIGINFO form which contains access to the registers kept during 
	 * the fault. We grab the IP from there and 'fix' the backtrace. 
	 * 
	 * This needs to be different per arch, of course.
	 */ 
	
#ifdef __i386__
	  array[distance] = (void *)(context->uc_mcontext.gregs[REG_EIP]);
#endif /* __i386__ */

#ifdef __PPC__ 
	  array[distance] = (void *)(context->uc_mcontext.regs->nip);
#endif /* __PPC__ */
	  
	return ret;
}


/* The fault handler function.
 * 
 * OK. The rules of the battle are those:
 * 
 * 1. Can't use any function that relies on malloc and friends working as the malloc arena may be corrupt.
 * 2. Can only use a the POSIX.1-2003 list of async-safe functions.
 * 3. Some of the functions on the list are not always safe (like fork when atfork() is used), 
 *    so need to avoid these also.
 * 4. No locking allowed. We don't know in what state the process/thread was when the exception
 *    occured.
 */
void fault_handler (int signal, siginfo_t * siginfo, void *context)
{
	int i, ret;
	
#ifdef USE_THREADS
	
  ret = pthread_spin_trylock(&g_thread_lock);
  
  if (ret == EBUSY) {
	  /* Think of the following as an async-signal safe super sched_yield that
	   *  yields even to threads with lower real-time priority */
	  sigset_t smask;
	  sigemptyset(&smask);
	  pselect(0, NULL, NULL, NULL, NULL, &smask);
  }

#endif /* USE_THREADS */
	
	/* Get the backtrace. See signal_backtrace for the parameters */
	
	g_crash_msg.num_backtrace_frames = signal_backtrace(g_crash_msg.backtrace, 
			CRASH_MAX_BACKTRACE_DEPTH, (ucontext_t *)context, 0);
	
	/* Grab the kernel thread id. Because signal handler are shared between all
	 * threads of the same process, this can only be doen in fault time. */
	
	g_crash_msg.thread_id = gettid();
	
	/* Grab the signal number */
	g_crash_msg.signal_number = signal;
	
	/* Grab time stamp */
	clock_gettime(CLOCK_REALTIME, &g_crash_msg.timestamp);
	
	/* Copy the assert buffer without using strings.h fucntions. */
	for (i = 0; i < CRASH_ASSERT_BUFFER_SIZE; ++i) {
		g_crash_msg.assert_buffer[i] = *(g_assert_buf_ptr++);
	}
	
	if (siginfo) /* No reasons for this to be NULL, but still... */
	{	
		/* See description of these in crash_msg.h */
		g_crash_msg.signal_code = siginfo->si_code;
		g_crash_msg.fault_address = siginfo->si_addr;
		g_crash_msg.signal_errno = siginfo->si_errno;
		g_crash_msg.handler_errno = errno;
	}
	
retry_write:

	ret = write(g_logfd, &g_crash_msg, sizeof(g_crash_msg));

	/* If we got interrupt by a signal, retry the write.
	 * This shouldn't really happen since we mask all signals
	 * during the handler run via sigaction sa_mask field but
	 *  it can't hurt to test.
	 * 
	 * It's useless to test for any other condition since we 
	 * can't do anything if we fail 
	 */
	if (ret && errno == EINTR)
		goto retry_write;
	
	/* We use backtrace_symbols_fd rather then backtrace_symbols since
	 * the latter uses malloc to allocate memory and if we got here
	 * because of malloc arena curroption we'll double fault.
	 */
	backtrace_symbols_fd(g_crash_msg.backtrace, g_crash_msg.num_backtrace_frames, g_logfd);
	
	close(g_logfd);

	/* Produce a core dump for post morteum debugging */
	abort();
	
	Assert(0 /* Not Reached */);
}

/* Set the FD_CLOEXEC  flag of desc if value is nonzero,
	or clear the flag if value is 0.
	Return 0 on success, or -1 on error with errno  set. */ 
	
int set_cloexec_flag (int desc, int value)
{
	int oldflags = fcntl(desc, F_GETFD, 0);

	/* If reading the flags failed, return error indication now. */
	 if (oldflags < 0) {
		 return oldflags;
	 }

	 /* Set just the flag we want to set. */
	if (value != 0) {
		oldflags |= FD_CLOEXEC;
	} else {
		oldflags &= ~FD_CLOEXEC;
	}

	/* Store modified flag word in the descriptor. */
	return fcntl(desc, F_SETFD, oldflags);
}

/* Registration function. Needs to be called once by each process (not thread)
 * process_name is argv[0] or whatever you'd like.
 * assert_buf_ptr needs to point to the 128 byte  assert buffer.
 * */
int register_crash_handler(const char * process_name, unsigned char * assert_buf_ptr)
{
  struct sigaction act;	/* Signal handler register struct */
  int ret;				/* Return value for various calls */
  int pfd[2];			/* Pipe file descriptor array */

  /* See ahead about these two: */
  void * dummy_trace_array[1];
  unsigned int dummy_trace_size;

  Assert(sizeof(g_crash_msg) <= CRASH_MAX_MSG_SIZE);
  
  if(!process_name || !assert_buf_ptr) {
	  return EINVAL;
  }
  
#ifdef USE_THREADS
  
  ret = pthread_spin_init(&g_thread_lock, 0);
  if(ret) {
	  return ret;
  }
#endif /* USE_THREADS */

  
  /* If we're called again (perhaps after a fork() ), the pipe is already open.
   * That's just fine with us */
  
  if(g_logfd == -1) {

	/* Grab us a pipe to communicate with our crash daemon */
	ret = pipe(pfd);
	
	if(ret == -1) {
		return errno;
	}
	
	g_logfd = pfd[1]; /* Grab the write end of the pipe */
	
	/* If the caller program execs, we want the pipe to close,
	 * because it's not likely a random program will have the
	 * right signal handler set to use the crash daemon. */
	ret = set_cloexec_flag(g_logfd, 1);
	
	if(ret == -1) {
		return errno;
	}
	
	/* Set our daemon up */
	crashd_main(1, process_name, pfd);
	
	close(pfd[0]);

  }
  
  /* This requires some explaining:
   * In theory, neither backtrace nor backtrace_symbold_fd call malloc and friends so
   * we are able to use them in a an exception handler safely.
   * 
   * In practice recent glibc versions put these function in a seperate shared library
   * called libgcc_s.so when gets loaded automagically by the dynamic linker when any these
   * of these functions are first used and, you guessed it, the dynamic linker uses malloc
   * in the process to get some internal buffer.
   * 
   * We therefore give these a dummy call here during registration to assure that the library 
   * gets loaded where it's safe to malloc. 
   */
  
  dummy_trace_size = backtrace(dummy_trace_array, 1);
  backtrace_symbols_fd (dummy_trace_array, dummy_trace_size, -1);
  
  /* This data we can already grab during registration, not need to wait for crash */
  g_crash_msg.magic = CRASH_MSG_MAGIC;
  memcpy(g_crash_msg.process_name, process_name, strnlen(process_name, CRASH_MAX_PROCESS_NAME_SIZE)+1);
  g_crash_msg.process_id = getpid();
  
  g_assert_buf_ptr = assert_buf_ptr;
  
  /* Prepare a sigaction struct for exception handler registrations */
  memset(&act, 0, sizeof (act));
  act.sa_sigaction = fault_handler;
  /* No signals during handler run, please */
  sigfillset (&act.sa_mask);
  /* We want the 3 parameter form of the handler with the siginfo_t addtional data */
  act.sa_flags = SA_SIGINFO;

  
 /* Register the handler for all exception signals. */
  ret = sigaction (SIGSEGV, &act, NULL); 
  ret |= sigaction (SIGILL, &act, NULL);
  ret |= sigaction (SIGFPE, &act, NULL);  
  ret |= sigaction (SIGBUS, &act, NULL);
  ret |= sigaction (SIGQUIT, &act, NULL);

  return ret;                                                                                                        
}


#endif