#include "../engine/n_shared.h"
#include "g_game.h"
#define STB_RECT_PACK_IMPLEMENTATION
#include "../rendercommon/imstb_rectpack.h"
#define STB_TRUETYPE_IMPLEMENTATION
#include "../rendercommon/imstb_truetype.h"
#include "../rendercommon/imgui.h"
#include "../rendercommon/r_public.h"
#include "../rendercommon/imgui_impl_opengl3.h"
#include "../rendercommon/imgui_impl_sdlrenderer2.h"
#include "../rendercommon/imgui_impl_sdl2.h"
#include "../engine/n_threads.h"

/*
typedef struct {
	char m_szText[MAXPRINTMSG];	// text buffer
	uint32_t m_nDeltaTime;		// how long has it been around?
	uint32_t m_nLifeTime;		// con_maxNotifyTime->i, once delta time is greater, DIE
} notify_t;

class CDevConsole
{
public:
	CDevConsole( void );
	~CDevConsole();

	void AddText( const char *pText );
	void Draw( void );
	void Run( void );
private:
	void DrawText( const char *pText );
	void DrawNotify( void );
	void DrawSolidConsole( void ) const;
	void DrawInput( void ) const;

	vec4_t m_Color;

	char *m_pBuffer;
	uint64_t m_nBufUsed;

	notify_t *m_pNotify;
	uint64_t m_nNofityCount;

	float m_nDisplayFrac;
	float m_nFinalFrac;
	qboolean m_bInitialized;
};

static CDevConsole s_devConsole;
cvar_t *con_maxTextSize;
cvar_t *con_textShuffle;


CDevConsole::CDevConsole( void ) {
}

CDevConsole::~CDevConsole() {
}

void CDevConsole::Draw( void )
{

}

void CDevConsole::AddText( const char *pText )
{
	uint64_t len;

	len = strlen( pText );

	if ( len + m_nBufUsed >= con_maxTextSize->i ) {
		if ( con_textShuffle->i == 0 ) {
			memset( m_pBuffer, 0, con_maxTextSize->i );
		} else {
			memmove( m_pBuffer, m_pBuffer + m_nBufUsed );
		}
	}
}

void CDevConsole::DrawText( const char *pText )
{
	uint64_t len, i;
	int currentColorIndex;
	int colorIndex;
	qboolean usedColor;
	const char *text;
	char s[2];

	currentColorIndex = ColorIndex( S_COLOR_WHITE );
	len = strlen( pText );
	usedColor = qfalse;

	if ( RobotoMono ) {
		FontCache()->SetActiveFont( RobotoMono );
	} else {
		RobotoMono = FontCache()->AddFontToCache( "fonts/RobotoMono/RobotoMono-Bold.ttf" );
	}

	for ( i = 0, text = pText; i < len; i++, text++ ) {
		// track color changes
		while ( N_IsColorString( text ) && *( text + 1 ) != '\n' ) {
			colorIndex = ColorIndexFromChar( *( text + 1 ) );
			if ( currentColorIndex != colorIndex ) {
				currentColorIndex = colorIndex;
				if ( usedColor ) {
					ImGui::PopStyleColor();
				}
				ImGui::PushStyleColor( ImGuiCol_Text, ImVec4( g_color_table[ colorIndex ] ) );
				usedColor = qtrue;
			}
			text += 2;
		}
		
		switch ( *text ) {
		case '\n':
			if ( usedColor ) {
				ImGui::PopStyleColor();
				currentColorIndex = ColorIndex( S_COLOR_WHITE );
				ImGui::PushStyleColor( ImGuiCol_Text, ImVec4( g_color_table[currentColorIndex] ) );
				usedColor = qfalse;
			}
			ImGui::NewLine();
			break;
		case '\r':
			ImGui::SameLine();
			break;
		default:
			s[0] = *text;
			s[1] = 0;

			ImGui::TextUnformatted( s );
			ImGui::SameLine();
			break;
		};
	}
}
*/

#define  DEFAULT_CONSOLE_WIDTH 78
#define  MAX_CONSOLE_WIDTH 120

#define  NUM_CON_TIMES  4

