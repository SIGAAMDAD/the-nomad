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
    char name[MAX_NPATH];
    fileStats_t stats;
    uint64_t index;
    gamedata_t gd;
    qboolean valid;
    qboolean *modsLoaded;

    char creationTime[32];
    char modificationTime[32];
} saveinfo_t;

typedef struct {
    menuframework_t menu;

    saveinfo_t *saveList;
    uint64_t numSaves;

    uint64_t currentSave;

    nhandle_t load_0;
    nhandle_t load_1;
    qboolean loadGameHovered;

    qboolean saveOpen;

    const stringHash_t *title;
} loadGameMenu_t;

static loadGameMenu_t *s_loadGame;

static void LoadGameMenu_Draw( void )
{
    uint64_t i;
    const int treeNodeFlags = ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_CollapsingHeader
                            | ImGuiTreeNodeFlags_Framed;

    ImGui::Begin( s_loadGame->menu.name, NULL, s_loadGame->menu.flags );
    ImGui::SetWindowSize( ImVec2( s_loadGame->menu.width, s_loadGame->menu.height ) );
    ImGui::SetWindowPos( ImVec2( s_loadGame->menu.x, s_loadGame->menu.y ) );

    UI_EscapeMenuToggle();
    if ( UI_MenuTitle( s_loadGame->title->value, 2.25f ) ) {
        UI_PopMenu();

        ImGui::End();
        return;
    }

    ImGui::SetWindowFontScale( ( ImGui::GetFont()->Scale * 1.5f ) * ui->scale );
    
    if ( s_loadGame->numSaves ) {
        {
            ImGui::Begin( "##ModList", NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove
                | ImGuiWindowFlags_NoMouseInputs | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse
                | ImGuiWindowFlags_NoScrollbar );
            ImGui::SetWindowPos( ImVec2( ui->gpuConfig.vidWidth * 0.75f, 64 * ui->scale ) );
            ImGui::SetWindowSize( ImVec2( ui->gpuConfig.vidWidth * 0.25f, ui->gpuConfig.vidHeight - 10 ) );
            ImGui::SeparatorText( "Loaded Modules" );
            if ( s_loadGame->saveOpen ) {
                for ( uint64_t m = 0; m < s_loadGame->saveList[s_loadGame->currentSave].gd.numMods; m++ ) {
                    if ( !s_loadGame->saveList[s_loadGame->currentSave].modsLoaded[m] ) {
                        ImGui::PushStyleColor( ImGuiCol_Text, colorRed );
                        ImGui::PushStyleColor( ImGuiCol_TextDisabled, colorRed );
                        ImGui::PushStyleColor( ImGuiCol_TextSelectedBg, colorRed );
                    } else {
                        ImGui::PushStyleColor( ImGuiCol_Text, colorGreen );
                        ImGui::PushStyleColor( ImGuiCol_TextDisabled, colorGreen );
                        ImGui::PushStyleColor( ImGuiCol_TextSelectedBg, colorGreen );
                    }
                    ImGui::Text( "%s v%i.%i.%i", s_loadGame->saveList[s_loadGame->currentSave].gd.modList[m].name,
                        s_loadGame->saveList[s_loadGame->currentSave].gd.modList[m].nVersionMajor,
                        s_loadGame->saveList[s_loadGame->currentSave].gd.modList[m].nVersionUpdate,
                        s_loadGame->saveList[s_loadGame->currentSave].gd.modList[m].nVersionPatch );
                    ImGui::PopStyleColor( 3 );
                    if ( ImGui::IsItemHovered() && !s_loadGame->saveList[s_loadGame->currentSave].modsLoaded[m] ) {
                        ImGui::SetTooltip(
                                    "This module either hasn't been loaded or failed to load, check"
                                    "the console/logfile for more detailed information" );
                    }
                }
            }
            ImGui::End();
        }
        FontCache()->SetActiveFont( RobotoMono );
        s_loadGame->saveOpen = qfalse;
        for ( i = 0; i < s_loadGame->numSaves; i++ ) {
            if ( ImGui::CollapsingHeader( s_loadGame->saveList[i].name ) ) {
                if ( ImGui::IsItemActivated() && ImGui::IsMouseClicked( ImGuiMouseButton_Left ) ) {
                    Snd_PlaySfx( ui->sfx_select );
                }
                s_loadGame->saveOpen = qtrue;
                s_loadGame->currentSave = i;

                ImGui::SetWindowFontScale( ( ImGui::GetFont()->Scale * 1.05f ) * ui->scale );
                ImGui::BeginTable( va( "Save Slots##%lu", i ), 3 );
                {
                    ImGui::TableNextColumn();
                    ImGui::TextUnformatted( "Creation Time" );

                    ImGui::TableNextColumn();
                    ImGui::TextUnformatted( "Last Save Time" );
                    
                    ImGui::TableNextColumn();
                    ImGui::TextUnformatted( "Difficulty" );
                    
                    ImGui::TableNextRow();
                    
                    ImGui::TableNextColumn();
                    ImGui::TextUnformatted( s_loadGame->saveList[i].creationTime ); // creation time
                    
                    ImGui::TableNextColumn();
                    ImGui::TextUnformatted( s_loadGame->saveList[i].modificationTime ); // last used time
                    
                    ImGui::TableNextColumn();
                    ImGui::TextUnformatted( difficultyTable[ s_loadGame->saveList[i].gd.dif ].name );
                }
                ImGui::EndTable();
                ImGui::SetWindowFontScale( ( ImGui::GetFont()->Scale * 1.5f ) * ui->scale );
            }
        }
    }
    else {
        ImGui::TextUnformatted( "No Saves" );
    }

    if ( s_loadGame->saveOpen && g_pArchiveHandler->SlotIsUsed( s_loadGame->currentSave ) ) {
        ImGui::SetCursorScreenPos( ImVec2( 528 * ui->scale, 680 * ui->scale ) );
        ImGui::Image( (ImTextureID)(uintptr_t)( s_loadGame->loadGameHovered ? s_loadGame->load_1 : s_loadGame->load_0 ),
			ImVec2( 256 * ui->scale, 72 * ui->scale ) );
        s_loadGame->loadGameHovered = ImGui::IsItemHovered( ImGuiHoveredFlags_AllowWhenDisabled | ImGuiHoveredFlags_DelayNone );
        if ( ImGui::IsItemClicked( ImGuiMouseButton_Left ) ) {
	    	Snd_PlaySfx( ui->sfx_select );
	    	Cvar_SetIntegerValue( "sgame_Difficulty", s_loadGame->saveList[ s_loadGame->currentSave ].gd.dif );
            Cvar_Set( "sgame_SaveName", s_loadGame->saveList[ s_loadGame->currentSave ].name );
            gi.state = GS_LEVEL;
            g_pArchiveHandler->Load( s_loadGame->saveList[ s_loadGame->currentSave ].name );
            Cbuf_ExecuteText( EXEC_APPEND, va( "setmap \"%s\"\n", s_loadGame->saveList[ s_loadGame->currentSave ].gd.mapname ) );
	    }
    }
    ImGui::End();
}

