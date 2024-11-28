#include "ui_lib.h"
#include "../rendercommon/imgui_internal.h"

typedef struct {
    menuframework_t menu;
    nlohmann::json *sections;
    bool *filters;
    uint64_t numSections;
    const nlohmann::json *currentEntry;
} dataBaseMenu_t;

static dataBaseMenu_t *s_dataBase;

static void DataBase_Load( void )
{
    char **fileList;
    uint64_t nFiles, i;
    union {
        char *b;
        void *v;
    } f;
    uint64_t nLength;

    Con_Printf( "Loading game data base...\n" );

    fileList = FS_ListFiles( "database", ".json", &nFiles );
    s_dataBase->numSections = nFiles;
    s_dataBase->sections = (nlohmann::json *)Hunk_Alloc( sizeof( *s_dataBase->sections ) * nFiles, h_high );
    s_dataBase->filters = (bool *)Hunk_Alloc( sizeof( *s_dataBase->filters ) * nFiles, h_high );

    for ( i = 0; i < nFiles; i++ ) {
        try {
            nLength = FS_LoadFile( va( "database/%s", fileList[i] ), &f.v );
            if ( !nLength || !f.v ) {
                Con_Printf( COLOR_YELLOW "Error loading dataBase file '%s'\n", fileList[i] );
                continue;
            }
            s_dataBase->sections[i] = nlohmann::json::parse( f.b, f.b + nLength );

            Con_Printf( "- Loaded section '%s'\n", fileList[i] );
            FS_FreeFile( f.v );
        } catch ( const nlohmann::json::exception& e ) {
            Con_Printf( COLOR_RED "Error parsing dataBase file '%s', skipping\n", fileList[i] );
            if ( f.v ) {
                FS_FreeFile( f.v );
            }
        }
    }
    FS_FreeFileList( fileList );
}

/*
* DataBase_DrawEntry: this is a monster, but works for now
*/
static void DataBase_DrawEntry( const nlohmann::json& entry )
{
    FontCache()->SetActiveFont( RobotoMono );
    ImGui::SetWindowFontScale( ImGui::GetFont()->Scale * 1.5f );
    for ( const auto& it : entry.at( "data" ) ) {
        const string_t& type = it.at( "type" );

        if ( type == "text" ) {
            const string_t& text = it.at( "text" );
            ImGui::TextWrapped( text.c_str() );
        }
        else if ( type == "separator" ) {
            ImGui::Separator();
        }
        else if ( type == "collapsing_header" ) {
            const string_t& title = it.at( "title" );
            if ( ImGui::CollapsingHeader( va( "%s##CollapsingHeader%lu", title.c_str(), (uintptr_t)eastl::addressof( it ) ) ) ) {
                if ( ImGui::IsItemClicked( ImGuiMouseButton_Left ) ) {
                    Snd_PlaySfx( ui->sfx_select );
                }

                const string_t& text = it.at( "text" );
                if ( ImGui::GetCurrentContext()->CurrentWindow->DC.TextWrapPos < 0.0f ) {
                    ImGui::PushTextWrapPos( 0.0f );
                }
                UI_DrawText( text.c_str() );
                if ( ImGui::GetCurrentContext()->CurrentWindow->DC.TextWrapPos < 0.0f ) {
                    ImGui::PopTextWrapPos();
                }
            } else {
                if ( ImGui::IsItemClicked( ImGuiMouseButton_Left ) ) {
                    Snd_PlaySfx( ui->sfx_select );
                }
            }
        }
        else if ( type == "bullet_list" ) {
            const nlohmann::json& data = it.at( "bullets" );
            for ( const auto& it : data ) {
                const string_t& text = it;

                ImGui::Bullet();
                ImGui::TextWrapped( "%s", text.c_str() );
            }
        }
        else {
            N_Error( ERR_DROP, "DataBase_DrawEntry: invalid entry type '%s'", type.c_str() );
        }

        if ( it.contains( "data" ) ) {
            DataBase_DrawEntry( it );
        }
    }
}

static void DataBase_DrawSection( const nlohmann::json& section )
{
    FontCache()->SetActiveFont( RobotoMono );
    if ( section.contains( "sections" ) ) {
        const string_t& name = section.at( "name" );

        // embedded child sections
        if ( ImGui::TreeNodeEx( va( "##SectionTree%lu", (uintptr_t)eastl::addressof( section ) ), ImGuiTreeNodeFlags_OpenOnArrow,
            "%s", name.c_str() ) )
        {
            if ( ImGui::IsItemClicked( ImGuiMouseButton_Left ) ) {
                Snd_PlaySfx( ui->sfx_select );
            }
            for ( const auto& it : section.at( "sections" ) ) {
                DataBase_DrawSection( it );
            }
            ImGui::TreePop();
        } else {
            if ( ImGui::IsItemClicked( ImGuiMouseButton_Left ) ) {
                Snd_PlaySfx( ui->sfx_select );
            }
        }
    }
    if ( !section.contains( "entries" ) ) {
        return;
    }
    for ( const auto& it : section.at( "entries" ) ) {
        const string_t& name = it.at( "name" );
        const nlohmann::json *entry = eastl::addressof( it );
        
        if ( s_dataBase->currentEntry == entry ) {
            ImGui::PushStyleColor( ImGuiCol_Text, colorGold );
        }
        ImGui::PushID( ImGui::GetID( va( "SectionBranch%lu", (uintptr_t)entry ) ) );
        ImGui::TextWrapped( "%s", name.c_str() );
        if ( ImGui::IsItemClicked( ImGuiMouseButton_Left ) ) {
            Snd_PlaySfx( ui->sfx_select );
            s_dataBase->currentEntry = entry;
        }
        ImGui::PopID();
        if ( s_dataBase->currentEntry == entry ) {
            ImGui::PopStyleColor();
        }
    }
}

