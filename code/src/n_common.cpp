#include "n_shared.h"
#include "n_scf.h"
#include "../rendergl/rgl_public.h"
#include "g_game.h"
#include "g_sound.h"

cvar_t com_demo               = {"com_demo", "", 0.0f, 0, qfalse, TYPE_BOOL, CVG_ENGINE, CVAR_ROM};

cvar_t r_ticrate                  = {"r_ticrate", "", 0.0f, 35, qfalse, TYPE_INT, CVG_RENDERER, CVAR_SAVE};
cvar_t r_screenheight             = {"r_screenheight", "", 0.0f, 720, qfalse, TYPE_INT, CVG_RENDERER, CVAR_SAVE | CVAR_ROM};
cvar_t r_screenwidth              = {"r_screenwidth", "", 0.0f, 1024, qfalse, TYPE_INT, CVG_RENDERER, CVAR_SAVE | CVAR_ROM};
cvar_t r_vsync                    = {"r_vsync", "", 0.0f, 0, qtrue, TYPE_BOOL, CVG_RENDERER, CVAR_SAVE};
cvar_t r_fullscreen               = {"r_fullscreen", "", 0.0f, 0, qtrue, TYPE_BOOL, CVG_RENDERER, CVAR_SAVE};
cvar_t r_native_fullscreen        = {"r_native_fullscreen", "", 0.0f, 0, qfalse, TYPE_BOOL, CVG_RENDERER, CVAR_SAVE};
cvar_t r_hidden                   = {"r_hidden", "", 0.0f, 0, qfalse, TYPE_BOOL, CVG_RENDERER, CVAR_SAVE};
cvar_t r_drawFPS                  = {"r_drawFPS", "", 0.0f, 0, qfalse, TYPE_BOOL, CVG_RENDERER, CVAR_SAVE};
cvar_t r_dither                   = {"r_dither", "", 0.0f, 0, qfalse, TYPE_BOOL, CVG_RENDERER, CVAR_SAVE};
cvar_t r_multisampleAmount        = {"r_multisampleAmount", "", 0.0f, 2, qfalse, TYPE_INT, CVG_RENDERER, CVAR_SAVE};
cvar_t r_multisampleType          = {"r_multisampleType", "MSAA", 0.0f, 0, qfalse, TYPE_STRING, CVG_RENDERER, CVAR_SAVE};
cvar_t r_renderapi                = {"r_renderapi", "", 0.0f, (int32_t)R_OPENGL, qfalse, TYPE_INT, CVG_RENDERER, CVAR_SAVE};
cvar_t r_EXT_anisotropicFiltering = {"r_EXT_anisotropicFiltering", "", 0.0f, 0, qfalse, TYPE_INT, CVG_RENDERER, CVAR_SAVE};
cvar_t r_gammaAmount              = {"r_gammaAmount", "", 2.2f, 0, qfalse, TYPE_FLOAT, CVG_RENDERER, CVAR_SAVE};
cvar_t r_textureMagFilter         = {"r_textureMagFilter", "GL_NEAREST", 0.0f, 0, qfalse, TYPE_STRING, CVG_RENDERER, CVAR_SAVE | CVAR_DEV};
cvar_t r_textureMinFilter         = {"r_textureMinFilter", "GL_NEAREST", 0.0f, 0, qfalse, TYPE_STRING, CVG_RENDERER, CVAR_SAVE | CVAR_DEV};
cvar_t r_textureFiltering         = {"r_textureFiltering", "Nearest", 0.0f, 0, qfalse, TYPE_STRING, CVG_RENDERER, CVAR_SAVE};
cvar_t r_textureCompression       = {"r_textureCompression", "", 0.0f, 0, qfalse, TYPE_BOOL, CVG_RENDERER, CVAR_SAVE};
cvar_t r_textureDetail            = {"r_textureDetail", "medium", 0.0f, 0, qfalse, TYPE_STRING, CVG_RENDERER, CVAR_SAVE};
cvar_t r_bloomOn                  = {"r_bloomOn", "", 0.0f, 0, qtrue, TYPE_BOOL, CVG_RENDERER, CVAR_SAVE};
cvar_t r_useExtensions            = {"r_useExtensions", "", 0.0f, 0, qtrue, TYPE_BOOL, CVG_RENDERER, CVAR_SAVE};
cvar_t r_fovWidth                 = {"r_fovWidth", "", 0.0f, 60, qfalse, TYPE_INT, CVG_RENDERER, CVAR_SAVE};
cvar_t r_fovHeight                = {"r_fovHeight", "", 0.0f, 40, qfalse, TYPE_INT, CVG_RENDERER, CVAR_SAVE};

