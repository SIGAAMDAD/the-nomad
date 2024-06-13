#include "n_shared.h"
#include "n_common.h"
#include "../game/g_game.h"
#include "n_cvar.h"
#include "n_threads.h"


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

class CCommandBuffer
{
public:
	CCommandBuffer( void );
	~CCommandBuffer();

	void Init( void );
	void AddText( const char *text );
private:
	int64_t m_nMaxSize;
	int64_t m_nCurSize;
	eastl::vector<byte> m_DataBuffer;
};

cvar_t *com_MaxCommandBufferSize;

CCommandBuffer::CCommandBuffer( void )
{
	m_DataBuffer.reserve( MAX_CMD_BUFFER );
}

CCommandBuffer::~CCommandBuffer()
{
}

void CCommandBuffer::Init( void )
{
	memset( m_DataBuffer.data(), 0, m_DataBuffer.size() );
	m_nMaxSize = MAX_CMD_BUFFER;
	m_nCurSize = 0;
}

typedef struct {
	byte *data;
	int64_t maxsize;
	int64_t cursize;
} cbuf_t;

static int32_t cmd_wait;
static cbuf_t cmd_text;
static byte  cmd_text_buf[MAX_CMD_BUFFER];


//=============================================================================

/*
============
Cmd_Wait_f

Causes execution of the remainder of the command buffer to be delayed until
next frame.  This allows commands like:
bind g "cmd use rocket ; +attack ; wait ; -attack ; cmd use blaster"
============
*/
static void Cmd_Wait_f( void )
{
	if ( Cmd_Argc() == 2 ) {
		cmd_wait = atoi( Cmd_Argv( 1 ) );
		if ( cmd_wait < 0 )
			cmd_wait = 1; // ignore the argument
	} else {
		cmd_wait = 1;
	}
}


/*
=============================================================================

						COMMAND BUFFER

=============================================================================
*/

/*
============
Cbuf_Init
============
*/
void Cbuf_Init( void )
{
	memset( cmd_text_buf, 0, sizeof( cmd_text_buf ) );
	cmd_text.data = cmd_text_buf;
	cmd_text.maxsize = MAX_CMD_BUFFER;
	cmd_text.cursize = 0;
}


/*
============
Cbuf_AddText

Adds command text at the end of the buffer, does NOT add a final \n
============
*/
void Cbuf_AddText( const char *text )
{
	const uint64_t l = strlen( text );

	if ( cmd_text.cursize + l >= cmd_text.maxsize ) {
		Con_Printf ( "Cbuf_AddText: overflow\n" );
		return;
	}

	memcpy( &cmd_text.data[cmd_text.cursize], text, l );
	cmd_text.cursize += l;
}


