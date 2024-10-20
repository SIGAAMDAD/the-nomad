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

#define MAX_MODS			1024
#define NAMEBUFSIZE			( MAX_MODS * 48 )
#define GAMEBUFSIZE			( MAX_MODS * 16 )

typedef struct {
	module_t *modList;
    uint32_t numMods;

	menuframework_t menu;

    nhandle_t backgroundShader;
    nhandle_t ambience;
	qboolean changed;
    
    uint32_t selectedMod;
} modmenu_t;

static modmenu_t *mods;

static uint32_t GetModLoadIndex( const char *pName ) {
    uint32_t i;

    for ( i = 0; i < mods->numMods; i++ ) {
        if ( !N_strcmp( pName, mods->modList[i].info->m_szName ) ) {
            break;
        }
    }

    return i;
}

static void ModsMenu_ClearLoadList_f( void ) {
    FS_Remove( CACHE_DIR "/loadlist.json" );
    FS_HomeRemove( CACHE_DIR "/loadlist.json" );
}

static const module_t *GetModuleFromName( const char *pName )
{
    uint64_t i;

    for ( i = 0; i < mods->numMods; i++ ) {
        if ( !N_strcmp( pName, mods->modList[i].info->m_szName ) && mods->modList[i].active ) {
            return &mods->modList[i];
        }
    }

    return NULL;
}

static void ModsMenu_LoadMod( module_t *mod ) {
}

qboolean ModsMenu_IsModuleActive( const char *pName ) {
    uint32_t i;

    for ( i = 0; i < mods->numMods; i++ ) {
        if ( !N_strcmp( pName, mods->modList[i].info->m_szName ) && ( mods->modList[i].active || mods->modList[i].valid ) ) {
            return qtrue;
        }
    }
    return qfalse;
}

void ModsMenu_SaveModList( void )
{
    uint32_t i, j;
    fileHandle_t f;

    Con_Printf( "Saving mod list...\n" );
    
    f = FS_FOpenWrite( CACHE_DIR "/loadlist.cfg" );
    if ( f == FS_INVALID_HANDLE ) {
    	N_Error( ERR_DROP, "ModsMenu_SaveModList: failed to save " CACHE_DIR "/loadlist.cfg" );
    }
    
    for ( i = 0; i < mods->numMods; i++ ) {
    	FS_Printf( f, "\"%s\" %i %i\n" , mods->modList[i].info->m_szName, mods->modList[i].valid, mods->modList[i].active );
    }
    
    FS_FClose( f );
}

static void ModsMenu_LoadModList( void )
{
    char *b;
    uint64_t nLength;
	int i, j;
	const char **text;
	const char *text_p;
	const char *tok;
	char *modName;
	char **loadList;
    nlohmann::json json;

    nLength = FS_LoadFile( CACHE_DIR "/loadlist.json", (void **)&b );
    if ( !nLength || !b ) {
        return;
    }

    try {
        json = nlohmann::json::parse( b, b + nLength );
    } catch ( const nlohmann::json::exception& e ) {
        Con_Printf( COLOR_RED "ModsMenu_LoadModList: invalid loadlist.json (nlohmann::json::exception) ->\n  id: %i\n  message: %s\n",
            e.id, e.what() );
		FS_FreeFile( b );
		return;
    }
    FS_FreeFile( b );

    if ( json.at( "LoadList" ).size() != mods->numMods ) {
        Con_Printf( COLOR_YELLOW "WARNING: ModsMenu_LoadModList: bad load list, mods in list different than in memory\n" );
    }

    const nlohmann::json& data = json.at( "LoadList" );
    for ( i = 0; i < mods->numMods; i++ ) {
        for ( const auto& it : data ) {
			if ( !N_strcmp( mods->modList[i].info->m_szName, it.at( "Name" ).get<nlohmann::json::string_t>().c_str() ) ) {
			    mods->modList[i].valid = it.at( "Valid" );
			    mods->modList[i].active = it.at( "Active" );
			    mods->modList[i].bootIndex = i;
			}
        }
    }
	
    for ( i = 0; i < mods->numMods; i++ ) {
		module_t m = mods->modList[ mods->modList[i].bootIndex ];
		mods->modList[ mods->modList[i].bootIndex ] = mods->modList[i];
		mods->modList[i] = m;
    }
}

