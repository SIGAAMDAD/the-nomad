#include "n_shared.h"
#include "n_common.h"
#include "../game/g_game.h"
#include "../rendercommon/r_public.h"
#include "../rendercommon/imgui_impl_sdl2.h"
#include "vm_local.h"
#include <setjmp.h>
#include <zip.h>

#define MAX_EVENT_QUEUE 256
#define MASK_QUEUED_EVENTS (MAX_EVENT_QUEUE - 1)
#define MAX_PUSHED_EVENTS 256

fileHandle_t logfile = FS_INVALID_HANDLE;
static fileHandle_t com_journalFile = FS_INVALID_HANDLE;

static sysEvent_t eventQueue[MAX_EVENT_QUEUE];
static sysEvent_t *lastEvent = eventQueue + MAX_EVENT_QUEUE - 1;
static uint32_t eventHead = 0;
static uint32_t eventTail = 0;
static uint32_t com_pushedEventsHead;
static uint32_t com_pushedEventsTail;
static sysEvent_t com_pushedEvents[MAX_PUSHED_EVENTS];

uint32_t com_fps;
int CPU_flags;
cvar_t *com_demo;
cvar_t *com_journal;
cvar_t *com_logfile;
cvar_t *com_maxfps;
errorCode_t com_errorCode;
#ifdef USE_AFFINITY_MASK
cvar_t *com_affinityMask;
#endif
cvar_t *sys_cpuString;
cvar_t *com_devmode;
cvar_t *com_version;
static int lastTime;
int com_frameTime;
uint64_t com_frameNumber = 0;
uint64_t com_cacheLine; // L1 cacheline
char com_errorMessage[MAXPRINTMSG];
static jmp_buf abortframe;
qboolean com_errorEntered = qfalse;
qboolean com_fullyInitialized = qfalse;

/*
===============================================================

LOGGING

===============================================================
*/

static char	*rd_buffer;
static int	rd_buffersize;
static qboolean rd_flushing = qfalse;
static void	(*rd_flush)( const char *buffer );

void Com_BeginRedirect( char *buffer, uint64_t buffersize, void (*flush)(const char *) )
{
	if (!buffer || !buffersize || !flush)
		return;
	rd_buffer = buffer;
	rd_buffersize = buffersize;
	rd_flush = flush;

	*rd_buffer = '\0';
}


void Com_EndRedirect( void )
{
	if ( rd_flush ) {
		rd_flushing = qtrue;
		rd_flush( rd_buffer );
		rd_flushing = qfalse;
	}

	rd_buffer = NULL;
	rd_buffersize = 0;
	rd_flush = NULL;
}

void GDR_ATTRIBUTE((format(printf, 1, 2))) GDR_DECL Con_Printf(const char *fmt, ...)
{
    va_list argptr;
    int length;
    char msg[MAXPRINTMSG];
    static qboolean opening_console = qfalse;

	memset(msg, 0, sizeof(msg));
    va_start(argptr, fmt);
    length = N_vsnprintf(msg, sizeof(msg), fmt, argptr);
    va_end(argptr);

    if (rd_buffer && !rd_flushing) {
        if (length + strlen(rd_buffer) > (rd_buffersize + 1)) {
            rd_flushing = qtrue;
            rd_flush(rd_buffer);
            rd_flushing = qfalse;
            *rd_buffer = '\0';
        }
        N_strcat(rd_buffer, rd_buffersize, msg);
    }

    // append to the debug console buffer
	G_ConsolePrint(msg);

    // echo to the actual console
    Sys_Print(msg);

    // slap that shit into the logfile
    if (com_logfile && com_logfile->i) {
        if (logfile == FS_INVALID_HANDLE && FS_Initialized() && !opening_console) {
            const char *logName = "debug.log";
            int32_t mode;
            opening_console = qtrue;

            mode = com_logfile->i - 1;

            if (mode & 2)
                logfile = FS_FOpenAppend(logName);
            else
                logfile = FS_FOpenWrite(logName);
            
            if (logfile != FS_INVALID_HANDLE) {
                struct tm *newtime;
				time_t aclock;
				char timestr[32];

				time( &aclock );
				newtime = localtime( &aclock );
				strftime( timestr, sizeof( timestr ), "%a %b %d %X %Y", newtime );

				Con_Printf( "logfile opened on %s\n", timestr );

				if ( mode & 1 ) {
					// force it to not buffer so we get valid
					// data even if we are crashing
					FS_ForceFlush( logfile );
				}
			} else {
				Con_Printf( COLOR_YELLOW "Opening %s failed!\n", logName );
				Cvar_Set( "logfile", "0" );
			}
            opening_console = qfalse;
        }
        if (logfile != FS_INVALID_HANDLE && FS_Initialized()) {
			FS_Write( msg, strlen(msg), logfile );
        }
    }
}

void GDR_ATTRIBUTE((format(printf, 1, 2))) GDR_DECL Con_DPrintf(const char *fmt, ...)
{
	va_list argptr;
	char msg[MAXPRINTMSG];

	if ( !com_devmode || !com_devmode->i ) {
		return; // don't confuse non-developers with techie stuff... "it's a techy thing!"
	}

	va_start( argptr,fmt );
	N_vsnprintf( msg, sizeof( msg ), fmt, argptr );
	va_end( argptr );

	Con_Printf( COLOR_CYAN "%s", msg );
}

void GDR_NORETURN GDR_ATTRIBUTE((format(printf, 2, 3))) GDR_DECL N_Error(errorCode_t code, const char *err, ...)
{
	va_list argptr;
	static uint64_t lastErrorTime;
	static uint64_t errorCount;
	uint64_t currentTime;
	static qboolean calledSystemError = qfalse;

	if (com_errorEntered) {
		if (!calledSystemError) {
			calledSystemError = qtrue;
			Sys_Error("recursive error after: %s", com_errorMessage);
		}
	}
	com_errorEntered = qtrue;

	Cvar_Set( "com_errorCode", va( "%i" , code) );

	// if we are getting a solid stream of ERR_DROP, do an ERR_FATAL
	currentTime = Sys_Milliseconds();
	if ( currentTime - lastErrorTime < 100 ) {
		if ( ++errorCount > 3 ) {
			code = ERR_FATAL;
		}
	} else {
		errorCount = 0;
	}
	lastErrorTime = currentTime;

	va_start(argptr, err);
    N_vsnprintf(com_errorMessage, sizeof(com_errorMessage), err, argptr);
    va_end(argptr);

	Cbuf_Init();

	if (code == ERR_DROP) {
		Con_Printf( "********************\nERROR: %s\n********************\n",
			com_errorMessage );
		VM_Forced_Unload_Start();
		Com_EndRedirect();
		G_FlushMemory();
		VM_Forced_Unload_Done();

		com_errorEntered = qfalse;

		Q_longjmp( abortframe, 1 );
	} else { // ERR_FATAL
		VM_Forced_Unload_Start();
		G_ShutdownVMs();
		G_ShutdownRenderer(REF_UNLOAD_DLL);
		Com_EndRedirect();
		VM_Forced_Unload_Done();
	}

	Com_Shutdown();

	calledSystemError = qtrue;
	Sys_Error("%s", com_errorMessage);
}


/*
===============================================================

EVENT LOOP

===============================================================
*/


static void Com_InitPushEvent(void)
{
	// clear the static buffer array
	// this requires SE_NONE to be accepted as a valid but NOP event
	memset( com_pushedEvents, 0, sizeof(com_pushedEvents) );
	// reset counters while we are at it
	// beware: GetEvent might still return an SE_NONE from the buffer
	com_pushedEventsHead = 0;
	com_pushedEventsTail = 0;
}

static const char *Com_EventName(sysEventType_t evType)
{
	static const char *evNames[SE_MAX] = {
		"SE_NONE",
		"SE_KEY",
		"SE_MOUSE",
		"SE_JOYSTICK_AXIS",
		"SE_CONSOLE",
		"SE_WINDOW"
	};

	if ((unsigned)evType >= arraylen(evNames))
		return "SE_UNKOWN";
	else
		return evNames[evType];
}

