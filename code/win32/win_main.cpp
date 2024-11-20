#include "../engine/n_shared.h"
#include "../game/g_game.h"
#include "sys_win32.h"
#include "win_local.h"
#include <signal.h>
#include <excpt.h>

#define MEM_THRESHOLD (96*1024*1024)

WinVars_t g_wv;

extern SDL_Window *SDL_window;

cvar_t *in_forceCharset;
cvar_t *in_mouse;
cvar_t *in_logitechbug;

int			HotKey = 0;
int			hkinstalled = 0;

extern void SetGameDisplaySettings( void );
extern void SetDesktopDisplaySettings( void );
static void VID_AppActivate( qboolean active );

#define TIMER_M 11
#define TIMER_T 12
static UINT uTimerM;
static UINT uTimerT;
static HHOOK WinHook;


static HWINEVENTHOOK hWinEventHook;

const char *Sys_GetError( void ) {
	switch ( ::GetLastError() ) {
	case ERROR_INVALID_FUNCTION: return "ERROR_INVALID_FUNCTION";
	case ERROR_FILE_NOT_FOUND: return "ERROR_FILE_NOT_FOUND";
	case ERROR_PATH_NOT_FOUND: return "ERROR_PATH_NOT_FOUND";
	case ERROR_TOO_MANY_OPEN_FILES: return "ERROR_TOO_MANY_OPEN_FILES";
	case ERROR_ACCESS_DENIED: return "ERROR_ACCESS_DENIED";
	case ERROR_INVALID_HANDLE: return "ERROR_INVALID_HANDLE";
	case ERROR_ARENA_TRASHED: return "ERROR_ARENA_TRASHED";
	case ERROR_NOT_ENOUGH_MEMORY: return "ERROR_NOT_ENOUGH_MEMORY";
	case ERROR_INVALID_BLOCK: return "ERROR_INVALID_BLOCK";
	case ERROR_BAD_ENVIRONMENT: return "ERROR_BAD_ENVIRONMENT";
	case ERROR_BAD_FORMAT: return "ERROR_BAD_FORMAT";
	case ERROR_INVALID_ACCESS: return "ERROR_INVALID_ACCESS";
	case ERROR_INVALID_DATA: return "ERROR_INVALID_DATA";
	case ERROR_OUTOFMEMORY: return "ERROR_OUT_OF_MEMORY";
	case ERROR_INVALID_DRIVE: return "ERROR_INVALID_DRIVE";
	case ERROR_WRITE_FAULT: return "ERROR_WRITE_FAULT";
	case ERROR_READ_FAULT: return "ERROR_READ_FAULT";
	case ERROR_HANDLE_EOF: return "ERROR_HANDLE_EOF";
	case ERROR_NOT_SUPPORTED: return "ERROR_NOT_SUPPORTED";
	case ERROR_HANDLE_DISK_FULL: return "ERROR_HANDLE_DISK_FULL";
	case ERROR_BUFFER_OVERFLOW: return "ERROR_BUFFER_OVERFLOW";
	case ERROR_DRIVE_LOCKED: return "ERROR_DRIVE_LOCKED";
	case ERROR_OPEN_FAILED: return "ERROR_OPEN_FAILED";
	case ERROR_BROKEN_PIPE: return "ERROR_BROKEN_PIPE";
	case ERROR_DISK_FULL: return "ERROR_DISK_FULL";
	case ERROR_INSUFFICIENT_BUFFER: return "ERROR_INSUFFICIENT_BUFFER";
	case ERROR_INVALID_NAME: return "ERROR_INVALID_NAME";
	case ERROR_NEGATIVE_SEEK: return "ERROR_NEGATIVE_SEEK";
	case ERROR_MAX_THRDS_REACHED: return "ERROR_MAX_THRDS_REACHED";
	case ERROR_ALREADY_EXISTS: return "ERROR_ALREADY_EXISTS";
	default:
		break;
	};
	return "Unknown Error";
}

bool Sys_IsInDebugSession( void ) {
	return (::IsDebuggerPresent() != 0);
}

static VOID CALLBACK WinEventProc( HWINEVENTHOOK h_WinEventHook, DWORD dwEvent, HWND hWnd, LONG idObject, LONG idChild, DWORD dwEventThread, DWORD dwmsEventTime )
{
	if ( SDL_window )
	{
		if ( !( SDL_GetWindowFlags( SDL_window ) & SDL_WINDOW_MINIMIZED ) )// disable topmost window style
		{
			SetWindowPos( g_wv.hWnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE );
		}
		SetForegroundWindow( hWnd );
	}
}

/*
==================
WinKeyHook
==================
*/
static LRESULT CALLBACK WinKeyHook( int code, WPARAM wParam, LPARAM lParam )
{
	PKBDLLHOOKSTRUCT key = (PKBDLLHOOKSTRUCT)lParam;
	return CallNextHookEx( NULL, code, wParam, lParam );
}


/*
==================
WIN_DisableHook
==================
*/
void WIN_DisableHook( void  ) 
{
	if ( WinHook ) {
		UnhookWindowsHookEx( WinHook );
		WinHook = NULL;
	}
}


void WIN_Minimize( void ) {
	static int minimize = 0;

	if ( minimize )
		return;

	minimize = 1;

#ifdef FAST_MODE_SWITCH
	// move game window to background
	if ( !( SDL_GetWindowFlags( SDL_window ) & SDL_WINDOW_MINIMIZED ) ) {
		if ( SDL_window )
			SetForegroundWindow( GetDesktopWindow() );
		// and wait some time before minimizing
		if ( !uTimerM )
			uTimerM = SetTimer( g_wv.hWnd, TIMER_M, 50, NULL );
	} else {
		ShowWindow( g_wv.hWnd, SW_MINIMIZE );
	}
#else
	ShowWindow( g_wv.hWnd, SW_MINIMIZE );
#endif

	minimize = 0;
}

/*
==================
WIN_EnableHook

Capture PrintScreen and Win* keys
==================
*/
void WIN_EnableHook( void  ) 
{
	if ( !WinHook )
	{
		WinHook = SetWindowsHookEx( WH_KEYBOARD_LL, WinKeyHook, g_wv.hInstance, 0 );
	}
}


