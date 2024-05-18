#include "g_game.h"

static uint32_t frame_msec;
static int32_t old_com_frameTime;

/*
===============================================================================

KEY BUTTONS

Continuous button event tracking is complicated by the fact that two different
input sources (say, mouse button 1 and the control key) can both press the
same button, but the button should only be released when both of the
pressing key have been released.

When a key event issues a button command (+forward, +button0, etc), it appends
its key number as argv(1) so it can be matched up with the release.

argv(2) will be set to the time the event happened, which allows exact
control even at low framerates when the down and up events may both get queued
at the same time.

===============================================================================
*/

typedef struct {
	uint32_t	down[2];		// key nums holding it down
	uint32_t	downtime;		// msec timestamp
	uint32_t	msec;			// msec down this frame if both a down and up happened
	qboolean	active;			// current state
	qboolean	wasPressed;		// set when down, not cleared when up
} kbutton_t;

static kbutton_t in_north, in_west, in_east, in_south;
static kbutton_t in_jump;
static kbutton_t in_buttons[16];

static cvar_t *g_mouseAcceleration;
static cvar_t *g_mouseSensitivity;
static cvar_t *g_mouseSmoothing;
static cvar_t *g_debugMove;

static void IN_KeyDown( kbutton_t *b ) {
	const char *c;
    int32_t k;

	c = Cmd_Argv( 1 );
	if ( c[0] ) {
		k = atoi( c );
	} else {
		k = -1;		// typed manually at the console for continuous down
	}

	if ( k == b->down[0] || k == b->down[1] ) {
		return;		// repeating key
	}

	if ( !b->down[0] ) {
		b->down[0] = k;
	} else if ( !b->down[1] ) {
		b->down[1] = k;
	} else {
		Con_Printf( "Three keys down for a button!\n" );
		return;
	}

	if ( b->active ) {
		return;		// still down
	}

	// save timestamp for partial frame summing
	c = Cmd_Argv( 2 );
	b->downtime = atoi( c );

	b->active = qtrue;
	b->wasPressed = qtrue;
}


static void IN_KeyUp( kbutton_t *b ) {
	unsigned uptime;
	const char *c;
	int32_t k;

	c = Cmd_Argv( 1 );
	if ( c[0] ) {
		k = atoi( c );
	} else {
		// typed manually at the console, assume for unsticking, so clear all
		b->down[0] = b->down[1] = 0;
		b->active = qfalse;
		return;
	}

	if ( b->down[0] == k ) {
		b->down[0] = 0;
	} else if ( b->down[1] == k ) {
		b->down[1] = 0;
	} else {
		return;		// key up without corresponding down (menu pass through)
	}
	if ( b->down[0] || b->down[1] ) {
		return;		// some other key is still holding it down
	}

	b->active = qfalse;

	// save timestamp for partial frame summing
	c = Cmd_Argv( 2 );
	uptime = atoi( c );
	if ( uptime ) {
		b->msec += uptime - b->downtime;
	} else {
		b->msec += frame_msec / 2;
	}

	b->active = qfalse;
}


/*
===============
G_KeyState

Returns the fraction of the frame that the key was down
===============
*/
static float G_KeyState( kbutton_t *key ) {
	float		val;
	uint32_t	msec;

	msec = key->msec;
	key->msec = 0;

	if ( key->active ) {
		// still down
		if ( !key->downtime ) {
			msec = com_frameTime;
		} else {
			msec += com_frameTime - key->downtime;
		}
		key->downtime = com_frameTime;
	}

#if 0
	if (msec) {
		Con_Printf( "%i ", msec );
	}
#endif

	val = (float)msec / frame_msec;
	if ( val < 0 ) {
		val = 0;
	}
	if ( val > 1 ) {
		val = 1;
	}

	return val;
}

static void IN_NorthDown( void ) { IN_KeyDown( &in_north ); }
static void IN_NorthUp( void ) { IN_KeyUp( &in_north ); }
static void IN_SouthDown( void ) { IN_KeyDown( &in_south ); }
static void IN_SouthUp( void ) { IN_KeyUp( &in_south ); }
static void IN_WestDown( void ) { IN_KeyDown( &in_west ); }
static void IN_WestUp( void ) { IN_KeyUp( &in_west ); }
static void IN_EastDown( void ) { IN_KeyDown( &in_east ); }
static void IN_EastUp( void )  { IN_KeyUp( &in_east ); }
static void IN_JumpDown( void ) { IN_KeyDown( &in_jump ); }
static void IN_JumpUp( void ) { IN_KeyUp( &in_jump ); }

