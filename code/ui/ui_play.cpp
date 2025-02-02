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

#define ID_BEGINGAME        1
#define ID_SAVENAMEPROMPT   2
#define ID_DIFFICULTY       3

#define ID_CONTINUE			0
#define ID_DELETE			1
#define ID_EXIT				2

typedef struct {
	char name[ MAX_NPATH ];
	gamedata_t gd;
	qboolean valid;
	qboolean *modsLoaded;
} saveinfo_t;

typedef struct {
	menuframework_t menu;

	menutext_t continuePlay;
	menutext_t deleteData;
	menutext_t exit;

	nhandle_t background;

	uint32_t numSaveFiles;
	uint32_t hoveredSaveSlot;
	int32_t selectedSaveSlot;

	const void *focusedItem;
	char *slotNames;
	saveinfo_t *saveSlots;

	qboolean slotConflict;
	qboolean deleteSlot;
	qboolean missionSelect;

	menutext_t difficulties[ NUMDIFS ];

	const char **difficultyList;
	const stringHash_t *hardestString;
} playMenu_t;

static playMenu_t *s_playMenu;

extern void Menu_DrawItemGeneric( menucommon_t *generic );
extern void Menu_DrawItemList( void **items, int numitems );

static void SfxFocused( const void *ptr ) {
	if ( ImGui::IsItemHovered( ImGuiHoveredFlags_AllowWhenDisabled | ImGuiHoveredFlags_DelayNone ) ) {
		if ( s_playMenu->focusedItem != ptr ) {
			s_playMenu->focusedItem = ptr;
			Snd_PlaySfx( ui->sfx_move );
		}
	}
}

static void BeginNewGame( void )
{
	const saveinfo_t *slot = &s_playMenu->saveSlots[ s_playMenu->selectedSaveSlot ];

	UI_SetActiveMenu( UI_MENU_NONE );

	gi.mapCache.currentMapLoaded = 1;
	gi.mapLoaded = qtrue;
	gi.state = GS_LEVEL;

	UI_SetActiveMenu( UI_MENU_NONE );
	Cvar_SetIntegerValue( "g_paused", 0 );
	Cvar_SetIntegerValue( "g_levelIndex", 0 );
	Cvar_Set( "mapname", *gi.mapCache.mapList );
	Cvar_Set( "nextmap", "" );

	Con_Printf( "Beginning new game on map \"%s\"...\n", *gi.mapCache.mapList );

	gi.playTimeStart = Sys_Milliseconds();

	// start a new game
	g_pModuleLib->ModuleCall( sgvm, ModuleOnLevelStart, 0 );
}

