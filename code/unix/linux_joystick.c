/*
===========================================================================
Copyright (C) 2023-2024 GDR Games

This file is part of The Nomad source code.

The Nomad source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

The Nomad source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Foobar; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/
/*
** linux_joystick.c
**
** This file contains ALL Linux specific stuff having to do with the
** Joystick input.  When a port is being made the following functions
** must be implemented by the port:
**
** Authors: mkv, bk
**
*/

#include <linux/joystick.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include "../game/g_game.h"
#include "unix_local.h"

/* We translate axes movement into keypresses. */
int joy_keys[16] = {
	KEY_LEFTARROW, KEY_RIGHTARROW,
	KEY_UPARROW, KEY_DOWNARROW,
	KEY_JOY16, KEY_JOY17,
	KEY_JOY18, KEY_JOY19,
	KEY_JOY20, KEY_JOY21,
	KEY_JOY22, KEY_JOY23,
	
	KEY_JOY24, KEY_JOY25,
	KEY_JOY26, KEY_JOY27
};

/* Our file descriptor for the joystick device. */
static int joy_fd = -1;

extern cvar_t *in_joystick;
extern cvar_t *in_joystickDebug;
extern cvar_t *joy_threshold;

/**********************************************/
/* Joystick routines.                         */
/**********************************************/
