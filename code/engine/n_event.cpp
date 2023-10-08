#include "n_shared.h"
#include "../rendergl/rgl_public.h"
#include "../game/g_game.h"

typedef struct
{
	const char *name;
	uint32_t keynum;
} keyname_t;

static const keyname_t keynames[] = {
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

	{"LSHIFT", KEY_LSHIFT},
	{"RSHIFT", KEY_RSHIFT},
	{"LCTRL", KEY_LCTRL},
	{"RCTRL", KEY_RCTRL},
	{"TAB", KEY_TAB},
	{"ENTER", KEY_ENTER},
	{"BACKSPACE", KEY_BACKSPACE},

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


nkey_t keys[NUMKEYS];

qboolean Key_IsDown(uint32_t keynum)
{
    if (keynum >= NUMKEYS)
        return qfalse;
    
    return keys[keynum].down;
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
	keys[keynum].binding = Z_Strdup(binding);
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

static void Com_KeyDownEvent(uint32_t key, uint32_t time)
{
	keys[key].down = qtrue;
	keys[key].bound = qfalse;
	keys[key].repeats++;

	// not alpanumerical, scancode to keycode conversion needed
//	if (key < 'a' && key > 'z') {
//		key = SDL_SCANCODE_TO_KEYCODE(key);
//	}

	// console key is hardcoded, so the user can never unbind it
	if (key == KEY_CONSOLE || (keys[KEY_LSHIFT].down && key == KEY_ESCAPE)) {
		if (Key_GetCatcher() & KEYCATCH_CONSOLE) {
			Key_SetCatcher(Key_GetCatcher() & ~KEYCATCH_CONSOLE);
		}
		else {
			Key_SetCatcher(Key_GetCatcher() | KEYCATCH_CONSOLE);
		}
		return;
	}

	// only let the console process the event if its open
	if (Key_GetCatcher() & KEYCATCH_CONSOLE) {
		keys[key].down = qfalse;
		keys[key].repeats--;
		return;
	}

	// hardcoded screenshot key
	if (key == KEY_SCREENSHOT) {
		if (keys[KEY_LSHIFT].down) {
			Cbuf_InsertText("screenshotBMP");
		}
		else {
			Cbuf_InsertText("screenshotBMP clipboard");
		}
		return;
	}

	// escape is always handled special
    if (key == KEY_ESCAPE) {
//        VM_Call();
        return;
    }

//	if (Key_GetCatcher() & KEYCATCH_SCRIPT) {
//		G_CallVM(VM_UI, 2, UI_KEY_EVENT, key, qfalse);
//	}
	if (Key_GetCatcher() & KEYCATCH_SGAME) {
		VM_Call(sgvm, 2, SGAME_KEY_EVENT, key, qtrue);
	}
	if (Key_GetCatcher() & KEYCATCH_UI) {
		VM_Call(uivm, 2, UI_KEY_EVENT, key, qtrue);
	}
}

static void Com_KeyUpEvent(uint32_t key, uint32_t time)
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

void Com_KeyEvent(uint32_t key, qboolean down, uint32_t time)
{
	if (down)
		Com_KeyDownEvent(key, time);
	else
		Com_KeyUpEvent(key, time);
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
			Com_KeyEvent( i, qfalse, 0 );

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