static void Com_PushEvent(const sysEvent_t *event)
{
	sysEvent_t *ev;
	static qboolean printedWarning = qfalse;

	ev = &com_pushedEvents[com_pushedEventsTail & (MAX_EVENT_QUEUE - 1)];

	if (com_pushedEventsHead - com_pushedEventsTail >= MAX_EVENT_QUEUE) {
		// don't print the warning constantly, or it can give time for more...
		if (!printedWarning) {
			printedWarning = qtrue;
			Con_Printf(COLOR_YELLOW "Com_PushEvent: overflow\\n");
		}

		if (ev->evPtr) {
			Z_Free(ev->evPtr);
		}
		com_pushedEventsTail++;
	}
	else {
		printedWarning = qfalse;
	}

	*ev = *event;
	com_pushedEventsHead++;
}

#define MAX_CONSOLE_KEYS 16

static keynum_t Com_TranslateSDL2ToQ3Key( SDL_Keysym *keysym, qboolean down )
{
	keynum_t key = (keynum_t)0;

	if ( keysym->scancode >= SDL_SCANCODE_1 && keysym->scancode <= SDL_SCANCODE_0 ) {
		// Always map the number keys as such even if they actually map
		// to other characters (eg, "1" is "&" on an AZERTY keyboard).
		// This is required for SDL before 2.0.6, except on Windows
		// which already had this behavior.
		if ( keysym->scancode == SDL_SCANCODE_0 ) {
			key = '0';
		} else {
			key = '1' + keysym->scancode - SDL_SCANCODE_1 ;
		}
	} else {
		if ( keysym->scancode >= SDL_SCANCODE_A && keysym->scancode <= SDL_SCANCODE_Z ) {
			key = 'a' + keysym->scancode - SDL_SCANCODE_A;
		} else {
			switch ( keysym->scancode ) {
			case SDL_SCANCODE_MINUS:        key = '-';  break;
			case SDL_SCANCODE_EQUALS:       key = '=';  break;
			case SDL_SCANCODE_LEFTBRACKET:  key = '[';  break;
			case SDL_SCANCODE_RIGHTBRACKET: key = ']';  break;
			case SDL_SCANCODE_NONUSBACKSLASH:
			case SDL_SCANCODE_BACKSLASH:    key = '\\'; break;
			case SDL_SCANCODE_SEMICOLON:    key = ';';  break;
			case SDL_SCANCODE_APOSTROPHE:   key = '\''; break;
			case SDL_SCANCODE_COMMA:        key = ',';  break;
			case SDL_SCANCODE_PERIOD:       key = '.';  break;
			case SDL_SCANCODE_SLASH:        key = '/';  break;
			default:
				/* key = 0 */
				break;
			};
		}
	}

	if ( !key && keysym->sym >= SDLK_SPACE && keysym->sym < SDLK_DELETE ) {
		// These happen to match the ASCII chars
		key = (uint32_t)keysym->sym;
	}
	else if ( !key ) {
		switch ( keysym->sym ) {
			case SDLK_PAGEUP:       key = KEY_PAGEUP;      		break;
			case SDLK_KP_9:         key = KEY_KP_PAGEUP;    	break;
			case SDLK_PAGEDOWN:     key = KEY_PAGEDOWN;      	break;
			case SDLK_KP_3:         key = KEY_KP_PAGEDOWN;      break;
			case SDLK_KP_7:         key = KEY_KP_HOME;       	break;
			case SDLK_HOME:         key = KEY_HOME;          	break;
			case SDLK_KP_1:         key = KEY_KP_END;        	break;
			case SDLK_END:          key = KEY_END;           	break;
			case SDLK_KP_4:         key = KEY_KP_LEFT;  		break;
			case SDLK_LEFT:         key = KEY_LEFT;     		break;
			case SDLK_KP_6:         key = KEY_KP_RIGHT; 		break;
			case SDLK_RIGHT:        key = KEY_RIGHT;    		break;
			case SDLK_KP_2:         key = KEY_KP_DOWN;  		break;
			case SDLK_DOWN:         key = KEY_DOWN;     		break;
			case SDLK_KP_8:         key = KEY_KP_UP;    		break;
			case SDLK_UP:           key = KEY_UP;       		break;
			case SDLK_ESCAPE:       key = KEY_ESCAPE;        	break;
			case SDLK_KP_ENTER:     key = KEY_KP_ENTER;      	break;
			case SDLK_RETURN:       key = KEY_ENTER;         	break;
			case SDLK_TAB:          key = KEY_TAB;           	break;
			case SDLK_F1:           key = KEY_F1;            	break;
			case SDLK_F2:           key = KEY_F2;            	break;
			case SDLK_F3:           key = KEY_F3;            	break;
			case SDLK_F4:           key = KEY_F4;            	break;
			case SDLK_F5:           key = KEY_F5;            	break;
			case SDLK_F6:           key = KEY_F6;            	break;
			case SDLK_F7:           key = KEY_F7;            	break;
			case SDLK_F8:           key = KEY_F8;            	break;
			case SDLK_F9:           key = KEY_F9;            	break;
			case SDLK_F10:          key = KEY_F10;           	break;
			case SDLK_F11:          key = KEY_F11;           	break;
			case SDLK_F12:          key = KEY_F12;           	break;

			case SDLK_BACKSPACE:    key = KEY_BACKSPACE;     	break;
			case SDLK_KP_PERIOD:    key = KEY_KP_DELETE;        break;
			case SDLK_DELETE:       key = KEY_DELETE;           break;
			case SDLK_PAUSE:        key = KEY_PAUSE;         	break;

			case SDLK_LSHIFT:
			case SDLK_RSHIFT:       key = KEY_SHIFT;         	break;

			case SDLK_LCTRL:
			case SDLK_RCTRL:        key = KEY_CTRL;          	break;

#ifdef __APPLE__
			case SDLK_RGUI:
			case SDLK_LGUI:         key = KEY_COMMAND;       	break;
#else
			case SDLK_RGUI:
			case SDLK_LGUI:         key = KEY_SUPER;         	break;
#endif

			case SDLK_RALT:
			case SDLK_LALT:         key = KEY_ALT;           	break;

			case SDLK_KP_5:         key = KEY_KP_5;          	break;
			case SDLK_INSERT:       key = KEY_INSERT;           break;
			case SDLK_KP_0:         key = KEY_KP_INSERT;        break;
			case SDLK_KP_MULTIPLY:  key = '*'; /*K_KP_STAR;*/ 	break;
			case SDLK_KP_PLUS:      key = KEY_KP_PLUS;       	break;
			case SDLK_KP_MINUS:     key = KEY_KP_MINUS;      	break;
			case SDLK_KP_DIVIDE:    key = KEY_KP_SLASH;      	break;

			case SDLK_MODE:         key = KEY_MODE;          	break;
			case SDLK_HELP:         key = KEY_HELP;          	break;
			case SDLK_PRINTSCREEN:  key = KEY_SCREENSHOT;       break;
			case SDLK_SYSREQ:       key = KEY_SYSREQ;        	break;
			case SDLK_MENU:         key = KEY_MENU;          	break;
			case SDLK_APPLICATION:	key = KEY_MENU;          	break;
			case SDLK_POWER:        key = KEY_POWER;         	break;
			case SDLK_UNDO:         key = KEY_UNDO;          	break;
			case SDLK_SCROLLLOCK:   key = KEY_SCROLLOCK;     	break;
			case SDLK_NUMLOCKCLEAR: key = KEY_KP_NUMLOCK;    	break;
			case SDLK_CAPSLOCK:     key = KEY_CAPSLOCK;      	break;

			default:
#if 1
				key = 0;
#else
				if ( !( keysym->sym & SDLK_SCANCODE_MASK ) && keysym->scancode <= 95 ) {
					// Map Unicode characters to 95 world keys using the key's scan code.
					// FIXME: There aren't enough world keys to cover all the scancodes.
					// Maybe create a map of scancode to quake key at start up and on
					// key map change; allocate world key numbers as needed similar
					// to SDL 1.2.
					key = KEY_WORLD_0 + (int)keysym->scancode;
				}
#endif
				break;
		}
	}

	if ( keysym->scancode == SDL_SCANCODE_GRAVE ) {
		//SDL_Keycode translated = SDL_GetKeyFromScancode( SDL_SCANCODE_GRAVE );

		//if ( translated == SDLK_CARET )
		{
			// Console keys can't be bound or generate characters
			key = KEY_CONSOLE;
		}
	}

	return key;
}

