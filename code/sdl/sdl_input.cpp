#include "../engine/n_shared.h"
#include "../engine/n_common.h"
#include "../game/g_game.h"
#include <SDL2/SDL.h>

#define CTRL(a) ((a)-'a'+1)

static cvar_t *in_keyboardDebug;
static cvar_t *in_forceCharset;

static SDL_GameController *gamepad;
static SDL_Joystick *stick;

static qboolean mouseAvailable = qfalse;
static qboolean mouseActive = qfalse;

static cvar_t *in_mouse;

static cvar_t *in_joystick;
static cvar_t *in_joystickThreshold;
static cvar_t *in_joystickNo;
static cvar_t *in_joystickUseAnalog;

static cvar_t *j_pitch;
static cvar_t *j_yaw;
static cvar_t *j_forward;
static cvar_t *j_side;
static cvar_t *j_up;
static cvar_t *j_pitch_axis;
static cvar_t *j_yaw_axis;
static cvar_t *j_forward_axis;
static cvar_t *j_side_axis;
static cvar_t *j_up_axis;


static cvar_t *g_consoleKeys;

static qboolean mouse_focus;
static uint64_t in_eventTime = 0;


/*
===============
IN_PrintKey
===============
*/
static void IN_PrintKey( const SDL_Keysym *keysym, keynum_t key, qboolean down )
{
	if ( down ) {
		Con_Printf( "+ " );
	} else {
		Con_Printf( "  " );
	}

	Con_Printf( "Scancode: 0x%02x(%s) Sym: 0x%02x(%s)",
			keysym->scancode, SDL_GetScancodeName( keysym->scancode ),
			keysym->sym, SDL_GetKeyName( keysym->sym ) );

	if ( keysym->mod & KMOD_LSHIFT ) {
		Con_Printf( " KMOD_LSHIFT" );
	}
	if ( keysym->mod & KMOD_RSHIFT ) {
		Con_Printf( " KMOD_RSHIFT" );
	}
	if ( keysym->mod & KMOD_LCTRL ) {
		Con_Printf( " KMOD_LCTRL" );
	}
	if ( keysym->mod & KMOD_RCTRL ) {
		Con_Printf( " KMOD_RCTRL" );
	}
	if ( keysym->mod & KMOD_LALT ) {
		Con_Printf( " KMOD_LALT" );
	}
	if ( keysym->mod & KMOD_RALT ) {
		Con_Printf( " KMOD_RALT" );
	}
	if ( keysym->mod & KMOD_LGUI ) {
		Con_Printf( " KMOD_LGUI" );
	}
	if ( keysym->mod & KMOD_RGUI ) {
		Con_Printf( " KMOD_RGUI" );
	}
	if ( keysym->mod & KMOD_NUM ) {
		Con_Printf( " KMOD_NUM" );
	}
	if ( keysym->mod & KMOD_CAPS ) {
		Con_Printf( " KMOD_CAPS" );
	}
	if ( keysym->mod & KMOD_MODE ) {
		Con_Printf( " KMOD_MODE" );
	}
	if ( keysym->mod & KMOD_RESERVED ) {
		Con_Printf( " KMOD_RESERVED" );
	}

	Con_Printf( " Q:0x%02x(%s)\n", key, Key_KeynumToString( key ) );
}


#define MAX_CONSOLE_KEYS 16

/*
===============
IN_IsConsoleKey

TODO: If the SDL_Scancode situation improves, use it instead of
      both of these methods
===============
*/
static qboolean IN_IsConsoleKey( keynum_t key, int32_t character )
{
	typedef struct consoleKey_s {
		enum {
			QUAKE_KEY,
			CHARACTER
		} type;
		union {
			keynum_t key;
			int character;
		} u;
	} consoleKey_t;

	static consoleKey_t consoleKeys[ MAX_CONSOLE_KEYS ];
	static int32_t numConsoleKeys = 0;
	int32_t i;

	// Only parse the variable when it changes
	if ( g_consoleKeys->modified ) {
		const char *text_p, *token;

		g_consoleKeys->modified = qfalse;
		text_p = g_consoleKeys->string;
		numConsoleKeys = 0;

		while ( numConsoleKeys < MAX_CONSOLE_KEYS ) {
			consoleKey_t *c = &consoleKeys[ numConsoleKeys ];
			int32_t charCode = 0;

			token = COM_Parse( &text_p );
			if ( !token[ 0 ] ) {
				break;
			}

			charCode = Com_HexStrToInt( token );

			if ( charCode > 0 ) {
				c->type = CHARACTER;
				c->u.character = charCode;
			}
			else {
				c->type = QUAKE_KEY;
				c->u.key = Key_StringToKeynum( token );

				// 0 isn't a key
				if ( c->u.key <= 0 ) {
					continue;
				}
			}

			numConsoleKeys++;
		}
	}

	// If the character is the same as the key, prefer the character
	if ( key == character ) {
		key = 0;
	}

	for ( i = 0; i < numConsoleKeys; i++ ) {
		consoleKey_t *c = &consoleKeys[ i ];

		switch ( c->type ) {
		case QUAKE_KEY: {
			if ( key && c->u.key == key ) {
				return qtrue;
			}
			break; }
		case CHARACTER: {
			if ( c->u.character == character ) {
				return qtrue;
			}
			break; }
		};
	}

	return qfalse;
}