typedef struct
{
    SDL_Event event;
	qboolean kbstate[NUMKEYS];
	uint32_t windowEvents;
} eventState_t;

namespace EA::StdC {
	int Vsnprintf(char* EA_RESTRICT pDestination, size_t n, const char* EA_RESTRICT pFormat, va_list arguments)
	{ return vsnprintf(pDestination, n, pFormat, arguments); }
};

static char *com_buffer;
static int32_t com_bufferLen;

eventState_t evState;

/*
==================================================
common functions that are used almost everywhere
==================================================
*/


static void done(void)
{
	Sys_Exit(1);
}

/*
Com_GetEvents: used by the external engine libraries as a helper for the events loop
*/
void* Com_GetEvents(void)
{
	return &evState.event;
}

uint32_t Com_GetWindowEvents(void)
{
	return evState.windowEvents;
}

qboolean* Com_GetKeyboard(void)
{
	return evState.kbstate;
}

void Com_UpdateEvents(void)
{
	EASY_FUNCTION();
    SDL_PumpEvents();

	memset(&evState, 0, sizeof(evState));
	while (SDL_PollEvent(&evState.event)) {
		if (!RE_ConsoleIsOpen())
			switch (evState.event.type) {
			case SDL_KEYDOWN:
				switch (evState.event.key.keysym.sym) {
				case SDLK_a: evState.kbstate[KEY_A] = qtrue; break;
				case SDLK_b: evState.kbstate[KEY_B] = qtrue; break;
				case SDLK_c: evState.kbstate[KEY_C] = qtrue; break;
				case SDLK_d: evState.kbstate[KEY_D] = qtrue; break;
				case SDLK_e: evState.kbstate[KEY_E] = qtrue; break;
				case SDLK_f: evState.kbstate[KEY_F] = qtrue; break;
				case SDLK_g: evState.kbstate[KEY_G] = qtrue; break;
				case SDLK_h: evState.kbstate[KEY_H] = qtrue; break;
				case SDLK_i: evState.kbstate[KEY_I] = qtrue; break;
				case SDLK_j: evState.kbstate[KEY_J] = qtrue; break;
				case SDLK_k: evState.kbstate[KEY_K] = qtrue; break;
				case SDLK_l: evState.kbstate[KEY_L] = qtrue; break;
				case SDLK_m: evState.kbstate[KEY_M] = qtrue; break;
				case SDLK_n: evState.kbstate[KEY_N] = qtrue; break;
				case SDLK_o: evState.kbstate[KEY_O] = qtrue; break;
				case SDLK_p: evState.kbstate[KEY_P] = qtrue; break;
				case SDLK_q: evState.kbstate[KEY_Q] = qtrue; break;
				case SDLK_r: evState.kbstate[KEY_R] = qtrue; break;
				case SDLK_s: evState.kbstate[KEY_S] = qtrue; break;
				case SDLK_t: evState.kbstate[KEY_T] = qtrue; break;
				case SDLK_u: evState.kbstate[KEY_U] = qtrue; break;
				case SDLK_v: evState.kbstate[KEY_V] = qtrue; break;
				case SDLK_w: evState.kbstate[KEY_W] = qtrue; break;
				case SDLK_x: evState.kbstate[KEY_X] = qtrue; break;
				case SDLK_y: evState.kbstate[KEY_Y] = qtrue; break;
				case SDLK_z: evState.kbstate[KEY_Z] = qtrue; break;
				case SDLK_0: evState.kbstate[KEY_0] = qtrue; break;
				case SDLK_1: evState.kbstate[KEY_1] = qtrue; break;
				case SDLK_2: evState.kbstate[KEY_2] = qtrue; break;
				case SDLK_3: evState.kbstate[KEY_3] = qtrue; break;
				case SDLK_4: evState.kbstate[KEY_4] = qtrue; break;
				case SDLK_5: evState.kbstate[KEY_5] = qtrue; break;
				case SDLK_6: evState.kbstate[KEY_6] = qtrue; break;
				case SDLK_7: evState.kbstate[KEY_7] = qtrue; break;
				case SDLK_8: evState.kbstate[KEY_8] = qtrue; break;
				case SDLK_9: evState.kbstate[KEY_9] = qtrue; break;
				case SDLK_UP: evState.kbstate[KEY_UP] = qtrue; break;
				case SDLK_DOWN: evState.kbstate[KEY_DOWN] = qtrue; break;
				case SDLK_LEFT: evState.kbstate[KEY_LEFT] = qtrue; break;
				case SDLK_RIGHT: evState.kbstate[KEY_RIGHT] = qtrue; break;
				case SDLK_BACKQUOTE: evState.kbstate[KEY_BACKQUOTE] = qtrue; break;
				case SDLK_SPACE: evState.kbstate[KEY_SPACE] = qtrue; break;
				case SDLK_BACKSPACE: evState.kbstate[KEY_BACKSPACE] = qtrue; break;
				case SDLK_TAB: evState.kbstate[KEY_TAB] = qtrue; break;
				case SDLK_LSHIFT: evState.kbstate[KEY_LSHIFT] = qtrue; break;
				case SDLK_RSHIFT: evState.kbstate[KEY_RSHIFT] = qtrue; break;
				case SDLK_F1: evState.kbstate[KEY_F1] = qtrue; break;
				case SDLK_F2: evState.kbstate[KEY_F2] = qtrue; break;
				case SDLK_F3: evState.kbstate[KEY_F3] = qtrue; break;
				case SDLK_F4: evState.kbstate[KEY_F4] = qtrue; break;
				case SDLK_F5: evState.kbstate[KEY_F5] = qtrue; break;
				case SDLK_F6: evState.kbstate[KEY_F6] = qtrue; break;
				case SDLK_F7: evState.kbstate[KEY_F7] = qtrue; break;
				case SDLK_F8: evState.kbstate[KEY_F8] = qtrue; break;
				case SDLK_F9: evState.kbstate[KEY_F9] = qtrue; break;
				case SDLK_F10: evState.kbstate[KEY_F10] = qtrue; break;
				case SDLK_F11: evState.kbstate[KEY_F11] = qtrue; break;
				case SDLK_F12: evState.kbstate[KEY_F12] = qtrue; break;
				case SDLK_ESCAPE: evState.kbstate[KEY_ESCAPE] = qtrue; break;
				case SDLK_LCTRL: evState.kbstate[KEY_LCTRL] = qtrue; break;
				case SDLK_RCTRL: evState.kbstate[KEY_RCTRL] = qtrue; break;
			case SDL_QUIT:
				done();
				break;
			case SDL_KEYUP:
				break;
			case SDL_WINDOWEVENT:
				switch (evState.event.window.type) {
				case SDL_WINDOWEVENT_RESIZED:
					evState.windowEvents |= SDL_WINDOWEVENT_RESIZED;
					break;
				};
				break;
			};
		}
	}
}