static void Com_PumpKeyEvents(void)
{
	SDL_Event event;
	uint32_t in_eventTime;

	SDL_PumpEvents();

	in_eventTime = Sys_Milliseconds();

	while (SDL_PollEvent(&event)) {
		if ( Key_GetCatcher() & KEYCATCH_CONSOLE || Key_GetCatcher() & KEYCATCH_UI ) {
			ImGui_ImplSDL2_ProcessEvent(&event);
		}

		switch (event.type) {
		case SDL_KEYDOWN:
			Com_QueueEvent( in_eventTime, SE_KEY, Com_TranslateSDL2ToQ3Key( &event.key.keysym, qtrue ), qtrue, 0, NULL );
			break;
		case SDL_KEYUP:
			Com_QueueEvent( in_eventTime, SE_KEY, Com_TranslateSDL2ToQ3Key( &event.key.keysym, qfalse ), qfalse, 0, NULL );
			break;
		case SDL_WINDOWEVENT:
			switch (event.window.event) {
			case SDL_WINDOWEVENT_MOVED:
				if (!((SDL_GetWindowFlags(G_GetSDLWindow()) & SDL_WINDOW_FULLSCREEN) && (SDL_GetWindowFlags(G_GetSDLWindow()) & SDL_WINDOW_FULLSCREEN_DESKTOP))
				&& G_GetSDLWindow() && !(SDL_GetWindowFlags(G_GetSDLWindow()) & SDL_WINDOW_MINIMIZED)) {
//					Cvar_SetIntegerValue( "vid_xpos", event.window.data1 );
//					Cvar_SetIntegerValue( "vid_ypos", event.window.data2 );
				}
				break;
			case SDL_WINDOWEVENT_MINIMIZED:
			case SDL_WINDOWEVENT_FOCUS_LOST:
				Cvar_Set( "g_paused", "1" );
				Key_ClearStates();
				break;
			};
			break;
		case SDL_QUIT:
			Cbuf_ExecuteText( EXEC_NOW, "quit\n" );
			break;
		case SDL_MOUSEMOTION:
			if (!event.motion.xrel && !event.motion.yrel) {
				break;
			}
			Com_QueueEvent( com_frameTime, SE_MOUSE, event.motion.xrel, event.motion.yrel, 0, NULL );
			break;
		case SDL_MOUSEBUTTONUP:
		case SDL_MOUSEBUTTONDOWN:
			Com_QueueEvent( com_frameTime, SE_KEY, event.button.button, (event.type == SDL_MOUSEBUTTONDOWN ? qtrue : qfalse), 0, NULL );
			break;
		case SDL_MOUSEWHEEL:
			if (event.wheel.y > 0) {
				Com_QueueEvent( com_frameTime, SE_KEY, KEY_WHEEL_UP, qtrue, 0, NULL );
				Com_QueueEvent( com_frameTime, SE_KEY, KEY_WHEEL_UP, qfalse, 0, NULL );
			}
			else if (event.wheel.y < 0) {
				Com_QueueEvent( com_frameTime, SE_KEY, KEY_WHEEL_DOWN, qtrue, 0, NULL );
				Com_QueueEvent( com_frameTime, SE_KEY, KEY_WHEEL_DOWN, qtrue, 0, NULL );
			}
			break;
		default:
			break;
		};
	}
}

static sysEvent_t Com_GetSystemEvent(void)
{
	sysEvent_t ev;
	const char *s;
	int evTime;

	// return if we have data
	if (eventHead - eventTail > 0)
		return eventQueue[(eventTail++) & MASK_QUEUED_EVENTS];
	
	Com_PumpKeyEvents();

	evTime = Sys_Milliseconds();

	// check for console commands
	s = Sys_ConsoleInput();
	if (s) {
		char *b;
		uint64_t len;

		len = strlen(s) + 1;
		b = (char *)Z_Malloc(len, TAG_STATIC);
		strcpy(b, s);
		Com_QueueEvent( evTime, SE_CONSOLE, 0, 0, len, b );
	}

	// return if we have data
	if (eventHead - eventTail > 0)
		return eventQueue[(eventTail++) & MASK_QUEUED_EVENTS];
	
	// create a new empty event to return
	memset(&ev, 0, sizeof(ev));
	ev.evTime = evTime;

	return ev;
}

static sysEvent_t Com_GetRealEvent(void)
{
	// get or save an event from/to the journal file
	if (com_journalFile != FS_INVALID_HANDLE) {
		uint64_t r;
		sysEvent_t ev;

		if (com_journal->i == JOURNAL_PLAYBACK) {
			Com_PumpKeyEvents();
			r = FS_Read(&ev, ev.evPtrLength, com_journalFile);
			if (r != sizeof(ev)) {
				N_Error(ERR_FATAL, "Error reading from journal file");
			}
			if (ev.evPtrLength) {
				ev.evPtr = Z_Malloc(ev.evPtrLength, TAG_STATIC);
				r = FS_Read(ev.evPtr, ev.evPtrLength, com_journalFile);
				if (r != ev.evPtrLength) {
					N_Error(ERR_FATAL, "Error reading from journal file");
				}
			}
		}
		else {
			ev = Com_GetSystemEvent();

			// write the journal value out if needed
			if (com_journal->i == JOURNAL_WRITE) {
				r = FS_Write(&ev, sizeof(ev), com_journalFile);
				if (r != sizeof(ev)) {
					N_Error(ERR_FATAL, "Error writing to journal file");
				}
				if (ev.evPtrLength) {
					r = FS_Write(ev.evPtr, ev.evPtrLength, com_journalFile);
					if (r != ev.evPtrLength) {
						N_Error(ERR_FATAL, "Error writing to journal file");
					}
				}
			}
		}

		return ev;
	}

	return Com_GetSystemEvent();
}

void Com_QueueEvent(uint32_t evTime, sysEventType_t evType, uint32_t evValue, uint32_t evValue2, uint32_t ptrLength, void *ptr)
{
	sysEvent_t *ev;

	if (evTime == 0) {
		evTime = Sys_Milliseconds();
	}

	// try to combine all sequential mouse moves in one event
	if (evType == SE_MOUSE && lastEvent->evType == SE_MOUSE && eventHead != eventTail) {
		lastEvent->evValue += evValue;
		lastEvent->evValue2 += evValue2;
		lastEvent->evTime = evTime;
		return;
	}

	ev = &eventQueue[eventHead & MASK_QUEUED_EVENTS];

	if (eventHead - eventTail >= MAX_EVENT_QUEUE) {
		Con_Printf("%s(type=%s,keys=(%i,%i),time=%i): overflow\n", __func__, Com_EventName(evType), evValue, evValue2, evTime);
		// we are discarding an event, but avoid leaking memory
		if (ev->evPtr) {
			Z_Free(ev->evPtr);
		}
		eventTail++;
	}

	eventHead++;

	ev->evTime = evTime;
	ev->evType = evType;
	ev->evValue = evValue;
	ev->evValue2 = evValue2;
	ev->evPtrLength = ptrLength;
	ev->evPtr = ptr;

	lastEvent = ev;
}

static sysEvent_t Com_GetEvent(void)
{
	if (com_pushedEventsHead - com_pushedEventsTail > 0) {
		return com_pushedEvents[(com_pushedEventsTail++) & (MAX_EVENT_QUEUE - 1)];
	}

	return Com_GetRealEvent();
}

uint64_t Com_EventLoop(void)
{
	sysEvent_t ev;

	while (1) {
		ev = Com_GetEvent();

		// no more events are available
		if (ev.evType == SE_NONE) {
			return ev.evTime;
		}

		switch ( ev.evType ) {
		case SE_KEY:
			G_KeyEvent( ev.evValue, (qboolean)ev.evValue2, ev.evTime );
			break;
		case SE_MOUSE:
			G_MouseEvent(ev.evValue, ev.evValue2);
			break;
		case SE_CONSOLE:
			Cbuf_AddText((char *)ev.evPtr);
			Cbuf_AddText("\n");
			break;
		default:
			N_Error(ERR_FATAL, "Com_EventLoop: bad event type %i", ev.evType);
		};

		// free any block data
		if (ev.evPtr) {
			Z_Free(ev.evPtr);
			ev.evPtr = NULL;
		}
	}

	return 0; // never reached
}

