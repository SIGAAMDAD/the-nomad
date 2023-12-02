#include "g_game.h"
#include "../rendercommon/imgui.h"
#include "../rendercommon/imgui_impl_sdl2.h"

extern field_t g_consoleField;
qboolean key_overstrikeMode;

typedef struct {
	const char *name;
	uint32_t keynum;
} keyname_t;

static const keyname_t keynames[] = {
	{"WHEEL_DOWN", KEY_WHEEL_DOWN},
	{"WHEEL_UP", KEY_WHEEL_UP},

	{"MOUSE_LEFT", KEY_MOUSE_LEFT},
	{"MOUSE_MIDDLE", KEY_MOUSE_MIDDLE},
	{"MOUSE_RIGHT", KEY_MOUSE_RIGHT},
	{"a", KEY_A},
	{"b", KEY_B},
	{"c", KEY_C},
	{"d", KEY_D},
	{"e", KEY_E},
	{"f", KEY_F},
	{"g", KEY_G},
	{"h", KEY_H},
	{"i", KEY_I},
	{"j", KEY_J},
	{"k", KEY_K},
	{"l", KEY_L},
	{"m", KEY_M},
	{"n", KEY_N},
	{"o", KEY_O},
	{"p", KEY_P},
	{"q", KEY_Q},
	{"r", KEY_R},
	{"s", KEY_S},
	{"t", KEY_T},
	{"u", KEY_U},
	{"v", KEY_V},
	{"w", KEY_W},
	{"x", KEY_X},
	{"y", KEY_Y},
	{"z", KEY_Z},

	{"LALT", KEY_LALT},
	{"RALT", KEY_RALT},
	{"LSHIFT", KEY_LSHIFT},
	{"RSHIFT", KEY_RSHIFT},
	{"LCTRL", KEY_LCTRL},
	{"RCTRL", KEY_RCTRL},
	{"TAB", KEY_TAB},
	{"ENTER", KEY_ENTER},
	{"BACKSPACE", KEY_BACKSPACE},
	{"SPACE", KEY_SPACE},

	{"CONSOLE", KEY_CONSOLE},
	{"PRINTSCREEN", KEY_SCREENSHOT},

	{"UPARROW", KEY_UP},
	{"DOWNARROW", KEY_DOWN},
	{"RIGHTARROW", KEY_RIGHT},
	{"LEFTARROW", KEY_LEFT},

	{"F1", KEY_F1},
	{"F2", KEY_F2},
	{"F3", KEY_F3},
	{"F4", KEY_F4},
	{"F5", KEY_F5},
	{"F6", KEY_F6},
	{"F7", KEY_F7},
	{"F8", KEY_F8},
	{"F9", KEY_F9},
	{"F10", KEY_F10},
	{"F11", KEY_F11},
	{"F12", KEY_F12},

	{NULL, 0}
};

static ImGuiKey EngineKeyToImGuiKey(uint32_t key);
static void Field_CharEvent( field_t *edit, uint32_t ch );

nkey_t keys[NUMKEYS];

qboolean Key_AnyDown(void)
{
	for (uint32_t i = 0; i < NUMKEYS; i++) {
		if (keys[i].down)
			return qtrue;
	}
	return qfalse;
}

qboolean Key_IsDown(uint32_t keynum)
{
    if (keynum >= NUMKEYS)
        return qfalse;
    
    return keys[keynum].down;
}

qboolean Key_GetOverstrikeMode( void ) {
	return key_overstrikeMode;
}

void Key_SetOverstrikeMode( qboolean overstrike ) {
	key_overstrikeMode = overstrike;
}

static void Key_CompleteBind(const char *args, uint32_t argnum)
{
	const char *p;

	if (argnum == 2) {
		// skip "bind "
		p = Com_SkipTokens(args, 1, " ");

		if (p > args)
			Field_CompleteKeyname();
	}
	else if (argnum >= 3) {
		uint32_t key;

		// skip "bind <key> "
		p = Com_SkipTokens(args, 2, " ");
		if (*p == '\0' && (key = Key_StringToKeynum(Cmd_Argv(1))) >= 0) {
			Field_CompleteKeyBind(key);
		}
		else if (p > args) {
			Field_CompleteCommand(p, qtrue, qtrue);
		}
	}
}

static void Key_CompleteUnbind(const char *args, uint32_t argnum)
{
	if (argnum) {
		// skip "unbind "
		const char *p = Com_SkipTokens(args, 1, " ");

//		if (p > args)
//			Field_CompleteKeyname();
	}
}

/*
Key_StringToKeynum: Returns a key number to be used to index keys[] by looking at
the given string.  Single ascii characters return themselves, while the K_* names are matched up.

0x11 will be interpreted as raw hex, which will allow new controllers

to be configured even if they don't have defined names.
*/
uint32_t Key_StringToKeynum( const char *str )
{
	const keyname_t	*kn;

	if ( !str || str[0] == '\0' ) {
		return -1;
	}
	if ( str[1] == '\0' ) {
		return str[0];
	}

	// check for hex code
	if ( strlen( str ) == 4 ) {
		uint32_t n = Com_HexStrToInt( str );

		if ( n >= 0 ) {
			return n;
		}
	}

	// scan for a text match
	for ( kn = keynames ; kn->name ; kn++ ) {
		if ( !N_stricmp( str, kn->name ) )
			return kn->keynum;
	}

	return -1;
}

