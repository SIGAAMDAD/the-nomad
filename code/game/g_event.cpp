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

	{"ALT", KEY_ALT},
	{"SHIFT", KEY_SHIFT},
	{"CTRL", KEY_CTRL},
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
    case KEY_SHIFT:
        return "SHIFT";
    case KEY_ALT:
        return "ALT";
    case KEY_CTRL:
        return "CTRL";
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
* Key_WriteBindings: Writes lines containing "bind key value"
*/
void Key_WriteBindings( fileHandle_t f )
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
	uint32_t i;

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
			snprintf(cmd, sizeof(cmd), "%c%s %i %i\n", (down) ? '+' : '-', p + 1, key, time);
			Cbuf_AddText(cmd);
			if (down) {
				keys[key].bound = qtrue;
			}
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

static void G_KeyDownEvent(uint32_t key, uint32_t time)
{
	keys[key].down = qtrue;
	keys[key].bound = qfalse;
	keys[key].repeats++;

#ifndef _WIN32
	if (keys[KEY_ALT].down && key == KEY_ENTER) {
		Cvar_SetIntegerValue("r_fullscreen", !Cvar_VariableInteger("r_fullscreen"));
		Cbuf_ExecuteText(EXEC_APPEND, "vid_restart\n");
		return;
	}
#endif

	// console key is hardcoded, so the user can never unbind it
	if ( key == KEY_CONSOLE || (keys[KEY_SHIFT].down && key == KEY_ESCAPE ) ) {
		Con_ToggleConsole_f();
		return;
	}

	// hardcoded screenshot key
	if (key == KEY_SCREENSHOT) {
		if (keys[KEY_SHIFT].down) {
			Cbuf_ExecuteText(EXEC_APPEND, "screenshotBMP\n");
		}
		else {
			Cbuf_ExecuteText(EXEC_APPEND, "screenshotBMP clipboard\n");
		}
		return;
	}

	if ( key == KEY_ESCAPE ) {
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

		if ( !( Key_GetCatcher() & KEYCATCH_UI ) ) {
			Cmd_Clear();
			Cvar_Set( "com_errorMessage", "" );
		}
		return;
	}

	if ( Key_GetCatcher() & KEYCATCH_CONSOLE ) {
	} else if ( Key_GetCatcher() & KEYCATCH_UI ) {
	} else if ( Key_GetCatcher() & KEYCATCH_SGAME ) {
		if ( sgvm ) {
			g_pModuleLib->ModuleCall( sgvm, ModuleOnKeyEvent, 2, key, qtrue );
		}
	} else {
		// send the bound action
		Key_ParseBinding( key, qtrue, time );
	}
}

static void G_KeyUpEvent(uint32_t key, uint32_t time)
{
	const qboolean bound = keys[key].bound;

	keys[key].repeats = 0;
	keys[key].down = qfalse;
	keys[key].bound = qfalse;

	// don't process key-up events for the console key
	if (key == KEY_CONSOLE || (key == KEY_ESCAPE && keys[KEY_SHIFT].down)) {
		return;
	}

	// hardcoded screenshot key
	if (key == KEY_SCREENSHOT) {
		return;
	}
	
	if ( Key_GetCatcher() & KEYCATCH_SGAME ) {
		if ( sgvm ) {
			g_pModuleLib->ModuleCall( sgvm, ModuleOnKeyEvent, 2, key, qfalse );
		}
	}
	else {
		Key_ParseBinding( key, qfalse, time );
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
	switch ( key ) {
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
    case KEY_RIGHT: return ImGuiKey_RightArrow;
    case KEY_LEFT: return ImGuiKey_LeftArrow;
    case KEY_DOWN: return ImGuiKey_DownArrow;
    case KEY_UP: return ImGuiKey_UpArrow;
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