/*
===============
IN_TranslateSDLToQ3Key
===============
*/
static keynum_t IN_TranslateSDLToQ3Key( SDL_Keysym *keysym, qboolean down )
{
	keynum_t key = 0;

	if ( keysym->scancode >= SDL_SCANCODE_1 && keysym->scancode <= SDL_SCANCODE_0 ) {
		// Always map the number keys as such even if they actually map
		// to other characters (eg, "1" is "&" on an AZERTY keyboard).
		// This is required for SDL before 2.0.6, except on Windows
		// which already had this behavior.
		if ( keysym->scancode == SDL_SCANCODE_0 ) {
			key = '0';
		} else {
			key = '1' + keysym->scancode - SDL_SCANCODE_1;
		}
	}
	else if ( in_forceCharset->i > 0 ) {
		if ( keysym->scancode >= SDL_SCANCODE_A && keysym->scancode <= SDL_SCANCODE_Z ) {
			key = 'a' + keysym->scancode - SDL_SCANCODE_A;
		}
		else {
			switch ( keysym->scancode ) {
			case SDL_SCANCODE_MINUS:        key = '-';  break;
			case SDL_SCANCODE_EQUALS:       key = '=';  break;
			case SDL_SCANCODE_LEFTBRACKET:  key = '[';  break;
			case SDL_SCANCODE_RIGHTBRACKET: key = ']';  break;
			case SDL_SCANCODE_NONUSBACKSLASH:
			case SDL_SCANCODE_BACKSLASH:    key = '\\'; break;
			case SDL_SCANCODE_SEMICOLON:    key = ';';  break;
			case SDL_SCANCODE_APOSTROPHE:   key = '\''; break;
			case SDL_SCANCODE_COMMA:        key = ',';  break;
			case SDL_SCANCODE_PERIOD:       key = '.';  break;
			case SDL_SCANCODE_SLASH:        key = '/';  break;
			default:
				/* key = 0 */
				break;
			};
		}
	}

	if ( !key && keysym->sym >= SDLK_SPACE && keysym->sym < SDLK_DELETE ) {
		// These happen to match the ASCII chars
		key = (int)keysym->sym;
	}
	else if ( !key ) {
		switch ( keysym->sym ) {
		case SDLK_PAGEUP:       key = KEY_PGUP;          break;
		case SDLK_KP_9:         key = KEY_KP_PGUP;       break;
		case SDLK_PAGEDOWN:     key = KEY_PGDN;          break;
		case SDLK_KP_3:         key = KEY_KP_PGDN;       break;
		case SDLK_KP_7:         key = KEY_KP_HOME;       break;
		case SDLK_HOME:         key = KEY_HOME;          break;
		case SDLK_KP_1:         key = KEY_KP_END;        break;
		case SDLK_END:          key = KEY_END;           break;
		case SDLK_KP_4:         key = KEY_KP_LEFT;  break;
		case SDLK_LEFT:         key = KEY_LEFT;     break;
		case SDLK_KP_6:         key = KEY_KP_RIGHT; break;
		case SDLK_RIGHT:        key = KEY_RIGHT;    break;
		case SDLK_KP_2:         key = KEY_KP_DOWN;  break;
		case SDLK_DOWN:         key = KEY_DOWN;     break;
		case SDLK_KP_8:         key = KEY_KP_UP;    break;
		case SDLK_UP:           key = KEY_UP;       break;
		case SDLK_ESCAPE:       key = KEY_ESCAPE;        break;
		case SDLK_KP_ENTER:     key = KEY_KP_ENTER;      break;
		case SDLK_RETURN:       key = KEY_ENTER;         break;
		case SDLK_TAB:          key = KEY_TAB;           break;
		case SDLK_F1:           key = KEY_F1;            break;
		case SDLK_F2:           key = KEY_F2;            break;
		case SDLK_F3:           key = KEY_F3;            break;
		case SDLK_F4:           key = KEY_F4;            break;
		case SDLK_F5:           key = KEY_F5;            break;
		case SDLK_F6:           key = KEY_F6;            break;
		case SDLK_F7:           key = KEY_F7;            break;
		case SDLK_F8:           key = KEY_F8;            break;
		case SDLK_F9:           key = KEY_F9;            break;
		case SDLK_F10:          key = KEY_F10;           break;
		case SDLK_F11:          key = KEY_F11;           break;
		case SDLK_F12:          key = KEY_F12;           break;
		case SDLK_F13:          key = KEY_F13;           break;
		case SDLK_F14:          key = KEY_F14;           break;
		case SDLK_F15:          key = KEY_F15;           break;

		case SDLK_BACKSPACE:    key = KEY_BACKSPACE;     break;
		case SDLK_KP_PERIOD:    key = KEY_KP_DEL;        break;
		case SDLK_DELETE:       key = KEY_DEL;           break;
		case SDLK_PAUSE:        key = KEY_PAUSE;         break;

		case SDLK_LSHIFT:
		case SDLK_RSHIFT:       key = KEY_SHIFT;         break;

		case SDLK_LCTRL:
		case SDLK_RCTRL:        key = KEY_CTRL;          break;

#ifdef __APPLE__
		case SDLK_RGUI:
		case SDLK_LGUI:         key = KEY_COMMAND;       break;
#else
		case SDLK_RGUI:
		case SDLK_LGUI:         key = KEY_SUPER;         break;
#endif

		case SDLK_RALT:
		case SDLK_LALT:         key = KEY_ALT;           break;

		case SDLK_KP_5:         key = KEY_KP_5;          break;
		case SDLK_INSERT:       key = KEY_INS;           break;
		case SDLK_KP_0:         key = KEY_KP_INS;        break;
		case SDLK_KP_MULTIPLY:  key = '*'; /*KEY_KP_STAR;*/ break;
		case SDLK_KP_PLUS:      key = KEY_KP_PLUS;       break;
		case SDLK_KP_MINUS:     key = KEY_KP_MINUS;      break;
		case SDLK_KP_DIVIDE:    key = KEY_KP_SLASH;      break;

		case SDLK_MODE:         key = KEY_MODE;          break;
		case SDLK_HELP:         key = KEY_HELP;          break;
		case SDLK_PRINTSCREEN:  key = KEY_PRINT;         break;
		case SDLK_SYSREQ:       key = KEY_SYSREQ;        break;
		case SDLK_MENU:         key = KEY_MENU;          break;
		case SDLK_APPLICATION:	key = KEY_MENU;          break;
		case SDLK_POWER:        key = KEY_POWER;         break;
		case SDLK_UNDO:         key = KEY_UNDO;          break;
		case SDLK_SCROLLLOCK:   key = KEY_SCROLLOCK;     break;
		case SDLK_NUMLOCKCLEAR: key = KEY_KP_NUMLOCK;    break;
		case SDLK_CAPSLOCK:     key = KEY_CAPSLOCK;      break;

		default: {
#if 1
			key = 0;
#else
			if ( !( keysym->sym & SDLK_SCANCODE_MASK ) && keysym->scancode <= 95 ) {
				// Map Unicode characters to 95 world keys using the key's scan code.
				// FIXME: There aren't enough world keys to cover all the scancodes.
				// Maybe create a map of scancode to quake key at start up and on
				// key map change; allocate world key numbers as needed similar
				// to SDL 1.2.
				key = KEY_WORLD_0 + (int)keysym->scancode;
			}
#endif
			break; }
		};
	}

	if ( in_keyboardDebug->i ) {
		IN_PrintKey( keysym, key, down );
	}

	if ( keysym->scancode == SDL_SCANCODE_GRAVE ) {
		//SDL_Keycode translated = SDL_GetKeyFromScancode( SDL_SCANCODE_GRAVE );

		//if ( translated == SDLK_CARET )
		{
			// Console keys can't be bound or generate characters
			key = KEY_CONSOLE;
		}
	}
	else if ( IN_IsConsoleKey( key, 0 ) ) {
		// Console keys can't be bound or generate characters
		key = KEY_CONSOLE;
	}

	return key;
}



