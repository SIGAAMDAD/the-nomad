#include "../engine/n_shared.h"
#include "../engine/n_common.h"
#include "../game/g_game.h"
#include "../game/g_threads.h"
#include "unix_local.h"
#include <wait.h>
#include <stdarg.h>

#define SYS_BACKTRACE_MAX 1024
#define MEM_THRESHOLD (96*1024*1024)

static field_t tty_con;
static int stdin_flags;
static struct termios tty_tc;
static qboolean stdin_active;
static qboolean ttycon_active;
static int ttycon_hide;
static int tty_erase;
static int tty_eof;
static qboolean ttycon_on = qfalse;
static qboolean ttycon_color_on = qfalse;
static cvar_t *ttycon;
static cvar_t *ttycon_ansicolor;
extern fileHandle_t logfile;

typedef enum {
	TTY_ENABLED,
	TTY_DISABLED,
	TTY_ERROR
} tty_err;

tty_err Sys_ConsoleInputInit( void );

bool Sys_IsInDebugSession( void )
{
	char buf[4096];
	int status_fd;
	ssize_t readCount;
	constexpr const char tracerPidString[] = "TracerPid:";
	const char *tracer_pid_ptr, *characterPtr;

	status_fd = open("/proc/self/status", O_RDONLY);
	if (status_fd == -1)
		return false;

	readCount = read(status_fd, buf, sizeof(buf) - 1);
	close(status_fd);

	if (readCount <= 0)
		return false;

	buf[readCount] = '\0';
	tracer_pid_ptr = strstr(buf, tracerPidString);
	if (!tracer_pid_ptr)
		return false;

	for (characterPtr = tracer_pid_ptr + sizeof(tracerPidString) - 1; characterPtr <= buf + readCount; ++characterPtr) {
		if (isspace(*characterPtr))
			continue;
		else
			return isdigit(*characterPtr) != 0 && *characterPtr != '0';
	}

	return false;
}

// flush stdin, I suspect some terminals are sending a LOT of shit
static void tty_FlushIn( void )
{
	tcflush( STDIN_FILENO, TCIFLUSH );
}

// do a backspace
// TTimo NOTE: it seems on some terminals just sending '\b' is not enough
//   so for now, in any case we send "\b \b" .. yeah well ..
//   (there may be a way to find out if '\b' alone would work though)
static void tty_Back( void )
{
	write( STDOUT_FILENO, "\b \b", 3 );
}

// clear the display of the line currently edited
// bring cursor back to beginning of line
void tty_Hide( void )
{
	int i;

	if ( !ttycon_on )
		return;

	if ( ttycon_hide )
	{
		ttycon_hide++;
		return;
	}

	if ( tty_con.cursor > 0 )
	{
		for ( i = 0; i < tty_con.cursor; i++ )
		{
			tty_Back();
		}
	}
	tty_Back(); // delete "]" ? -EC-
	ttycon_hide++;
}


// show the current line
// FIXME TTimo need to position the cursor if needed??
void tty_Show( void )
{
	if ( !ttycon_on )
		return;

	if ( ttycon_hide > 0 )
	{
		ttycon_hide--;
		if ( ttycon_hide == 0 )
		{
			write( STDOUT_FILENO, "]", 1 ); // -EC-

			if ( tty_con.cursor > 0 )
			{
				write( STDOUT_FILENO, tty_con.buffer, tty_con.cursor );
			}
		}
	}
}

qboolean Sys_LowPhysicalMemory( void )
{
#if 1
	uint64_t pageSize = sysconf( _SC_PAGESIZE );
	uint64_t numPhysPages = sysconf( _SC_AVPHYS_PAGES );
	return ( pageSize * numPhysPages ) < MEM_THRESHOLD ? qtrue : qfalse;
#else
	struct statfs buf;
	size_t memLeft;

	if (statfs( "/", &buf ) == -1) {
		N_Error( ERR_FATAL, "statfs() failed, strerror(): %s", strerror( errno ) );
	}
	return (buf.f_bsize * buf.f_bfree) > MEM_THRESHOLD ? qtrue : qfalse;
#endif
}

const exittype_t signals[] = {
	{ SIGSEGV,  qfalse, "segmentation violation" },
	{ SIGBUS,   qfalse, "bus error" },
	{ SIGABRT,  qfalse, "abnormal program termination" },
	{ SIGSTOP,  qtrue,  "pausing program" },
	{ SIGTERM,  qtrue,  "program termination" },
	{ SIGILL,   qtrue,  "illegal instruction" },
	{ SIGTRAP,  qtrue,  "debug breakpoint" },
	{ 0,        qtrue,  "No System Error "}
};

const exittype_t *exit_type;

//
// Sys_MessageBox: adapted slightly from the source engine
//
int Sys_MessageBox( const char *title, const char *text, bool ShowOkAndCancelButton )
{
	extern SDL_Window *SDL_window;

	int buttonid = 0;
	SDL_MessageBoxData boxData = { 0 };
	SDL_MessageBoxButtonData buttonData[] = {
		{ SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT, 1, "OK"      },
		{ SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT, 0, "Cancel"  },
	};

	boxData.window = SDL_window;
	boxData.title = title;
	boxData.message = text;
	boxData.numbuttons = ShowOkAndCancelButton ? 2 : 1;
	boxData.buttons = buttonData;

	SDL_ShowMessageBox( &boxData, &buttonid );
	return ( buttonid == 1 );
}

static qboolean errorCaught = qfalse;

static char execBuffer[ 1024 ];
static char *execBufferPointer;
static char *execArgv[ 16 ];
static int execArgc;

