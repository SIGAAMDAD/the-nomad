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
#define MAX_CMD_BUFFER  65536

typedef struct {
	byte *data;
	uint64_t maxsize;
	uint64_t cursize;
} cbuf_t;

static byte cmd_text_buf[MAX_CMD_BUFFER];
static cbuf_t cmd_text;

/*
Cbuf_Clear: clears the command buffer
*/
void Cbuf_Clear(void)
{
	memset(cmd_text_buf, 0, sizeof(cmd_text_buf));
	cmd_text.data = cmd_text_buf;
	cmd_text.maxsize = MAX_CMD_BUFFER;
	cmd_text.cursize = 0;
}

/*
Cbuf_Init: initializes the command buffer
*/
void Cbuf_Init(void)
{
	Cbuf_Clear();
}

/*
Cbuf_InsertText

Adds command text immediately after the current command
Adds a \n to the text
*/
void Cbuf_InsertText( const char *text )
{
	uint64_t len;
	uint64_t i;

	len = strlen( text ) + 1;

	if ( len + cmd_text.cursize > cmd_text.maxsize ) {
		Con_Printf( WARNING, "Cbuf_InsertText overflowed" );
		return;
	}

	// move the existing command text
	for ( i = cmd_text.cursize - 1 ; i >= 0 ; i-- ) {
		cmd_text.data[ i + len ] = cmd_text.data[ i ];
	}

	// copy the new text in
	memcpy( cmd_text.data, text, len - 1 );

	// add a \n
	cmd_text.data[ len - 1 ] = '\n';

	cmd_text.cursize += len;
}

/*
Cbuf_AddText: Adds command text at the end of the buffer, does NOT add a final \n
*/
void Cbuf_AddText( const char *text )
{
	const uint64_t l = strlen( text );

	if (cmd_text.cursize + l >= cmd_text.maxsize) {
		Con_Printf ("Cbuf_AddText: overflow");
		return;
	}

	memcpy(&cmd_text.data[cmd_text.cursize], text, l);
	cmd_text.cursize += l;
}



void Cbuf_Execute(void)
{
	uint64_t i;
	char *text;
	char line[MAX_CMD_LINE];
	uint64_t quotes;

	// This will keep // style comments all on one line by not breaking on
	// a semicolon.  It will keep /* ... */ style comments all on one line by not
	// breaking it for semicolon or newline.
	qboolean in_star_comment = qfalse;
	qboolean in_slash_comment = qfalse;
	while ( cmd_text.cursize > 0 ) {
#if 0
		if ( cmd_wait > 0 ) {
			// skip out while text still remains in buffer, leaving it
			// for next frame
			cmd_wait--;
			break;
		}
#endif

		// find a \n or ; line break or comment: // or /* */
		text = (char *)cmd_text.data;

		quotes = 0;
		for ( i = 0 ; i< cmd_text.cursize ; i++ ) {
			if (text[i] == '"')
				quotes++;

			if ( !(quotes&1)) {
				if ( i < cmd_text.cursize - 1 ) {
					if ( !in_star_comment && text[i] == '/' && text[i+1] == '/' )
						in_slash_comment = qtrue;
					else if ( !in_slash_comment && text[i] == '/' && text[i+1] == '*' )
						in_star_comment = qtrue;
					else if ( in_star_comment && text[i] == '*' && text[i+1] == '/' ) {
						in_star_comment = qfalse;
						// If we are in a star comment, then the part after it is valid
						// Note: This will cause it to NUL out the terminating '/'
						// but ExecuteString doesn't require it anyway.
						i++;
						break;
					}
				}
				if ( !in_slash_comment && !in_star_comment && text[i] == ';')
					break;
			}
			if ( !in_star_comment && (text[i] == '\n' || text[i] == '\r') ) {
				in_slash_comment = qfalse;
				break;
			}
		}

		if ( i >= (MAX_CMD_LINE - 1) )
			i = MAX_CMD_LINE - 1;

		memcpy( line, text, i );
		line[i] = '\0';

		// delete the text from the command buffer and move remaining commands down
		// this is necessary because commands (exec) can insert data at the
		// beginning of the text buffer

		if ( i == cmd_text.cursize )
			cmd_text.cursize = 0;
		else {
			i++;
			cmd_text.cursize -= i;
			// skip all repeating newlines/semicolons
			while ( ( text[i] == '\n' || text[i] == '\r' || text[i] == ';' ) && cmd_text.cursize > 0 ) {
				cmd_text.cursize--;
				i++;
			}
			memmove( text, text+i, cmd_text.cursize );
		}

		// execute the command line
		Cmd_ExecuteText( line );
	}
}