/*
Key_KeynumToString: returns a string (either a single ascii char, a SDLK_* name, o a 0x11 hex string) for the
given keynum
*/
const char *Key_KeynumToString(uint32_t keynum)
{
	const keyname_t *kn;
	static char tinystr[5];
	uint32_t i, j;

	if (keynum >= NUMKEYS) {
		return "<OUT OF RANGE>";
	}

	// manual overrides for keys that show up as ASCII but really shouldn't show up as ASCII
	switch (keynum) {
	case KEY_MOUSE_LEFT:
		return "MOUSE_LEFT";
	case KEY_MOUSE_RIGHT:
		return "MOUSE_RIGHT";
	case KEY_MOUSE_MIDDLE:
		return "MOUSE_MIDDLE";
	case KEY_WHEEL_DOWN:
		return "WHEEL_DOWN";
	case KEY_WHEEL_UP:
		return "WHEEL_UP";
    case KEY_TAB: // shows up as '+'
        return "TAB";
    case KEY_LSHIFT:
        return "LSHIFT";
    case KEY_RSHIFT:
        return "RSHIFT";
    case KEY_LALT:
        return "LALT";
    case KEY_RALT:
        return "RALT";
    case KEY_LCTRL:
        return "LCTRL";
    case KEY_RCTRL:
        return "RCTRL";
    case KEY_HOME:
        return "HOME";
    case KEY_SPACE:
        return "SPACE";
    case KEY_BACKSPACE:
        return "BACKSPACE";
    case KEY_ENTER:
        return "ENTER";
    case KEY_PAGEUP:
        return "PAGEUP";
    case KEY_PAGEDOWN:
        return "PAGEDOWN";
    case KEY_END:
        return "END";
    case KEY_DELETE:
        return "DELETE";
    case KEY_INSERT:
        return "INSERT";
    case KEY_UP:
        return "UPARROW";
    case KEY_DOWN:
        return "DOWNARROW";
    case KEY_LEFT:
        return "LEFTARROW";
    case KEY_RIGHT:
        return "RIGHTARROW";
    default:
        break;
	};

	// check for printable ascii (don't use quote)
	if (keynum > ' ' && keynum < '~' && keynum != '"' && keynum != ';') {
		tinystr[0] = keynum;
		tinystr[1] = '\0';
		return tinystr;
	}

	// check for a key string
	for (kn = keynames; kn->name; kn++) {
		if (keynum == kn->keynum) {
			return kn->name;
		}
	}

	// make a hex string
	i = keynum >> 4;
	j = keynum & 15;

	tinystr[0] = '0';
	tinystr[1] = 'x';
	tinystr[2] = i > 9 ? i - 10 + 'a' : i + '0';
	tinystr[3] = j > 9 ? j - 10 + 'a' : j + '0';
	tinystr[4] = 0;

	return tinystr;
}

void Key_SetBinding(uint32_t keynum, const char *binding)
{
	if (keynum >= NUMKEYS) {
		return;
	}

	// free old bindings
	if (keys[keynum].binding) {
		Z_Free(keys[keynum].binding);
	}

	// allocate new memory for new binding
	keys[keynum].binding = CopyString(binding);
}

const char *Key_GetBinding(uint32_t keynum)
{
	if (keynum >= NUMKEYS) {
		return "";
	}

	return keys[keynum].binding;
}

uint32_t Key_GetKey( const char *binding )
{
	uint32_t i;

	if ( binding ) {
		for ( i = 0 ; i < NUMKEYS ; i++ ) {
			if ( keys[i].binding && N_stricmp( binding, keys[i].binding ) == 0 ) {
				return i;
			}
		}
	}
	return -1;
}

static void Key_Unbindall_f( void )
{
	uint32_t i;

	for ( i = 0 ; i < NUMKEYS; i++ ) {
		if ( keys[i].binding ) {
			Key_SetBinding( i, "" );
		}
	}
}

static void Key_Bind_f( void )
{
	uint32_t c, b;

	c = Cmd_Argc();

	if ( c < 2 ) {
		Con_Printf( "bind <key> [command] : attach a command to a key\n" );
		return;
	}

	b = Key_StringToKeynum( Cmd_Argv( 1 ) );
	if ( b == -1 ) {
		Con_Printf( "\"%s\" isn't a valid key\n", Cmd_Argv( 1 ) );
		return;
	}

	if ( c == 2 ) {
		if ( keys[b].binding && keys[b].binding[0] )
			Con_Printf( "\"%s\" = \"%s\"\n", Cmd_Argv( 1 ), keys[b].binding );
		else
			Con_Printf( "\"%s\" is not bound\n", Cmd_Argv( 1 ) );
		
		return;
	}

	// copy the rest of the command line
	Key_SetBinding( b, Cmd_ArgsFrom( 2 ) );
}