/*
===============
IN_GobbleMotionEvents
===============
*/
static void IN_GobbleMouseEvents( void )
{
	SDL_Event dummy[ 1 ];
	int32_t val = 0;

	// Gobble any mouse events
	SDL_PumpEvents();

	while ( ( val = SDL_PeepEvents( dummy, ARRAY_LEN( dummy ), SDL_GETEVENT,
		SDL_MOUSEMOTION, SDL_MOUSEWHEEL ) ) > 0 ) { }

	if ( val < 0 ) {
		Con_Printf( "%s failed: %s\n", __func__, SDL_GetError() );
	}
}


/*
===============
IN_ActivateMouse
===============
*/
static void IN_ActivateMouse( void )
{
	if ( !mouseAvailable ) {
		return;
	}

	if ( !mouseActive ) {
		IN_GobbleMouseEvents();

		SDL_SetRelativeMouseMode( in_mouse->i == 1 ? SDL_TRUE : SDL_FALSE );
		SDL_SetWindowGrab( SDL_window, SDL_TRUE );

		if ( glw_state.isFullscreen )
			SDL_ShowCursor( SDL_FALSE );

		SDL_WarpMouseInWindow( SDL_window, glw_state.window_width / 2, glw_state.window_height / 2 );

#ifdef DEBUG_EVENTS
		Con_Printf( "%4lu %s\n", Sys_Milliseconds(), __func__ );
#endif
	}

	// in_nograb makes no sense in fullscreen mode
	if ( !glw_state.isFullscreen ) {
		if ( in_nograb->modified || !mouseActive ) {
			if ( in_nograb->i ) {
				SDL_SetRelativeMouseMode( SDL_FALSE );
				SDL_SetWindowGrab( SDL_window, SDL_FALSE );
			} else {
				SDL_SetRelativeMouseMode( in_mouse->i == 1 ? SDL_TRUE : SDL_FALSE );
				SDL_SetWindowGrab( SDL_window, SDL_TRUE );
			}

			in_nograb->modified = qfalse;
		}
	}

	mouseActive = qtrue;
}


