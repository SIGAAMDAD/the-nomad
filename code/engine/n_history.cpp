#include "n_shared.h"
#include "n_common.h"
#include "n_cvar.h"
#include <ctype.h>

void Field_Clear( field_t *edit )
{
	memset( edit->buffer, 0, sizeof( edit->buffer ) );
	edit->cursor = 0;
	edit->scroll = 0;
}

static const char *completionString;
static char shortestMatch[MAX_TOKEN_CHARS];
static uint64_t matchCount;
// field we are working on, passed to Field_AutoComplete(&con_field for instance)
static field_t *completionField;

static void FindMatches( const char *s )
{
	uint64_t i, n;

	if ( N_stricmpn( s, completionString, strlen( completionString ) ) ) {
		return;
	}
	matchCount++;
	if ( matchCount == 1 ) {
		N_strncpyz( shortestMatch, s, sizeof( shortestMatch ) );
		return;
	}

	n = strlen(s);
	// cut shortestMatch to the amount common with s
	for ( i = 0 ; shortestMatch[i] ; i++ ) {
		if ( i >= n ) {
			shortestMatch[i] = '\0';
			break;
		}

		if ( tolower(shortestMatch[i]) != tolower(s[i]) ) {
			shortestMatch[i] = '\0';
		}
	}
}

static void PrintMatches( const char *s )
{
	if ( !N_stricmpn( s, shortestMatch, strlen( shortestMatch ) ) ) {
		Con_Printf( "    %s\n", s );
	}
}

static void PrintCvarMatches( const char *s )
{
	char value[ TRUNCATE_LENGTH ];

	if ( !N_stricmpn( s, shortestMatch, strlen( shortestMatch ) ) ) {
		Com_TruncateLongString( value, Cvar_VariableString( s ) );
		Con_Printf( "    %s = \"%s\"\n", s, value );
	}
}

static const char *Field_FindFirstSeparator( const char *s )
{
	char c;
	while ( (c = *s) != '\0' ) {
		if ( c == ';' )
			return s;
		s++;
	}
	return NULL;
}

static void Field_AddSpace( void )
{
	uint64_t len = strlen( completionField->buffer );
	if ( len && len < sizeof( completionField->buffer ) - 1 && completionField->buffer[ len - 1 ] != ' ' ) {
		memcpy( completionField->buffer + len, " ", 2 );
		completionField->cursor = (int)(len + 1);
	}
}

static qboolean Field_Complete( void )
{
	uint64_t completionOffset;

	if( matchCount == 0 )
		return qtrue;

	completionOffset = strlen( completionField->buffer ) - strlen( completionString );

	N_strncpyz( &completionField->buffer[ completionOffset ], shortestMatch,
		sizeof( completionField->buffer ) - completionOffset );

	completionField->cursor = strlen( completionField->buffer );

	if( matchCount == 1 ) {
		Field_AddSpace();
		return qtrue;
	}

	Con_Printf( "]%s\n", completionField->buffer );

	return qfalse;
}


void Field_CompleteKeyname( void )
{
	matchCount = 0;
	shortestMatch[ 0 ] = '\0';

	Key_KeynameCompletion( FindMatches );

	if ( !Field_Complete() )
		Key_KeynameCompletion( PrintMatches );
}

void Field_CompleteKeyBind( uint32_t key )
{
	const char *value;
	uint64_t vlen;
	uint64_t blen;

	value = Key_GetBinding( key );
	if ( value == NULL || *value == '\0' )
		return;

	blen = strlen( completionField->buffer );
	vlen = strlen( value );

	if ( Field_FindFirstSeparator( (char *)value ) ) {
		value = va( "\"%s\"", value );
		vlen += 2;
	}

	if ( vlen + blen > sizeof( completionField->buffer ) - 1 )
		return;

	memcpy( completionField->buffer + blen, value, vlen + 1 );
	completionField->cursor = blen + vlen;

	Field_AddSpace();
}