void GDR_NORETURN GDR_DECL N_Error(const char *err, ...)
{
    char msg[1024];
    memset(msg, 0, sizeof(msg));
    va_list argptr;
    va_start(argptr, err);
    stbsp_vsnprintf(msg, sizeof(msg) - 1, err, argptr);
    va_end(argptr);

	Con_Printf(ERROR, "%s", msg);
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
	vfprintf(stdout, fmt, argptr);
    va_end(argptr);
	fprintf(stdout, "\n");
}

/*
Com_Error: the vm's version of N_Error
*/
void GDR_DECL Com_Error(const char *fmt,  ...)
{
    int length;
    va_list argptr;
    char msg[MAX_MSG_SIZE];

    va_start(argptr, fmt);
    length = stbsp_vsnprintf(msg, sizeof(msg), fmt, argptr);
    va_end(argptr);

    N_Error("(VM Error) %s", msg);
}

/*
Com_Crash_f: force crash, only for devs
*/
void Com_Crash_f(void)
{
    *((int *)0) = 0x1234;
}

/*
Com_Shutdown_f: for testing exit/crashing processes
*/
void Com_Shutdown_f(void)
{
    N_Error("testing");
}

/*
==================================================
Commands:
anything with a Cmd_ or CMD_ prefix is a function that operates on the command-line (or in-game console) functionality.
mostly meant for developers/debugging
==================================================
*/