#define  CON_TEXTSIZE   65536

uint32_t bigchar_width;
uint32_t bigchar_height;
uint32_t smallchar_width;
uint32_t smallchar_height;

typedef struct {
	qboolean	initialized;

	char text[CON_TEXTSIZE];
	uint32_t used;
	uint32_t current;		// line where next message will be printed
	uint32_t x;				// offset in current line for next print
	uint32_t display;		// bottom of console displays this line

	uint32_t contime;

	float	displayFrac;	// aproaches finalFrac at scr_conspeed
	float	finalFrac;		// 0.0 to 1.0 lines of console to display

	char times[NUM_CON_TIMES][MAXPRINTMSG];	// gi.realtime time the line was generated
								// for transparent notify lines
	int32_t contimes[NUM_CON_TIMES];
	vec4_t	color;
} console_t;

static void Con_DrawSolidConsole( float frac );

console_t con;

field_t g_consoleField;
cvar_t *con_conspeed;
cvar_t *con_autoclear;
cvar_t *con_notifytime;
cvar_t *con_scale;
cvar_t *con_color;
cvar_t *con_noprint;
cvar_t *g_conXOffset;

uint32_t g_console_field_width;

/*
================
Con_ToggleConsole_f
================
*/
void Con_ToggleConsole_f( void ) {
	const int windowFlags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar;

	// Can't toggle the console when it's the only thing available
//    if ( Key_GetCatcher() == KEYCATCH_CONSOLE ) {
//		return;
//	}

	if ( con_autoclear->i ) {
		Field_Clear( &g_consoleField );
	}

	g_consoleField.widthInChars = g_console_field_width;

	Con_ClearNotify();

	if ( Key_GetCatcher() & KEYCATCH_CONSOLE ) {
		Key_SetCatcher( Key_GetCatcher() & ~KEYCATCH_CONSOLE );
	} else {
		Key_SetCatcher( Key_GetCatcher() | KEYCATCH_CONSOLE );
	}

	// set the scroll to the absolute bottom
	if ( Key_GetCatcher() & KEYCATCH_CONSOLE ) {
		Con_DrawConsole( qtrue );
	}
}


/*
================
Con_Clear_f
================
*/
static void Con_Clear_f( void ) {
	memset( &con, 0, sizeof( con ) );
}

						
/*
================
Con_Dump_f

Save the console contents out to a file
================
*/
static void Con_Dump_f( void )
{
	uint32_t		i;
	fileHandle_t	f;
	uint32_t		bufferlen;
	char			*buffer;
	char			filename[ MAX_OSPATH ];
	const char		*ext;

	if ( Cmd_Argc() != 2 )
	{
		Con_Printf( "usage: condump <filename>\n" );
		return;
	}

	N_strncpyz( filename, Cmd_Argv( 1 ), sizeof( filename ) );
	COM_DefaultExtension( filename, sizeof( filename ), ".txt" );

	if ( !FS_AllowedExtension( filename, qfalse, &ext ) ) {
		Con_Printf( "%s: Invalid filename extension '%s'.\n", __func__, ext );
		return;
	}

	f = FS_FOpenWrite( filename );
	if ( f == FS_INVALID_HANDLE )
	{
		Con_Printf( "ERROR: couldn't open %s.\n", filename );
		return;
	}

	Con_Printf( "Dumped console text to %s.\n", filename );
	
	bufferlen = con.used;
	buffer = (char *)Hunk_AllocateTempMemory( bufferlen );

	// write the remaining lines
	buffer[ bufferlen - 1 ] = '\0';

	for ( i = 0; i < bufferlen ; i++ )  {
		buffer[i] = con.text[i] & 0xff;

		if (buffer[i] == 0) {
			FS_Write( buffer, strlen( buffer ), f );
			break;
		}
	}

	Hunk_FreeTempMemory( buffer );
	FS_FClose( f );
}

						
/*
================
Con_ClearNotify
================
*/
void Con_ClearNotify( void ) {
	uint32_t i;
	
	//for ( i = 0 ; i < NUM_CON_TIMES ; i++ ) {
	//	con.contimes[i] = 0;
	//}
	con.current = 0;
	memset( con.times, 0, sizeof( con.times ) );
	memset( con.contimes, 0, sizeof( con.contimes ) );
}