/*
==============
Sys_ClearExecBuffer
==============
*/
static void Sys_ClearExecBuffer( void )
{
	execBufferPointer = execBuffer;
	memset( execArgv, 0, sizeof( execArgv ) );
	execArgc = 0;
}

/*
==============
Sys_AppendToExecBuffer
==============
*/
static void Sys_AppendToExecBuffer( const char *text )
{
	size_t size = sizeof( execBuffer ) - ( execBufferPointer - execBuffer );
	int length = strlen( text ) + 1;

	if ( length > size || execArgc >= arraylen( execArgv ) ) {
		return;
	}

	N_strncpyz( execBufferPointer, text, size );
	execArgv[ execArgc++ ] = execBufferPointer;

	execBufferPointer += length;
}

/*
==============
Sys_Exec
==============
*/
static int Sys_Exec( void )
{
	pid_t pid = fork( );

	if ( pid < 0 ) {
		return -1;
	}

	if ( pid ) {
		// Parent
		int exitCode;

		wait( &exitCode );

		return WEXITSTATUS( exitCode );
	}
	else {
		// Child
		execvp( execArgv[ 0 ], execArgv );

		// Failed to execute
		exit( -1 );

		return -1;
	}
}

/*
==============
Sys_ZenityCommand
==============
*/
static void Sys_ZenityCommand( dialogType_t type, const char *message, const char *title )
{
	Sys_ClearExecBuffer();
	Sys_AppendToExecBuffer( "zenity" );

	switch ( type ) {
	default:
	case DT_INFO:      Sys_AppendToExecBuffer( "--info" ); break;
	case DT_WARNING:   Sys_AppendToExecBuffer( "--warning" ); break;
	case DT_ERROR:     Sys_AppendToExecBuffer( "--error" ); break;
	case DT_YES_NO:
		Sys_AppendToExecBuffer( "--question" );
		Sys_AppendToExecBuffer( "--ok-label=Yes" );
		Sys_AppendToExecBuffer( "--cancel-label=No" );
		break;
	case DT_OK_CANCEL:
		Sys_AppendToExecBuffer( "--question" );
		Sys_AppendToExecBuffer( "--ok-label=OK" );
		Sys_AppendToExecBuffer( "--cancel-label=Cancel" );
		break;
	};

	Sys_AppendToExecBuffer( va( "--text=%s", message ) );
	Sys_AppendToExecBuffer( va( "--title=%s", title ) );
}

/*
==============
Sys_KdialogCommand
==============
*/
static void Sys_KdialogCommand( dialogType_t type, const char *message, const char *title )
{
	Sys_ClearExecBuffer( );
	Sys_AppendToExecBuffer( "kdialog" );

	switch( type ) {
	default:
	case DT_INFO:      Sys_AppendToExecBuffer( "--msgbox" ); break;
	case DT_WARNING:   Sys_AppendToExecBuffer( "--sorry" ); break;
	case DT_ERROR:     Sys_AppendToExecBuffer( "--error" ); break;
	case DT_YES_NO:    Sys_AppendToExecBuffer( "--warningyesno" ); break;
	case DT_OK_CANCEL: Sys_AppendToExecBuffer( "--warningcontinuecancel" ); break;
	};

	Sys_AppendToExecBuffer( message );
	Sys_AppendToExecBuffer( va( "--title=%s", title ) );
}

/*
==============
Sys_XmessageCommand
==============
*/
static void Sys_XmessageCommand( dialogType_t type, const char *message, const char *title )
{
	Sys_ClearExecBuffer( );
	Sys_AppendToExecBuffer( "xmessage" );
	Sys_AppendToExecBuffer( "-buttons" );

	switch ( type ) {
	default:           Sys_AppendToExecBuffer( "OK:0" ); break;
	case DT_YES_NO:    Sys_AppendToExecBuffer( "Yes:0,No:1" ); break;
	case DT_OK_CANCEL: Sys_AppendToExecBuffer( "OK:0,Cancel:1" ); break;
	};

	Sys_AppendToExecBuffer( "-center" );
	Sys_AppendToExecBuffer( message );
}

/*
==============
Sys_Dialog

Display a *nix dialog box
==============
*/
dialogResult_t Sys_Dialog( dialogType_t type, const char *message, const char *title )
{
	typedef enum
	{
		NONE = 0,
		ZENITY,
		KDIALOG,
		XMESSAGE,
		NUM_DIALOG_PROGRAMS
	} dialogCommandType_t;
	typedef void (*dialogCommandBuilder_t)( dialogType_t, const char *, const char * );

	const char              *session = getenv( "DESKTOP_SESSION" );
	qboolean                tried[ NUM_DIALOG_PROGRAMS ] = { qfalse };
	dialogCommandBuilder_t  commands[ NUM_DIALOG_PROGRAMS ] = { NULL };
	dialogCommandType_t     preferredCommandType = NONE;
	int                     i;

	commands[ ZENITY ] = &Sys_ZenityCommand;
	commands[ KDIALOG ] = &Sys_KdialogCommand;
	commands[ XMESSAGE ] = &Sys_XmessageCommand;

	// This may not be the best way
	if ( !N_stricmp( session, "gnome" ) ) {
		preferredCommandType = ZENITY;
	} else if ( !N_stricmp( session, "kde" ) ) {
		preferredCommandType = KDIALOG;
	}

	for( i = NONE + 1; i < NUM_DIALOG_PROGRAMS; i++ )
	{
		if ( preferredCommandType != NONE && preferredCommandType != i ) {
			continue;
		}

		if ( !tried[ i ] ) {
			int exitCode;

			commands[ i ]( type, message, title );
			exitCode = Sys_Exec( );

			if ( exitCode >= 0 ) {
				switch ( type ) {
				case DT_YES_NO:    return exitCode ? DR_NO : DR_YES;
				case DT_OK_CANCEL: return exitCode ? DR_CANCEL : DR_OK;
				default:           return DR_OK;
				};
			}

			tried[ i ] = qtrue;

			// The preference failed, so start again in order
			if ( preferredCommandType != NONE ) {
				preferredCommandType = NONE;
				i = NONE + 1;
			}
		}
	}

	Con_DPrintf( COLOR_YELLOW "WARNING: failed to show a dialog\n" );
	return DR_OK;
}

