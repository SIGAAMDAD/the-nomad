#include "n_shared.h"
#include "n_scf.h"
#include "../game/g_game.h"
#include "../rendergl/rgl_public.h"
#include "n_sound.h"
#include "n_map.h"
#include "../common/vm.h"
#include "../rendercommon/imgui_impl_sdl2.h"

namespace EA::StdC {
	int Vsnprintf(char* EA_RESTRICT pDestination, size_t n, const char* EA_RESTRICT pFormat, va_list arguments)
	{ return vsnprintf(pDestination, n, pFormat, arguments); }
};

#define MAX_EVENT_QUEUE 256
#define MASK_QUEUED_EVENTS (MAX_EVENT_QUEUE - 1)
#define MAX_PUSHED_EVENTS 256

static sysEvent_t eventQueue[MAX_EVENT_QUEUE];
static sysEvent_t *lastEvent = eventQueue + MAX_EVENT_QUEUE - 1;
static uint32_t eventHead = 0;
static uint32_t eventTail = 0;
static uint32_t com_pushedEventsHead;
static uint32_t com_pushedEventsTail;
static sysEvent_t com_pushedEvents[MAX_PUSHED_EVENTS];

int CPU_flags;
cvar_t *com_demo;
cvar_t *com_journal;
cvar_t *com_logfile;
cvar_t *com_errorCode;
cvar_t *sys_cpuString;
cvar_t *com_devmode;
cvar_t *com_version;
file_t com_journalFile = FS_INVALID_HANDLE;
uint64_t com_frameTime;
uint64_t com_cacheLine; // L1 cacheline
char com_errorMessage[MAXPRINTMSG];
static jmp_buf abortframe;
qboolean com_errorEntered;

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
    Con_AddText(msg);

    // echo to the actual console
    Sys_Print(msg);

    // slap that shit into the logfile
    if (com_logfile && com_logfile->i) {
		extern file_t logfile;
        if (logfile == FS_INVALID_HANDLE && FS_Initialized() && !opening_console) {
            const char *logName = "debug.log";
            int mode;
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
            FS_Write(msg, length, logfile);
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

	Cvar_Set("com_errorCode", va("%i", code));

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

		FS_Restart();
		com_errorEntered = qfalse;

		Q_longjmp( abortframe, 1 );
	} else { // ERR_FATAL
		VM_Forced_Unload_Start();
		G_Shutdown(qtrue);
		Com_EndRedirect();
		VM_Forced_Unload_Done();
	}

	Com_Shutdown();

	calledSystemError = qtrue;
	Sys_Error("%s", com_errorMessage);
}

char *Sys_GetClipboardData( void )
{
	char *data = NULL;
	char *cliptext;

	if ( ( cliptext = SDL_GetClipboardText() ) != NULL ) {
		if ( cliptext[0] != '\0' ) {
			size_t bufsize = strlen( cliptext ) + 1;

			data = (char *)Z_Malloc( bufsize, TAG_STATIC );
			N_strncpyz( data, cliptext, bufsize );

			// find first listed char and set to '\0'
			strtok( data, "\n\r\b" );
		}
		SDL_free( cliptext );
	}
	return data;
}


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

