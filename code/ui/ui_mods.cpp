#include "ui_lib.h"

typedef struct module_s {
    qboolean active;        // is it active?
    qboolean valid;         // did it fail to load?
    CModuleInfo *info;      // the stuff the module_lib deals with
    uint32_t numDependencies;
    uint32_t bootIndex;
    qboolean isRequired;
    qboolean allDepsActive;

	module_s& operator=( module_s& other );
	module_s& operator=( const module_s& other );
	bool operator==( const UtlString& other ) const;
	bool operator!=( const UtlString& other ) const;
    bool operator<( const module_s& other ) const;
    bool operator>( const module_s& other ) const;
    bool operator==( const module_s& other ) const;
    bool operator!=( const module_s& other ) const;
} module_t;

typedef struct {
	module_t *modList;
    uint32_t numMods;

    nhandle_t backgroundShader;
    nhandle_t ambience;
    
    uint32_t selectedMod;
    
    const stringHash_t *titleString;
    const stringHash_t *loadString;
    const stringHash_t *backString;
} modmenu_t;

static modmenu_t *mods;

//
// required modules, those made with love, by Your Resident Fiend
//
static const char *requiredModules[] = {
    "nomadmain"
};

static uint32_t GetModLoadIndex( const char *pName ) {
    uint32_t i;

    for ( i = 0; i < mods->numMods; i++ ) {
        if ( !N_strcmp( pName, mods->modList[i].info->m_szName ) ) {
            break;
        }
    }

    return i;
}

static bool IsRequiredMod( const char *pName ) {
    for ( const auto& it : requiredModules ) {
        if ( !N_strcmp( pName, it ) ) {
            return true;
        }
    }
    return false;
}

static void ModsMenu_ClearLoadList_f( void ) {
    FS_Remove( "_cache/loadlist.json" );
    FS_HomeRemove( "_cache/loadlist.json" );
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
    nlohmann::json json;
    nlohmann::json::array_t loadList;
    fileHandle_t f;

    Con_Printf( "Saving mod list...\n" );
    
    f = FS_FOpenWrite( "_cache/loadlist.json" );
    if ( f == FS_INVALID_HANDLE ) {
    	N_Error( ERR_DROP, "ModsMenu_SaveModList: failed to save _cache/loadlist.json" );
    }
    
    FS_Printf( f, "{\n" );
    FS_Printf( f, "\t\"LoadList\": [\n" );
    for ( i = 0; i < mods->numMods; i++ ) {
    	FS_Printf( f,
    		"\t\t{\n"
    		"\t\t\t\"Name\": \"%s\",\n"
    		"\t\t\t\"Valid\": %i,\n"
    		"\t\t\t\"Active\": %i,\n"
    		"\t\t\t\"Dependencies\": [\n"
    		, mods->modList[i].info->m_szName, mods->modList[i].valid, mods->modList[i].active );
    	
    	for ( j = 0; j < mods->modList[i].numDependencies; j++ ) {
    		FS_Printf( f, "\t\t\t\t\"%s\"", mods->modList[i].info->m_Dependencies[j].c_str() );
    		if ( j != mods->modList[i].numDependencies - 1 ) {
    			FS_Printf( f, "," );
    		}
    		FS_Printf( f, "\n" );
    	}
		FS_Printf( f, "\t\t\t]\n" );
    	
    	if ( i != mods->numMods - 1 ) {
    		FS_Printf( f, "\t\t},\n" );
    	} else {
    		FS_Printf( f, "\t\t}\n" );
    	}
    }
    FS_Printf( f, "\t]\n" );
    FS_Printf( f, "}\n" );
    
    FS_FClose( f );
}