static void PlayMenu_MissionSelect( void )
{
	int i;
	saveinfo_t *slot;

	slot = &s_playMenu->saveSlots[ s_playMenu->selectedSaveSlot ];

	ImGui::SetWindowFontScale( ( ImGui::GetFont()->Scale * 2.75f ) * ui->scale );
	ImGui::SetCursorScreenPos( ImVec2( 200 * ui->scale, 390 * ui->scale ) );
	{
		ImGui::TableNextColumn();
		if ( ImGui::ArrowButton( "##MapIndexArrowLeft", ImGuiDir_Left ) ) {
			Snd_PlaySfx( ui->sfx_select );
			slot->gd.mapIndex--;
			if ( slot->gd.mapIndex < 0 ) {
				slot->gd.mapIndex = gi.mapCache.numMapFiles - 1;
			}
		}
		SfxFocused( "MapIndexArrowLeft" );
		ImGui::SameLine();
		ImGui::TextUnformatted( gi.mapCache.mapList[ slot->gd.mapIndex ] );
		SfxFocused( gi.mapCache.mapList[ slot->gd.mapIndex ] );
		ImGui::SameLine();
		if ( ImGui::ArrowButton( "##MapIndexArrowRight", ImGuiDir_Right ) ) {
			Snd_PlaySfx( ui->sfx_select );
			slot->gd.mapIndex++;
			if ( slot->gd.mapIndex >= gi.mapCache.numMapFiles ) {
				slot->gd.mapIndex = 0;
			}
		}
		SfxFocused( "MapIndexArrowRight" );
	}
	{
		if ( ImGui::ArrowButton( "##DifficultySinglePlayerMenuConfigLeft", ImGuiDir_Left ) ) {
			Snd_PlaySfx( ui->sfx_select );
			slot->gd.saveDif--;
			if ( slot->gd.saveDif < DIF_EASY ) {
				slot->gd.saveDif = slot->gd.highestDif;
			}
		}
		SfxFocused( "DifficultyArrowLeft" );
		ImGui::SameLine();
		if ( ImGui::BeginCombo( "##SinglePlayerMenuDifficultyConfigList", s_playMenu->difficultyList[ slot->gd.saveDif ] ) ) {
			if ( ImGui::IsItemActivated() && ImGui::IsItemClicked( ImGuiMouseButton_Left ) ) {
				Snd_PlaySfx( ui->sfx_select );
			}
			for ( i = 0; i < slot->gd.highestDif + 1; i++ ) {
				if ( ImGui::Selectable( va( "%s##%sSinglePlayerMenuDifficultySelectable_%i", s_playMenu->difficultyList[ i ],
					s_playMenu->difficultyList[ i ], i ), ( slot->gd.saveDif == i ) ) )
				{
					Snd_PlaySfx( ui->sfx_select );
					slot->gd.saveDif = i;
				}
			}
			ImGui::EndCombo();
		} else {
			if ( ImGui::IsItemClicked( ImGuiMouseButton_Left ) ) {
				Snd_PlaySfx( ui->sfx_select );
			}
		}
		SfxFocused( "DifficultyList" );
		ImGui::SameLine();
		if ( ImGui::ArrowButton( "##DifficultySinglePlayerMenuConfigRight", ImGuiDir_Right ) ) {
			Snd_PlaySfx( ui->sfx_select );
			slot->gd.saveDif++;
			if ( slot->gd.saveDif > slot->gd.highestDif ) {
				slot->gd.saveDif = DIF_EASY;
			}
		}
		SfxFocused( "DifficultyArrowRight" );
	}

	const ImVec4& color = ImGui::GetStyle().Colors[ ImGuiCol_Button ];
	for ( i = 0; i < gi.mapCache.numMapFiles; i++ ) {
		if ( slot->gd.mapIndex == i ) {
			ImGui::PushStyleColor( ImGuiCol_Button, ImVec4( 0.0f, 0.0f, 1.0f, 1.0f ) );
			ImGui::PushStyleColor( ImGuiCol_ButtonActive, ImVec4( 0.0f, 0.0f, 1.0f, 1.0f ) );
			ImGui::PushStyleColor( ImGuiCol_ButtonHovered, ImVec4( 0.0f, 0.0f, 1.0f, 1.0f ) );
		} else {
			ImGui::PushStyleColor( ImGuiCol_Button, color );
			ImGui::PushStyleColor( ImGuiCol_ButtonActive, color );
			ImGui::PushStyleColor( ImGuiCol_ButtonHovered, color );
		}
		ImGui::Button( "" );
		ImGui::PopStyleColor( 3 );
		if ( i != gi.mapCache.numMapFiles - 1 ) {
			ImGui::SameLine();
		}
	}
}

static void DeleteSlot_Draw( void )
{
	ImGui::TextUnformatted(
		"                   *** WARNING ***             \n"
		"You are about to delete a save slot permanently\n"
		"              Are you sure about this?         \n"
	);
}

static void DeleteSlot_Event( qboolean action )
{
	if ( !action ) {
		return;
	}

	g_pArchiveHandler->DeleteSlot( s_playMenu->selectedSaveSlot );

	s_playMenu->deleteSlot = qfalse;
	memset( &s_playMenu->saveSlots[ s_playMenu->selectedSaveSlot ], 0, sizeof( saveinfo_t ) );
	s_playMenu->selectedSaveSlot = 0;
	s_playMenu->numSaveFiles = 0;

	UI_PopMenu();
}

static void MissionMenu_Event( void *ptr, int event )
{
	if ( event != EVENT_ACTIVATED ) {
		return;
	}
	
	saveinfo_t *slot = &s_playMenu->saveSlots[ s_playMenu->selectedSaveSlot ];

	switch ( ( (const menucommon_t *)ptr )->id ) {
	case ID_CONTINUE: {
		const saveinfo_t *slot = &s_playMenu->saveSlots[ s_playMenu->selectedSaveSlot ];

		UI_SetActiveMenu( UI_MENU_NONE );

		gi.state = GS_LEVEL;

		Con_Printf( "Continuing game on map \"%s\"...\n", gi.mapCache.mapList[ slot->gd.mapIndex ] );

		Cvar_SetIntegerValue( "g_paused", 0 );
		Cvar_SetIntegerValue( "g_levelIndex", 0 );
		Cvar_SetIntegerValue( "sgame_SaveSlot", slot - s_playMenu->saveSlots );
		Cvar_Set( "mapname", gi.mapCache.mapList[ slot->gd.mapIndex ] );

		gi.playTimeStart = Sys_Milliseconds();

		g_pArchiveHandler->Load( s_playMenu->selectedSaveSlot );
		break; }
	case ID_DELETE:
		UI_ConfirmMenu( "", DeleteSlot_Draw, DeleteSlot_Event );
		break;
	case ID_EXIT:
		UI_PopMenu();
		break;
	};
}

