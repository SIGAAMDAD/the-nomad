#include "n_shared.h"
#include "m_renderer.h"
#include "g_game.h"
#include "g_sound.h"

static char *com_buffer;
static int com_bufferLen;

eventState_t evState;
qboolean console_open = qfalse;

/*
==================================================
Common stuff:
common functions that are used almost everywhere
==================================================
*/


static void done(void)
{
    Game::Get()->~Game();
    exit(EXIT_SUCCESS);
}

void Com_UpdateEvents(void)
{
	EASY_FUNCTION();
    SDL_PumpEvents();

	memset(evState.kbstate, qfalse, sizeof(evState.kbstate));
	while (SDL_PollEvent(&evState.event)) {
		if (console_open) {
			ImGui_ImplSDL2_ProcessEvent(&evState.event);
			if (evState.event.type == SDL_KEYDOWN && evState.event.key.keysym.sym == SDLK_BACKQUOTE) {
				console_open = qfalse;
			}
			else if (evState.event.type == SDL_QUIT) {
				done();
			}
		}
		else {
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
				case SDLK_BACKQUOTE:
					if (console_open)
						console_open = qfalse;
					else
						console_open = qtrue;
					break;
				};
				break;
			case SDL_QUIT:
				done();
				break;
			case SDL_KEYUP:
				break;
			case SDL_WINDOWEVENT:
				switch (evState.event.window.type) {
				case SDL_WINDOWEVENT_RESIZED:
					renderer->camera.Resize();
					break;
				};
				break;
			};
		}
	}
}

void Con_RenderConsole(void)
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();
}


/*
Com_GenerateHashValue: used in renderer and filesystem
*/
// ASCII lowcase conversion table with '\\' turned to '/' and '.' to '\0'
static const byte hash_locase[ 256 ] =
{
	0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,
	0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f,
	0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,
	0x18,0x19,0x1a,0x1b,0x1c,0x1d,0x1e,0x1f,
	0x20,0x21,0x22,0x23,0x24,0x25,0x26,0x27,
	0x28,0x29,0x2a,0x2b,0x2c,0x2d,0x00,0x2f,
	0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,
	0x38,0x39,0x3a,0x3b,0x3c,0x3d,0x3e,0x3f,
	0x40,0x61,0x62,0x63,0x64,0x65,0x66,0x67,
	0x68,0x69,0x6a,0x6b,0x6c,0x6d,0x6e,0x6f,
	0x70,0x71,0x72,0x73,0x74,0x75,0x76,0x77,
	0x78,0x79,0x7a,0x5b,0x2f,0x5d,0x5e,0x5f,
	0x60,0x61,0x62,0x63,0x64,0x65,0x66,0x67,
	0x68,0x69,0x6a,0x6b,0x6c,0x6d,0x6e,0x6f,
	0x70,0x71,0x72,0x73,0x74,0x75,0x76,0x77,
	0x78,0x79,0x7a,0x7b,0x7c,0x7d,0x7e,0x7f,
	0x80,0x81,0x82,0x83,0x84,0x85,0x86,0x87,
	0x88,0x89,0x8a,0x8b,0x8c,0x8d,0x8e,0x8f,
	0x90,0x91,0x92,0x93,0x94,0x95,0x96,0x97,
	0x98,0x99,0x9a,0x9b,0x9c,0x9d,0x9e,0x9f,
	0xa0,0xa1,0xa2,0xa3,0xa4,0xa5,0xa6,0xa7,
	0xa8,0xa9,0xaa,0xab,0xac,0xad,0xae,0xaf,
	0xb0,0xb1,0xb2,0xb3,0xb4,0xb5,0xb6,0xb7,
	0xb8,0xb9,0xba,0xbb,0xbc,0xbd,0xbe,0xbf,
	0xc0,0xc1,0xc2,0xc3,0xc4,0xc5,0xc6,0xc7,
	0xc8,0xc9,0xca,0xcb,0xcc,0xcd,0xce,0xcf,
	0xd0,0xd1,0xd2,0xd3,0xd4,0xd5,0xd6,0xd7,
	0xd8,0xd9,0xda,0xdb,0xdc,0xdd,0xde,0xdf,
	0xe0,0xe1,0xe2,0xe3,0xe4,0xe5,0xe6,0xe7,
	0xe8,0xe9,0xea,0xeb,0xec,0xed,0xee,0xef,
	0xf0,0xf1,0xf2,0xf3,0xf4,0xf5,0xf6,0xf7,
	0xf8,0xf9,0xfa,0xfb,0xfc,0xfd,0xfe,0xff
};

uint64_t Com_GenerateHashValue( const char *fname, const uint32_t size )
{
	const byte *s;
	uint32_t hash;
	int c;

	s = (byte *)fname;
	hash = 0;
	
	while ( (c = hash_locase[(byte)*s++]) != '\0' ) {
		hash = hash * 101 + c;
	}
	
	hash = (hash ^ (hash >> 10) ^ (hash >> 20));
	hash &= (size-1);

	return hash;
}

/*
Com_Printf: can be used by either the main engine, or the vm
a raw string should NEVER be passed as fmt, same reason as the quake3 engine.
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
System Calls:
anything with the Sys_ prefix is a system-specific function. Most are defined here.
==================================================
*/