static void Com_PumpKeyEvents(void)
{
	SDL_Event event;

	SDL_PumpEvents();

	while (SDL_PollEvent(&event)) {
		if (Key_GetCatcher() & KEYCATCH_CONSOLE)
			ImGui_ImplSDL2_ProcessEvent(&event);

		switch (event.type) {
		case SDL_KEYDOWN:
			Com_QueueEvent(com_frameTime, SE_KEY, event.key.keysym.scancode, qtrue, 0, NULL);
			break;
		case SDL_KEYUP:
			Com_QueueEvent(com_frameTime, SE_KEY, event.key.keysym.scancode, qfalse, 0, NULL);
			break;
		case SDL_QUIT:
			Com_QueueEvent(com_frameTime, SE_WINDOW, event.type, 0, 0, NULL);
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
			Com_KeyEvent(KEY_CONSOLE, 1, ev.evTime);
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

//
// Com_Crash_f: force crash, only for devs
//
static void Com_Crash_f(void)
{
    *((volatile int *)0) = 0x1234;
}

//
// Com_Shutdown_f: for testing exit/crashing processes
//
static void Com_Shutdown_f(void)
{
    N_Error(ERR_FATAL, "testing");
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
Com_InitJournals: initializes the logfile and the event journal
*/
void Com_InitJournals(void)
{
	if (!com_journal->i) { // no journaling wanted
		return;
	}

	if (com_journal->i == JOURNAL_WRITE) {
		Con_Printf("writing an event journal\n");
		com_journalFile = FS_FOpenWrite("journal.dat");
		if (com_journalFile == FS_INVALID_HANDLE) {
			Con_Printf(COLOR_YELLOW "Failed to open event journal\n");
		}
	}
	else if (com_journal->i == JOURNAL_PLAYBACK) {
		Con_Printf("replaying event journal\n");
		FS_FOpenFileRead("journal.dat", &com_journalFile);
	}
}

void Com_RestartGame(void)
{
	static qboolean gameRestarting = qfalse;

	// make sure no recursion is possible
	if (!gameRestarting) {
		gameRestarting = qtrue;

		Con_Printf("Com_RestartGame: restarting all engine systems\n");

		G_ClearMem();

		// reset console history
		Con_ResetHistory();

		// clear the filesystem before restarting the cvars
		FS_Shutdown(qtrue);

		// reset all cvars
		Cvar_Restart(qtrue);

		FS_Restart();

		// reload the config file
		Com_LoadConfig();

		G_Restart();

		gameRestarting = qtrue;
	}
}

/*
Com_Shutdown: shuts down all the engine's systems
*/
void Com_Shutdown(void)
{
	Con_Printf("Com_Shutdown: shutting down engine\n");

	Con_Shutdown();
	FS_Shutdown(qtrue);
	G_Shutdown(qtrue);
	Memory_Shutdown();
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

static void Sys_GetProccesorInfo(void)
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
//	if ( regs[3] & ( 1 << 15 ) )
//		CPU_flags |= CPU_FCOM;

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
				if (print_flags & CPU_FCOM)
					strcat(vendor, " CMOV");
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
	Com_sprintf( vendor, 100, "%s", ARCH_STRING );
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
	int		flags = 0;
	int		i;

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

/*
Com_Init: initializes all the engine's systems
*/
void Com_Init(char *commandLine)
{
	uint64_t lastTime;

	Con_Init();
	Con_Printf(
		"================================================\n"
		"\"The Nomad\" is free software distributed under\n"
		"the terms of the GNU General Public License v2.0\n"
		"================================================\n"
		COLOR_BLUE "version: %s\n"
		COLOR_BLUE "platform: " OS_STRING ", " ARCH_STRING "\n"
		COLOR_BLUE "date compiled: " __DATE__ "\n",
	Com_VersionString());

	if (Q_setjmp(abortframe)) {
		Sys_Error("Error during initialization");
	}
	Con_Printf("Com_Init: initializing systems\n");

	// get the cacheline for optimized allocations and memory management
	com_cacheLine = SDL_GetCPUCacheLineSize();

	Z_Init();
	Cvar_Init();
	Cbuf_Init();
	Cmd_Init();

	Com_ParseCommandLine(commandLine);

	// override anything from the config files with command line args
	Com_StartupVariable( NULL );

	Com_StartupVariable("com_devmode");
#ifdef _NOMAD_DEBUG
	com_devmode = Cvar_Get("com_devmode", "1", CVAR_INIT | CVAR_PROTECTED);
#else
	com_devmode = Cvar_Get("com_devmode", "0", CVAR_INIT | CVAR_PROTECTED);
#endif
	Cvar_CheckRange(com_devmode, "0", "1", CVT_INT);
	Cvar_SetDescription(com_devmode, "Enables a bunch of extra diagnostics for developers.");

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
	
	com_version = Cvar_Get("com_version", Com_VersionString(), CVAR_PROTECTED | CVAR_ROM);
	Cvar_SetDescription(com_version, "Read-only CVar to see the version of the game.");

	com_frameTime = 0;

	Com_InitKeyCommands();

	FS_InitFilesystem();

	Com_StartupVariable("com_logfile");
	com_logfile = Cvar_Get("com_logfile", "1", CVAR_TEMP);
	Cvar_CheckRange(com_logfile, "0", "1", CVT_INT);
	Cvar_SetDescription(com_logfile, "System console logging");

	Com_InitJournals();
	Com_InitEvents();

	Hunk_InitMemory();

	Cmd_AddCommand("shutdown", Com_Shutdown_f);
	Cmd_AddCommand("restart", Com_GameRestart_f);

	if (com_devmode->i) {
		Cmd_AddCommand("freeze", Com_Freeze_f);
		Cmd_AddCommand("error", Com_Error_f);
		Cmd_AddCommand("crash", Com_Crash_f);
	}

	sys_cpuString = Cvar_Get("sys_cpuString", "detect", CVAR_PROTECTED | CVAR_ROM | CVAR_NORESTART);
	if (!N_stricmp(Cvar_VariableString("sys_cpuString"), "detect")) {
		char vendor[128];
		Con_Printf("...detecting CPU, found ");
		Sys_GetProcessorId(vendor);
		Cvar_Set("sys_cpuString", vendor);
	}
	Con_Printf("%s\n", Cvar_VariableString("sys_cpuString"));

	G_Init();

	// set com_frameTime so that if a map is started on the
	// command line it will still be able to count on com_frameTime
	// being random enough for a serverid
	lastTime = com_frameTime = Com_Milliseconds();

	Con_Printf("==== Common Initialization Done ====\n");
}

/*
Com_Frame: runs a single frame for the game
*/
void Com_Frame(void)
{
	uint64_t msec, realMsec, lastTime;

	if (Q_setjmp(abortframe)) {
		return; // an ERR_DROP was thrown
	}

	lastTime = com_frameTime;
	com_frameTime = Com_EventLoop();
	realMsec = com_frameTime - lastTime;

	Cbuf_Execute();

	// run it again
	Com_EventLoop();

	G_Frame(msec, realMsec);
}

/*
================
Sys_SnapVector
================
*/
#if (__GNUC__ || _MSC_VER)
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
