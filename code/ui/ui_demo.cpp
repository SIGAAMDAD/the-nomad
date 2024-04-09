// ui_demo.cpp -- a wee little thank you popup for anyone playing the early versions
// of the game and/or the demos of the full release

#include "ui_lib.h"

static menuframework_t *demo;

// NOTE: this will change with each major version
#define DEMO_STRING \
    "Thank you very much for playing this demo of \"The Nomad\"! :)\n" \
    "More levels and a full alpha/early-acccess versions are in development."

static void DemoMenu_Draw( void )
{
    const int windowFlags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize
        | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoCollapse;

    ImGui::Begin( "##DemoMenu", NULL, windowFlags );
    ImGui::SetWindowFontScale( 2.5f * gi.scale );
    ImGui::SetWindowPos( ImVec2( 100 * gi.scale, 200 * gi.scale ) );

    ImGui::TextUnformatted( DEMO_STRING );

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
    demo = (menuframework_t *)Hunk_Alloc( sizeof( *demo ), h_high );

    demo->draw = DemoMenu_Draw;
    demo->fullscreen = qfalse;
}

void UI_DemoMenu( void )
{
    UI_PushMenu( demo );
}