#include "ui_lib.h"

typedef struct module_s {
    qboolean active;        // is it active?
    qboolean valid;         // did it fail to load?
    CModuleInfo *info;      // the stuff the module_lib deals with
    char *dependencies;     // a csv style list of dependencies
    uint64_t numDependencies;
    uint64_t bootIndex;
    qboolean isRequired;

    bool operator<( const module_s& other ) const;
    bool operator>( const module_s& other ) const;
    bool operator==( const module_s& other ) const;
    bool operator!=( const module_s& other ) const;
} module_t;

typedef struct {
    uint64_t numMods;
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
    uint64_t i;
    module_t *m;
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
        N_Error( ERR_DROP, "ModsMenu_LoadModList: bad load list, more mods in list than in memory" );
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
}

void ModsMenu_Draw( void )
{
    uint64_t i;
    const int windowFlags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse |
                            ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBackground;

    Snd_SetLoopingTrack( mods->ambience );

    ui->EscapeMenuToggle( STATE_MAIN );
    if ( ui->Menu_Title( mods->titleString->value, 1.75f ) ) {
        ui->SetState( STATE_MAIN );
        ModsMenu_SaveModList();
        return;
    }

    ImGui::PushStyleVar( ImGuiStyleVar_FramePadding, ImVec2( 2.0f, 2.0f ) );
    ImGui::BeginTable( "##ApplyMods", 4 );

    ImGui::TableNextColumn();
    ImGui::TextUnformatted( "Name" );
    ImGui::TableNextColumn();
    ImGui::TextUnformatted( "Game Version" );
    ImGui::TableNextColumn();
    ImGui::TextUnformatted( "Mod Version" );
    ImGui::TableNextColumn();
    ImGui::TextUnformatted( "Active" );

    ImGui::TableNextRow();

    for ( i = 0; i < mods->numMods; i++ ) {
        if ( !mods->modList[i].valid || mods->modList[i].isRequired ) {
            ImGui::PushStyleColor( ImGuiCol_FrameBg, ImVec4( 0.75f, 0.75f, 0.75f, 1.0f ) );
            ImGui::PushStyleColor( ImGuiCol_FrameBgHovered, ImVec4( 0.75f, 0.75f, 0.75f, 1.0f ) );
            ImGui::PushStyleColor( ImGuiCol_FrameBgActive, ImVec4( 0.75f, 0.75f, 0.75f, 1.0f ) );
        }
        
        ImGui::TableNextColumn();
        ImGui::TextUnformatted( mods->modList[i].info->m_szName );
        if ( ImGui::IsItemHovered( ImGuiHoveredFlags_AllowWhenDisabled ) ) {
            if ( !mods->modList[i].dependencies ) {
                ImGui::SetItemTooltip( "No Dependencies" );
            } else {
                ImGui::SetItemTooltip( "%s", mods->modList[i].dependencies );
            }
        }

        ImGui::TableNextColumn();
        ImGui::Text( "v%hu.%hu.%u", mods->modList[i].info->m_GameVersion.m_nVersionMajor,
            mods->modList[i].info->m_GameVersion.m_nVersionUpdate, mods->modList[i].info->m_GameVersion.m_nVersionPatch );

        ImGui::TableNextColumn();
        ImGui::Text( "v%i.%i.%i", mods->modList[i].info->m_nModVersionMajor, mods->modList[i].info->m_nModVersionUpdate,
            mods->modList[i].info->m_nModVersionPatch );

        ImGui::TableNextColumn();
        if ( ImGui::RadioButton( va( "##Active%s", mods->modList[i].info->m_szName ), mods->modList[i].active ) ) {
            if ( !mods->modList[i].isRequired ) { // make it const
                mods->modList[i].active = !mods->modList[i].active;
            }
        }
        if ( ImGui::IsItemHovered( ImGuiHoveredFlags_AllowWhenDisabled ) ) {
            if ( !mods->modList[i].valid ) {
                ImGui::SetItemTooltip( "Mod \"%s\" failed to load, check console log for details.", mods->modList[i].info->m_szName );
            }
        }

        if ( i < mods->numMods - 1 ) {
            ImGui::TableNextRow();
        }

        if ( !mods->modList[i].valid ) {
            ImGui::PopStyleColor( 3 );
        }
    }
    ImGui::EndTable();
    ImGui::PopStyleVar();
}

static uint64_t GetModLoadIndex( const char *pName ) {
    uint64_t i;

    for ( i = 0; i < mods->numMods; i++ ) {
        if ( !N_strcmp( pName, mods->modList[i].info->m_szName ) ) {
            break;
        }
    }

    return i;
}