/*
===============
IN_DeactivateMouse
===============
*/
static void IN_DeactivateMouse( void )
{
	if ( !mouseAvailable ) {
		return;
	}

	if ( mouseActive ) {
#ifdef DEBUG_EVENTS
		Con_Printf( "%4lu %s\n", Sys_Milliseconds(), __func__ );
#endif
		IN_GobbleMouseEvents();

		SDL_SetWindowGrab( SDL_window, SDL_FALSE );
		SDL_SetRelativeMouseMode( SDL_FALSE );

		if ( gw_active ) {
			SDL_WarpMouseInWindow( SDL_window, glw_state.window_width / 2, glw_state.window_height / 2 );
		} else {
			if ( glw_state.isFullscreen ) {
				SDL_ShowCursor( SDL_TRUE );
			}

			SDL_WarpMouseGlobal( glw_state.desktop_width / 2, glw_state.desktop_height / 2 );
		}

		mouseActive = qfalse;
	}

	// Always show the cursor when the mouse is disabled,
	// but not when fullscreen
	if ( !glw_state.isFullscreen ) {
		SDL_ShowCursor( SDL_TRUE );
	}
}

// We translate axes movement into keypresses
static const int32_t joy_keys[16] = {
	KEY_LEFT, KEY_RIGHT,
	KEY_UP, KEY_DOWN,
	KEY_JOY17, K_JOY18,
	KEY_JOY19, K_JOY20,
	KEY_JOY21, K_JOY22,
	KEY_JOY23, K_JOY24,
	KEY_JOY25, K_JOY26,
	KEY_JOY27, K_JOY28
};

// translate hat events into keypresses
// the 4 highest buttons are used for the first hat ...
static const int32_t hat_keys[16] = {
	KEY_JOY29, KEY_JOY30,
	KEY_JOY31, KEY_JOY32,
	KEY_JOY25, KEY_JOY26,
	KEY_JOY27, KEY_JOY28,
	KEY_JOY21, KEY_JOY22,
	KEY_JOY23, KEY_JOY24,
	KEY_JOY17, KEY_JOY18,
	KEY_JOY19, KEY_JOY20
};


struct {
	qboolean buttons[SDL_CONTROLLER_BUTTON_MAX + 1]; // +1 because old max was 16, current SDL_CONTROLLER_BUTTON_MAX is 15
	unsigned int oldaxes;
	int oldaaxes[MAX_JOYSTICK_AXIS];
	unsigned int oldhats;
} stick_state;