static void IN_Button0Down( void ) {IN_KeyDown(&in_buttons[0]);}
static void IN_Button0Up( void ) {IN_KeyUp(&in_buttons[0]);}
static void IN_Button1Down( void ) {IN_KeyDown(&in_buttons[1]);}
static void IN_Button1Up( void ) {IN_KeyUp(&in_buttons[1]);}
static void IN_Button2Down( void ) {IN_KeyDown(&in_buttons[2]);}
static void IN_Button2Up( void ) {IN_KeyUp(&in_buttons[2]);}
static void IN_Button3Down( void ) {IN_KeyDown(&in_buttons[3]);}
static void IN_Button3Up( void ) {IN_KeyUp(&in_buttons[3]);}
static void IN_Button4Down( void ) {IN_KeyDown(&in_buttons[4]);}
static void IN_Button4Up( void ) {IN_KeyUp(&in_buttons[4]);}
static void IN_Button5Down( void ) {IN_KeyDown(&in_buttons[5]);}
static void IN_Button5Up( void ) {IN_KeyUp(&in_buttons[5]);}
static void IN_Button6Down( void ) {IN_KeyDown(&in_buttons[6]);}
static void IN_Button6Up( void ) {IN_KeyUp(&in_buttons[6]);}
static void IN_Button7Down( void ) {IN_KeyDown(&in_buttons[7]);}
static void IN_Button7Up( void ) {IN_KeyUp(&in_buttons[7]);}
static void IN_Button8Down( void ) {IN_KeyDown(&in_buttons[8]);}
static void IN_Button8Up( void ) {IN_KeyUp(&in_buttons[8]);}
static void IN_Button9Down( void ) {IN_KeyDown(&in_buttons[9]);}
static void IN_Button9Up( void ) {IN_KeyUp(&in_buttons[9]);}

//==========================================================================


/*
================
G_AdjustAngles

Moves the local angle positions
================
*/
static void G_AdjustAngles( void )
{
	float	speed;
	
    speed = 0.001 * gi.frametime;

//    gi.viewangles[YAW] -= speed*g_mouseSensitivity->f*G_KeyState (&in_east);
//	gi.viewangles[YAW] += speed*g_mouseSensitivity->f*G_KeyState (&in_west);
//
//	gi.viewangles[PITCH] -= speed*g_mouseSensitivity->f; //* G_KeyState (&in_lookup);
//	gi.viewangles[PITCH] += speed*g_mouseSensitivity->f; //* G_KeyState (&in_lookdown);
}

/*
================
G_KeyMove

Sets the usercmd_t based on key states
================
*/
static void G_KeyMove( usercmd_t *cmd )
{
	int32_t     movespeed;
	int32_t		forward, side, up;
	int32_t		jump;

//	cmd->buttons |= BUTTON_WALKING;
	movespeed = 4;
	forward = 0;
	side = 0;
	up = 0;

	side += movespeed * G_KeyState( &in_east );
	side -= movespeed * G_KeyState( &in_west );

	forward += movespeed * G_KeyState( &in_north );
	forward -= movespeed * G_KeyState( &in_south );

	up += movespeed * G_KeyState( &in_jump );

	cmd->forwardmove = ClampCharMove( forward );
	cmd->sidemove = ClampCharMove( side );
	cmd->upmove = ClampCharMove( up );
}


/*
=================
G_MouseEvent
=================
*/
void G_MouseEvent( int32_t dx, int32_t dy /*, int time*/ )
{
	if ( Key_GetCatcher() & KEYCATCH_SGAME ) {
		g_pModuleLib->ModuleCall( sgvm, ModuleOnMouseEvent, dx, dy );
	} else {
		gi.mouseDx[gi.mouseIndex] += dx;
		gi.mouseDy[gi.mouseIndex] += dy;
	}
}