static void Sys_CrashPrintf( int fd, const char *fmt, ... )
{
	va_list argptr;
	char str[ 1024 ];

	va_start( argptr, fmt );
	vsnprintf( str, sizeof( str ) - 1, fmt, argptr );
	va_end( argptr );

	write( fd, str, strlen( str ) );
}

void Sys_WriteCrashReport( int fd )
{
	char msg[ MAX_CVAR_VALUE ];
	char timestr[32];
	struct tm *t;
	time_t ctime;
	cvar_t *var;
	extern cvar_t *cvar_vars;

	ctime = time( NULL );
	t = localtime( &ctime );
	strftime( timestr, sizeof( timestr ) - 1, "%d%m%Y_%S%M%H", t );

	Cvar_VariableStringBuffer( "com_errorMessage", msg, sizeof( msg ) - 1 );
	
	strftime( timestr, sizeof( timestr ) - 1, "%d/%m/%Y %S:%M:%H", t );

	Sys_CrashPrintf( fd, "[ERROR INFO]\n" );
	Sys_CrashPrintf( fd, "Time: %s\n", timestr );
	Sys_CrashPrintf( fd, "Message: %s\n", msg );
	if ( gi.state == GS_LEVEL ) {
		Sys_CrashPrintf( fd, "MapName: %s\n", gi.mapCache.info.name );
		Sys_CrashPrintf( fd, "SaveName: %s\n", Cvar_VariableString( "sgame_SaveName" ) );
	}

	Sys_CrashPrintf( fd, "[ENGINE INFO]\n" );
	Sys_CrashPrintf( fd, "Version String: " GLN_VERSION "\n" );
	Sys_CrashPrintf( fd, "Architecture: " ARCH_STRING "\n" );
	Sys_CrashPrintf( fd, "Operating System: " OS_STRING "\n" );
	Sys_CrashPrintf( fd, "CPU: %s\n", sys_cpuString->s );
	Sys_CrashPrintf( fd, "Build Information:\n" );
	Sys_CrashPrintf( fd, "\tC++ Version: %li\n", __cplusplus );
	Sys_CrashPrintf( fd, "\tCompiler: " COMPILER_STRING "\n" );
	Sys_CrashPrintf( fd, "\tCompiler Version: %lu\n", (uint64_t)EA_COMPILER_VERSION );
	Sys_CrashPrintf( fd, "Compiler Macros:\n" );
#ifdef USE_CPU_AFFINITY
	Sys_CrashPrintf( fd, "\tUSE_CPU_AFFINITY\n" );
#endif
#ifdef USE_OPENGL_API
	Sys_CrashPrintf( fd, "\tUSE_OPENGL_API\n" );
#endif
#ifdef USE_VULKAN_API
	Sys_CrashPrintf( fd, "\tUSE_VULKAN_API\n" );
#endif
	Sys_CrashPrintf( fd, "\n" );

	Sys_CrashPrintf( fd, "[CVAR INFO]\n" );
	for ( var = cvar_vars; var; var = var->next ) {
		if ( !var->name ) {
			continue;
		}
		else if ( var->name && !N_stricmp( var->name, "com_errorMessage" ) ) {
			continue; // don't print this redundantly
		}

		if ( var->flags & CVAR_SERVERINFO ) {
			Sys_CrashPrintf( fd, "S" );
		} else {
			Sys_CrashPrintf( fd, " " );
		}
		if ( var->flags & CVAR_SYSTEMINFO ) {
			Sys_CrashPrintf( fd, "s" );
		} else {
			Sys_CrashPrintf( fd, " " );
		}
		if ( var->flags & CVAR_USERINFO ) {
			Sys_CrashPrintf( fd, "U" );
		} else {
			Sys_CrashPrintf( fd, " " );
		}
		if ( var->flags & CVAR_ROM ) {
			Sys_CrashPrintf( fd, "R" );
		} else {
			Sys_CrashPrintf( fd, " " );
		}
		if ( var->flags & CVAR_INIT ) {
			Sys_CrashPrintf( fd, "I" );
		} else {
			Sys_CrashPrintf( fd, " ");
		}
		if ( var->flags & CVAR_SAVE ) {
			Sys_CrashPrintf( fd, "A" );
		} else {
			Sys_CrashPrintf( fd, " " );
		}
		if ( var->flags & CVAR_LATCH ) {
			Sys_CrashPrintf( fd, "L" );
		} else {
			Sys_CrashPrintf( fd, " " );
		}
		if ( var->flags & CVAR_CHEAT ) {
			Sys_CrashPrintf( fd, "C" );
		} else {
			Sys_CrashPrintf( fd, " " );
		}
		if ( var->flags & CVAR_USER_CREATED ) {
			Sys_CrashPrintf( fd, "?" );
		} else {
			Sys_CrashPrintf( fd, " " );
		}

		Sys_CrashPrintf( fd, " %s \"%s\"\n", var->name, var->s );
	}

	Sys_CrashPrintf( fd, "\n" );
	Sys_CrashPrintf( fd, "[FILESYSTEM INFO]\n" );
	Sys_CrashPrintf( fd, "Referenced BFFs: %s\n", FS_ReferencedBFFNames() );
	Sys_CrashPrintf( fd, "Loaded BFFs: %s\n", FS_LoadedBFFNames() );
}