/*
===============
IN_InitJoystick
===============
*/
static void IN_InitJoystick( void )
{
	cvar_t *cv;
	int i = 0;
	int total = 0;
	char buf[16384] = "";

	if (gamepad)
		SDL_GameControllerClose(gamepad);

	if (stick != NULL)
		SDL_JoystickClose(stick);

	stick = NULL;
	gamepad = NULL;
	memset(&stick_state, '\0', sizeof (stick_state));

	// SDL 2.0.4 requires SDL_INIT_JOYSTICK to be initialized separately from
	// SDL_INIT_GAMECONTROLLER for SDL_JoystickOpen() to work correctly,
	// despite https://wiki.libsdl.org/SDL_Init (retrieved 2016-08-16)
	// indicating SDL_INIT_JOYSTICK should be initialized automatically.
	if (!SDL_WasInit(SDL_INIT_JOYSTICK))
	{
		Com_DPrintf("Calling SDL_Init(SDL_INIT_JOYSTICK)...\n");
		if (SDL_Init(SDL_INIT_JOYSTICK) != 0)
		{
			Com_DPrintf("SDL_Init(SDL_INIT_JOYSTICK) failed: %s\n", SDL_GetError());
			return;
		}
		Com_DPrintf("SDL_Init(SDL_INIT_JOYSTICK) passed.\n");
	}

	if (!SDL_WasInit(SDL_INIT_GAMECONTROLLER))
	{
		Com_DPrintf("Calling SDL_Init(SDL_INIT_GAMECONTROLLER)...\n");
		if (SDL_Init(SDL_INIT_GAMECONTROLLER) != 0)
		{
			Com_DPrintf("SDL_Init(SDL_INIT_GAMECONTROLLER) failed: %s\n", SDL_GetError());
			return;
		}
		Com_DPrintf("SDL_Init(SDL_INIT_GAMECONTROLLER) passed.\n");
	}

	total = SDL_NumJoysticks();
	Com_DPrintf("%d possible joysticks\n", total);

	// Print list and build cvar to allow ui to select joystick.
	for (i = 0; i < total; i++)
	{
		Q_strcat(buf, sizeof(buf), SDL_JoystickNameForIndex(i));
		Q_strcat(buf, sizeof(buf), "\n");
	}

	cv = Cvar_Get( "in_availableJoysticks", buf, CVAR_ROM );
	Cvar_SetDescription( cv, "List of available joysticks." );

	if( !in_joystick->i ) {
		Com_DPrintf( "Joystick is not active.\n" );
		SDL_QuitSubSystem(SDL_INIT_GAMECONTROLLER);
		return;
	}

	in_joystickNo = Cvar_Get( "in_joystickNo", "0", CVAR_ARCHIVE );
	Cvar_SetDescription( in_joystickNo, "Select which joystick to use." );
	if( in_joystickNo->i < 0 || in_joystickNo->i >= total )
		Cvar_Set( "in_joystickNo", "0" );

	in_joystickUseAnalog = Cvar_Get( "in_joystickUseAnalog", "0", CVAR_ARCHIVE );
	Cvar_SetDescription( in_joystickUseAnalog, "Do not translate joystick axis events to keyboard commands." );

	stick = SDL_JoystickOpen( in_joystickNo->i );

	if (stick == NULL) {
		Com_DPrintf( "No joystick opened: %s\n", SDL_GetError() );
		return;
	}

	if (SDL_IsGameController(in_joystickNo->i))
		gamepad = SDL_GameControllerOpen(in_joystickNo->i);

	Com_DPrintf( "Joystick %d opened\n", in_joystickNo->i );
	Com_DPrintf( "Name:       %s\n", SDL_JoystickNameForIndex(in_joystickNo->i) );
	Com_DPrintf( "Axes:       %d\n", SDL_JoystickNumAxes(stick) );
	Com_DPrintf( "Hats:       %d\n", SDL_JoystickNumHats(stick) );
	Com_DPrintf( "Buttons:    %d\n", SDL_JoystickNumButtons(stick) );
	Com_DPrintf( "Balls:      %d\n", SDL_JoystickNumBalls(stick) );
	Com_DPrintf( "Use Analog: %s\n", in_joystickUseAnalog->i ? "Yes" : "No" );
	Com_DPrintf( "Is gamepad: %s\n", gamepad ? "Yes" : "No" );

	SDL_JoystickEventState(SDL_QUERY);
	SDL_GameControllerEventState(SDL_QUERY);
}


/*
===============
IN_ShutdownJoystick
===============
*/
static void IN_ShutdownJoystick( void )
{
	if ( !SDL_WasInit( SDL_INIT_GAMECONTROLLER ) )
		return;

	if ( !SDL_WasInit( SDL_INIT_JOYSTICK ) )
		return;

	if (gamepad)
	{
		SDL_GameControllerClose(gamepad);
		gamepad = NULL;
	}

	if (stick)
	{
		SDL_JoystickClose(stick);
		stick = NULL;
	}

	SDL_QuitSubSystem(SDL_INIT_GAMECONTROLLER);
	SDL_QuitSubSystem(SDL_INIT_JOYSTICK);
}


static qboolean KeyToAxisAndSign(int keynum, int *outAxis, int *outSign)
{
	const char *bind;

	if (!keynum)
		return qfalse;

	bind = Key_GetBinding(keynum);

	if (!bind || *bind != '+')
		return qfalse;

	*outSign = 0;

	if (Q_stricmp(bind, "+forward") == 0)
	{
		*outAxis = j_forward_axis->i;
		*outSign = j_forward->value > 0.0f ? 1 : -1;
	}
	else if (Q_stricmp(bind, "+back") == 0)
	{
		*outAxis = j_forward_axis->i;
		*outSign = j_forward->value > 0.0f ? -1 : 1;
	}
	else if (Q_stricmp(bind, "+moveleft") == 0)
	{
		*outAxis = j_side_axis->i;
		*outSign = j_side->value > 0.0f ? -1 : 1;
	}
	else if (Q_stricmp(bind, "+moveright") == 0)
	{
		*outAxis = j_side_axis->i;
		*outSign = j_side->value > 0.0f ? 1 : -1;
	}
	else if (Q_stricmp(bind, "+lookup") == 0)
	{
		*outAxis = j_pitch_axis->i;
		*outSign = j_pitch->value > 0.0f ? -1 : 1;
	}
	else if (Q_stricmp(bind, "+lookdown") == 0)
	{
		*outAxis = j_pitch_axis->i;
		*outSign = j_pitch->value > 0.0f ? 1 : -1;
	}
	else if (Q_stricmp(bind, "+left") == 0)
	{
		*outAxis = j_yaw_axis->i;
		*outSign = j_yaw->value > 0.0f ? 1 : -1;
	}
	else if (Q_stricmp(bind, "+right") == 0)
	{
		*outAxis = j_yaw_axis->i;
		*outSign = j_yaw->value > 0.0f ? -1 : 1;
	}
	else if (Q_stricmp(bind, "+moveup") == 0)
	{
		*outAxis = j_up_axis->i;
		*outSign = j_up->value > 0.0f ? 1 : -1;
	}
	else if (Q_stricmp(bind, "+movedown") == 0)
	{
		*outAxis = j_up_axis->i;
		*outSign = j_up->value > 0.0f ? -1 : 1;
	}

	return *outSign != 0;
}


