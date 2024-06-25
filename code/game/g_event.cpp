#include "g_game.h"
#include "../rendercommon/imgui.h"
#include "../rendercommon/imgui_impl_sdl2.h"
#include "../rendercommon/imgui_internal.h"

extern field_t g_consoleField;
qboolean key_overstrikeMode;

typedef struct {
	const char *name;
	uint32_t keynum;
} keyname_t;

static const keyname_t keynames[] = {
	{ "Wheel Down", KEY_WHEEL_DOWN },
	{ "Wheel Up", KEY_WHEEL_UP },

	{ "Mouse Left", KEY_MOUSE_LEFT },
	{ "Mouse Middle", KEY_MOUSE_MIDDLE },
	{ "Mouse Right", KEY_MOUSE_RIGHT },
	{ "Mouse Button 4", KEY_MOUSE_BUTTON_4 },
	{ "Mouse Button 5", KEY_MOUSE_BUTTON_5 },
	{ "a", KEY_A },
	{ "b", KEY_B },
	{ "c", KEY_C },
	{ "d", KEY_D },
	{ "e", KEY_E },
	{ "f", KEY_F },
	{ "g", KEY_G },
	{ "h", KEY_H },
	{ "i", KEY_I },
	{ "j", KEY_J },
	{ "k", KEY_K },
	{ "l", KEY_L },
	{ "m", KEY_M },
	{ "n", KEY_N },
	{ "o", KEY_O },
	{ "p", KEY_P },
	{ "q", KEY_Q },
	{ "r", KEY_R },
	{ "s", KEY_S },
	{ "t", KEY_T },
	{ "u", KEY_U },
	{ "v", KEY_V },
	{ "w", KEY_W },
	{ "x", KEY_X },
	{ "y", KEY_Y },
	{ "z", KEY_Z },

	{ "Escape", KEY_ESCAPE },
	{ "Alt", KEY_ALT },
	{ "Shift", KEY_SHIFT },
	{ "Ctrl", KEY_CTRL },
	{ "Tab", KEY_TAB },
	{ "Enter", KEY_ENTER },
	{ "Backspace", KEY_BACKSPACE },
	{ "Space", KEY_SPACE },

	{ "CONSOLE", KEY_CONSOLE },
	{ "PRINTSCREEN", KEY_SCREENSHOT },

	{ "Up Arrow", KEY_UPARROW },
	{ "Down Arrow", KEY_DOWNARROW },
	{ "Right Arrow", KEY_RIGHTARROW },
	{ "Left Arrow", KEY_LEFTARROW },

	{ "F1", KEY_F1 },
	{ "F2", KEY_F2 },
	{ "F3", KEY_F3 },
	{ "F4", KEY_F4 },
	{ "F5", KEY_F5 },
	{ "F6", KEY_F6 },
	{ "F7", KEY_F7 },
	{ "F8", KEY_F8 },
	{ "F9", KEY_F9 },
	{ "F10", KEY_F10 },
	{ "F11", KEY_F11 },
	{ "F12", KEY_F12 },

	{ "JOY1", KEY_JOY1 },
	{ "JOY2", KEY_JOY2 },
	{ "JOY3", KEY_JOY3 },
	{ "JOY4", KEY_JOY4 },
	{ "JOY5", KEY_JOY5 },
	{ "JOY6", KEY_JOY6 },
	{ "JOY7", KEY_JOY7 },
	{ "JOY8", KEY_JOY8 },
	{ "JOY9", KEY_JOY9 },
	{ "JOY10", KEY_JOY10 },
	{ "JOY11", KEY_JOY11 },
	{ "JOY12", KEY_JOY12 },
	{ "JOY13", KEY_JOY13 },
	{ "JOY14", KEY_JOY14 },
	{ "JOY15", KEY_JOY15 },
	{ "JOY16", KEY_JOY16 },
	{ "JOY17", KEY_JOY17 },
	{ "JOY18", KEY_JOY18 },
	{ "JOY19", KEY_JOY19 },
	{ "JOY20", KEY_JOY20 },
	{ "JOY21", KEY_JOY21 },
	{ "JOY22", KEY_JOY22 },
	{ "JOY23", KEY_JOY23 },
	{ "JOY24", KEY_JOY24 },
	{ "JOY25", KEY_JOY25 },
	{ "JOY26", KEY_JOY26 },
	{ "JOY27", KEY_JOY27 },
	{ "JOY28", KEY_JOY28 },
	{ "JOY29", KEY_JOY29 },
	{ "JOY30", KEY_JOY30 },
	{ "JOY31", KEY_JOY31 },
	{ "JOY32", KEY_JOY32 },

	{ "AUX1", KEY_AUX1 },
	{ "AUX2", KEY_AUX2 },
	{ "AUX3", KEY_AUX3 },
	{ "AUX4", KEY_AUX4 },
	{ "AUX5", KEY_AUX5 },
	{ "AUX6", KEY_AUX6 },
	{ "AUX7", KEY_AUX7 },
	{ "AUX8", KEY_AUX8 },
	{ "AUX9", KEY_AUX9 },
	{ "AUX10", KEY_AUX10 },
	{ "AUX11", KEY_AUX11 },
	{ "AUX12", KEY_AUX12 },
	{ "AUX13", KEY_AUX13 },
	{ "AUX14", KEY_AUX14 },
	{ "AUX15", KEY_AUX15 },
	{ "AUX16", KEY_AUX16 },

	{ "SUPER", KEY_SUPER },
	{ "COMPOSE", KEY_COMPOSE },
	{ "MODE", KEY_MODE },
	{ "HELP", KEY_HELP },
	{ "SYSREQ", KEY_SYSREQ },
	{ "SCROLLLOCK", KEY_SCROLLOCK },
	{ "BREAK", KEY_BREAK },
	{ "MENU", KEY_MENU },
	{ "EURO", KEY_EURO },
	{ "UNDO", KEY_UNDO },

	{ "Gamepad A", KEY_PAD0_A },
	{ "Gamepad B", KEY_PAD0_B },
	{ "Gamepad X", KEY_PAD0_X },
	{ "Gamepad Y", KEY_PAD0_Y },
	{ "Gamepad Back", KEY_PAD0_BACK },
	{ "Gamepad Guide", KEY_PAD0_GUIDE },
	{ "Gamepad Start", KEY_PAD0_START },
	{ "Gamepad LeftClick", KEY_PAD0_LEFTSTICK_CLICK },
	{ "Gamepad RightClick", KEY_PAD0_RIGHTSTICK_CLICK },
	{ "Gamepad LeftButton", KEY_PAD0_LEFTBUTTON },
	{ "Gamepad RightButton", KEY_PAD0_RIGHTBUTTON },
	{ "Gamepad DPad Up", KEY_PAD0_DPAD_UP },
	{ "Gamepad DPad Down", KEY_PAD0_DPAD_DOWN },
	{ "Gamepad DPad Left", KEY_PAD0_DPAD_LEFT },
	{ "Gamepad DPad Right", KEY_PAD0_DPAD_RIGHT },

	{ "Gamepad LeftStick Left", KEY_PAD0_LEFTSTICK_LEFT },
	{ "Gamepad LeftStick Right", KEY_PAD0_LEFTSTICK_RIGHT },
	{ "Gamepad LeftStick Up", KEY_PAD0_LEFTSTICK_UP },
	{ "Gamepad LeftStick Down", KEY_PAD0_LEFTSTICK_DOWN },
	{ "Gamepad RightStick Left", KEY_PAD0_RIGHTSTICK_LEFT },
	{ "Gamepad RightStick Right", KEY_PAD0_RIGHTSTICK_RIGHT },
	{ "Gamepad RightStick Up", KEY_PAD0_RIGHTSTICK_UP },
	{ "Gamepad RightStick Down", KEY_PAD0_RIGHTSTICK_DOWN },
	{ "Gamepad LeftTrigger", KEY_PAD0_LEFTTRIGGER },
	{ "Gamepad RightTrigger", KEY_PAD0_RIGHTTRIGGER },

	{ "Gamepad_MISC1", KEY_PAD0_MISC1 },
	{ "Gamepad_PADDLE1", KEY_PAD0_PADDLE1 },
	{ "Gamepad_PADDLE2", KEY_PAD0_PADDLE2 },
	{ "Gamepad_PADDLE3", KEY_PAD0_PADDLE3 },
	{ "Gamepad_PADDLE4", KEY_PAD0_PADDLE4 },
	{ "Gamepad_TOUCHPAD", KEY_PAD0_TOUCHPAD },

	{ NULL, 0 }
};

