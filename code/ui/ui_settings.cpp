#include "../game/g_game.h"
#include "ui_public.h"
#include "ui_menu.h"
#include "ui_lib.h"
#include "ui_window.h"
#include "ui_string_manager.h"

typedef struct {
    CUIMenu menu;

    int currentAntialising;
    menuList_t antialising;
    menuList_t textureFilter;
} settingsMenu_t;

static settingsMenu_t settings;

static const menuListItem_t antialiasing_strings[] = {
    {"MSAA 32x",                    "Traditional Multisampling, 32 Samples Per Pixel"},
    {"MSAA 16x",                    "Traditional Multisampling, 16 Samples Per Pixel"},
    {"MSAA 8x",                     "Traditional Multisampling, 8 Samples Per Pixel"},
    {"MSAA 4x",                     "Traditional Multisampling, 4 Samples Per Pixel"},
    {"MSAA 2x",                     "Traditional Multisampling, 2 Samples Per Pixel"},
    {"SSAA",                        "Super Sampling"}
//    {"Adaptive Super-Sampling",},
};

static const char *textureFilters[] = {
    "Linear",
    "Nearest",
    "Bilinear",
    "Trilinear"
};

static const char *textureDetails[] = {
    ""
};

static void SettingsMenuControls_Draw( const CUIWindow& window )
{
    if (ImGui::BeginMenu("Controls")) {
        

        ImGui::EndMenu();
    }
}

static void SettingsMenuGraphics_Draw( const CUIWindow& window )
{
    if (ImGui::BeginMenu("Graphics")) {
        ImGui::BeginTable(" ", 2);
        {
            ImGui::TableNextColumn();
            ImGui::TextUnformatted("Antialising");
            ImGui::TableNextColumn();

            window.DrawMenuList( &settings.antialising );
        }
        ImGui::EndTable();

        ImGui::EndMenu();
    }
}

void SettingsMenu_Draw( void )
{
    CUIWindow window("SettingsMenu");

    window.SetFontScale(4);
    window.DrawString("Settings");

    if (ImGui::BeginMenuBar()) {
        SettingsMenuControls_Draw( window );
        SettingsMenuGraphics_Draw( window );

        ImGui::EndMenuBar();
    }
}

void SettingsMenu_Cache( void )
{
    memset(&settings, 0, sizeof(settings));

    settings.menu.Draw = SettingsMenu_Draw;

    settings.antialising.items = antialiasing_strings;
    settings.antialising.nitems = arraylen(antialiasing_strings);
    settings.antialising.menuName = "Select Antialising Type";
}

void UI_SettingsMenu( void )
{
    SettingsMenu_Cache();

    ui->PushMenu( &settings.menu );
}
