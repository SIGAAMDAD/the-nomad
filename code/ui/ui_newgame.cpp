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

typedef struct {
	menuframework_t menu;

	char saveName[MAX_NPATH];

	int diff;

	int selectedSaveSlot;
	uint64_t numSaveFiles;
	char **slotNames;
	qboolean slotConflict;

	const char **difficultyList;
	const stringHash_t *hardestString;

	const stringHash_t *title;
	const stringHash_t *newGameSaveNamePrompt;
	const stringHash_t *newGameBegin;

	nhandle_t accept_0;
	nhandle_t accept_1;

	qboolean acceptHovered;
	const void *focusedItem;
} newGameMenu_t;

#define ID_BEGINGAME        1
#define ID_SAVENAMEPROMPT   2
#define ID_DIFFICULTY       3

static newGameMenu_t *s_newGame;

static void SfxFocused( const void *ptr ) {
	if ( ImGui::IsItemHovered( ImGuiHoveredFlags_AllowWhenDisabled | ImGuiHoveredFlags_DelayNone ) ) {
		if ( s_newGame->focusedItem != ptr ) {
			s_newGame->focusedItem = ptr;
			Snd_PlaySfx( ui->sfx_move );
		}
	}
}

static void BeginNewGame( void )
{
	UI_SetActiveMenu( UI_MENU_NONE );

	gi.state = GS_LEVEL;

	N_strncpyz( s_newGame->saveName, COM_SkipPath( s_newGame->saveName ), sizeof( s_newGame->saveName ) );

	Cvar_SetIntegerValue( "g_paused", 0 );
	Cvar_SetIntegerValue( "g_levelIndex", 0 );
	Cvar_Set( "sgame_SaveName", s_newGame->saveName );
	Cvar_Set( "mapname", *gi.mapCache.mapList );

	memset( s_newGame->saveName, 0, sizeof( s_newGame->saveName ) );

	// start a new game
	g_pModuleLib->ModuleCall( sgvm, ModuleOnLevelStart, 0 );
}

