#include "ui_local.h"

static const char *buttons[] = {
    "New Game",
    "Load Game",
    "Settings",
    "Credits",
    "Exit To Title",
    "Exit To Desktop"
};

static const char *copyrightString =
"Copyright (C) GDR Games 2022-2024. Copyright (C) SIGAAMDAD 2022-2024. This is free software licensed under the\n"
"GNU GPL (General Public License) v2.0 or later.";

void UI_InitMainMenu(void)
{
    menu_t *m;
    int i;

    m = &curMenu;
    UI_InitMenu(arraylen(buttons), 1, 0, "Main Menu");
    
    for (i = 0; i < m->numButtons; i++) {
        m->buttons[i].label = (char *)buttons[i];
        m->buttons[i].on = qfalse;
    }
    
    m->textBoxes[0].label = NULL;
    m->textBoxes[0].textBuf = copyrightString;
    m->textBoxes[0].base.height = m->screenHeight / 4;
    m->textBoxes[0].base.width = m->screenWidth / 2;
    m->textBoxes[0].base.x = m->screenWidth / 2;
    m->textBoxes[0].base.y = m->screenHeight - (m->textBoxes[0].base.height);
}