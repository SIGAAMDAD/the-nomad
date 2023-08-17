#include "n_shared.h"
#include "n_scf.h"
#include "../src/g_game.h"
#include "../rendergl/rgl_public.h"
#include "n_sound.h"
#include "n_map.h"

static cvar_t *com_demo;

namespace EA::StdC {
	int Vsnprintf(char* EA_RESTRICT pDestination, size_t n, const char* EA_RESTRICT pFormat, va_list arguments)
	{ return vsnprintf(pDestination, n, pFormat, arguments); }
};

#define MAX_EVENT_QUEUE 256
#define MASK_QUEUED_EVENTS (MAX_EVENT_QUEUE - 1)
#define MAX_PUSHED_EVENTS 256

static char *com_buffer;
static int32_t com_bufferLen;

static sysEvent_t eventQueue[MAX_EVENT_QUEUE];
static sysEvent_t *lastEvent = eventQueue + MAX_EVENT_QUEUE - 1;
static uint32_t eventHead = 0;
static uint32_t eventTail = 0;
static uint32_t com_pushedEventsHead;
static uint32_t com_pushedEventsTail;
static sysEvent_t com_pushedEvents[MAX_PUSHED_EVENTS];

#define JOURNAL_NONE	 0		// no logging
#define JOURNAL_LOGGING  1		// only write to a logfile
#define JOURNAL_ALL		 2		// write a logfile and an event journal
#define JOURNAL_PLAYBACK 3		// replay the event journal

cvar_t *com_journal;
cvar_t *com_frameTime;
file_t com_journalFile;

/*
===============================================================

LOGGING

===============================================================
*/

/*
===============================================================

EVENT LOOP

===============================================================
*/


static void Com_InitEvents(void)
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
			Con_Printf(WARNING, "Com_PushEvent: overflow");
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

static void Com_PumpKeyEvents(void)
{
	SDL_Event event;

	SDL_PumpEvents();

	while (SDL_PollEvent(&event)) {
		if (RE_ConsoleIsOpen())
			RE_ProcessConsoleEvents(&event);

		switch (event.type) {
		case SDL_KEYDOWN:
			Com_QueueEvent(com_frameTime->i, SE_KEY, event.key.keysym.scancode, qtrue, 0, NULL);
			break;
		case SDL_KEYUP:
			Com_QueueEvent(com_frameTime->i, SE_KEY, event.key.keysym.scancode, qfalse, 0, NULL);
			break;
		case SDL_QUIT:
			Com_QueueEvent(com_frameTime->i, SE_WINDOW, event.type, 0, 0, NULL);
			break;
		case SDL_MOUSEMOTION:
		case SDL_MOUSEBUTTONUP:
		case SDL_MOUSEBUTTONDOWN:
		case SDL_MOUSEWHEEL:
			break;
		};
	}
}