/*
================
Con_CheckResize

If the line width has changed, reformat the buffer.
================
*/
void Con_CheckResize( void )
{
	uint32_t i, j, width, oldwidth, oldtotallines, oldcurrent, numlines, numchars;
	char tbuf[CON_TEXTSIZE], *src, *dst;
	static uint32_t old_width, old_vispage;
	uint32_t vispage;
	float	scale;

//	if ( con.viswidth == gi.gpuConfig.vidWidth && !con_scale->modified ) {
//		return;
//	}

	scale = con_scale->f;

//	con.viswidth = gi.gpuConfig.vidWidth;

	smallchar_width = SMALLCHAR_WIDTH * scale * gi.con_factor;
	smallchar_height = SMALLCHAR_HEIGHT * scale * gi.con_factor;
	bigchar_width = BIGCHAR_WIDTH * scale * gi.con_factor;
	bigchar_height = BIGCHAR_HEIGHT * scale * gi.con_factor;

	if (!smallchar_width) {
		smallchar_width = SMALLCHAR_WIDTH;
	}
	if (!smallchar_height) {
		smallchar_height = SMALLCHAR_HEIGHT;
	}

#if 0
	if ( gi.gpuConfig.vidWidth == 0 ) { // video hasn't been initialized yet
		g_console_field_width = DEFAULT_CONSOLE_WIDTH;
		width = DEFAULT_CONSOLE_WIDTH * scale;
		con.linewidth = width;
		con.totallines = CON_TEXTSIZE / con.linewidth;
		con.vispage = 4;

		Con_Clear_f();
	}
	else {
		width = ((gi.gpuConfig.vidWidth / smallchar_width) - 2);

		g_console_field_width = width;
		g_consoleField.widthInChars = g_console_field_width;

		if ( width > MAX_CONSOLE_WIDTH ) {
			width = MAX_CONSOLE_WIDTH;
		}

		vispage = gi.gpuConfig.vidHeight / ( smallchar_height * 2 ) - 1;

		if ( old_vispage == vispage && old_width == width )
			return;

		oldwidth = con.linewidth;
		oldtotallines = con.totallines;
		oldcurrent = con.current;

		con.linewidth = 1024;
		con.totallines = CON_TEXTSIZE / con.linewidth;
		con.vispage = vispage;

		old_vispage = vispage;
		old_width = width;

		numchars = oldwidth;
		if ( numchars > con.linewidth )
			numchars = con.linewidth;

		if ( oldcurrent > oldtotallines )
			numlines = oldtotallines;	
		else
			numlines = oldcurrent + 1;	

		if ( numlines > con.totallines )
			numlines = con.totallines;

		memcpy( tbuf, con.text, sizeof( con.text ) );

		for ( i = 0; i < numlines; i++ )
		{
			src = &tbuf[ ((oldcurrent - i + oldtotallines) % oldtotallines) * oldwidth ];
			dst = &con.text[ (numlines - 1 - i) * con.linewidth ];
			for ( j = 0; j < numchars; j++ ) {
				*dst++ = *src++;
			}
		}

		Con_ClearNotify();

		con.current = numlines - 1;
	}
#endif

	con.display = con.current;

	con_scale->modified = qfalse;
}


/*
==================
Cmd_CompleteTxtName
==================
*/
static void Cmd_CompleteTxtName(const char *args, uint32_t argNum ) {
	if ( argNum == 2 ) {
		Field_CompleteFilename( "", "txt", qfalse, FS_MATCH_EXTERN );
	}
}