static void LoadGameMenu_InitSaveFiles( void )
{
    struct tm *fileTime;
    saveinfo_t *info;
    const char **fileList;
    uint64_t i, j;
    const char *path;

    if ( ui->uiAllocated ) {
        return;
    }

    //
    // init savefiles
    //

    s_loadGame->numSaves = 0;
    fileList = g_pArchiveHandler->GetSaveFiles( &s_loadGame->numSaves );

    if ( s_loadGame->numSaves ) {
        Cvar_Set( "sgame_NumSaves", va( "%li", (int64_t)s_loadGame->numSaves ) );

        s_loadGame->saveList = (saveinfo_t *)Hunk_Alloc( sizeof( saveinfo_t ) * s_loadGame->numSaves, h_high );
        info = s_loadGame->saveList;

        for ( i = 0; i < s_loadGame->numSaves; i++, info++ ) {
            N_strncpyz( info->name, fileList[i], sizeof( info->name ) );

            info->index = i;

            if ( i >= g_pArchiveHandler->NumUsedSaveSlots() ) {
                continue;
            }
            path = FS_BuildOSPath( FS_GetHomePath(), NULL, va( "SaveData/%s", info->name ) );
            if ( !Sys_GetFileStats( &info->stats, path ) ) { // this should never fail
                N_Error( ERR_DROP, "Failed to stat savefile '%s' even though it exists", path );
            }
            fileTime = localtime( &info->stats.ctime );
            strftime( info->creationTime, sizeof( info->creationTime ) - 1, "%x %-I:%-M:%-S %p", fileTime );

            fileTime = localtime( &info->stats.mtime );
            strftime( info->modificationTime, sizeof( info->modificationTime ) - 1, "%x %-I:%-M:%-S %p", fileTime );

            if ( !g_pArchiveHandler->LoadPartial( info->name, &info->gd ) ) { // just get the header and basic game information
                Con_Printf( COLOR_YELLOW "WARNING: Failed to get valid header data from savefile '%s'\n", info->name );
                info->valid = qfalse;
            } else {
                info->valid = qtrue;
            }

            info->modsLoaded = (qboolean *)Hunk_Alloc( sizeof( *info->modsLoaded ) * info->gd.numMods, h_high );
            for ( uint64_t a = 0; a < info->gd.numMods; a++ ) {
                info->modsLoaded[a] = g_pModuleLib->GetModule( info->gd.modList[a].name ) != NULL;
            }

            COM_StripExtension( fileList[i], info->name, sizeof( info->name ) );
            if ( info->name[ strlen( info->name ) - 1 ] == '.' ) {
                info->name[ strlen( info->name ) - 1 ] = 0;
            }
            Con_Printf( "parsed saved game '%s'\n", info->name );
        }
    }
}

void LoadGameMenu_Cache( void )
{
    const stringHash_t *hardest;

    if ( !ui->uiAllocated ) {
        s_loadGame = (loadGameMenu_t *)Hunk_Alloc( sizeof( *s_loadGame ), h_high );
    }
    memset( s_loadGame, 0, sizeof( *s_loadGame ) );

    s_loadGame->title = strManager->ValueForKey( "SP_LOADGAME_TITLE" );

    s_loadGame->menu.draw = LoadGameMenu_Draw;
    s_loadGame->menu.flags = MENU_DEFAULT_FLAGS | ImGuiWindowFlags_HorizontalScrollbar;
    s_loadGame->menu.name = s_loadGame->title->value;
    s_loadGame->menu.x = 0;
    s_loadGame->menu.y = 0;
    s_loadGame->menu.width = ui->gpuConfig.vidWidth * 0.75f;
    s_loadGame->menu.height = ui->gpuConfig.vidHeight;
    s_loadGame->menu.fullscreen = qtrue;
    s_loadGame->menu.titleFontScale = 3.5f;
    s_loadGame->menu.textFontScale = 1.5f;

    s_loadGame->load_0 = re.RegisterShader( "menu/load_0" );
    s_loadGame->load_1 = re.RegisterShader( "menu/load_1" );

    LoadGameMenu_InitSaveFiles();
}

void UI_LoadGameMenu( void )
{
    UI_PushMenu( &s_loadGame->menu );
}