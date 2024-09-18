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

#define ID_TITLE       0
#define ID_HELP        1
#define ID_RESUME      2
#define ID_SETTINGS    3
#define ID_CHECKPOINT  4
#define ID_PHOTOMODE   5
#define ID_EXIT        6

typedef struct {
	float exposure;
	float cameraScale;
	float cameraRotation;

	float oldCameraScale;
} photomode_t;

typedef struct {
	menuframework_t menu;

	menutext_t help;
	menutext_t resume;
	menutext_t settings;
	menutext_t checkpoint;
	menutext_t exitToMainMenu;

	menutext_t dailyTipText;

	photomode_t photomode;

	sfxHandle_t hPausedSnapshot;

	char **dailyTips;
	uint64_t numDailyTips;
	qboolean popped;
} pauseMenu_t;

// PAUSE. REWIND. PLAY.
static pauseMenu_t *s_pauseMenu;

static void PauseMenu_EventCallback( void *ptr, int event )
{
	if ( event != EVENT_ACTIVATED ) {
		return;
	}

	switch ( ( (menucommon_t *)ptr )->id ) {
	case ID_RESUME:
		UI_SetActiveMenu( UI_MENU_NONE );
		break;
	case ID_CHECKPOINT:
		// rewind the checkpoint
		Cbuf_ExecuteText( EXEC_NOW, "sgame.rewind_checkpoint\n" );
		UI_SetActiveMenu( UI_MENU_NONE );
		break;
	case ID_PHOTOMODE: 
		s_pauseMenu->photomode.cameraRotation = 0.0f;
		s_pauseMenu->photomode.cameraScale = s_pauseMenu->photomode.oldCameraScale = gi.cameraZoom;
		s_pauseMenu->photomode.exposure = Cvar_VariableFloat( "r_autoExposure" );

		Cbuf_ExecuteText( EXEC_APPEND, "togglephotomode\n" );
		break;
	case ID_HELP:
		UI_DataBaseMenu();
		break;
	case ID_SETTINGS:
		gi.state = GS_MENU;
		Cvar_SetIntegerValue( "g_paused", 0 );
		UI_SettingsMenu();
		break;
	case ID_EXIT:
		UI_PopMenu();
		UI_SetActiveMenu( UI_MENU_MAIN );
		gi.mapLoaded = qfalse;
		gi.mapCache.currentMapLoaded = -1;
		gi.state = GS_INACTIVE;
		g_pModuleLib->ModuleCall( sgvm, ModuleOnLevelEnd, 0 );
		g_pModuleLib->RunModules( ModuleOnLevelEnd, 0 );
		Snd_StopSfx( s_pauseMenu->hPausedSnapshot );
		Cvar_SetIntegerValue( "g_paused", 0 );
		Cbuf_ExecuteText( EXEC_APPEND, "setmap\n" ); // setting an empty mapname will unload the level
		break;
	default:
		break;
	};
}

static void DailyTip_Draw( void *ptr )
{
	FontCache()->SetActiveFont( AlegreyaSC );
	ImGui::PushStyleColor( ImGuiCol_FrameBg, colorGold );
	ImGui::PushStyleColor( ImGuiCol_FrameBgActive, colorGold );
	ImGui::PushStyleColor( ImGuiCol_FrameBgHovered, colorGold );
	ImGui::SetCursorScreenPos( ImVec2( 900 * ui->scale, 100 * ui->scale ) );
	ImGui::BeginChild( ImGui::GetID( "Tip of the Day" ), ImVec2( 400 * ui->scale, 500 * ui->scale ), ImGuiChildFlags_Border );
	ImGui::SeparatorText( "Tip of the Day" );
	FontCache()->SetActiveFont( RobotoMono );
	ImGui::TextWrapped( s_pauseMenu->dailyTipText.text );
	ImGui::EndChild();
	ImGui::PopStyleColor( 3 );
}