#define BIG_INFO_STRING 8192
#define MAX_STRING_TOKENS 1024
#define MAX_HISTORY 32

typedef void (*cmdfunc_t)(void);
typedef struct cmd_s
{
    GDRStr name;
    cmdfunc_t function;
	completionFunc_t complete;
    struct cmd_s* next;
} cmd_t;

static boost::mutex cmdLock;
static cmd_t* cmd_functions;
static uint32_t numCommands = 0;
static eastl::atomic<uint32_t> cmd_argc;
static char cmd_tokenized[BIG_INFO_STRING+MAX_STRING_TOKENS];
static char *cmd_argv[MAX_STRING_TOKENS];
static char cmd_cmd[BIG_INFO_STRING];

static char cmd_history[MAX_HISTORY][BIG_INFO_STRING];
static eastl::atomic<uint32_t> cmd_historyused;

uint32_t Cmd_Argc(void)
{
	return cmd_argc.load();
}

void Cmd_Clear(void)
{
	boost::unique_lock<boost::mutex> lock{cmdLock};
	memset(cmd_cmd, 0, sizeof(cmd_cmd));
	memset(cmd_argv, 0, sizeof(cmd_argv));
	memset(cmd_tokenized, 0, sizeof(cmd_tokenized));
	lock.unlock();
	cmd_argc.store(0);
}

const char *Cmd_Argv(uint32_t index)
{
	boost::unique_lock<boost::mutex> lock{cmdLock};
    if ((unsigned)index >= cmd_argc.load()) {
        return "";
    }
    return cmd_argv[index];
}
static cmd_t* Cmd_FindCommand(const char *name)
{
    for (cmd_t *cmd = cmd_functions; cmd; cmd = cmd->next) {
        if (N_strneq(cmd->name.c_str(), name, cmd->name.size())) {
            return cmd;
		}
    }
    return NULL;
}

static void Cmd_TokenizeString(const char *str)
{
	const char *p;
	char *tok;

	Cmd_Clear();
	N_strncpy(cmd_cmd, str, sizeof(cmd_cmd));
	p = str;
	tok = cmd_tokenized;

	while (1) {
		if (cmd_argc.load() >= arraylen(cmd_argv)) {
			return; // usually something malicious
		}
		while (*p && *p <= ' ') {
			p++; // skip whitespace
		}
		if (!*p) {
			break; // end of string
		}
		if (*p == '"') {
			cmd_argv[cmd_argc] = tok;
			cmd_argc.fetch_add(1);
			p++;
			while (*p && *p != '"') {
				*tok++ = *p++;
			}
			if (!*p) {
				return; // end of string
			}
			p++;
			continue;
		}

		// regular stuff
		cmd_argv[cmd_argc] = tok;
		cmd_argc.fetch_add(1);

		// skip until whitespace
		while (*p > ' ') {
			*tok++ = *p++;
		}
		*tok++ = '\0';
		if (!*p) {
			return; // end of string
		}
	}
}