/*
================
Con_Init
================
*/
void Con_Init( void ) 
{
	con_notifytime = Cvar_Get( "con_notifytime", "3", 0 );
	Cvar_SetDescription( con_notifytime, "Defines how long messages (from players or the system) are on the screen (in seconds)." );
	con_conspeed = Cvar_Get( "scr_conspeed", "3", 0 );
	Cvar_SetDescription( con_conspeed, "Console opening/closing scroll speed." );
	con_autoclear = Cvar_Get("con_autoclear", "1", CVAR_ARCHIVE_ND);
	Cvar_SetDescription( con_autoclear, "Enable/disable clearing console input text when console is closed." );
	con_scale = Cvar_Get( "con_scale", "1", CVAR_ARCHIVE_ND );
	Cvar_CheckRange( con_scale, "0.5", "8", CVT_FLOAT );
	Cvar_SetDescription( con_scale, "Console font size scale." );
	con_noprint = Cvar_Get( "con_noprint", "0", CVAR_LATCH );
	Cvar_CheckRange( con_noprint, "0", "1", CVT_INT );
	Cvar_SetDescription( con_noprint, "Toggles logging to ingame console." );

	g_conXOffset = Cvar_Get ("g_conXOffset", "0", 0);
	Cvar_SetDescription( g_conXOffset, "Console notifications X-offset." );
	con_color = Cvar_Get( "con_color", "", 0 );
	Cvar_SetDescription( con_color, "Console background color, set as R G B A values from 0-255, use with \\sets to save in config." );

	Field_Clear( &g_consoleField );
	g_consoleField.widthInChars = g_console_field_width;

	memset( &con, 0, sizeof(con) );

	Cmd_AddCommand( "clear", Con_Clear_f );
	Cmd_AddCommand( "condump", Con_Dump_f );
	Cmd_SetCommandCompletionFunc( "condump", Cmd_CompleteTxtName );
	Cmd_AddCommand( "toggleconsole", Con_ToggleConsole_f );
}


/*
================
Con_Shutdown
================
*/
void Con_Shutdown( void ) {
	Cmd_RemoveCommand( "clear" );
	Cmd_RemoveCommand( "condump" );
	Cmd_RemoveCommand( "toggleconsole" );
}

/*
================
G_ConsolePrint

Handles cursor positioning, line wrapping, etc
All console printing must go through this in order to be logged to disk
If no console is visible, the text will appear at the top of the game window
================
*/
void G_ConsolePrint( const char *txt ) {
	qboolean skipnotify = qfalse;
	char *buf, *startLine;
	int colorIndex;
	int c;
	uint64_t len;

	// TTimo - prefix for text that shows up in console but not in notify
	// backported from RTCW
	if ( !N_strncmp( txt, "[skipnotify]", 12 ) ) {
		skipnotify = qtrue;
		txt += 12;
	}

	// for some demos we don't want to ever show anything on the console
	if ( con_noprint && con_noprint->i ) {
		return;
	}
	
	if ( !con.initialized ) {
		static cvar_t null_cvar = { 0 };
		con.color[0] =
		con.color[1] =
		con.color[2] =
		con.color[3] = 1.0f;
//		con.viswidth = -9999;
		gi.con_factor = 1.0f;
		con.contime = 0;
		con_scale = &null_cvar;
		con_scale->f = 1.0f;
		con_scale->modified = qtrue;
		//Con_CheckResize();
		con.initialized = qtrue;
	}
	
	buf = con.times[con.contime % NUM_CON_TIMES];
	startLine = buf;
	len = strlen( txt );

	if ( con.used + len >= CON_TEXTSIZE ) {
		Con_Clear_f();
	}

	memcpy( con.text + con.used, txt, len );
	con.used += len;

	while ( ( c = *txt ) != 0 ) {
		txt++;
		switch ( *txt ) {
		case '\n':
//			con.totallines++;
			if ( skipnotify ) {
				con.contimes[ con.contime % NUM_CON_TIMES ] = 0;
			} else {
				con.contimes[ con.contime % NUM_CON_TIMES ] = gi.realtime;
			}
			buf = con.times[con.contime % NUM_CON_TIMES];
			con.current++;
			startLine = buf;
			break;
		case '\r':
			buf = startLine;
			break;
		default:
			*buf++ = *txt;
			break;
		};
	}
	con.contime++;
}


/*
==============================================================================

CONSOLE FIELD EDITING

==============================================================================
*/

void Field_Paste( field_t *edit );

