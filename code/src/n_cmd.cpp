#include "n_shared.h"
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
	Sys_Exit(1);
}

static void Cmd_Exec_f(void)
{
	if (Cmd_Argc() < 1) {
		Con_Printf("usage: exec <script file> [args...] (maximum of 16 args)");
		return;
	}

	char args[16][256];
	for (int32_t i = 1; i < Cmd_Argc(); i++)
		N_strncpyz(args[i], Cmd_ArgsFrom(i), sizeof(*args));

	const char *path = Cmd_Argv(0);
	file_t f = FS_FOpenRead(path);
	if (f == FS_INVALID_HANDLE) {
		Con_Printf(ERROR, "Could not find script file %s", path);
		return;
	}

	const uint64_t numLines = FS_FileLength(f);
	for (uint64_t i = 0; i < numLines; i++) {
		char buffer[1024];
		FS_GetLine(buffer, sizeof(buffer), f);
		Cmd_ExecuteText(buffer);
	}
	FS_FClose(f);
}

void Cmd_Init(void)
{
    Cmd_AddCommand("cmdlist", Cmd_List_f);
    Cmd_AddCommand("echo", Cmd_Echo_f);
}