static void PauseMenu_DrawPhotoMode( void )
{
	ImGui::Begin( "##PhotoModeWindow", NULL, MENU_DEFAULT_FLAGS );
	{
		ImGui::PushStyleColor( ImGuiCol_Text, ImVec4( 0.0f, 0.0f, 0.0f, 0.0f ) );

		ImGui::SetCursorScreenPos( ImVec2( 64 * ui->scale, 64 * ui->scale ) );
		ImGui::BeginChild( ImGui::GetID( "PhotoModeSettings" ), ImVec2( 72 * ui->scale, 128 * ui->scale ), ImGuiChildFlags_AlwaysAutoResize );
		if ( ImGui::SliderFloat( "Exposure##PhotoModeSettingsExposure", &s_pauseMenu->photomode.exposure, 0.0f, 10.0f ) ) {
			Cvar_SetFloatValue( "r_autoExposure", s_pauseMenu->photomode.exposure );
		}
		ImGui::EndChild();

		ImGui::PopStyleColor();

		ImGui::SetCursorScreenPos( ImVec2( 0, 700 * ui->scale ) );
		if ( ImGui::Button( "Exit" ) ) {
			Cbuf_ExecuteText( EXEC_APPEND, "togglephotomode\n" );
		}
		if ( ImGui::Button( "Take Picture" ) ) {
			Cbuf_ExecuteText( EXEC_APPEND, "screenshotJPEG\n" );
		}
	}
	ImGui::End();
}

static void PauseMenu_Draw( void )
{
	FontCache()->SetActiveFont( AlegreyaSC );
	if ( gi.togglePhotomode ) {
		PauseMenu_DrawPhotoMode();
	}
	Menu_Draw( &s_pauseMenu->menu );
	FontCache()->SetActiveFont( AlegreyaSC );
}

static void PauseMenu_LoadDailyTips( void )
{
	union {
		char *b;
		void *v;
	} f;
	const char *tok;
	const char **text_p, *text;
	uint64_t i;

	FS_LoadFile( "dailytips.txt", &f.v );
	if ( !f.v ) {
		N_Error( ERR_DROP, "PauseMenu_Cache: failed to load dailytips.txt" );
	}

	text = f.b;
	text_p = (const char **)&text;

	while ( 1 ) {
		tok = COM_ParseExt( text_p, qtrue );
		if ( !tok[0] ) {
			break;
		}

		s_pauseMenu->numDailyTips++;
	}

	text = f.b;
	text_p = (const char **)&text;

	s_pauseMenu->dailyTips = (char **)Hunk_Alloc( sizeof( *s_pauseMenu->dailyTips ) * s_pauseMenu->numDailyTips, h_high );
	i = 0;
	while ( 1 ) {
		tok = COM_ParseExt( text_p, qtrue );
		if ( !tok[0] ) {
			break;
		}

		s_pauseMenu->dailyTips[i] = (char *)Hunk_Alloc( strlen( tok ) + 1, h_high );
		strcpy( s_pauseMenu->dailyTips[i], tok );
		i++;
	}
	Con_Printf( "%lu daily tips loaded.\n", s_pauseMenu->numDailyTips );

	FS_FreeFile( f.v );
}