static int Con_TextCallback( ImGuiInputTextCallbackData *data )
{
	field_t *edit;
	uint32_t len;

	edit = &g_consoleField;

	len = strlen( edit->buffer );

	if ( Key_IsDown( KEY_CTRL ) && Key_IsDown( KEY_A ) ) {
		data->SelectAll();
	}

	// ctrl-L clears screen
	if ( keys[KEY_L].down && keys[ KEY_CTRL ].down ) {
		Cbuf_AddText( "clear\n" );
		return 1;
	}

	// command completion

	if ( keys[KEY_TAB].down ) {
		Field_AutoComplete( &g_consoleField );
		
		data->CursorPos = 0;
		data->BufTextLen = 0;
		data->InsertChars( 0, g_consoleField.buffer );

		data->CursorPos = g_consoleField.cursor;
	}

	// command history (ctrl-p ctrl-n for unix style)

	if ( ( keys[KEY_WHEEL_UP].down && keys[KEY_SHIFT].down ) || keys[KEY_UP].down
		|| ( keys[KEY_P].down && keys[KEY_CTRL].down ) )
	{
		if ( Con_HistoryGetPrev( &g_consoleField ) ) {
			data->DeleteChars( 0, data->BufTextLen );
			data->InsertChars( data->CursorPos, edit->buffer + data->CursorPos, edit->buffer + edit->cursor );
		} else {
			edit->cursor = data->CursorPos = 0;
		}
	}

/*
	if ( keys[KEY_CTRL].down && keys[KEY_C].down ) {
		if ( !data->HasSelection() ) {
			SDL_SetClipboardText( " " );
		} else {
			SDL_SetClipboardText( data->Buf + data->SelectionStart );
		}
	}
	if ( keys[KEY_CTRL].down && keys[KEY_V].down ) {
		if ( data->HasSelection() ) {
			data->DeleteChars( data->SelectionStart, data->SelectionEnd - data->SelectionStart );
		}
		data->InsertChars( data->CursorPos, buf );
		edit->cursor = data->CursorPos;
		Z_Free( buf );
	}
*/
	if ( ( keys[KEY_WHEEL_DOWN].down && keys[KEY_SHIFT].down ) || keys[KEY_DOWN].down
		|| ( keys[KEY_N].down && keys[KEY_CTRL].down ) )
	{
		if ( Con_HistoryGetNext( &g_consoleField ) ) {
			data->DeleteChars( 0, data->BufTextLen );
			data->InsertChars( data->CursorPos, edit->buffer + data->CursorPos, edit->buffer + edit->cursor );
		} else {
			data->DeleteChars( 0, data->BufTextLen );
		}
	}

	if ( keys[KEY_INSERT].down ) {
		key_overstrikeMode = !key_overstrikeMode;
	}

	if ( Key_IsDown( KEY_BACKSPACE ) ) {
		if ( data->HasSelection() ) {
			data->DeleteChars( data->CursorPos, data->SelectionEnd - data->SelectionStart );
			data->ClearSelection();
		} else {
			data->DeleteChars( data->CursorPos, 1 );
		}
		edit->cursor = data->CursorPos;
	}

	if ( keys[KEY_HOME].down ) {
		edit->cursor = data->CursorPos = 0;
	}

	if ( keys[KEY_END].down ) {
		edit->cursor = data->CursorPos = len;
	}

	// ctrl-home = top of console
	if ( Key_IsDown( KEY_HOME ) && Key_IsDown( KEY_CTRL ) ) {
		ImGui::SetScrollX( 0.0f );
		ImGui::SetScrollY( 0.0f );
	}

	// ctrl-end = bottom of console
	if ( Key_IsDown( KEY_END ) && Key_IsDown( KEY_CTRL ) ) {
		ImGui::SetScrollX( 0.0f );
		ImGui::SetScrollY( ImGui::GetScrollMaxY() );
	}

	edit->cursor = data->CursorPos;

	ImGui::SetScrollHereY();
	
	return 1;
}

