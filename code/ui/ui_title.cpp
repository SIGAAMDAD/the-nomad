#include "../game/g_game.h"
#include "ui_public.h"
#include "ui_menu.h"
#include "ui_lib.h"
#include "ui_window.h"
#include "ui_string_manager.h"

typedef struct {
    CUIMenu menu;

    uint32_t newLineHeight;

    const stringHash_t *thenomad;
    const stringHash_t *enterGame;
} titlemenu_t;

static titlemenu_t title;

static void TitleMenu_Draw( void )
{
    float font_scale;
    uint32_t i;

    font_scale = ImGui::GetFont()->Scale;

    // setup window
    CUIWindow window("TitleMenu");

    ImGui::SetWindowFontScale( font_scale * 6.5f * ui->scale );
    window.DrawStringCentered( title.thenomad->value );

    for (i = 0; i < title.newLineHeight; i++) {
        ImGui::NewLine();
    }

    ImGui::SetWindowFontScale( font_scale * 1.5f * ui->scale );
    window.DrawStringCentered( "Press Any Key" );

    // exit?
    if (Key_IsDown( KEY_ESCAPE )) {
        Sys_Exit(1);
    }
    // if the console's open, don't catch
    if (Key_GetCatcher() & KEYCATCH_CONSOLE) {
        return;
    }

    // is a key down?
    for (i = 0; i < NUMKEYS; i++) {
        if (Key_IsDown( i )) {
            ui->SetActiveMenu(UI_MENU_MAIN);
        }
    }
}

void TitleMenu_Cache( void )
{
    memset(&title, 0, sizeof(title));

    title.menu.Draw = TitleMenu_Draw;

    switch (ui->GetConfig().vidHeight) {
    case 768:
        title.newLineHeight = 6;
        break;
    case 1536:
        title.newLineHeight = 7;
        break;
    case 720:
        title.newLineHeight = 6;
        break;
    case 900:
        title.newLineHeight = 6;
        break;
    case 1080:  
        title.newLineHeight = 6;
        break;
    case 2160:
        title.newLineHeight = 8;
        break;
    };

    title.thenomad = strManager->ValueForKey("MENU_LOGO_STRING");
    title.enterGame = strManager->ValueForKey("MENU_TITLE_ENTER_GAME");

    if (!title.thenomad) {
        N_Error(ERR_FATAL, "TitleMenu_Cache: failed to load MENU_LOGO_STRING value string");
    }
    if (!title.enterGame) {
        N_Error(ERR_FATAL, "TitleMenu_Cache: failed to load MENU_TITLE_ENTER_GAME value string");
    }
}

void UI_TitleMenu( void )
{
    TitleMenu_Cache();

    ui->PushMenu( &title.menu );
}
