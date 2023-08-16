#include "ui_local.h"

menu_t curMenu;

void UI_DrawRect(int x, int y, int width, int height)
{
    RE_DrawRect(x, y, width, height);
}

static int UI_Init(int menuIndex);
static int UI_Clear(void);
static int UI_Shutdown(void);

int vmMain(int command, int arg0, int arg1, int arg2, int arg3, int arg4,
            int arg5, int arg6, int arg7, int arg8, int arg10, int arg11)
{
    switch (command) {
    case UI_INIT:
        return UI_Init(arg0);
    default:

    };

    return -1;
}

void UI_DrawButtonBox(const menuWidget_t *box)
{
    RE_DrawLine(box->x, );
    RE_DrawRect(box->)
}

void UI_DrawMenu(const menu_t *menu)
{
    int i;
    const menuWidget_t *base;

    RE_DrawText(menu->displayName, 0, 0, 0, 0);

    for (i = 0; i < menu->numButtons; i++) {
        base = &menu->buttons[i].base;
        RE_DrawText(base->label, base->labelX, base->labelY, base->labelWidth, base->labelHeight);
    }
}

void UI_InitMenu(int numButtons, int numTextBoxes, int numSliders, const char *name)
{
    curMenu.sliders = (menuSlider_t *)G_AllocMem(sizeof(*curMenu.sliders) * numSliders);
    curMenu.buttons = (menuButton_t *)G_AllocMem(sizeof(*curMenu.buttons) * numButtons);
    curMenu.textBoxes = (menuTextBox_t *)G_AllocMem(sizeof(*curMenu.textBoxes) * numTextBoxes);
    curMenu.numButtons = numButtons;
    curMenu.numSliders = numSliders;
    curMenu.numTextBoxes = numTextBoxes;
    curMenu.screenHeight = R_ScreenHeight();
    curMenu.screenWidth = R_SreenWidth();
    N_strncpyz(curMenu.displayName, name, sizeof(curMenu.displayName));
}

static int UI_Init(int menuIndex)
{
    G_ClearMem();
    memset(&curMenu, 0, sizeof(curMenu));

    switch (menuIndex) {
    case MENU_INDEX_TITLE:
        UI_InitTitleScreen();
        break;
    case MENU_INDEX_MAIN:
        UI_InitMainMenu();
        break;
    default:
        Com_Error( ERR_FATAL, "Invalid menu index: %i", menuIndex );
    };

    return 0;
}

static int UI_Clear(void)
{
    G_ClearMem();
    memset(&curMenu, 0, sizeof(curMenu));

    return 0;
}

static int UI_Shutdown(void)
{
    return UI_Clear();
}

// 256 KiB for the ui vm to use
#define MEMPOOL_SIZE (256*1024)
static char mempool[MEMPOOL_SIZE];
static int allocPoint;

void G_ClearMem(void)
{
    memset(mempool, 0, sizeof(mempool));
    allocPoint = 0;
}

void *G_AllocMem(int size)
{
    char *buf;

    if (!size || size >= sizeof(mempool))
        Com_Error( ERR_FATAL, "G_AllocMem: bad size: %i", size );
    
    // round to 16-bit
    size = (size - 15) & ~15;

    buf = &mempool[allocPoint];
    allocPoint += size;

    return buf;
}