static void PlayMenu_DrawSlotEdit( void )
{
	int i;
	saveinfo_t *slot;
	nhandle_t hShader;
	ImVec2 imageSize;

	slot = &s_playMenu->saveSlots[ s_playMenu->selectedSaveSlot ];

	// mission select menu
	if ( s_playMenu->missionSelect ) {
		PlayMenu_MissionSelect();
		return;
	}

	Menu_DrawItemGeneric( &s_playMenu->continuePlay.generic );
	Menu_DrawItemGeneric( &s_playMenu->deleteData.generic );
	Menu_DrawItemGeneric( &s_playMenu->exit.generic );
}

static void PlayMenu_DrawNewGameEdit( void )
{
	int i;
	saveinfo_t *slot;
	nhandle_t hShader;
	ImVec2 imageSize;
	int focusedDif;

	slot = &s_playMenu->saveSlots[ s_playMenu->selectedSaveSlot ];

	focusedDif = 0;
	for ( i = 0; i < NUMDIFS; i++ ) {
		Menu_DrawItemGeneric( &s_playMenu->difficulties[i].generic );
		if ( s_playMenu->difficulties[i].generic.focused ) {
			focusedDif = i;
			Cvar_SetIntegerValue( "sgame_Difficulty", i );
		}
	}

	ImGui::SetWindowFontScale( 1.5f );
	ImGui::SetCursorScreenPos( ImVec2( 16 * ui->scale, 600 * ui->scale ) );
	FontCache()->SetActiveFont( RobotoMono );
	ImGui::TextWrapped( "%s", difficultyTable[ focusedDif ].tooltip );

	ImGui::End();
}

static void PlayMenu_SaveConflict( void )
{
	ImGui::Begin( "Save Slot Overwrite", NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse
		| ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoMove );
	
	ImGui::SetWindowSize( ImVec2( 256 * ui->scale, 72 * ui->scale ) );
	ImGui::SetWindowPos( ImVec2( 480 * ui->scale, 460 * ui->scale ) );
	ImGui::TextUnformatted( "                   *** WARNING ***                   " );
	ImGui::TextUnformatted( "You about to overwrite a save slot with existing data" );
	ImGui::TextUnformatted( "Are you sure about this?                             " );
	ImGui::SetCursorScreenPos( ImVec2( 106 * ui->scale, 490 * ui->scale ) );
	if ( ImGui::Button( "YES" ) ) {
		s_playMenu->slotConflict = qfalse;
		BeginNewGame();
	}
	ImGui::SameLine();
	ImGui::TextUnformatted( "/" );
	ImGui::SameLine();
	if ( ImGui::Button( "NO" ) ) {
		s_playMenu->slotConflict = qfalse;
	}

	ImGui::End();
}

static void PlayMenu_Draw_SaveSlotSelect( void )
{
	uint64_t i;

	ImGui::BeginTable( "##SaveSlotsConfigListSelector", 1, ImGuiTableFlags_BordersH | ImGuiTableFlags_BordersOuterV );

	ImGui::SetWindowFontScale( ( ImGui::GetFont()->Scale * 2.25f ) * ui->scale );

	s_playMenu->selectedSaveSlot = -1;

	for ( i = 0; i < g_maxSaveSlots->i; i++ ) {
		if ( s_playMenu->hoveredSaveSlot == i ) {
		} else {
		}
		ImGui::TableNextColumn();

		if ( !g_pArchiveHandler->SlotIsUsed( i ) ) {
			if ( ImGui::MenuItem( va( "EMPTY##EMPTYSAVESLOT%lu", i ) ) ) {
				Snd_PlaySfx( ui->sfx_select );
				s_playMenu->selectedSaveSlot = i;
			}
		} else {
			if ( ImGui::MenuItem( va( "Save Slot %lu##USEDSAVESLOT%lu", i, i ) ) )
			{
				Snd_PlaySfx( ui->sfx_select );
				s_playMenu->selectedSaveSlot = i;
				ui->menubackShader = s_playMenu->background;
			}
		}
		SfxFocused( &s_playMenu->saveSlots[ i ] );
		if ( i <= g_maxSaveSlots->i - 1 ) {
			ImGui::TableNextRow();
		}
	}
	ImGui::EndTable();
}