/*
Key_WriteBindings: Writes lines containing "bind key value"
*/
void Key_WriteBindings( file_t f )
{
	uint32_t i;

	FS_Printf( f, "unbindall" GDR_NEWLINE );

	for ( i = 0 ; i < NUMKEYS ; i++ ) {
		if (!keys[i].binding || !keys[i].binding[0])
			continue;
		
		FS_Printf( f, "bind %s \"%s\"" GDR_NEWLINE, Key_KeynumToString(i), keys[i].binding );
	}
}

static void Key_Bindlist_f( void )
{
	uint32_t i;

	for ( i = 0 ; i < NUMKEYS ; i++ ) {
		if ( keys[i].binding && keys[i].binding[0] ) {
			Con_Printf( "%s \"%s\"\n", Key_KeynumToString(i), keys[i].binding );
		}
	}
}


void Key_KeynameCompletion( void(*callback)(const char *s) )
{
	int	i;

	for( i = 0; keynames[ i ].name != NULL; i++ )
		callback( keynames[ i ].name );
}


/*
Key_ParseBinding: execute the commands in the bind string
*/
void Key_ParseBinding(uint32_t key, qboolean down, uint32_t time)
{
	char buf[MAX_STRING_CHARS], *p, *end;

	if (!keys[key].binding || keys[key].binding[0] == '\0')
		return;
	
	p = buf;

	N_strncpyz(buf, keys[key].binding, sizeof(buf));

	while (1) {
		while (isspace(*p))
			p++;
		
		end = strchr(p, ';');
		if (end)
			*end = '\0';
		if (*p == '+') {
			// button commands add keynum and time as parameters
			// so that multiple sources can be discriminated and
			// subframe corrected
			char cmd[1024];
			snprintf(cmd, sizeof(cmd), "%c%s %i %i\n", (down) ? '+' : '-', p +1, key, time);
			Cbuf_AddText(cmd);
			if (down)
				keys[key].bound = qtrue;
		}
		else if (down) {
			// normal commands only execute on key press
			Cbuf_AddText(p);
			Cbuf_AddText("\n");
		}

		if (!end)
			break;
		
		p = end + 1;
	}
}

/*
===================
Field_Draw

Handles horizontal scrolling and cursor blinking
x, y, and width are in pixels
===================
*/
static void Field_VariableSizeDraw( field_t *edit, uint32_t x, uint32_t y, uint32_t width, uint32_t size, qboolean showCursor,
		qboolean noColorEscape ) {
	uint32_t len;
	uint32_t drawLen;
	uint32_t prestep;
	uint32_t cursorChar;
	char	str[MAX_STRING_CHARS], *s;
	char	text[2];
	uint32_t i;
	uint32_t curColor;

	drawLen = edit->widthInChars - 1; // - 1 so there is always a space for the cursor
	len = strlen( edit->buffer );

	// guarantee that cursor will be visible
	if ( len <= drawLen ) {
		prestep = 0;
	} else {
		if ( edit->scroll + drawLen > len ) {
			edit->scroll = len - drawLen;
			if ( edit->scroll < 0 ) {
				edit->scroll = 0;
			}
		}
		prestep = edit->scroll;
	}

	if ( prestep + drawLen > len ) {
		drawLen = len - prestep;
	}

	// extract <drawLen> characters from the field at <prestep>
	if ( drawLen >= MAX_STRING_CHARS ) {
		N_Error( ERR_DROP, "drawLen >= MAX_STRING_CHARS" );
	}

	memcpy( str, edit->buffer + prestep, drawLen );
	str[ drawLen ] = '\0';

	// color tracking
	curColor = S_COLOR_WHITE;

	if ( prestep > 0 ) {
		// we need to track last actual color because we cut some text before
		s = edit->buffer;
		for ( i = 0; i < prestep + 1; i++, s++ ) {
			if ( Q_IsColorString( s ) ) {
				curColor = *(s+1);
				s++;
			}
		}
		// scroll marker
		// FIXME: force white color?
		if ( str[0] ) {
			str[0] = '<';
		}
	}

	// draw it
	ImGui::TextColored( ImVec4( g_color_table[ ColorIndexFromChar( curColor ) ] ), "%s", str );
	if ( len > drawLen + prestep ) {
		ImGui::SameLine();
		text[0] = ' ';
		text[1] = 0;
		for (int a = 0; a < edit->widthInChars; a++) {
			ImGui::TextUnformatted( text, text + 1 );
			ImGui::SameLine();
		}
		text[0] = '>';
		ImGui::TextUnformatted( text, text + 1 );
		ImGui::SameLine();
	}

	// draw the cursor
	if ( showCursor ) {
		if ( gi.realtime & 256 ) {
			return;		// off blink
		}

		if ( key_overstrikeMode ) {
			cursorChar = 11;
		} else {
			cursorChar = 10;
		}

		i = drawLen - strlen( str );

		str[0] = cursorChar;
		str[1] = '\0';

		text[0] = ' ';
		text[1] = 0;
		for (int a = 0; a < edit->cursor - prestep - i; a++) {
			ImGui::TextUnformatted( text, text + 1 );
			ImGui::SameLine();
		}

		ImGui::TextUnformatted( str, str + 1 );
//		SCR_DrawBigString( x + ( edit->cursor - prestep - i ) * BIGCHAR_WIDTH, y, str, 1.0, qfalse );
	}
}