static void ModToggleActive( module_t *mod, qboolean valid )
{
	uint32_t i, j;
	if ( valid ) {
		mod->active = !mod->active;
	} else {
		return;
	}
	
	// toggle anything thay may depend on this mod
	for ( i = 0; i < mods->numMods; i++ ) {
		for ( j = 0; j < mods->modList[i].info->m_nDependencies; j++ ) {
			if ( N_strcmp( mod->info->m_szName, mods->modList[i].info->m_pDependencies[j].c_str() ) == 0 ) {
				module_t *m = eastl::find( mods->modList, mods->modList + mods->numMods, mods->modList[i].info->m_pDependencies[j] );
				if ( m != &mods->modList[ mods->numMods ] ) {
					m->active = mod->active;
				} else {
					Con_Printf( COLOR_YELLOW "WARNING: weird module dependency mismatch.\n" );
				}
			}
		}
	}
}

static void ModsMenu_DrawListing( module_t *mod, qboolean dimColor )
{
	uint32_t j;
	const int index = (int)( mods->modList - mod );
	const qboolean active = ( mod->active && mod->valid ) || mod->isRequired;
	
	ImGui::TableNextColumn();
	if ( dimColor ) {
		// dim it a little bit
		ImGui::PushStyleColor( ImGuiCol_FrameBg, ImVec4( 0.75f, 0.75f, 0.75f, 1.0f ) );
		ImGui::PushStyleColor( ImGuiCol_FrameBgActive, ImVec4( 0.75f, 0.75f, 0.75f, 1.0f ) );
		ImGui::PushStyleColor( ImGuiCol_FrameBgHovered, ImVec4( 0.75f, 0.75f, 0.75f, 1.0f ) );
	}
	
	if ( active ) {
		ImGui::PushStyleColor( ImGuiCol_FrameBgHovered, ImVec4( 0.71f, 0.65f, 0.26f, 1.0f ) );
		ImGui::PushStyleColor( ImGuiCol_FrameBgActive, ImVec4( 0.71f, 0.65f, 0.26f, 1.0f ) );
	}
	if ( ImGui::RadioButton( mod->active ? va( "##ModLoad%iON", index ) : va( "##ModLoad%iOFF", index ),
		mod->active ) )
	{
		Snd_PlaySfx( ui->sfx_select );
		ModToggleActive( mod, !dimColor );
	}
	if ( active ) {
		ImGui::PopStyleColor( 2 );
	}
	
	ImGui::TableNextColumn();
	if ( mod->numDependencies ) {
		ImGui::PushStyleColor( ImGuiCol_Text, ImVec4( 0.71f, 0.65f, 0.26f, 1.0f ) );
		ImGui::PushStyleColor( ImGuiCol_TextDisabled, ImVec4( 0.71f, 0.65f, 0.26f, 1.0f ) );
		ImGui::PushStyleColor( ImGuiCol_FrameBg, ImVec4( 0.0f, 0.0f, 0.0f, 0.0f ) );
		ImGui::PushStyleColor( ImGuiCol_FrameBgActive, ImVec4( 0.0f, 0.0f, 0.0f, 0.0f ) );
		ImGui::PushStyleColor( ImGuiCol_FrameBgHovered, ImVec4( 0.0f, 0.0f, 0.0f, 0.0f ) );
		ImGui::PushStyleColor( ImGuiCol_ButtonHovered, ImVec4( 0.0f, 0.0f, 0.0f, 1.0f ) );
		ImGui::Text( "(!)" );
		ImGui::PopStyleColor( 6 );
		if ( ImGui::IsItemHovered( ImGuiHoveredFlags_AllowWhenDisabled | ImGuiHoveredFlags_DelayShort ) ) {
			ImGui::BeginTooltip();
			const float fontScale = ImGui::GetFont()->Scale;
			ImGui::SetWindowFontScale( fontScale * 1.5f );
			ImGui::SeparatorText( "this module depends on" );
			for ( j = 0; j < mod->info->m_nDependencies; j++ ) {
				ImGui::TextUnformatted( mod->info->m_pDependencies[j].c_str() );
			}
			ImGui::SetWindowFontScale( fontScale );
			ImGui::EndTooltip();
		}
		ImGui::SameLine();
	}
	if ( mod->info->m_GameVersion.m_nVersionMajor != NOMAD_VERSION
		|| mod->info->m_GameVersion.m_nVersionUpdate != NOMAD_VERSION_UPDATE )
	{
		ImGui::PushStyleColor( ImGuiCol_Text, ImVec4( 1.0f, 0.0f, 0.0f, 1.0f ) );
		ImGui::PushStyleColor( ImGuiCol_TextDisabled, ImVec4( 1.0f, 0.0f, 0.0f, 1.0f ) );
		ImGui::PushStyleColor( ImGuiCol_FrameBg, ImVec4( 0.0f, 0.0f, 0.0f, 0.0f ) );
		ImGui::PushStyleColor( ImGuiCol_FrameBgActive, ImVec4( 0.0f, 0.0f, 0.0f, 0.0f ) );
		ImGui::PushStyleColor( ImGuiCol_FrameBgHovered, ImVec4( 0.0f, 0.0f, 0.0f, 0.0f ) );
		ImGui::PushStyleColor( ImGuiCol_ButtonHovered, ImVec4( 0.0f, 0.0f, 0.0f, 1.0f ) );
		ImGui::Text( "(!)" );
		ImGui::PopStyleColor( 6 );
		if ( ImGui::IsItemHovered( ImGuiHoveredFlags_AllowWhenDisabled | ImGuiHoveredFlags_DelayShort ) ) {
			ImGui::BeginTooltip();
			ImGui::TextUnformatted( "this module has been built with an outdated/unsupported game version" );
			// not really my fault if the modder hasn't update their shit
			ImGui::TextUnformatted( "NOTE: GDR Games is not responsible for any unstable experiences when using this mod" );
			ImGui::EndTooltip();
		}
		ImGui::SameLine();
	}
}