/*
================
Con_DrawInput

Draw the editline after a ] prompt
================
*/
static void Con_DrawInput( void ) {
	const int windowFlags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar
		| ImGuiWindowFlags_AlwaysAutoResize;
	
	if ( !( Key_GetCatcher( ) & KEYCATCH_CONSOLE ) ) {
		return;
	}

	ImGui::PushStyleColor( ImGuiCol_FrameBg, ImVec4( 1.0f, 1.0f, 1.0f, 0.0f ) );
	ImGui::PushStyleColor( ImGuiCol_FrameBgActive, ImVec4( 1.0f, 1.0f, 1.0f, 0.0f ) );
	ImGui::PushStyleColor( ImGuiCol_FrameBgHovered, ImVec4( 1.0f, 1.0f, 1.0f, 0.0f ) );

	ImGui::NewLine();
	ImGui::PushStyleColor( ImGuiCol_Text, ImVec4( g_color_table[ ColorIndex( S_COLOR_WHITE ) ] ) );
	ImGui::TextUnformatted( "] " );
	ImGui::SameLine();

	if ( ImGui::InputText( "##ConsoleInputField", g_consoleField.buffer, sizeof(g_consoleField.buffer) - 1,
		ImGuiInputTextFlags_CallbackHistory | ImGuiInputTextFlags_CallbackCompletion |
		ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_CtrlEnterForNewLine |
		( key_overstrikeMode ? ImGuiInputTextFlags_AlwaysOverwrite : 0 ),
		Con_TextCallback, NULL ) )
	{
		// enter finishes the line
		// if not in the game explicitly prepend a slash if needed
		if ( gi.state == GS_LEVEL
			&& g_consoleField.buffer[0] != '\0'
			&& g_consoleField.buffer[0] != '\\'
			&& g_consoleField.buffer[0] != '/' ) {
			char	temp[MAX_EDIT_LINE-1];

			N_strncpyz( temp, g_consoleField.buffer, sizeof( temp ) );
			Com_snprintf( g_consoleField.buffer, sizeof( g_consoleField.buffer ), "\\%s", temp );
			g_consoleField.cursor++;
		}

		Con_Printf( "]%s\n", g_consoleField.buffer );

		// leading slash is an explicit command
		if ( g_consoleField.buffer[0] == '\\' || g_consoleField.buffer[0] == '/' ) {
			Cbuf_AddText( g_consoleField.buffer+1 );	// valid command
			Cbuf_AddText( "\n" );
		} else {
			// other text will be chat messages
			if ( !g_consoleField.buffer[0] ) {
				return;	// empty lines just scroll the console without adding to history
			} else {
				Cbuf_AddText( g_consoleField.buffer+1 );	// valid command
				Cbuf_AddText( "\n" );
	//			Cbuf_AddText( "cmd say " );
	//			Cbuf_AddText( g_consoleField.buffer );
	//			Cbuf_AddText( "\n" );
			}
		}
		// copy line to history buffer
		Con_SaveField( &g_consoleField );

		Field_Clear( &g_consoleField );
		g_consoleField.widthInChars = g_console_field_width;
	}

	ImGui::PopStyleColor( 4 );
}

/*
==============================================================================

DRAWING

==============================================================================
*/

static qboolean Con_IsValidChar( int c )
{
	if ( N_isalpha( c ) ) {
		return qtrue;
	}
	switch ( c ) {
	case '[':
	case ']':
	case '{':
	case '}':
	case ':':
	case '\"':
	case '\'':
	case '\\':
	case '/':
	case '+':
	case '*':
	case '#':
		return qtrue;
	default:
		break;
	};
	return qfalse;
}