/*
==============
Sys_ErrorDialog

Display an error message
==============
*/
void Sys_ErrorDialog( const char *error )
{
	char buffer[ 1024 ];
	unsigned int size;
	int f = -1;
	const char *homepath = Cvar_VariableString( "fs_homepath" );
	const char *gamedir = Cvar_VariableString( "fs_game" );
	const char *fileName = "crashlog.txt";
	char *dirpath = FS_BuildOSPath( homepath, gamedir, "");
	char *ospath = FS_BuildOSPath( homepath, gamedir, fileName );

	Sys_Print( va( "%s\n", error ) );

	Sys_Dialog( DT_ERROR, va( "%s. See \"%s\" for details.", error, ospath ), "Error" );

	// Make sure the write path for the crashlog exists...

	if ( !Sys_mkdir( homepath ) ) {
		Con_Printf( "ERROR: couldn't create path '%s' for crash log.\n", homepath );
		return;
	}

	if ( !Sys_mkdir( dirpath ) ) {
		Con_Printf( "ERROR: couldn't create path '%s' for crash log.\n", dirpath );
		return;
	}

	// We might be crashing because we maxed out the Quake MAX_FILE_HANDLES,
	// which will come through here, so we don't want to recurse forever by
	// calling FS_FOpenFileWrite()...use the Unix system APIs instead.
	f = open( ospath, O_CREAT | O_TRUNC | O_WRONLY, 0640 );
	if ( f == -1 ) {
		Con_Printf( "ERROR: couldn't open %s\n", fileName );
		return;
	}
	Sys_WriteCrashReport( f );

	close( f );
}

void GDR_NORETURN GDR_ATTRIBUTE((format(printf, 1, 2))) GDR_DECL Sys_Error( const char *fmt, ... )
{
	va_list argptr;
	char text[MAXPRINTMSG];
	const char *msg;

	// change stdin to non blocking
	// NOTE TTimo not sure how well that goes with tty console mode
	if ( stdin_active ) {
//		fcntl( STDIN_FILENO, F_SETFL, fcntl( STDIN_FILENO, F_GETFL, 0) & ~FNDELAY );
		fcntl( STDIN_FILENO, F_SETFL, stdin_flags );
	}

	// don't bother do a show on this one heh
	if ( ttycon_on ) {
		tty_Hide();
	}

	va_start( argptr, fmt );
	N_vsnprintf( text, sizeof( text ), fmt, argptr );
	va_end( argptr );

	Sys_DebugStacktrace( MAX_STACKTRACE_FRAMES );
	
	// fprintf COULD call malloc, and if we're out of memory, this would not do anything
	// but make even more problems
//	fprintf( stderr, "Sys_Error: %s\n", text );

	Sys_ErrorDialog( text );

	Sys_Exit( -1 ); // bk010104 - use single exit point.
}

// never exit without calling this, or your terminal will be left in a pretty bad state
void Sys_ConsoleInputShutdown( void )
{
	if ( ttycon_on ) {
//		Con_Printf( "Shutdown tty console\n" ); // -EC-
		tty_Back(); // delete "]" ? -EC-
		tcsetattr( STDIN_FILENO, TCSADRAIN, &tty_tc );
	}

	// Restore blocking to stdin reads
	if ( stdin_active ) {
		fcntl( STDIN_FILENO, F_SETFL, stdin_flags );
//		fcntl( STDIN_FILENO, F_SETFL, fcntl( STDIN_FILENO, F_GETFL, 0 ) & ~O_NONBLOCK );
	}

	memset( &tty_con, 0, sizeof( tty_con ) );

	stdin_active = qfalse;
	ttycon_on = qfalse;

	ttycon_hide = 0;
}

/*
==============
Sys_PID
==============
*/
int Sys_PID( void )
{
	return getpid( );
}

/*
==============
Sys_PIDIsRunning
==============
*/
qboolean Sys_PIDIsRunning( int pid )
{
	return kill( pid, 0 ) == 0;
}

/*
=================
Sys_WritePIDFile

Return qtrue if there is an existing stale PID file
=================
*/
static qboolean Sys_WritePIDFile( const char *gamedir )
{
	const char	*pidFile = Sys_PIDFileName( gamedir );
	FILE		*f;
	qboolean	stale = qfalse;

	if ( pidFile == NULL ) {
		return qfalse;
	}

	// First, check if the pid file is already there
	if ( ( f = fopen( pidFile, "r" ) ) != NULL ) {
		char  pidBuffer[ 64 ] = { 0 };
		int   pid;

		pid = fread( pidBuffer, sizeof( char ), sizeof( pidBuffer ) - 1, f );
		fclose( f );

		if ( pid > 0 ) {
			pid = atoi( pidBuffer );
			if ( !Sys_PIDIsRunning( pid ) ) {
				stale = qtrue;
			}
		}
		else {
			stale = qtrue;
		}
	}

	if ( FS_CreatePath( pidFile ) ) {
		return 0;
	}

	if ( ( f = fopen( pidFile, "w" ) ) != NULL ) {
		fprintf( f, "%d", Sys_PID() );
		fclose( f );
	}
	else {
		Con_Printf( COLOR_YELLOW "Couldn't write %s.\n", pidFile );
	}

	return stale;
}