void Cmd_ExecuteText(const char *str)
{
	boost::unique_lock<boost::mutex> lock{cmdLock};
    cmd_t *cmd;
	const char *cmdstr;

    if (!*str) {
        return; // nothing to do
    }
	Cmd_TokenizeString(str+1);
	if (!Cmd_Argc()) {
		return; // no tokens
	}
	cmdstr = cmd_argv[0];
    cmd = Cmd_FindCommand(cmdstr);
	if (cmd && cmd->function) {
		cmd->function();
		return;
	}
	else {
		Con_Printf("Command '%s' doesn't exist", cmdstr);
		return;
	}

//	if (Cvar_Command()) {
//		return;
//	}
}

void Cmd_AddCommand(const char *name, cmdfunc_t func)
{
	boost::unique_lock<boost::mutex> lock{cmdLock};
    cmd_t* cmd;
    if (Cmd_FindCommand(name)) {
        if (func)
            Con_Printf("Cmd_AddCommand: %s already defined", name);
        return;
    }
    Con_Printf("Registered command %s", name);

    cmd = (cmd_t *)Z_Malloc(sizeof(*cmd), TAG_STATIC, &cmd, "cmd");
    cmd->name = name;
    cmd->function = func;
	cmd->complete = NULL;
    cmd->next = cmd_functions;
    cmd_functions = cmd;
	numCommands++;
}

void Cmd_SetCommandCompletetionFunc(const char *name, completionFunc_t func)
{
	boost::unique_lock<boost::mutex> lock{cmdLock};
    for (cmd_t *cmd = cmd_functions; cmd; cmd = cmd->next) {
        if (cmd->name == name) {
            cmd->complete = func;
            return;
        }
    }
}

void Cmd_RemoveCommand(const char *name)
{
	boost::unique_lock<boost::mutex> lock{cmdLock};
    cmd_t *cmd, **back;

    back = &cmd_functions;
    while (1) {
        cmd = *back;
        if (!cmd) {
            // command wasn't active
            break;
        }
        if (cmd->name.casecmp(name)) {
            *back = cmd->next;
            if (cmd->name.casecmp(name)) {
                cmd->name.clear();
            }
            numCommands--;
            Z_Free(cmd);
            return;
        }
        back = &cmd->next;
    }
}

char* Cmd_ArgsFrom(int32_t arg)
{
	boost::unique_lock<boost::mutex> lock{cmdLock};
    static char cmd_args[BIG_INFO_STRING], *s;
    int32_t i;

    s = cmd_args;
    *s = '\0';
    if (arg < 0)
        arg = 0;
    for (i = arg; i < cmd_argc.load(); i++) {
        s = N_stradd(s, cmd_argv[i]);
        if (i != cmd_argc - 1) {
            s = N_stradd(s, " ");
        }
    }
    return cmd_args;
}

static void Cmd_List_f(void)
{
    Con_Printf("Total number of commands: %i", numCommands);
    for (const cmd_t *cmd = cmd_functions; cmd; cmd = cmd->next) {
        Con_Printf("%s", cmd->name.c_str());
    }
}

static void Cmd_Echo_f(void)
{
    Con_Printf("%s", Cmd_ArgsFrom(1));
}

static void Cmd_Exit_f(void)
{
	done();
}

static void Cmd_Init(void)
{
    Cmd_AddCommand("cmdlist", Cmd_List_f);
    Cmd_AddCommand("crash", Com_Crash_f);
    Cmd_AddCommand("echo", Cmd_Echo_f);
}