/*
===============
IN_GamepadMove
===============
*/
static void IN_GamepadMove( void )
{
	int i;
	int translatedAxes[MAX_JOYSTICK_AXIS];
	qboolean translatedAxesSet[MAX_JOYSTICK_AXIS];

	SDL_GameControllerUpdate();

	// check buttons
	for (i = 0; i < SDL_CONTROLLER_BUTTON_MAX; i++)
	{
		qboolean pressed = SDL_GameControllerGetButton(gamepad, SDL_CONTROLLER_BUTTON_A + i);
		if (pressed != stick_state.buttons[i])
		{
#if SDL_VERSION_ATLEAST( 2, 0, 14 )
			if ( i >= SDL_CONTROLLER_BUTTON_MISC1 ) {
				Com_QueueEvent(in_eventTime, SE_KEY, KEY_PAD0_MISC1 + i - SDL_CONTROLLER_BUTTON_MISC1, pressed, 0, NULL);
			} else
#endif
			{
				Com_QueueEvent(in_eventTime, SE_KEY, KEY_PAD0_A + i, pressed, 0, NULL);
			}
			stick_state.buttons[i] = pressed;
		}
	}

	// must defer translated axes until all real axes are processed
	// must be done this way to prevent a later mapped axis from zeroing out a previous one
	if (in_joystickUseAnalog->i)
	{
		for (i = 0; i < MAX_JOYSTICK_AXIS; i++)
		{
			translatedAxes[i] = 0;
			translatedAxesSet[i] = qfalse;
		}
	}

	// check axes
	for ( i = 0; i < SDL_CONTROLLER_AXIS_MAX; i++ ) {
		int axis = SDL_GameControllerGetAxis(gamepad, SDL_CONTROLLER_AXIS_LEFTX + i);
		int oldAxis = stick_state.oldaaxes[i];

		// Smoothly ramp from dead zone to maximum value
		float f = ((float)abs(axis) / 32767.0f - in_joystickThreshold->value) / (1.0f - in_joystickThreshold->value);

		if (f < 0.0f)
			f = 0.0f;

		axis = (int)(32767 * ((axis < 0) ? -f : f));

		if (axis != oldAxis)
		{
			const int negMap[SDL_CONTROLLER_AXIS_MAX] = { KEY_PAD0_LEFTSTICK_LEFT,  KEY_PAD0_LEFTSTICK_UP,   KEY_PAD0_RIGHTSTICK_LEFT,  KEY_PAD0_RIGHTSTICK_UP, 0, 0 };
			const int posMap[SDL_CONTROLLER_AXIS_MAX] = { KEY_PAD0_LEFTSTICK_RIGHT, KEY_PAD0_LEFTSTICK_DOWN, KEY_PAD0_RIGHTSTICK_RIGHT, KEY_PAD0_RIGHTSTICK_DOWN, KEY_PAD0_LEFTTRIGGER, KEY_PAD0_RIGHTTRIGGER };

			qboolean posAnalog = qfalse, negAnalog = qfalse;
			int negKey = negMap[i];
			int posKey = posMap[i];

			if (in_joystickUseAnalog->i)
			{
				int posAxis = 0, posSign = 0, negAxis = 0, negSign = 0;

				// get axes and axes signs for keys if available
				posAnalog = KeyToAxisAndSign( posKey, &posAxis, &posSign );
				negAnalog = KeyToAxisAndSign( negKey, &negAxis, &negSign );

				// positive to negative/neutral -> keyup if axis hasn't yet been set
				if ( posAnalog && !translatedAxesSet[posAxis] && oldAxis > 0 && axis <= 0 ) {
					translatedAxes[posAxis] = 0;
					translatedAxesSet[posAxis] = qtrue;
				}

				// negative to positive/neutral -> keyup if axis hasn't yet been set
				if ( negAnalog && !translatedAxesSet[negAxis] && oldAxis < 0 && axis >= 0 ) {
					translatedAxes[negAxis] = 0;
					translatedAxesSet[negAxis] = qtrue;
				}

				// negative/neutral to positive -> keydown
				if ( posAnalog && axis > 0 ) {
					translatedAxes[posAxis] = axis * posSign;
					translatedAxesSet[posAxis] = qtrue;
				}

				// positive/neutral to negative -> keydown
				if ( negAnalog && axis < 0 ) {
					translatedAxes[negAxis] = -axis * negSign;
					translatedAxesSet[negAxis] = qtrue;
				}
			}

			// keyups first so they get overridden by keydowns later

			// positive to negative/neutral -> keyup
			if ( !posAnalog && posKey && oldAxis > 0 && axis <= 0 ) {
				Com_QueueEvent( in_eventTime, SE_KEY, posKey, qfalse, 0, NULL );
			}

			// negative to positive/neutral -> keyup
			if ( !negAnalog && negKey && oldAxis < 0 && axis >= 0 ) {
				Com_QueueEvent( in_eventTime, SE_KEY, negKey, qfalse, 0, NULL );
			}

			// negative/neutral to positive -> keydown
			if ( !posAnalog && posKey && oldAxis <= 0 && axis > 0 )
				Com_QueueEvent( in_eventTime, SE_KEY, posKey, qtrue, 0, NULL );

			// positive/neutral to negative -> keydown
			if ( !negAnalog && negKey && oldAxis >= 0 && axis < 0 ) {
				Com_QueueEvent( in_eventTime, SE_KEY, negKey, qtrue, 0, NULL );
			}

			stick_state.oldaaxes[i] = axis;
		}
	}

	// set translated axes
	if ( in_joystickUseAnalog->i ) {
		for ( i = 0; i < MAX_JOYSTICK_AXIS; i++ ) {
			if ( translatedAxesSet[i] ) {
				Com_QueueEvent( in_eventTime, SE_JOYSTICK_AXIS, i, translatedAxes[i], 0, NULL );
			}
		}
	}
}