static sysEvent_t Com_GetSystemEvent(void)
{
	sysEvent_t ev;
	const char *s;
	uint32_t evTime;

	// return if we have data
	if (eventHead - eventTail > 0)
		return eventQueue[(eventTail++) & MASK_QUEUED_EVENTS];
	
	Com_PumpKeyEvents();

	evTime = Sys_Milliseconds();

	// check for console commands

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

		if (com_journal->i == JOURNAL_ALL) {
			Com_PumpKeyEvents();
			r = FS_Read(&ev, ev.evPtrLength, com_journalFile);
			if (r != sizeof(ev)) {
				N_Error("Error reading from journal file");
			}
			if (ev.evPtrLength) {
				ev.evPtr = Z_Malloc(ev.evPtrLength, TAG_STATIC, &ev.evPtr, "eventPtr");
				r = FS_Read(ev.evPtr, ev.evPtrLength, com_journalFile);
				if (r != ev.evPtrLength) {
					N_Error("Error reading from journal file");
				}
			}
		}
		else {
			ev = Com_GetSystemEvent();

			// write the journal value out if needed
			if (com_journal->i == JOURNAL_ALL) {
				r = FS_Write(&ev, sizeof(ev), com_journalFile);
				if (r != sizeof(ev)) {
					N_Error("Error writing to journal file");
				}
				if (ev.evPtrLength) {
					r = FS_Write(ev.evPtr, ev.evPtrLength, com_journalFile);
					if (r != ev.evPtrLength) {
						N_Error("Error writing to journal file");
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
		Con_Printf("%s(type=%s,keys=(%i,%i),time=%i): overflow", __func__, Com_EventName(evType), evValue, evValue2, evTime);
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

static void Com_WindowEvent(uint32_t value)
{
	if (value == SDL_QUIT) {
		Sys_Exit(1);
	}
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

		switch (ev.evType) {
		case SE_KEY:
			Com_KeyEvent(ev.evValue, (qboolean)ev.evValue2, ev.evTime);
			break;
		case SE_WINDOW:
			Com_WindowEvent(ev.evValue);
			break;
//		case SE_MOUSE:
//			Com_MouseEvent(ev.evValue, ev.evValue2);
//			break;
		case SE_CONSOLE:
			RE_ToggleConsole();
			break;
		default:
			N_Error("Com_EventLoop: bad event type %i", ev.evType);
		};

		// free any block data
		if (ev.evPtr) {
			Z_Free(ev.evPtr);
			ev.evPtr = NULL;
		}
	}

	return 0; // never reached
}

static qboolean com_errorEntered = qfalse;

void GDR_NORETURN GDR_DECL N_Error(const char *err, ...)
{
    char msg[1024];
    memset(msg, 0, sizeof(msg));
    va_list argptr;
    va_start(argptr, err);
    stbsp_vsnprintf(msg, sizeof(msg) - 1, err, argptr);
    va_end(argptr);

	fprintf(stderr, C_RED "ERROR: " C_RESET "%s\n", msg);
	Sys_Exit(-1);
}

/*
Com_Printf: can be used by either the main engine, or the vm.
A raw string should NEVER be passed as fmt, same reason as the quake3 engine.
*/
void GDR_DECL Com_Printf(const char *fmt, ...)
{
    int length;
    va_list argptr;
    char msg[MAX_MSG_SIZE];

    va_start(argptr, fmt);
	length = N_vsnprintf(msg, sizeof(msg), fmt, argptr);
    va_end(argptr);

	Con_Printf("%s", msg);
}


static jmp_buf abort_vmframe;

/*
Com_Error: the vm's version of N_Error
*/
void GDR_DECL Com_Error(vm_t *vm, int level, const char *fmt,  ...)
{
    int length;
    va_list argptr;
    char msg[MAX_MSG_SIZE];

    va_start(argptr, fmt);
    length = N_vsnprintf(msg, sizeof(msg), fmt, argptr);
    va_end(argptr);

	if (level == ERR_FATAL) {
		VM_Restart(vm);
		Con_Error(false, "(VM Fatal Error): %s", msg);
	}
	else if (level == ERR_FRAME || level == ERR_RESTART) {
		if (level == ERR_FRAME) {
			longjmp(abort_vmframe, 0);
		}
	}
}

/*
Com_Crash_f: force crash, only for devs
*/
static void Com_Crash_f(void)
{
    *((int *)0) = 0x1234;
}

/*
Com_Shutdown_f: for testing exit/crashing processes
*/
static void Com_Shutdown_f(void)
{
    N_Error("testing");
}

static const nmap_t *G_GetCurrentMap(void)
{
	return level->mapData;
}

int I_GetParm(const char *parm)
{
	int i;
    if (!parm)
        N_Error("I_GetParm: parm is NULL");

    for (i = 1; i < myargc; i++) {
        if (!N_stricmp(myargv[i], parm))
            return i;
    }
    return -1;
}

#if defined(__OS2__) || defined(_WIN32)
SDL_Thread *PFN_SDL_CreateThread(SDL_ThreadFunction fn, const char *name, void *data)
{
	return SDL_CreateThread(fn, name, data);
}
SDL_Thread *PFN_SDL_CreateThreadWithStackSize(SDL_ThreadFunction fn, const char *name, const size_t stacksize, void *data)
{
	return SDL_CreateThreadWithStackSize(fn, name, stacksize, data);
}
#endif


/*
Com_FillImport: fills render import functions for the dynamic library to use
*/
void Com_FillImport(renderImport_t *import)
{
#ifdef _NOMAD_DEBUG
    import->Hunk_AllocDebug = Hunk_AllocDebug;
#else
    import->Hunk_Alloc = Hunk_Alloc;
#endif
    import->Hunk_MemoryRemaining = Hunk_MemoryRemaining;
	import->Hunk_AllocateTempMemory = Hunk_AllocateTempMemory;
	import->Hunk_FreeTempMemory = Hunk_FreeTempMemory;

#ifdef _NOMAD_DEBUG
    import->Z_MallocDebug = Z_MallocDebug;
    import->Z_CallocDebug = Z_CallocDebug;
    import->Z_ReallocDebug = Z_ReallocDebug;
	import->Z_StrdupDebug = Z_StrdupDebug;
	import->Z_FreeDebug = Z_FreeDebug;
#else
    import->Z_Malloc = Z_Malloc;
    import->Z_Calloc = Z_Calloc;
    import->Z_Realloc = Z_Realloc;
	import->Z_Strdup = Z_Strdup;
	import->Z_Free = Z_Free;
#endif

	import->Z_FreeTags = Z_FreeTags;
	import->Z_ChangeTag = Z_ChangeTag;
	import->Z_ChangeUser = Z_ChangeUser;
	import->Z_CleanCache = Z_CleanCache;
	import->Z_CheckHeap = Z_CheckHeap;
	import->Z_ClearZone = Z_ClearZone;
    import->Z_FreeMemory = Z_FreeMemory;
    import->Z_NumBlocks = Z_NumBlocks;
	import->Z_BlockSize = Z_BlockSize;

	import->Sys_FreeFileList = Sys_FreeFileList;
	
    import->Mem_Alloc = Mem_Alloc;
    import->Mem_Free = Mem_Free;

	// get the specific logger function (it's been overloaded)
	import->Con_Printf = static_cast<void(*)(loglevel_t, const char *, ...)>(Con_Printf);
    import->N_Error = N_Error;

	import->Cvar_VariableStringBuffer = Cvar_VariableStringBuffer;
	import->Cvar_VariableStringBufferSafe = Cvar_VariableStringBufferSafe;
	import->Cvar_VariableInteger = Cvar_VariableInteger;
	import->Cvar_VariableFloat = Cvar_VariableFloat;
	import->Cvar_VariableBoolean = Cvar_VariableBoolean;
	import->Cvar_VariableString = Cvar_VariableString;
	import->Cvar_Flags = Cvar_Flags;
	import->Cvar_Get = Cvar_Get;
	import->Cvar_SetGroup = Cvar_SetGroup;
	import->Cvar_SetDescription = Cvar_SetDescription;
	import->Cvar_SetSafe = Cvar_SetSafe;
	import->Cvar_SetBooleanValue = Cvar_SetBooleanValue;
	import->Cvar_SetStringValue = Cvar_SetStringValue;
	import->Cvar_SetFloatValue = Cvar_SetFloatValue;
	import->Cvar_SetIntegerValue = Cvar_SetIntegerValue;
	import->Cvar_CheckRange = Cvar_CheckRange;

    import->Cmd_AddCommand = Cmd_AddCommand;
    import->Cmd_RemoveCommand = Cmd_RemoveCommand;
    import->Cmd_ExecuteString = Cmd_ExecuteString;
    import->Cmd_Argc = Cmd_Argc;
    import->Cmd_ArgsFrom = Cmd_ArgsFrom;
    import->Cmd_Argv = Cmd_Argv;
    import->Cmd_Clear = Cmd_Clear;

    import->FS_Write = FS_Write;
    import->FS_Read = FS_Read;
    import->FS_FileSeek = FS_FileSeek;
    import->FS_FileTell = FS_FileTell;
    import->FS_FileLength = FS_FileLength;
    import->FS_FileExists = FS_FileExists;
    import->FS_FOpenRead = FS_FOpenRead;
    import->FS_FOpenWrite = FS_FOpenWrite;
    import->FS_FClose = FS_FClose;
    import->FS_FreeFile = FS_FreeFile;
    import->FS_LoadFile = FS_LoadFile;
	import->FS_ListFiles = FS_ListFiles;

	import->Key_IsDown = Key_IsDown;

	import->G_GetCurrentMap = G_GetCurrentMap;
	import->Map_GetSpriteCoords = Map_GetSpriteCoords;

	// most of this stuff is for imgui's usage
	import->SDL_SetCursor = SDL_SetCursor;
    import->SDL_SetHint = SDL_SetHint;
    import->SDL_FreeCursor = SDL_FreeCursor;
    import->SDL_CaptureMouse = SDL_CaptureMouse;
    import->SDL_GetKeyboardFocus = SDL_GetKeyboardFocus;
    import->SDL_GetWindowFlags = SDL_GetWindowFlags;
    import->SDL_WarpMouseInWindow = SDL_WarpMouseInWindow;
    import->SDL_GetGlobalMouseState = SDL_GetGlobalMouseState;
    import->SDL_GetWindowPosition = SDL_GetWindowPosition;
	import->SDL_GetWindowSize = SDL_GetWindowSize;
    import->SDL_ShowCursor = SDL_ShowCursor;
    import->SDL_GetRendererOutputSize = SDL_GetRendererOutputSize;
    import->SDL_GameControllerGetButton = SDL_GameControllerGetButton;
    import->SDL_GameControllerGetAxis = SDL_GameControllerGetAxis;
    import->SDL_GameControllerOpen = SDL_GameControllerOpen;
    import->SDL_GetClipboardText = SDL_GetClipboardText;
    import->SDL_SetClipboardText = SDL_SetClipboardText;
    import->SDL_SetTextInputRect = SDL_SetTextInputRect;
    import->SDL_GetCurrentVideoDriver = SDL_GetCurrentVideoDriver;
    import->SDL_CreateSystemCursor = SDL_CreateSystemCursor;
    import->SDL_GetWindowWMInfo = SDL_GetWindowWMInfo;
    import->SDL_Init = SDL_Init;
    import->SDL_Quit = SDL_Quit;
    import->SDL_GetTicks64 = SDL_GetTicks64;
    import->SDL_GetPerformanceCounter = SDL_GetPerformanceCounter;
    import->SDL_GetPerformanceFrequency = SDL_GetPerformanceFrequency;
    import->SDL_GL_GetDrawableSize = SDL_GL_GetDrawableSize;
    import->SDL_CreateWindow = SDL_CreateWindow;
    import->SDL_DestroyWindow = SDL_DestroyWindow;
    import->SDL_GL_SwapWindow = SDL_GL_SwapWindow;
    import->SDL_GL_CreateContext = SDL_GL_CreateContext;
    import->SDL_GL_GetProcAddress = SDL_GL_GetProcAddress;
    import->SDL_GL_DeleteContext = SDL_GL_DeleteContext;
    import->SDL_GL_SetAttribute = SDL_GL_SetAttribute;
    import->SDL_GL_MakeCurrent = SDL_GL_MakeCurrent;
    import->SDL_GL_SetSwapInterval = SDL_GL_SetSwapInterval;
    import->SDL_GetError = SDL_GetError;
	import->SDL_PollEvent = SDL_PollEvent;

	import->Sys_Exit = Sys_Exit;

#if defined(__OS2__) || defined(_WIN32)
	import->SDL_CreateThread = PFN_SDL_CreateThread;
	import->SDL_CreateThreadWithStackSize = PFN_SDL_CreateThreadWithStackSize;
#else
	import->SDL_CreateThread = SDL_CreateThread;
	import->SDL_CreateThreadWithStackSize = SDL_CreateThreadWithStackSize;
#endif
    import->SDL_WaitThread = SDL_WaitThread;
    import->SDL_SetThreadPriority = SDL_SetThreadPriority;
    import->SDL_DetachThread = SDL_DetachThread;
    import->SDL_GetThreadName = SDL_GetThreadName;
    import->SDL_ThreadID = SDL_ThreadID;
    import->SDL_GetThreadID = SDL_GetThreadID;

	import->SDL_CreateMutex = SDL_CreateMutex;
    import->SDL_DestroyMutex = SDL_DestroyMutex;
    import->SDL_LockMutex = SDL_LockMutex;
    import->SDL_UnlockMutex = SDL_UnlockMutex;
    import->SDL_TryLockMutex = SDL_TryLockMutex;

	import->SDL_CreateCond = SDL_CreateCond;
    import->SDL_DestroyCond = SDL_DestroyCond;
    import->SDL_CondSignal = SDL_CondSignal;
    import->SDL_CondBroadcast = SDL_CondBroadcast;
    import->SDL_CondWait = SDL_CondWait;
    import->SDL_CondWaitTimeout = SDL_CondWaitTimeout;
}

/*
Com_InitJournals: initializes the logfile and the event journal
*/
void Com_InitJournals(void)
{
	com_journal = Cvar_Get("com_journal", va("%i", JOURNAL_NONE), CVAR_PRIVATE | CVAR_SAVE);
	if (!com_journal->i) { // no journaling wanted
		return;
	}

	if (com_journal->i == JOURNAL_LOGGING) {
		Con_Printf("using a logfile");
		logfile = FS_FOpenWrite("logfile.txt");
		if (logfile == FS_INVALID_HANDLE) {
			Con_Printf("Failed to open logfile");
		}
	}
	else if (com_journal->i == JOURNAL_ALL) {
		Con_Printf("using a logfile and an event journal");
		com_journalFile = FS_FOpenWrite("journal.dat");
		if (com_journalFile == FS_INVALID_HANDLE) {
			Con_Printf("Failed to open even journal");
		}

		logfile = FS_FOpenWrite("logfile.txt");
		if (logfile == FS_INVALID_HANDLE) {
			Con_Printf("Failed to open logfile");
		}
	}
	else if (com_journal->i == JOURNAL_PLAYBACK) {
		Con_Printf("replaying event journal");
		FS_FOpenFileRead("journal.dat", &com_journalFile);
	}
}

void Com_Shutdown(void)
{
	if (logfile != FS_INVALID_HANDLE) {
		FS_FClose(logfile);
		logfile = FS_INVALID_HANDLE;
	}
}
/*
Com_Init: initializes all the engine's systems
*/
void Com_Init(void)
{
    Con_Printf("Com_Init: initializing systems");

	Z_Init();
	Cvar_Init();
	Cmd_Init();
	Con_Init();

	Com_InitKeyCommands();
	Hunk_InitMemory();
	FS_InitFilesystem();
	Com_LoadConfig();

	Com_InitEvents();

	com_frameTime = Cvar_Get("com_frameTime", "0", CVAR_ROM | CVAR_PRIVATE | CVAR_SAVE);
	Cvar_SetDescription(com_frameTime, "framerate counter but for tics");

	Com_InitJournals();

	Cmd_AddCommand("crash", Com_Crash_f);
	Cmd_AddCommand("shutdown", Com_Shutdown_f);

	// initialize the rendering engine
	renderImport_t import;
	Com_FillImport(&import);
    RE_Init(&import);

	Com_CacheMaps();

	// initialize OpenAL
	Snd_Init();
	I_CacheAudio();

	// initialize the vm
	VM_Init();
	
	Game::Init();

    Con_Printf(
        "+===========================================================+\n"
         "\"The Nomad\" is free software distributed under the terms\n"
         "of both the GNU General Public License v2.0 and Apache License\n"
         "v2.0\n"
         "+==========================================================+\n"
    );

	Cmd_AddCommand("crash", Com_Crash_f);

	Con_Printf("==== Common Initialization Done ====");

	Hunk_ClearTempMemory();
}

/*
Com_Frame: runs a single frame for the game
*/
void Com_Frame(void)
{
	vm_command = SGAME_RUNTIC;

	// process events
	Com_EventLoop();

	// run the backend threads
//	Snd_Run();

	// setjmp for abort_vmframe will be true if the vm has thrown an error
	if (!setjmp(abort_vmframe))
		VM_Run(SGAME_VM);

#if 0
	RE_BeginFrame();

	RE_DrawBuffers();

	RE_EndFrame(); // submit everything
#endif

	VM_Stop(SGAME_VM);
//	Snd_Stop();

	// 'framerate cap'
	sleepfor(1000 / Cvar_VariableInteger("r_ticrate"));
}

/*
===============================================================

Parsing

===============================================================
*/

static	char	com_token[MAX_TOKEN_CHARS];
static	char	com_parsename[MAX_TOKEN_CHARS];
static	uint64_t com_lines;
static  uint64_t com_tokenline;

// for complex parser
tokenType_t		com_tokentype;

void COM_BeginParseSession( const char *name )
{
	com_lines = 1;
	com_tokenline = 0;
	snprintf(com_parsename, sizeof(com_parsename), "%s", name);
}


uint64_t COM_GetCurrentParseLine( void )
{
	if ( com_tokenline )
	{
		return com_tokenline;
	}

	return com_lines;
}


const char *COM_Parse( const char **data_p )
{
	return COM_ParseExt( data_p, qtrue );
}

void COM_ParseError( const char *format, ... )
{
	va_list argptr;
	static char string[4096];

	va_start( argptr, format );
	N_vsnprintf (string, sizeof(string), format, argptr);
	va_end( argptr );

	Con_Printf( ERROR, ": %s, line %lu: %s", com_parsename, COM_GetCurrentParseLine(), string );
}

void COM_ParseWarning( const char *format, ... )
{
	va_list argptr;
	static char string[4096];

	va_start( argptr, format );
	N_vsnprintf (string, sizeof(string), format, argptr);
	va_end( argptr );

	Con_Printf( WARNING, "%s, line %lu: %s", com_parsename, COM_GetCurrentParseLine(), string );
}


/*
==============
COM_Parse

Parse a token out of a string
Will never return NULL, just empty strings

If "allowLineBreaks" is qtrue then an empty
string will be returned if the next token is
a newline.
==============
*/
const char *SkipWhitespace( const char *data, qboolean *hasNewLines ) {
	int c;

	while( (c = *data) <= ' ') {
		if( !c ) {
			return NULL;
		}
		if( c == '\n' ) {
			com_lines++;
			*hasNewLines = qtrue;
		}
		data++;
	}

	return data;
}

int COM_Compress( char *data_p ) {
	const char *in;
	char *out;
	int c;
	qboolean newline = qfalse, whitespace = qfalse;

	in = out = data_p;
	while ((c = *in) != '\0') {
		// skip double slash comments
		if ( c == '/' && in[1] == '/' ) {
			while (*in && *in != '\n') {
				in++;
			}
		// skip /* */ comments
		} else if ( c == '/' && in[1] == '*' ) {
			while ( *in && ( *in != '*' || in[1] != '/' ) ) 
				in++;
			if ( *in ) 
				in += 2;
			// record when we hit a newline
		} else if ( c == '\n' || c == '\r' ) {
			newline = qtrue;
			in++;
			// record when we hit whitespace
		} else if ( c == ' ' || c == '\t') {
			whitespace = qtrue;
			in++;
			// an actual token
		} else {
			// if we have a pending newline, emit it (and it counts as whitespace)
			if (newline) {
				*out++ = '\n';
				newline = qfalse;
				whitespace = qfalse;
			} else if (whitespace) {
				*out++ = ' ';
				whitespace = qfalse;
			}
			// copy quoted strings unmolested
			if (c == '"') {
				*out++ = c;
				in++;
				while (1) {
					c = *in;
					if (c && c != '"') {
						*out++ = c;
						in++;
					} else {
						break;
					}
				}
				if (c == '"') {
					*out++ = c;
					in++;
				}
			} else {
				*out++ = c;
				in++;
			}
		}
	}

	*out = '\0';

	return out - data_p;
}

const char *COM_ParseExt( const char **data_p, qboolean allowLineBreaks )
{
	int c = 0, len;
	qboolean hasNewLines = qfalse;
	const char *data;

	data = *data_p;
	len = 0;
	com_token[0] = '\0';
	com_tokenline = 0;

	// make sure incoming data is valid
	if ( !data ) {
		*data_p = NULL;
		return com_token;
	}

	while ( 1 ) {
		// skip whitespace
		data = SkipWhitespace( data, &hasNewLines );
		if ( !data ) {
			*data_p = NULL;
			return com_token;
		}
		if ( hasNewLines && !allowLineBreaks ) {
			*data_p = data;
			return com_token;
		}

		c = *data;

		// skip double slash comments
		if ( c == '/' && data[1] == '/' ) {
			data += 2;
			while (*data && *data != '\n') {
				data++;
			}
		}
		// skip /* */ comments
		else if ( c == '/' && data[1] == '*' ) {
			data += 2;
			while ( *data && ( *data != '*' || data[1] != '/' ) ) {
				if ( *data == '\n' ) {
					com_lines++;
				}
				data++;
			}
			if ( *data ) {
				data += 2;
			}
		}
		else {
			break;
		}
	}

	// token starts on this line
	com_tokenline = com_lines;

	// handle quoted strings
	if ( c == '"' )
	{
		data++;
		while ( 1 )
		{
			c = *data;
			if ( c == '"' || c == '\0' )
			{
				if ( c == '"' )
					data++;
				com_token[ len ] = '\0';
				*data_p = data;
				return com_token;
			}
			data++;
			if ( c == '\n' )
			{
				com_lines++;
			}
			if ( len < arraylen( com_token )-1 )
			{
				com_token[ len ] = c;
				len++;
			}
		}
	}

	// parse a regular word
	do
	{
		if ( len < arraylen( com_token )-1 )
		{
			com_token[ len ] = c;
			len++;
		}
		data++;
		c = *data;
	} while ( c > ' ' );

	com_token[ len ] = '\0';

	*data_p = data;
	return com_token;
}
	

/*
==============
COM_ParseComplex
==============
*/
char *COM_ParseComplex( const char **data_p, qboolean allowLineBreaks )
{
	static const byte is_separator[ 256 ] =
	{
	// \0 . . . . . . .\b\t\n . .\r . .
		1,0,0,0,0,0,0,0,0,1,1,0,0,1,0,0,
	//  . . . . . . . . . . . . . . . .
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	//    ! " # $ % & ' ( ) * + , - . /
		1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0, // excl. '-' '.' '/'
	//  0 1 2 3 4 5 6 7 8 9 : ; < = > ?
		0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,
	//  @ A B C D E F G H I J K L M N O
		1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	//  P Q R S T U V W X Y Z [ \ ] ^ _
		0,0,0,0,0,0,0,0,0,0,0,1,0,1,1,0, // excl. '\\' '_'
	//  ` a b c d e f g h i j k l m n o
		1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	//  p q r s t u v w x y z { | } ~ 
		0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1
	};

	int c, len, shift;
	const byte *str;

	str = (byte*)*data_p;
	len = 0; 
	shift = 0; // token line shift relative to com_lines
	com_tokentype = TK_GENEGIC;
	
__reswitch:
	switch ( *str )
	{
	case '\0':
		com_tokentype = TK_EOF;
		break;

	// whitespace
	case ' ':
	case '\t':
		str++;
		while ( (c = *str) == ' ' || c == '\t' )
			str++;
		goto __reswitch;

	// newlines
	case '\n':
	case '\r':
	com_lines++;
		if ( *str == '\r' && str[1] == '\n' )
			str += 2; // CR+LF
		else
			str++;
		if ( !allowLineBreaks ) {
			com_tokentype = TK_NEWLINE;
			break;
		}
		goto __reswitch;

	// comments, single slash
	case '/':
		// until end of line
		if ( str[1] == '/' ) {
			str += 2;
			while ( (c = *str) != '\0' && c != '\n' && c != '\r' )
				str++;
			goto __reswitch;
		}

		// comment
		if ( str[1] == '*' ) {
			str += 2;
			while ( (c = *str) != '\0' && ( c != '*' || str[1] != '/' ) ) {
				if ( c == '\n' || c == '\r' ) {
					com_lines++;
					if ( c == '\r' && str[1] == '\n' ) // CR+LF?
						str++;
				}
				str++;
			}
			if ( c != '\0' && str[1] != '\0' ) {
				str += 2;
			} else {
				// FIXME: unterminated comment?
			}
			goto __reswitch;
		}

		// single slash
		com_token[ len++ ] = *str++;
		break;
	
	// quoted string?
	case '"':
		str++; // skip leading '"'
		//com_tokenline = com_lines;
		while ( (c = *str) != '\0' && c != '"' ) {
			if ( c == '\n' || c == '\r' ) {
				com_lines++; // FIXME: unterminated quoted string?
				shift++;
			}
			if ( len < MAX_TOKEN_CHARS-1 ) // overflow check
				com_token[ len++ ] = c;
			str++;
		}
		if ( c != '\0' ) {
			str++; // skip ending '"'
		} else {
			// FIXME: unterminated quoted string?
		}
		com_tokentype = TK_QUOTED;
		break;

	// single tokens:
	case '+': case '`':
	/*case '*':*/ case '~':
	case '{': case '}':
	case '[': case ']':
	case '?': case ',':
	case ':': case ';':
	case '%': case '^':
		com_token[ len++ ] = *str++;
		break;

	case '*':
		com_token[ len++ ] = *str++;
		com_tokentype = TK_MATCH;
		break;

	case '(':
		com_token[ len++ ] = *str++;
		com_tokentype = TK_SCOPE_OPEN;
		break;

	case ')':
		com_token[ len++ ] = *str++;
		com_tokentype = TK_SCOPE_CLOSE;
		break;

	// !, !=
	case '!':
		com_token[ len++ ] = *str++;
		if ( *str == '=' ) {
			com_token[ len++ ] = *str++;
			com_tokentype = TK_NEQ;
		}
		break;

	// =, ==
	case '=':
		com_token[ len++ ] = *str++;
		if ( *str == '=' ) {
			com_token[ len++ ] = *str++;
			com_tokentype = TK_EQ;
		}
		break;

	// >, >=
	case '>':
		com_token[ len++ ] = *str++;
		if ( *str == '=' ) {
			com_token[ len++ ] = *str++;
			com_tokentype = TK_GTE;
		} else {
			com_tokentype = TK_GT;
		}
		break;

	//  <, <=
	case '<':
		com_token[ len++ ] = *str++;
		if ( *str == '=' ) {
			com_token[ len++ ] = *str++;
			com_tokentype = TK_LTE;
		} else {
			com_tokentype = TK_LT;
		}
		break;

	// |, ||
	case '|':
		com_token[ len++ ] = *str++;
		if ( *str == '|' ) {
			com_token[ len++ ] = *str++;
			com_tokentype = TK_OR;
		}
		break;

	// &, &&
	case '&':
		com_token[ len++ ] = *str++;
		if ( *str == '&' ) {
			com_token[ len++ ] = *str++;
			com_tokentype = TK_AND;
		}
		break;

	// rest of the charset
	default:
		com_token[ len++ ] = *str++;
		while ( !is_separator[ (c = *str) ] ) {
			if ( len < MAX_TOKEN_CHARS-1 )
				com_token[ len++ ] = c;
			str++;
		}
		com_tokentype = TK_STRING;
		break;

	} // switch ( *str )

	com_tokenline = com_lines - shift;
	com_token[ len ] = '\0';
	*data_p = ( char * )str;
	return com_token;
}


/*
==================
COM_MatchToken
==================
*/
void COM_MatchToken( const char **buf_p, const char *match ) {
	const char *token;

	token = COM_Parse( buf_p );
	if ( strcmp( token, match ) ) {
		N_Error( "MatchToken: %s != %s", token, match );
	}
}


/*
=================
SkipBracedSection

The next token should be an open brace or set depth to 1 if already parsed it.
Skips until a matching close brace is found.
Internal brace depths are properly skipped.
=================
*/
qboolean SkipBracedSection( const char **program, int depth ) {
	const char			*token;

	do {
		token = COM_ParseExt( program, qtrue );
		if( token[1] == 0 ) {
			if( token[0] == '{' ) {
				depth++;
			}
			else if( token[0] == '}' ) {
				depth--;
			}
		}
	} while( depth && *program );

	return (qboolean)( depth == 0 );
}


/*
=================
SkipRestOfLine
=================
*/
void SkipRestOfLine( const char **data ) {
	const char *p;
	int		c;

	p = *data;

	if ( !*p )
		return;

	while ( (c = *p) != '\0' ) {
		p++;
		if ( c == '\n' ) {
			com_lines++;
			break;
		}
	}

	*data = p;
}

int ParseHex(const char *text)
{
    int value;
    int c;

    value = 0;
    while ((c = *text++) != 0) {
        if (c >= '0' && c <= '9') {
            value = value * 16 + c - '0';
            continue;
        }
        if (c >= 'a' && c <= 'f') {
            value = value * 16 + 10 + c - 'a';
            continue;
        }
        if (c >= 'A' && c <= 'F') {
            value = value * 16 + 10 + c - 'A';
            continue;
        }
    }

    return value;
}

void Parse1DMatrix( const char **buf_p, int x, float *m ) {
	const char	*token;
	int		i;

	COM_MatchToken( buf_p, "(" );

	for (i = 0 ; i < x; i++) {
		token = COM_Parse( buf_p );
		m[i] = N_atof( token );
	}

	COM_MatchToken( buf_p, ")" );
}

void Parse2DMatrix( const char **buf_p, int y, int x, float *m ) {
	int		i;

	COM_MatchToken( buf_p, "(" );

	for (i = 0 ; i < y ; i++) {
		Parse1DMatrix (buf_p, x, m + i * x);
	}

	COM_MatchToken( buf_p, ")" );
}

void Parse3DMatrix( const char **buf_p, int z, int y, int x, float *m ) {
	int		i;

	COM_MatchToken( buf_p, "(" );

	for (i = 0 ; i < z ; i++) {
		Parse2DMatrix (buf_p, y, x, m + i * x*y);
	}

	COM_MatchToken( buf_p, ")" );
}

int Hex( char c )
{
	if ( c >= '0' && c <= '9' ) {
		return c - '0';
	}
	else
	if ( c >= 'A' && c <= 'F' ) {
		return 10 + c - 'A';
	}
	else
	if ( c >= 'a' && c <= 'f' ) {
		return 10 + c - 'a';
	}

	return -1;
}


/*
===================
Com_HexStrToInt
===================
*/
int32_t Com_HexStrToInt(const char *str)
{
	if (!str)
		return -1;

	// check for hex code
	if (str[ 0 ] == '0' && str[ 1 ] == 'x' && str[ 2 ] != '\0') {
	    int32_t i, digit, n = 0, len = strlen( str );

		for (i = 2; i < len; i++) {
			n *= 16;

			digit = Hex( str[ i ] );

			if ( digit < 0 )
				return -1;

			n += digit;
		}

		return n;
	}

	return -1;
}

qboolean Com_GetHashColor(const char *str, byte *color)
{
	int32_t i, len, hex[6];

	color[0] = color[1] = color[2] = 0;

	if ( *str++ != '#' ) {
		return qfalse;
	}

	len = (int)strlen( str );
	if ( len <= 0 || len > 6 ) {
		return qfalse;
	}

	for ( i = 0; i < len; i++ ) {
		hex[i] = Hex( str[i] );
		if ( hex[i] < 0 ) {
			return qfalse;
		}
	}

	switch ( len ) {
		case 3: // #rgb
			color[0] = hex[0] << 4 | hex[0];
			color[1] = hex[1] << 4 | hex[1];
			color[2] = hex[2] << 4 | hex[2];
			break;
		case 6: // #rrggbb
			color[0] = hex[0] << 4 | hex[1];
			color[1] = hex[2] << 4 | hex[3];
			color[2] = hex[4] << 4 | hex[5];
			break;
		default: // unsupported format
			return qfalse;
	}

	return qtrue;
}

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