static void PlayMenu_Draw( void )
{
	int i;
	extern cvar_t *in_joystick;
	const char *menuTitle;

	ui->menubackShader = s_playMenu->background;

	if ( s_playMenu->selectedSaveSlot != -1 ) {
		if ( g_pArchiveHandler->SlotIsUsed( s_playMenu->selectedSaveSlot ) ) {
			menuTitle = "LEVEL";
		} else {
			menuTitle = "NEW GAME";
		}
	} else {
		menuTitle = "SAVE SLOTS";
	}

	ImGui::Begin( s_playMenu->menu.name, NULL, s_playMenu->menu.flags );
	ImGui::SetWindowSize( ImVec2( s_playMenu->menu.width, s_playMenu->menu.height ) );
	ImGui::SetWindowPos( ImVec2( s_playMenu->menu.x, s_playMenu->menu.y ) );

	if ( s_playMenu->slotConflict ) {
		PlayMenu_SaveConflict();
	} else {
		UI_EscapeMenuToggle();
	}
	if ( ui->activemenu != &s_playMenu->menu ) {
		s_playMenu->hoveredSaveSlot = 0;
		s_playMenu->selectedSaveSlot = -1;
	}
	if ( UI_MenuTitle( menuTitle, s_playMenu->menu.titleFontScale ) ) {
		UI_PopMenu();
		s_playMenu->hoveredSaveSlot = 0;
		s_playMenu->selectedSaveSlot = -1;
		return;
	}

	ImGui::SetWindowFontScale( ( ImGui::GetFont()->Scale * s_playMenu->menu.textFontScale ) * ui->scale );

	if ( s_playMenu->selectedSaveSlot == -1 ) {
		PlayMenu_Draw_SaveSlotSelect();
	} else if ( g_pArchiveHandler->SlotIsUsed( s_playMenu->selectedSaveSlot ) ) {
		PlayMenu_DrawSlotEdit();
	} else {
		PlayMenu_DrawNewGameEdit();
	}
	ImGui::End();
}

void UI_ReloadSaveFiles_f( void )
{
	uint64_t i, numSaveFiles;
	const char **saveFiles;
	saveinfo_t *info;

	saveFiles = g_pArchiveHandler->GetSaveFiles( &numSaveFiles );
	s_playMenu->numSaveFiles = numSaveFiles;

	s_playMenu->saveSlots = (saveinfo_t *)Z_Malloc( sizeof( saveinfo_t ) * s_playMenu->numSaveFiles, TAG_SAVEFILE );
	memset( s_playMenu->saveSlots, 0, sizeof( saveinfo_t ) * s_playMenu->numSaveFiles );
	info = s_playMenu->saveSlots;

	for ( i = 0; i < g_maxSaveSlots->i; i++ ) {
		char szMapName[ MAX_NPATH ];

		if ( !g_pArchiveHandler->SlotIsUsed( i ) ) {
			info->valid = qtrue;
			info->gd.highestDif = DIF_HARD;
			continue;
		}

		Com_snprintf( info->name, sizeof( info->name ), "%lu", i + 1 );

		info->valid = g_pArchiveHandler->LoadPartial( i, &info->gd );
		if ( !info->valid ) {
			Con_Printf( COLOR_YELLOW "WARNING: Failed to get valid header data from savefile '%s'\n", info->name );
		}

		COM_StripExtension( gi.mapCache.mapList[ info->gd.mapIndex ], szMapName, sizeof( szMapName ) );
		szMapName[ strlen( szMapName ) - 1 ] = '\0';
	}
}

static void PlayMenu_InitSlots( void )
{
	if ( ui->uiAllocated ) {
		return;
	}

	UI_ReloadSaveFiles_f();
}

static void PlayMenu_DifficultyEvent( void *self, int event )
{
	if ( event != EVENT_ACTIVATED ) {
		return;
	}
	s_playMenu->saveSlots[ s_playMenu->selectedSaveSlot ].gd.saveDif = ( (const menucommon_t *)self )->id;
	BeginNewGame();
}