typedef void (*cmdfunc_t)(void);
typedef struct cmd_s
{
    char *name;
	char *helpString;
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
        if (N_streq(cmd->name, name)) {
            return cmd;
		}
    }
    return NULL;
}

void Cmd_TokenizeString2(const char *str, qboolean ignoreQuotes)
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
		// handle quoted strings
		if (!ignoreQuotes && *p == '"') {
			cmd_argv[cmd_argc.load()] = tok;
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
		cmd_argv[cmd_argc.load()] = tok;
		cmd_argc.fetch_add(1);

		// skip until whitespace, quote, or command
		while (*p > ' ') {
			if (!ignoreQuotes && p[0] == '"') {
				break;
			}

			if (p[0] == '/' && p[1] == '/') {
				// accept protocol headers (e.g. http://) in command lines that match "*?[a-z]://" pattern
				if (p < cmd_cmd + 3 || p[-1] != ':' || p[-2] < 'a' || p[-2] > 'z') {
					break;
				}
			}

			// skip /* */ comments
			if (p[0] == '/' && p[1] == '*') {
				break;
			}

			*tok++ = *p++;
		}
		*tok++ = '\0';
		if (!*p) {
			return; // end of string
		}
	}
}

qboolean Cmd_CompleteArgument(const char *command, const char *args, uint32_t argnum)
{
	const cmd_t *cmd;

	for (cmd = cmd_functions; cmd; cmd = cmd->next) {
		if (!N_stricmp(command, cmd->name)) {
			if (cmd->complete)
				cmd->complete(args, argnum);

			return qtrue;
		}
	}
}

void Cmd_TokenizeStringIgnoreQuotes(const char *text_p)
{
	Cmd_TokenizeString2(text_p, qtrue);
}

void Cmd_TokenizeString(const char *text_p)
{
	Cmd_TokenizeString2(text_p, qfalse);
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

void Cmd_SetHelpString(const char *name, const char *help)
{
	cmd_t *cmd;
	if (help && help[0] != '\0') {
		cmd = Cmd_FindCommand(name);
		if (!cmd)
			return;
		if (cmd->helpString)
			Z_Free(cmd->helpString);
		
		cmd->helpString = Z_Strdup(help);
	}
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
    cmd->name = Z_Strdup(name);
    cmd->function = func;
	cmd->complete = NULL;
    cmd->next = cmd_functions;
    cmd_functions = cmd;
	numCommands++;
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
        if (!N_stricmp(cmd->name, name)) {
            *back = cmd->next;
            if (cmd->name)
                Z_Free(cmd->name);
			
            numCommands--;
            Z_Free(cmd);
            return;
        }
        back = &cmd->next;
    }
}

void Cmd_SetCommandCompletionFunc(const char *name, completionFunc_t fn)
{
	boost::unique_lock<boost::mutex> lock{cmdLock};
	cmd_t *cmd;

	for (cmd = cmd_functions; cmd; cmd = cmd->next) {
		if (!N_stricmp(name, cmd->name)) {
			cmd->complete = fn;
			return;
		}
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
        Con_Printf("%s", cmd->name);
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
	char *buffer;
	FS_LoadFile(path, (void **)&buffer);

	Cbuf_InsertText(buffer);
	Cbuf_Execute();
}

void Cmd_Init(void)
{
    Cmd_AddCommand("cmdlist", Cmd_List_f);
    Cmd_AddCommand("echo", Cmd_Echo_f);
	Cmd_AddCommand("exec", Cmd_Exec_f);
	Cmd_AddCommand("start", Cmd_Exec_f);
	Cmd_AddCommand("run", Cmd_Exec_f);
	Cmd_AddCommand("execq", Cmd_Exec_f);
//	Cmd_SetCommandCompletionFunc("exec", Cmd_CompleteCfgName);

	cmd_text.cursize = 0;
	cmd_text.maxsize = MAX_CMD_BUFFER;
	cmd_text.data = NULL;
}
