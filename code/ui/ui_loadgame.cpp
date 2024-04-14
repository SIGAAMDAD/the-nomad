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

static loadGameMenu_t *s_loadGame;

static void LoadGameMenu_Draw( void )
{
    extern ImFont *PressStart2P;
    ImVec2 mousePos;
    float font_scale;
    uint64_t i;
    const int treeNodeFlags = ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_CollapsingHeader
                            | ImGuiTreeNodeFlags_Framed;

    ImGui::Begin( s_loadGame->menu.name, NULL, s_loadGame->menu.flags );
    ImGui::SetWindowSize( ImVec2( s_loadGame->menu.width, s_loadGame->menu.height ) );
    ImGui::SetWindowPos( ImVec2( s_loadGame->menu.x, s_loadGame->menu.y ) );

    UI_EscapeMenuToggle();
    if ( UI_MenuTitle( s_loadGame->title->value, 2.25f ) ) {
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
    if ( s_loadGame->numSaves ) {
        {
            ImGui::Begin( "##ModList", NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove
                | ImGuiWindowFlags_NoMouseInputs | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse );
            ImGui::SetWindowPos( ImVec2( ui->gpuConfig.vidWidth * 0.75f, 64 * ui->scale ) );
            ImGui::SetWindowSize( ImVec2( ui->gpuConfig.vidWidth * 0.25f, ui->gpuConfig.vidHeight - 10 ) );
            ImGui::SeparatorText( "Loaded Modules" );
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
            ImGui::End();
        }
        FontCache()->SetActiveFont( PressStart2P );
        for ( i = 0; i < s_loadGame->numSaves; i++ ) {
            if ( ImGui::TreeNodeEx( (void *)(uintptr_t)s_loadGame->saveList[i].name, treeNodeFlags, s_loadGame->saveList[i].name ) ) {
                if ( ImGui::IsMouseClicked( ImGuiMouseButton_Left ) ) {
                    Snd_PlaySfx( ui->sfx_select );
                }
                s_loadGame->currentSave = i;

                if ( ImGui::IsItemClicked( ImGuiMouseButton_Left ) && ImGui::IsMouseDoubleClicked( ImGuiMouseButton_Left ) ) {
                    Snd_PlaySfx( ui->sfx_select );
                    Cvar_Set( "sgame_SaveName", s_loadGame->saveList[i].name );
                    gi.state = GS_LEVEL;
                    g_pModuleLib->ModuleCall( sgvm, ModuleOnLoadGame, 0 );
                }

                ImGui::SetWindowFontScale( ( font_scale * 0.75f ) * ui->scale );
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
                ImGui::SetWindowFontScale( ( font_scale ) * ui->scale );
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

    if ( !ui->uiAllocated ) {
        s_loadGame = (loadGameMenu_t *)Hunk_Alloc( sizeof( *s_loadGame ), h_high );
    }
    memset( s_loadGame, 0, sizeof( *s_loadGame ) );

    s_loadGame->title = strManager->ValueForKey( "SP_LOADGAME_TITLE" );

    s_loadGame->menu.draw = LoadGameMenu_Draw;
    s_loadGame->menu.flags = MENU_DEFAULT_FLAGS;
    s_loadGame->menu.name = s_loadGame->title->value;
    s_loadGame->menu.x = 0;
    s_loadGame->menu.y = 0;
    s_loadGame->menu.width = ui->gpuConfig.vidWidth;
    s_loadGame->menu.height = ui->gpuConfig.vidHeight;
    s_loadGame->menu.fullscreen = qtrue;
    s_loadGame->menu.titleFontScale = 3.5f;
    s_loadGame->menu.textFontScale = 1.5f;

    //
    // init savefiles
    //

    s_loadGame->numSaves = 0;
    fileList = g_pArchiveHandler->GetSaveFiles( &s_loadGame->numSaves );

    if ( s_loadGame->numSaves ) {
        Cvar_Set( "sg_numSaves", va( "%li", (int64_t)s_loadGame->numSaves ) );

        s_loadGame->saveList = (saveinfo_t *)Z_Malloc( sizeof( saveinfo_t ) * s_loadGame->numSaves, TAG_SAVEFILE );
        info = s_loadGame->saveList;

        for ( i = 0; i < s_loadGame->numSaves; i++, info++ ) {
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
    UI_PushMenu( &s_loadGame->menu );
}