void Win_AddHotkey( void ) 
{
	UINT modifiers, vk;
	ATOM atom;

	if ( !HotKey || !g_wv.hWnd || hkinstalled )
		return;

	modifiers = 0;

	if ( HotKey & HK_MOD_ALT )		modifiers |= MOD_ALT;
	if ( HotKey & HK_MOD_CONTROL )	modifiers |= MOD_CONTROL;
	if ( HotKey & HK_MOD_SHIFT )	modifiers |= MOD_SHIFT;
	if ( HotKey & HK_MOD_WIN )		modifiers |= MOD_WIN;

	vk = HotKey & 0xFF;

	atom = GlobalAddAtom( TEXT( "Q3MinimizeHotkey" ) );
	if ( !RegisterHotKey( g_wv.hWnd, atom, modifiers, vk ) ) {
		GlobalDeleteAtom( atom );
		return;
	}
	hkinstalled = 1;
}


void Win_RemoveHotkey( void ) 
{
	ATOM atom;

	if ( !g_wv.hWnd || !hkinstalled )
		return;

	atom = GlobalFindAtom( TEXT( "Q3MinimizeHotkey" ) );
	if ( atom ) {
		UnregisterHotKey( g_wv.hWnd, atom );
 		GlobalDeleteAtom( atom );
		hkinstalled = 0;
	}
}


BOOL Win_CheckHotkeyMod( void ) {

	if ( !(HotKey & HK_MOD_XMASK) )
 		return TRUE;

 	if ((HotKey&HK_MOD_LALT) && !GetAsyncKeyState(VK_LMENU)) return FALSE;
 	if ((HotKey&HK_MOD_RALT) && !GetAsyncKeyState(VK_RMENU)) return FALSE;
 	if ((HotKey&HK_MOD_LSHIFT) && !GetAsyncKeyState(VK_LSHIFT)) return FALSE;
 	if ((HotKey&HK_MOD_RSHIFT) && !GetAsyncKeyState(VK_RSHIFT)) return FALSE;
 	if ((HotKey&HK_MOD_LCONTROL) && !GetAsyncKeyState(VK_LCONTROL)) return FALSE;
 	if ((HotKey&HK_MOD_RCONTROL) && !GetAsyncKeyState(VK_RCONTROL)) return FALSE;
 	if ((HotKey&HK_MOD_LWIN) && !GetAsyncKeyState(VK_LWIN)) return FALSE;
 	if ((HotKey&HK_MOD_RWIN) && !GetAsyncKeyState(VK_RWIN)) return FALSE;

 	return TRUE;
}


//
// Sys_MessageBox: adapted slightly from the source engine
//
int Sys_MessageBox(const char *title, const char *text, bool ShowOkAndCancelButton)
{
#if 0
    if (MessageBox( NULL, title, text, MB_ICONEXCLAMATION | ( ShowOkAndCancelButton ? MB_OKCANCEL : MB_OK )) == IDOK) {
        return true;
    }
    return false;
#else
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
#endif
}

qboolean Sys_LowPhysicalMemory( void )
{
    MEMORYSTATUSEX stat;
    GlobalMemoryStatusEx( &stat );
    return (stat.ullTotalPhys <= MEM_THRESHOLD) ? qtrue : qfalse;
}

void GDR_NORETURN GDR_ATTRIBUTE((format(printf, 1, 2))) GDR_DECL Sys_Error( const char *err, ... )
{
    va_list argptr;
    char text[MAXPRINTMSG];
    MSG msg;
	const char *str;

    va_start( argptr, err );
    N_vsnprintf( text, sizeof(text), err, argptr );
    va_end( argptr );

	str = va("Sys_Error: %s\n", text);
	Sys_Print( str );

	Sys_DebugStacktrace( MAX_STACKTRACE_FRAMES );
	Sys_MessageBox( "Engine Error", text, false );

	Com_Shutdown();

    // wait for the user to quit
	#if 0
    while (1) {
        if (!GetMessage( &msg, NULL, 0, 0 )) {
            Cmd_Clear();
        }
        TranslateMessage( &msg );
        DispatchMessage( &msg );
    }
	#endif

    Sys_Exit( -1 );
}

void GDR_NORETURN Sys_Exit( int code )
{
    if (code == -1) {
        exit( EXIT_FAILURE );
    }

    exit( EXIT_SUCCESS );
}