static void Field_CompleteCvarValue(const char *value, const char *current)
{
    uint64_t vlen, blen;

    if (!*value)
        return;
    
    blen = strlen(completionField->buffer);
    vlen = strlen(value);

    if (*current)
        return;
    
    if (Field_FindFirstSeparator((char *)value)) {
        value = va("\"%s\"", value);
        vlen += 2;
    }

    if (vlen + blen > sizeof(completionField->buffer) - 1)
        return;
    
    if (blen > 1) {
        if (completionField->buffer[blen - 1] == '"' && completionField->buffer[blen - 2] == ' ') {
            completionField->buffer[blen--] = '\0'; // strip starting quote
        }
    }
    
    memcpy(completionField->buffer + blen, value, vlen + 1);
    completionField->cursor = vlen + blen;

    Field_AddSpace();
}

void Field_CompleteFilename(const char *dir, const char *ext, qboolean stripExt, int flags)
{
    matchCount = 0;
    shortestMatch[0] = '\0';

    FS_FilenameCompletion(dir, ext, stripExt, FindMatches, flags);

    if (!Field_Complete())
        FS_FilenameCompletion(dir, ext, stripExt, PrintMatches, flags);
}

void Field_CompleteCommand(const char *cmd, qboolean doCommands, qboolean doCvars)
{
    uint32_t completionArgument;

    // skip leading whitespace and quotes
    cmd = Com_SkipCharset(cmd, " \"");

    Cmd_TokenizeStringIgnoreQuotes(cmd);
    completionArgument = Cmd_Argc();

    // if there is trailing whitespace on the cmd
    if (*(cmd + strlen(cmd) - 1) == ' ') {
        completionString = "";
        completionArgument++;
    }
    else
        completionString = Cmd_Argv(completionArgument - 1);
    
    // unconditionally add a '\' to the start of the buffer
    if (completionField->buffer[0] && completionField->buffer[0] != '\\') {
        if (completionField->buffer[0] != '/') {
            // buffer is full, refuse to complete
            if (strlen(completionField->buffer) + 1 >= sizeof(completionField->buffer))
                return;
            
            memmove(&completionField->buffer[1], &completionField->buffer[0], strlen(completionField->buffer + 1));
            completionField->cursor++;
        }
        completionField->buffer[0] = '/';
    }
    
    if (completionArgument > 1) {
        const char *baseCmd = Cmd_Argv(0);
        const char *p;

        // this should always be true
        if (baseCmd[0] == '\\' || baseCmd[0] == '/')
            baseCmd++;
        
		Field_Complete();
        if ((p = Field_FindFirstSeparator(cmd)) != NULL)
            Field_CompleteCommand(p + 1, qtrue, qtrue); // compound command
        else {
            qboolean argumentCompleted = Cmd_CompleteArgument(baseCmd, cmd, completionArgument);
            if ((matchCount == 1 || argumentCompleted) && doCvars) {
                if (cmd[0] == '/' || cmd[0] == '\\')
                    cmd++;
                
                Cmd_TokenizeString(cmd);
                Field_CompleteCvarValue(Cvar_VariableString(Cmd_Argv(0)), Cmd_Argv(1));
            }
        }
    }
    else {
        if (completionString[0] == '\\' || completionString[0] == '/')
            completionString++;
        
        matchCount = 0;
        shortestMatch[0] = '\0';
        
        if (completionString[0] == '\0')
            return;
        
        if (doCommands)
            Cmd_CommandCompletion(FindMatches);
        if (doCvars)
            Cvar_CommandCompletion(FindMatches);
        
        if (!Field_Complete()) {
            // run through again, printing matches
            if (doCommands)
                Cmd_CommandCompletion(PrintMatches);
            if (doCvars)
                Cvar_CommandCompletion(PrintMatches);
		}
    }
}


#define CONSOLE_HISTORY_FILE LOG_DIR "/history.txt"
/*
Field_AutoComplete: perform tab expansion
*/
void Field_AutoComplete(field_t *field)
{
    completionField = field;
    Field_CompleteCommand(completionField->buffer, qtrue, qtrue);
}

static qboolean historyLoaded = qfalse;