void PauseMenu_Cache( void )
{
	const stringHash_t *titleString;
	const stringHash_t *helpString;
	const stringHash_t *resumeString;
	const stringHash_t *settingsString;
	const stringHash_t *checkpointString;
	const stringHash_t *exitToMainMenuString;

	if ( !ui->uiAllocated ) {
		s_pauseMenu = (pauseMenu_t *)Hunk_Alloc( sizeof( *s_pauseMenu ), h_high );
		PauseMenu_LoadDailyTips();
	}

	titleString = strManager->ValueForKey( "MENU_PAUSE_TITLE" );
	resumeString = strManager->ValueForKey( "MENU_PAUSE_RESUME" );
	checkpointString = strManager->ValueForKey( "MENU_PAUSE_CHECKPOINT" );
	exitToMainMenuString = strManager->ValueForKey( "MENU_PAUSE_ETMM" );
	helpString = strManager->ValueForKey( "MENU_PAUSE_HELP" );
	settingsString = strManager->ValueForKey( "MENU_PAUSE_SETTINGS" );

	s_pauseMenu->hPausedSnapshot = Snd_RegisterSfx( "snapshot:/PauseMenu" );

	s_pauseMenu->menu.width = ui->gpuConfig.vidWidth;
	s_pauseMenu->menu.height = ui->gpuConfig.vidHeight;
	s_pauseMenu->menu.fullscreen = qfalse;
	s_pauseMenu->menu.name = titleString->value;
	s_pauseMenu->menu.draw = PauseMenu_Draw;
	s_pauseMenu->menu.titleFontScale = 3.5f;
	s_pauseMenu->menu.textFontScale = 1.90f;
	s_pauseMenu->menu.flags = MENU_DEFAULT_FLAGS;
	s_pauseMenu->menu.x = 0;
	s_pauseMenu->menu.y = 0;

	s_pauseMenu->resume.generic.type = MTYPE_TEXT;
	s_pauseMenu->resume.generic.id = ID_RESUME;
	s_pauseMenu->resume.generic.flags = QMF_HIGHLIGHT_IF_FOCUS;
	s_pauseMenu->resume.generic.eventcallback = PauseMenu_EventCallback;
	s_pauseMenu->resume.generic.font = AlegreyaSC;
	s_pauseMenu->resume.text = resumeString->value;
	s_pauseMenu->resume.color = color_white;

	s_pauseMenu->checkpoint.generic.type = MTYPE_TEXT;
	s_pauseMenu->checkpoint.generic.id = ID_CHECKPOINT;
	s_pauseMenu->checkpoint.generic.flags = QMF_HIGHLIGHT_IF_FOCUS;
	s_pauseMenu->checkpoint.generic.eventcallback = PauseMenu_EventCallback;
	s_pauseMenu->checkpoint.generic.font = AlegreyaSC;
	s_pauseMenu->checkpoint.text = checkpointString->value;
	s_pauseMenu->checkpoint.color = color_white;

	s_pauseMenu->settings.generic.type = MTYPE_TEXT;
	s_pauseMenu->settings.generic.id = ID_SETTINGS;
	s_pauseMenu->settings.generic.flags = QMF_HIGHLIGHT_IF_FOCUS;
	s_pauseMenu->settings.generic.eventcallback = PauseMenu_EventCallback;
	s_pauseMenu->settings.generic.font = AlegreyaSC;
	s_pauseMenu->settings.text = settingsString->value;
	s_pauseMenu->settings.color = color_white;

	s_pauseMenu->help.generic.type = MTYPE_TEXT;
	s_pauseMenu->help.generic.id = ID_HELP;
	s_pauseMenu->help.generic.flags = QMF_HIGHLIGHT_IF_FOCUS;
	s_pauseMenu->help.generic.eventcallback = PauseMenu_EventCallback;
	s_pauseMenu->help.generic.font = AlegreyaSC;
	s_pauseMenu->help.text = helpString->value;
	s_pauseMenu->help.color = color_white;

	s_pauseMenu->exitToMainMenu.generic.type = MTYPE_TEXT;
	s_pauseMenu->exitToMainMenu.generic.id = ID_EXIT;
	s_pauseMenu->exitToMainMenu.generic.flags = QMF_HIGHLIGHT_IF_FOCUS;
	s_pauseMenu->exitToMainMenu.generic.eventcallback = PauseMenu_EventCallback;
	s_pauseMenu->exitToMainMenu.generic.font = AlegreyaSC;
	s_pauseMenu->exitToMainMenu.text = exitToMainMenuString->value;
	s_pauseMenu->exitToMainMenu.color = color_white;

	s_pauseMenu->dailyTipText.generic.type = MTYPE_TEXT;
	s_pauseMenu->dailyTipText.generic.flags = QMF_OWNERDRAW;
	s_pauseMenu->dailyTipText.generic.ownerdraw = DailyTip_Draw;
	srand( Sys_Milliseconds() );
	s_pauseMenu->dailyTipText.color = color_white;
	s_pauseMenu->dailyTipText.text = s_pauseMenu->dailyTips[ rand() & s_pauseMenu->numDailyTips - 1 ];

	Menu_AddItem( &s_pauseMenu->menu, &s_pauseMenu->resume );
	Menu_AddItem( &s_pauseMenu->menu, &s_pauseMenu->checkpoint );
	Menu_AddItem( &s_pauseMenu->menu, &s_pauseMenu->settings );
	Menu_AddItem( &s_pauseMenu->menu, &s_pauseMenu->help );
	Menu_AddItem( &s_pauseMenu->menu, &s_pauseMenu->exitToMainMenu );
	Menu_AddItem( &s_pauseMenu->menu, &s_pauseMenu->dailyTipText );
}

void UI_PauseMenu( void )
{
	gi.state = GS_LEVEL;

	// force as top level menu
	UI_ForceMenuOff();

	if ( !ui_active->i ) {
		Key_SetCatcher( Key_GetCatcher() | KEYCATCH_UI );
		UI_PushMenu( &s_pauseMenu->menu );
	}
	Key_SetCatcher( Key_GetCatcher() | KEYCATCH_SGAME );
	Snd_PlaySfx( ui->sfx_select );
	Cvar_SetIntegerValue( "g_paused", !ui_active->i );

	if ( !ui_active->i ) {
		Snd_PlaySfx( s_pauseMenu->hPausedSnapshot );
	}
}