void ModsMenu_Draw( void )
{
    uint32_t i, j;
    const int windowFlags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse |
                            ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBackground;
    ImGuiStyle *style;
    char loadId[MAX_STRING_CHARS];
    qboolean dimColor;
	float itemSpacing;
    const float fontScale = ImGui::GetFont()->Scale;
    extern ImFont *RobotoMono;
	module_t *swap;
    
	ui->menubackShader = mods->backgroundShader;

	ImGui::Begin( "##ModsMenuMainMenuConfigMenu", NULL, mods->menu.flags );
	ImGui::SetWindowSize( ImVec2( mods->menu.width, mods->menu.height ) );
	ImGui::SetWindowPos( ImVec2( mods->menu.x, mods->menu.y ) );
    
	UI_EscapeMenuToggle();
    if ( UI_MenuTitle( strManager->ValueForKey( "MOD_MENU_TITLE" )->value, 1.75f ) ) {
		UI_MainMenu();
        ModsMenu_SaveModList();
        return;
    }
    
    style = eastl::addressof( ImGui::GetStyle() );
	itemSpacing = style->ItemSpacing.y;
    style->ItemSpacing.y = 50.0f;
   	
   	ImGui::BeginTable( "##ModLoadList", 5 );

   	ImGui::SetWindowFontScale( ( fontScale * 1.5f ) );
	ImGui::TableNextColumn();
   	ImGui::TextUnformatted( "Active" );
	ImGui::TableNextColumn();
	ImGui::TextUnformatted( "Name" );
	ImGui::TableNextColumn();
   	ImGui::TextUnformatted( "Mod Version" );
	ImGui::TableNextColumn();
   	ImGui::TextUnformatted( "Game Version" );
	ImGui::TableNextColumn();
	ImGui::SetWindowFontScale( ( fontScale * 1.75f ) );
	
	if ( RobotoMono ) {
		FontCache()->SetActiveFont( RobotoMono );
	}

	ImGui::TableNextRow();
   	
	swap = NULL;
   	for ( i = 0; i < mods->numMods; i++ ) {
   		Com_snprintf( loadId, sizeof( loadId ) - 1, "%s##ModLoad%i", mods->modList[i].info->m_szName, i );
   		ImGui::PushID( i );

		if ( mods->modList[i].isRequired || !mods->modList[i].allDepsActive ) {
	   		dimColor = qtrue;
	   	} else {
	   		dimColor = qfalse;
	   	}
   		
   		if ( Key_IsDown( KEY_UPARROW ) ) {
			Snd_PlaySfx( ui->sfx_select );
   			if ( mods->selectedMod == 0 ) {
   				mods->selectedMod = mods->numMods - 1;
   			} else {
	   			mods->selectedMod--;
	   		}
		}
   		if ( Key_IsDown( KEY_DOWNARROW ) ) {
			Snd_PlaySfx( ui->sfx_select );
   			mods->selectedMod++;
   			if ( mods->selectedMod >= mods->numMods ) {
   				mods->selectedMod = 0;
   			}
   		}
   		if ( Key_IsDown( KEY_ENTER ) ) {
			Snd_PlaySfx( ui->sfx_select );
   			ModToggleActive( &mods->modList[ mods->selectedMod ], !dimColor );
   		}
		ModsMenu_DrawListing( &mods->modList[i], dimColor );
		if ( ImGui::BeginDragDropSource( ImGuiDragDropFlags_SourceNoDisableHover ) ) {
			ImGui::SetDragDropPayload( "##ModuleReorderDragDrop", &mods->modList[i], sizeof( module_t ), ImGuiCond_Once );
			ImGui::EndDragDropSource();
		}
		if ( ImGui::BeginDragDropTarget() ) {
			const ImGuiPayload *pData = ImGui::AcceptDragDropPayload( "##ModuleReorderDragDrop" );
			if ( pData != NULL ) {
				if ( pData->DataSize != sizeof( *swap ) ) {
					N_Error( ERR_DROP, "ImGuiPayload dataSize != sizeof( module_t )!" );
				}
				swap = (module_t *)const_cast<ImGuiPayload *>( pData )->Data;

				if ( !IsDependentOn( swap, &mods->modList[i] )
					&& !IsDependentOn( &mods->modList[i], swap ) )
				{
					eastl::swap( *swap, mods->modList[i] );
				}
			}
			ImGui::EndDragDropTarget();
		}

		ImGui::TextUnformatted( mods->modList[i].info->m_szName );
		ImGui::TableNextColumn();
		ImGui::Text( "v%i.%i.%i", mods->modList[i].info->m_nModVersionMajor, mods->modList[i].info->m_nModVersionUpdate,
            mods->modList[i].info->m_nModVersionPatch );
        ImGui::TableNextColumn();
        ImGui::Text( "v%hu.%hu.%u", mods->modList[i].info->m_GameVersion.m_nVersionMajor,
            mods->modList[i].info->m_GameVersion.m_nVersionUpdate, mods->modList[i].info->m_GameVersion.m_nVersionPatch );
		
		if ( dimColor ) {
			ImGui::PopStyleColor( 3 );
		}

		ImGui::TableNextColumn();
		
		if ( ImGui::ArrowButton( "##ModsMenuConfigUp", ImGuiDir_Up ) ) {
			Snd_PlaySfx( ui->sfx_select );
			if ( i == 0 ) {
				swap = &mods->modList[ mods->numMods - 1 ];
			} else {
				swap = &mods->modList[ i - 1 ];
			}
		}
		ImGui::SameLine();
		if ( ImGui::ArrowButton( "##ModsMenuConfigDown", ImGuiDir_Down ) ) {
			Snd_PlaySfx( ui->sfx_select );
			if ( i == mods->numMods - 1 ) {
				swap = &mods->modList[ 0 ];
			} else {
				swap = &mods->modList[ i + 1 ];
			}
		}
		if ( swap ) {
			if ( !IsDependentOn( swap, &mods->modList[i] )
				&& !IsDependentOn( &mods->modList[i], swap ) )
			{
				eastl::swap( *swap, mods->modList[i] );
			}
		}
		
		if ( i < mods->numMods - 1 ) {
            ImGui::TableNextRow();
        }

		ImGui::PopID();
   	}
   	
    ImGui::EndTable();

	ImGui::End();

	style->ItemSpacing.y = itemSpacing;
}