/*
=================
Sys_InitPIDFile
=================
*/
void Sys_InitPIDFile( const char *gamedir ) {
	if ( Sys_WritePIDFile( gamedir ) ) {
		char message[1024];
		extern qboolean com_safeMode;

		Com_snprintf( message, sizeof( message ) - 1, "The last time the-nomad ran, "
			"it didn't exit properly. This may be due to inappropriate "
			"settings. Would you like to start with \"safe\" settings?" );

		if ( Sys_Dialog( DT_YES_NO, message, "Abnormal Exit" ) == DR_YES ) {
			if ( Com_SafeMode() ) {
				return; // already set
			}
			com_safeMode = qtrue;
		}
	}
}

void GDR_NORETURN Sys_Exit( int code )
{
	Sys_ConsoleInputShutdown();

	if ( code != -1 && com_fullyInitialized ) {
		// normal exit
		Sys_RemovePIDFile( FS_GetCurrentGameDir() );
	}
	
	exit( code );
}

/*
=================
Sys_SendKeyEvents

Platform-dependent event handling
=================
*/
void Sys_SendKeyEvents( void )
{
	HandleEvents();
}

void fpe_exception_handler( int signum )
{
	signal( SIGFPE, fpe_exception_handler );
}

/*
==================
CON_SigCont
Reinitialize console input after receiving SIGCONT, as on Linux the terminal seems to lose all
set attributes if user did CTRL+Z and then does fg again.
==================
*/
void CON_SigCont( int signum )
{
	Sys_ConsoleInputInit();
}

/*
==================
Sys_Sleep

Block execution for msec or until input is received.
==================
*/
void Sys_Sleep( int msec ) {
	struct timeval timeout;
	fd_set fdset;
	int res;

	//if ( msec == 0 )
	//	return;

	if ( msec < 0 ) {
		// special case: wait for console input or network packet
		if ( stdin_active ) {
			msec = 300;
			do {
				FD_ZERO( &fdset );
				FD_SET( STDIN_FILENO, &fdset );
				timeout.tv_sec = msec / 1000;
				timeout.tv_usec = (msec % 1000) * 1000;
				res = select( STDIN_FILENO + 1, &fdset, NULL, NULL, &timeout );
			} while ( res == 0 );
		} else {
			// can happen only if no map loaded
			// which means we totally stuck as stdin is also disabled :P
			//usleep( 300 * 1000 );
		}
		return;
	}
#if 1
	usleep( msec * 1000 );
#else
	if ( com_dedicated->integer && stdin_active ) {
		FD_ZERO( &fdset );
		FD_SET( STDIN_FILENO, &fdset );
		timeout.tv_sec = msec / 1000;
		timeout.tv_usec = (msec % 1000) * 1000;
		select( STDIN_FILENO + 1, &fdset, NULL, NULL, &timeout );
	} else {
		usleep( msec * 1000 );
	}
#endif
}

static const struct Q3ToAnsiColorTable_s {
	const char Q3color;
	const char *ANSIcolor;
} tty_colorTable[ ] = {
	{ S_COLOR_BLACK,    "30" },
	{ S_COLOR_RED,      "31" },
	{ S_COLOR_GREEN,    "32" },
	{ S_COLOR_YELLOW,   "33" },
	{ S_COLOR_BLUE,     "34" },
	{ S_COLOR_CYAN,     "36" },
	{ S_COLOR_MAGENTA,  "35" },
	{ S_COLOR_WHITE,    "0" }
};

static const char *getANSIcolor( char Q3color ) {
	int i;

	for ( i = 0; i < arraylen( tty_colorTable ); i++ ) {
		if ( Q3color == tty_colorTable[ i ].Q3color ) {
			return tty_colorTable[ i ].ANSIcolor;
		}
	}
	return NULL;
}

void Sys_ShutdownConsole( void )
{
	if ( ttycon_on ) {
		tty_Back();
		tcsetattr( STDIN_FILENO, TCSADRAIN, &tty_tc );
	}

	// restore stdin blocking mode
	if ( stdin_active ) {
		fcntl( STDIN_FILENO, F_SETFL, stdin_flags );
	}

	memset( &tty_con, 0, sizeof( tty_con ) );

	stdin_active = qfalse;
	ttycon_on = qfalse;

	ttycon_hide = 0;
}

void CON_SigTStp( int signum )
{
	sigset_t mask;

	tty_FlushIn();
	Sys_ConsoleInputShutdown();

	sigemptyset( &mask );
	sigaddset( &mask, SIGTSTP );
	sigprocmask( SIG_UNBLOCK, &mask, NULL );

	signal( SIGTSTP, SIG_DFL );

	kill( getpid(),  SIGTSTP );
}


void Sys_SigCont( int signum )
{
	Sys_ConsoleInputInit();
}

void Sys_SigTStp( int signum )
{
	sigset_t mask;

	tty_FlushIn();
	Sys_ShutdownConsole();

	sigemptyset( &mask );
	sigaddset( &mask, SIGTSTP );
	sigprocmask( SIG_UNBLOCK, &mask, NULL );

	signal( SIGTSTP, SIG_DFL );

	kill( getpid(), SIGTSTP );
}

static qboolean printableChar( char c ) {
	if ( ( c >= ' ' && c <= '~' ) || c == '\n' || c == '\r' || c == '\t' ) {
		return qtrue;
	}
	return qfalse;
}