static const struct Q3ToAnsiColorTable_s
{
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

static qboolean printableChar( char c ) {
	if ( ( c >= ' ' && c <= '~' ) || c == '\n' || c == '\r' || c == '\t' )
		return qtrue;
	else
		return qfalse;
}

void Sys_ANSIColorMsg(const char *msg, char *buffer, uint64_t bufSize)
{
    uint64_t msgLength;
    uint64_t i;
    char tmpbuf[8];
    const char *ANSIcolor;

    if (!msg || !buffer)
        return;

    msgLength = strlen(msg);
    i = 0;
    buffer[0] = '\0';

    while (i < msgLength) {
        if (msg[i] == '\n') {
            snprintf(tmpbuf, sizeof(tmpbuf), "%c[0m\n", 0x1B);
            strncat(buffer, tmpbuf, bufSize - 1);
            i += 1;
        }
        else if (msg[i] == Q_COLOR_ESCAPE && (ANSIcolor = getANSIcolor(msg[i+1])) != NULL) {
            snprintf(tmpbuf, sizeof(tmpbuf), "%c[%sm", 0x1B, ANSIcolor);
            strncat(buffer, tmpbuf, bufSize - 1);
            i += 2;
        }
        else {
            if (printableChar(msg[i])) {
                snprintf(tmpbuf, sizeof(tmpbuf), "%c", msg[i]);
                strncat(buffer, tmpbuf, bufSize - 1);
            }
            i += 1;
        }
    }
}

static qboolean isInConsole;
static HANDLE hStdOutConsole;
static HANDLE hConsole;
void Sys_InitConsole( void )
{
//	HWND consoleWnd = GetConsoleWindow();
//	DWORD dwProcessId;
//	GetWindowProcessId( consoleWnd, &dwProcessId );
//	if (GetCurrentProcessId() == dwProcessId) {
//		isInConsole = qtrue;
//		Con_Printf( "using system console.\n" );
//	}
//	else {
//		Con_Printf( "using engine console only\n" );
//	}

	DWORD dwMode;

	hStdOutConsole = GetStdHandle( STD_OUTPUT_HANDLE );
	hConsole = CreateConsoleScreenBuffer( GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL );
	SetConsoleActiveScreenBuffer( hConsole );
	dwMode = 0;
	GetConsoleMode( hConsole, &dwMode );
	dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
	SetConsoleMode( hConsole, dwMode );
}

void Sys_Print(const char *msg)
{
    char printmsg[MAXPRINTMSG];
    size_t len;
	DWORD dwBytesWritten;

    memset(printmsg, 0, sizeof(printmsg));

    if (hConsole) {
        Sys_ANSIColorMsg(msg, printmsg, sizeof(printmsg));
        len = strlen(printmsg);
		::WriteConsole( hConsole, printmsg, len, &dwBytesWritten, NULL );
    }

/*    else {
        char *out = printmsg;
        while (*msg != '\0' && out < printmsg + sizeof(printmsg)) {
            if (printableChar(*msg))
                *out++ = *msg;
            msg++;
        }
        len = out - printmsg;
    } */

//    _write(STDERR_FILENO, printmsg, len);
}

/*
==============================================================

DIRECTORY SCANNING

==============================================================
*/

void Sys_ListFilteredFiles( const char *basedir, const char *subdirs, const char *filter, char **list, uint64_t *numfiles ) {
	char		search[MAX_OSPATH*2+1];
	char		newsubdirs[MAX_OSPATH*2];
	char		filename[MAX_OSPATH*2];
	intptr_t	findhandle;
	struct _finddata_t findinfo;

	if ( *numfiles >= MAX_FOUND_FILES - 1 ) {
		return;
	}

	if ( *subdirs ) {
		Com_snprintf( search, sizeof(search), "%s\\%s\\*", basedir, subdirs );
	}
	else {
		Com_snprintf( search, sizeof(search), "%s\\*", basedir );
	}

	findhandle = _findfirst (search, &findinfo);
	if (findhandle == -1) {
		return;
	}

	do {
		if (findinfo.attrib & _A_SUBDIR) {
			if ( !N_streq( findinfo.name, "." ) && !N_streq( findinfo.name, ".." ) ) {
				if ( *subdirs ) {
					Com_snprintf( newsubdirs, sizeof(newsubdirs), "%s\\%s", subdirs, findinfo.name );
				} else {
					Com_snprintf( newsubdirs, sizeof(newsubdirs), "%s", findinfo.name );
				}
				Sys_ListFilteredFiles( basedir, newsubdirs, filter, list, numfiles );
			}
		}
		if ( *numfiles >= MAX_FOUND_FILES - 1 ) {
			break;
		}
		Com_snprintf( filename, sizeof(filename), "%s\\%s", subdirs, findinfo.name );
		if ( !Com_FilterPath( filter, filename ) )
			continue;
		list[ *numfiles ] = FS_CopyString( filename );
		(*numfiles)++;
	} while ( _findnext (findhandle, &findinfo) != -1 );

	_findclose (findhandle);
}


/*
=============
Sys_Sleep
=============
*/
void Sys_Sleep( uint64_t msec )
{
	if ( msec < 0 ) {
		// special case: wait for event or network packet
		DWORD dwResult;
		msec = 300;
		do {
			dwResult = MsgWaitForMultipleObjects( 0, NULL, FALSE, msec, QS_ALLEVENTS );
		} while ( dwResult == WAIT_TIMEOUT );
		//WaitMessage();
		return;
	}

	// busy wait there because Sleep(0) will relinquish CPU - which is not what we want
	//if ( msec == 0 )
	//	return;

	Sleep ( msec );
}


/*
=============
Sys_ListFiles
=============
*/
char **Sys_ListFiles( const char *directory, const char *extension, const char *filter, uint64_t *numfiles, qboolean wantsubs ) {
	char		search[MAX_OSPATH*2+MAX_GDR_PATH+1];
	uint64_t	nfiles;
	char		**listCopy;
	char		*list[MAX_FOUND_FILES];
	struct _finddata_t findinfo;
	intptr_t	findhandle;
	int			flag;
	uint64_t	extLen;
	uint64_t	length;
	uint32_t	i;
	const char	*x;
	qboolean	hasPatterns;

	if ( filter ) {

		nfiles = 0;
		Sys_ListFilteredFiles( directory, "", filter, list, &nfiles );

		list[ nfiles ] = NULL;
		*numfiles = nfiles;

		if (!nfiles)
			return NULL;

		listCopy = (char **)Z_Malloc( ( nfiles + 1 ) * sizeof( listCopy[0] ), TAG_STATIC );
		for ( i = 0 ; i < nfiles ; i++ ) {
			listCopy[i] = list[i];
		}
		listCopy[i] = NULL;

		return listCopy;
	}

	if ( !extension ) {
		extension = "";
	}

	// passing a slash as extension will find directories
	if ( extension[0] == '/' && extension[1] == 0 ) {
		extension = "";
		flag = 0;
	} else {
		flag = _A_SUBDIR;
	}

	Com_snprintf( search, sizeof(search), "%s\\*%s", directory, extension );

	findhandle = _findfirst( search, &findinfo );
	if ( findhandle == -1 ) {
		*numfiles = 0;
		return NULL;
	}

	extLen = (int)strlen( extension );
	hasPatterns = Com_HasPatterns( extension );
	if ( hasPatterns && extension[0] == '.' && extension[1] != '\0' ) {
		extension++;
	}

	// search
	nfiles = 0;

	do {
		if ( (!wantsubs && flag ^ ( findinfo.attrib & _A_SUBDIR )) || (wantsubs && findinfo.attrib & _A_SUBDIR) ) {
			if ( nfiles == MAX_FOUND_FILES - 1 ) {
				break;
			}
			if ( *extension ) {
				if ( hasPatterns ) {
					x = strrchr( findinfo.name, '.' );
					if ( !x || !Com_FilterExt( extension, x+1 ) ) {
						continue;
					}
				} else {
					length = strlen( findinfo.name );
					if ( length < extLen || N_stricmp( findinfo.name + length - extLen, extension ) ) {
						continue;
					}
				}
			}
			list[ nfiles ] = FS_CopyString( findinfo.name );
			nfiles++;
		}
	} while ( _findnext (findhandle, &findinfo) != -1 );

	list[ nfiles ] = NULL;

	_findclose (findhandle);

	// return a copy of the list
	*numfiles = nfiles;

	if ( !nfiles ) {
		return NULL;
	}

	listCopy = (char **)Z_Malloc( ( nfiles + 1 ) * sizeof( listCopy[0] ), TAG_STATIC );
	for ( i = 0 ; i < nfiles ; i++ ) {
		listCopy[i] = list[i];
	}
	listCopy[i] = NULL;

	Com_SortFileList( listCopy, nfiles, extension[0] != '\0' );

	return listCopy;
}


/*
=============
Sys_FreeFileList
=============
*/
void Sys_FreeFileList( char **list ) {
	int		i;

	if ( !list ) {
		return;
	}

	for ( i = 0 ; list[i] ; i++ ) {
		Z_Free( list[i] );
	}

	Z_Free( list );
}


/*
=============
Sys_GetFileStats
=============
*/
qboolean Sys_GetFileStats( const char *filename, fileOffset_t *size, fileTime_t *mtime, fileTime_t *ctime ) {
	struct _stat s;

	if ( _stat( filename, &s ) == 0 ) {
		*size = (fileOffset_t)s.st_size;
		*mtime = (fileTime_t)s.st_mtime;
		*ctime = (fileTime_t)s.st_ctime;
		return qtrue;
	} else {
		*size = 0;
		*mtime = *ctime = 0;
		return qfalse;
	}
}

static const char *GetExceptionName( DWORD code )
{
	static char buf[ 32 ];

	switch ( code )
	{
		case EXCEPTION_ACCESS_VIOLATION: return "ACCESS_VIOLATION";
		case EXCEPTION_DATATYPE_MISALIGNMENT: return "DATATYPE_MISALIGNMENT";
		case EXCEPTION_ARRAY_BOUNDS_EXCEEDED: return "ARRAY_BOUNDS_EXCEEDED";
		case EXCEPTION_PRIV_INSTRUCTION: return "PRIV_INSTRUCTION";
		case EXCEPTION_IN_PAGE_ERROR: return "IN_PAGE_ERROR";
		case EXCEPTION_ILLEGAL_INSTRUCTION: return "ILLEGAL_INSTRUCTION";
		case EXCEPTION_NONCONTINUABLE_EXCEPTION: return "NONCONTINUABLE_EXCEPTION";
		case EXCEPTION_STACK_OVERFLOW: return "STACK_OVERFLOW";
		case EXCEPTION_INVALID_DISPOSITION: return "INVALID_DISPOSITION";
		case EXCEPTION_GUARD_PAGE: return "GUARD_PAGE";
		case EXCEPTION_INVALID_HANDLE: return "INVALID_HANDLE";
		default: break;
	}

	sprintf( buf, "0x%08X", (unsigned int)code );
	return buf;
}


/*
==================
ExceptionFilter

Restore gamma and hide fullscreen window in case of crash
==================
*/
static LONG WINAPI ExceptionFilter( struct _EXCEPTION_POINTERS *ExceptionInfo )
{
	GLimp_Minimize();

	if ( ExceptionInfo->ExceptionRecord->ExceptionCode != EXCEPTION_BREAKPOINT ) {
		char msg[1024], name[MAX_OSPATH];
		const char *basename;
		HMODULE hModule, hKernel32;
		byte *addr;

		memset( msg, 0, sizeof( msg ) );

		hModule = NULL;
		name[0] = '\0';
		basename = name;
		addr = (byte*)ExceptionInfo->ExceptionRecord->ExceptionAddress;

		hKernel32 = GetModuleHandleA( "kernel32" );
		if ( hKernel32 != NULL ) {
			typedef BOOL (WINAPI *PFN_GetModuleHandleExA)( DWORD dwFlags, LPCSTR lpModuleName, HMODULE *phModule );
			PFN_GetModuleHandleExA pGetModuleHandleExA;

			pGetModuleHandleExA = (PFN_GetModuleHandleExA) GetProcAddress( hKernel32, "GetModuleHandleExA" );
			if ( pGetModuleHandleExA != NULL ) {
				if ( pGetModuleHandleExA( GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, (LPCTSTR)addr, &hModule ) ) {
					if (GetModuleFileNameA( hModule, name, arraylen(name) - 1) != 0 ) {
						name[arraylen(name) - 1] = '\0';
						basename = strrchr( name, '\\' );
						if ( basename ) {
							basename = basename + 1;
						}
						else {
							basename = strrchr( name, '/' );
							if ( basename ) {
								basename = basename + 1;
							}
						}
					}
				}
			}
		}

		if ( basename && *basename ) {
			Com_snprintf( msg, sizeof( msg ), "Exception Code: %s\nException Address: %s@%x\n",
				GetExceptionName( ExceptionInfo->ExceptionRecord->ExceptionCode ),
				basename, (uint32_t)(addr - (byte*)hModule) );
		} else {
			Com_snprintf( msg, sizeof( msg ), "Exception Code: %s\nException Address: %p\n",
				GetExceptionName( ExceptionInfo->ExceptionRecord->ExceptionCode ),
				addr );
		}

		N_Error( ERR_FATAL, "Unhandled exception caught\n%s", msg );
	}

	return EXCEPTION_EXECUTE_HANDLER;
}

LRESULT WINAPI MainWndProc( HWND hWnd, UINT uMsg, WPARAM  wParam, LPARAM lParam )
{
	#define TIMER_ID 10
	//static UINT uTimerID;
	static qboolean flip = qtrue;
	static qboolean focused = qfalse;
	qboolean active;
	qboolean minimized;
	int zDelta, i;

	// http://msdn.microsoft.com/library/default.asp?url=/library/en-us/winui/winui/windowsuserinterface/userinput/mouseinput/aboutmouseinput.asp
	// Windows 95, Windows NT 3.51 - uses MSH_MOUSEWHEEL
	// only relevant for non-DI input
	//
	// NOTE: not sure how reliable this is anymore, might trigger double wheel events
	/* if (in_mouse->i == -1)
	{
		if ( uMsg == MSH_MOUSEWHEEL )
		{
			if ( ( ( int ) wParam ) > 0 )
			{
				Sys_QueEvent( g_wv.sysMsgTime, SE_KEY, K_MWHEELUP, qtrue, 0, NULL );
				Sys_QueEvent( g_wv.sysMsgTime, SE_KEY, K_MWHEELUP, qfalse, 0, NULL );
			}
			else
			{
				Sys_QueEvent( g_wv.sysMsgTime, SE_KEY, K_MWHEELDOWN, qtrue, 0, NULL );
				Sys_QueEvent( g_wv.sysMsgTime, SE_KEY, K_MWHEELDOWN, qfalse, 0, NULL );
			}
			return DefWindowProc( hWnd, uMsg, wParam, lParam );
		}
	} */

	switch (uMsg) {
	case WM_CREATE:

		//MSH_MOUSEWHEEL = RegisterWindowMessage( TEXT( "MSWHEEL_ROLLMSG" ) ); 

		WIN_EnableHook(); // for PrintScreen and Win* keys

		hWinEventHook = SetWinEventHook( EVENT_SYSTEM_SWITCHSTART, EVENT_SYSTEM_SWITCHSTART, NULL, WinEventProc, 
			0, 0, WINEVENT_OUTOFCONTEXT | WINEVENT_SKIPOWNPROCESS );
		g_wv.hWnd = hWnd;
		GetWindowRect( hWnd, &g_wv.winRect );
		g_wv.winRectValid = qtrue;
//		gw_minimized = qfalse;
		uTimerM = 0;
		uTimerT = 0;

		in_forceCharset = Cvar_Get( "in_forceCharset", "1", CVAR_ARCHIVE_ND );
		Cvar_SetDescription( in_forceCharset, "Try to translate non-ASCII chars in keyboard input or force EN/US keyboard layout." );

		break;
	case WM_DESTROY:
		Win_RemoveHotkey();
		if ( hWinEventHook ) {
			UnhookWinEvent( hWinEventHook );
		}
		if ( uTimerM ) {
			KillTimer( g_wv.hWnd, uTimerM ); uTimerM = 0;
		}
		if ( uTimerT ) {
			KillTimer( g_wv.hWnd, uTimerT ); uTimerT = 0;
		}
		hWinEventHook = NULL;
		g_wv.hWnd = NULL;
		g_wv.winRectValid = qfalse;
		//gw_minimized = qfalse;
//		gw_active = qfalse;
		//WIN_EnableAltTab();
		return 0;

	case WM_CLOSE:
		Cbuf_ExecuteText( EXEC_APPEND, "quit\n" );
		// filter this message or we may lose window before renderer shutdown ?
		return 0;

	/*
		on minimize:
			WM_WINDOWPOSCHANGING WindowPlacement:ShowCmd = SW_SHOWMINIMIZED
			WM_KILLFOCUS
			WM_MOVE (x:garbage y:garbage)
			WM_SIZE (SIZE_MINIMIZED w=0 h=0)
			WM_ACTIVATE (active=0 minimized=1)

		on restore:
			WM_WINDOWPOSCHANGING WindowPlacement:ShowCmd = SW_SHOWNORMAL
			WM_ACTIVATE (active=1 minimized=1)
			WM_MOVE (x, y)
			WM_SIZE (SIZE_RESTORED width height)
			WM_SETFOCUS
			WM_ACTIVATE (active=1 minimized=0)
			WM_WINDOWPOSCHANGING WindowPlacement:ShowCmd = SW_SHOWNORMAL

		on click in:
			WM_WINDOWPOSCHANGING WindowPlacement:ShowCmd = SW_SHOWNORMAL
			WM_ACTIVATE (active=1 minimized=0)
			WM_WINDOWPOSCHANGING WindowPlacement:ShowCmd = SW_SHOWNORMAL
			WM_SETFOCUS

		on click out, destroy:
			WM_ACTIVATE (active=0 minimized=0)
			WM_WINDOWPOSCHANGING WindowPlacement:ShowCmd = SW_SHOWNORMAL
			WM_KILLFOCUS

		on create:
			WM_WINDOWPOSCHANGING WindowPlacement:ShowCmd = SW_SHOWNORMAL
			WM_ACTIVATE (active=1 minimized=0)
			WM_WINDOWPOSCHANGING WindowPlacement:ShowCmd = SW_SHOWNORMAL
			WM_SETFOCUS
			WM_SIZE (SIZE_RESTORED width height)
			WM_MOVE (x, y)

		on win+d:
			WM_WINDOWPOSCHANGING WindowPlacement:ShowCmd = SW_SHOWMINIMIZED
			WM_MOVE (x:garbage, y:garbage)
			WM_SIZE (SIZE_MINIMIZED)
			WM_ACTIVATE (active=0 minimized=1)
			WM_WINDOWPOSCHANGING WindowPlacement:ShowCmd = SW_SHOWMINIMIZED
			WM_KILLFOCUS
			
	*/

	case WM_ACTIVATE:
		active = (LOWORD( wParam ) != WA_INACTIVE) ? qtrue : qfalse;
		minimized = (BOOL)HIWORD( wParam ) ? qtrue : qfalse;

		// We can receive Active & Minimized when restoring from minimized state
		if ( active && minimized ) {
			GLimp_Minimize();
			break;
		}

//		gw_active = active;
		if (minimized) {
			GLimp_Minimize();
		}

		VID_AppActivate( active );
		Win_AddHotkey();

//		if ( !( SDL_GetWindowFlags( SDL_window ) & SDL_WINDOW_MINIMIZED ) ) {
//			if ( gw_active ) {
//				SetGameDisplaySettings();
//				if ( re.SetColorMappings )
//					re.SetColorMappings();
//			} else {
//				// don't restore gamma if we have multiple monitors
//				if ( glw_state.monitorCount <= 1 || gw_minimized )
//					GLW_RestoreGamma();
//				// minimize if there is only one monitor
//				if ( glw_state.monitorCount <= 1 ) {
//					if ( ( re.CanMinimize && re.CanMinimize() ) ) {
//						if ( !gw_minimized ) {
//							WIN_Minimize();
//						}
//						SetDesktopDisplaySettings();
//					}
//				}
//			}
//		} else {
//			if ( gw_active ) {
//				if ( re.SetColorMappings )
//					re.SetColorMappings();
//			} else {
//				GLW_RestoreGamma();
//			}
//		}

		// after ALT+TAB, even if we selected other window we may receive WM_ACTIVATE 1 and then WM_ACTIVATE 0
		// if we set HWND_TOPMOST in VID_AppActivate() other window will be not visible despite obtained input focus
		// so delay HWND_TOPMOST setup to make sure we have no such bogus activation
		if ( !( SDL_GetWindowFlags( SDL_window ) & SDL_WINDOW_MINIMIZED ) ) {
			if ( uTimerT ) {
				KillTimer( g_wv.hWnd, uTimerT );
			}
			uTimerT = SetTimer( g_wv.hWnd, TIMER_T, 20, NULL );
		}

		break;

	case WM_SETFOCUS:
		focused = qtrue;
		break;

	case WM_KILLFOCUS:
		//gw_active = qfalse;
		focused = qfalse;
		break;

	case WM_MOVE:
		if ( !SDL_window || SDL_GetWindowFlags( SDL_window ) & SDL_WINDOW_MINIMIZED || !focused )
			break;

		GetWindowRect( hWnd, &g_wv.winRect );
		g_wv.winRectValid = qtrue;
		UpdateMonitorInfo( &g_wv.winRect );

		if ( !( SDL_GetWindowFlags( SDL_window ) & SDL_WINDOW_MINIMIZED ) )	{
			Cvar_SetIntegerValue( "vid_xpos", g_wv.winRect.left );
			Cvar_SetIntegerValue( "vid_ypos", g_wv.winRect.top );
			vid_xpos->modified = qfalse;
			vid_ypos->modified = qfalse;
		}
		break;

	case WM_SIZE:
		if ( SDL_window && focused && !( SDL_GetWindowFlags( SDL_window ) & SDL_WINDOW_MINIMIZED ) ) {
			GetWindowRect( hWnd, &g_wv.winRect );
			g_wv.winRectValid = qtrue;
			UpdateMonitorInfo( &g_wv.winRect );
//			IN_UpdateWindow( NULL, qtrue );
		}
		break;
	case WM_WINDOWPOSCHANGING:
		{
			WINDOWPLACEMENT wp;

			// set minimized flag as early as possible
			memset( &wp, 0, sizeof( wp ) );
			wp.length = sizeof( WINDOWPLACEMENT );
			if ( GetWindowPlacement( hWnd, &wp ) && wp.showCmd == SW_SHOWMINIMIZED )
				GLimp_Minimize();

			if ( g_wv.borderless )
			{
				WINDOWPOS *pos = (LPWINDOWPOS) lParam;
				const int threshold = 10;
				HMONITOR hMonitor;
				MONITORINFO mi;
				const RECT *r;
				RECT rr;

				rr.left = pos->x;
				rr.right = pos->x + pos->cx;
				rr.top = pos->y;
				rr.bottom = pos->y + pos->cy;
				hMonitor = MonitorFromRect( &rr, MONITOR_DEFAULTTONEAREST );

				if ( hMonitor )
				{
					mi.cbSize = sizeof( mi );
					GetMonitorInfo( hMonitor, &mi );
					r = &mi.rcWork;

					// snap window to current monitor borders
					if ( pos->x >= ( r->left - threshold ) && pos->x <= ( r->left + threshold ) )
						pos->x = r->left;
					else if ( ( pos->x + pos->cx ) >= ( r->right - threshold ) && ( pos->x + pos->cx ) <= ( r->right + threshold ) )
						pos->x = ( r->right - pos->cx );

					if ( pos->y >= ( r->top - threshold ) && pos->y <= ( r->top + threshold ) )
						pos->y = r->top;
					else if ( ( pos->y + pos->cy ) >= ( r->bottom - threshold ) && ( pos->y + pos->cy ) <= ( r->bottom + threshold ) )
						pos->y = ( r->bottom - pos->cy );

					return 0;
				}
			}
		}
		break;
	case WM_SYSCOMMAND:
		// Prevent Alt+Letter commands from hanging the application temporarily
		if ( wParam == SC_KEYMENU || wParam == SC_MOUSEMENU + HTSYSMENU || wParam == SC_CLOSE + HTSYSMENU )
			return 0;

		if ( wParam == SC_SCREENSAVE || wParam == SC_MONITORPOWER )
			return 0;

//		if ( wParam == SC_MINIMIZE && !( re.CanMinimize && re.CanMinimize() ) ) // CL_VideoRecording
//			return 0;

		// simulate drag move to avoid ~500ms delay between DefWindowProc() and further WM_ENTERSIZEMOVE
		if ( wParam == SC_MOVE + HTCAPTION )
		{
			mouse_event( MOUSEEVENTF_MOVE | MOUSEEVENTF_LEFTDOWN, 7, 0, 0, 0 );
			mouse_event( MOUSEEVENTF_MOVE | MOUSEEVENTF_LEFTDOWN, (DWORD)-7, 0, 0, 0 );
		}
		break;

	case WM_CONTEXTMENU:
		// disable context menus to avoid blocking message loop
		return 0;

	case WM_HOTKEY:
		// check for left/right modifiers
		if ( Win_CheckHotkeyMod() )
		{
			if ( SDL_window )
			{
				if ( ( re.CanMinimize && re.CanMinimize() ) )
					WIN_Minimize();
			}
			else
			{
				SetForegroundWindow( hWnd );
				SetFocus( hWnd );
				ShowWindow( hWnd, SW_RESTORE );
			}
			return 0;
		}
		break;

	case WM_NCHITTEST:
		// in borderless mode - drag using client area when holding ALT
		if ( g_wv.borderless && GetKeyState( VK_MENU ) & (1<<15) )
			return HTCAPTION;
		break;

	case WM_ERASEBKGND: 
		// avoid GDI clearing the OpenGL window background in Vista/7
		return 1;
	};

	return DefWindowProc( hWnd, uMsg, wParam, lParam );
}

/*
==================
VID_AppActivate
==================
*/
static void VID_AppActivate( qboolean active )
{
	Key_ClearStates();

//	IN_Activate( active );

	if ( active ) {
		WIN_EnableHook();
		SetWindowPos( g_wv.hWnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE );
	} else {
		WIN_DisableHook();
		SetWindowPos( g_wv.hWnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE );
	}
}

//==========================================================================

static const int s_scantokey[ 128 ] = 
{ 
//	0        1       2       3       4       5       6       7 
//	8        9       A       B       C       D       E       F 
	0  , KEY_ESCAPE,  '1',    '2',    '3',    '4',    '5',    '6', 
	'7',    '8',    '9',    '0',    '-',    '=',KEY_BACKSPACE,KEY_TAB,  // 0 
	'q',    'w',    'e',    'r',    't',    'y',    'u',    'i', 
	'o',    'p',    '[',    ']',  KEY_ENTER, KEY_CTRL,	'a',	's',	// 1 
	'd',    'f',    'g',    'h',    'j',    'k',    'l',    ';', 
	'\'',KEY_CONSOLE,KEY_SHIFT, '\\',   'z',    'x',    'c',    'v',	// 2 
	'b',    'n',    'm',    ',',    '.',    '/',  KEY_SHIFT,  '*', 
	KEY_ALT,  ' ',KEY_CAPSLOCK, KEY_F1,   KEY_F2,   KEY_F3,   KEY_F4,  KEY_F5,    // 3 
	KEY_F6, KEY_F7,  KEY_F8,   KEY_F9,  KEY_F10, KEY_PAUSE, KEY_SCROLLOCK, KEY_HOME, 
	KEY_UPARROW,KEY_PAGEUP,KEY_KP_MINUS,KEY_LEFTARROW,KEY_KP_5,KEY_RIGHTARROW,KEY_KP_PLUS,KEY_END, //4 
	KEY_DOWNARROW,KEY_PAGEDOWN,KEY_INSERT,KEY_DELETE, 0,      0,      0,    KEY_F11, 
	KEY_F12,  0  ,    0  ,    0  ,    0  ,  KEY_MENU,   0  ,    0,     // 5
	0  ,    0  ,    0  ,    0  ,    0  ,    0  ,    0  ,    0, 
	0  ,    0  ,    0  ,    0  ,    0  ,    0  ,    0  ,    0,     // 6 
	0  ,    0  ,    0  ,    0  ,    0  ,    0  ,    0  ,    0, 
	0  ,    0  ,    0  ,    0  ,    0  ,    0  ,    0  ,    0      // 7 
};

static qboolean directMap( const WPARAM chr ) {

	if ( !in_forceCharset->i )
		return qtrue;

	switch ( chr ) // edit control sequences
	{
		case 'c'-'a'+1:
		case 'v'-'a'+1:
		case 'h'-'a'+1:
		case 'a'-'a'+1:
		case 'e'-'a'+1:
		case 'n'-'a'+1:
		case 'p'-'a'+1:
		case 'l'-'a'+1: // CTRL+L
			return qtrue;
	}

	if ( chr < ' ' || chr > 127 || in_forceCharset->i > 1 )
		return qfalse;
	else
		return qtrue;
}


/*
==================
MapChar

Map input to ASCII charset
==================
*/
static int MapChar( WPARAM wParam, byte scancode ) 
{
	static const int s_scantochar[ 128 ] = 
	{ 
//	0        1       2       3       4       5       6       7 
//	8        9       A       B       C       D       E       F 
 	 0,      0,     '1',    '2',    '3',    '4',    '5',    '6', 
	'7',    '8',    '9',    '0',    '-',    '=',    0x8,    0x9,	// 0
	'q',    'w',    'e',    'r',    't',    'y',    'u',    'i', 
	'o',    'p',    '[',    ']',    0xD,     0,     'a',    's',	// 1
	'd',    'f',    'g',    'h',    'j',    'k',    'l',    ';', 
	'\'',    0,      0,     '\\',   'z',    'x',    'c',    'v',	// 2
	'b',    'n',    'm',    ',',    '.',    '/',     0,     '*', 
	 0,     ' ',     0,      0,      0,      0,      0,      0,     // 3

	 0,      0,     '!',    '@',    '#',    '$',    '%',    '^', 
	'&',    '*',    '(',    ')',    '_',    '+',    0x8,    0x9,	// 4
	'Q',    'W',    'E',    'R',    'T',    'Y',    'U',    'I', 
	'O',    'P',    '{',    '}',    0xD,     0,     'A',    'S',	// 5
	'D',    'F',    'G',    'H',    'J',    'K',    'L',    ':',
	'"',     0,      0,     '|',    'Z',    'X',    'C',    'V',	// 6
	'B',    'N',    'M',    '<',    '>',    '?',     0,     '*', 
 	 0,     ' ',     0,      0,      0,      0,      0,      0,     // 7
	}; 

	if ( scancode == 0x53 )
		return '.';

	if ( directMap( wParam ) || scancode > 0x39 )
	{
		return wParam;
	}
	else 
	{
		char ch = s_scantochar[ scancode ];
		int shift = (GetKeyState( VK_SHIFT ) >> 15) & 1;
		if ( ch >= 'a' && ch <= 'z' ) 
		{
			int  capital = GetKeyState( VK_CAPITAL ) & 1;
			if ( capital ^ shift ) 
			{
				ch = ch - 'a' + 'A';
			}
		} 
		else 
		{
			ch = s_scantochar[ scancode | (shift<<6) ];
		}

		return ch;
	}
}

void Sys_GetRAMUsage( uint64_t *curVirt, uint64_t *curPhys, uint64_t *peakVirt, uint64_t *peakPhys )
{
}


#if 0
// Measure the processor clock speed by sampling the cycle count, waiting
// for some fraction of a second, then measuring the elapsed number of cycles.
static int64_T CalculateClockSpeed( void )
{
#if defined( _X360 ) || defined(_PS3)
	// Xbox360 and PS3 have the same clock speed and share a lot of characteristics on PPU
	return 3200000000LL;
#else	
#if defined( _WIN32 )
	LARGE_INTEGER waitTime, startCount, curCount;
	CCycleCount start, end;

	// Take 1/32 of a second for the measurement.
	QueryPerformanceFrequency( &waitTime );
	int scale = 5;
	waitTime.QuadPart >>= scale;

	QueryPerformanceCounter( &startCount );
	start.Sample();
	do
	{
		QueryPerformanceCounter( &curCount );
	}
	while ( curCount.QuadPart - startCount.QuadPart < waitTime.QuadPart );
	end.Sample();

	return (end.m_Int64 - start.m_Int64) << scale;
#elif defined(POSIX)
	uint64 CalculateCPUFreq(); // from cpu_linux.cpp
	int64 freq =(int64)CalculateCPUFreq();
	if ( freq == 0 ) // couldn't calculate clock speed
	{
		Error( "Unable to determine CPU Frequency\n" );
	}
	return freq;
#else
	#error "Please implement Clock Speed function for this platform"
#endif
#endif
}
#endif

double Sys_CalculateCPUFreq( void )
{
	return 0;
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



/*
================
Sys_GetClipboardData
================
*/
char *Sys_GetClipboardData( void )
{
	char *data = NULL;
	char *cliptext;

	if ( OpenClipboard( NULL ) ) {
		HANDLE hClipboardData;
		DWORD size;

		// GetClipboardData performs implicit CF_UNICODETEXT => CF_TEXT conversion
		if ( ( hClipboardData = GetClipboardData( CF_TEXT ) ) != 0 ) {
			if ( ( cliptext = (char *)GlobalLock( hClipboardData ) ) != 0 ) {
				size = GlobalSize( hClipboardData ) + 1;
				data = (char *)Z_Malloc( size, TAG_STATIC );
				N_strncpyz( data, cliptext, size );
				GlobalUnlock( hClipboardData );
				
				strtok( data, "\n\r\b" );
			}
		}
		CloseClipboard();
	}
	return data;
}


void Sys_SetClipboadBitmap( const byte *bitmap, uint64_t length )
{
    HGLOBAL hMem;
	byte *ptr;

	if ( !g_wv.hWnd || !OpenClipboard( g_wv.hWnd ) )
		return;

	EmptyClipboard();
	hMem = GlobalAlloc( GMEM_MOVEABLE | GMEM_DDESHARE, length );
	if ( hMem != NULL ) {
		ptr = ( byte* )GlobalLock( hMem );
		if ( ptr != NULL ) {
			memcpy( ptr, bitmap, length ); 
		}
		GlobalUnlock( hMem );
		SetClipboardData( CF_DIB, hMem );
	}
	CloseClipboard();
}

/*
==============
Sys_ErrorDialog

Display an error message
==============
*/
void Sys_ErrorDialog( const char *error )
{
	Sys_Print( va( "%s\n", error ) );

	if( Sys_Dialog( DT_YES_NO, va( "%s. Copy console log to clipboard?", error ),
			"Error" ) == DR_YES )
	{
		HGLOBAL memoryHandle;
		char *clipMemory;

		memoryHandle = GlobalAlloc( GMEM_MOVEABLE|GMEM_DDESHARE, CON_LogSize( ) + 1 );
		clipMemory = (char *)GlobalLock( memoryHandle );

		if( clipMemory )
		{
			char *p = clipMemory;
			char buffer[ 1024 ];
			unsigned int size;

			while( ( size = CON_LogRead( buffer, sizeof( buffer ) ) ) > 0 )
			{
				Com_Memcpy( p, buffer, size );
				p += size;
			}

			*p = '\0';

			if( OpenClipboard( NULL ) && EmptyClipboard( ) )
				SetClipboardData( CF_TEXT, memoryHandle );

			GlobalUnlock( clipMemory );
			CloseClipboard( );
		}
	}
}

/*
==============
Sys_Dialog

Display a win32 dialog box
==============
*/
dialogResult_t Sys_Dialog( dialogType_t type, const char *message, const char *title )
{
	UINT uType;

	switch( type )
	{
		default:
		case DT_INFO:      uType = MB_ICONINFORMATION|MB_OK; break;
		case DT_WARNING:   uType = MB_ICONWARNING|MB_OK; break;
		case DT_ERROR:     uType = MB_ICONERROR|MB_OK; break;
		case DT_YES_NO:    uType = MB_ICONQUESTION|MB_YESNO; break;
		case DT_OK_CANCEL: uType = MB_ICONWARNING|MB_OKCANCEL; break;
	}

	switch( MessageBox( NULL, message, title, uType ) )
	{
		default:
		case IDOK:      return DR_OK;
		case IDCANCEL:  return DR_CANCEL;
		case IDYES:     return DR_YES;
		case IDNO:      return DR_NO;
	}
}

/*
==================
WinMain
==================
*/
int WINAPI WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow ) 
{
	static char	sys_cmdline[ MAX_STRING_CHARS ];
	char con_title[ MAX_CVAR_VALUE ];
	int xpos, ypos;
	qboolean useXYpos;
	HANDLE hProcess;
	DWORD dwPriority;

	// should never get a previous instance in Win32
	if ( hPrevInstance ) {
		return 0;
	}

	// slightly boost process priority if it set to default
	hProcess = GetCurrentProcess();
	dwPriority = GetPriorityClass( hProcess );
	if ( dwPriority == NORMAL_PRIORITY_CLASS || dwPriority == ABOVE_NORMAL_PRIORITY_CLASS ) {
		SetPriorityClass( hProcess, HIGH_PRIORITY_CLASS );
	}

	//SetDPIAwareness();

	g_wv.hInstance = hInstance;
	N_strncpyz( sys_cmdline, lpCmdLine, sizeof( sys_cmdline ) );

	useXYpos = Com_EarlyParseCmdLine( sys_cmdline, con_title, sizeof( con_title ), &xpos, &ypos );

	// done before Com/Sys_Init since we need this for error output
	Sys_InitConsole();
	Sys_CreateConsole( con_title, xpos, ypos, useXYpos );

	// no abort/retry/fail errors
//	SetErrorMode( SEM_FAILCRITICALERRORS );

	SetUnhandledExceptionFilter( ExceptionFilter );

	// get the initial time base
	Sys_Milliseconds();

	Com_Init( sys_cmdline );

	Con_Printf( "Working directory: %s\n", Sys_pwd() );

	// main game loop
	while ( 1 ) {
		// set low precision every frame, because some system calls
		// reset it arbitrarily
		// _controlfp( _PC_24, _MCW_PC );
		// _controlfp( -1, _MCW_EM  ); // no exceptions, even if some crappy syscall turns them back on!

		Com_Frame( qfalse );
	}

	// never gets here
	return 0;
}