static void IN_DeactivateMouse( void )
{

}

static void IN_ShutdownJoystick( void )
{
	if ( !SDL_WasInit( SDL_INIT_GAMECONTROLLER ) ) {
		return;
	}
	if ( !SDL_WasInit( SDL_INIT_JOYSTICK ) ) {
		return;
	}

	if ( gamepad ) {
		SDL_GameControllerClose( gamepad );
		gamepad = NULL;
	}

	if ( stick ) {
		SDL_JoystickClose( stick );
		stick = NULL;
	}

	SDL_QuitSubSystem( SDL_INIT_GAMECONTROLLER );
	SDL_QuitSubSystem( SDL_INIT_JOYSTICK );
}

void HandleEvents( void )
{
	SDL_Event e;
	keynum_t key = 0;
	static keynum_t lastKeyDown = 0;

	if ( !SDL_WasInit( SDL_INIT_VIDEO ) ) {
		return;
	}

	in_eventTime = Sys_Milliseconds();

	while ( SDL_PollEvent( &e ) ) {
		switch ( e.type ) {
		case SDL_KEYDOWN: {
			if ( e.key.repeat && Key_GetCatcher() == 0 ) {
				break;
			}
			key = IN_TranslateSDLToQ3Key( &e.key.keysym, qtrue );
			if ( key == KEY_ENTER && keys[KEY_ALT].down ) {
				Cvar_SetValuei( "r_fullscreen", gi.gpuConfig.isFullscreen ? 0 : 1 );
				Cbuf_AddText( "vid_restart\n" );
				break;
			}
			if ( key ) {
				Com_QueueEvent( in_eventTime, SE_KEY, key, qtrue, 0, NULL );

				if ( key == KEY_BACKSPACE ) {
					Com_QueueEvent( in_eventTime, SE_CHAR, CTRL( 'h' ), 0, 0, NULL );
				} else if ( key == KEY_ESCAPE ) {
					Com_QueueEvent( in_eventTime, SE_CHAR, key, 0, 0, NULL );
				} else if ( keys[KEY_CTRL].down && key >= 'a' && key <= 'z' ) {
					Com_QueueEvent( in_eventTime, SE_CHAR, CTRL( key ), 0, 0, NULL );
				}
			}
			lastKeyDown = key;
			break; }
		case SDL_KEYUP: {
			if ( ( key = IN_TranslateSDLToQ3Key( &e.key.keysym, qfalse ) ) ) {
				Com_QueueEvent( in_eventTime, SE_KEY, key, qfalse, 0, NULL );
			}
			lastKeyDown = 0;
			break; }
		case SDL_TEXTINPUT:
			break;
		case SDL_MOUSEMOTION: {
			break; }
		case SDL_MOUSEBUTTONDOWN:
		case SDL_MOUSEBUTTONUP:
			break;
		};
	}
}

static void IN_Restart( void )
{
	IN_ShutdownJoystick();
	IN_Shutdown();
	IN_Init();
}

