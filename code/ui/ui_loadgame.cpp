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

    const stringHash_t *title;
} loadGameMenu_t;

static loadGameMenu_t loadGame;

static void LoadGameMenu_Draw( void )
{
    extern ImFont *PressStart2P;
    ImVec2 mousePos;
    float font_scale;
    uint64_t i;
    const int treeNodeFlags = ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_CollapsingHeader
                            | ImGuiTreeNodeFlags_Framed;

    ImGui::Begin( loadGame.menu.name, NULL, loadGame.menu.flags );
    ImGui::SetWindowSize( ImVec2( loadGame.menu.width, loadGame.menu.height ) );
    ImGui::SetWindowPos( ImVec2( loadGame.menu.x, loadGame.menu.y ) );

    UI_EscapeMenuToggle();
    if ( UI_MenuTitle( loadGame.title->value, 2.25f ) ) {
        Snd_PlaySfx( ui->sfx_back );
        UI_PopMenu();

        ImGui::End();
        return;
    }
    mousePos = ImGui::GetCursorScreenPos();
    font_scale = ImGui::GetFont()->Scale;

    const ImVec2& windowSize = ImGui::GetWindowSize();
    
    ImGui::SetWindowSize( ImVec2( ui->gpuConfig.vidWidth * 0.75f, windowSize.y ) );
    
    ImGui::SetCursorScreenPos( ImVec2( mousePos.x, mousePos.y + 10 ) );
    if ( loadGame.numSaves ) {
        {
            ImGui::Begin( "##ModList", NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove
                | ImGuiWindowFlags_NoMouseInputs | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse );
            ImGui::SetWindowPos( ImVec2( ui->gpuConfig.vidWidth * 0.75f, 64 * ui->scale ) );
            ImGui::SetWindowSize( ImVec2( ui->gpuConfig.vidWidth * 0.25f, ui->gpuConfig.vidHeight - 10 ) );
            ImGui::SeparatorText( "Loaded Modules" );
            for ( uint64_t m = 0; m < loadGame.saveList[loadGame.currentSave].gd.numMods; m++ ) {
                if ( !loadGame.saveList[loadGame.currentSave].modsLoaded[m] ) {
                    ImGui::PushStyleColor( ImGuiCol_Text, colorRed );
                    ImGui::PushStyleColor( ImGuiCol_TextDisabled, colorRed );
                    ImGui::PushStyleColor( ImGuiCol_TextSelectedBg, colorRed );
                } else {
                    ImGui::PushStyleColor( ImGuiCol_Text, colorGreen );
                    ImGui::PushStyleColor( ImGuiCol_TextDisabled, colorGreen );
                    ImGui::PushStyleColor( ImGuiCol_TextSelectedBg, colorGreen );
                }
                ImGui::Text( "%s v%i.%i.%i", loadGame.saveList[loadGame.currentSave].gd.modList[m].name,
                    loadGame.saveList[loadGame.currentSave].gd.modList[m].nVersionMajor,
                    loadGame.saveList[loadGame.currentSave].gd.modList[m].nVersionUpdate,
                    loadGame.saveList[loadGame.currentSave].gd.modList[m].nVersionPatch );
                ImGui::PopStyleColor( 3 );
                if ( ImGui::IsItemHovered() && !loadGame.saveList[loadGame.currentSave].modsLoaded[m] ) {
                    ImGui::SetTooltip(
                                "This module either hasn't been loaded or failed to load, check"
                                "the console/logfile for more detailed information" );
                }
            }
            ImGui::End();
        }
        FontCache()->SetActiveFont( PressStart2P );
        for ( i = 0; i < loadGame.numSaves; i++ ) {
            if ( ImGui::TreeNodeEx( (void *)(uintptr_t)loadGame.saveList[i].name, treeNodeFlags, loadGame.saveList[i].name ) ) {
                if ( ImGui::IsMouseClicked( ImGuiMouseButton_Left ) ) {
                    Snd_PlaySfx( ui->sfx_select );
                }
                loadGame.currentSave = i;

                if ( ImGui::IsItemClicked( ImGuiMouseButton_Left ) && ImGui::IsMouseDoubleClicked( ImGuiMouseButton_Left ) ) {
                    Snd_PlaySfx( ui->sfx_select );
                    Cvar_Set( "sgame_SaveName", loadGame.saveList[i].name );
                    gi.state = GS_LEVEL;
                    g_pModuleLib->ModuleCall( sgvm, ModuleOnLoadGame, 0 );
                }

                ImGui::SetWindowFontScale( font_scale * 0.75f );
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
                    ImGui::TextUnformatted( loadGame.saveList[i].creationTime ); // creation time
                    
                    ImGui::TableNextColumn();
                    ImGui::TextUnformatted( loadGame.saveList[i].modificationTime ); // last used time
                    
                    ImGui::TableNextColumn();
                    ImGui::TextUnformatted( difficultyTable[ loadGame.saveList[i].gd.dif ].name );
                }
                ImGui::EndTable();
                ImGui::SetWindowFontScale( font_scale );
                ImGui::TreePop();
            }
        }
    }
    else {
        ImGui::TextUnformatted( "No Saves" );
    }
    ImGui::End();
}

void LoadGameMenu_Cache( void )
{
    saveinfo_t *info;
    const char **fileList;
    uint64_t i, j;
    const char *path;
    const stringHash_t *hardest;
    struct tm *fileTime;

    loadGame.menu.draw = LoadGameMenu_Draw;
    loadGame.menu.flags = MENU_DEFAULT_FLAGS;
    loadGame.menu.name = "LoadGameMenu##MainMenuLoadGameConfig";

    loadGame.title = strManager->ValueForKey( "SP_LOADGAME_TITLE" );

    //
    // init savefiles
    //

    loadGame.numSaves = 0;
    fileList = g_pArchiveHandler->GetSaveFiles( &loadGame.numSaves );

    if ( loadGame.numSaves ) {
        Cvar_Set( "sg_numSaves", va( "%li", (int64_t)loadGame.numSaves ) );

        loadGame.saveList = (saveinfo_t *)Z_Malloc( sizeof( saveinfo_t ) * loadGame.numSaves, TAG_SAVEFILE );
        info = loadGame.saveList;

        for ( i = 0; i < loadGame.numSaves; i++, info++ ) {
            N_strncpyz( info->name, fileList[i], sizeof( info->name ) );

            info->index = i;
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

            info->modsLoaded = (qboolean *)Z_Malloc( sizeof( *info->modsLoaded ) * info->gd.numMods, TAG_SAVEFILE );
            for ( uint64_t a = 0; a < info->gd.numMods; a++ ) {
                info->modsLoaded[a] = g_pModuleLib->GetModule( info->gd.modList[a].name ) != NULL;
            }

            COM_StripExtension( fileList[i], info->name, sizeof( info->name ) );
            if ( info->name[ strlen( info->name ) - 1 ] == '.' ) {
                info->name[ strlen( info->name ) - 1 ] = 0;
            }
        }
    }
}

void UI_LoadGameMenu( void )
{
    UI_PushMenu( &loadGame.menu );
}