void PlayMenu_Cache( void )
{
	const stringHash_t *hardest;
	uint64_t i;
	int seed;
	uint64_t hardestIndex, numHardestStrings;
	static const char *difficulties[NUMDIFS];

	if ( !ui->uiAllocated ) {
		static playMenu_t menu;
		s_playMenu = &menu;
	}

	seed = Sys_Milliseconds();
	numHardestStrings = 0;
	while ( strManager->StringExists( va( "SP_DIFF_THE_MEMES_%lu", numHardestStrings ) ) ) {
		numHardestStrings++;
	}

	hardestIndex = Q_rand( &seed ) % numHardestStrings;
	hardest = strManager->ValueForKey( va( "SP_DIFF_THE_MEMES_%lu", hardestIndex ) );

	difficulties[ DIF_EASY ] = strManager->ValueForKey( "SP_DIFF_EASY" )->value;
	difficulties[ DIF_NORMAL ] = strManager->ValueForKey( "SP_DIFF_NORMAL" )->value;
	difficulties[ DIF_HARD ] = strManager->ValueForKey( "SP_DIFF_HARD" )->value;
	difficulties[ DIF_VERY_HARD ] = strManager->ValueForKey( "SP_DIFF_VERY_HARD" )->value;
	difficulties[ DIF_INSANE ] = strManager->ValueForKey( "SP_DIFF_INSANE" )->value;
	difficulties[ DIF_MEME ] = hardest->value;

	s_playMenu->menu.name = "##SinglePlayerNewGameMenu";
	s_playMenu->menu.fullscreen = qtrue;
	s_playMenu->menu.draw = PlayMenu_Draw;
	s_playMenu->menu.flags = MENU_DEFAULT_FLAGS;
	s_playMenu->menu.width = gi.gpuConfig.vidWidth;
	s_playMenu->menu.height = gi.gpuConfig.vidHeight - ( 100 * ui->scale );
	s_playMenu->menu.titleFontScale = 3.5f;
	s_playMenu->menu.textFontScale = 1.75f;
	s_playMenu->menu.x = 0;
	s_playMenu->menu.y = 0;
	s_playMenu->menu.track = Snd_RegisterSfx( "event:/music/menu/main_theme" );

	s_playMenu->continuePlay.generic.type = MTYPE_TEXT;
	s_playMenu->continuePlay.generic.eventcallback = MissionMenu_Event;
	s_playMenu->continuePlay.generic.flags = QMF_HIGHLIGHT_IF_FOCUS;
	s_playMenu->continuePlay.generic.id = ID_CONTINUE;
	s_playMenu->continuePlay.generic.parent = &s_playMenu->menu;
	s_playMenu->continuePlay.color = color_white;
	s_playMenu->continuePlay.text = "CONTINUE";

	s_playMenu->deleteData.generic.type = MTYPE_TEXT;
	s_playMenu->deleteData.generic.eventcallback = MissionMenu_Event;
	s_playMenu->deleteData.generic.flags = QMF_HIGHLIGHT_IF_FOCUS;
	s_playMenu->deleteData.generic.id = ID_DELETE;
	s_playMenu->deleteData.generic.parent = &s_playMenu->menu;
	s_playMenu->deleteData.color = color_white;
	s_playMenu->deleteData.text = "DELETE";

	s_playMenu->exit.generic.type = MTYPE_TEXT;
	s_playMenu->exit.generic.eventcallback = MissionMenu_Event;
	s_playMenu->exit.generic.flags = QMF_HIGHLIGHT_IF_FOCUS;
	s_playMenu->exit.generic.id = ID_EXIT;
	s_playMenu->exit.generic.parent = &s_playMenu->menu;
	s_playMenu->exit.color = color_white;
	s_playMenu->exit.text = "EXIT";

	for ( i = 0; i < NUMDIFS; i++ ) {
		s_playMenu->difficulties[i].color = color_red;
		s_playMenu->difficulties[i].text = difficulties[i];
		s_playMenu->difficulties[i].generic.parent = &s_playMenu->menu;
		s_playMenu->difficulties[i].generic.flags = QMF_HIGHLIGHT_IF_FOCUS;
		s_playMenu->difficulties[i].generic.type = MTYPE_TEXT;
		s_playMenu->difficulties[i].generic.font = AlegreyaSC;
		s_playMenu->difficulties[i].generic.eventcallback = PlayMenu_DifficultyEvent;
		s_playMenu->difficulties[i].generic.id = i;
	}

	s_playMenu->difficultyList = difficulties;

	s_playMenu->selectedSaveSlot = -1;
	s_playMenu->hoveredSaveSlot = 0;

	s_playMenu->background = re.RegisterShader( "menu/playBackground" );
	ui->menubackShader = s_playMenu->background;

	PlayMenu_InitSlots();

	ImGui::SetNextWindowFocus();
}

void UI_PlayMenu( void )
{
	PlayMenu_Cache();
	UI_PushMenu( &s_playMenu->menu );
}