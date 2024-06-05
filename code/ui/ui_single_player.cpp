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

#include "../game/g_game.h"
#include "ui_public.hpp"
#include "ui_menu.h"
#include "ui_lib.h"
#include "ui_string_manager.h"

#define ID_NEWGAME      0
#define ID_LOADGAME     1
#define ID_PLAMISSION   2

typedef struct {
    menuframework_t menu;

    menutext_t newGame;
    menutext_t loadGame;
    menutext_t playMission;
} campaignMenu_t;

static campaignMenu_t *s_campaignMenu;

static void SinglePlayerMenu_EventCallback( void *ptr, int event )
{
    if ( event != EVENT_ACTIVATED ) {
        return;
    }

    switch ( ( (menucommon_t *)ptr )->id ) {
    case ID_NEWGAME:
        UI_NewGameMenu();
        break;
    case ID_LOADGAME:
        UI_LoadGameMenu();
        break;
    case ID_PLAMISSION:
        break;
    default:
        break;
    };
}

void SinglePlayerMenu_Cache( void )
{
    const stringHash_t *titleString;
    const stringHash_t *newGameString;
    const stringHash_t *loadGameString;
    const stringHash_t *playMissionString;

    if ( !ui->uiAllocated ) {
        s_campaignMenu = (campaignMenu_t *)Hunk_Alloc( sizeof( *s_campaignMenu ), h_high );
    }
    memset( s_campaignMenu, 0, sizeof( *s_campaignMenu ) );

	titleString = strManager->ValueForKey( "SP_MENU_TITLE" );	
    newGameString = strManager->ValueForKey( "SP_NEWGAME" );
    loadGameString = strManager->ValueForKey( "SP_LOADGAME" );
    playMissionString = strManager->ValueForKey( "SP_PLAY_MISSION" );

    s_campaignMenu->menu.name = titleString->value;
    s_campaignMenu->menu.flags = MENU_DEFAULT_FLAGS | ImGuiWindowFlags_HorizontalScrollbar;
    s_campaignMenu->menu.x = 0;
    s_campaignMenu->menu.y = 0;
    s_campaignMenu->menu.titleFontScale = 3.5f;
    s_campaignMenu->menu.textFontScale = 1.5f;
    s_campaignMenu->menu.width = ui->gpuConfig.vidWidth;
    s_campaignMenu->menu.height = ui->gpuConfig.vidHeight;
    s_campaignMenu->menu.fullscreen = qtrue;

    s_campaignMenu->newGame.generic.name = StringDup( newGameString, "SinglePlayerNewGameOption" );
    s_campaignMenu->newGame.generic.type = MTYPE_TEXT;
    s_campaignMenu->newGame.generic.id = ID_NEWGAME;
    s_campaignMenu->newGame.generic.flags = QMF_HIGHLIGHT_IF_FOCUS;
    s_campaignMenu->newGame.generic.eventcallback = SinglePlayerMenu_EventCallback;
    s_campaignMenu->newGame.text = newGameString->value;
    s_campaignMenu->newGame.color = color_white;

    s_campaignMenu->loadGame.generic.name = StringDup( loadGameString, "SinglePlayerLoadGameOption" );
    s_campaignMenu->loadGame.generic.type = MTYPE_TEXT;
    s_campaignMenu->loadGame.generic.id = ID_LOADGAME;
    s_campaignMenu->loadGame.generic.flags = QMF_HIGHLIGHT_IF_FOCUS;
    s_campaignMenu->loadGame.generic.eventcallback = SinglePlayerMenu_EventCallback;
    s_campaignMenu->loadGame.text = loadGameString->value;
    s_campaignMenu->loadGame.color = color_white;

    s_campaignMenu->playMission.generic.name = StringDup( playMissionString, "SinglePlayerPlayMissionOption" );
    s_campaignMenu->playMission.generic.type = MTYPE_TEXT;
    s_campaignMenu->playMission.generic.id = ID_PLAMISSION;
    s_campaignMenu->playMission.generic.flags = QMF_HIGHLIGHT_IF_FOCUS;
    s_campaignMenu->playMission.generic.eventcallback = SinglePlayerMenu_EventCallback;
    s_campaignMenu->playMission.text = playMissionString->value;
    s_campaignMenu->playMission.color = color_white;

    Menu_AddItem( &s_campaignMenu->menu, &s_campaignMenu->newGame );
    Menu_AddItem( &s_campaignMenu->menu, &s_campaignMenu->loadGame );
    Menu_AddItem( &s_campaignMenu->menu, &s_campaignMenu->playMission );
}

void UI_SinglePlayerMenu( void )
{
    UI_PushMenu( &s_campaignMenu->menu );
    Key_SetCatcher( KEYCATCH_UI );
}