void Field_Draw( field_t *edit, uint32_t x, uint32_t y, uint32_t width, qboolean showCursor, qboolean noColorEscape )
{
	Field_VariableSizeDraw( edit, x, y, width, smallchar_width, showCursor, noColorEscape );
}


void Field_BigDraw( field_t *edit, uint32_t x, uint32_t y, uint32_t width, qboolean showCursor, qboolean noColorEscape )
{
	Field_VariableSizeDraw( edit, x, y, width, bigchar_width, showCursor, noColorEscape );
}


/*
================
Field_Paste
================
*/
static void Field_Paste( field_t *edit ) {
	char	*cbd;
	uint32_t pasteLen, i;

	cbd = Sys_GetClipboardData();

	if ( !cbd ) {
		return;
	}

	// send as if typed, so insert / overstrike works properly
	pasteLen = strlen( cbd );
	for ( i = 0 ; i < pasteLen ; i++ ) {
		Field_CharEvent( edit, cbd[i] );
	}

	Z_Free( cbd );
}


/*
=================
Field_NextWord
=================
*/
static void Field_SeekWord( field_t *edit, uint32_t direction )
{
	if ( direction > 0 ) {
		while ( edit->buffer[ edit->cursor ] == ' ' )
			edit->cursor++;
		while ( edit->buffer[ edit->cursor ] != '\0' && edit->buffer[ edit->cursor ] != ' ' )
			edit->cursor++;
		while ( edit->buffer[ edit->cursor ] == ' ' )
			edit->cursor++;
	} else {
		while ( edit->cursor > 0 && edit->buffer[ edit->cursor-1 ] == ' ' )
			edit->cursor--;
		while ( edit->cursor > 0 && edit->buffer[ edit->cursor-1 ] != ' ' )
			edit->cursor--;
		if ( edit->cursor == 0 && ( edit->buffer[ 0 ] == '/' || edit->buffer[ 0 ] == '\\' ) )
			edit->cursor++;
	}
}


/*
=================
Field_KeyDownEvent

Performs the basic line editing functions for the console,
in-game talk, and menu fields

Key events are used for non-printable characters, others are gotten from char events.
=================
*/
static void Field_KeyDownEvent( field_t *edit, uint32_t key ) {
	uint64_t len;

	// shift-insert is paste
	if ( ( ( key == KEY_INSERT ) ) && keys[KEY_LSHIFT].down ) {
		Field_Paste( edit );
		return;
	}

	len = strlen( edit->buffer );

	switch ( key ) {
	case KEY_DELETE:
		if ( edit->cursor < len ) {
			memmove( edit->buffer + edit->cursor,
				edit->buffer + edit->cursor + 1, len - edit->cursor );
		}
		break;
	case KEY_RIGHT:
		if ( edit->cursor < len ) {
			if ( keys[ KEY_LCTRL ].down ) {
				Field_SeekWord( edit, 1 );
			} else {
				edit->cursor++;
			}
		}
		break;
	case KEY_LEFT:
		if ( edit->cursor > 0 ) {
			if ( keys[ KEY_LCTRL ].down ) {
				Field_SeekWord( edit, -1 );
			} else {
				edit->cursor--;
			}
		}
		break;
	case KEY_HOME:
		edit->cursor = 0;
		break;
	case KEY_END:
		edit->cursor = len;
		break;
	case KEY_INSERT:
		key_overstrikeMode = !key_overstrikeMode;
		break;
	default:
		break;
	};

	// Change scroll if cursor is no longer visible
	if ( edit->cursor < edit->scroll ) {
		edit->scroll = edit->cursor;
	} else if ( edit->cursor >= edit->scroll + edit->widthInChars && edit->cursor <= len ) {
		edit->scroll = edit->cursor - edit->widthInChars + 1;
	}
}