static ImGuiKey EngineKeyToImGuiKey( uint32_t key );
static void Field_CharEvent( field_t *edit, uint32_t ch );

void G_MouseEvent( int dx, int dy /*, int time */ )
{
	if ( Key_GetCatcher() & KEYCATCH_SGAME ) {
		g_pModuleLib->ModuleCall( sgvm, ModuleOnMouseEvent, 2, dx, dy );
	} else {
		gi.mouseDx[ gi.mouseIndex ] += dx;
		gi.mouseDy[ gi.mouseIndex ] += dy;
	}
}

/*
* G_JoystickEvent: joystick values stay set until changed
*/
void G_JoystickEvent( int axis, int value, int time ) {
	if ( axis < 0 || axis >= MAX_JOYSTICK_AXIS ) {
		N_Error( ERR_DROP, "G_JoystickEvent: bad axis %i", axis );
	} else {
		gi.joystickAxis[axis] = value;
	}
}

nkey_t keys[NUMKEYS];

qboolean Key_AnyDown( void )
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

static void Key_CompleteBind( const char *argi, uint32_t argnum )
{
	const char *p;

	if ( argnum == 2 ) {
		// skip "bind "
		p = Com_SkipTokens( argi, 1, " " );

		if ( p > argi ) {
			Field_CompleteKeyname();
		}
	}
	else if ( argnum >= 3 ) {
		uint32_t key;

		// skip "bind <key> "
		p = Com_SkipTokens( argi, 2, " " );
		if ( *p == '\0' && ( key = Key_StringToKeynum( Cmd_Argv( 1 ) ) ) >= 0 ) {
			Field_CompleteKeyBind( key );
		}
		else if ( p > argi ) {
			Field_CompleteCommand( p, qtrue, qtrue );
		}
	}
}