inline bool module_s::operator<( const module_s& other ) const {
    if ( info->m_Dependencies.size() ) {
        for ( const auto& it : info->m_Dependencies ) {
            if ( !N_strcmp( other.info->m_szName, it.c_str() ) ) {
                if ( other.bootIndex > bootIndex ) {
                    // we need to load the dependency before this one
                    return true;
                } else {
                    return false;
                }
            }
        }
    }
    // if we've got no dependencies or it's not a dependency, just sort alphabetically
    return N_stricmp( info->m_szName, other.info->m_szName ) == -1 ? true : false;
}
inline bool module_s::operator>( const module_s& other ) const {
    if ( info->m_Dependencies.size() ) {
        for ( const auto& it : info->m_Dependencies ) {
            if ( !N_strcmp( other.info->m_szName, it.c_str() ) ) {
                if ( other.bootIndex > bootIndex ) {
                    // we need to load it after
                    return false;
                } else {
                    return true;
                }
            }
        }
    }
    // if we've got no dependencies or it's not a dependency, just sort alphabetically
    return N_stricmp( other.info->m_szName, info->m_szName ) == 1 ? true : false;
}
inline bool module_s::operator==( const module_s& other ) const {
    return N_stricmp( info->m_szName, other.info->m_szName ) == 0;
}
inline bool module_s::operator!=( const module_s& other ) const {
    return N_stricmp( info->m_szName, other.info->m_szName ) != 0;
}

/*
* ModsMenu_Sort: sorts each mod alphabetically, then in load order by dependencies
*/
static void ModsMenu_Sort( void ) {
    uint64_t i, j;
    uint64_t index;
    const CModuleInfo *mod;

    eastl::sort( mods->modList, mods->modList + mods->numMods );
/*
    return;

    for ( i = 0; i < mods->numMods; i++ ) {
        if ( mods->modList[i].numDependencies ) {
            //
            // check if we're loading any dependencies before it.
            // If so, reorder it to load the dependencies first to
            // avoid any missing resources issues/errors
            //
            for ( j = 0; j < mod->m_Dependencies.size(); j++ ) {
                index = GetModLoadIndex( mod->m_Dependencies[j].c_str() );
                
                // it's loaded after, force a reorder
                if ( index > i ) {

                }
            }
        }
    }
*/
}

static void ModsMenu_Load( void )
{
    uint64_t i, j, size;
    module_t *m;

    Con_Printf( "Caching module info data...\n" );

    const UtlVector<CModuleInfo *>& loadList = g_pModuleLib->GetLoadList();

    mods->numMods = loadList.size();
    if ( !mods->numMods ) {
        Con_Printf( COLOR_YELLOW "WARNING: no mods to load!\n" );
        return;
    }

    mods->modList = (module_t *)Hunk_Alloc( sizeof( *mods->modList ) * mods->numMods, h_high );

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

        size = 0;
        for ( j = 0; j < loadList[i]->m_Dependencies.size(); j++ ) {
            size += PAD( loadList[i]->m_Dependencies[j].size(), sizeof( uintptr_t ) );
        }

        if ( size ) {
            m->dependencies = (char *)Hunk_Alloc( size, h_high );
            m->numDependencies = loadList[i]->m_Dependencies.size();
            for ( j = 0; j < loadList[i]->m_Dependencies.size(); j++ ) {
                if ( j < loadList[i]->m_Dependencies.size() - 1 ) {
                    N_strcat( m->dependencies, size, va( "%s, ", loadList[i]->m_Dependencies[j].c_str() ) );
                } else {
                    N_strcat( m->dependencies, size, loadList[i]->m_Dependencies[j].c_str() );
                }
            }
            Con_Printf( "Module \"%s\" dependencies: %s\n", m->info->m_szName, m->dependencies );
        } else {
            Con_Printf( "Module \"%s\" has no dependencies.\n", m->info->m_szName );
        }
        
        m++;
    }

    ModsMenu_LoadModList();
    ModsMenu_Sort();

    // we may have some outdated info
    for ( i = 0; i < mods->numMods; i++ ) {
        if ( loadList[i]->m_pHandle->IsValid() && !mods->modList[i].valid ) {
            mods->modList[i].valid = qtrue;
        }
    }

    Con_Printf( "...Got %lu modules\n", mods->numMods );
}

void ModsMenu_Cache( void )
{
    Con_Printf( "Setting menu to mods menu...\n" );

    mods = (modmenu_t *)Hunk_Alloc( sizeof( *mods ), h_high );
    memset( mods, 0, sizeof( *mods ) );

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
    ModsMenu_Cache();
}
