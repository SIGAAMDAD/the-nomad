#include "ui_lib.h"

typedef struct module_s {
    const char *name;       // name of the mod
    uint64_t gameVersion;   // game version mod is compiled with
    uint64_t modVersion;    // mod's current version
    char *dependencies;     // does this mod need other mods to work?
    qboolean active;        // is it active?
    qboolean valid;         // did it fail to load?
    vm_t *vm;
} module_t;

typedef struct
{
    CUIMenu menu;

    uint64_t numMods;
    char **modNames;
    module_t *modList;

    nhandle_t backgroundShader;
    nhandle_t ambience;
    
    const stringHash_t *titleString;
    const stringHash_t *loadString;
    const stringHash_t *backString;
} modmenu_t;

static modmenu_t mods;

/*
* ModsMenu_LoadMod: maybe this shouldn't really be here...
*/
static void ModsMenu_LoadMod( module_t *mod )
{
}

static void ModsMenu_Draw( void )
{
    uint64_t i;
    const int windowFlags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar;

    Snd_SetLoopingTrack( mods.ambience );

    ImGui::Begin( "ModsMenu", NULL, windowFlags );

    ui->Menu_Title( "Tales Around The Campfire" );

    ImGui::PushStyleVar( ImGuiStyleVar_FramePadding, ImVec2( 16.0f, 16.0f ) );
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

    for ( i = 0; i < mods.numMods; i++ ) {
        if ( !mods.modList[i].valid ) {
            ImGui::PushStyleColor( ImGuiCol_FrameBg, ImVec4( 0.75f, 0.75f, 0.75f, 1.0f ) );
            ImGui::PushStyleColor( ImGuiCol_FrameBgHovered, ImVec4( 0.75f, 0.75f, 0.75f, 1.0f ) );
            ImGui::PushStyleColor( ImGuiCol_FrameBgActive, ImVec4( 0.75f, 0.75f, 0.75f, 1.0f ) );
        }

        ImGui::TableNextColumn();
        ImGui::TextUnformatted( mods.modNames[i] );

        ImGui::TableNextColumn();
        ImGui::Text( "%lu", mods.modList[i].gameVersion );

        ImGui::TableNextColumn();
        ImGui::Text( "%lu", mods.modList[i].modVersion );

        ImGui::TableNextColumn();
        if ( ImGui::RadioButton( va( "##Active%s", mods.modNames[i] ), mods.modList[i].active ) ) {
            mods.modList[i].active = !mods.modList[i].active;
        }
        if ( ImGui::IsItemHovered( ImGuiHoveredFlags_AllowWhenDisabled ) ) {
            if ( !mods.modList[i].valid ) {
                ImGui::SetItemTooltip( "Mod \"%s\" failed to load, check console log for details.", mods.modNames[i] );
            } else {
                ImGui::SetItemTooltip( "Dependendencies: %s", mods.modList[i].dependencies );
            }
        }

        if ( !mods.modList[i].valid ) {
            ImGui::PopStyleColor( 3 );
        }
    }
    ImGui::EndTable();
    ImGui::PopStyleVar();

    ImGui::End();
}

static void ModsMenu_LoadConfigs( void )
{
    union {
        void *v;
        char *b;
    } f;
    char **fileList;
    char *modPtr;
    const char *tok, *text_p, **text;
    const char *path;
    uint64_t i, size;

    fileList = FS_ListFiles( "modules", "/", &mods.numMods );

    size = sizeof(*fileList) * mods.numMods;
    for ( i = 0; i < mods.numMods; i++ ) {
        size += PAD( strlen( fileList[i] ) + 1, sizeof(uintptr_t) );
    }

    mods.modNames = (char **)Hunk_Alloc( size, h_high );
    mods.modList = (module_t *)Hunk_Alloc( sizeof(*mods.modList) * mods.numMods, h_high );

    modPtr = (char *)mods.modNames;
    for ( i = 0; i < mods.numMods; i++ ) {
        mods.modNames[i] = modPtr;
        strcpy( mods.modNames[i], COM_SkipPath( fileList[i] ) );
        Con_Printf( "Loading module \"%s\":\n", mods.modNames[i] );

        modPtr += PAD( strlen( fileList[i] ) + 1, sizeof(uintptr_t) );

        mods.modList[i].name = mods.modNames[i];
        path = FS_BuildOSPath( FS_GetBasePath(), fileList[i], "module.cfg" );
        Con_Printf( "- Loading module config \"%s\"...\n", path );

        FS_LoadFile( FS_BuildOSPath( FS_GetBasePath(), mods.modNames[i], "module.cfg" ), &f.v );

        if ( !f.v ) {
            Con_Printf( COLOR_RED "WARNING: failed to load mod configuration for \"%s\", ignoring mod\n", mods.modNames[i] );
            mods.modList[i].valid = qfalse;
            continue;
        }

        text_p = f.b;
        text = &text_p;
        
        while ( 1 ) {
            tok = COM_ParseExt( text, qfalse );

            if ( !tok[0] ) {
                continue;
            } else if ( !N_stricmp( tok, "endconfig" ) ) {
                break;
            }

            if ( !N_stricmp( tok, "mob_file" ) ) {
                
            } else {
                COM_ParseWarning( "unrecognized token \"%s\" in mod config \"%s\"", tok, path );
            }
        }

        FS_FreeFile( f.v );
    }

    Con_Printf( "Found %lu modules.\n", mods.numMods );

    FS_FreeFileList( fileList );
}

void ModsMenu_Cache( void )
{
    Con_Printf( "Setting menu to mods menu...\n" );

    memset( &mods, 0, sizeof(mods) );

    ModsMenu_LoadConfigs();

    mods.ambience = Snd_RegisterTrack( "music/tales_around_the_campfire.ogg" );
    mods.backgroundShader = re.RegisterShader( "menu/tales_around_the_campfire" );

    mods.titleString = strManager->ValueForKey( "MOD_MENU_TITLE" );
    mods.loadString = strManager->ValueForKey( "MOD_MENU_LOAD" );
    mods.backString = strManager->ValueForKey( "MOD_MENU_BACK" );
}

void UI_ModsMenu( void )
{
    ModsMenu_Cache();
}