static void Key_CompleteUnbind( const char *argi, uint32_t argnum )
{
	if ( argnum ) {
		// skip "unbind "
		const char *p = Com_SkipTokens( argi, 1, " " );

		if ( p > argi ) {
			Field_CompleteKeyname();
		}
	}
}

/*
Key_StringToKeynum: Returns a key number to be used to index keys[] by looking at
the given string.  Single ascii characters return themselves, while the KEY_* names are matched up.

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

	// scan for a text match
	for ( kn = keynames; kn->name ; kn++ ) {
		if ( !N_stricmp( str, kn->name ) ) {
			return kn->keynum;
		}
	}

	// check for hex code
	if ( strlen( str ) == 4 ) {
		uint32_t n = Com_HexStrToInt( str );

		if ( n >= 0 ) {
			return n;
		}
	}

	return -1;
}

/*
* Key_KeynumToString: returns a string (either a single ascii char, a SDLKEY_* name, o a 0x11 hex string) for the
* given keynum
*/
const char *Key_KeynumToString(uint32_t keynum)
{
	const keyname_t *kn;
	static char tinystr[5];
	uint32_t i, j;

	if ( keynum >= NUMKEYS ) {
		return "<OUT OF RANGE>";
	}

	// check for printable ascii (don't use quote)
	if ( keynum > ' ' && keynum < '~' && keynum != '"' && keynum != ';' ) {
		tinystr[0] = keynum;
		tinystr[1] = '\0';
		return tinystr;
	}

	// check for a key string
	for ( kn = keynames; kn->name; kn++ ) {
		if ( keynum == kn->keynum ) {
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

void Key_SetBinding( uint32_t keynum, const char *binding )
{
	if ( keynum >= NUMKEYS ) {
		return;
	}

	// free old binding
	if ( keys[keynum].binding ) {
		Z_Free( keys[keynum].binding );
	}

	// allocate new memory for new binding
	keys[keynum].binding = CopyString( binding );
}

const char *Key_GetBinding( int32_t keynum )
{
	if ( keynum >= NUMKEYS ) {
		return "";
	}

	return keys[keynum].binding;
}

int32_t Key_GetKey( const char *binding )
{
	int i;

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
	int c, b;

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
		if ( keys[b].binding && keys[b].binding[0] ) {
			Con_Printf( "\"%s\" = \"%s\"\n", Cmd_Argv( 1 ), keys[b].binding );
		} else {
			Con_Printf( "\"%s\" is not bound\n", Cmd_Argv( 1 ) );
		}
		
		return;
	}

	// copy the rest of the command line
	Key_SetBinding( b, Cmd_ArgsFrom( 2 ) );
}

/*
* Key_WriteBindings: Writes lines containing "bind key value"
*/
void Key_WriteBindings( fileHandle_t f )
{
	uint32_t i;

	FS_Printf( f, "unbindall" GDR_NEWLINE );

	for ( i = 0 ; i < NUMKEYS ; i++ ) {
		if ( !keys[i].binding || !keys[i].binding[0] ) {
			continue;
		}
		
		FS_Printf( f, "bind \"%s\" \"%s\"" GDR_NEWLINE, Key_KeynumToString( i ), keys[i].binding );
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


void Key_KeynameCompletion( void(*callback)( const char *s ) )
{
	uint32_t i;

	for( i = 0; keynames[ i ].name != NULL; i++ )
		callback( keynames[ i ].name );
}


/*
* Key_ParseBinding: execute the commands in the bind string
*/
void Key_ParseBinding( uint32_t key, qboolean down, uint32_t time )
{
	char buf[MAX_STRING_CHARS], *p, *end;

	if ( !keys[key].binding || keys[key].binding[0] == '\0' ) {
		return;
	}
	
	p = buf;

	N_strncpyz( buf, keys[key].binding, sizeof( buf ) );

	while ( 1 ) {
		while ( isspace( *p ) ) {
			p++;
		}
		
		end = strchr( p, ';' );
		if ( end ) {
			*end = '\0';
		}
		if ( *p == '+' ) {
			// button commands add keynum and time as parameters
			// so that multiple sources can be discriminated and
			// subframe corrected
			char cmd[1024];
			snprintf( cmd, sizeof( cmd ), "%c%s %i %i\n", ( down ) ? '+' : '-', p + 1, key, time );
			Cbuf_AddText( cmd );
			if ( down ) {
				keys[key].bound = qtrue;
			}
		}
		else if ( down ) {
			// normal commands only execute on key press
			Cbuf_AddText( p );
			Cbuf_AddText( "\n" );
		}

		if ( !end ) {
			break;
		}
		
		p = end + 1;
	}
}

static void Field_CharEvent( field_t *edit, int ch );

/*
=============================================================================

EDIT FIELDS

=============================================================================
*/


/*
===================
Field_Draw

Handles horizontal scrolling and cursor blinking
x, y, and width are in pixels
===================
*/
static void Field_VariableSizeDraw( field_t *edit, int x, int y, int width, int size, qboolean showCursor,
		qboolean noColorEscape ) {
	int		len;
	int		drawLen;
	int		prestep;
	int		cursorChar;
	char	str[MAX_STRING_CHARS], *s;
	int		i;
	int		curColor;

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
	if ( size == smallchar_width ) {
		SCR_DrawSmallStringExt( x, y, str, g_color_table[ ColorIndexFromChar( curColor ) ],
			qfalse, noColorEscape );
		if ( len > drawLen + prestep ) {
			SCR_DrawSmallChar( x + ( edit->widthInChars - 1 ) * size, y, '>' );
		}
	} else {
		if ( len > drawLen + prestep ) {
			SCR_DrawStringExt( x + ( edit->widthInChars - 1 ) * BIGCHAR_WIDTH, y, size, ">",
				g_color_table[ ColorIndex( S_COLOR_WHITE ) ], qfalse, noColorEscape );
		}
		// draw big string with drop shadow
		SCR_DrawStringExt( x, y, BIGCHAR_WIDTH, str, g_color_table[ ColorIndexFromChar( curColor ) ],
			qfalse, noColorEscape );
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

		if ( size == smallchar_width ) {
			SCR_DrawSmallChar( x + ( edit->cursor - prestep - i ) * size, y, cursorChar );
		} else {
			str[0] = cursorChar;
			str[1] = '\0';
			SCR_DrawBigString( x + ( edit->cursor - prestep - i ) * BIGCHAR_WIDTH, y, str, 1.0, qfalse );
		}
	}
}


void Field_Draw( field_t *edit, uint32_t x, uint32_t y, uint32_t width, qboolean showCursor, qboolean noColorEscape )
{
	Field_VariableSizeDraw( edit, x, y, width, smallchar_width, showCursor, noColorEscape );
}


void Field_BigDraw( field_t *edit, int x, int y, int width, qboolean showCursor, qboolean noColorEscape )
{
	Field_VariableSizeDraw( edit, x, y, width, bigchar_width, showCursor, noColorEscape );
}


/*
================
Field_Paste
================
*/
static void Field_Paste( field_t *edit ) {
	char	*cbd; // heh heh
	int		pasteLen, i;

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
static void Field_SeekWord( field_t *edit, int direction )
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
static void Field_KeyDownEvent( field_t *edit, int key ) {
	int		len;

	// shift-insert is paste
	if ( ( ( key == KEY_INSERT ) || ( key == KEY_KP_INSERT ) ) && keys[KEY_SHIFT].down ) {
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
	case KEY_RIGHTARROW:
		if ( edit->cursor < len ) {
			if ( keys[ KEY_CTRL ].down ) {
				Field_SeekWord( edit, 1 );
			} else {
				edit->cursor++;
			}
		}
		break;
		case KEY_LEFTARROW:
		if ( edit->cursor > 0 ) {
			if ( keys[ KEY_CTRL ].down ) {
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
static void Field_CharEvent( field_t *edit, int ch ) {
	int		len;

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
/*
static void Console_Key( int key ) {
	// ctrl-L clears screen
	if ( key == 'l' && keys[KEY_CTRL].down ) {
		Cbuf_AddText( "clear\n" );
		return;
	}

	// enter finishes the line
	if ( key == KEY_ENTER || key == KEY_KP_ENTER ) {
		// if not in the game explicitly prepend a slash if needed
		if ( gi.state != GS_LEVEL
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
				Cbuf_AddText( g_consoleField.buffer );
				Cbuf_AddText( "\n" );
			}
		}

		// copy line to history buffer
		Con_SaveField( &g_consoleField );

		Field_Clear( &g_consoleField );
		g_consoleField.widthInChars = g_console_field_width;

		if ( gi.state == GS_INACTIVE ) {
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

	if ( (key == KEY_WHEEL_UP && keys[KEY_SHIFT].down) || ( key == KEY_UP ) || ( key == KEY_KP_UP ) ||
		 ( ( tolower(key) == 'p' ) && keys[KEY_CTRL].down ) ) {
		Con_HistoryGetPrev( &g_consoleField );
		g_consoleField.widthInChars = g_console_field_width;
		return;
	}

	if ( (key == KEY_WHEEL_DOWN && keys[KEY_SHIFT].down) || ( key == KEY_DOWN ) || ( key == KEY_KP_DOWN ) ||
		 ( ( tolower(key) == 'n' ) && keys[KEY_CTRL].down ) ) {
		Con_HistoryGetNext( &g_consoleField );
		g_consoleField.widthInChars = g_console_field_width;
		return;
	}

	// console scrolling
	if ( key == KEY_PAGEUP || key == KEY_WHEEL_UP ) {
		if ( keys[KEY_CTRL].down ) {	// hold <ctrl> to accelerate scrolling
			Con_PageUp( 0 );		// by one visible page
		} else {
			Con_PageUp( 1 );
		}
		return;
	}

	if ( key == KEY_PAGEDOWN || key == KEY_WHEEL_DOWN ) {
		if ( keys[KEY_CTRL].down ) {	// hold <ctrl> to accelerate scrolling
			Con_PageDown( 0 );		// by one visible page
		} else {
			Con_PageDown( 1 );
		}
		return;
	}

	// ctrl-home = top of console
	if ( key == KEY_HOME && keys[KEY_CTRL].down ) {
		Con_Top();
		return;
	}

	// ctrl-end = bottom of console
	if ( key == KEY_END && keys[KEY_CTRL].down ) {
		Con_Bottom();
		return;
	}

	// pass to the normal editline routine
	Field_KeyDownEvent( &g_consoleField, key );
}
*/

//============================================================================

static void G_KeyDownEvent( uint32_t key, uint32_t time )
{
	keys[key].down = qtrue;
	keys[key].bound = qfalse;
	keys[key].repeats++;

#ifndef _WIN32
	if ( keys[KEY_ALT].down && key == KEY_ENTER ) {
		Cvar_SetIntegerValue( "r_fullscreen", !Cvar_VariableInteger( "r_fullscreen" ) );
		Cbuf_ExecuteText( EXEC_APPEND, "vid_restart\n" );
		return;
	}
#endif
	if ( keys[KEY_ALT].down && key == KEY_F4 ) {
		Cbuf_ExecuteText( EXEC_NOW, "quit" );
	}

	// console key is hardcoded, so the user can never unbind it
	if ( key == KEY_CONSOLE || ( keys[KEY_SHIFT].down && key == KEY_ESCAPE ) ) {
		Con_ToggleConsole_f();
		return;
	}

	// hardcoded screenshot key
	if ( key == KEY_SCREENSHOT ) {
		if ( keys[KEY_SHIFT].down ) {
			Cbuf_ExecuteText( EXEC_APPEND, "screenshotBMP\n" );
		} else {
			Cbuf_ExecuteText( EXEC_APPEND, "screenshotBMP clipboard\n" );
		}
		return;
	}

	if ( key == KEY_ESCAPE || ( gi.state == GS_MENU && key == KEY_PAD0_B ) || ( gi.state == GS_LEVEL && key == KEY_PAD0_START ) ) {
		if ( Key_GetCatcher() & KEYCATCH_CONSOLE ) {
			// escape always closes the console
			Con_ToggleConsole_f();
			Key_ClearStates();
		}

		// escape always gets out of SGAME stuff
		if ( Key_GetCatcher() & KEYCATCH_SGAME ) {
			Key_SetCatcher( Key_GetCatcher() & ~KEYCATCH_SGAME );
//			VM_Call( sgvm, 1, SGAME_EVENT_HANDLING, SGAME_EVENT_NONE );
			return;
		}

		if ( gi.mapLoaded ) {
			Cbuf_ExecuteText( EXEC_APPEND, "togglepausemenu\n" );
		}

		if ( Key_GetCatcher() & KEYCATCH_UI || com_errorEntered ) {
			Cmd_Clear();
			Cvar_Set( "com_errorMessage", "" );
		}
		return;
	}

	if ( Key_GetCatcher() & KEYCATCH_CONSOLE ) {
	}
	if ( Key_GetCatcher() & KEYCATCH_UI ) {
	}
	if ( Key_GetCatcher() & KEYCATCH_SGAME ) {
		if ( sgvm ) {
			g_pModuleLib->ModuleCall( sgvm, ModuleOnKeyEvent, 2, key, qtrue );
		}
		// send the bound action
		if ( !( Key_GetCatcher() & KEYCATCH_CONSOLE ) && !g_paused->i ) {
			Key_ParseBinding( key, qtrue, time );
		}
	}
}

static void G_KeyUpEvent( uint32_t key, uint32_t time )
{
	const qboolean bound = keys[key].bound;

	keys[key].repeats = 0;
	keys[key].down = qfalse;
	keys[key].bound = qfalse;

	// don't process key-up events for the console key
	if ( key == KEY_CONSOLE || ( key == KEY_ESCAPE && keys[KEY_SHIFT].down ) ) {
		return;
	}

	// hardcoded screenshot key
	if ( key == KEY_SCREENSHOT ) {
		return;
	}
	
	if ( Key_GetCatcher() & KEYCATCH_SGAME ) {
		if ( sgvm ) {
			g_pModuleLib->ModuleCall( sgvm, ModuleOnKeyEvent, 2, key, qfalse );
		}
		Key_ParseBinding( key, qfalse, time );
	}
	if ( Key_GetCatcher() & KEYCATCH_UI ) {
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

void G_KeyEvent( uint32_t key, qboolean down, uint32_t time )
{
	if ( down ) {
		G_KeyDownEvent( key, time );
	} else {
		G_KeyUpEvent( key, time );
	}
}

static void Key_PrintCatchers_f( void )
{
	Con_Printf( "Key Catcher(s): " );
	
	if ( Key_GetCatcher() & KEYCATCH_CONSOLE ) {
		Con_Printf( "Console " );
	}
	
	if ( Key_GetCatcher() & KEYCATCH_SGAME ) {
		Con_Printf( "SGame " );
	}

	if ( Key_GetCatcher() & KEYCATCH_UI ) {
		Con_Printf( "UI" );
	}

	if ( !Key_GetCatcher() ) {
		Con_Printf( "None" );
	}

	Con_Printf( "\n" );
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
	Cmd_AddCommand( "printcatchers", Key_PrintCatchers_f );
}

/*
===================
Key_ClearStates
===================
*/
void Key_ClearStates( void )
{
	int i;

	for ( i = 0; i < NUMKEYS; i++ ) {
		if ( keys[i].down ) {
			G_KeyEvent( i, qfalse, 0 );
		}

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



static ImGuiKey EngineKeyToImGuiKey( uint32_t key )
{
	switch ( key ) {
	case KEY_PAD0_A: return ImGuiKey_GamepadFaceDown;
	case KEY_PAD0_B: return ImGuiKey_GamepadFaceRight;
	case KEY_PAD0_X: return ImGuiKey_GamepadFaceLeft;
	case KEY_PAD0_Y: return ImGuiKey_GamepadFaceUp;
	case KEY_PAD0_LEFTBUTTON: return ImGuiKey_GamepadL1;
	case KEY_PAD0_RIGHTBUTTON: return ImGuiKey_GamepadR1;
	case KEY_PAD0_LEFTSTICK_CLICK: return ImGuiKey_GamepadL3;
	case KEY_PAD0_LEFTSTICK_UP: return ImGuiKey_GamepadLStickUp;
	case KEY_PAD0_LEFTSTICK_RIGHT: return ImGuiKey_GamepadLStickRight;
	case KEY_PAD0_LEFTSTICK_DOWN: return ImGuiKey_GamepadLStickDown;
	case KEY_PAD0_LEFTSTICK_LEFT: return ImGuiKey_GamepadLStickLeft;
	case KEY_PAD0_RIGHTSTICK_CLICK: return ImGuiKey_GamepadR3;
	case KEY_PAD0_RIGHTSTICK_UP: return ImGuiKey_GamepadRStickUp;
	case KEY_PAD0_RIGHTSTICK_RIGHT: return ImGuiKey_GamepadRStickRight;
	case KEY_PAD0_RIGHTSTICK_DOWN: return ImGuiKey_GamepadRStickDown;
	case KEY_PAD0_RIGHTSTICK_LEFT: return ImGuiKey_GamepadRStickLeft;
	case KEY_PAD0_LEFTTRIGGER: return ImGuiKey_GamepadL2;
	case KEY_PAD0_RIGHTTRIGGER: return ImGuiKey_GamepadR2;
	case KEY_PAD0_BACK: return ImGuiKey_GamepadBack;
	case KEY_PAD0_START: return ImGuiKey_GamepadStart;
	case KEY_PAD0_DPAD_UP: return ImGuiKey_GamepadDpadUp;
	case KEY_PAD0_DPAD_RIGHT: return ImGuiKey_GamepadDpadRight;
	case KEY_PAD0_DPAD_DOWN: return ImGuiKey_GamepadDpadDown;
	case KEY_PAD0_DPAD_LEFT: return ImGuiKey_GamepadDpadLeft;
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
    case '1': return ImGuiKey_1;
    case '2': return ImGuiKey_2;
    case '3': return ImGuiKey_3;
    case '4': return ImGuiKey_4;
    case '5': return ImGuiKey_5;
    case '6': return ImGuiKey_6;
    case '7': return ImGuiKey_7;
    case '8': return ImGuiKey_8;
    case '9': return ImGuiKey_9;
    case '0': return ImGuiKey_0;
    case KEY_ENTER: return ImGuiKey_Enter;
    case KEY_ESCAPE: return ImGuiKey_Escape;
    case KEY_BACKSPACE: return ImGuiKey_Backspace;
    case KEY_TAB: return ImGuiKey_Tab;
    case KEY_SPACE: return ImGuiKey_Space;
    case KEY_MINUS: return ImGuiKey_Minus;
    case KEY_EQUAL: return ImGuiKey_Equal;
    case KEY_BRACKET_OPEN: return ImGuiKey_LeftBracket;
    case KEY_BRACKET_CLOSE: return ImGuiKey_RightBracket;
	case KEY_BACKSLASH: return ImGuiKey_Backslash;    
	case KEY_SEMICOLON: return ImGuiKey_Semicolon;
    case KEY_CONSOLE: return ImGuiKey_GraveAccent;
    case KEY_COMMA: return ImGuiKey_Comma;
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
    case KEY_PAUSE: return ImGuiKey_Pause;
    case KEY_INSERT: return ImGuiKey_Insert;
    case KEY_HOME: return ImGuiKey_Home;
    case KEY_PAGEUP: return ImGuiKey_PageUp;
    case KEY_DELETE: return ImGuiKey_Delete;
    case KEY_END: return ImGuiKey_End;
    case KEY_PAGEDOWN: return ImGuiKey_PageDown;
    case KEY_RIGHTARROW: return ImGuiKey_RightArrow;
    case KEY_LEFTARROW: return ImGuiKey_LeftArrow;
    case KEY_DOWNARROW: return ImGuiKey_DownArrow;
    case KEY_UPARROW: return ImGuiKey_UpArrow;
    case KEY_KP_MINUS: return ImGuiKey_KeypadSubtract;
    case KEY_KP_PLUS: return ImGuiKey_KeypadAdd;
    case KEY_KP_ENTER: return ImGuiKey_KeypadEnter;
    case KEY_KP_EQUALS: return ImGuiKey_KeypadEqual;
	case KEY_MENU: return ImGuiKey_Menu;
	case KEY_CTRL: return ImGuiKey_LeftCtrl;
    case KEY_SHIFT: return ImGuiKey_LeftShift;
	case KEY_HELP:
	case KEY_UNDO:
    	break; // no ImGui equivalent
	};
	return ImGuiKey_None;
}