static void ModsMenu_Load( void )
{
    uint32_t i, j;
	module_t *m;

    Con_Printf( "Caching module info data...\n" );

    mods->modList = g_pModuleLib->m_pModList;
	mods->numMods = g_pModuleLib->GetModCount();
}

void ModsMenu_Cache( void )
{
	uint64_t size, i;
	
    Con_Printf( "Setting menu to mods menu...\n" );
	
	size = 0;
	size += PAD( sizeof( *mods ), sizeof( uintptr_t ) );
	size += PAD( sizeof( *mods->modList ) * g_pModuleLib->GetModCount(), sizeof( uintptr_t ) );
    mods = (modmenu_t *)Hunk_Alloc( size, h_high );
    memset( mods, 0, size );
    if ( g_pModuleLib->GetModCount() ) {
	    mods->modList = (module_t *)( mods + 1 );
	}

	mods->menu.draw = ModsMenu_Draw;
	mods->menu.fullscreen = qtrue;
	mods->menu.flags = MENU_DEFAULT_FLAGS | ImGuiWindowFlags_HorizontalScrollbar;
	mods->menu.width = ui->gpuConfig.vidWidth;
	mods->menu.height = 680 * ui->scale;
	mods->menu.name = strManager->ValueForKey( "MOD_MENU_TITLE" )->value;
	mods->menu.track = Snd_RegisterTrack( "event:/music/campfire" );

    ModsMenu_Load();

    mods->backgroundShader = re.RegisterShader( "menu/tales_around_the_campfire" );

	ui->menubackShader = mods->backgroundShader;

    Cmd_AddCommand( "ui.clear_load_list", ModsMenu_ClearLoadList_f );

//    ModsMenu_SaveModList();
}

void UI_ModsMenu( void )
{
	Key_SetCatcher( KEYCATCH_UI );
	UI_PushMenu( &mods->menu );
}