static void ModsMenu_LoadModList( void )
{
    char *b;
    uint64_t nLength;
    uint32_t i;
    nlohmann::json json;

    nLength = FS_LoadFile( "_cache/loadlist.json", (void **)&b );
    if ( !nLength || !b ) {
        return;
    }

    try {
        json = nlohmann::json::parse( b, b + nLength );
    } catch ( const nlohmann::json::exception& e ) {
        N_Error( ERR_DROP, "ModsMenu_LoadModList: invalid loadlist.json (nlohmann::json::exception) ->\n  id: %i\n  message: %s",
            e.id, e.what() );
    }
    FS_FreeFile( b );

    if ( json.at( "LoadList" ).size() != mods->numMods ) {
        Con_Printf( COLOR_YELLOW "WARNING: ModsMenu_LoadModList: bad load list, mods in list different than in memory\n" );
    }

    const nlohmann::json& data = json.at( "LoadList" );
    for ( i = 0; i < mods->numMods; i++ ) {
        for ( const auto& it : data ) {
            if ( !N_strcmp( mods->modList[i].info->m_szName, it.at( "Name" ).get<const nlohmann::json::string_t>().c_str() ) ) {
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
		for ( j = 0; j < mods->modList[i].info->m_Dependencies.size(); j++ ) {
			if ( N_strcmp( mod->info->m_szName, mods->modList[i].info->m_Dependencies[j].c_str() ) == 0 ) {
				module_t *m = eastl::find( mods->modList, mods->modList + mods->numMods, mods->modList[i].info->m_Dependencies[j] );
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
		ui->PlaySelected();
		ModToggleActive( mod, !dimColor );
	}
	if ( active ) {
		ImGui::PopStyleColor( 2 );
	}
	
	ImGui::TableNextColumn();
	if ( mod->info->m_Dependencies.size() ) {
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
			for ( const auto& it : mod->info->m_Dependencies ) {
				ImGui::TextUnformatted( it.c_str() );
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
			ImGui::TextUnformatted( "NOTE: GDR Software is not responsible for any unstable experiences when using this mod" );
			ImGui::EndTooltip();
		}
		ImGui::SameLine();
	}
}

static qboolean IsLoadedAfter( const module_t *mod, const module_t *dep ) {
	return ( dep > mod );
}

static qboolean IsLoadedBefore( const module_t *mod, const module_t *dep ) {
	return ( dep > mod );
}

static qboolean IsDependentOn( const module_t *mod, const module_t *dep ) {
	if ( !mod->info ) {
		Con_Printf( COLOR_YELLOW "WARNING: bad mod info\n" );
		return qfalse;
	}
	for ( uint32_t i = 0; i < mod->info->m_Dependencies.size(); i++ ) {
		if ( !N_stricmp( mod->info->m_Dependencies[i].c_str(), dep->info->m_szName ) ) {
			return qtrue;
		}
	}
	return qfalse;
}

void ModsMenu_Draw( void )
{
    uint32_t i, j;
    const int windowFlags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse |
                            ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBackground;
    ImGuiStyle *style;
    char loadId[MAX_STRING_CHARS];
    qboolean dimColor;
    const float fontScale = ImGui::GetFont()->Scale;
    extern ImFont *RobotoMono;
    
	ui->menu_background = mods->backgroundShader;
    Snd_SetLoopingTrack( mods->ambience );
    
    ui->EscapeMenuToggle( STATE_MAIN );
    if ( ui->Menu_Title( mods->titleString->value, 1.75f ) ) {
        ui->SetState( STATE_MAIN );
        ModsMenu_SaveModList();
        return;
    }
    
    style = eastl::addressof( ImGui::GetStyle() );
    style->ItemSpacing.y = 50.0f;
   	
   	ImGui::BeginTable( "##ModLoadList", 5 );
   	
   	ImGui::SetWindowFontScale( ( fontScale * 1.5f ) * ui->scale );
   	ImGui::TableNextColumn();
   	ImGui::TextUnformatted( "Active" );
   	ImGui::TableNextColumn();
   	ImGui::TextUnformatted( "Name" );
   	ImGui::TableNextColumn();
   	ImGui::TextUnformatted( "Mod Version" );
   	ImGui::TableNextColumn();
   	ImGui::TextUnformatted( "Game Version" );
	ImGui::TableNextColumn();
	ImGui::SetWindowFontScale( ( fontScale * 1.75f ) * ui->scale );
	
	if ( RobotoMono ) {
		FontCache()->SetActiveFont( RobotoMono );
	}

	ImGui::TableNextRow();
   	
   	for ( i = 0; i < mods->numMods; i++ ) {
   		Com_snprintf( loadId, sizeof( loadId ) - 1, "%s##ModLoad%i", mods->modList[i].info->m_szName, i );
   		ImGui::PushID( i );

		if ( mods->modList[i].isRequired || !mods->modList[i].allDepsActive ) {
	   		dimColor = qtrue;
	   	} else {
	   		dimColor = qfalse;
	   	}
   		
   		if ( Key_IsDown( KEY_UP ) ) {
   			ui->PlaySelected();
   			if ( mods->selectedMod == 0 ) {
   				mods->selectedMod = mods->numMods - 1;
   			} else {
	   			mods->selectedMod--;
	   		}
		}
   		if ( Key_IsDown( KEY_DOWN ) ) {
   			ui->PlaySelected();
   			mods->selectedMod++;
   			if ( mods->selectedMod >= mods->numMods ) {
   				mods->selectedMod = 0;
   			}
   		}
   		if ( Key_IsDown( KEY_ENTER ) ) {
   			ui->PlaySelected();
   			ModToggleActive( &mods->modList[ mods->selectedMod ], !dimColor );
   		}
		
		ModsMenu_DrawListing( &mods->modList[i], dimColor );
		
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
		
		module_t *swap = NULL;
		if ( ImGui::ArrowButton( "##ModsMenuConfigUp", ImGuiDir_Up ) ) {
			ui->PlaySelected();
			if ( i == 0 ) {
				swap = &mods->modList[ mods->numMods - 1 ];
			} else {
				swap = &mods->modList[ i - 1 ];
			}
		}
		ImGui::SameLine();
		if ( ImGui::ArrowButton( "##ModsMenuConfigDown", ImGuiDir_Down ) ) {
			ui->PlaySelected();
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
}

inline module_s& module_s::operator=( module_s& other ) {
	info = other.info;
	active = other.active;
	valid = other.valid;
	numDependencies = other.numDependencies;
	bootIndex = other.bootIndex;
	isRequired = other.isRequired;
	allDepsActive = other.allDepsActive;
	return *this;
}

inline module_s& module_s::operator=( const module_s& other ) {
	info = const_cast<CModuleInfo *>( other.info );
	active = other.active;
	valid = other.valid;
	numDependencies = other.numDependencies;
	bootIndex = other.bootIndex;
	isRequired = other.isRequired;
	allDepsActive = other.allDepsActive;
	return *this;
}

inline bool module_s::operator<( const module_s& other ) const {
	if ( IsDependentOn( eastl::addressof( other ), this ) ) {
		return true;
	}
    return N_strcmp( info->m_szName, other.info->m_szName ) == 1 ? true : false;
}
inline bool module_s::operator>( const module_s& other ) const {
	if ( IsDependentOn( this, eastl::addressof( other ) ) ) {
		return false;
	}
    return N_strcmp( other.info->m_szName, info->m_szName ) == -1 ? true : false;
}
inline bool module_s::operator==( const module_s& other ) const {
    return N_strcmp( info->m_szName, other.info->m_szName ) == 0;
}
inline bool module_s::operator!=( const module_s& other ) const {
    return N_strcmp( info->m_szName, other.info->m_szName ) != 0;
}

inline bool module_s::operator==( const UtlString& other ) const {
	return N_strcmp( info->m_szName, other.c_str() ) == 0;
}

inline bool module_s::operator!=( const UtlString& other ) const {
	return N_strcmp( info->m_szName, other.c_str() ) != 0;
}

static void ModsMenu_Load( void )
{
    uint32_t i, j;
	module_t *m;

    Con_Printf( "Caching module info data...\n" );

    const UtlVector<CModuleInfo *>& loadList = g_pModuleLib->GetLoadList();

    mods->numMods = loadList.size();
    if ( !mods->numMods ) {
        Con_Printf( COLOR_YELLOW "WARNING: no mods to load!\n" );
        return;
    }

    m = mods->modList;
    for ( i = 0; i < mods->numMods; i++ ) {
        m->info = loadList[i];
        m->valid = loadList[i]->m_pHandle->IsValid();
        m->active = qtrue;
        m->bootIndex = i;
        m->isRequired = IsRequiredMod( m->info->m_szName );
		m->numDependencies = loadList[i]->m_Dependencies.size();

        // check if we have any dependencies that either don't exist or aren't properly loaded
        for ( const auto& it : loadList[i]->m_Dependencies ) {
            const CModuleInfo *dep = g_pModuleLib->GetModule( it.c_str() );

            if ( !dep || !dep->m_pHandle->IsValid() ) {
                m->valid = m->active = qfalse;
            }
        }
        
        if ( loadList[i]->m_Dependencies.size() ) {
        	Con_Printf( "Module \"%s\" has dependencies: ", m->info->m_szName );
        	for ( j = 0; j < loadList[i]->m_Dependencies.size(); j++ ) {
        		if ( j < loadList[i]->m_Dependencies.size() - 1 ) {
        			Con_Printf( ", " );
        		}
        		Con_Printf( "%s", loadList[i]->m_Dependencies[j].c_str() );
        	}
        	Con_Printf( "\n" );
        }
        
        m++;
    }

    ModsMenu_LoadModList();
    eastl::sort( mods->modList, mods->modList + mods->numMods );

	// check for missing dependencies
    for ( i = 0; i < mods->numMods; i++ ) {
		bool done = false;
		mods->modList[i].allDepsActive = qtrue;
    	for ( const auto& it : mods->modList[i].info->m_Dependencies ) {
			for ( j = 0; j < mods->numMods; j++ ) {
				if ( N_strcmp( mods->modList[j].info->m_szName, it.c_str() ) == 0 ) {
					if ( !mods->modList[j].info->m_pHandle->IsValid() ) {
						mods->modList[i].allDepsActive = qfalse;
						done = true;
						break;
					}
				}
			}
			if ( done ) {
				break;
			}
		}
    	mods->modList[i].active = mods->modList[i].valid = mods->modList[i].allDepsActive;
    }

	// check for required modules
	for ( i = 0; i < mods->numMods; i++ ) {
		if ( !N_stricmp( mods->modList[i].info->m_szName, "nomadmain" ) ) {
			mods->modList[i].isRequired = qtrue;
		}
	}

    Con_Printf( "...Got %u modules\n", mods->numMods );
}

void ModsMenu_Cache( void )
{
	uint64_t size;
	
    Con_Printf( "Setting menu to mods menu...\n" );
	
	size = 0;
	size += PAD( sizeof( *mods ), sizeof( uintptr_t ) );
	size += PAD( sizeof( *mods->modList ) * g_pModuleLib->GetLoadList().size(), sizeof( uintptr_t ) );
    mods = (modmenu_t *)Hunk_Alloc( size, h_high );
    memset( mods, 0, size );
    if ( g_pModuleLib->GetLoadList().size() ) {
	    mods->modList = (module_t *)( mods + 1 );
	}

    ModsMenu_Load();
	
    mods->ambience = Snd_RegisterTrack( "music/tales_around_the_campfire.ogg" );
    mods->backgroundShader = re.RegisterShader( "menu/tales_around_the_campfire" );

    mods->titleString = strManager->ValueForKey( "MOD_MENU_TITLE" );
    mods->loadString = strManager->ValueForKey( "MOD_MENU_LOAD" );
    mods->backString = strManager->ValueForKey( "MOD_MENU_BACK" );

    Cmd_AddCommand( "ui.clear_load_list", ModsMenu_ClearLoadList_f );

    ModsMenu_SaveModList();
}

void UI_ModsMenu( void )
{
	Key_SetCatcher( KEYCATCH_UI );
    ModsMenu_Cache();
}