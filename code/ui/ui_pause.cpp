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

static void PauseMenuResume( void ) {
    Cvar_Set( "sg_paused", "0" );
    Cvar_Set( "ui_active", "0" );
}

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
    }

    ui->EscapeMenuToggle( STATE_NONE );
    if (ui->GetState() != STATE_PAUSE) {
        PauseMenuResume();
        ui->PopMenu();
        Key_SetCatcher( KEYCATCH_SGAME );
        return;
    }
    else if (ui->Menu_Title( menu.title->value )) {
        PauseMenuResume();
        ui->PopMenu();
        Key_SetCatcher( KEYCATCH_SGAME );
        return;
    }

    ImGui::BeginTable( " ", 2 );
    {
        if (ui->Menu_Option( menu.resume->value )) {
            PauseMenuResume();
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
            UI_MainMenu();
            SettingsMenu_Cache();
            ui->SetState( STATE_SETTINGS );
        }
        ImGui::TableNextRow();
        if (ui->Menu_Option( menu.exitToMainMenu->value )) {
            menu.exitToMM = qtrue;
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
}

void UI_PauseMenu( void )
{
    PauseMenu_Cache();

    // set sgame to paused
    Cvar_Set( "sg_paused", "1" );

    Key_SetCatcher( KEYCATCH_UI );

    ui->SetState( STATE_PAUSE );
    ui->PushMenu( &menu.menu );
}
