// ui_demo.cpp -- a wee little thank you popup for anyone playing the early versions
// of the game and/or the demos of the full release

#include "ui_lib.h"
#include <SDL2/SDL.h>

typedef struct {
    menuframework_t menu;

    nhandle_t discordShader;
    nhandle_t sitesShader;
} demoMenu_t;

static demoMenu_t *s_demo;

// NOTE: this will change with each major version
#define DEMO_STRING \
    "Thank you very much for playing this demo of \"The Nomad\"! :)\n" \
    "More levels and a full alpha/early-acccess versions are in development.\n" \
    "If you would like to stay update, check out the discord for updates or the weekly dev blog.\n"

static void DemoMenu_Draw( void )
{
    const int windowFlags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize
        | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoCollapse;

    ImGui::Begin( "##DemoMenu", NULL, windowFlags );
    ImGui::SetWindowFontScale( 2.5f * gi.scale );
    ImGui::SetWindowPos( ImVec2( 100 * gi.scale, 200 * gi.scale ) );

    ImGui::TextUnformatted( DEMO_STRING );
    ImGui::NewLine();

    ImGui::Image( (ImTextureID)(uintptr_t)s_demo->discordShader, ImVec2( 256 * ui->scale, 256 * ui->scale ) );
    if ( ImGui::IsItemClicked() ) {
        Snd_PlaySfx( ui->sfx_select );
        // TODO: slap that shit in here
    }

    ImGui::SameLine();

    ImGui::Image( (ImTextureID)(uintptr_t)s_demo->sitesShader, ImVec2( 256 * ui->scale, 256 * ui->scale ) );
    if ( ImGui::IsItemClicked() ) {
        Snd_PlaySfx( ui->sfx_select );
        SDL_OpenURL( "https://sites.google.com/view/gdrgames" );
    }

    ImGui::End();

    if ( Key_AnyDown() ) {
        UI_PopMenu();
        UI_ForceMenuOff();
        ui->menusp = 0;

        UI_SetActiveMenu( UI_MENU_MAIN );
        return;
    }
}

void DemoMenu_Cache( void )
{
    if ( !ui->uiAllocated ) {
        s_demo = (demoMenu_t *)Hunk_Alloc( sizeof( *s_demo ), h_high );
    }
    memset( s_demo, 0, sizeof( *s_demo ) );

    s_demo->menu.draw = DemoMenu_Draw;
    s_demo->menu.fullscreen = qfalse;

    s_demo->discordShader = re.RegisterShader( "menu/demo/discordIcon" );
    s_demo->sitesShader = re.RegisterShader( "menu/demo/sitesIcon" );
}

void UI_DemoMenu( void )
{
    DemoMenu_Cache();
    UI_PushMenu( &s_demo->menu );
}