/*
=================
CL_JoystickEvent

Joystick values stay set until changed
=================
*/
void CL_JoystickEvent( int axis, int value, int time ) {
	if ( axis < 0 || axis >= MAX_JOYSTICK_AXIS ) {
		N_Error( ERR_DROP, "CL_JoystickEvent: bad axis %i", axis );
	} else {
		gi.joystickAxis[axis] = value;
	}
}


/*
=================
CL_JoystickMove
=================
*/
static void CL_JoystickMove( usercmd_t *cmd ) {
	//int		movespeed;
	float	anglespeed;

//	if ( in_speed.active ^ cl_run->integer ) {
//		//movespeed = 2;
//	} else {
//		//movespeed = 1;
//		cmd->buttons |= BUTTON_WALKING;
//	}

//	if ( in_speed.active ) {
//		anglespeed = 0.001 * cls.frametime * cl_anglespeedkey->value;
//	} else {
//		anglespeed = 0.001 * cls.frametime;
//	}

//	if ( !in_strafe.active ) {
//		gi.viewangles[YAW] += anglespeed * g_mouseSensitivity->f * gi.joystickAxis[AXIS_SIDE];
//	} else {
//		cmd->rightmove = ClampCharMove( cmd->rightmove + gi.joystickAxis[AXIS_SIDE] );
//	}
//
//	if ( in_mode->i == 0 ) {
//		gi.viewangles[PITCH] += anglespeed * g_mouseSensitivity->f * gi.joystickAxis[AXIS_FORWARD];
//	} else {
//		cmd->forwardmove = ClampCharMove( cmd->forwardmove + gi.joystickAxis[AXIS_FORWARD] );
//	}

	cmd->upmove = ClampCharMove( cmd->upmove + gi.joystickAxis[AXIS_UP] );
}

/*
=================
G_MouseMove
=================
*/
static void G_MouseMove( usercmd_t *cmd )
{
	float mx, my;

	// allow mouse smoothing
	if (g_mouseSmoothing->i) {
		mx = (gi.mouseDx[0] + gi.mouseDx[1]) * 0.5f;
		my = (gi.mouseDy[0] + gi.mouseDy[1]) * 0.5f;
	}
	else {
		mx = gi.mouseDx[gi.mouseIndex];
		my = gi.mouseDy[gi.mouseIndex];
	}

	gi.mouseIndex ^= 1;
	gi.mouseDx[gi.mouseIndex] = 0;
	gi.mouseDy[gi.mouseIndex] = 0;

	if (mx == 0.0f && my == 0.0f)
		return;

	if ( g_mouseAcceleration->i ) {
		if ( 0 ) {
			float accelSensitivity;
			float rate;

			rate = sqrt(mx * mx + my * my) / (float) frame_msec;

			accelSensitivity = g_mouseSensitivity->f + rate * 1.5f;
			mx *= accelSensitivity;
			my *= accelSensitivity;

//			if ( cl_showMouseRate->integer )
//				Com_Printf( "rate: %f, accelSensitivity: %f\n", rate, accelSensitivity );
		}
		else {
			float rate[2];
			float power[2];
			float offset = 5.0;//cl_mouseAccelOffset->value;

			// clip at a small positive number to avoid division
			// by zero (or indeed going backwards!)
			if ( offset < 0.001f ) {
				offset = 0.001f;
			}

			// sensitivity remains pretty much unchanged at low speeds
			// cl_mouseAccel is a power value to how the acceleration is shaped
			// cl_mouseAccelOffset is the rate for which the acceleration will have doubled the non accelerated amplification
			// NOTE: decouple the config cvars for independent acceleration setup along X and Y?

			rate[0] = fabsf( mx ) / (float) frame_msec;
			rate[1] = fabsf( my ) / (float) frame_msec;
			power[0] = powf( rate[0] / offset, 1.5f );
			power[1] = powf( rate[1] / offset, 1.5f );

			mx = g_mouseSensitivity->f * (mx + ((mx < 0) ? -power[0] : power[0]) * offset);
			my = g_mouseSensitivity->f * (my + ((my < 0) ? -power[1] : power[1]) * offset);

		//	if(cl_showMouseRate->integer)
		//		Com_Printf("ratex: %f, ratey: %f, powx: %f, powy: %f\n", rate[0], rate[1], power[0], power[1]);
		}
	}
	else {
		mx *= g_mouseSensitivity->f;
		my *= g_mouseSensitivity->f;
	}

	// ingame FOV
	mx *= g_mouseSensitivity->f;
	my *= g_mouseSensitivity->f;

	// add mouse X/Y movement to cmd
//	gi.viewangles[YAW] -= g_->value * mx;
//	gi.viewangles[PITCH] += m_pitch->value * my;
}