void DataBaseMenu_Draw( void )
{
    uint64_t i;
    const float scale = ImGui::GetFont()->Scale;

    bool *emptyFilters = (bool *)alloca( sizeof( *emptyFilters ) * s_dataBase->numSections );
    memset( emptyFilters, false, sizeof( *emptyFilters ) * s_dataBase->numSections );

    const bool filtersActive = memcmp( s_dataBase->filters, emptyFilters, sizeof( *emptyFilters ) * s_dataBase->numSections ) != 0;

    ImGui::Begin( "##DataBaseMenu", NULL, MENU_DEFAULT_FLAGS );
    ImGui::SetWindowPos( ImVec2( s_dataBase->menu.x, s_dataBase->menu.y ) );
    ImGui::SetWindowSize( ImVec2( s_dataBase->menu.width, s_dataBase->menu.height ) );

    UI_EscapeMenuToggle();
    if ( UI_MenuTitle( s_dataBase->menu.name ) ) {
        UI_PopMenu();
        ImGui::End();
        return;
    }
    
    ImGui::SetCursorScreenPos( ImVec2( 0, 32 * ui->scale ) );
    ImGui::BeginChild( ImGui::GetID( "SectionTree" ), ImVec2( 400 * ui->scale, 650 * ui->scale ), ImGuiChildFlags_None,
        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoTitleBar
        | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBringToFrontOnFocus );
    FontCache()->SetActiveFont( RobotoMono );
    ImGui::SeparatorText( "FILTERS" );
    for ( i = 0; i < s_dataBase->numSections; i++ ) {
        const string_t& name = s_dataBase->sections[i].at( "name" );
        if ( ImGui::Checkbox( va( "%s##DataBaseSectionFilter%lu", name.c_str(), (uintptr_t)&s_dataBase->filters[i] ),
            (bool *)&s_dataBase->filters[i] ) )
        {
            Snd_PlaySfx( ui->sfx_select );
        }
    }

    const ImGuiStyle& style = ImGui::GetStyle();
    if ( !filtersActive ) {
        ImGui::PushStyleColor( ImGuiCol_Button, ImVec4( 0.75f, 0.75f, 0.75f, style.Colors[ ImGuiCol_Button ].w ) );
        ImGui::PushStyleColor( ImGuiCol_ButtonActive, ImVec4( 0.75f, 0.75f, 0.75f, style.Colors[ ImGuiCol_ButtonActive ].w ) );
        ImGui::PushStyleColor( ImGuiCol_ButtonHovered, ImVec4( 0.75f, 0.75f, 0.75f, style.Colors[ ImGuiCol_ButtonHovered ].w ) );
    }
    if ( ImGui::Button( "Clear Filters" ) && filtersActive ) {
        Snd_PlaySfx( ui->sfx_select );
        memset( s_dataBase->filters, false, sizeof( *s_dataBase->filters ) * s_dataBase->numSections );
    }
    if ( !filtersActive ) {
        ImGui::PopStyleColor( 3 );
    }

    ImGui::SeparatorText( "DATABASE" );
    ImGui::SetWindowFontScale( ( scale * 1.8f ) * ui->scale );
    for ( i = 0; i < s_dataBase->numSections; i++ ) {
        if ( !s_dataBase->filters[i] && filtersActive ) {
            continue;
        }
        DataBase_DrawSection( s_dataBase->sections[i] );
    }
    ImGui::EndChild();

    ImGui::SetCursorScreenPos( ImVec2( 404 * ui->scale, 32 * ui->scale ) );
    ImGui::BeginChild( ImGui::GetID( "EntryDraw" ), ImVec2( 700 * ui->scale, ( 768 - 64 ) * ui->scale ), ImGuiChildFlags_None,
        MENU_DEFAULT_FLAGS );
    if ( s_dataBase->currentEntry ) {
        const string_t& name = s_dataBase->currentEntry->at( "name" );
        ImGui::SetWindowFontScale( scale * 1.5f );
        FontCache()->SetActiveFont( AlegreyaSC );
        ImGui::SeparatorText( name.c_str() );

        DataBase_DrawEntry( *s_dataBase->currentEntry );
    }
    ImGui::EndChild();
    ImGui::SetWindowFontScale( scale );

    ImGui::End();
}

void DataBaseMenu_Cache( void )
{
    if ( !ui->uiAllocated ) {
        static dataBaseMenu_t menu;
        s_dataBase = &menu;
        DataBase_Load();
    }
    s_dataBase->currentEntry = NULL;

    s_dataBase->menu.draw = DataBaseMenu_Draw;
    s_dataBase->menu.x = 0;
    s_dataBase->menu.y = 0;
    s_dataBase->menu.width = gi.gpuConfig.vidWidth;
    s_dataBase->menu.height = gi.gpuConfig.vidHeight;
    s_dataBase->menu.fullscreen = qtrue;
    s_dataBase->menu.flags = MENU_DEFAULT_FLAGS;
    s_dataBase->menu.track = Snd_RegisterSfx( "event:/music/main_theme" );

    ui->menubackShader = re.RegisterShader( "menu/mainbackground" );
}

void UI_DataBaseMenu( void )
{
    DataBaseMenu_Cache();
    UI_PushMenu( &s_dataBase->menu );
}