//
// Com_Error_f: Just throw a fatal error to
// test error shutdown procedures
//
static void GDR_NORETURN Com_Error_f (void)
{
	if ( Cmd_Argc() > 1 ) {
		N_Error( ERR_DROP, "Testing drop error" );
	} else {
		N_Error( ERR_FATAL, "Testing fatal error" );
	}
}

//
// Com_Freeze_f: Just freeze in place for a given number of
// seconds to test error recovery
//
static void Com_Freeze_f( void ) {
	int		s;
	int		start, now;

	if ( Cmd_Argc() != 2 ) {
		Con_Printf( "freeze <seconds>\n" );
		return;
	}
	s = atoi( Cmd_Argv(1) ) * 1000;

	start = Com_Milliseconds();

	while ( 1 ) {
		now = Com_Milliseconds();
		if ( now - start > s ) {
			break;
		}
	}
}

#define NOMAD_CONFIG "glnomad.cfg"

//
// Com_LoadConfig: loads the default configuration file
//
void Com_LoadConfig( void )
{
    Cbuf_ExecuteText(EXEC_APPEND, va("exec %s", Cvar_VariableString( "com_defaultcfg" )));
    Cbuf_Execute();
}

/*
* Com_Crash_f: force crash, only for devs
*/
static void Com_Crash_f( void ) {
    *((volatile int *)0) = 0x12346789;
}

/*
* Com_Shutdown_f: for testing exit/crashing processes
*/
static void Com_Shutdown_f( void ) {
    N_Error( ERR_FATAL, "testing" );
}

//
// Com_ExecuteCfg: for controlling engine variables
//
static void Com_ExecuteCfg(void)
{
	Cbuf_ExecuteText(EXEC_NOW, "exec default.cfg\n");
	Cbuf_Execute(); // Always execute after exec to prevent text buffer overflowing
}

/*
* Com_InitJournals: initializes the logfile and the event journal
*/
void Com_InitJournals(void)
{
	if (!com_journal->i) { // no journaling wanted
		return;
	}

	Con_Printf( "Checking for jounaling...\n" );
	if (com_journal->i == 1) {
		Con_Printf( "Journaling events.\n" );

		com_journalFile = FS_FOpenWrite( "journal.dat" );
	} else if (com_journal->i == 2) {
		Con_Printf( "Replaying journaled events.\n" );

		FS_FOpenFileRead( "journal.dat", &com_journalFile );
	}

	if (com_journalFile == FS_INVALID_HANDLE) {
		Cvar_Set( "com_journal", "0" );
		Con_Printf( "Couldn't open journal file.\n" );
	}
}

void Com_RestartGame( void )
{
	static qboolean gameRestarting = qfalse;

	// make sure no recursion is possible
	if (!gameRestarting && com_fullyInitialized) {
		gameRestarting = qtrue;

		Con_Printf("Com_RestartGame: restarting all engine systems\n");

		G_ShutdownAll();
		G_ClearMem();

		// reset console history
		Con_ResetHistory();

		// clear the filesystem before restarting the cvars
		FS_Shutdown( qtrue );

		// reset all cvars
		Cvar_Restart( qtrue );

		FS_Restart();

		// reload the config file
		Com_LoadConfig();

		G_StartHunkUsers();

		gameRestarting = qfalse;
	}
}

/*
* Com_Shutdown: closes logging files
*/
void Com_Shutdown(void)
{
	if (logfile != FS_INVALID_HANDLE) {
		FS_FClose( logfile );
		logfile = FS_INVALID_HANDLE;
	}
	if (com_journalFile != FS_INVALID_HANDLE) {
		FS_FClose( com_journalFile );
		com_journalFile = FS_INVALID_HANDLE;
	}
}

#ifdef USE_AFFINITY_MASK
static uint64_t eCoreMask;
static uint64_t pCoreMask;
static uint64_t affinityMask; // saved at startup
#endif

#if (GDRx664 || GDRi386)

#ifdef _MSC_VER
#include <intrin.h>
static void CPUID(int func, uint32_t *regs)
{
	__cpuid((int *)regs, func);
}

#ifdef USE_AFFINITY_MASK
#if GDRx64
extern void CPUID_EX(int func, int param, uint32_t *regs);
#else
void CPUID_EX(int func, int param, uint32_t *regs)
{
	__asm {
		push edi
		mov eax, func
		mov ecx, param
		cpuid
		mov edi, regs
		mov [edi +0], eax
		mov [edi +4], ebx
		mov [edi +8], ecx
		mov [edi+12], edx
		pop edi
	}
}
#endif
#endif

#else // gcc/mingw/clang

static void CPUID(int func, uint32_t *regs)
{
	__asm__ __volatile__( "cpuid" :
		"=a"(regs[0]),
		"=b"(regs[1]),
		"=c"(regs[2]),
		"=d"(regs[3]) :
		"a"(func) );
}

#ifdef USE_AFFINITY_MASK
static void CPUID_EX( int func, int param, unsigned int *regs )
{
	__asm__ __volatile__( "cpuid" :
		"=a"(regs[0]),
		"=b"(regs[1]),
		"=c"(regs[2]),
		"=d"(regs[3]) :
		"a"(func),
		"c"(param) );
}
#endif

#endif

static void Sys_GetProcessorId( char *vendor )
{
	uint32_t regs[4];; // eax, ebx, ecx, edx
	uint32_t cpuid_level_ex;
	char vendor_str[12 + 1]; // short CPU vendor string
	
	// setup initial features
#ifdef GDRx64
	CPU_flags |= CPU_SSE | CPU_SSE2;
#else
	CPU_flags = 0;
#endif
	vendor[0] = '\0';

	CPUID( 0x80000000, regs );
	cpuid_level_ex = regs[0];

	// get CPUID level & short CPU vendor string
	CPUID( 0x0, regs );
	memcpy(vendor_str + 0, (char*)&regs[1], 4);
	memcpy(vendor_str + 4, (char*)&regs[3], 4);
	memcpy(vendor_str + 8, (char*)&regs[2], 4);
	vendor_str[12] = '\0';

	// get CPU feature bits
	CPUID( 0x1, regs );

	// bit 15 of EDX denotes CMOV/FCMOV/FCOMI existence
#ifdef CPU_FCOM
	if ( regs[3] & ( 1 << 15 ) )
		CPU_flags |= CPU_FCOM;
#endif

	// bit 23 of EDX denotes MMX existence
	if ( regs[3] & ( 1 << 23 ) )
		CPU_flags |= CPU_MMX;

	// bit 25 of EDX denotes SSE existence
	if ( regs[3] & ( 1 << 25 ) )
		CPU_flags |= CPU_SSE;

	// bit 26 of EDX denotes SSE2 existence
	if ( regs[3] & ( 1 << 26 ) )
		CPU_flags |= CPU_SSE2;

	// bit 0 of ECX denotes SSE3 existence
	//if ( regs[2] & ( 1 << 0 ) )
	//	CPU_Flags |= CPU_SSE3;

	// bit 19 of ECX denotes SSE41 existence
	if ( regs[ 2 ] & ( 1 << 19 ) )
		CPU_flags |= CPU_SSE41;

	if ( vendor ) {
		if ( cpuid_level_ex >= 0x80000004 ) {
			// read CPU Brand string
			uint32_t i;
			for ( i = 0x80000002; i <= 0x80000004; i++) {
				CPUID( i, regs );
				memcpy( vendor+0, (char*)&regs[0], 4 );
				memcpy( vendor+4, (char*)&regs[1], 4 );
				memcpy( vendor+8, (char*)&regs[2], 4 );
				memcpy( vendor+12, (char*)&regs[3], 4 );
				vendor[16] = '\0';
				vendor += strlen( vendor );
			}
		} else {
			const int print_flags = CPU_flags;
			vendor = N_stradd( vendor, vendor_str );
			if (print_flags) {
				// print features
				strcat(vendor, " w/");
			#ifdef CPU_FCOM
				if (print_flags & CPU_FCOM)
					strcat(vendor, " CMOV");
			#endif
				if (print_flags & CPU_MMX)
					strcat(vendor, " MMX");
				if (print_flags & CPU_SSE)
					strcat(vendor, " SSE");
				if (print_flags & CPU_SSE2)
					strcat(vendor, " SSE2");
				//if ( CPU_Flags & CPU_SSE3 )
				//	strcat( vendor, " SSE3" );
				if (print_flags & CPU_SSE41)
					strcat(vendor, " SSE4.1");
			}
		}
	}
}

