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
    uint32_t numMods;
    module_t *modList;

    nhandle_t backgroundShader;
    nhandle_t ambience;
    
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
    uint64_t i;

    for ( i = 0; i < mods->numMods; i++ ) {
        if ( !N_strcmp( pName, mods->modList[i].info->m_szName ) && mods->modList[i].active ) {
            return qtrue;
        }
    }
    return qfalse;
}

void ModsMenu_SaveModList( void )
{
    uint64_t i;
    nlohmann::json json;
    nlohmann::json::array_t loadList;
    fileHandle_t f;

    Con_Printf( "Saving mod list...\n" );

    loadList.resize( mods->numMods );
    for ( i = 0; i < mods->numMods; i++ ) {
        loadList[i]["Name"] = mods->modList[i].info->m_szName;
        loadList[i]["Valid"] = mods->modList[i].valid;
        loadList[i]["Active"] = mods->modList[i].active;
    }
    json["LoadList"] = eastl::move( loadList );

    f = FS_FOpenWrite( "_cache/loadlist.json" );
    if ( f == FS_INVALID_HANDLE ) {
        N_Error( ERR_DROP, "ModsMenu_SaveModList: failed to save _cache/loadlist.json" );
    }
    const nlohmann::json::string_t&& data = json.dump( 1, '\t' );
    FS_Printf( f, "%s", data.c_str() );
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

static void ModsMenu_DrawListing( module_t *mod, qboolean dimColor )
{
	uint32_t j;
	const int index = (int)( mods->modList - mod );
	
	ImGui::TableNextColumn();
	if ( dimColor ) {
		// dim it a little bit
		ImGui::PushStyleColor( ImGuiCol_FrameBg, ImVec4( 0.75f, 0.75f, 0.75f, 1.0f ) );
		ImGui::PushStyleColor( ImGuiCol_FrameBgActive, ImVec4( 0.75f, 0.75f, 0.75f, 1.0f ) );
		ImGui::PushStyleColor( ImGuiCol_FrameBgHovered, ImVec4( 0.75f, 0.75f, 0.75f, 1.0f ) );
	}
	if ( ImGui::RadioButton( mod->active ? va( "##ModLoad%iON", index ) : va( "##ModLoad%iOFF", index ),
		mod->active ) )
	{
		ui->PlaySelected();
		if ( !dimColor ) {
			mod->active = !mod->active;
		}
		
		// disable anything thay may depend on this mod
		for ( j = 0; j < mods->numMods; j++ ) {
			auto it = eastl::find( mod->info->m_Dependencies.begin(),
				mod->info->m_Dependencies.end(), mod->info->m_szName );
			
			if ( it != mod->info->m_Dependencies.end() ) {
				// turn it off, but not permanently
				mods->modList[ GetModLoadIndex( it->c_str() ) ].active = qfalse;
			}
		}
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
		if ( ImGui::IsItemHovered( ImGuiHoveredFlags_AllowWhenDisabled | ImGuiHoveredFlags_DelayShort ) ) {
			ImGui::BeginTooltip();
			ImGui::SeparatorText( "this module depends on" );
			for ( const auto& it : mod->info->m_Dependencies ) {
				ImGui::TextUnformatted( it.c_str() );
			}
			ImGui::EndTooltip();
		}
		ImGui::PopStyleColor( 6 );
		ImGui::SameLine();
	}
}

void ModsMenu_Draw( void )
{
    uint32_t i, j;
    const int windowFlags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse |
                            ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBackground;
    ImGuiStyle *style;
    ImVec2 windowPos, cursorPos, windowSize;
    char loadId[MAX_STRING_CHARS];
    qboolean dimColor;
    
    Snd_SetLoopingTrack( mods->ambience );
    
    ui->EscapeMenuToggle( STATE_MAIN );
    if ( ui->Menu_Title( mods->titleString->value, 1.75f ) ) {
        ui->SetState( STATE_MAIN );
        ModsMenu_SaveModList();
        return;
    }
    
    windowPos = ImGui::GetWindowPos();
    cursorPos = ImGui::GetCursorPos();
    windowSize = ImGui::GetWindowSize();
    style = eastl::addressof( ImGui::GetStyle() );
    style->ItemSpacing.y = 50.0f;
   	
   	ImGui::BeginTable( "##ModLoadList", 5 );
   	
   	ImGui::TableNextColumn();
   	ImGui::TextUnformatted( "Active" );
   	ImGui::TableNextColumn();
   	ImGui::TextUnformatted( "Name" );
   	ImGui::TableNextColumn();
   	ImGui::TextUnformatted( "Mod Version" );
   	ImGui::TableNextColumn();
   	ImGui::TextUnformatted( "Game Version" );
	ImGui::TableNextColumn();

	ImGui::TableNextRow();
   	
   	for ( i = 0; i < mods->numMods; i++ ) {
   		Com_snprintf( loadId, sizeof( loadId ) - 1, "%s##ModLoad%i", mods->modList[i].info->m_szName, i );
   		ImGui::PushID( i );
   		
   		if ( mods->modList[i].isRequired || !mods->modList[i].allDepsActive ) {
	   		dimColor = qtrue;
	   	} else {
	   		dimColor = qfalse;
	   	}
		
		ModsMenu_DrawListing( &mods->modList[i], dimColor );

		if ( ImGui::BeginDragDropSource() ) {
			module_t *m = (module_t *)ImGui::MemAlloc( sizeof( *m ) );
			*m = mods->modList[i];
			ImGui::SetDragDropPayload( "##ModLoadListDragDropPayload", m, sizeof( *mods->modList ) );
			ModsMenu_DrawListing( &mods->modList[i], dimColor );
			ImGui::EndDragDropSource();
		}
		if ( ImGui::BeginDragDropTarget() ) {
			const ImGuiPayload *data = ImGui::AcceptDragDropPayload( "##ModLoadListDragDropPayload" );
			if ( data ) {
				if ( data->DataSize != sizeof( *mods->modList ) ) {
					Assert( data->DataSize == sizeof( *mods->modList ) );
					N_Error( ERR_FATAL, "ModsMenu_Draw: drag drop payload dataSize mismatch" );
				}
				module_t *m = (module_t *)const_cast<ImGuiPayload *>( data )->Data;
				qboolean reject = qfalse;

				// check if they're trying to load before or after dependencies
				if ( m->numDependencies ) {
					for ( const auto& it : m->info->m_Dependencies ) {
						if ( GetModLoadIndex( it.c_str() ) > i ) {
							reject = qtrue;
						}
					}
					// is this a dependency?
					for ( j = 0; j < mods->numMods; j++ ) {
						const auto it = eastl::find( mods->modList[j].info->m_Dependencies.cbegin(),
							mods->modList[j].info->m_Dependencies.cend(), m->info->m_szName );
						if ( it != mods->modList[j].info->m_Dependencies.cend() ) {
							reject = i < j;
						}
					}
				}

				if ( !reject ) {
					eastl::swap( *m, *( m + 1 ) );

					// shuffle the list
					memmove( mods->modList + ( mods->numMods - 1 ), m + 1,
						sizeof( *mods->modList ) * ( ( mods->modList - m ) + 1 ) );
					for ( j = 0; j < mods->numMods; j++ ) {
						mods->modList[j].bootIndex = j;
					}
					// write it to disk
					ModsMenu_SaveModList();
				}
				ImGui::MemFree( m );
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
			if ( i == 0 ) {
				eastl::swap( mods->modList[ mods->numMods - 1 ], mods->modList[i] );
			} else {
				eastl::swap( mods->modList[ i - 1 ], mods->modList[i] );
			}
		}
		ImGui::SameLine();
		if ( ImGui::ArrowButton( "##ModsMenuConfigDown", ImGuiDir_Down ) ) {
			if ( i == mods->numMods - 1 ) {
				eastl::swap( mods->modList[ i + 1 ], mods->modList[i] );
			} else {
				eastl::swap( *mods->modList, mods->modList[i] );
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
    return N_stricmp( info->m_szName, other.info->m_szName ) == -1 ? true : false;
}
inline bool module_s::operator>( const module_s& other ) const {
    return N_stricmp( other.info->m_szName, info->m_szName ) == 1 ? true : false;
}
inline bool module_s::operator==( const module_s& other ) const {
    return N_stricmp( info->m_szName, other.info->m_szName ) == 0;
}
inline bool module_s::operator!=( const module_s& other ) const {
    return N_stricmp( info->m_szName, other.info->m_szName ) != 0;
}

inline bool module_s::operator==( const UtlString& other ) const {
	return N_stricmp( info->m_szName, other.c_str() ) == 0;
}

inline bool module_s::operator!=( const UtlString& other ) const {
	return N_stricmp( info->m_szName, other.c_str() ) != 0;
}

/*
* ModsMenu_Sort: sorts each mod alphabetically, then in load order by dependencies
*/
static void ModsMenu_Sort( void ) {
    uint32_t i, j;
    const CModuleInfo *mod;
	
	uint32_t index;
	// a bit-packed 64 bit array of indexes
	uint64_t *sortedList;
	
	// sort alphabetically first
	eastl::sort( mods->modList, mods->modList + mods->numMods );
	
	sortedList = (uint64_t *)alloca( sizeof( *sortedList ) * mods->numMods );
	memset( sortedList, 0, sizeof( *sortedList ) * mods->numMods );
	
	// now sort by dependencies
	for ( i = 0; i < mods->numMods; i++ ) {
		if ( mods->modList[i].numDependencies ) {
			mod = mods->modList[i].info;
			//
			// check if we're loading any dependencies before it.
			// If so, reorder it to load the dependencies first to
			// avoid any missing resources issues/errors
			//
			for ( j = 0; j < mod->m_Dependencies.size(); j++ ) {
				index = GetModLoadIndex( mod->m_Dependencies[j].c_str() );
				
				// it's loaded after, force a reorder
				if ( index < i ) {
					//eastl::swap( mods->modList[i], mods->modList[index] );
					( (uint32_t *)&sortedList[i] )[0] = i; // store the index of the module
					( (uint32_t *)&sortedList[i] )[1] = index; // store the index of the dependency
				} else {
					( (uint32_t *)&sortedList[i] )[0] = i;
					( (uint32_t *)&sortedList[i] )[1] = i;
				}
			}
		}
	}
	for ( i = 0; i < mods->numMods; i++ ) {
		// manually swap
		module_t m = mods->modList[ ( (uint32_t *)&sortedList[i] )[1] ];
		mods->modList[ ( (uint32_t *)&sortedList[i] )[1] ] = mods->modList[ ( (uint32_t *)&sortedList[i] )[0] ];
		mods->modList[ ( (uint32_t *)&sortedList[i] )[0] ] = m;
	}
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
        }
        
        m++;
    }

    ModsMenu_LoadModList();
    ModsMenu_Sort();

    // we may have some outdated info
    for ( i = 0; i < mods->numMods; i++ ) {
        if ( mods->modList[i].info->m_pHandle->IsValid() && !mods->modList[i].valid ) {
            mods->modList[i].valid = qtrue;
            
            for ( const auto& it : mods->modList[i].info->m_Dependencies ) {
            	const auto dep = eastl::find( loadList.cbegin(), loadList.cend(), it );
            	
            	if ( dep == loadList.cend() ) {
            		mods->modList[i].allDepsActive = qfalse;
            		break;
            	}
            }
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