void Sys_ANSIColorMsg( const char *msg, char *buffer, uint64_t bufSize )
{
	uint64_t msgLength;
	uint64_t i;
	char tmpbuf[8];
	const char *ANSIcolor;

	if ( !msg || !buffer ) {
		return;
	}

	msgLength = strlen( msg );
	i = 0;
	buffer[0] = '\0';

	while ( i < msgLength ) {
		if ( msg[i] == '\n' ) {
			snprintf( tmpbuf, sizeof( tmpbuf ), "%c[0m\n", 0x1B );
			strncat( buffer, tmpbuf, bufSize - 1 );
			i += 1;
		}
		else if ( msg[i] == Q_COLOR_ESCAPE && ( ANSIcolor = getANSIcolor( msg[i+1] ) ) != NULL ) {
			snprintf( tmpbuf, sizeof( tmpbuf ), "%c[%sm", 0x1B, ANSIcolor );
			strncat( buffer, tmpbuf, bufSize - 1 );
			i += 2;
		}
		else {
			if ( printableChar( msg[i] ) ) {
				snprintf( tmpbuf, sizeof( tmpbuf ), "%c", msg[i] );
				strncat( buffer, tmpbuf, bufSize - 1 );
			}
			i += 1;
		}
	}
}

void Sys_Print( const char *msg )
{
	char printmsg[MAXPRINTMSG];
	size_t len;

	memset( printmsg, 0, sizeof( printmsg ) );

	if ( ttycon_on ) {
		tty_Hide();
	}
	if ( ttycon_color_on ) {
		Sys_ANSIColorMsg( msg, printmsg, sizeof( printmsg ) );
		len = strlen( printmsg );
	}
	else {
		char *out = printmsg;
		while ( *msg != '\0' && out < printmsg + sizeof( printmsg ) ) {
			if ( printableChar( *msg ) ) {
				*out++ = *msg;
			}
			msg++;
		}
		len = out - printmsg;
	}

	write( STDERR_FILENO, printmsg, len );
	if ( ttycon_on ) {
		tty_Show();
	}
}

char *Sys_ConsoleInput( void )
{
	// we use this when sending back commands
	static char text[ sizeof( tty_con.buffer ) ];
	int avail;
	char key;
	char *s;
	field_t history;

	if ( ttycon_on ) {
		avail = read( STDIN_FILENO, &key, 1 );
		if ( avail != -1 ) {
			// we have something
			// backspace?
			// NOTE TTimo testing a lot of values .. seems it's the only way to get it to work everywhere
			if ( ( key == tty_erase ) || ( key == 127 ) || ( key == 8 ) ) {
				if ( tty_con.cursor > 0 ) {
					tty_con.cursor--;
					tty_con.buffer[tty_con.cursor] = '\0';
					tty_Back();
				}
				return NULL;
			}

			// check if this is a control char
			if ( key && key < ' ' ) {
				if ( key == '\n' ) {
					// push it in history
					Con_SaveField( &tty_con );
					s = tty_con.buffer;
					while ( *s == '\\' || *s == '/' ) // skip leading slashes
						s++;
					N_strncpyz( text, s, sizeof( text ) );
					Field_Clear( &tty_con );
					write( STDOUT_FILENO, "\n]", 2 );
					return text;
				}

				if ( key == '\t' ) {
					tty_Hide();
					Field_AutoComplete( &tty_con );
					tty_Show();
					return NULL;
				}

				avail = read( STDIN_FILENO, &key, 1 );
				if (avail != -1) {
					// VT 100 keys
					if (key == '[' || key == 'O') {
						avail = read( STDIN_FILENO, &key, 1 );
						if (avail != -1) {
							switch (key) {
							case 'A':
								if ( Con_HistoryGetPrev( &history ) ) {
									tty_Hide();
									tty_con = history;
									tty_Show();
								}
								tty_FlushIn();
								return NULL;
								break;
							case 'B':
								if ( Con_HistoryGetNext( &history ) ) {
									tty_Hide();
									tty_con = history;
									tty_Show();
								}
								tty_FlushIn();
								return NULL;
								break;
							case 'C': // right
							case 'D': // left
							//case 'H': // home
							//case 'F': // end
								return NULL;
							};
						}
					}
				}

				if ( key == 12 ) { // clear teaminal
					write( STDOUT_FILENO, "\ec]", 3 );
					if ( tty_con.cursor ) {
						write( STDOUT_FILENO, tty_con.buffer, tty_con.cursor );
					}
					tty_FlushIn();
					return NULL;
				}

				Con_DPrintf( "dropping ISCTL sequence: %d, tty_erase: %d\n", key, tty_erase );
				tty_FlushIn();
				return NULL;
			}
			if ( tty_con.cursor >= sizeof( text ) - 1 ) {
				return NULL;
			}

			// push regular character
			tty_con.buffer[ tty_con.cursor ] = key;
			tty_con.cursor++;
			// print the current line (this is differential)
			write( STDOUT_FILENO, &key, 1 );
		}
		return NULL;
	}
	else if ( stdin_active ) {
		int len;
		fd_set fdset;
		struct timeval timeout;

		FD_ZERO( &fdset );
		FD_SET( STDIN_FILENO, &fdset ); // stdin
		timeout.tv_sec = 0;
		timeout.tv_usec = 0;
		if ( select( STDIN_FILENO + 1, &fdset, NULL, NULL, &timeout) == -1 || !FD_ISSET( STDIN_FILENO, &fdset ) ) {
			return NULL;
		}

		len = read( STDIN_FILENO, text, sizeof( text ) );
		if ( len == 0 ) { // eof!
			fcntl( STDIN_FILENO, F_SETFL, stdin_flags );
			stdin_active = qfalse;
			return NULL;
		}

		if ( len < 1 ) {
			return NULL;
		}

		text[len-1] = '\0'; // rip off the /n and terminate
		s = text;

		while ( *s == '\\' || *s == '/' ) { // skip leading slashes
			s++;
		}

		return s;
	}

	return NULL;
}