void IN_Init( void )
{
    if ( !SDL_WasInit( SDL_INIT_VIDEO ) ) {
		N_Error( ERR_FATAL, "IN_Init called before SDL_Init( SDL_INIT_VIDEO )" );
		return;
	}

	Con_DPrintf( "\n------- Input Initialization -------\n" );

	in_keyboardDebug = Cvar_Get( "in_keyboardDebug", "0", CVAR_SAVE );
	Cvar_SetDescription( in_keyboardDebug, "Print keyboard debug info." );
	in_forceCharset = Cvar_Get( "in_forceCharset", "1", CVAR_ARCHIVE_ND );
	Cvar_SetDescription( in_forceCharset, "Try to translate non-ASCII chars in keyboard input or force EN/US keyboard layout." );

	// mouse variables
	in_mouse = Cvar_Get( "in_mouse", "1", CVAR_SAVE );
	Cvar_CheckRange( in_mouse, "-1", "1", CVT_INT );
	Cvar_SetDescription( in_mouse,
		"Mouse data input source:\n" \
		"  0 - disable mouse input\n" \
		"  1 - di/raw mouse\n" \
		" -1 - win32 mouse" );

	in_joystick = Cvar_Get( "in_joystick", "0", CVAR_SAVE | CVAR_LATCH );
	Cvar_SetDescription( in_joystick, "Whether or not joystick support is on." );
	in_joystickThreshold = Cvar_Get( "joy_threshold", "0.15", CVAR_SAVE );
	Cvar_SetDescription( in_joystickThreshold, "Threshold of joystick moving distance." );

	j_pitch = Cvar_Get( "j_pitch", "0.022", CVAR_ARCHIVE_ND );
	Cvar_SetDescription( j_pitch, "Joystick pitch rotation speed/direction." );
	j_yaw = Cvar_Get( "j_yaw", "-0.022", CVAR_ARCHIVE_ND );
	Cvar_SetDescription( j_yaw, "Joystick yaw rotation speed/direction." );
	j_forward = Cvar_Get( "j_forward", "-0.25", CVAR_ARCHIVE_ND );
	Cvar_SetDescription( j_forward, "Joystick forward movement speed/direction." );
	j_side = Cvar_Get( "j_side", "0.25", CVAR_ARCHIVE_ND );
	Cvar_SetDescription( j_side, "Joystick side movement speed/direction." );
	j_up = Cvar_Get( "j_up", "0", CVAR_ARCHIVE_ND );
	Cvar_SetDescription( j_up, "Joystick up movement speed/direction." );

	j_pitch_axis = Cvar_Get( "j_pitch_axis", "3", CVAR_ARCHIVE_ND );
	Cvar_CheckRange( j_pitch_axis, "0", va( "%i", MAX_JOYSTICK_AXIS - 1 ), CVT_INT );
	Cvar_SetDescription( j_pitch_axis, "Selects which joystick axis controls pitch." );
	j_yaw_axis = Cvar_Get( "j_yaw_axis",     "2", CVAR_ARCHIVE_ND );
	Cvar_CheckRange( j_yaw_axis,     "0", va( "%i", MAX_JOYSTICK_AXIS - 1 ), CVT_INT );
	Cvar_SetDescription( j_yaw_axis, "Selects which joystick axis controls yaw." );
	j_forward_axis = Cvar_Get( "j_forward_axis", "1", CVAR_ARCHIVE_ND );
	Cvar_CheckRange( j_forward_axis, "0", va( "%i", MAX_JOYSTICK_AXIS - 1 ), CVT_INT );
	Cvar_SetDescription( j_forward_axis, "Selects which joystick axis controls forward/back." );
	j_side_axis = Cvar_Get( "j_side_axis", "0", CVAR_ARCHIVE_ND );
	Cvar_CheckRange( j_side_axis "0", va( "%i", MAX_JOYSTICK_AXIS - 1 ), CVT_INT );
	Cvar_SetDescription( j_side_axis, "Selects which joystick axis controls left/right." );
	j_up_axis = Cvar_Get( "j_up_axis", "4", CVAR_ARCHIVE_ND );
	Cvar_CheckRange( j_up_axis, "0", va( "%i", MAX_JOYSTICK_AXIS - 1 ), CVT_INT );
	Cvar_SetDescription( j_up_axis, "Selects which joystick axis controls up/down." );

	// ~ and `, as keys and characters
	cl_consoleKeys = Cvar_Get( "cl_consoleKeys", "~ ` 0x7e 0x60", CVAR_SAVE );
	Cvar_SetDescription( cl_consoleKeys, "Space delimited list of key names or characters that toggle the console." );

	mouseAvailable = ( in_mouse->value != 0 ) ? qtrue : qfalse;

	SDL_StartTextInput();

	//IN_DeactivateMouse();

	IN_InitJoystick();

	Cmd_AddCommand( "input.minimize", IN_Minimize );
	Cmd_AddCommand( "input.restart", IN_Restart );

	Con_DPrintf( "------------------------------------\n" );
}

void IN_Shutdown( void )
{
    SDL_StopTextInput();

    IN_DeactivateMouse();

    mouseAvailable = qfalse;

    Cmd_RemoveCommand( "input.minimize" );
    Cmd_RemoveCommand( "input.restart" );
}

void IN_Frame( void )
{

}