static void NewGameMenu_Draw_FixedSlots( void )
{
	int i;

	ImGui::BeginTable( "##SinglePlayerMenuNewGameConfigTable", 2 );
	{
		ImGui::TableNextColumn();
		ImGui::TextUnformatted( s_newGame->newGameBegin->value );
		ImGui::TableNextColumn();

		if ( in_mode->i == 1 ) {
			if ( ImGui::Button( va( "%s##SinglePlayerMenuSaveNamePromptInput", s_newGame->saveName ),
				ImVec2( 528 * ui->scale, 72 * ui->scale ) ) )
			{
				// accessing it through the controller, toggle the on-screen keyboard
				ui->virtKeyboard.open = qtrue;
				ui->virtKeyboard.bufMaxLen = sizeof( s_newGame->saveName );
				ui->virtKeyboard.bufTextLen = strlen( s_newGame->saveName );
				ui->virtKeyboard.pBuffer = s_newGame->saveName;

				Snd_PlaySfx( ui->sfx_select );
			}
			if ( ui->virtKeyboard.open ) {
				if ( UI_VirtualKeyboard( "##SinglePlayerMenuSaveNamePromptInput", s_newGame->saveName, sizeof( s_newGame->saveName ) ) ) {
					Snd_PlaySfx( ui->sfx_select );
				}
			}
		} else if ( in_mode->i == 0 ) {
			if ( ImGui::InputText( "##SinglePlayerMenuSaveNamePromptInput", s_newGame->saveName, sizeof( s_newGame->saveName ) - 1,
				ImGuiInputTextFlags_EnterReturnsTrue ) )
			{
				Snd_PlaySfx( ui->sfx_select );
			}
		}
		SfxFocused( "SinglePlayerNewInput" );

		ImGui::TableNextRow();

		ImGui::TableNextColumn();
		ImGui::TextUnformatted( "Difficulty" );
		ImGui::TableNextColumn();
		if ( ImGui::ArrowButton( "##DifficultySinglePlayerMenuConfigLeft", ImGuiDir_Left ) ) {
			Snd_PlaySfx( ui->sfx_select );
			s_newGame->diff--;
			if ( s_newGame->diff <= DIF_NOOB ) {
				s_newGame->diff = DIF_HARDEST;
			}
		}
		SfxFocused( "DifficultyArrowLeft" );
		ImGui::SameLine();
		if ( ImGui::BeginCombo( "##SinglePlayerMenuDifficultyConfigList", s_newGame->difficultyList[ (int)s_newGame->diff ] ) ) {
			if ( ImGui::IsItemActivated() && ImGui::IsItemClicked( ImGuiMouseButton_Left ) ) {
				Snd_PlaySfx( ui->sfx_select );
			}
			for ( i = 0; i < NUMDIFS; i++ ) {
				if ( ImGui::Selectable( va( "%s##%sSinglePlayerMenuDifficultySelectable_%i", s_newGame->difficultyList[ i ],
					s_newGame->difficultyList[ i ], i ), ( s_newGame->diff == i ) ) )
				{
					Snd_PlaySfx( ui->sfx_select );
					s_newGame->diff = i;
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
			s_newGame->diff++;
			if ( s_newGame->diff > DIF_HARDEST ) {
				s_newGame->diff = DIF_NOOB;
			}
		}
		SfxFocused( "DifficultyArrowRight" );

		ImGui::TableNextRow();

		ImGui::TableNextColumn();
		ImGui::TextUnformatted( "Save Slot" );
		ImGui::TableNextColumn();
		if ( ImGui::ArrowButton( "##SaveSlotSinglePlayerMenuConfigLeft", ImGuiDir_Left ) ) {
			Snd_PlaySfx( ui->sfx_select );
			s_newGame->selectedSaveSlot--;
			if ( s_newGame->selectedSaveSlot < 0 ) {
				s_newGame->selectedSaveSlot = Cvar_VariableInteger( "g_maxSaveSlots" ) - 1;
			}
		}
		SfxFocused( "SaveSlotArrowLeft" );
		ImGui::SameLine();
		if ( ImGui::BeginCombo( "##SinglePlayerMenuSaveSlotConfigList", s_newGame->slotNames[ s_newGame->selectedSaveSlot ] ) ) {
			if ( ImGui::IsItemActivated() && ImGui::IsItemClicked( ImGuiMouseButton_Left ) ) {
				Snd_PlaySfx( ui->sfx_select );
			}
			for ( i = 0; i < s_newGame->numSaveFiles; i++ ) {
				if ( ImGui::Selectable( va( "%s##%sSinglePlayerMenuSaveSlotSelectable_%i", s_newGame->slotNames[i],
					s_newGame->slotNames[i], i ), ( s_newGame->selectedSaveSlot == i ) ) )
				{
					Snd_PlaySfx( ui->sfx_select );
					s_newGame->selectedSaveSlot = i;
				}
			}
			ImGui::EndCombo();
		} else {
			if ( ImGui::IsItemClicked( ImGuiMouseButton_Left ) ) {
				Snd_PlaySfx( ui->sfx_select );
			}
		}
		SfxFocused( "SaveSlotList" );
		ImGui::SameLine();
		if ( ImGui::ArrowButton( "##SaveSlotSinglePlayerMenuConfigRight", ImGuiDir_Right ) ) {
			Snd_PlaySfx( ui->sfx_select );
			s_newGame->selectedSaveSlot++;
			if ( s_newGame->selectedSaveSlot >= Cvar_VariableInteger( "g_maxSaveSlots" ) ) {
				s_newGame->selectedSaveSlot = 0;
			}
		}
		SfxFocused( "SaveSlotArrowRight" );
	}
	ImGui::EndTable();
}

static void NewGameMenu_Draw_DynamicSlots( void )
{
	int i;
	
	ImGui::BeginTable( "##SinglePlayerMenuNewGameConfigTable", 2 );
	{
		ImGui::TableNextColumn();
		ImGui::TextUnformatted( s_newGame->newGameBegin->value );
		ImGui::TableNextColumn();

		if ( in_mode->i == 1 ) {
			if ( ImGui::Button( va( "%s##SinglePlayerMenuSaveNamePromptInput", s_newGame->saveName ),
				ImVec2( 528 * ui->scale, 72 * ui->scale ) ) )
			{
				// accessing it through the controller, toggle the on-screen keyboard
				ui->virtKeyboard.open = qtrue;
				ui->virtKeyboard.bufMaxLen = sizeof( s_newGame->saveName );
				ui->virtKeyboard.bufTextLen = strlen( s_newGame->saveName );
				ui->virtKeyboard.pBuffer = s_newGame->saveName;

				Snd_PlaySfx( ui->sfx_select );
			}
			if ( ui->virtKeyboard.open ) {
				if ( UI_VirtualKeyboard( "##SinglePlayerMenuSaveNamePromptInput", s_newGame->saveName, sizeof( s_newGame->saveName ) ) ) {
					Snd_PlaySfx( ui->sfx_select );
				}
			}
		} else if ( in_mode->i == 0 ) {
			if ( ImGui::InputText( "##SinglePlayerMenuSaveNamePromptInput", s_newGame->saveName, sizeof( s_newGame->saveName ) - 1,
				ImGuiInputTextFlags_EnterReturnsTrue ) )
			{
				Snd_PlaySfx( ui->sfx_select );
			}
		}

		ImGui::TableNextRow();

		ImGui::TableNextColumn();
		ImGui::TextUnformatted( "Difficulty" );
		ImGui::TableNextColumn();
		if ( ImGui::ArrowButton( "##DifficultySinglePlayerMenuConfigLeft", ImGuiDir_Left ) ) {
			Snd_PlaySfx( ui->sfx_select );
			s_newGame->diff--;
			if ( s_newGame->diff <= DIF_NOOB ) {
				s_newGame->diff = DIF_HARDEST;
			}
		}
		ImGui::SameLine();
		if ( ImGui::BeginCombo( "##SinglePlayerMenuDifficultyConfigList", s_newGame->difficultyList[ (int)s_newGame->diff ] ) ) {
			if ( ImGui::IsItemActivated() && ImGui::IsItemClicked( ImGuiMouseButton_Left ) ) {
				Snd_PlaySfx( ui->sfx_select );
			}
			for ( i = 0; i < NUMDIFS; i++ ) {
				if ( ImGui::Selectable( va( "%s##%sSinglePlayerMenuDifficultySelectable_%i", s_newGame->difficultyList[ i ],
					s_newGame->difficultyList[ i ], i ), ( s_newGame->diff == i ) ) )
				{
					Snd_PlaySfx( ui->sfx_select );
					s_newGame->diff = i;
				}
			}
			ImGui::EndCombo();
		} else {
			if ( ImGui::IsItemClicked( ImGuiMouseButton_Left ) ) {
				Snd_PlaySfx( ui->sfx_select );
			}
		}
		ImGui::SameLine();
		if ( ImGui::ArrowButton( "##DifficultySinglePlayerMenuConfigRight", ImGuiDir_Right ) ) {
			Snd_PlaySfx( ui->sfx_select );
			s_newGame->diff++;
			if ( s_newGame->diff > DIF_HARDEST ) {
				s_newGame->diff = DIF_NOOB;
			}
		}
	}
	ImGui::EndTable();
}

static void NewGameMenu_SaveConflict( void )
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
		s_newGame->slotConflict = qfalse;
		BeginNewGame();
	}
	ImGui::SameLine();
	ImGui::TextUnformatted( "/" );
	ImGui::SameLine();
	if ( ImGui::Button( "NO" ) ) {
		s_newGame->slotConflict = qfalse;
	}

	ImGui::End();
}