const char *Sys_GetError( void ) {
	return strerror( errno );
}

tty_err Sys_ConsoleInputInit( void )
{
	struct termios tc;
	const char *term;

	// TTimo
	// https://zerowing.idsoftware.com/bugzilla/show_bug.cgi?id=390
	// ttycon 0 or 1, if the process is backgrounded (running non interactively)
	// then SIGTTIN or SIGTOU is emitted, if not catched, turns into a SIGSTP
	signal( SIGTTIN, SIG_IGN );
	signal( SIGTTOU, SIG_IGN );

	// if SIGCONT is recieved, reinitialize the console
	signal( SIGCONT, CON_SigCont );

	if ( signal( SIGTSTP, SIG_IGN ) == SIG_DFL ) {
		signal( SIGTSTP, CON_SigTStp );
	}

	stdin_flags = fcntl( STDIN_FILENO, F_GETFL, 0 );
	if ( stdin_flags == -1 ) {
		stdin_active = qfalse;
		return TTY_ERROR;
	}

	// set non-blocking mode
	fcntl( STDIN_FILENO, F_SETFL, stdin_flags | O_NONBLOCK );
	stdin_active = qtrue;

	ttycon_color_on = qtrue;

	term = getenv( "TERM" );
	// ncurses doing me favors here
	if ( isatty( STDIN_FILENO ) != 1 || !term || !strcmp( term, "dumb" ) || !strcmp( term, "raw" ) ) {
		ttycon_on = qfalse;
		return TTY_DISABLED;
	}

	Field_Clear( &tty_con );
	tcgetattr( STDIN_FILENO, &tty_tc );
	tty_erase = tty_tc.c_cc[VERASE];
	tty_eof = tty_tc.c_cc[VEOF];
	tc = tty_tc;

	tc.c_lflag &= ~( ECHO | ICANON );
	tc.c_iflag &= ~( ISTRIP | INPCK );
	tc.c_cc[VMIN] = 1;
	tc.c_cc[VTIME] = 0;
	tcsetattr( STDIN_FILENO, TCSADRAIN, &tc );

	if ( ttycon_ansicolor && ttycon_ansicolor->i ) {
		ttycon_color_on = qtrue;
	}

	ttycon_on = qtrue;

	tty_Hide();
	tty_Show();

	return TTY_ENABLED;
}

void Sys_ConfigureFPU( void )  // bk001213 - divide by zero
{
#ifdef __linux__
#ifdef __i386
#ifdef __GLIBC__
#ifndef NDEBUG
	// bk0101022 - enable FPE's in debug mode
	static int fpu_word = _FPU_DEFAULT & ~(_FPU_MASK_ZM | _FPU_MASK_IM);
	int current = 0;
	_FPU_GETCW( current );
	if ( current != fpu_word ) {
#if 0
		Con_Printf("FPU Control 0x%x (was 0x%x)\n", fpu_word, current );
		_FPU_SETCW( fpu_word );
		_FPU_GETCW( current );
		assert(fpu_word==current);
#endif
	}
#else // NDEBUG
	static int fpu_word = _FPU_DEFAULT;
	_FPU_SETCW( fpu_word );
#endif // NDEBUG
#endif // __GLIBC__
#endif // __i386
#endif // __linux
}

#ifdef __APPLE__
static char binaryPath[ MAX_OSPATH ] = { 0 };
static char installPath[ MAX_OSPATH ] = { 0 };


/*
=================
Sys_SetBinaryPath
=================
*/
static void Sys_SetBinaryPath( const char *path )
{
	char *d;
	N_strncpyz( binaryPath, path, sizeof( binaryPath ) );

	d = dirname( binaryPath );
	if ( d != NULL && d != binaryPath ) {
		N_strncpyz( binaryPath, d, sizeof( binaryPath ) );
	}
}


/*
=================
Sys_SetDefaultBasePath
=================
*/
static void Sys_SetDefaultBasePath( const char *path )
{
	N_strncpyz( installPath, path, sizeof( installPath ) );
}


/*
=================
Sys_StripAppBundle
Discovers if passed dir is suffixed with the directory structure of a Mac OS X
.app bundle. If it is, the .app directory structure is stripped off the end and
the result is returned. If not, dir is returned untouched.
=================
*/
// Used to determine where to store user-specific files
static char *Sys_StripAppBundle( char *dir )
{
	static char cwd[MAX_OSPATH];

	N_strncpyz( cwd, dir, sizeof( cwd ) );
	if ( strcmp( basename( cwd ), "MacOS" ) != 0 ) { 
		return dir;
	}

	N_strncpyz( cwd, dirname( cwd ), sizeof( cwd ) );
	if ( strcmp( basename( cwd ), "Contents" ) != 0 ) {
		return dir;
	}

	N_strncpyz( cwd, dirname( cwd ), sizeof( cwd ) ); 
	if ( strstr( basename( cwd ), ".app") == NULL ) {
		return dir;
	}

	N_strncpyz( cwd, dirname( cwd ), sizeof( cwd ) );

	return cwd;
}


/*
=================
Sys_DefaultAppPath
=================
*/
char *Sys_DefaultAppPath( void )
{
	return binaryPath;
}
#endif // __APPLE__

const char *Sys_DefaultBasePath( void )
{
#ifdef __APPLE__
	if ( installPath[0] != '\0' ) {
		return installPath;
	}
#endif
	return Sys_pwd();
}