/*
==================
Field_CharEvent
==================
*/
static void Field_CharEvent( field_t *edit, uint32_t ch ) {
	uint64_t len;

	if ( ch == 'v' - 'a' + 1 ) {	// ctrl-v is paste
		Field_Paste( edit );
		return;
	}

	if ( ch == 'c' - 'a' + 1 ) {	// ctrl-c clears the field
		Field_Clear( edit );
		return;
	}

	len = strlen( edit->buffer );

	if ( ch == 'h' - 'a' + 1 )	{	// ctrl-h is backspace
		if ( edit->cursor > 0 ) {
			memmove( edit->buffer + edit->cursor - 1,
				edit->buffer + edit->cursor, len + 1 - edit->cursor );
			edit->cursor--;
			if ( edit->cursor < edit->scroll )
			{
				edit->scroll--;
			}
		}
		return;
	}

	if ( ch == 'a' - 'a' + 1 ) {	// ctrl-a is home
		edit->cursor = 0;
		edit->scroll = 0;
		return;
	}

	if ( ch == 'e' - 'a' + 1 ) {	// ctrl-e is end
		edit->cursor = len;
		edit->scroll = edit->cursor - edit->widthInChars;
		return;
	}

	//
	// ignore any other non printable chars
	//
	if ( ch < ' ' ) {
		return;
	}

	if ( key_overstrikeMode ) {
		// - 2 to leave room for the leading slash and trailing \0
		if ( edit->cursor == MAX_EDIT_LINE - 2 )
			return;
		edit->buffer[edit->cursor] = ch;
		edit->cursor++;
	} else {	// insert mode
		// - 2 to leave room for the leading slash and trailing \0
		if ( len == MAX_EDIT_LINE - 2 ) {
			return; // all full
		}
		memmove( edit->buffer + edit->cursor + 1,
			edit->buffer + edit->cursor, len + 1 - edit->cursor );
		edit->buffer[edit->cursor] = ch;
		edit->cursor++;
	}


	if ( edit->cursor >= edit->widthInChars ) {
		edit->scroll++;
	}

	if ( edit->cursor == len + 1) {
		edit->buffer[edit->cursor] = '\0';
	}
}

/*
=============================================================================

CONSOLE LINE EDITING

==============================================================================
*/

/*
====================
Console_Key

Handles history and console scrollback
====================
*/
void Console_Key( uint32_t key ) {
	// ctrl-L clears screen
	if ( key == 'l' && keys[KEY_LCTRL].down ) {
		Cbuf_AddText( "clear\n" );
		return;
	}

	// enter finishes the line
	if ( key == KEY_ENTER || key == KEY_KP_ENTER ) {
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
//				Cbuf_AddText( "cmd say " );
//				Cbuf_AddText( g_consoleField.buffer );
//				Cbuf_AddText( "\n" );
			}
		}

		// copy line to history buffer
		Con_SaveField( &g_consoleField );

		Field_Clear( &g_consoleField );
		g_consoleField.widthInChars = g_console_field_width;

		if ( gi.state == GS_MENU ) {
			SCR_UpdateScreen ();	// force an update, because the command
		}							// may take some time
		return;
	}

	// command completion

	if (key == KEY_TAB) {
		Field_AutoComplete(&g_consoleField);
		return;
	}

	// command history (ctrl-p ctrl-n for unix style)

	if ( (key == KEY_WHEEL_UP && keys[KEY_LSHIFT].down) || ( key == KEY_UP ) ||
		 ( ( tolower(key) == 'p' ) && keys[KEY_LCTRL].down ) ) {
		Con_HistoryGetPrev( &g_consoleField );
		g_consoleField.widthInChars = g_console_field_width;
		return;
	}

	if ( (key == KEY_WHEEL_DOWN && keys[KEY_LSHIFT].down) || ( key == KEY_DOWN ) ||
		 ( ( tolower(key) == 'n' ) && keys[KEY_LCTRL].down ) ) {
		Con_HistoryGetNext( &g_consoleField );
		g_consoleField.widthInChars = g_console_field_width;
		return;
	}

	// console scrolling
	if ( key == KEY_PAGEUP || key == KEY_WHEEL_UP ) {
		if ( keys[KEY_LCTRL].down ) {	// hold <ctrl> to accelerate scrolling
			Con_PageUp( 0 );		// by one visible page
		} else {
			Con_PageUp( 1 );
		}
		return;
	}

	if ( key == KEY_PAGEDOWN || key == KEY_WHEEL_DOWN ) {
		if ( keys[KEY_LCTRL].down ) {	// hold <ctrl> to accelerate scrolling
			Con_PageDown( 0 );		// by one visible page
		} else {
			Con_PageDown( 1 );
		}
		return;
	}

	// ctrl-home = top of console
	if ( key == KEY_HOME && keys[KEY_LCTRL].down ) {
		Con_Top();
		return;
	}

	// ctrl-end = bottom of console
	if ( key == KEY_END && keys[KEY_LCTRL].down ) {
		Con_Bottom();
		return;
	}

	// pass to the normal editline routine
	Field_KeyDownEvent( &g_consoleField, key );
}