static void NewGameMenu_Draw( void )
{
	int i;
	extern cvar_t *in_joystick;
	nhandle_t hShader;
	ImVec2 imageSize;

	ImGui::Begin( s_newGame->menu.name, NULL, s_newGame->menu.flags );
	ImGui::SetWindowSize( ImVec2( s_newGame->menu.width, s_newGame->menu.height ) );
	ImGui::SetWindowPos( ImVec2( s_newGame->menu.x, s_newGame->menu.y ) );

	if ( s_newGame->slotConflict ) {
		NewGameMenu_SaveConflict();
	} else {
		UI_EscapeMenuToggle();
	}
	if ( ui->activemenu != &s_newGame->menu ) {
		memset( s_newGame->saveName, 0, sizeof( s_newGame->saveName ) );
		memset( &ui->virtKeyboard, 0, sizeof( ui->virtKeyboard ) );
	}
	if ( UI_MenuTitle( s_newGame->title->value, s_newGame->menu.titleFontScale ) ) {
		UI_PopMenu();
		memset( s_newGame->saveName, 0, sizeof( s_newGame->saveName ) );
		memset( &ui->virtKeyboard, 0, sizeof( ui->virtKeyboard ) );
		return;
	}

	ImGui::SetWindowFontScale( ( ImGui::GetFont()->Scale * s_newGame->menu.textFontScale ) * ui->scale );

	if ( Cvar_VariableInteger( "g_maxSaveSlots" ) == 0 ) {
		NewGameMenu_Draw_DynamicSlots();
	} else {
		NewGameMenu_Draw_FixedSlots();
	}

	ImGui::SetCursorScreenPos( ImVec2( 16 * ui->scale, 300 * ui->scale ) );

/*
	FontCache()->SetActiveFont( AlegreyaSC );
	ImGui::SetWindowFontScale( ( ImGui::GetFont()->Scale * 3.75f ) * ui->scale );
	ImGui::TextUnformatted( "Difficulty Description" );
	ImGui::SetWindowFontScale( ( ImGui::GetFont()->Scale * 1.75f ) * ui->scale );
*/

	FontCache()->SetActiveFont( RobotoMono );
	ImGui::TextWrapped( "%s", difficultyTable[ (int)s_newGame->diff ].tooltip );

	ImGui::End();

	if ( in_mode->i == 1 ) {
		hShader = ui->controller_start;
		imageSize = ImVec2( 172 * ui->scale, 64 * ui->scale );
	} else {
		hShader = s_newGame->acceptHovered ? s_newGame->accept_1 : s_newGame->accept_0;
		imageSize = ImVec2( 256 * ui->scale, 72 * ui->scale );
	}
	
	if ( !sgvm->m_pHandle->IsValid() ) {
		hShader = s_newGame->accept_0;
	}

	ImGui::Begin( "##AcceptNewGameButton", NULL, MENU_DEFAULT_FLAGS );
	ImGui::SetWindowSize( ImVec2( imageSize.x + 26, imageSize.y + 26 ) );
	ImGui::SetWindowPos( ImVec2( 970 * ui->scale, 680 * ui->scale ) );
	ImGui::SetCursorScreenPos( ImVec2( 900 * ui->scale, 680 * ui->scale ) );
	ImGui::Image( (ImTextureID)(uintptr_t)hShader, imageSize );
	if ( !s_newGame->slotConflict ) {
		s_newGame->acceptHovered = ImGui::IsItemHovered( ImGuiHoveredFlags_AllowWhenDisabled | ImGuiHoveredFlags_DelayNone );
		if ( sgvm->m_pHandle->IsValid() ) {
			if ( ImGui::IsItemClicked( ImGuiMouseButton_Left ) || Key_IsDown( KEY_PAD0_START ) ) {
				Snd_PlaySfx( ui->sfx_select );
				if ( Cvar_VariableInteger( "g_maxSaveSlots" ) != 0 && g_pArchiveHandler->SlotIsUsed( s_newGame->selectedSaveSlot ) ) {
					s_newGame->slotConflict = qtrue;
				} else {
					BeginNewGame();
				}
			}
		} else {
			if ( s_newGame->acceptHovered ) {
				FontCache()->SetActiveFont( RobotoMono );
				ImGui::SetItemTooltip( "ERROR: cannot begin new game, SGame module isn't valid, check the console log for details" );
			}
		}
	}
	ImGui::End();
}