const char* GDR_DECL va(const char *format, ...)
{
	boost::unique_lock<boost::mutex> lock{cmdLock};
	char *buf;
	va_list argptr;
	static uint32_t index = 0;
	static char string[2][32000];	// in case va is called by nested functions

	buf = string[ index ];
	index ^= 1;

	va_start( argptr, format );
	vsprintf( buf, format, argptr );
	va_end( argptr );

	return buf;
}

static const eastl::shared_ptr<GDRMap>& G_GetCurrentMap(void)
{
	return Game::Get()->c_map;
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

/*
Com_FillImport: fills render import functions for the dynamic library to use
*/
static void Com_FillImport(renderImport_t *import)
{
#ifdef _NOMAD_DEBUG
    import->Hunk_AllocDebug = Hunk_AllocDebug;
#else
    import->Hunk_Alloc = Hunk_Alloc;
#endif
//    import->Hunk_Log = Hunk_Log;
//    import->Hunk_SmallLog = Hunk_SmallLog;
    import->Hunk_MemoryRemaining = Hunk_MemoryRemaining;
//    import->Hunk_SetMark = Hunk_SetMark;
//    import->Hunk_ClearToMark = Hunk_ClearToMark;
//    import->Hunk_CheckMark = Hunk_CheckMark;
//    import->Hunk_Clear = Hunk_Clear;

    import->Z_Malloc = Z_Malloc;
    import->Z_Calloc = Z_Calloc;
    import->Z_Realloc = Z_Realloc;

	import->Z_Free = Z_Free;
	import->Z_FreeTags = Z_FreeTags;
	import->Z_ChangeTag = Z_ChangeTag;
	import->Z_ChangeUser = Z_ChangeUser;
	import->Z_ChangeName = Z_ChangeName;
	import->Z_CleanCache = Z_CleanCache;
	import->Z_CheckHeap = Z_CheckHeap;
	import->Z_ClearZone = Z_ClearZone;
	import->Z_Print = Z_Print;
    import->Z_FreeMemory = Z_FreeMemory;
    import->Z_NumBlocks = Z_NumBlocks;
	
    import->Mem_Alloc = Mem_Alloc;
    import->Mem_Free = Mem_Free;
//	import->Mem_Msize = Mem_Msize;
//	import->Mem_DefragIsActive = Mem_DefragIsActive;
//	import->Mem_AllocDefragBlock = Mem_AllocDefragBlock;

	// get the specific logger function (its been overloaded)
	import->Con_Printf = static_cast<void(*)(loglevel_t, const char *, ...)>(Con_Printf);
	import->Con_GetBuffer = Con_GetBuffer;
	import->Con_Error = Con_Error;
	import->va = va;
    import->N_Error = N_Error;

	import->Cvar_Find = Cvar_Find;
    import->Cvar_RegisterName = Cvar_RegisterName;
    import->Cvar_ChangeValue = Cvar_ChangeValue;
    import->Cvar_Register = Cvar_Register;
    import->Cvar_GetValue = Cvar_GetValue;

    import->Cmd_AddCommand = Cmd_AddCommand;
    import->Cmd_RemoveCommand = Cmd_RemoveCommand;
    import->Cmd_ExecuteText = Cmd_ExecuteText;
    import->Cmd_Argc = Cmd_Argc;
    import->Cmd_ArgsFrom = Cmd_ArgsFrom;
    import->Cmd_Argv = Cmd_Argv;
    import->Cmd_Clear = Cmd_Clear;

    import->FS_Write = FS_Write;
    import->FS_Read = FS_Read;
    import->FS_OpenBFF = FS_OpenBFF;
    import->FS_FOpenRead = FS_FOpenRead;
    import->FS_FOpenWrite = FS_FOpenWrite;
    import->FS_CreateTmp = FS_CreateTmp;
    import->FS_GetOSPath = FS_GetOSPath;
    import->FS_GetBFFData = FS_GetBFFData;
    import->FS_FClose = FS_FClose;
    import->FS_FileLength = FS_FileLength;
    import->FS_Remove = FS_Remove;
    import->FS_FileTell = FS_FileTell;
    import->FS_FileSeek = FS_FileSeek;
    import->FS_FileExists = FS_FileExists;
	import->FS_LoadFile = FS_LoadFile;
	import->FS_FreeFile = FS_FreeFile;

	import->Com_GetWindowEvents = Com_GetWindowEvents;
	import->Com_GetEvents = Com_GetEvents;
	import->Com_GetKeyboard = Com_GetKeyboard;

	import->BFF_FetchInfo = BFF_FetchInfo;
	import->BFF_FetchLevel = BFF_FetchLevel;
	import->BFF_FetchScript = BFF_FetchScript;
	import->BFF_OrderLevels = BFF_OrderLevels;
	import->BFF_OrderTextures = BFF_OrderTextures;

	import->G_GetCurrentMap = G_GetCurrentMap;

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
}

/*
Com_InitJournals: initializes the logfile and the event journal
*/
static void Com_InitJournals(void)
{
	if (!c_devmode.b)
		return;

	logfile = FS_FOpenWrite("logfile.txt");
	if (logfile == FS_INVALID_HANDLE) {
		Con_Printf("Failed to open logfile");
	}
}

/*
Com_Init: initializes all the engine's systems
*/
void Com_Init(void)
{
    Con_Printf("Com_Init: initializing systems");

	// initialize the cvar system
	Cvar_Init();

	Con_Printf("G_LoadSCF: parsing scf file");
    G_LoadSCF();

	// initialize the allocation daemons
	Memory_Init();
	
	// initialize the command console
	Con_Init();
	Cmd_Init();

	// initialize the filesystem
	FS_Init();

	Com_InitJournals();
    
	// initialize OpenAL
	Snd_Init();

	Con_Printf("G_LoadBFF: loading bff file");
    G_LoadBFF("nomadmain.bff");

	// initialize the vm
	VM_Init();
	
	Game::Init();

	// initialize the rendering engine
	renderImport_t import;
	Com_FillImport(&import);
    RE_Init(&import);

	I_CacheAudio((void *)BFF_FetchInfo());
	Com_CacheMaps();
	
	BFF_FreeInfo(BFF_FetchInfo());

	// clean all the uncached bffs out of memory
	FS_ThePurge();

    Con_Printf(
        "+===========================================================+\n"
         "\"The Nomad\" is free software distributed under the terms\n"
         "of both the GNU General Public License v2.0 and Apache License\n"
         "v2.0\n"
         "+==========================================================+\n"
    );
}

/*
Com_Frame: runs a single frame for the game
*/
void Com_Frame(void)
{
	vm_command = SGAME_RUNTIC;

	// update the event queue
	Com_UpdateEvents();

	// run the backend threads
//	Snd_Run();
	VM_Run(SGAME_VM);

#if 0
	RE_BeginFrame();

	RE_DrawBuffers();

	RE_EndFrame(); // submit everything
#endif

	VM_Stop(SGAME_VM);
//	Snd_Stop();

	// 'framerate cap'
	sleepfor(1000 / r_ticrate.i);
}

/*
Parsing functions mostly meant for shader stuff, but is also used on occasion around the project
*/


static	char	com_token[MAX_TOKEN_CHARS];
static	char	com_parsename[MAX_TOKEN_CHARS];
static	int		com_lines;
static  int		com_tokenline;

// for complex parser
tokenType_t		com_tokentype;

void COM_BeginParseSession( const char *name )
{
	com_lines = 1;
	com_tokenline = 0;
	snprintf(com_parsename, sizeof(com_parsename), "%s", name);
}


int COM_GetCurrentParseLine( void )
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
	vsnprintf (string, sizeof(string), format, argptr);
	va_end( argptr );

	Con_Printf( "ERROR: %s, line %d: %s\n", com_parsename, COM_GetCurrentParseLine(), string );
}