static void G_KeyDownEvent(uint32_t key, uint32_t time)
{
	keys[key].down = qtrue;
	keys[key].bound = qfalse;
	keys[key].repeats++;

#ifndef _WIN32
	if (keys[KEY_LALT].down && key == KEY_ENTER) {
		Cvar_SetIntegerValue("r_fullscreen", !Cvar_VariableInteger("r_fullscreen"));
		Cbuf_ExecuteText(EXEC_APPEND, "vid_restart\n");
		return;
	}
#endif

	// console key is hardcoded, so the user can never unbind it
	if (key == KEY_CONSOLE || (keys[KEY_LSHIFT].down && key == KEY_ESCAPE)) {
		Con_ToggleConsole_f();
		return;
	}

	// hardcoded screenshot key
	if (key == KEY_SCREENSHOT) {
		if (keys[KEY_LSHIFT].down) {
			Cbuf_ExecuteText(EXEC_APPEND, "screenshotBMP\n");
		}
		else {
			Cbuf_ExecuteText(EXEC_APPEND, "screenshotBMP clipboard\n");
		}
		return;
	}

	// escape is always handled special
	if ( key == KEY_ESCAPE ) {
		if (Key_GetCatcher() & KEYCATCH_CONSOLE) {
			// escape always closes console
			Con_ToggleConsole_f();
		}
		// escape always gets out of SGAME stuff
		if (Key_GetCatcher() & KEYCATCH_SGAME) {
			Key_SetCatcher(Key_GetCatcher() & ~KEYCATCH_SGAME);
			VM_Call(sgvm, 1, SGAME_EVENT_HANDLING, SGAME_EVENT_NONE);
			return;
		}

		if (!(Key_GetCatcher() & KEYCATCH_UI)) {
			if (gi.state == GS_LEVEL) {
//				VM_Call(uivm, 1, UI_, UI_MENU_PAUSE);
				gi.state = GS_PAUSE;
			}
			else if (gi.state == GS_MENU) {
				Cmd_Clear();
				Cvar_Set("com_errorMessage", "");
				G_FlushMemory();
//				VM_Call(uivm, 1, UI_SET_ACTIVE_MENU, UI_MENU_MAIN);
			}
		}

//		VM_Call(uivm, 2, UI_KEY_EVENT, key, qtrue);
		return;
	}

	// distribute the key down event to the appropriate handler
	if (Key_GetCatcher() & KEYCATCH_SGAME) {
		if (sgvm) {
			VM_Call(sgvm, 2, SGAME_KEY_EVENT, key, qtrue);
		}
	}
	else if (Key_GetCatcher() & KEYCATCH_UI) {
		if (uivm) {
//			VM_Call(uivm, 2, UI_KEY_EVENT, key, qtrue);
		}
	}
	else if (Key_GetCatcher() & KEYCATCH_CONSOLE) {
//		Console_Key(key);
	}
}

static void G_KeyUpEvent(uint32_t key, uint32_t time)
{
	const qboolean bound = keys[key].bound;

	keys[key].repeats = 0;
	keys[key].down = qfalse;
	keys[key].bound = qfalse;

	// don't process key-up events for the console key
	if (key == KEY_CONSOLE || (key == KEY_ESCAPE && keys[KEY_LSHIFT].down)) {
		return;
	}

	// hardcoded screenshot key
	if (key == KEY_SCREENSHOT) {
		return;
	}

	if (Key_GetCatcher() & KEYCATCH_UI) {
		if (uivm) {
//			VM_Call(uivm, 2, UI_KEY_EVENT, key, qfalse);
		}
	}
	else if (Key_GetCatcher() & KEYCATCH_SGAME) {
		if (sgvm) {
			VM_Call(sgvm, 2, SGAME_KEY_EVENT, key, qfalse);
		}
	}
}

void G_CharEvent(uint32_t key)
{
	// delete is not a printable character and is
	// otherwise handled by Field_KeyDownEvent
	if (key == KEY_DELETE) {
		return;
	}

	// distribute the key down event to the appropriate handler
	if (Key_GetCatcher() & KEYCATCH_CONSOLE) {
		Field_CharEvent(&g_consoleField, key);
	}
	else if (Key_GetCatcher() & KEYCATCH_UI) {
//		VM_Call(uivm, 2, UI_KEY_EVENT, key | K_CHAR_FLAG, qtrue);
	}
}

static void Key_Unbind_f( void )
{
	uint32_t b;

	if ( Cmd_Argc() != 2 ) {
		Con_Printf( "unbind <key> : remove commands from a key\n" );
		return;
	}

	b = Key_StringToKeynum( Cmd_Argv( 1 ) );
	if ( b == -1 ) {
		Con_Printf( "\"%s\" isn't a valid key\n", Cmd_Argv( 1 ) );
		return;
	}

	Key_SetBinding( b, "" );
}

void G_KeyEvent(uint32_t key, qboolean down, uint32_t time)
{
	if (down)
		G_KeyDownEvent(key, time);
	else
		G_KeyUpEvent(key, time);
}

void Com_InitKeyCommands( void )
{
	// register client functions
	Cmd_AddCommand( "bind", Key_Bind_f );
	Cmd_SetCommandCompletionFunc( "bind", Key_CompleteBind );
	Cmd_AddCommand( "unbind", Key_Unbind_f );
	Cmd_SetCommandCompletionFunc( "unbind", Key_CompleteUnbind );
	Cmd_AddCommand( "unbindall", Key_Unbindall_f );
	Cmd_AddCommand( "bindlist", Key_Bindlist_f );
}

/*
===================
Key_ClearStates
===================
*/
void Key_ClearStates( void )
{
	for (uint32_t i = 0; i < NUMKEYS; i++) {
		if ( keys[i].down )
			G_KeyEvent( i, qfalse, 0 );

		keys[i].down = qfalse;
		keys[i].repeats = 0;
	}
}


static uint32_t keyCatchers = 0;