int32_t count_fields( const char *line );
char **parse_csv( const char *line );

void NewGameMenu_Cache( void )
{
	const char **saveFiles;
	const stringHash_t *hardest;
	uint64_t i;
	int seed;
	uint64_t hardestIndex, numHardestStrings;
	static const char *difficulties[NUMDIFS];

	if ( !ui->uiAllocated ) {
		s_newGame = (newGameMenu_t *)Hunk_Alloc( sizeof( *s_newGame ), h_high );
	}
	memset( s_newGame, 0, sizeof( *s_newGame ) );
	memset( &ui->virtKeyboard, 0, sizeof( ui->virtKeyboard ) );

	seed = Sys_Milliseconds();
	numHardestStrings = 0;
	while ( strManager->StringExists( va( "SP_DIFF_THE_MEMES_%lu", numHardestStrings ) ) ) {
		numHardestStrings++;
	}

	hardestIndex = Q_rand( &seed ) % numHardestStrings;
	hardest = strManager->ValueForKey( va( "SP_DIFF_THE_MEMES_%lu", hardestIndex ) );

	difficulties[ DIF_NOOB ] = strManager->ValueForKey( "SP_DIFF_VERY_EASY" )->value;
	difficulties[ DIF_RECRUIT ] = strManager->ValueForKey( "SP_DIFF_EASY" )->value;
	difficulties[ DIF_MERC ] = strManager->ValueForKey( "SP_DIFF_MEDIUM" )->value;
	difficulties[ DIF_NOMAD ] = strManager->ValueForKey( "SP_DIFF_HARD" )->value;
	difficulties[ DIF_BLACKDEATH ] = strManager->ValueForKey( "SP_DIFF_VERY_HARD" )->value;
	difficulties[ DIF_HARDEST ] = hardest->value;

	s_newGame->title = strManager->ValueForKey( "SP_NEWGAME" );
	s_newGame->newGameSaveNamePrompt = strManager->ValueForKey( "SP_SAVE_NAME_PROMPT" );
	s_newGame->newGameBegin = strManager->ValueForKey( "SP_BEGIN_NEWGAME" );

	s_newGame->menu.name = "##SinglePlayerNewGameMenu";
	s_newGame->menu.fullscreen = qtrue;
	s_newGame->menu.draw = NewGameMenu_Draw;
	s_newGame->menu.flags = MENU_DEFAULT_FLAGS;
	s_newGame->menu.width = ui->gpuConfig.vidWidth;
	s_newGame->menu.height = ui->gpuConfig.vidHeight - ( 300 * ui->scale );
	s_newGame->menu.titleFontScale = 3.5f;
	s_newGame->menu.textFontScale = 1.5f;
	s_newGame->menu.x = 0;
	s_newGame->menu.y = 0;

	s_newGame->difficultyList = difficulties;

	s_newGame->accept_0 = re.RegisterShader( "menu/accept_0" );
	s_newGame->accept_1 = re.RegisterShader( "menu/accept_1" );

	strcpy( s_newGame->saveName, Cvar_VariableString( "name" ) );

	saveFiles = g_pArchiveHandler->GetSaveFiles( &s_newGame->numSaveFiles );
	if ( !ui->uiAllocated ) {
		s_newGame->slotNames = (char **)Hunk_Alloc( sizeof( *s_newGame->slotNames ) * s_newGame->numSaveFiles, h_high );
	}
	for ( i = 0; i < s_newGame->numSaveFiles; i++ ) {
		s_newGame->slotNames[i] = (char *)Hunk_Alloc( strlen( saveFiles[i] ) + i, h_high );
		COM_StripExtension( saveFiles[i], s_newGame->slotNames[i], MAX_NPATH );
		char *dot = strrchr( s_newGame->slotNames[i], '.' );
		if ( dot != NULL ) {
			*dot = '\0';
		}
	}

	for ( i = 0; i < s_newGame->numSaveFiles; i++ ) {
		if ( !g_pArchiveHandler->SlotIsUsed( i ) ) {
			s_newGame->selectedSaveSlot = i;
			break;
		}
	}

	ImGui::SetNextWindowFocus();
}