#ifdef USE_AFFINITY_MASK
static void DetectCPUCoresConfig( void )
{
	uint32_t regs[4];
	uint32_t i;

	// get highest function parameter and vendor id
	CPUID( 0x0, regs );
	if ( regs[1] != 0x756E6547 || regs[2] != 0x6C65746E || regs[3] != 0x49656E69 || regs[0] < 0x1A ) {
		// non-intel signature or too low cpuid level - unsupported
		eCoreMask = pCoreMask = affinityMask;
		return;
	}

	eCoreMask = 0;
	pCoreMask = 0;

	for ( i = 0; i < sizeof( affinityMask ) * 8; i++ ) {
		const uint64_t mask = 1ULL << i;
		if ( (mask & affinityMask) && Sys_SetAffinityMask( mask ) ) {
			CPUID_EX( 0x1A, 0x0, regs );
			switch ( (regs[0] >> 24) & 0xFF ) {
				case 0x20: eCoreMask |= mask; break;
				case 0x40: pCoreMask |= mask; break;
				default: // non-existing leaf
					eCoreMask = pCoreMask = 0;
					break;
			}
		}
	}

	// restore original affinity
	Sys_SetAffinityMask( affinityMask );

	if ( pCoreMask == 0 || eCoreMask == 0 ) {
		// if either mask is empty - assume non-hybrid configuration
		eCoreMask = pCoreMask = affinityMask;
	}
}
#endif // USE_AFFINITY_MASK

#else // non-x86

#ifndef __linux__

static void Sys_GetProcessorId( char *vendor )
{
	Com_snprintf( vendor, 100, "%s", ARCH_STRING );
}

#else // __linux__

#include <sys/auxv.h>

#if arm32
#include <asm/hwcap.h>
#endif

static void Sys_GetProcessorId( char *vendor )
{
#if arm32
	const char *platform;
	long hwcaps;
	CPU_flags = 0;

	platform = (const char*)getauxval( AT_PLATFORM );

	if ( !platform || *platform == '\0' ) {
		platform = "(unknown)";
	}

	if ( platform[0] == 'v' || platform[0] == 'V' ) {
		if ( atoi( platform + 1 ) >= 7 ) {
			CPU_flags |= CPU_ARMv7;
		}
	}

	sprintf( vendor, 100, "ARM %s", platform );
	hwcaps = getauxval( AT_HWCAP );
	if ( hwcaps & ( HWCAP_IDIVA | HWCAP_VFPv3 ) ) {
		strcat( vendor, " /w" );

		if ( hwcaps & HWCAP_IDIVA ) {
			CPU_flags |= CPU_IDIVA;
			strcat( vendor, " IDIVA" );
		}

		if ( hwcaps & HWCAP_VFPv3 ) {
			CPU_flags |= CPU_VFPv3;
			strcat( vendor, " VFPv3" );
		}

		if ( ( CPU_flags & ( CPU_ARMv7 | CPU_VFPv3 ) ) == ( CPU_ARMv7 | CPU_VFPv3 ) ) {
			strcat( vendor, " QVM-bytecode" );
		}
	}
#else // !arm32
	CPU_flags = 0;
#if arm64
	snprintf( vendor, 100, "%s", ARCH_STRING );
#else
	snprintf( vendor, 128, "%s %s", ARCH_STRING, (const char*)getauxval( AT_PLATFORM ) );
#endif
#endif // !arm32
}

#endif // __linux__

#endif // non-x86


#ifdef USE_AFFINITY_MASK

static int hex_code( const int code ) {
	if ( code >= '0' && code <= '9' ) {
		return code - '0';
	}
	if ( code >= 'A' && code <= 'F' ) {
		return code - 'A' + 10;
	}
	if ( code >= 'a' && code <= 'f' ) {
		return code - 'a' + 10;
	}
	return -1;
}


static const char *parseAffinityMask( const char *str, uint64_t *outv, int level ) {
	uint64_t v, mask = 0;

	while ( *str != '\0' ) {
		if ( *str == 'A' || *str == 'a' ) {
			mask = affinityMask;
			++str;
			continue;
		}
		else if ( *str == 'P' || *str == 'p' ) {
			mask = pCoreMask;
			++str;
			continue;
		}
		else if ( *str == 'E' || *str == 'e' ) {
			mask = eCoreMask;
			++str;
			continue;
		}
		else if ( *str == '0' && (str[1] == 'x' || str[1] == 'X') && (v = hex_code( str[2] )) >= 0 ) {
			int hex;
			str += 3; // 0xH
			while ( (hex = hex_code( *str )) >= 0 ) {
				v = v * 16 + hex;
				str++;
			}
			mask = v;
			continue;
		}
		else if ( *str >= '0' && *str <= '9' ) {
			mask = *str++ - '0';
			while ( *str >= '0' && *str <= '9' ) {
				mask = mask * 10 + *str - '0';
				++str;
			}
			continue;
		}

		if ( level == 0 ) {
			while ( *str == '+' || *str == '-' ) {
				str = parseAffinityMask( str + 1, &v, level + 1 );
				switch ( *str ) {
					case '+': mask |= v; break;
					case '-': mask &= ~v; break;
					default: str = ""; break;
				}
			}
			if ( *str != '\0' ) {
				++str; // skip unknown characters
			}
		} else {
			break;
		}
	}

	*outv = mask;
	return str;
}


// parse and set affinity mask
static void Com_SetAffinityMask( const char *str )
{
	uint64_t mask = 0;

	parseAffinityMask( str, &mask, 0 );

	if ( ( mask & affinityMask ) == 0 ) {
		mask = affinityMask; // reset to default
	}

	if ( mask != 0 ) {
		Sys_SetAffinityMask( mask );
	}
}
#endif // USE_AFFINITY_MASK

const char *Com_VersionString(void)
{
	const char *versionType;

#ifdef _NOMAD_DEBUG
	versionType = "glnomad debug";
#elif defined(_NOMAD_EXPERIMENTAL)
	versionType = "glnomad experimental";
#else
	versionType = "glnomad official";
#endif

	return va("%s v%i.%i.%i", versionType, NOMAD_VERSION, NOMAD_VERSION_UPDATE, NOMAD_VERSION_PATCH);
}

//
// Com_GameRestart_f: expose possibility to change current running mod to the user
//
static void Com_GameRestart_f(void)
{
	Cvar_Set("fs_game", Cmd_Argv(1));
	Com_RestartGame();
}



/*
============================================================================

COMMAND LINE FUNCTIONS

+ characters separate the commandLine string into multiple console
command lines.

All of these are valid:

quake3 +set test blah +map test
quake3 set test blah+map test
quake3 set test blah + map test

============================================================================
*/

#define	MAX_CONSOLE_LINES 64
static int	com_numConsoleLines;
static char	*com_consoleLines[MAX_CONSOLE_LINES];

int I_GetParm(const char *parm)
{
	int i;
	const char *name;
    if (!parm)
        N_Error(ERR_FATAL, "I_GetParm: parm is NULL");

	for (i = 0; i < com_numConsoleLines; i++) {
		Cmd_TokenizeString(com_consoleLines[i]);
		if (N_stricmp(Cmd_Argv(0), "set"))
			continue;
		
		name = Cmd_Argv(1);
		if (!parm || N_stricmp(name, parm) == 0) {
			return i;
		}
    }
    return -1;
}

/*
===============
Com_StartupVariable

Searches for command line parameters that are set commands.
If match is not NULL, only that cvar will be looked for.
That is necessary because cddir and basedir need to be set
before the filesystem is started, but all other sets should
be after execing the config and default.
===============
*/
void Com_StartupVariable( const char *match )
{
	int i;
	const char *name;

	for ( i = 0; i < com_numConsoleLines; i++ ) {
		Cmd_TokenizeString( com_consoleLines[i] );
		if ( N_stricmp( Cmd_Argv( 0 ), "set" ) ) {
			continue;
		}

		name = Cmd_Argv( 1 );
		if ( !match || N_stricmp( name, match ) == 0 ) {
			if ( Cvar_Flags( name ) == CVAR_NONEXISTENT )
				Cvar_Get( name, Cmd_ArgsFrom( 2 ), CVAR_USER_CREATED );
			else
				Cvar_Set2( name, Cmd_ArgsFrom( 2 ), qfalse );
		}
	}
}