/*
==============
G_CmdButtons
==============
*/
static void G_CmdButtons( usercmd_t *cmd ) {
	uint32_t	i;

	//
	// figure button bits
	// send a button bit even if the key was pressed and released in
	// less than a frame
	//
	for ( i = 0 ; i < arraylen( in_buttons ); i++ ) {
		in_buttons[i].wasPressed = qfalse;
	}

	// allow the game to know if any key at all is
	// currently pressed, even if it isn't bound to anything
//	if ( anykeydown && Key_GetCatcher() == 0 ) {
//		cmd->buttons |= BUTTON_ANY;
//	}
}


/*
==============
G_FinishMove
==============
*/
static void G_FinishMove( usercmd_t *cmd ) {
	int i;

	// copy the state that the cgame is currently sending
//	cmd->weapon = gi.cgameUserCmdValue;

	// send the current server time so the amount of movement
	// can be determined without allowing cheating
//	cmd->serverTime = gi.serverTime;

	for ( i = 0; i < 3; i++ ) {
	//	cmd->angles[i] = ANGLE2SHORT( gi.viewangles[i] );
	}
}

/*
=================
G_CreateCmd
=================
*/
static usercmd_t G_CreateCmd( void ) {
	usercmd_t	cmd;
	vec3_t		oldAngles;

//	VectorCopy( gi.viewangles, oldAngles );

	// keyboard angle adjustment
	G_AdjustAngles ();

	memset( &cmd, 0, sizeof( cmd ) );

	G_CmdButtons( &cmd );

	// get basic movement from keyboard
	G_KeyMove( &cmd );

	// get basic movement from mouse
	G_MouseMove( &cmd );

	// get basic movement from joystick
	CL_JoystickMove( &cmd );

	// check to make sure the angles haven't wrapped
//	if ( gi.viewangles[PITCH] - oldAngles[PITCH] > 90 ) {
//		gi.viewangles[PITCH] = oldAngles[PITCH] + 90;
//	} else if ( oldAngles[PITCH] - gi.viewangles[PITCH] > 90 ) {
//		gi.viewangles[PITCH] = oldAngles[PITCH] - 90;
//	}

	// store out the final values
	G_FinishMove( &cmd );

	// draw debug graphs of turning for mouse testing
	if ( g_debugMove->i ) {
	}

	return cmd;
}

/*
=================
G_CreateNewCommand

Create a new usercmd_t structure for this frame
=================
*/
void G_CreateNewCommands( void ) {
	int cmdNum;

	// no need to create usercmds until we have a gamestate
	if ( gi.state < GS_LEVEL ) {
		return;
	}

	frame_msec = com_frameTime - old_com_frameTime;

	// if running over 1000fps, act as if each frame is 1ms
	// prevents divisions by zero
	if ( frame_msec < 1 ) {
		frame_msec = 1;
	}

	// if running less than 5fps, truncate the extra time to prevent
	// unexpected moves after a hitch
	if ( frame_msec > 200 ) {
		frame_msec = 200;
	}
	old_com_frameTime = com_frameTime;

	// generate a command for this frame
	gi.cmdNumber++;
	cmdNum = gi.cmdNumber & CMD_MASK;
	gi.cmds[cmdNum] = G_CreateCmd();
}

void G_SendCmd( void ) {
    // dont't send the usercmd to the vm if we aren't running anything
    if ( gi.state != GS_LEVEL ) {
        return;
    }

    // don't send commands if paused
    if ( g_paused->i ) {
        return;
    }

    // create a new command
    G_CreateNewCommands();
}