#define COMMAND_HISTORY 32

static field_t historyEditLines[COMMAND_HISTORY];
static uint64_t nextHistoryLine; // the last line in the history buffer, not masked
static uint64_t historyLine; // the line being displayed from the history buffer, will be <= nextHistoryLine

#define MAX_CONSOLE_SAVE_BUFFER (COMMAND_HISTORY * (MAX_EDIT_LINE + 13))

static void Con_LoadHistory(void);
static void Con_SaveHistory(void);

void Con_ResetHistory(void)
{
    historyLoaded = qfalse;
    nextHistoryLine = 0;
    historyLine = 0;
}

void Con_SaveField(const field_t *field)
{
    const field_t *h;

    if (!field || !field->buffer[0])
        return;
    
    if (historyLoaded == qfalse) {
        historyLoaded = qtrue;
        Con_LoadHistory();
    }

    // try to avoid inserting duplicates
    if (nextHistoryLine > 0) {
        h = &historyEditLines[(nextHistoryLine - 1) % COMMAND_HISTORY];
        if (field->cursor == h->cursor && field->scroll == h->scroll && !strcmp(field->buffer, h->buffer)) {
            historyLine = nextHistoryLine;
            return;
        }
    }

    historyEditLines[nextHistoryLine % COMMAND_HISTORY] = *field;
    nextHistoryLine++;
    historyLine = nextHistoryLine;

    Con_SaveHistory();
}

/*
* Con_HistoryGetPrev: returns qtrue if previously returned edit field needs to be updated
*/
qboolean Con_HistoryGetPrev(field_t *field)
{
    qboolean bresult;

    if (historyLoaded == qfalse) {
        historyLoaded = qtrue;
        Con_LoadHistory();
    }

    if (nextHistoryLine - historyLine < COMMAND_HISTORY && historyLine > 0) {
        bresult = qtrue;
        historyLine--;
    }
    else
        bresult = qfalse;

	memcpy( field, &historyEditLines[historyLine % COMMAND_HISTORY], sizeof(*field) );

    return bresult;
}

/*
* Con_HistoryGetNext: returns qtrue if previously returned edit field needs to be updated
*/
qboolean Con_HistoryGetNext( field_t *field )
{
	qboolean bresult;

	if ( historyLoaded == qfalse ) {
		historyLoaded = qtrue;
		Con_LoadHistory();
	}

	historyLine++;

	if ( historyLine >= nextHistoryLine ) {
		if ( historyLine == nextHistoryLine )
			bresult = qtrue;
		else
			bresult = qfalse;
        
		historyLine = nextHistoryLine;
		Field_Clear( field );
		return bresult;
	}

	memcpy( field, &historyEditLines[historyLine % COMMAND_HISTORY], sizeof(*field) );

	return qtrue;
}