/*
==================
Com_ParseCommandLine

Break it up into multiple console lines
==================
*/
static void Com_ParseCommandLine( char *commandLine )
{
	static int parsed = 0;
	int inq;

	if ( parsed )
		return;

	inq = 0;
	com_consoleLines[0] = commandLine;

	while ( *commandLine ) {
		if (*commandLine == '"') {
			inq = !inq;
		}
		// look for a + separating character
		// if commandLine came from a file, we might have real line separators
		if ( (*commandLine == '+' && !inq) || *commandLine == '\n'  || *commandLine == '\r' ) {
			if ( com_numConsoleLines == MAX_CONSOLE_LINES ) {
				break;
			}
			com_consoleLines[com_numConsoleLines] = commandLine + 1;
			com_numConsoleLines++;
			*commandLine = '\0';
		}
		commandLine++;
	}
	parsed = 1;
}

char cl_title[ MAX_CVAR_VALUE ] = WINDOW_TITLE;

/*
===================
Com_EarlyParseCmdLine

returns qtrue if both vid_xpos and vid_ypos was set
===================
*/
qboolean Com_EarlyParseCmdLine( char *commandLine, char *con_title, int title_size, int *vid_xpos, int *vid_ypos )
{
	uint32_t flags = 0;
	uint32_t i;

	*con_title = '\0';
	Com_ParseCommandLine( commandLine );

	for ( i = 0 ; i < com_numConsoleLines ; i++ ) {
		Cmd_TokenizeString( com_consoleLines[i] );
		if ( !N_stricmpn( Cmd_Argv(0), "set", 3 ) && !N_stricmp( Cmd_Argv(1), "cl_title" ) ) {
			com_consoleLines[i][0] = '\0';
			N_strncpyz( cl_title, Cmd_ArgsFrom( 2 ), sizeof(cl_title) );
			continue;
		}
		if ( !N_stricmp( Cmd_Argv(0), "cl_title" ) ) {
			com_consoleLines[i][0] = '\0';
			N_strncpyz( cl_title, Cmd_ArgsFrom( 1 ), sizeof(cl_title) );
			continue;
		}
		if ( !N_stricmpn( Cmd_Argv(0), "set", 3 ) && !N_stricmp( Cmd_Argv(1), "con_title" ) ) {
			com_consoleLines[i][0] = '\0';
			N_strncpyz( con_title, Cmd_ArgsFrom( 2 ), title_size );
			continue;
		}
		if ( !N_stricmp( Cmd_Argv(0), "con_title" ) ) {
			com_consoleLines[i][0] = '\0';
			N_strncpyz( con_title, Cmd_ArgsFrom( 1 ), title_size );
			continue;
		}
		if ( !N_stricmpn( Cmd_Argv(0), "set", 3 ) && !N_stricmp( Cmd_Argv(1), "vid_xpos" ) ) {
			*vid_xpos = atoi( Cmd_Argv( 2 ) );
			flags |= 1;
			continue;
		}
		if ( !N_stricmp( Cmd_Argv(0), "vid_xpos" ) ) {
			*vid_xpos = atoi( Cmd_Argv( 1 ) );
			flags |= 1;
			continue;
		}
		if ( !N_stricmpn( Cmd_Argv(0), "set", 3 ) && !N_stricmp( Cmd_Argv(1), "vid_ypos" ) ) {
			*vid_ypos = atoi( Cmd_Argv( 2 ) );
			flags |= 2;
			continue;
		}
		if ( !N_stricmp( Cmd_Argv(0), "vid_ypos" ) ) {
			*vid_ypos = atoi( Cmd_Argv( 1 ) );
			flags |= 2;
			continue;
		}
	}

	return (flags == 3) ? qtrue : qfalse ;
}

/*
=============
Com_Quit_f

provided as a command so that it can be used
from the VMs
=============
*/
void Com_Quit_f( void ) {
	const char *p = Cmd_ArgsFrom( 1 );
	// don't try to shutdown if we are in a recursive error
	if ( !com_errorEntered ) {
		// Some VMs might execute "quit" command directly,
		// which would trigger an unload of active VM error.
		// Sys_Quit will kill this process anyways, so
		// a corrupt call stack makes no difference
		VM_Forced_Unload_Start();
		G_Shutdown(qtrue);
		VM_Forced_Unload_Done();
		Com_Shutdown();
		FS_Shutdown( qtrue );
	}
	Sys_Exit(EXIT_SUCCESS);
}

/*
================
Com_Milliseconds

Can be used for profiling, but will be journaled accurately
================
*/
int Com_Milliseconds( void )
{
	sysEvent_t	ev;

	// get events and push them until we get a null event with the current time
	do {
		ev = Com_GetRealEvent();
		if ( ev.evType != SE_NONE ) {
			Com_PushEvent( &ev );
		}
	} while ( ev.evType != SE_NONE );

	return ev.evTime;
}

static void Com_WriteConfig_f( void );

static void Com_PrintDivider( void ) {
	for (uint32_t i = 0; i < 75; ++i) {
		Con_Printf( "=" );
	}
	Con_Printf("\n");
}

static void Com_PrintBanner( const char *msg ) {
	uint32_t spaces = 35 - (strlen( msg ) / 2);

	for (uint32_t i = 0; i < spaces; ++i) {
		Con_Printf( " " );
	}
	Con_Printf( "%s\n", msg );
}

static void Com_PrintVersionStrings( const char *commandLine )
{
	SDL_version sdl_version;
	char version_str[1024];
	char gamedesc[1024];

	//
	// print program version and exit
	//
	if (strstr( commandLine, "--version" ) || strstr( commandLine, "-version" )) {
		Sys_Print( GLN_VERSION "\n" );
		Sys_Exit(1);
	}

	SDL_GetVersion( &sdl_version );
	Com_snprintf( version_str, sizeof(version_str), "%i.%i.%i", sdl_version.major, sdl_version.minor, sdl_version.patch );

#ifdef _NOMAD_DEBUG
	Com_snprintf( gamedesc, sizeof(gamedesc), "The Nomad (Debug)" );
#elif defined(_NOMAD_EXPERIMENTAL)
	Com_snprintf( gamedesc, sizeof(gamedesc), "The Nomad (Experimental)" ):
#elif defined(_NOMAD_DEMO)
	Com_snprintf( gamedesc, sizeof(gamedesc), "The Nomad (Demo)" );
#elif _NOMAD_VERSION_UPDATE == 2
	Com_snprintf( gamedesc, sizeof(gamedesc), "The Nomad (Alpha Test)" );
#elif _NOMAD_VERSION_UPDATE == 3
	Com_snprintf( gamedesc, sizeof(gamedesc), "The Nomad (Beta Test)" );
#elif _NOMAD_VERSION == 2
	Com_snprintf( gamedesc, sizeof(gamedesc), "The Nomad (Early Access)" );
#else
	Com_snprintf( gamedesc, sizeof(gamedesc), "The Nomad" );
#endif
	
	Com_PrintDivider();
	Com_PrintBanner( gamedesc );
	Com_PrintDivider();
	Con_Printf(
		" The Nomad is free software, covered by the GNU General Public\n"
	    " License v2.0. There is NO warranty; not even for MERCHANTABILITY or\n"
		" FITNESS FOR A PARTICULAR PURPOSE. You are welcome to change and\n"
		" distribute copies under certain conditions. See the source for\n"
		" more information.\n"
	);
	Com_PrintDivider();

	Con_Printf(
		COLOR_BLUE "game version: %s\n"
		COLOR_BLUE "sdl version: %s\n"
		COLOR_BLUE "platform: " OS_STRING ", " ARCH_STRING "\n"
		COLOR_BLUE "date compiled: " __DATE__ "\n",
	Com_VersionString(), version_str);
}