/*
Sys_Print: this is meant as a replacement for fprintf
*/
void GDR_DECL Sys_Print(const char* str)
{
    if (!str) {
        N_Error("Sys_Print: null string");
    }
    fwrite(str, strlen(str), 1, stdout);
    fflush(stdout);
//    int length = strlen(str);

    // no buffering, that's already done by the calling functions
#ifdef _WIN32
//    _write(STDOUT_FILENO, (const void *)str, length); // shitty win32 api
#else
//    write(STDOUT_FILENO, (const void *)str, length);
#endif
}

void GDR_DECL Sys_Exit(int code)
{
    exit(code);
}

int Sys_stat(nstat_t* buffer, const char *filepath)
{
#ifdef _WIN32
    return __stat64(filepath, buffer);
#else
    return stat(filepath, buffer);
#endif
}

FILE* Sys_FOpen(const char *filepath, const char *mode)
{
    if (!filepath)
        N_Error("Sys_FOpen: null filepath");
    if (!mode)
        N_Error("Sys_FOpen: null mode");
    
    return fopen(filepath, mode);
}

void* Sys_LoadLibrary(const char *libname)
{
#ifdef _WIN32
    if (!GetModuleHandleA(libname))
        return (void *)NULL;
    return LoadLibraryA(libname);
#elif defined(__unix__)
    if (!*libname)
        return (void *)NULL;
    return dlopen(libname, RTLD_NOW);
#endif
}

static const char *Sys_DLLerr(void)
{
#ifdef _WIN32
    return GetLastError();
#else
    return dlerror();
#endif
}

void* Sys_LoadProc(void *handle, const char *name)
{
    if (!handle)
        N_Error("Sys_LoadProc: null handle");
    if (!*name)
        N_Error("Sys_LoadProc: empty name");

    void *proc;
#ifdef _WIN32
    proc = GetProcAddress((HMODULE)handle, name);
#else
    proc = dlsym(handle, name);
#endif
    if (!proc) {
        Con_Printf("WARNING: failed to load library proc address %s, lasterr: %s", name, Sys_DLLerr());
    }
    return proc;
}

void Sys_FreeLibrary(void *handle)
{
    if (!handle)
        N_Error("Sys_FreeLibrary: null handle");
    
#ifdef _WIN32
    FreeLibraryA((HMODULE)handle);
#else
    dlclose(handle);
#endif
}


const char* Sys_pwd(void)
{
    static char buffer[MAX_OSPATH];
    char *p;

    p = getcwd(buffer, MAX_OSPATH);
    if (!p) {
        N_Error("Sys_pwd: getcwd returned NULL, errno: %s", strerror(errno));
    }
    return buffer;
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

static cmd_t* cmd_functions;
static uint32_t numCommands = 0;
static uint32_t cmd_argc;
static char cmd_tokenized[BIG_INFO_STRING+MAX_STRING_TOKENS];
static char *cmd_argv[MAX_STRING_TOKENS];
static char cmd_cmd[BIG_INFO_STRING];

static char cmd_history[MAX_HISTORY][BIG_INFO_STRING];
static uint32_t cmd_historyused;

uint32_t Cmd_Argc(void)
{
    return cmd_argc;
}

void Cmd_Clear(void)
{
	memset(cmd_cmd, 0, sizeof(cmd_cmd));
	memset(cmd_argv, 0, sizeof(cmd_argv));
	memset(cmd_tokenized, 0, sizeof(cmd_tokenized));
    cmd_argc = 0;
}

const char *Cmd_Argv(uint32_t index)
{
    if ((unsigned)index  >= cmd_argc) {
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
		if (cmd_argc >= arraylen(cmd_argv)) {
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
			cmd_argc++;
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
		cmd_argc++;

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
    for (cmd_t *cmd = cmd_functions; cmd; cmd = cmd->next) {
        if (cmd->name == name) {
            cmd->complete = func;
            return;
        }
    }
}

void Cmd_RemoveCmd(const char *name)
{
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
    static char cmd_args[BIG_INFO_STRING], *s;
    int32_t i;

    s = cmd_args;
    *s = '\0';
    if (arg < 0)
        arg = 0;
    for (i = arg; i < cmd_argc; i++) {
        s = N_stradd(s, cmd_argv[i]);
        if (i != cmd_argc - 1) {
            s = N_stradd(s, " ");
        }
    }
    return cmd_args;
}

static void Cmd_List_f(void)
{
    Con_Printf("Total number of commands: %lu", numCommands);
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
/*
Com_Init: initializes all the engine's systems
*/
void Com_Init(void)
{
    Con_Printf("Com_Init: initializing systems");

    Memory_Init();

	Con_Printf("G_LoadSCF: parsing scf file");
    G_LoadSCF();
	Con_Init();
    R_Init();
    Cmd_Init();
    Snd_Init();

    Con_Printf("G_LoadBFF: loading bff file");
    G_LoadBFF("nomadmain.bff");

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

// the game guarantees that no string from the network will ever
// exceed MAX_STRING_CHARS
#define	MAX_STRING_CHARS	1024	// max length of a string passed to Cmd_TokenizeString
#define	MAX_STRING_TOKENS	1024	// max tokens resulting from Cmd_TokenizeString
#define	MAX_TOKEN_CHARS		1024	// max length of an individual token

#define	MAX_INFO_STRING		1024
#define	MAX_INFO_KEY		1024
#define	MAX_INFO_VALUE		1024

#define MAX_USERINFO_LENGTH (MAX_INFO_STRING-13) // incl. length of 'connect ""' or 'userinfo ""' and reserving one byte to avoid q3msgboom
													
#define	BIG_INFO_STRING		8192  // used for system info key only
#define	BIG_INFO_KEY		  8192
#define	BIG_INFO_VALUE		8192


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