#include "../game/g_game.h"
#include "ui_public.h"
#include "ui_menu.h"
#include "ui_lib.h"
#include "ui_window.h"
#include "ui_string_manager.h"

typedef struct {
    CUIMenu menu;

    const stringHash_t *thenomad;
    const stringHash_t *enterGame;
} titlemenu_t;

static titlemenu_t title;

static void TitleMenu_Draw( void )
{
    float font_scale;
    int i;

    font_scale = ImGui::GetFont()->Scale;

    // setup window
    CUIWindow window( "TitleMenu" );
    {
        window.SetFontScale(font_scale * 6);
        window.DrawStringCentered(title.thenomad->value);
    }

    for (i = 0; i < 10; i++) {
        ImGui::NewLine();
    }

    window.SetFontScale(font_scale * 2.5f);
    window.DrawStringCentered( "Press Any Key" );

    // exit?
    if (Key_IsDown( KEY_ESCAPE )) {
        Sys_Exit(1);
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