void COM_ParseWarning( const char *format, ... )
{
	va_list argptr;
	static char string[4096];

	va_start( argptr, format );
	vsnprintf (string, sizeof(string), format, argptr);
	va_end( argptr );

	Con_Printf( "WARNING: %s, line %d: %s\n", com_parsename, COM_GetCurrentParseLine(), string );
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
static const char *SkipWhitespace( const char *data, qboolean *hasNewLines ) {
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
	if ( !data )
	{
		*data_p = NULL;
		return com_token;
	}

	while ( 1 )
	{
		// skip whitespace
		data = SkipWhitespace( data, &hasNewLines );
		if ( !data )
		{
			*data_p = NULL;
			return com_token;
		}
		if ( hasNewLines && !allowLineBreaks )
		{
			*data_p = data;
			return com_token;
		}

		c = *data;

		// skip double slash comments
		if ( c == '/' && data[1] == '/' )
		{
			data += 2;
			while (*data && *data != '\n') {
				data++;
			}
		}
		// skip /* */ comments
		else if ( c == '/' && data[1] == '*' )
		{
			data += 2;
			while ( *data && ( *data != '*' || data[1] != '/' ) )
			{
				if ( *data == '\n' )
				{
					com_lines++;
				}
				data++;
			}
			if ( *data )
			{
				data += 2;
			}
		}
		else
		{
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
static void COM_MatchToken( const char **buf_p, const char *match ) {
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


static int Hex( char c )
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
int Com_HexStrToInt(const char *str)
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


size_t Com_ReadFile(const char *filepath, void *buffer)
{
	if (!buffer) {
		N_Error("N_ReadFile: null buffer");
	}
	FILE* fp = fopen(filepath, "rb");
	if (!fp) {
		N_Error("N_LoadFile: failed to open file %s", filepath);
	}
	fseek(fp, 0L, SEEK_END);
	size_t fsize = ftell(fp);
	fseek(fp, 0L, SEEK_SET);
	void *buf = Z_Malloc(fsize, TAG_FILE_USED, &buf, "filebuf");
	if (fread(buf, fsize, 1, fp) == 0) {
		N_Error("N_LoadFile: failed to read %lu bytes from file %s", fsize, filepath);
	}
	memcpy(buffer, buf, fsize);
	Z_ChangeTag(buf, TAG_FILE_FREE);
	fclose(fp);
	return fsize;
}

size_t Com_FileSize(const char *filepath)
{
	FILE* fp = fopen(filepath, "rb");
	if (!fp) {
		N_Error("N_FileSize: failed to oepn file %s", filepath);
	}
	fseek(fp, 0L, SEEK_END);
	size_t fsize = ftell(fp);
	fseek(fp, 0L, SEEK_SET);
	fclose(fp);
	return fsize;
}
size_t Com_LoadFile(const char *filepath, void **buffer)
{
	FILE* fp = fopen(filepath, "rb");
	if (!fp) {
		N_Error("N_LoadFile: failed to open file %s", filepath);
	}
	fseek(fp, 0L, SEEK_END);
	size_t fsize = ftell(fp);
	fseek(fp, 0L, SEEK_SET);
	void *buf = Z_Malloc(fsize, TAG_FILE_USED, &buf, "filebuf");
	if (fread(buf, fsize, 1, fp) == 0) {
		N_Error("N_LoadFile: failed to read %lu bytes from file %s", fsize, filepath);
	}
	*buffer = buf;
	fclose(fp);
	return fsize;
}
void Com_WriteFile(const char *filepath, const void *data, size_t size)
{
	FILE* fp = fopen(filepath, "wb");
	if (!fp) {
		Con_Error(false, "N_WriteFile: failed to open file %s", filepath);
		return;
	}
	if (fwrite(data, size, 1, fp) == 0) {
		Con_Error(false, "N_WriteFile: failed to write %lu bytes to file %s", size, filepath);
		return;
	}
	fclose(fp);
}
