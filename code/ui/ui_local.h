#ifndef _UI_LOCAL_
#define _UI_LOCAL_

#include "ui_public.h"

typedef struct
{
    char *label;
    int labelY;
    int labelX;
    int labelWidth;
    int labelHeight;
    int x;
    int y;
    int width;
    int height;
} menuWidget_t;

typedef struct
{
    menuWidget_t base;
    float min;
    float max;
    float value;
} menuSlider_t;

typedef struct
{
    menuWidget_t base;
    qboolean on;
} menuButton_t;

typedef struct
{
    menuWidget_t base;
    char *textBuf;
} menuTextBox_t;

typedef struct
{
    char displayName[MAX_STRING_CHARS];

    menuButton_t *buttons;
    int numButtons;
    menuSlider_t *sliders;
    int numSliders;
    menuTextBox_t *textBoxes;
    int numTextBoxes;

    int screenWidth;
    int screenHeight;
} menu_t;

extern menu_t *curMenu;

void UI_InitMenu(int numButtons, int numTextBoxes, int numSliders, const char *name);

void UI_DrawRect(int x, int y, int width, int height);
void UI_DrawMenu(const menu_t *menu);

void UI_InitMainMenu(void);
void UI_InitTitleScreen(void);

void *G_AllocMem(int size);
void G_ClearMem(void);

#endif