static void Con_DrawText( const char *txt )
{
	uint64_t len, i;
	int currentColorIndex;
	int colorIndex;
	qboolean usedColor = qfalse;
	const char *text;
	char s[2];

	currentColorIndex = ColorIndex( S_COLOR_WHITE );

	len = strlen( txt );

	ImGui::GetStyle().ItemSpacing.y = 0.5f;
	ImGui::GetStyle().ItemSpacing.x = 0.0f;

	if ( RobotoMono ) {
		FontCache()->SetActiveFont( RobotoMono );
	} else {
		RobotoMono = FontCache()->AddFontToCache( "RobotoMono", "Bold" );
	}

	for ( i = 0, text = txt; i < len; i++, text++ ) {
		// track color changes
		while ( Q_IsColorString(text) && *(text+1) != '\n' ) {
			colorIndex = ColorIndexFromChar(*(text+1));
			if (currentColorIndex != colorIndex) {
				currentColorIndex = colorIndex;
				if (usedColor) {
					ImGui::PopStyleColor();
				}
				ImGui::PushStyleColor( ImGuiCol_Text, ImVec4( g_color_table[ colorIndex ] ) );
				usedColor = qtrue;
			}
			text += 2;
		}
		
		switch (*text) {
		case '\n':
			if ( usedColor ) {
				ImGui::PopStyleColor();
				currentColorIndex = ColorIndex( S_COLOR_WHITE );
				ImGui::PushStyleColor( ImGuiCol_Text, ImVec4( g_color_table[currentColorIndex] ) );
				usedColor = qfalse;
			}
			ImGui::NewLine();
			break;
		case '\r':
			ImGui::SameLine();
			break;
		default:
			s[0] = *text;
			s[1] = 0;

			ImGui::TextWrapped( s );
			ImGui::SameLine();
			break;
		};
	}
}

/*
================
Con_DrawSolidConsole

Draws the console with the solid background
================
*/
static void Con_DrawSolidConsole( float frac, qboolean open )
{
	static float conColorValue[4] = { 0.0, 0.0, 0.0, 0.0 };
	// for cvar value change tracking
	static char  conColorString[ MAX_CVAR_VALUE ] = { '\0' };
	int currentColorIndex, colorIndex;
	qboolean customColor = qfalse;
	uint32_t i;
	char *text;
	float height;
	renderSceneRef_t refdef;
	char buf[ MAX_CVAR_VALUE ], *v[4];
	const int windowFlags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar;

	memset( &refdef, 0, sizeof( refdef ) );
	refdef.x = 0;
	refdef.y = 0;
	refdef.width = gi.gpuConfig.vidWidth;
	refdef.height = gi.gpuConfig.vidHeight;
//	refdef.time = gi.frametime * 0.001f;
	refdef.time = -( gi.frametime - gi.realFrameTime ) * 0.10f;
	refdef.flags = RSF_NOWORLDMODEL | RSF_ORTHO_TYPE_SCREENSPACE;

	height = gi.gpuConfig.vidHeight * frac;
	if ( (int)height <= 0 ) {
		return;
	}

	// draw the background
	height = frac * gi.gpuConfig.vidHeight;
	re.ClearScene();

	ImGui::Begin( "CommandConsole", NULL, windowFlags | ImGuiWindowFlags_NoBackground );
	ImGui::SetWindowPos( ImVec2( 0, 0 ) );
	ImGui::SetWindowSize( ImVec2( refdef.width, height ) );
	ImGui::SetWindowFontScale( ImGui::GetFont()->Scale * con_scale->f );
//	ImGui::PushTextWrapPos( refdef.width );

	// custom console background color
	if ( con_color->s[0] ) {
		// track changes
		if ( strcmp( con_color->s, conColorString ) )  {
			N_strncpyz( conColorString, con_color->s, sizeof( conColorString ) );
			N_strncpyz( buf, con_color->s, sizeof( buf ) );
			Com_Split( buf, v, 4, ' ' );
			for ( i = 0; i < 4 ; i++ ) {
				conColorValue[ i ] = N_atof( v[ i ] ) / 255.0f;
				if ( conColorValue[ i ] > 1.0f ) {
					conColorValue[ i ] = 1.0f;
				} else if ( conColorValue[ i ] < 0.0f ) {
					conColorValue[ i ] = 0.0f;
				}
			}
		}
		ImGui::PushStyleColor( ImGuiCol_WindowBg, ImVec4( conColorValue ) );
		ImGui::Image( (ImTextureID)(uintptr_t)gi.whiteShader, ImVec2( (float)refdef.width, height ) );
		customColor = qtrue;
	} else {
		re.DrawImage( 0, 0, refdef.width, height, 0, 0, 1, 1, gi.consoleShader );
	}

	// draw from the bottom up
	{
		// draw arrows to show the buffer is backscrolled
		ImGui::PushStyleColor( ImGuiCol_Text, ImVec4( g_color_table[ ColorIndex( S_COLOR_RED ) ] ) );

		const uint32_t count = gi.gpuConfig.vidWidth / ImGui::GetFontSize();

		for ( i = 0; i < count; i++ ) {
			ImGui::TextUnformatted( "^" );
			ImGui::SameLine();
		}
		ImGui::PopStyleColor();

		ImGui::NewLine();
		ImGui::NewLine();
	}

	Con_DrawText( con.text );

	Con_DrawInput();

	if ( open ) {
		ImGui::SetScrollHereY();
	}

	ImGui::End();

	//
	// draw the version
	//
	ImGui::Begin( "CommandConsoleVersion", NULL, windowFlags | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_AlwaysAutoResize );
	ImGui::SetWindowPos( ImVec2( refdef.width - ( 200 * gi.scale ), height - 48 ) );
	ImGui::TextUnformatted( GLN_VERSION );
	ImGui::End();
	re.RenderScene( &refdef );
}