static void Com_CrashCheck( void )
{
	FILE *fp;

	fp = Sys_FOpen( "nomad.pid", "w+" );
	if ( fp ) {
		zip_t *zf;
		zip_source_t *source;
		int error;

		Sys_MessageBox( "WARNING", "nomad.pid is present meaning the game had an unnatural exit, please report this bug", false );

		zf = zip_open( "crash-report.zip", ZIP_CREATE | ZIP_TRUNCATE, &error );
		if ( !zf ) {
			Con_Printf( COLOR_RED "ERROR: Failed to create crash-report.zip, please send the debug.log!\n" );
			return;
		}

		source = zip_source_file( zf,  );
		zip_add( zf, "", );
	}
}

/*
* Com_Init: initializes all the engine's systems
*/
void Com_Init( char *commandLine )
{
	cvar_t *cv;

	com_fullyInitialized = qfalse;

	Com_PrintVersionStrings( commandLine );

	// get the cacheline for optimized allocations and memory management
	com_cacheLine = SDL_GetCPUCacheLineSize();

	if ( Q_setjmp( abortframe ) ) {
		Sys_Error( "Error during initialization" );
	}

	Com_InitPushEvent();

	Z_InitSmallZoneMemory();
	Cvar_Init();

	// prepare enough of the subsystems to handle
	// cvar and command buffer management
	Com_ParseCommandLine( commandLine );

	Cbuf_Init();

	// override anything from the config files with command line args
	Com_StartupVariable( NULL );

	Z_InitMemory();
	Cmd_Init();

	// get the developer cvar set as early as possible
	Com_StartupVariable( "com_devmode" );
#ifdef _NOMAD_DEBUG
	com_devmode = Cvar_Get( "com_devmode", "1", CVAR_INIT | CVAR_PROTECTED );
#else
	com_devmode = Cvar_Get( "com_devmode", "0", CVAR_INIT | CVAR_PROTECTED );
#endif
	Cvar_CheckRange(com_devmode, "0", "1", CVT_INT);
	Cvar_SetDescription(com_devmode, "Enables a bunch of extra diagnostics for developers.");

	cv = Cvar_Get( "com_version", GLN_VERSION, CVAR_INIT | CVAR_ROM | CVAR_CHEAT );
	Cvar_SetDescription( cv, "The current game binary's version." );

	Com_StartupVariable( "com_defaultcfg" );
	Cvar_Get( "com_defaultcfg", NOMAD_CONFIG, CVAR_TEMP | CVAR_INIT );

	Com_StartupVariable("vm_rtChecks");
	vm_rtChecks = Cvar_Get("vm_rtChecks", "15", CVAR_INIT | CVAR_PROTECTED);
	Cvar_CheckRange(vm_rtChecks, "0", "15", CVT_INT);
	Cvar_SetDescription(vm_rtChecks,
		"Runtime checks in compiled vm code, bitmask:\n 1 - program stack overflow\n" \
		" 2 - opcode stack overflow\n 4 - jump target range\n 8 - data read/write range");
	
	com_demo = Cvar_Get("com_demo", "0", CVAR_LATCH);

	Com_StartupVariable("com_journal");
	com_journal = Cvar_Get("com_journal", "0", CVAR_INIT | CVAR_PROTECTED);
	Cvar_CheckRange(com_journal, "0", "2", CVT_INT);
	Cvar_SetDescription(com_journal, "When enabled, writes events and its data to 'journal.dat'");

	// done early so bind command exists
	Com_InitKeyCommands();

	FS_InitFilesystem();

	Com_StartupVariable("logfile");
	com_logfile = Cvar_Get("logfile", "1", CVAR_TEMP);
	Cvar_CheckRange(com_logfile, "0", "4", CVT_INT);
	Cvar_SetDescription(com_logfile, "System console logging:\n"
									" 0 - disabled\n"
									" 1 - overwrite mode, buffered\n"
									" 2 - overwrite mode, synced\n"
									" 3 - append mode, buffered\n"
									" 4 - append mode, synced\n");

	Com_InitJournals();
	Com_LoadConfig();

	// override anything from the config files with command line args
	Com_StartupVariable( NULL );

	// allocate the stack based hunk allocator
	Hunk_InitMemory();

	// if any archived cvars are modified after this, we will trigger a writing
	// of the config file
	cvar_modifiedFlags &= ~CVAR_SAVE;

	//
	// init commands and vars
	//

	if (com_devmode->i) {
		Cmd_AddCommand("freeze", Com_Freeze_f);
		Cmd_AddCommand("error", Com_Error_f);
		Cmd_AddCommand("crash", Com_Crash_f);
	}

	com_maxfps = Cvar_Get("com_maxfps", "60", CVAR_LATCH | CVAR_SAVE | CVAR_PROTECTED);
#ifdef USE_AFFINITY_MASK
	com_affinityMask = Cvar_Get("com_affinityMask", "", CVAR_ARCHIVE_ND);
	Cvar_SetDescription( com_affinityMask, "Bind game process to bitmask-specified CPU core(s), special characters:\n A or a - all default cores\n P or p - performance cores\n E or e - efficiency cores\n 0x<value> - use hexadecimal notation\n + or - can be used to add or exclude particular cores" );
	com_affinityMask->modified = qfalse;
#endif

	sys_cpuString = Cvar_Get("sys_cpuString", "detect", CVAR_PROTECTED | CVAR_ROM | CVAR_NORESTART);
	if (!N_stricmp(Cvar_VariableString("sys_cpuString"), "detect")) {
		char vendor[128];
		Con_Printf("...detecting CPU, found ");
		Sys_GetProcessorId(vendor);
		Cvar_Set("sys_cpuString", vendor);
	}
	Con_Printf("%s\n", Cvar_VariableString("sys_cpuString"));

#ifdef USE_AFFINITY_MASK
	// get initial process affinity - we will respect it when setting custom affinity masks
	eCoreMask = pCoreMask = affinityMask = Sys_GetAffinityMask();
#if (GDRx64 || GDRi386)
	DetectCPUCoresConfig();
#endif
	if (com_affinityMask->s[0] != '\0') {
		Com_SetAffinityMask(com_affinityMask->s);
		com_affinityMask->modified = qfalse;
	}
#endif

	Cmd_AddCommand("shutdown", Com_Shutdown_f);
//	Cmd_AddCommand("game_restart", Com_GameRestart_f);
	Cmd_AddCommand("quit", Com_Quit_f);
	Cmd_AddCommand( "exit", Com_Quit_f ); // really just added for convenience...
	Cmd_AddCommand( "writecfg", Com_WriteConfig_f );

	VM_Init();

	G_Init();
	G_StartHunkUsers();

	// set com_frameTime so that if a map is started on the
	// command line it will still be able to count on com_frameTime
	// being random enough for a serverid
	lastTime = com_frameTime = Com_Milliseconds();

	com_fullyInitialized = qtrue;

	Con_Printf("==== Common Initialization Done ====\n");
}

//==================================================================

static void Com_WriteConfigToFile( const char *filename ) {
	fileHandle_t f;

	f = FS_FOpenWrite( filename );
	if ( f == FS_INVALID_HANDLE ) {
		if ( ( f = FS_FOpenWrite( filename ) ) == FS_INVALID_HANDLE ) {
			Con_Printf( "Couldn't write %s.\n", filename );
			return;
		}
	}

	FS_Printf( f, "// generated by glnomad, modify at your own risk" GDR_NEWLINE );
	Key_WriteBindings( f );
	Cvar_WriteVariables( f );
	FS_FClose( f );
}


/*
===============
Com_WriteConfiguration

Writes key bindings and archived cvars to config file if modified
===============
*/
void Com_WriteConfiguration( void ) {
	const char *basegame;
	const char *gamedir;

	// if we are quitting without fully initializing, make sure
	// we don't write out anything
	if ( !com_fullyInitialized ) {
		return;
	}

	if ( !(cvar_modifiedFlags & CVAR_SAVE ) ) {
		return;
	}
	cvar_modifiedFlags &= ~CVAR_SAVE;

	Com_WriteConfigToFile( NOMAD_CONFIG );

	gamedir = Cvar_VariableString( "fs_game" );
	basegame = Cvar_VariableString( "fs_basegame" );
//	if ( UI_usesUniqueCDKey() && gamedir[0] && Q_stricmp( basegame, gamedir ) ) {
//		Com_WriteCDKey( gamedir, &cl_cdkey[16] );
//	} else {
//		Com_WriteCDKey( basegame, cl_cdkey );
//	}
}