void UI_NewGameMenu( void )
{
	NewGameMenu_Cache();
	UI_PushMenu( &s_newGame->menu );
}

/*
//
// a small csv parser for c, credits to semitrivial for this
// https://github.com/semitrivial/csv_parser.git
//

void free_csv_line( char **parsed ) {
	char **ptr;

	for ( ptr = parsed; *ptr; ptr++ ) {
		Z_Free( *ptr );
	}

	Z_Free( parsed );
}

int32_t count_fields( const char *line ) {
	const char *ptr;
	int32_t cnt, fQuote;

	for ( cnt = 1, fQuote = 0, ptr = line; *ptr; ptr++ ) {
		if ( fQuote ) {
			if ( *ptr == '\"' ) {
				fQuote = 0;
			}
			continue;
		}

		switch ( *ptr ) {
		case '\"':
			fQuote = 1;
			continue;
		case ',':
			cnt++;
			continue;
		default:
			continue;
		};
	}

	if ( fQuote ) {
		return -1;
	}

	return cnt;
}

static char *CopyUIString( const char *str ) {
	char *out;
	uint64_t len;

	len = strlen( str ) + 1;
	out = (char *)Hunk_Alloc( len, h_high );
	N_strncpyz( out, str, len );

	return out;
}

//
// Given a string containing no linebreaks, or containing line breaks
// which are escaped by "double quotes", extract a NULL-terminated
// array of strings, one for every cell in the row.
//
char **parse_csv( const char *line ) {
	char **buf, **bptr, *tmp, *tptr;
	const char *ptr;
	int32_t fieldcnt, fQuote, fEnd;

	fieldcnt = count_fields( line );

	if ( fieldcnt == -1 ) {
		return NULL;
	}

	buf = (char **)Hunk_Alloc( sizeof( char * ) * ( fieldcnt + 1 ), h_high );
	tmp = (char *)Hunk_AllocateTempMemory( strlen( line ) + 1 );

	bptr = buf;

	for ( ptr = line, fQuote = 0, *tmp = '\0', tptr = tmp, fEnd = 0; ; ptr++ ) {
		if ( fQuote ) {
			if ( !*ptr ) {
				break;
			}

			if ( *ptr == '\"' ) {
				if ( ptr[1] == '\"' ) {
					*tptr++ = '\"';
					ptr++;
					continue;
				}
				fQuote = 0;
			}
			else {
				*tptr++ = *ptr;
			}

			continue;
		}

		switch ( *ptr ) {
		case '\"':
			fQuote = 1;
			continue;
		case '\0':
			fEnd = 1;
		case ',':
			*tptr = '\0';
			*bptr = CopyUIString( tmp );

			bptr++;
			tptr = tmp;

			if ( fEnd ) {
				break;
			}
			continue;
		default:
			*tptr++ = *ptr;
			continue;
		};

		if ( fEnd ) {
			break;
		}
	}

	*bptr = NULL;
	Hunk_FreeTempMemory( tmp );
	return buf;
}
*/