static void Con_DrawNotify( void ) {
	char		*text;
	int32_t		i;
	uint32_t	time;

	const int windowFlags = ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoBackground
		| ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoMouseInputs;

	ImGui::Begin( "Notify", NULL, windowFlags );
	ImGui::SetWindowPos( ImVec2( 0, 0 ) );
	ImGui::SetWindowFontScale( 1.5f * gi.scale );

	ImGui::PushStyleColor( ImGuiCol_Text, g_color_table[ ColorIndex( S_COLOR_WHITE ) ] );

	for ( i = con.contime - NUM_CON_TIMES + 1; i <= con.current; i++ ) {
		if ( i < 0 ) {
			continue;
		}
		time = con.contimes[i % NUM_CON_TIMES];
		if (time == 0) {
			continue;
		}
		time = gi.realtime - time;
		if ( time >= con_notifytime->f * 1000 ) {
			continue;
		}
		text = con.times[ i % NUM_CON_TIMES ];

		Con_DrawText( text );
	}

	ImGui::PopStyleColor();

	if ( Key_GetCatcher() & ( KEYCATCH_UI | KEYCATCH_SGAME ) ) {
		return;
	}

	ImGui::End();
}

/*
==================
Con_DrawConsole
==================
*/
void Con_DrawConsole( qboolean open ) {

	// check for console width changes from a vid mode change
	Con_CheckResize();

	if ( con.displayFrac ) {
		Con_DrawSolidConsole( con.displayFrac, open );
	} else {
		// draw notify lines
		if ( gi.state == GS_LEVEL ) {
			Con_DrawNotify();
		}
	}
}

//================================================================

/*
==================
Con_RunConsole

Scroll it up or down
==================
*/
void Con_RunConsole( void ) 
{
	// decide on the destination height of the console
	if ( Key_GetCatcher( ) & KEYCATCH_CONSOLE )
		con.finalFrac = 0.75f;	// half screen
	else
		con.finalFrac = 0.0f;	// none visible
	
	// scroll towards the destination height
	if ( con.finalFrac < con.displayFrac ) {
//		con.displayFrac -= con_conspeed->f * gi.frametime * 0.001;
		con.displayFrac -= con_conspeed->f * ( -( gi.frametime - gi.realFrameTime ) * 0.10f ) * 0.001f;
		if ( con.finalFrac > con.displayFrac ) {
			con.displayFrac = con.finalFrac;
		}

	}
	else if ( con.finalFrac > con.displayFrac ) {
//		con.displayFrac += con_conspeed->f * gi.frametime * 0.001;
		con.displayFrac += con_conspeed->f * ( -( gi.frametime - gi.realFrameTime ) * 0.10f ) * 0.001f;
		if ( con.finalFrac < con.displayFrac ) {
			con.displayFrac = con.finalFrac;
		}
	}
}


void Con_Close( void )
{
//	if ( !com_cl_running->i )
//		return;

	Field_Clear( &g_consoleField );
	Con_ClearNotify();
	Key_SetCatcher( Key_GetCatcher( ) & ~KEYCATCH_CONSOLE );
	con.finalFrac = 0.0;			// none visible
	con.displayFrac = 0.0;
}
