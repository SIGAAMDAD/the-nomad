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

#include "ui_lib.h"

#define ID_CONTINUE	0
#define ID_MISSION	1

typedef struct {
	menuframework_t menu;

	menutext_t continuePlay;
	menutext_t mission;
} missionMenu_t;

static missionMenu_t *s_mission;

static void MissionMenu_Event( void *ptr, int event )
{
	if ( event != EVENT_ACTIVATED ) {
		return;
	}
}

static void MissionMenu_Draw( void )
{
}

void MissionMenu_Cache( void )
{
	if ( !ui->uiAllocated ) {
		s_mission = (missionMenu_t *)Hunk_Alloc( sizeof( *s_mission ), h_high );
	}
	memset( s_mission, 0, sizeof( *s_mission ) );

	s_mission->menu.x = 0;
	s_mission->menu.y = 0;
	s_mission->menu.height = ui->gpuConfig.vidHeight - ( 100 * ui->scale );
	s_mission->menu.titleFontScale = 3.5f;
	s_mission->menu.textFontScale = 1.75f;
	s_mission->menu.draw = MissionMenu_Draw;
	s_mission->menu.fullscreen = qtrue;
	s_mission->menu.flags = MENU_DEFAULT_FLAGS;
	s_mission->menu.name = "##SinglePlayerMissionMenu";
	s_mission->menu.track = Snd_RegisterTrack( "event:/music/main_theme" );

	Menu_AddItem( &s_mission->menu, &s_mission->continuePlay );
	Menu_AddItem( &s_mission->menu, &s_mission->mission );
}

void UI_MissionMenu( void )
{
	UI_PushMenu( &s_mission->menu );
}