/*
============
Cbuf_Add

// Adds command text at the specified position of the buffer, adds \n when needed
============
*/
uint64_t Cbuf_Add( const char *text, uint64_t pos )
{
	uint64_t len;
	qboolean separate = qfalse;
	int64_t i;

	len = strlen( text );

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
		Con_Printf( COLOR_YELLOW "%s(%lu) overflowed\n", __func__, pos );
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


/*
============
Cbuf_InsertText

Adds command text immediately after the current command
Adds a \n to the text
============
*/
void Cbuf_InsertText( const char *text )
{
	uint64_t len;
	int64_t i;

	len = strlen( text ) + 1;

	if ( len + cmd_text.cursize > cmd_text.maxsize ) {
		Con_Printf( COLOR_YELLOW "Cbuf_InsertText overflowed\n" );
		return;
	}

	// move the existing command text
	// [glnomad] changed to memmove, much faster with hardware/SIMD
#if 1
	if ( cmd_text.cursize ) {
		memmove( &cmd_text.data[cmd_text.cursize - 1 + len], &cmd_text.data[cmd_text.cursize - 1], cmd_text.cursize - 1 );
	}
#else
	for ( i = cmd_text.cursize - 1 ; i >= 0 ; i-- ) {
		cmd_text.data[ i + len ] = cmd_text.data[ i ];
	}
#endif

	// copy the new text in
	memcpy( cmd_text.data, text, len - 1 );

	// add a \n
	cmd_text.data[ len - 1 ] = '\n';

	cmd_text.cursize += len;
}


/*
============
Cbuf_ExecuteText
============
*/
void Cbuf_ExecuteText( cbufExec_t exec_when, const char *text )
{
	switch ( exec_when ) {
	case EXEC_NOW:
		if ( text && text[0] != '\0' ) {
			Con_DPrintf( "EXEC_NOW %s\n", text );
			Cmd_ExecuteString( text );
		}
		else {
			Cbuf_Execute();
			Con_DPrintf( "EXEC_NOW %s\n", cmd_text.data );
		}
		break;
	case EXEC_INSERT:
		Cbuf_InsertText( text );
		break;
	case EXEC_APPEND:
		Cbuf_AddText( text );
		break;
	default:
		N_Error ( ERR_FATAL, "Cbuf_ExecuteText: bad exec_when");
	}
}


/*
============
Cbuf_Wait
============
*/
qboolean Cbuf_Wait( void )
{
	return ( cmd_wait > 0 ) ? qtrue : qfalse;
}

/*
============
Cbuf_Execute
============
*/
void Cbuf_Execute( void )
{
	int32_t i;
	char *text;
	char line[MAX_CMD_LINE];
	int32_t quotes;

	// This will keep // style comments all on one line by not breaking on
	// a semicolon.  It will keep /* ... */ style comments all on one line by not
	// breaking it for semicolon or newline.
	qboolean in_star_comment = qfalse;
	qboolean in_slash_comment = qfalse;
	while ( cmd_text.cursize > 0 ) {
		if ( cmd_wait > 0 ) {
			// skip out while text still remains in buffer, leaving it
			// for next frame
			cmd_wait--;
			break;
		}

		// find a \n or ; line break or comment: // or /* */
		text = (char *)cmd_text.data;

		quotes = 0;
		for ( i = 0 ; i< cmd_text.cursize ; i++ ) {
			if ( text[i] == '"' ) {
				quotes++;
			}

			if ( !(quotes & 1 ) ) {
				if ( i < cmd_text.cursize - 1 ) {
					if ( !in_star_comment && text[i] == '/' && text[i+1] == '/' ) {
						in_slash_comment = qtrue;
					} else if ( !in_slash_comment && text[i] == '/' && text[i+1] == '*' ) {
						in_star_comment = qtrue;
					} else if ( in_star_comment && text[i] == '*' && text[i+1] == '/' ) {
						in_star_comment = qfalse;
						// If we are in a star comment, then the part after it is valid
						// Note: This will cause it to NUL out the terminating '/'
						// but ExecuteString doesn't require it anyway.
						i++;
						break;
					}
				}
				if ( !in_slash_comment && !in_star_comment && text[i] == ';' ) {
					break;
				}
			}
			if ( !in_star_comment && (text[i] == '\n' || text[i] == '\r' ) ) {
				in_slash_comment = qfalse;
				break;
			}
		}

		if ( i >= ( MAX_CMD_LINE - 1 ) ) {
			i = MAX_CMD_LINE - 1;
		}

		memcpy( line, text, i );
		line[i] = '\0';

		// delete the text from the command buffer and move remaining commands down
		// this is necessary because commands (exec) can insert data at the
		// beginning of the text buffer

		if ( i == cmd_text.cursize ) {
			cmd_text.cursize = 0;
		} else {
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
		Cmd_ExecuteString( line );
	}
}

/*
==============================================================================

						SCRIPT COMMANDS

==============================================================================
*/


/*
===============
Cmd_Exec_f
===============
*/
static void Cmd_Exec_f( void )
{
	union {
		char *c;
		void *v;
	} f;
	qboolean quiet;
	char filename[MAX_GDR_PATH];

	quiet = (qboolean)!N_stricmp( Cmd_Argv( 0 ), "execq" );

	if (Cmd_Argc() != 2) {
		Con_Printf( "exec%s <filename> : execute a script file%s\n",
			quiet ? "q" : "", quiet ? " without notification" : "" );
		return;
	}

	N_strncpyz( filename, Cmd_Argv( 1 ), sizeof( filename ) );
	COM_DefaultExtension( filename, sizeof( filename ), ".cfg" );
	FS_LoadFile( filename, &f.v );
	if ( f.v == NULL ) {
		Con_Printf( "couldn't exec %s\n", filename );
		return;
	}
	if ( !quiet ) {
		Con_Printf ( "execing %s\n", filename );
	}

	Cbuf_InsertText( f.c );
	FS_FreeFile( f.v );
}

static void Cmd_ExecuteBuffer_f(void) {
	Cbuf_Execute();
}

/*
===============
Cmd_Vstr_f

Inserts the current value of a variable as command text
===============
*/
static void Cmd_Vstr_f( void )
{
	const char *v;

	if ( Cmd_Argc() != 2 ) {
		Con_Printf( "vstr <variablename> : execute a variable command\n" );
		return;
	}

	v = Cvar_VariableString( Cmd_Argv( 1 ) );
	Cbuf_InsertText( v );
}


/*
===============
Cmd_Echo_f

Just prints the rest of the line to the console
===============
*/
static void Cmd_Echo_f( void ) {
	Con_Printf( "%s\n", Cmd_ArgsFrom( 1 ) );
}


typedef void (*cmdfunc_t)( void );
typedef struct cmd_s
{
    char *name;
	char *helpString;
    cmdfunc_t function;
	completionFunc_t complete;
    struct cmd_s* next;
} cmd_t;

static CThreadMutex cmdLock;
static cmd_t *cmd_functions;
static uint32_t cmd_argc;
static char cmd_tokenized[BIG_INFO_STRING+MAX_STRING_TOKENS];
static char *cmd_argv[MAX_STRING_TOKENS];
static char cmd_cmd[BIG_INFO_STRING];

//static char cmd_history[MAX_HISTORY][BIG_INFO_STRING];
static uint32_t cmd_historyused;

uint32_t Cmd_Argc( void ) {
	return cmd_argc;
}

void Cmd_Clear( void ) {
	CThreadAutoLock<CThreadMutex> lock( cmdLock );

	cmd_argc = 0;

	// exec command breaks if this isn't done
	memset( cmd_cmd, 0, sizeof(cmd_cmd) );
	memset( cmd_tokenized, 0, sizeof(cmd_tokenized) );
}

const char *Cmd_Argv( uint32_t index ) {
    if ( index >= cmd_argc ) {
        return "";
    }
    return cmd_argv[index];
}

/*
* Cmd_ArgvBuffer: The interpreted versions use this because they can't have
* pointers returned to them
*/
void Cmd_ArgvBuffer( uint32_t arg, char *buffer, uint32_t bufLen ) {
	N_strncpyz( buffer, Cmd_Argv(arg), bufLen );
}

/*
Cmd_ArgsBuffer: The interpreted versions use this because they can't have
pointers returned to them
*/
void Cmd_ArgsBuffer( char *buffer, uint32_t bufferLength ) {
	N_strncpyz( buffer, Cmd_ArgsFrom( 1 ), bufferLength );
}


/*
============
Cmd_Args

Returns a single string containing argv(arg) to argv(argc()-1)
============
*/
char *Cmd_ArgsFrom( uint32_t arg ) {
	static char cmd_args[BIG_INFO_STRING], *s;
	uint32_t i;

	s = cmd_args;
	*s = '\0';
	if ( arg < 0 ) {
		arg = 0;
	}
	for ( i = arg; i < cmd_argc; i++ ) {
		s = N_stradd( s, cmd_argv[i] );
		if ( i != cmd_argc-1 ) {
			s = N_stradd( s, " " );
		}
	}

	return cmd_args;
}

static cmd_t* Cmd_FindCommand( const char *name )
{
	CThreadAutoLock<CThreadMutex> lock( cmdLock );
    for ( cmd_t *cmd = cmd_functions; cmd; cmd = cmd->next ) {
        if ( !N_stricmp( name, cmd->name ) ) {
            return cmd;
		}
    }
    return NULL;
}

/*
   Replace command separators with space to prevent interpretation
   This is a hack to protect buggy qvms
   https://bugzilla.icculus.org/show_bug.cgi?id=3593
   https://bugzilla.icculus.org/show_bug.cgi?id=4769
*/
void Cmd_Args_Sanitize( const char *separators )
{
	uint32_t i;

	for ( i = 1; i < cmd_argc; i++ ) {
		char *c = cmd_argv[i];

		while ( ( c = strpbrk( c, separators ) ) != NULL ) {
			*c = ' ';
			++c;
		}
	}
}


/*
============
Cmd_TokenizeString

Parses the given string into command line tokens.
The text is copied to a separate buffer and 0 characters
are inserted in the appropriate place, The argv array
will point into this temporary buffer.
============
*/
// NOTE TTimo define that to track tokenization issues
//#define TKN_DBG
static void Cmd_TokenizeString2( const char *text_in, qboolean ignoreQuotes ) {
	const char *text;
	char *textOut;

	CThreadAutoLock<CThreadMutex> lock( cmdLock );

#ifdef TKN_DBG
	// FIXME TTimo blunt hook to try to find the tokenization of userinfo
	Con_DPrintf("Cmd_TokenizeString: %s\n", text_in);
#endif
	
	cmd_argc = 0;

	// clear previous args
	cmd_cmd[0] = '\0';

	if ( !text_in ) {
		return;
	}

	N_strncpyz( cmd_cmd, text_in, sizeof( cmd_cmd ) );

	text = cmd_cmd; // read from safe-length buffer
	textOut = cmd_tokenized;

	while ( 1 ) {
		if ( cmd_argc >= arraylen( cmd_argv ) ) {
			return;			// this is usually something malicious
		}

		while ( 1 ) {
			// skip whitespace
			while ( *text && *text <= ' ' ) {
				text++;
			}
			if ( !*text ) {
				return;			// all tokens parsed
			}

			// skip // comments
			if ( text[0] == '/' && text[1] == '/' ) {
				// accept protocol headers (e.g. http://) in command lines that matching "*?[a-z]://" pattern
				if ( text < cmd_cmd + 3 || text[-1] != ':' || text[-2] < 'a' || text[-2] > 'z' ) {
					return; // all tokens parsed
				}
			}

			// skip /* */ comments
			if ( text[0] == '/' && text[1] =='*' ) {
				while ( *text && ( text[0] != '*' || text[1] != '/' ) ) {
					text++;
				}
				if ( !*text ) {
					return;		// all tokens parsed
				}
				text += 2;
			} else {
				break;			// we are ready to parse a token
			}
		}

		// handle quoted strings
		// NOTE TTimo this doesn't handle \" escaping
		if ( !ignoreQuotes && *text == '"' ) {
			cmd_argv[cmd_argc] = textOut;
			cmd_argc++;
			text++;

			while ( *text && *text != '"' ) {
				*textOut++ = *text++;
			}

			*textOut++ = '\0';

			if ( !*text ) {
				return;		// all tokens parsed
			}
			
			text++;
			continue;
		}

		// regular token
		cmd_argv[cmd_argc] = textOut;
		cmd_argc++;

		// skip until whitespace, quote, or command
		while ( *text > ' ' ) {
			if ( !ignoreQuotes && text[0] == '"' ) {
				break;
			}

			if ( text[0] == '/' && text[1] == '/' ) {
				// accept protocol headers (e.g. http://) in command lines that matching "*?[a-z]://" pattern
				if ( text < cmd_cmd + 3 || text[-1] != ':' || text[-2] < 'a' || text[-2] > 'z' ) {
					break;
				}
			}

			// skip /* */ comments
			if ( text[0] == '/' && text[1] =='*' ) {
				break;
			}

			*textOut++ = *text++;
		}

		*textOut++ = '\0';

		if ( !*text ) {
			return;		// all tokens parsed
		}
	}
}


/*
============
Cmd_TokenizeString
============
*/
void Cmd_TokenizeString( const char *text_in ) {
	Cmd_TokenizeString2( text_in, qfalse );
}


/*
============
Cmd_TokenizeStringIgnoreQuotes
============
*/
void Cmd_TokenizeStringIgnoreQuotes( const char *text_in ) {
	Cmd_TokenizeString2( text_in, qtrue );
}


void Cmd_SetHelpString( const char *name, const char *help )
{
	cmd_t *cmd;

	if (help && help[0] != '\0') {
		cmd = Cmd_FindCommand(name);
		if (!cmd)
			return;
		if (cmd->helpString)
			Z_Free(cmd->helpString);
		
		cmd->helpString = CopyString(help);
	}
}

void Cmd_AddCommand( const char *name, cmdfunc_t func )
{
    cmd_t* cmd;
	
    if ( Cmd_FindCommand( name ))  {
        if ( func ) {
            Con_Printf( "Cmd_AddCommand: %s already defined\n", name );
		}
        return;
    }
    Con_DPrintf( "Registered command %s\n", name );

    cmd = (cmd_t *)S_Malloc( sizeof(*cmd) );
    cmd->name = CopyString( name );
    cmd->function = func;
	cmd->complete = NULL;
    cmd->next = cmd_functions;
    cmd_functions = cmd;
}

void Cmd_RemoveCommand( const char *name )
{
    cmd_t *cmd, **back;

    back = &cmd_functions;
    while ( 1 ) {
        cmd = *back;
        if ( !cmd ) {
            // command wasn't active
            break;
        }
        if ( !N_stricmp( cmd->name, name ) ) {
            *back = cmd->next;
            if ( cmd->name ) {
                Z_Free( cmd->name );
			}
			
            Z_Free( cmd );
            return;
        }
        back = &cmd->next;
    }
}

/*
============
Cmd_RemoveCommandSafe

Only remove commands with no associated function
============
*/
void Cmd_RemoveCommandSafe( const char *cmd_name )
{
	const cmd_t *cmd = Cmd_FindCommand( cmd_name );

	if ( !cmd ) {
		return;
	}
	if ( cmd->function ) {
		N_Error( ERR_DROP, "Restricted source tried to remove "
			"system command \"%s\"", cmd_name );
		return;
	}

	Cmd_RemoveCommand( cmd_name );
}


/*
============
Cmd_RemoveSgameCommands

Remove sgame-created commands
============
*/
void Cmd_RemoveSgameCommands( void )
{
	const cmd_t *cmd;
	qboolean removed;

	do {
		removed = qfalse;
		for ( cmd = cmd_functions ; cmd ; cmd = cmd->next ) {
			if ( cmd->function == NULL ) {
				Cmd_RemoveCommand( cmd->name );
				removed = qtrue;
				break;
			}
		}
	} while ( removed );
}


/*
============
Cmd_CommandCompletion
============
*/
void Cmd_CommandCompletion( void (*callback)( const char *s ) ) {
	const cmd_t *cmd;

	for ( cmd = cmd_functions ; cmd ; cmd=cmd->next ) {
		callback( cmd->name );
	}
}


/*
============
Cmd_CompleteArgument
============
*/
qboolean Cmd_CompleteArgument( const char *command, const char *args, uint32_t argNum ) {
	const cmd_t *cmd;

	for( cmd = cmd_functions; cmd; cmd = cmd->next ) {
		if ( !N_stricmp( command, cmd->name ) ) {
			if ( cmd->complete ) {
				cmd->complete( args, argNum );
			}
			return qtrue;
		}
	}

	return qfalse;
}


/*
============
Cmd_ExecuteString

A complete command line has been parsed, so try to execute it
============
*/
void Cmd_ExecuteString( const char *text ) {
	cmd_t *cmd, **prev;

	// execute the command line
	Cmd_TokenizeString( text );
	if ( !Cmd_Argc() ) {
		return;		// no tokens
	}

	// check registered command functions
	for ( prev = &cmd_functions ; *prev ; prev = &cmd->next ) {
		cmd = *prev;
		if ( !N_stricmp( cmd_argv[0], cmd->name ) ) {
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
	if ( Cvar_Command() ) {
		return;
	}

	// check sgame commands
	if ( G_SGameCommand() ) {
		return;
	}

	// check ui commands
//	if ( UI_GameCommand() ) {
//		return;
//	}
}


/*
============
Cmd_List_f
============
*/
static void Cmd_List_f( void )
{
	const cmd_t *cmd;
	const char *match;
	uint32_t i;

	if ( Cmd_Argc() > 1 ) {
		match = Cmd_Argv( 1 );
	} else {
		match = NULL;
	}

	i = 0;
	for ( cmd = cmd_functions; cmd; cmd = cmd->next ) {
		if ( match && !Com_Filter( match, cmd->name ) ) {
			continue;
		}
		Con_Printf( "%s\n", cmd->name );
		i++;
	}
	Con_Printf( "%i commands\n", i );
}


/*
==================
Cmd_CompleteCfgName
==================
*/
static void Cmd_CompleteCfgName( const char *args, uint32_t argNum ) {
	if( argNum == 2 ) {
		Field_CompleteFilename( "", "cfg", qfalse, FS_MATCH_ANY );
	}
}


/*
==================
Cmd_CompleteWriteCfgName
==================
*/
void Cmd_CompleteWriteCfgName( const char *args, uint32_t argNum ) {
	if( argNum == 2 ) {
		Field_CompleteFilename( "", "cfg", qfalse, FS_MATCH_EXTERN );
	}
}

/*
============
Cmd_SetCommandCompletionFunc
============
*/
void Cmd_SetCommandCompletionFunc( const char *command, completionFunc_t complete ) {
	cmd_t *cmd;

	for ( cmd = cmd_functions; cmd; cmd = cmd->next ) {
		if ( !N_stricmp( command, cmd->name ) ) {
			cmd->complete = complete;
			return;
		}
	}
}

void Cmd_Init(void)
{
	Cbuf_Init();

	Cmd_AddCommand( "flushbuf", Cmd_ExecuteBuffer_f );
    Cmd_AddCommand( "cmdlist", Cmd_List_f );
    Cmd_AddCommand( "echo", Cmd_Echo_f );
	Cmd_AddCommand( "start", Cmd_Exec_f );
	Cmd_AddCommand( "run", Cmd_Exec_f );
	Cmd_AddCommand( "wait", Cmd_Wait_f );
	Cmd_AddCommand( "vstr", Cmd_Vstr_f );
	Cmd_SetCommandCompletionFunc( "vstr", Cvar_CompleteCvarName );
	Cmd_AddCommand( "exec", Cmd_Exec_f );
	Cmd_AddCommand( "execq", Cmd_Exec_f );
	Cmd_SetCommandCompletionFunc( "exec", Cmd_CompleteCfgName );
	Cmd_SetCommandCompletionFunc( "execq", Cmd_CompleteCfgName );
}