/*
===============
Com_WriteConfig_f

Write the config file to a specific name
===============
*/
static void Com_WriteConfig_f( void ) {
	char	filename[MAX_GDR_PATH];
	const char *ext;

	if ( Cmd_Argc() != 2 ) {
		Con_Printf( "Usage: writecfg <filename>\n" );
		return;
	}

	N_strncpyz( filename, Cmd_Argv(1), sizeof( filename ) );
	COM_DefaultExtension( filename, sizeof( filename ), ".cfg" );

	if ( !FS_AllowedExtension( filename, qfalse, &ext ) ) {
		Con_Printf( "%s: Invalid filename extension: '%s'.\n", __func__, ext );
		return;
	}

	Con_Printf( "Writing %s.\n", filename );
	Com_WriteConfigToFile( filename );
}

static int32_t Com_ModifyMsec( int32_t msec )
{
	int32_t clampTime;

	// for local single player gaming
	// we may want to clamp the time to prevent players from
	// flying off edges when something hitches.
	clampTime = 200;

	if (msec > clampTime) {
		msec = clampTime;
	}

	return msec;
}

static int Com_TimeVal( int minMsec )
{
	int timeVal;

	timeVal = Com_Milliseconds() - com_frameTime;

	if (timeVal >= minMsec) {
		timeVal = 0;
	}
	else {
		timeVal = minMsec - timeVal;
	}

	return timeVal;
}

static double Com_ClockToMilliseconds( clock_t ticks ) {
	return (ticks / (double)CLOCKS_PER_SEC) * 1000.0f;
}

static int32_t com_frameMsec;

/*
Com_Frame: runs a single frame for the game
*/
void Com_Frame( qboolean noDelay )
{
	static int32_t bias = 0;
	static int32_t lastTime;
	int32_t msec, minMsec;
	int32_t key;

	eastl::chrono::system_clock::time_point startCap, endCap;
	eastl::chrono::duration<double, eastl::milli> workTime, deltaCap;
	clock_t start, end, delta;
	double frameRate = 0, avg = 0;

	if (Q_setjmp(abortframe)) {
		return; // an ERR_DROP was thrown
	}

	// silence compiler warning
	minMsec = 0;

#ifdef USE_AFFINITY_MASK
	if (com_affinityMask->modified) {
		Com_SetAffinityMask( com_affinityMask->s );
		com_affinityMask->modified = qfalse;
	}
#endif

	// we may want to spin here if things are going too fast
	if ( com_maxfps->i > 0 ) {
		minMsec = 1000 / com_maxfps->i;
	} else {
		minMsec = 1;
	}
	do {
		com_frameTime = Com_EventLoop();
		if ( lastTime > com_frameTime ) {
			lastTime = com_frameTime;		// possible on first frame
		}
		msec = com_frameTime - lastTime;
	} while ( msec < minMsec );
	Cbuf_Execute ();

	// mess with msec if needed
	com_frameMsec = msec;
	msec = Com_ModifyMsec( msec );

	//
	// run the game loop
	//
	Com_EventLoop();
	Cbuf_Execute();

	G_Frame( msec, 0 );

	// run framerate diagnostics

	com_frameNumber++;
	/*
	end = clock();
	delta = end - start;

	if (delta > 0) {
		com_fps = CLOCKS_PER_SEC / delta;
	}
	*/
}

/*
================
Sys_SnapVector
================
*/
#if defined(__GNUC__) || defined(_MSC_VER ) || defined(__MINGW64__) || defined(__MINGW32__)
#include <xmmintrin.h>
#if GDRx64
void Sys_SnapVector( float *vector )
{
	__m128 vf0, vf1, vf2;
	__m128i vi;
#ifdef _WIN32
	DWORD mxcsr;
#else
	unsigned int mxcsr;
#endif

	mxcsr = _mm_getcsr();
	vf0 = _mm_setr_ps( vector[0], vector[1], vector[2], 0.0f );

	_mm_setcsr( mxcsr & ~0x6000 ); // enforce rounding mode to "round to nearest"

	vi = _mm_cvtps_epi32( vf0 );
	vf0 = _mm_cvtepi32_ps( vi );

	vf1 = _mm_shuffle_ps(vf0, vf0, _MM_SHUFFLE(1,1,1,1));
	vf2 = _mm_shuffle_ps(vf0, vf0, _MM_SHUFFLE(2,2,2,2));

	_mm_setcsr( mxcsr ); // restore rounding mode

	_mm_store_ss( &vector[0], vf0 );
	_mm_store_ss( &vector[1], vf1 );
	_mm_store_ss( &vector[2], vf2 );
}
#endif // GDRx64

#if GDRi386
void Sys_SnapVector( float *vector )
{
	static const DWORD cw037F = 0x037F;
#ifdef _WIN32
	DWORD cwCurr;
#else
	unsigned int cwCurr;
#endif
__asm {
	fnstcw word ptr [cwCurr]
	mov ecx, vector
	fldcw word ptr [cw037F]

	fld   dword ptr[ecx+8]
	fistp dword ptr[ecx+8]
	fild  dword ptr[ecx+8]
	fstp  dword ptr[ecx+8]

	fld   dword ptr[ecx+4]
	fistp dword ptr[ecx+4]
	fild  dword ptr[ecx+4]
	fstp  dword ptr[ecx+4]

	fld   dword ptr[ecx+0]
	fistp dword ptr[ecx+0]
	fild  dword ptr[ecx+0]
	fstp  dword ptr[ecx+0]

	fldcw word ptr cwCurr
	}; // __asm
}

#endif // GDRi386

#if arm64
void Sys_SnapVector( float *vector )
{
	vector[0] = rint( vector[0] );
	vector[1] = rint( vector[1] );
	vector[2] = rint( vector[2] );
}
#endif

#endif // __GNUC__ || _MSC_VER

#if 0 // clang

#if GDRi386

#define QROUNDX87(src) \
	"flds " src "\n" \
	"fistpl " src "\n" \
	"fildl " src "\n" \
	"fstps " src "\n"

void Sys_SnapVector( float *vector )
{
	static const unsigned short cw037F = 0x037F;
	unsigned short cwCurr;

	__asm__ volatile
	(
		"fnstcw %1\n" \
		"fldcw %2\n" \
		QROUNDX87("0(%0)")
		QROUNDX87("4(%0)")
		QROUNDX87("8(%0)")
		"fldcw %1\n" \
		:
		: "r" (vector), "m"(cwCurr), "m"(cw037F)
		: "memory", "st"
	);
}

#else // non-x86

void Sys_SnapVector( float *vector )
{
	vector[0] = rint( vector[0] );
	vector[1] = rint( vector[1] );
	vector[2] = rint( vector[2] );
}

#endif

#endif // clang


static qboolean strgtr(const char *s0, const char *s1)
{
	uint64_t l0, l1, i;

	l0 = strlen( s0 );
	l1 = strlen( s1 );

	if ( l1 < l0 ) {
		l0 = l1;
	}

	for( i = 0; i < l0; i++ ) {
		if ( s1[i] > s0[i] ) {
			return qtrue;
		}
		if ( s1[i] < s0[i] ) {
			return qfalse;
		}
	}
	return qfalse;
}


static void Com_SortList( char **list, uint64_t n )
{
	const char *m;
	char *temp;
	uint64_t i, j;
	i = 0;
	j = n;
	m = list[ n >> 1 ];
	do {
		while ( strcmp( list[i], m ) < 0 ) i++;
		while ( strcmp( list[j], m ) > 0 ) j--;
		if ( i <= j ) {
			temp = list[i];
			list[i] = list[j];
			list[j] = temp;
			i++;
			j--;
		}
	} while ( i <= j );
	if ( j > 0 ) Com_SortList( list, j );
	if ( n > i ) Com_SortList( list+i, n-i );
}


void Com_SortFileList( char **list, uint64_t nfiles, uint64_t fastSort )
{
	if ( nfiles > 1 && fastSort ) {
		Com_SortList( list, nfiles-1 );
	}
	else { // defrag mod demo UI can't handle _properly_ sorted directories
		uint64_t i, flag;
		do {
			flag = 0;
			for( i = 1; i < nfiles; i++ ) {
				if ( strgtr( list[i-1], list[i] ) ) {
					char *temp = list[i];
					list[i] = list[i-1];
					list[i-1] = temp;
					flag = 1;
				}
			}
		} while( flag );
	}
}