void G_InitInput( void )
{
	Cmd_AddCommand( "+north", IN_NorthDown );
	Cmd_AddCommand( "-north", IN_NorthUp );
	Cmd_AddCommand( "+south", IN_SouthDown );
	Cmd_AddCommand( "-south", IN_SouthUp );
	Cmd_AddCommand( "+west", IN_WestDown );
	Cmd_AddCommand( "-west", IN_WestUp );
	Cmd_AddCommand( "+east", IN_EastDown );
	Cmd_AddCommand( "-east", IN_EastUp );
	Cmd_AddCommand( "+jump", IN_JumpDown );
	Cmd_AddCommand( "-jump", IN_JumpUp );
    Cmd_AddCommand( "+button0", IN_Button0Down );
    Cmd_AddCommand( "-button0", IN_Button0Up );
    Cmd_AddCommand( "+button1", IN_Button1Down );
    Cmd_AddCommand( "-button1", IN_Button1Up );
    Cmd_AddCommand( "+button2", IN_Button2Down );
    Cmd_AddCommand( "-button2", IN_Button2Up );
    Cmd_AddCommand( "+button3", IN_Button3Down );
    Cmd_AddCommand( "-button3", IN_Button3Up );
    Cmd_AddCommand( "+button4", IN_Button4Down );
    Cmd_AddCommand( "-button4", IN_Button4Up );
    Cmd_AddCommand( "+button5", IN_Button5Down );
    Cmd_AddCommand( "-button5", IN_Button5Up );
    Cmd_AddCommand( "+button6", IN_Button6Down );
    Cmd_AddCommand( "-button6", IN_Button6Up );
    Cmd_AddCommand( "+button7", IN_Button7Down );
    Cmd_AddCommand( "-button7", IN_Button7Up );
    Cmd_AddCommand( "+button8", IN_Button8Down );
    Cmd_AddCommand( "-button8", IN_Button8Up );
    Cmd_AddCommand( "+button9", IN_Button9Down );
    Cmd_AddCommand( "-button9", IN_Button9Up );

    g_mouseAcceleration = Cvar_Get( "g_mouseAcceleration", "1", CVAR_LATCH | CVAR_SAVE );
    Cvar_SetDescription( g_mouseAcceleration, "Make mouse speed be determined by how fast it moves." );

    g_mouseSensitivity = Cvar_Get( "g_mouseSensitivity", "5", CVAR_SAVE | CVAR_LATCH );
    Cvar_SetDescription( g_mouseSensitivity, "Sets base mouse speed." );
    
    g_mouseSmoothing = Cvar_Get( "g_mouseSmoothing", "0", CVAR_SAVE );

#ifdef _NOMAD_DEBUG
    g_debugMove = Cvar_Get( "g_debugMove", "1", 0 );
#else
    g_debugMove = Cvar_Get( "g_debugMove", "0", 0 );
#endif
    Cvar_CheckRange( g_debugMove, "0", "2", CVT_INT );
    Cvar_SetDescription( g_debugMove, "Prints a graph of view angle deltas.\n 0: Disabled\n 1: Yaw\n 2: Pitch" );
}

void G_ShutdownInput( void )
{
    Cmd_RemoveCommand( "+north" );
	Cmd_RemoveCommand( "-north" );
	Cmd_RemoveCommand( "+south" );
	Cmd_RemoveCommand( "-south" );
	Cmd_RemoveCommand( "+west" );
	Cmd_RemoveCommand( "-west" );
	Cmd_RemoveCommand( "+east" );
	Cmd_RemoveCommand( "-east" );
	Cmd_RemoveCommand( "+jump" );
	Cmd_RemoveCommand( "-jump" );
    Cmd_RemoveCommand( "+button0" );
    Cmd_RemoveCommand( "-button0" );
    Cmd_RemoveCommand( "+button1" );
    Cmd_RemoveCommand( "-button1" );
    Cmd_RemoveCommand( "+button2" );
    Cmd_RemoveCommand( "-button2" );
    Cmd_RemoveCommand( "+button3" );
    Cmd_RemoveCommand( "-button3" );
    Cmd_RemoveCommand( "+button4" );
    Cmd_RemoveCommand( "-button4" );
    Cmd_RemoveCommand( "+button5" );
    Cmd_RemoveCommand( "-button5" );
    Cmd_RemoveCommand( "+button6" );
    Cmd_RemoveCommand( "-button6" );
    Cmd_RemoveCommand( "+button7" );
    Cmd_RemoveCommand( "-button7" );
    Cmd_RemoveCommand( "+button8" );
    Cmd_RemoveCommand( "-button8" );
    Cmd_RemoveCommand( "+button9" );
    Cmd_RemoveCommand( "-button9" );
}
