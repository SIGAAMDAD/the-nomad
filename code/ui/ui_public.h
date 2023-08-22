#ifndef _UI_PUBLIC_
#define _UI_PUBLIC_

#pragma once

#include "../engine/n_shared.h"

#define MENU_INDEX_TITLE    0
#define MENU_INDEX_MAIN     1
#define MENU_INDEX_LEVEL    2
#define MENU_INDEX_SETTINGS 3

typedef enum
{
    UI_FLOOR = 107,
    UI_CEIL,

    NUM_UIIMPORT
} uiImport_t;

typedef enum
{
    UI_INIT,
    UI_KEY_EVENT,
    UI_SHUTDOWN,
    UI_CLEAR,

    NUM_UIEXPORT
} uiExport_t;

#endif