const char *Sys_DefaultHomePath( void )
{
	// used to determine where to store user-specific files
	static char homePath[MAX_OSPATH];

	const char *p;

	if (*homePath)
		return homePath;

	if ( ( p = getenv( "HOME" ) ) != NULL ) {
		N_strncpyz( homePath, p, sizeof( homePath ) );
#ifdef MACOS_X
		N_strcat( homePath, sizeof( homePath ), "/Library/Application Support/TheNomad" );
#else
		N_strcat( homePath, sizeof( homePath ), "/.thenomad" );
#endif
		if ( mkdir( homePath, 0750 ) ) {
			if ( errno != EEXIST ) {
				N_Error( ERR_DROP, "Unable to create directory \"%s\", error is %s(%d)", homePath, strerror( errno ), errno );
			}
		}
		return homePath;
	}
	return ""; // assume current directory
}

void Sys_PrintBinVersion( const char* name )
{
	const char *date = __DATE__;
	const char *time = __TIME__;
	const char *sep = "==============================================================";

	Con_Printf( "\n\n%s\n", sep );
	Con_Printf( "Linux \"The Nomad\" Full Executable [%s %s]\n", date, time );
	Con_Printf( " local install: %s\n", name );
	Con_Printf( "%s\n\n", sep );
}

/*
=================
Sys_BinName

This resolves any symlinks to the binary. It's disabled for debug
builds because there are situations where you are likely to want
to symlink to binaries and /not/ have the links resolved.
=================
*/
#ifdef __APPLE__
#include <mach-o/dyld.h>
#endif
const char *Sys_BinName( const char *arg0 )
{
	static char dst[ PATH_MAX ];

#ifndef _NOMAD_DEBUG
#if defined (__linux__)
	int n = readlink( "/proc/self/exe", dst, PATH_MAX - 1 );

	if ( n >= 0 && n < PATH_MAX )
		dst[ n ] = '\0';
	else
		N_strncpyz( dst, arg0, PATH_MAX );
#elif defined (__APPLE__)
	uint32_t bufsize = sizeof( dst );

	if ( _NSGetExecutablePath( dst, &bufsize ) == -1 ) {
		N_strncpyz( dst, arg0, PATH_MAX );
	}
#else
	#warning Sys_BinName not implemented
	N_strncpyz( dst, arg0, PATH_MAX );
#endif

#else // DEBUG
	N_strncpyz( dst, arg0, PATH_MAX );
#endif
	return dst;
}

static int Sys_ParseArgs( int argc, const char* argv[] )
{
	if ( argc == 2 ) {
		if ( ( !strcmp( argv[1], "--version" ) ) || ( !strcmp( argv[1], "-v" ) ) ) {
			Sys_PrintBinVersion( Sys_BinName( argv[0] ) );
			return 1;
		}
	}

	return 0;
}

int main( int argc, char **argv )
{
	char con_title[ MAX_CVAR_VALUE ];
	int xpos, ypos;
	char *cmdline;
	int len, i;
	tty_err err;

#ifdef __APPLE__
	// This is passed if we are launched by double-clicking
	if ( argc >= 2 && N_strncmp( argv[1], "-psn", 4 ) == 0 ) {
		argc = 1;
	}
#endif

	if ( Sys_ParseArgs( argc, (const char **)argv ) ) {
		return 0; // print version and exit
	}

#ifdef __APPLE__
	Sys_SetBinaryPath( argv[ 0 ] );
	Sys_SetDefaultBasePath( Sys_StripAppBundle( binaryPath ) );
#endif

	// merge the command line, this is kinda silly
	for ( len = 1, i = 1; i < argc; i++ )
		len += strlen( argv[i] ) + 1;

	Con_Printf( "Working directory: %s\n", Sys_pwd() );

	InitSig();

	cmdline = (char *)malloc( len );
	if ( !cmdline ) { // oh shit
		write( STDERR_FILENO, "malloc() failed, out of memory, FREE UP SOME GODDAMN MEMORY\n",
			sizeof( "malloc() failed, out of memory, FREE UP SOME GODDAMN MEMORY\n" ) );
		_Exit( EXIT_FAILURE );
	}
	*cmdline = '\0';
	for ( i = 1; i < argc; i++ ) {
		if ( i > 1 )
			strcat( cmdline, " " );
		
		strcat( cmdline, argv[i] );
	}
	Com_EarlyParseCmdLine( cmdline, con_title, sizeof( con_title ), &xpos, &ypos );

	// get initial time base
	Sys_Milliseconds();

	err = Sys_ConsoleInputInit();
	if ( err == TTY_ENABLED ) {
		Con_Printf( "Started tty console (use +set ttycon 0 to disable)\n" );
	}
	else {
		if ( err == TTY_ERROR ) {
			Con_Printf( "stdin is not a tty, tty console mode failed\n" );
			Cvar_Set( "ttycon", "0" );
		}
	}
	
	Com_Init( cmdline );

	// Sys_ConsoleInputInit() might be called in signal handler
	// so modify/init any cvars here
	ttycon = Cvar_Get( "ttycon", "1", 0 );
	Cvar_SetDescription( ttycon, "Enable access to input/output console terminal." );
	ttycon_ansicolor = Cvar_Get( "ttycon_ansicolor", "0", CVAR_SAVE );
	Cvar_SetDescription( ttycon_ansicolor, "Convert in-game color codes to ANSI color codes in console terminal." );

	while ( 1 ) {
#ifdef __linux__
		Sys_ConfigureFPU();
#endif
		// check for other input devices
		IN_Frame();
		// run the game
		Com_Frame( qfalse );
	}
	// never gets here
	return 0;
}