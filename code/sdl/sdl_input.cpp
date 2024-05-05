#include "../engine/n_shared.h"
#include "../engine/n_common.h"
#include "../game/g_game.h"
#ifdef USE_LOCAL_HEADERS
#   include "SDL2/SDL.h"
#   include "SDL2/SDL_haptic.h"
#else
#   include <SDL2/SDL.h>
#   include <SDL2/SDL_haptic.h>
#endif
#include "sdl_glw.h"
#include "../rendercommon/imgui_impl_sdl2.h"

#define CTRL(a) ((a)-'a'+1)

static cvar_t *in_keyboardDebug;
static cvar_t *in_forceCharset;

SDL_GameController *gamepad;
SDL_Haptic *haptic;
SDL_Joystick *stick;

static qboolean mouseAvailable = qfalse;
static qboolean mouseActive = qfalse;

static cvar_t *in_mouse;

cvar_t *in_joystick;
cvar_t *in_joystickThreshold;
cvar_t *in_joystickNo;
cvar_t *in_joystickUseAnalog;
cvar_t *in_haptic;
cvar_t *in_mode;

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
		text_p = g_consoleKeys->s;
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
				c->type = consoleKey_t::CHARACTER;
				c->u.character = charCode;
			}
			else {
				c->type = consoleKey_t::QUAKE_KEY;
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
		case consoleKey_t::QUAKE_KEY: {
			if ( key && c->u.key == key ) {
				return qtrue;
			}
			break; }
		case consoleKey_t::CHARACTER: {
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
		case SDLK_PAGEUP:       key = KEY_PAGEUP;        break;
		case SDLK_KP_9:         key = KEY_KP_PAGEUP;     break;
		case SDLK_PAGEDOWN:     key = KEY_PAGEDOWN;      break;
		case SDLK_KP_3:         key = KEY_KP_PAGEDOWN;   break;
		case SDLK_KP_7:         key = KEY_KP_HOME;       break;
		case SDLK_HOME:         key = KEY_HOME;          break;
		case SDLK_KP_1:         key = KEY_KP_END;        break;
		case SDLK_END:          key = KEY_END;           break;
		case SDLK_KP_4:         key = KEY_KP_LEFT;       break;
		case SDLK_LEFT:         key = KEY_LEFT;          break;
		case SDLK_KP_6:         key = KEY_KP_RIGHT;      break;
		case SDLK_RIGHT:        key = KEY_RIGHT;         break;
		case SDLK_KP_2:         key = KEY_KP_DOWN;       break;
		case SDLK_DOWN:         key = KEY_DOWN;          break;
		case SDLK_KP_8:         key = KEY_KP_UP;         break;
		case SDLK_UP:           key = KEY_UP;            break;
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
		case SDLK_BACKSPACE:    key = KEY_BACKSPACE;     break;
		case SDLK_KP_PERIOD:    key = KEY_KP_DELETE;     break;
		case SDLK_DELETE:       key = KEY_DELETE;        break;
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
		case SDLK_INSERT:       key = KEY_INSERT;        break;
		case SDLK_KP_0:         key = KEY_KP_INSERT;     break;
		case SDLK_KP_MULTIPLY:  key = '*'; /*KEY_KP_STAR;*/ break;
		case SDLK_KP_PLUS:      key = KEY_KP_PLUS;       break;
		case SDLK_KP_MINUS:     key = KEY_KP_MINUS;      break;
		case SDLK_KP_DIVIDE:    key = KEY_KP_SLASH;      break;

		case SDLK_MODE:         key = KEY_MODE;          break;
		case SDLK_HELP:         key = KEY_HELP;          break;
		case SDLK_PRINTSCREEN:  key = KEY_SCREENSHOT;    break;
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

	while ( ( val = SDL_PeepEvents( dummy, arraylen( dummy ), SDL_GETEVENT,
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

//		SDL_SetRelativeMouseMode( in_mouse->i == 1 ? SDL_TRUE : SDL_FALSE );
//		SDL_SetWindowGrab( SDL_window, SDL_TRUE );

//		if ( glw_state.isFullscreen )
//			SDL_ShowCursor( SDL_FALSE );

//		SDL_WarpMouseInWindow( SDL_window, glw_state.window_width / 2, glw_state.window_height / 2 );

#ifdef DEBUG_EVENTS
		Con_Printf( "%4lu %s\n", Sys_Milliseconds(), __func__ );
#endif
	}

	// in_nograb makes no sense in fullscreen mode
	if ( !glw_state.isFullscreen ) {
		if ( in_nograb->modified || !mouseActive ) {
			if ( in_nograb->i ) {
//				SDL_SetRelativeMouseMode( SDL_FALSE );
//				SDL_SetWindowGrab( SDL_window, SDL_FALSE );
			} else {
//				SDL_SetRelativeMouseMode( in_mouse->i == 1 ? SDL_TRUE : SDL_FALSE );
//				SDL_SetWindowGrab( SDL_window, SDL_TRUE );
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

//		SDL_SetWindowGrab( SDL_window, SDL_FALSE );
//		SDL_SetRelativeMouseMode( SDL_FALSE );

		if ( gw_active ) {
//			SDL_WarpMouseInWindow( SDL_window, glw_state.window_width / 2, glw_state.window_height / 2 );
		} else {
			if ( glw_state.isFullscreen ) {
//				SDL_ShowCursor( SDL_TRUE );
			}

//			SDL_WarpMouseGlobal( glw_state.desktop_width / 2, glw_state.desktop_height / 2 );
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
	KEY_JOY17, KEY_JOY18,
	KEY_JOY19, KEY_JOY20,
	KEY_JOY21, KEY_JOY22,
	KEY_JOY23, KEY_JOY24,
	KEY_JOY25, KEY_JOY26,
	KEY_JOY27, KEY_JOY28
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

static void IN_HapticRumble( void )
{
	uint32_t length;
	float strength;

	if ( !in_haptic->i || !haptic ) {
		Con_Printf( "Haptic device not active.\n" );
		return;
	}

	if ( Cmd_Argc() != 3 ) {
		Con_Printf( "usage: in_haptic_rumble <strength> <duration (milliseconds)>\n" );
		return;
	}

	length = (uint32_t)atol( Cmd_Argv( 2 ) );

	strength = N_atof( Cmd_Argv( 1 ) );
	if ( N_isnan( strength ) ) {
		Con_Printf( "IN_HapticRumble: strength is NaN\n" );
		return;
	}

	Con_DPrintf( "Activating haptic rumble: %0.02f %ums\n", strength, length );

	if ( SDL_HapticRumblePlay( haptic, strength, length ) != 0 ) {
		Con_Printf( "Haptic rumble failed: %s\n", SDL_GetError() );
	}
}

static void IN_InitHaptic( void )
{
	int total = 0;
	int index = 0;

	if ( !in_haptic->i || !in_joystick->i || !stick ) {
		return;
	}

	if ( haptic ) {
		SDL_HapticClose( haptic );
	}

	haptic = NULL;

	if ( !SDL_WasInit( SDL_INIT_HAPTIC ) ) {
		Con_Printf( "Calling SDL_Init(SDL_INIT_HAPTIC)...\n" );
		if ( SDL_Init( SDL_INIT_HAPTIC ) != 0 ) {
			Con_Printf( "SDL_Init(SDL_INIT_HAPTIC) failed: %s\n", SDL_GetError() );
			return;
		}
		Con_Printf( "SDL_Init(SDL_INIT_HAPTIC) passed.\n" );
	}

	total = SDL_NumHaptics();
	Con_Printf( "%i possible haptic devices\n", total );
	
	if ( !SDL_JoystickIsHaptic( stick ) ) {
		Con_Printf( "Current controller doesn't support haptic feedback.\n" );
		return;
	}

	haptic = SDL_HapticOpenFromJoystick( stick );
	if ( !haptic ) {
		Con_Printf( "No haptic device opened: %s\n", SDL_GetError() );
		return;
	}

	index = SDL_HapticIndex( haptic );
	if ( index == -1 ) {
		SDL_HapticClose( haptic );
		Con_Printf( "Haptic device index is negative: %s\n", SDL_GetError() );
		return;
	}

	if ( SDL_HapticRumbleInit( haptic ) != 0 ) {
		SDL_HapticClose( haptic );
		Con_Printf( "Haptic device rumble could not be initialized: %s\n", SDL_GetError() );
		return;
	}

	Con_Printf( "Haptic Device %i opened\n", index );
	Con_Printf( "Name:       %s\n", SDL_HapticName( index ) );
	Con_Printf( "Axes:       %i\n", SDL_HapticNumAxes( haptic ) );
	Con_Printf( "MaxEffects: %i\n", SDL_HapticNumEffects( haptic ) );
}

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

	if ( gamepad ) {
		SDL_GameControllerClose( gamepad );
	}

	if ( stick != NULL ) {
		SDL_JoystickClose( stick );
	}

	stick = NULL;
	gamepad = NULL;
	memset( &stick_state, '\0', sizeof( stick_state ) );

	// SDL 2.0.4 requires SDL_INIT_JOYSTICK to be initialized separately from
	// SDL_INIT_GAMECONTROLLER for SDL_JoystickOpen() to work correctly,
	// despite https://wiki.libsdl.org/SDL_Init (retrieved 2016-08-16)
	// indicating SDL_INIT_JOYSTICK should be initialized automatically.
	if ( !SDL_WasInit( SDL_INIT_JOYSTICK ) ) {
		Con_Printf( "Calling SDL_Init(SDL_INIT_JOYSTICK)...\n" );
		if ( SDL_Init( SDL_INIT_JOYSTICK ) != 0 ) {
			Con_Printf( "SDL_Init(SDL_INIT_JOYSTICK) failed: %s\n", SDL_GetError() );
			return;
		}
		Con_Printf( "SDL_Init(SDL_INIT_JOYSTICK) passed.\n" );
	}

	if ( !SDL_WasInit( SDL_INIT_GAMECONTROLLER ) ) {
		Con_Printf( "Calling SDL_Init(SDL_INIT_GAMECONTROLLER)...\n" );
		if ( SDL_Init( SDL_INIT_GAMECONTROLLER ) != 0 ) {
			Con_Printf( "SDL_Init(SDL_INIT_GAMECONTROLLER) failed: %s\n", SDL_GetError() );
			return;
		}
		Con_Printf( "SDL_Init(SDL_INIT_GAMECONTROLLER) passed.\n" );
	}

	total = SDL_NumJoysticks();
	Con_Printf( "%i possible joysticks\n", total );

	// Print list and build cvar to allow ui to select joystick.
	for ( i = 0; i < total; i++ ) {
		N_strcat( buf, sizeof( buf ), SDL_JoystickNameForIndex( i ) );
		if ( buf[ strlen( buf ) - 1 ] != '\n' ) {
			N_strcat( buf, sizeof( buf ), "\n" );
		}
	}

	cv = Cvar_Get( "in_availableJoysticks", buf, CVAR_ROM );
	Cvar_SetDescription( cv, "List of available joysticks." );

	if ( !in_joystick->i ) {
		Con_DPrintf( "Joystick is not active.\n" );
		SDL_QuitSubSystem( SDL_INIT_GAMECONTROLLER );
		return;
	}

	in_joystickNo = Cvar_Get( "in_joystickNo", "0", CVAR_SAVE );
	Cvar_SetDescription( in_joystickNo, "Select which joystick to use." );
	if ( in_joystickNo->i < 0 || in_joystickNo->i >= total ) {
		Cvar_Set( "in_joystickNo", "0" );
	}

	in_joystickUseAnalog = Cvar_Get( "in_joystickUseAnalog", "0", CVAR_SAVE );
	Cvar_SetDescription( in_joystickUseAnalog, "Do not translate joystick axis events to keyboard commands." );

	stick = SDL_JoystickOpen( in_joystickNo->i );

	if ( stick == NULL ) {
		Con_Printf( "No joystick opened: %s\n", SDL_GetError() );
		return;
	}

	IN_InitHaptic();

	if ( SDL_IsGameController( in_joystickNo->i ) ) {
		gamepad = SDL_GameControllerOpen( in_joystickNo->i );
	}

	Con_Printf( "Joystick %li opened\n", in_joystickNo->i );
	Con_Printf( "Name:       %s\n", SDL_JoystickNameForIndex( in_joystickNo->i ) );
	Con_Printf( "Axes:       %i\n", SDL_JoystickNumAxes( stick ) );
	Con_Printf( "Hats:       %i\n", SDL_JoystickNumHats( stick ) );
	Con_Printf( "Buttons:    %i\n", SDL_JoystickNumButtons( stick ) );
	Con_Printf( "Balls:      %i\n", SDL_JoystickNumBalls( stick ) );
	Con_Printf( "Use Analog: %s\n", in_joystickUseAnalog->i ? "Yes" : "No" );
	Con_Printf( "Is gamepad: %s\n", gamepad ? "Yes" : "No" );

	SDL_JoystickEventState( SDL_QUERY );
	SDL_GameControllerEventState( SDL_QUERY );
}


/*
===============
IN_ShutdownJoystick
===============
*/
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

	if ( SDL_WasInit( SDL_INIT_HAPTIC ) ) {
		SDL_HapticClose( haptic );
		haptic = NULL;
		SDL_QuitSubSystem( SDL_INIT_HAPTIC );
	}

	SDL_QuitSubSystem( SDL_INIT_GAMECONTROLLER );
	SDL_QuitSubSystem( SDL_INIT_JOYSTICK );
}


static qboolean KeyToAxisAndSign( int keynum, int *outAxis, int *outSign )
{
	const char *bind;

	if ( !keynum ) {
		return qfalse;
	}

	bind = Key_GetBinding( keynum );

	if ( !bind || *bind != '+' ) {
		return qfalse;
	}

	*outSign = 0;

	if (N_stricmp(bind, "+northmove") == 0)
	{
		*outAxis = j_forward_axis->i;
		*outSign = j_forward->f > 0.0f ? 1 : -1;
	}
	else if (N_stricmp(bind, "+southmove") == 0)
	{
		*outAxis = j_forward_axis->i;
		*outSign = j_forward->f > 0.0f ? -1 : 1;
	}
	else if (N_stricmp(bind, "+westmove") == 0)
	{
		*outAxis = j_side_axis->i;
		*outSign = j_side->f > 0.0f ? -1 : 1;
	}
	else if (N_stricmp(bind, "+eastmove") == 0)
	{
		*outAxis = j_side_axis->i;
		*outSign = j_side->f > 0.0f ? 1 : -1;
	}
	/*
	else if (N_stricmp(bind, "+lookup") == 0)
	{
		*outAxis = j_pitch_axis->i;
		*outSign = j_pitch->f > 0.0f ? -1 : 1;
	}
	else if (N_stricmp(bind, "+lookdown") == 0)
	{
		*outAxis = j_pitch_axis->i;
		*outSign = j_pitch->f > 0.0f ? 1 : -1;
	}
	else if (N_stricmp(bind, "+left") == 0)
	{
		*outAxis = j_yaw_axis->i;
		*outSign = j_yaw->f > 0.0f ? 1 : -1;
	}
	else if (N_stricmp(bind, "+right") == 0)
	{
		*outAxis = j_yaw_axis->i;
		*outSign = j_yaw->f > 0.0f ? -1 : 1;
	}
	else if (N_stricmp(bind, "+moveup") == 0)
	{
		*outAxis = j_up_axis->i;
		*outSign = j_up->f > 0.0f ? 1 : -1;
	}
	else if (N_stricmp(bind, "+movedown") == 0)
	{
		*outAxis = j_up_axis->i;
		*outSign = j_up->f > 0.0f ? -1 : 1;
	}
	*/

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
	for ( i = 0; i < SDL_CONTROLLER_BUTTON_MAX; i++ ) {
		qboolean pressed = SDL_GameControllerGetButton( gamepad, (SDL_GameControllerButton)( SDL_CONTROLLER_BUTTON_A + i ) );
		if ( pressed != stick_state.buttons[i] ) {
			if ( in_mode->i == 0 ) {
				Cvar_Set( "in_mode", "1" );
			}
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
		int axis = SDL_GameControllerGetAxis( gamepad, (SDL_GameControllerAxis)( SDL_CONTROLLER_AXIS_LEFTX + i ) );
		int oldAxis = stick_state.oldaaxes[i];

		// Smoothly ramp from dead zone to maximum f
		float f = ((float)abs(axis) / 32767.0f - in_joystickThreshold->f) / (1.0f - in_joystickThreshold->f);

		if ( f < 0.0f ) {
			f = 0.0f;
		}

		axis = (int)(32767 * ((axis < 0) ? -f : f));

		if ( axis != oldAxis ) {
			if ( in_mode->i == 0 ) {
				Cvar_Set( "in_mode", "1" );
			}
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
				if ( in_mode->i == 0 ) {
					Cvar_Set( "in_mode", "1" );
				}
				Com_QueueEvent( in_eventTime, SE_JOYSTICK_AXIS, i, translatedAxes[i], 0, NULL );
			}
		}
	}
}

/*
===============
IN_JoyMove
===============
*/
static void IN_JoyMove( void )
{
	unsigned int axes = 0;
	unsigned int hats = 0;
	int total = 0;
	int i = 0;

	in_eventTime = Sys_Milliseconds();

	if (gamepad)
	{
		IN_GamepadMove();
		return;
	}

	if (!stick)
		return;

	SDL_JoystickUpdate();

	// update the ball state.
	total = SDL_JoystickNumBalls(stick);
	if (total > 0)
	{
		int balldx = 0;
		int balldy = 0;
		for (i = 0; i < total; i++)
		{
			int dx = 0;
			int dy = 0;
			SDL_JoystickGetBall(stick, i, &dx, &dy);
			balldx += dx;
			balldy += dy;
		}
		if (balldx || balldy)
		{
			// !!! FIXME: is this good for stick balls, or just mice?
			// Scale like the mouse input...
			if (abs(balldx) > 1)
				balldx *= 2;
			if (abs(balldy) > 1)
				balldy *= 2;
			
			if ( in_mode->i == 0 ) {
				Cvar_Set( "in_mode", "1" );
			}
			Com_QueueEvent( in_eventTime, SE_MOUSE, balldx, balldy, 0, NULL );
		}
	}

	// now query the stick buttons...
	total = SDL_JoystickNumButtons(stick);
	if (total > 0)
	{
		if (total > arraylen(stick_state.buttons))
			total = arraylen(stick_state.buttons);
		for (i = 0; i < total; i++)
		{
			qboolean pressed = (SDL_JoystickGetButton(stick, i) != 0);
			if (pressed != stick_state.buttons[i])
			{
				if ( in_mode->i == 0 ) {
					Cvar_Set( "in_mode", "1" );
				}
				Com_QueueEvent( in_eventTime, SE_KEY, KEY_JOY1 + i, pressed, 0, NULL );
				stick_state.buttons[i] = pressed;
			}
		}
	}

	// look at the hats...
	total = SDL_JoystickNumHats(stick);
	if (total > 0)
	{
		if (total > 4) total = 4;
		for (i = 0; i < total; i++)
		{
			((Uint8 *)&hats)[i] = SDL_JoystickGetHat(stick, i);
		}
	}

	// update hat state
	if (hats != stick_state.oldhats)
	{
		for( i = 0; i < 4; i++ ) {
			if( ((Uint8 *)&hats)[i] != ((Uint8 *)&stick_state.oldhats)[i] ) {
				// release event
				if ( in_mode->i == 0 ) {
					Cvar_Set( "in_mode", "1" );
				}
				switch( ((Uint8 *)&stick_state.oldhats)[i] ) {
					case SDL_HAT_UP:
						Com_QueueEvent( in_eventTime, SE_KEY, hat_keys[4*i + 0], qfalse, 0, NULL );
						break;
					case SDL_HAT_RIGHT:
						Com_QueueEvent( in_eventTime, SE_KEY, hat_keys[4*i + 1], qfalse, 0, NULL );
						break;
					case SDL_HAT_DOWN:
						Com_QueueEvent( in_eventTime, SE_KEY, hat_keys[4*i + 2], qfalse, 0, NULL );
						break;
					case SDL_HAT_LEFT:
						Com_QueueEvent( in_eventTime, SE_KEY, hat_keys[4*i + 3], qfalse, 0, NULL );
						break;
					case SDL_HAT_RIGHTUP:
						Com_QueueEvent( in_eventTime, SE_KEY, hat_keys[4*i + 0], qfalse, 0, NULL );
						Com_QueueEvent( in_eventTime, SE_KEY, hat_keys[4*i + 1], qfalse, 0, NULL );
						break;
					case SDL_HAT_RIGHTDOWN:
						Com_QueueEvent( in_eventTime, SE_KEY, hat_keys[4*i + 2], qfalse, 0, NULL );
						Com_QueueEvent( in_eventTime, SE_KEY, hat_keys[4*i + 1], qfalse, 0, NULL );
						break;
					case SDL_HAT_LEFTUP:
						Com_QueueEvent( in_eventTime, SE_KEY, hat_keys[4*i + 0], qfalse, 0, NULL );
						Com_QueueEvent( in_eventTime, SE_KEY, hat_keys[4*i + 3], qfalse, 0, NULL );
						break;
					case SDL_HAT_LEFTDOWN:
						Com_QueueEvent( in_eventTime, SE_KEY, hat_keys[4*i + 2], qfalse, 0, NULL );
						Com_QueueEvent( in_eventTime, SE_KEY, hat_keys[4*i + 3], qfalse, 0, NULL );
						break;
					default:
						break;
				}
				// press event
				switch( ((Uint8 *)&hats)[i] ) {
					case SDL_HAT_UP:
						Com_QueueEvent( in_eventTime, SE_KEY, hat_keys[4*i + 0], qtrue, 0, NULL );
						break;
					case SDL_HAT_RIGHT:
						Com_QueueEvent( in_eventTime, SE_KEY, hat_keys[4*i + 1], qtrue, 0, NULL );
						break;
					case SDL_HAT_DOWN:
						Com_QueueEvent( in_eventTime, SE_KEY, hat_keys[4*i + 2], qtrue, 0, NULL );
						break;
					case SDL_HAT_LEFT:
						Com_QueueEvent( in_eventTime, SE_KEY, hat_keys[4*i + 3], qtrue, 0, NULL );
						break;
					case SDL_HAT_RIGHTUP:
						Com_QueueEvent( in_eventTime, SE_KEY, hat_keys[4*i + 0], qtrue, 0, NULL );
						Com_QueueEvent( in_eventTime, SE_KEY, hat_keys[4*i + 1], qtrue, 0, NULL );
						break;
					case SDL_HAT_RIGHTDOWN:
						Com_QueueEvent( in_eventTime, SE_KEY, hat_keys[4*i + 2], qtrue, 0, NULL );
						Com_QueueEvent( in_eventTime, SE_KEY, hat_keys[4*i + 1], qtrue, 0, NULL );
						break;
					case SDL_HAT_LEFTUP:
						Com_QueueEvent( in_eventTime, SE_KEY, hat_keys[4*i + 0], qtrue, 0, NULL );
						Com_QueueEvent( in_eventTime, SE_KEY, hat_keys[4*i + 3], qtrue, 0, NULL );
						break;
					case SDL_HAT_LEFTDOWN:
						Com_QueueEvent( in_eventTime, SE_KEY, hat_keys[4*i + 2], qtrue, 0, NULL );
						Com_QueueEvent( in_eventTime, SE_KEY, hat_keys[4*i + 3], qtrue, 0, NULL );
						break;
					default:
						break;
				}
			}
		}
	}

	// save hat state
	stick_state.oldhats = hats;

	// finally, look at the axes...
	total = SDL_JoystickNumAxes(stick);
	if (total > 0)
	{
		if (in_joystickUseAnalog->i)
		{
			if (total > MAX_JOYSTICK_AXIS) total = MAX_JOYSTICK_AXIS;
			for (i = 0; i < total; i++)
			{
				Sint16 axis = SDL_JoystickGetAxis(stick, i);
				float f = ( (float) abs(axis) ) / 32767.0f;
				
				if( f < in_joystickThreshold->f ) axis = 0;

				if ( axis != stick_state.oldaaxes[i] )
				{
					if ( in_mode->i == 0 ) {
						Cvar_Set( "in_mode", "1" );
					}
					Com_QueueEvent( in_eventTime, SE_JOYSTICK_AXIS, i, axis, 0, NULL );
					stick_state.oldaaxes[i] = axis;
				}
			}
		}
		else
		{
			if (total > 16) total = 16;
			for (i = 0; i < total; i++)
			{
				Sint16 axis = SDL_JoystickGetAxis(stick, i);
				float f = ( (float) axis ) / 32767.0f;
				if( f < -in_joystickThreshold->f ) {
					axes |= ( 1 << ( i * 2 ) );
				} else if( f > in_joystickThreshold->f ) {
					axes |= ( 1 << ( ( i * 2 ) + 1 ) );
				}
			}
		}
	}

	/* Time to update axes state based on old vs. new. */
	if (axes != stick_state.oldaxes)
	{
		for( i = 0; i < 16; i++ ) {
			if( ( axes & ( 1 << i ) ) && !( stick_state.oldaxes & ( 1 << i ) ) ) {
				if ( in_mode->i == 0 ) {
					Cvar_Set( "in_mode", "1" );
				}
				Com_QueueEvent( in_eventTime, SE_KEY, joy_keys[i], qtrue, 0, NULL );
			}

			if( !( axes & ( 1 << i ) ) && ( stick_state.oldaxes & ( 1 << i ) ) ) {
				if ( in_mode->i == 0 ) {
					Cvar_Set( "in_mode", "1" );
				}
				Com_QueueEvent( in_eventTime, SE_KEY, joy_keys[i], qfalse, 0, NULL );
			}
		}
	}

	/* Save for future generations. */
	stick_state.oldaxes = axes;
}

//static void IN_ProcessEvents( void )
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
		if ( ( Key_GetCatcher() & KEYCATCH_CONSOLE || Key_GetCatcher() & KEYCATCH_UI ) && ImGui::GetCurrentContext() ) {
			ImGui_ImplSDL2_ProcessEvent( &e );
		}

		switch( e.type ) {
		case SDL_KEYDOWN:
			if ( e.key.repeat && Key_GetCatcher() == 0 )
				break;
			key = IN_TranslateSDLToQ3Key( &e.key.keysym, qtrue );

			if ( in_mode->i == 1 ) {
				Cvar_Set( "in_mode", "0" );
			}

			if ( key == KEY_ENTER && keys[KEY_ALT].down ) {
				Cvar_SetIntegerValue( "r_fullscreen", glw_state.isFullscreen ? 0 : 1 );
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
			break;
		case SDL_KEYUP:
			if ( ( key = IN_TranslateSDLToQ3Key( &e.key.keysym, qfalse ) ) ) {
				Com_QueueEvent( in_eventTime, SE_KEY, key, qfalse, 0, NULL );
			}

			lastKeyDown = 0;
			break;
		case SDL_TEXTINPUT:
			if ( lastKeyDown != KEY_CONSOLE ) {
				char *c = e.text.text;

				// Quick and dirty UTF-8 to UTF-32 conversion
				while ( *c ) {
					int utf32 = 0;

					if ( ( *c & 0x80 ) == 0 ) {
						utf32 = *c++;
					}
					else if ( ( *c & 0xE0 ) == 0xC0 ) { // 110x xxxx
						utf32 |= ( *c++ & 0x1F ) << 6;
						utf32 |= ( *c++ & 0x3F );
					}
					else if ( ( *c & 0xF0 ) == 0xE0 ) { // 1110 xxxx
						utf32 |= ( *c++ & 0x0F ) << 12;
						utf32 |= ( *c++ & 0x3F ) << 6;
						utf32 |= ( *c++ & 0x3F );
					}
					else if ( ( *c & 0xF8 ) == 0xF0 ) { // 1111 0xxx
						utf32 |= ( *c++ & 0x07 ) << 18;
						utf32 |= ( *c++ & 0x3F ) << 12;
						utf32 |= ( *c++ & 0x3F ) << 6;
						utf32 |= ( *c++ & 0x3F );
					}
					else {
						Con_DPrintf( "Unrecognised UTF-8 lead byte: 0x%x\n", (unsigned int)*c );
						c++;
					}

					if ( utf32 != 0 ) {
						if ( IN_IsConsoleKey( 0, utf32 ) ) {
							Com_QueueEvent( in_eventTime, SE_KEY, KEY_CONSOLE, qtrue, 0, NULL );
							Com_QueueEvent( in_eventTime, SE_KEY, KEY_CONSOLE, qfalse, 0, NULL );
						}
						else {
							Com_QueueEvent( in_eventTime, SE_CHAR, utf32, 0, 0, NULL );
						}
					}
				}
			}
			break;
		case SDL_MOUSEMOTION:
			if ( in_mode->i == 1 ) {
//				Cvar_Set( "in_mode", "0" );
			}

			if( mouseActive ) {
				if ( !e.motion.xrel && !e.motion.yrel )
					break;
				Com_QueueEvent( in_eventTime, SE_MOUSE, e.motion.xrel, e.motion.yrel, 0, NULL );
			}
			break;
		case SDL_MOUSEBUTTONDOWN:
		case SDL_MOUSEBUTTONUP: {
			int b;
			Cvar_Set( "in_mode", "0" );

			switch ( e.button.button ) {
			case SDL_BUTTON_LEFT:   b = KEY_MOUSE_LEFT;     break;
			case SDL_BUTTON_MIDDLE: b = KEY_MOUSE_MIDDLE;   break;
			case SDL_BUTTON_RIGHT:  b = KEY_MOUSE_RIGHT;    break;
			case SDL_BUTTON_X1:     b = KEY_MOUSE_BUTTON_4; break;
			case SDL_BUTTON_X2:     b = KEY_MOUSE_BUTTON_5; break;
			default:                b = KEY_AUX1 + ( e.button.button - SDL_BUTTON_X2 + 1 ) % 16; break;
			};
			Com_QueueEvent( in_eventTime, SE_KEY, b,
				( e.type == SDL_MOUSEBUTTONDOWN ? qtrue : qfalse ), 0, NULL );
			break; }
		case SDL_MOUSEWHEEL:
			if ( in_mode->i == 1 ) {
				Cvar_Set( "in_mode", "0" );
			}
			if ( e.wheel.y > 0 ) {
				Com_QueueEvent( in_eventTime, SE_KEY, KEY_WHEEL_UP, qtrue, 0, NULL );
				Com_QueueEvent( in_eventTime, SE_KEY, KEY_WHEEL_UP, qfalse, 0, NULL );
			}
			else if ( e.wheel.y < 0 ) {
				Com_QueueEvent( in_eventTime, SE_KEY, KEY_WHEEL_DOWN, qtrue, 0, NULL );
				Com_QueueEvent( in_eventTime, SE_KEY, KEY_WHEEL_DOWN, qfalse, 0, NULL );
			}
			break;
		case SDL_CONTROLLERDEVICEADDED:
			Cvar_Set( "in_mode", "1" );
		case SDL_CONTROLLERDEVICEREMOVED:
			if ( in_joystick->i ) {
				IN_InitJoystick();
			}
			break;
		case SDL_QUIT:
			Cbuf_ExecuteText( EXEC_NOW, "quit Closed window\n" );
			break;
		case SDL_WINDOWEVENT:
#ifdef DEBUG_EVENTS
			Con_Printf( "%4i %s\n", e.window.timestamp, eventName( e.window.event ) );
#endif
			switch ( e.window.event ) {
			case SDL_WINDOWEVENT_MOVED:
				if ( gw_active && !gw_minimized && !glw_state.isFullscreen ) {
					//Cvar_Setif( "vid_xpos", e.window.data1 );
					//Cvar_Setif( "vid_ypos", e.window.data2 );
				}
				break;
			// window states:
			case SDL_WINDOWEVENT_HIDDEN:
			case SDL_WINDOWEVENT_MINIMIZED:
				if ( gi.state == GS_LEVEL && gi.mapLoaded && !Cvar_VariableInteger( "g_paused" ) ) {
					Cbuf_ExecuteText( EXEC_APPEND, "togglepausemenu\n" );
				}
				Key_ClearStates();
				gw_active = qfalse;
				gw_minimized = qtrue;
				break;
			case SDL_WINDOWEVENT_SHOWN:
			case SDL_WINDOWEVENT_RESTORED:
			case SDL_WINDOWEVENT_MAXIMIZED:
				gw_minimized = qfalse;
				break;
			// keyboard focus:
			case SDL_WINDOWEVENT_FOCUS_LOST:
				if ( gi.state == GS_LEVEL && gi.mapLoaded && !Cvar_VariableInteger( "g_paused" ) ) {
					Cbuf_ExecuteText( EXEC_APPEND, "togglepausemenu\n" );
				}
				lastKeyDown = 0;
				Key_ClearStates();
				gw_active = qfalse; break;
			case SDL_WINDOWEVENT_FOCUS_GAINED:
				lastKeyDown = 0;
				Key_ClearStates();
				gw_active = qtrue;
				gw_minimized = qfalse;
				if ( re.SetColorMappings ) {
					re.SetColorMappings();
				}
				break;
				// mouse focus:
				case SDL_WINDOWEVENT_ENTER: mouse_focus = qtrue; break;
				case SDL_WINDOWEVENT_LEAVE: if ( glw_state.isFullscreen ) mouse_focus = qfalse; break;
			};
			break;
		default:
			break;
		};
	}
}

/*
===============
IN_Minimize

Minimize the game so that user is back at the desktop
===============
*/
static void IN_Minimize( void )
{
	SDL_MinimizeWindow( SDL_window );
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

	in_mode = Cvar_Get( "in_mode", "0", CVAR_SAVE );
	Cvar_SetDescription( in_mode,
		"Sets how the game recieves user input:\n"
		" 0 - keyboard & mouse\n"
		" 1 - controller\n" );

	in_keyboardDebug = Cvar_Get( "in_keyboardDebug", "0", CVAR_SAVE );
	Cvar_SetDescription( in_keyboardDebug, "Print keyboard debug info." );
	in_forceCharset = Cvar_Get( "in_forceCharset", "1", CVAR_ARCHIVE_ND );
	Cvar_SetDescription( in_forceCharset, "Try to translate non-ASCII chars in keyboard input or force EN/US keyboard layout." );

	// mouse variables
	in_mouse = Cvar_Get( "in_mouse", "1", CVAR_SAVE );
	Cvar_CheckRange( in_mouse, "-1", "1", CVT_INT );
	Cvar_SetDescription( in_mouse,
		"Mouse data input source:\n"
		"  0 - disable mouse input\n"
		"  1 - di/raw mouse\n"
		" -1 - win32 mouse" );
	
	in_haptic = Cvar_Get( "in_haptic", "1", CVAR_SAVE | CVAR_LATCH );
	Cvar_SetDescription( in_haptic, "Whether or not haptic feedback is on." );

	in_joystick = Cvar_Get( "in_joystick", "1", CVAR_SAVE | CVAR_LATCH );
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
	Cvar_CheckRange( j_side_axis, "0", va( "%i", MAX_JOYSTICK_AXIS - 1 ), CVT_INT );
	Cvar_SetDescription( j_side_axis, "Selects which joystick axis controls left/right." );
	j_up_axis = Cvar_Get( "j_up_axis", "4", CVAR_ARCHIVE_ND );
	Cvar_CheckRange( j_up_axis, "0", va( "%i", MAX_JOYSTICK_AXIS - 1 ), CVT_INT );
	Cvar_SetDescription( j_up_axis, "Selects which joystick axis controls up/down." );

	// ~ and `, as keys and characters
	g_consoleKeys = Cvar_Get( "g_consoleKeys", "~ ` 0x7e 0x60", CVAR_SAVE );
	Cvar_SetDescription( g_consoleKeys, "Space delimited list of key names or characters that toggle the console." );

	mouseAvailable = ( in_mouse->f != 0 ) ? qtrue : qfalse;

	SDL_StartTextInput();

	//IN_DeactivateMouse();

	IN_InitJoystick();

	Cmd_AddCommand( "minimize", IN_Minimize );
	Cmd_AddCommand( "in_restart", IN_Restart );
	Cmd_AddCommand( "in_haptic_rumble", IN_HapticRumble );

	Con_DPrintf( "------------------------------------\n" );
}

void IN_Shutdown( void )
{
    SDL_StopTextInput();

    IN_DeactivateMouse();

    mouseAvailable = qfalse;

	IN_ShutdownJoystick();

    Cmd_RemoveCommand( "minimize" );
    Cmd_RemoveCommand( "in_restart" );
	Cmd_RemoveCommand( "in_haptic_rumble" );
}

void IN_Frame( void )
{
	IN_JoyMove();

	if ( Key_GetCatcher() & KEYCATCH_CONSOLE ) {
		// temporarily deactivate if not in the game and
		// running on the desktop with multimonitor configuration
		if ( !glw_state.isFullscreen || glw_state.monitorCount > 1 ) {
			IN_DeactivateMouse();
			return;
		}
	}

	if ( !gw_active || !mouse_focus || in_nograb->i ) {
		IN_DeactivateMouse();
		return;
	}

	IN_ActivateMouse();

	//IN_ProcessEvents();
	//HandleEvents();

	// Set event time for next frame to earliest possible time an event could happen
	//in_eventTime = Sys_Milliseconds();
}