static void Con_LoadHistory( void )
{
	char consoleSaveBuffer[ MAX_CONSOLE_SAVE_BUFFER ];
	uint64_t  consoleSaveBufferSize;
	const char *token, *text_p;
	uint64_t i, numChars, numLines = 0;
	field_t *edit;
	fileHandle_t f;

	for ( i = 0 ; i < COMMAND_HISTORY ; i++ ) {
		Field_Clear( &historyEditLines[i] );
	}

    f = FS_FOpenRead(CONSOLE_HISTORY_FILE);
	if ( f == FS_INVALID_HANDLE )
	{
		Con_Printf( "Couldn't read %s.\n", CONSOLE_HISTORY_FILE );
		return;
	}
    consoleSaveBufferSize = FS_FileLength( f );

	if ( consoleSaveBufferSize < MAX_CONSOLE_SAVE_BUFFER &&
	        FS_Read( consoleSaveBuffer, consoleSaveBufferSize, f ) == consoleSaveBufferSize ) {
		consoleSaveBuffer[ consoleSaveBufferSize ] = '\0';
		text_p = consoleSaveBuffer;

		for ( i = COMMAND_HISTORY - 1; i >= 0; i-- ) {
        	if ( !*( token = COM_Parse( &text_p ) ) )
				break;

			edit = &historyEditLines[ i ];

			edit->cursor = atoi( token );

			if ( !*( token = COM_Parse( &text_p ) ) )
				break;

			edit->scroll = atoi( token );

			if( !*( token = COM_Parse( &text_p ) ) )
				break;

			numChars = atoi( token );
			text_p++;
			if ( numChars > ( consoleSaveBufferSize - ( text_p - consoleSaveBuffer ) ) || numChars >= sizeof( edit->buffer ) ) {
				Con_DPrintf( "WARNING: probable corrupt history\n" );
				break;
			}

			if ( edit->cursor > sizeof( edit->buffer ) - 1 )
				edit->cursor = sizeof( edit->buffer ) - 1;
			else if ( edit->cursor < 0 )
				edit->cursor = 0;

			if ( edit->scroll > edit->cursor )
				edit->scroll = edit->cursor;
			else if ( edit->scroll < 0 )
				edit->scroll = 0;

			memcpy( edit->buffer, text_p, numChars );
			edit->buffer[ numChars ] = '\0';
			text_p += numChars;

			numLines++;
		}

		memmove( &historyEditLines[ 0 ], &historyEditLines[ i + 1 ],
				numLines * sizeof( field_t ) );
		for ( i = numLines; i < COMMAND_HISTORY; i++ )
			Field_Clear( &historyEditLines[ i ] );

		historyLine = nextHistoryLine = numLines;
	}
	else
		Con_Printf( "Couldn't read %s.\n", CONSOLE_HISTORY_FILE );

	FS_FClose( f );
}

static void Con_SaveHistory( void )
{
	char            consoleSaveBuffer[ MAX_CONSOLE_SAVE_BUFFER ];
	uint64_t        consoleSaveBufferSize;
	uint64_t        i;
	uint64_t        lineLength, saveBufferLength, additionalLength;
	fileHandle_t          f;

	consoleSaveBuffer[ 0 ] = '\0';

	i = ( nextHistoryLine - 1 + COMMAND_HISTORY ) % COMMAND_HISTORY;
	do {
		if ( historyEditLines[ i ].buffer[ 0 ] ) {
			lineLength = strlen( historyEditLines[ i ].buffer );
			saveBufferLength = strlen( consoleSaveBuffer );

			//ICK
			additionalLength = lineLength + 13; // strlen( "999 999 999  " )

			if ( saveBufferLength + additionalLength < MAX_CONSOLE_SAVE_BUFFER ) {
				N_strcat( consoleSaveBuffer, MAX_CONSOLE_SAVE_BUFFER,
						va( "%i %i %lu %s ",
						historyEditLines[ i ].cursor,
						historyEditLines[ i ].scroll,
						lineLength,
						historyEditLines[ i ].buffer ) );
			}
			else
				break;
		}
		i = ( i - 1 + COMMAND_HISTORY ) % COMMAND_HISTORY;
	}
	while ( i != ( nextHistoryLine - 1 + COMMAND_HISTORY ) % COMMAND_HISTORY );

	consoleSaveBufferSize = strlen( consoleSaveBuffer );

	f = FS_FOpenWrite( CONSOLE_HISTORY_FILE );
	if ( f == FS_INVALID_HANDLE ) {
		Con_Printf( "Couldn't write %s.\n", CONSOLE_HISTORY_FILE );
		return;
	}

	if ( FS_Write( consoleSaveBuffer, consoleSaveBufferSize, f ) < consoleSaveBufferSize )
		Con_Printf( "Couldn't write %s.\n", CONSOLE_HISTORY_FILE );

	FS_FClose( f );
}

/*
* Con_PrintHistory_f: prints console history just like bash history
*/
void Con_PrintHistory_f( void )
{
	field_t field;
	int64_t numFields, i;

	if ( !historyLoaded ) {
		Con_LoadHistory();
	}

	for ( numFields = historyLine; numFields >= 0; numFields--, i++ ) {
		Con_Printf( "%-4li  %s\n", i, historyEditLines[ numFields ].buffer );
	}
}
