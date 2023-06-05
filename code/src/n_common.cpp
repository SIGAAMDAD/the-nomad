#include "n_shared.h"
#include "m_renderer.h"

#ifdef __unix__
#include <unistd.h>
#include <sys/stat.h>
#endif

static char *com_buffer;
static int com_bufferLen;

/*
==================
Com_GenerateHashValue

used in renderer and filesystem
==================
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
Com_Init: initializes all the engine's systems
*/
void Com_Init(void)
{
    Con_Printf("Com_Init: initializing systems");

    Memory_Init();
    R_Init();

    com_bufferLen = 0;
    com_buffer = (char *)Hunk_Alloc(MAX_BUFFER_SIZE, "combuffer", h_low);
    memset(com_buffer, 0, MAX_BUFFER_SIZE);
}


/*
Com_Printf: can be used by either the main engine, or the vm
a raw string should NEVER be passed as fmt, same reason as the quake3 engine.
*/
void GDR_DECL Com_Printf(const char *fmt, ...)
{
    va_list argptr;

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
    va_list argptr;
    char msg[MAX_MSG_SIZE];

    va_start(argptr, fmt);
    stbsp_vsnprintf(msg, sizeof(msg), fmt, argptr);
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

void Sys_mkdir(const char *dirpath)
{
    if (!dirpath)
        N_Error("Sys_mkdir: NULL dirpath");
    
    mkdir(dirpath, (mode_t)0777);
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
anything with a Cmd_, Cbuf_, or CMD_ prefix is a function that operates on the command-line (or in-game console) functionality.
mostly meant for developers/debugging
==================================================
*/

#define MAX_CMD_BUFFER 65536
#define MAX_STRING_TOKENS 1024
#define BIG_INFO_STRING 8192

typedef struct
{
    byte *data;
    uint32_t maxsize;
    uint32_t cursize;
} cbuf_t;

static cbuf_t cmd_text;
static byte cmd_text_buf[MAX_CMD_BUFFER];

void Cbuf_Init(void)
{
    cmd_text.data = cmd_text_buf;
    cmd_text.maxsize = MAX_CMD_BUFFER;
    cmd_text.cursize = 0;
}

void Cbud_AddText(const char *text)
{
    const uint64_t l = (uint64_t)strlen(text);
    
    if (cmd_text.cursize + l >= cmd_text.maxsize) {
        Con_Printf("Cbuf_AddText: overflow");
        return;
    }

    memcpy(&cmd_text.data[cmd_text.cursize], text, l);
    cmd_text.cursize += l;
}

int Cbuf_Add( const char *text, int pos )
{
	int len = (int)strlen( text );
	qboolean separate = qfalse;
	int i;

	if ( len == 0 ) {
		return cmd_text.cursize;
	}

	if ( pos > cmd_text.cursize || pos < 0 ) {
		// insert at the text end
		pos = cmd_text.cursize;
	}

	if ( text[len - 1] == '\n' || text[len - 1] == ';' ) {
		// command already has separator
	} else {
		separate = qtrue;
		len += 1;
	}

	if ( len + cmd_text.cursize > cmd_text.maxsize ) {
		Con_Printf("%s(%i) overflowed", FUNC_SIG, pos );
		return cmd_text.cursize;
	}

	// move the existing command text
	for ( i = cmd_text.cursize - 1; i >= pos; i-- ) {
		cmd_text.data[i + len] = cmd_text.data[i];
	}

	if ( separate ) {
		// copy the new text in + add a \n
		memcpy( cmd_text.data + pos, text, len - 1 );
		cmd_text.data[pos + len - 1] = '\n';
	} else {
		// copy the new text in
		memcpy( cmd_text.data + pos, text, len );
	}

	cmd_text.cursize += len;

	return pos + len;
}



typedef struct cmd_s
{
    GDRStr name;
    cmdfunc_t function;
    completionFunc_t complete;
    struct cmd_s* next;
} cmd_t;
static cmd_t* cmd_functions;
static uint64_t numCommands = 0;
static int cmd_argc;
static char cmd_argv[MAX_STRING_TOKENS];

static cmd_t* Cmd_FindCommand(const char *name)
{
    for (cmd_t *cmd = cmd_functions; cmd; cmd = cmd->next) {
        if (cmd->name == name) {
            return cmd;
        }
    }
    return NULL;
}

void Cmd_AddCommand(const char *name, cmdfunc_t func)
{
    cmd_t *cmd;

    if (strlen(name) >= BASE_CHAR_BUFFER_SIZE) {
        Con_Printf("WARNING: strlen(name) >= BASE_CHAR_BUFFER_SIZE in Cmd_AddCommand, heap allocation required");
    }

    // fail if the command already exists
    if (Cmd_FindCommand(name)) {
        // allow completeion-only commands to be silently doubled
        if (func != NULL)
            Con_Printf("Cmd_AddCommand: %s already defined", name);
        return;
    }

    cmd = (cmd_t *)Z_Malloc(sizeof(cmd_t), TAG_STATIC, &cmd, "cmd");
    cmd->name = name;
    cmd->function = func;
    cmd->complete = NULL;
    cmd->next = cmd_functions;
    cmd_functions = cmd;
    numCommands++;
}

/*
============
Cmd_ExecuteString

A complete command line has been parsed, so try to execute it
============
*/
#if 0
void Cmd_ExecuteString( const char *text )
{
	cmd_t *cmd, **prev;

	// execute the command line
	Cmd_TokenizeString( text );
	if ( !Cmd_Argc() ) {
		return;		// no tokens
	}

	// check registered command functions
	for ( prev = &cmd_functions ; *prev ; prev = &cmd->next ) {
		cmd = *prev;
		if ( N_strcasecmp( cmd_argv[0], cmd->name ) ) {
			// rearrange the links so that the command will be
			// near the head of the list next time it is used
			*prev = cmd->next;
			cmd->next = cmd_functions;
			cmd_functions = cmd;

			// perform the action
			if ( !cmd->function ) {
				// let the cgame or game handle it
				break;
			} else {
				cmd->function();
			}
			return;
		}
	}

	// check cvars
	if (Cvar_Command()) {
		return;
	}
}
#endif

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
//        s = N_stradd(s, cmd_argv[i]);
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

void Cmd_Init(void)
{
    Cmd_AddCommand("cmdlist", Cmd_List_f);
    Cmd_AddCommand("crash", Com_Crash_f);
    Cmd_AddCommand("echo", Cmd_Echo_f);
}