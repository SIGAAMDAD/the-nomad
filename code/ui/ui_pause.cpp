#include "ui_lib.h"

typedef enum {
    HELP_PARRY,

    NUMHELPSTRINGS
} helpstate_t;

typedef struct
{
    CUIMenu menu;
    
    const char *tipOfTheDay;
    qboolean helpMenu;
    helpstate_t helpstate;

    qboolean exitToMM;
    qboolean settingsMenu;

    const stringHash_t *title;
    const stringHash_t *help;
    const stringHash_t *resume;
    const stringHash_t *settings;
    const stringHash_t *checkpoint;
    const stringHash_t *exitToMainMenu;
} pausemenu_t;

#define PAUSEMENU_VOLUME_CAP 40

static const char *dailyTips[] = {
    "You can parry anything that's a projectile, that includes bullets, flying corpses, blades, etc.",
    "The grappling hook deals a little bit of damage every time it hooks onto an enemy",
};

static const char *helpStrings[NUMHELPSTRINGS][2] = {
    {
        "Tutorial 1: Parrying",

        "Parrying is a mechanic that can help you a lot in fights"
    },
};

// PAUSE. REWIND. PLAY.
static pausemenu_t menu;

static void PauseMenu_Help( void )
{
    switch (ui->GetState()) {
    case STATE_HELP:
        ui->EscapeMenuToggle( STATE_PAUSE );
        if (ui->GetState() != STATE_HELP) {
            menu.helpMenu = qfalse;
            break;
        }
        else if (ui->Menu_Title( "HELP" )) {
            menu.helpMenu = qfalse;
            break;
        }
        if (ui->Menu_Option( "Tutorial 1: How To Parry" )) {
            ui->SetState( STATE_HELP_SHOW );
            menu.helpstate = HELP_PARRY;
        }
        break;
    case STATE_HELP_SHOW:
        ui->EscapeMenuToggle( STATE_HELP );
        if (ui->GetState() != STATE_HELP_SHOW) {
            break;
        }
        else if (ui->Menu_Title( helpStrings[menu.helpstate][0] )) {
            ui->SetState( STATE_HELP );
            break;
        }

        ImGui::NewLine();
        ImGui::TextUnformatted( helpStrings[menu.helpstate][1] );
        break;
    };
}

static void PauseMenu_DrawTitle( void ) {
    const float font_scale = ImGui::GetFont()->Scale;

    ImGui::SetWindowFontScale( font_scale * 3.75f * ui->scale );
    ImGui::TextUnformatted( menu.title->value );
    ImGui::SetWindowFontScale( font_scale * 1.5f * ui->scale );
}

static void PauseMenu_ExitToMainMenu( void )
{
    // verify with the user that they actually want to end the level
    if ( VM_Call( sgvm, 0, SGAME_RUNTIC ) ) {
        // restart everything
        Cbuf_ExecuteText( EXEC_NOW, "vid_restart fast\n" );
        
        // clear all menus, then restack them
        ui->ForceMenuOff();
        Key_SetCatcher( KEYCATCH_UI );
        ui->SetActiveMenu( UI_MENU_TITLE );
        ui->SetState( STATE_MAIN );
        ui->SetActiveMenu( UI_MENU_MAIN );
    }
}

static void PauseMenu_Draw( void )
{
    const int windowFlags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize
                            | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize;

    ImGui::Begin("PauseMenu", NULL, windowFlags);
    ImGui::SetWindowSize( ImVec2( (float)ui->GetConfig().vidWidth, (float)ui->GetConfig().vidHeight ) );
    ImGui::SetWindowPos( ImVec2( 0, 0 ) );

    if (menu.helpMenu) {
        PauseMenu_Help();
        return;
    } else if ( menu.exitToMM ) {
        PauseMenu_ExitToMainMenu();
        return;
    } else if ( menu.settingsMenu ) {
        // we don't want to be drawing everything else behind the settings menu
        // otherwise its very hard to see the options
        ui->SetActiveMenu( UI_MENU_MAIN );
        ui->SetState( STATE_SETTINGS );
        return;
    }

    ui->EscapeMenuToggle( STATE_NONE );
    if ( ui->GetState() == STATE_NONE ) {
        ui->SetActiveMenu( UI_MENU_NONE );
        return;
    }

    PauseMenu_DrawTitle();

    ImGui::BeginTable( " ", 2 );
    {
        if (ui->Menu_Option( menu.resume->value )) {
            ui->SetState( STATE_NONE );
            ui->SetActiveMenu( UI_MENU_NONE );
        }
        ImGui::TableNextRow();
        if (ui->Menu_Option( menu.help->value )) {
            menu.helpMenu = qtrue;
        }
        ImGui::TableNextRow();
        if (ui->Menu_Option( menu.checkpoint->value )) {
            VM_Call( sgvm, 0, SGAME_REWIND_TO_LAST_CHECKPOINT );
        }
        ImGui::TableNextRow();
        if (ui->Menu_Option( menu.settings->value )) {
            menu.settingsMenu = qtrue;
        }
        ImGui::TableNextRow();
        if (ui->Menu_Option( menu.exitToMainMenu->value )) {
            menu.exitToMM = qtrue;
            VM_Call( sgvm, 0, SGAME_ENDLEVEL );
        }
    }
    ImGui::EndTable();

    ImGui::End();
}

void PauseMenu_Cache( void )
{
    memset( &menu, 0, sizeof(menu) );

    menu.menu.Draw = PauseMenu_Draw;

    menu.title = strManager->ValueForKey( "MENU_PAUSE_TITLE" );
    menu.checkpoint = strManager->ValueForKey( "MENU_PAUSE_CHECKPOINT" );
    menu.help = strManager->ValueForKey( "MENU_PAUSE_HELP" );
    menu.resume = strManager->ValueForKey( "MENU_PAUSE_RESUME" );
    menu.settings = strManager->ValueForKey( "MENU_PAUSE_SETTINGS" );
    menu.exitToMainMenu = strManager->ValueForKey( "MENU_PAUSE_ETMM" );

    ui->PushMenu( &menu.menu );
}

void UI_PauseMenu( void )
{
    // force as top level menu
    ui->ForceMenuOff();
    Key_SetCatcher( KEYCATCH_UI );

    PauseMenu_Cache();
}