/*
====================
Key_GetCatcher
====================
*/
uint32_t Key_GetCatcher( void )
{
	return keyCatchers;
}


/*
====================
Key_SetCatcher
====================
*/
void Key_SetCatcher( uint32_t catcher )
{
	// If the catcher state is changing, clear all key states
	if ( catcher != keyCatchers )
		Key_ClearStates();

	keyCatchers = catcher;
}



static ImGuiKey EngineKeyToImGuiKey(uint32_t key)
{
	switch (key) {
	case KEY_A: return ImGuiKey_A;
    case KEY_B: return ImGuiKey_B;
    case KEY_C: return ImGuiKey_C;
    case KEY_D: return ImGuiKey_D;
    case KEY_E: return ImGuiKey_E;
    case KEY_F: return ImGuiKey_F;
    case KEY_G: return ImGuiKey_G;
    case KEY_H: return ImGuiKey_H;
    case KEY_I: return ImGuiKey_I;
    case KEY_J: return ImGuiKey_J;
    case KEY_K: return ImGuiKey_K;
    case KEY_L: return ImGuiKey_L;
    case KEY_M: return ImGuiKey_M;
    case KEY_N: return ImGuiKey_N;
    case KEY_O: return ImGuiKey_O;
    case KEY_P: return ImGuiKey_P;
    case KEY_Q: return ImGuiKey_Q;
    case KEY_R: return ImGuiKey_R;
    case KEY_S: return ImGuiKey_S;
    case KEY_T: return ImGuiKey_T;
    case KEY_U: return ImGuiKey_U;
    case KEY_V: return ImGuiKey_V;
    case KEY_W: return ImGuiKey_W;
    case KEY_X: return ImGuiKey_X;
    case KEY_Y: return ImGuiKey_Y;
    case KEY_Z: return ImGuiKey_Z;
    case KEY_1: return ImGuiKey_1;
    case KEY_2: return ImGuiKey_2;
    case KEY_3: return ImGuiKey_3;
    case KEY_4: return ImGuiKey_4;
    case KEY_5: return ImGuiKey_5;
    case KEY_6: return ImGuiKey_6;
    case KEY_7: return ImGuiKey_7;
    case KEY_8: return ImGuiKey_8;
    case KEY_9: return ImGuiKey_9;
    case KEY_0: return ImGuiKey_0;
    case KEY_ENTER: return ImGuiKey_Enter;
    case KEY_ESCAPE: return ImGuiKey_Escape;
    case KEY_BACKSPACE: return ImGuiKey_Backspace;
    case KEY_TAB: return ImGuiKey_Tab;
    case KEY_SPACE: return ImGuiKey_Space;
    case KEY_MINUS: return ImGuiKey_Minus;
    case KEY_EQUALS: return ImGuiKey_Equal;
    case KEY_LEFTBRACKET: return ImGuiKey_LeftBracket;
    case KEY_RIGHTBRACKET: return ImGuiKey_RightBracket;
    
	case KEY_BACKSLASH:
    case KEY_ISO_USB_BACKSLASH:
		return ImGuiKey_Backslash;
    
	case KEY_SEMICOLON: return ImGuiKey_Semicolon;
    case KEY_APOSTROPHE: return ImGuiKey_Apostrophe;
    case KEY_CONSOLE: return ImGuiKey_GraveAccent;
    case KEY_COMMA: return ImGuiKey_Comma;
    case KEY_PERIOD: return ImGuiKey_Period;
    case KEY_SLASH: return ImGuiKey_Slash;
    case KEY_CAPSLOCK: return ImGuiKey_CapsLock;
    case KEY_F1: return ImGuiKey_F1;
    case KEY_F2: return ImGuiKey_F2;
    case KEY_F3: return ImGuiKey_F3;
    case KEY_F4: return ImGuiKey_F4;
    case KEY_F5: return ImGuiKey_F5;
    case KEY_F6: return ImGuiKey_F6;
    case KEY_F7: return ImGuiKey_F7;
    case KEY_F8: return ImGuiKey_F8;
    case KEY_F9: return ImGuiKey_F9;
    case KEY_F10: return ImGuiKey_F10;
    case KEY_F11: return ImGuiKey_F11;
    case KEY_F12: return ImGuiKey_F12;
    case KEY_SCREENSHOT: return ImGuiKey_PrintScreen;
    case KEY_SCROLLLOCK: return ImGuiKey_ScrollLock;
    case KEY_PAUSE: return ImGuiKey_Pause;
    case KEY_INSERT: return ImGuiKey_Insert;
    case KEY_HOME: return ImGuiKey_Home;
    case KEY_PAGEUP: return ImGuiKey_PageUp;
    case KEY_DELETE: return ImGuiKey_Delete;
    case KEY_END: return ImGuiKey_End;
    case KEY_PAGEDOWN: return ImGuiKey_PageDown;
    case KEY_RIGHT: return ImGuiKey_RightArrow;
    case KEY_LEFT: return ImGuiKey_LeftArrow;
    case KEY_DOWN: return ImGuiKey_DownArrow;
    case KEY_UP: return ImGuiKey_UpArrow;
    case KEY_NUMLOCKCLEAR:
#ifdef __APPLE__
		break;
#else
		return ImGuiKey_NumLock;
#endif
    case KEY_KP_DIVIDE: return ImGuiKey_KeypadDivide;
    case KEY_KP_MULTIPLY: return ImGuiKey_KeypadMultiply;
    case KEY_KP_MINUS: return ImGuiKey_KeypadSubtract;
    case KEY_KP_PLUS: return ImGuiKey_KeypadAdd;
    case KEY_KP_ENTER: return ImGuiKey_KeypadEnter;
    case KEY_KP_1: return ImGuiKey_Keypad1;
    case KEY_KP_2: return ImGuiKey_Keypad2;
    case KEY_KP_3: return ImGuiKey_Keypad3;
    case KEY_KP_4: return ImGuiKey_Keypad4;
    case KEY_KP_5: return ImGuiKey_Keypad5;
    case KEY_KP_6: return ImGuiKey_Keypad6;
    case KEY_KP_7: return ImGuiKey_Keypad7;
    case KEY_KP_8: return ImGuiKey_Keypad8;
    case KEY_KP_9: return ImGuiKey_Keypad9;
    case KEY_KP_0: return ImGuiKey_Keypad0;
    case KEY_KP_EQUALS: return ImGuiKey_KeypadEqual;
	case KEY_MENU: return ImGuiKey_Menu;
	case KEY_LCTRL: return ImGuiKey_LeftCtrl;
    case KEY_LSHIFT: return ImGuiKey_LeftShift;
    case KEY_LALT: return ImGuiKey_LeftAlt;
    case KEY_RCTRL: return ImGuiKey_RightCtrl;
    case KEY_RSHIFT: return ImGuiKey_RightShift;
    case KEY_RALT: return ImGuiKey_RightAlt;

	case KEY_KP_PERIOD:
    case KEY_F13:
    case KEY_F14:
    case KEY_F15:
    case KEY_F16:
    case KEY_F17:
    case KEY_F18:
    case KEY_F19:
    case KEY_F20:
    case KEY_F21:
    case KEY_F22:
    case KEY_F23:
    case KEY_F24:
	case KEY_HELP:
	case KEY_SELECT:
	case KEY_STOP:
	case KEY_AGAIN:
    case KEY_UNDO:
    case KEY_CUT:
    case KEY_COPY:
    case KEY_PASTE:
    case KEY_FIND:
	case KEY_MUTE:
    case KEY_VOLUMEUP:
    case KEY_VOLUMEDOWN:
	case KEY_KP_COMMA:
    case KEY_KP_EQUALSAS400:
    case KEY_KP_00:
    case KEY_KP_000:
    case KEY_THOUSANDSSEPARATOR:
    case KEY_DECIMALSEPARATOR:
    case KEY_CURRENCYUNIT:
    case KEY_CURRENCYSUBUNIT:
	case KEY_KP_LEFTPAREN:
    case KEY_KP_RIGHTPAREN:
    case KEY_KP_LEFTBRACE:
    case KEY_KP_RIGHTBRACE:
	case KEY_KP_TAB:
    case KEY_KP_BACKSPACE:
    case KEY_KP_A:
    case KEY_KP_B:
    case KEY_KP_C:
    case KEY_KP_D:
    case KEY_KP_E:
    case KEY_KP_F:
    case KEY_KP_XOR:
    case KEY_KP_POWER:
    case KEY_KP_PERCENT:
    case KEY_KP_LESS:
    case KEY_KP_GREATER:
    case KEY_KP_AMPERSAND:
    case KEY_KP_DBLAMPERSAND:
    case KEY_KP_VERTICALBAR:
    case KEY_KP_DBLVERTICALBAR:
    case KEY_KP_COLON:
    case KEY_KP_HASH:
    case KEY_KP_SPACE:
    case KEY_KP_AT:
    case KEY_KP_EXCLAM:
    case KEY_KP_MEMSTORE:
    case KEY_KP_MEMRECALL:
    case KEY_KP_MEMCLEAR:
    case KEY_KP_MEMADD:
    case KEY_KP_MEMSUBTRACT:
    case KEY_KP_MEMMULTIPLY:
    case KEY_KP_MEMDIVIDE:
    case KEY_KP_PLUSMINUS:
    case KEY_KP_CLEAR:
    case KEY_KP_CLEARENTRY:
    case KEY_KP_BINARY:
    case KEY_KP_OCTAL:
    case KEY_KP_DECIMAL:
    case KEY_KP_HEXADECIMAL:
	case KEY_AUDIONEXT:
    case KEY_AUDIOPREV:
    case KEY_AUDIOSTOP:
    case KEY_AUDIOPLAY:
    case KEY_AUDIOMUTE:
	case KEY_BRIGHTNESSDOWN:
    case KEY_BRIGHTNESSUP:
    case KEY_EJECT:
    case KEY_SLEEP:
		break; // no ImGui equivalent
	};
	return ImGuiKey_None;
}