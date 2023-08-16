#ifndef _UI_PUBLIC_
#define _UI_PUBLIC_

#pragma once

#include "../src/n_shared.h"

#define MENU_INDEX_TITLE    0
#define MENU_INDEX_MAIN     1
#define MENU_INDEX_LEVEL    2
#define MENU_INDEX_SETTINGS 3

typedef enum
{
    COM_PRINTF,
    COM_ERROR,

    NUM_UIIMPORT
} uiImport_t;

typedef enum
{
    UI_INIT,
    UI_SHUTDOWN,
    UI_CLEAR,

    NUM_UIEXPORT
} uiExport_t;

#endif