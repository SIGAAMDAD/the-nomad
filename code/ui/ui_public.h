#ifndef __UI_PUBLIC__
#define __UI_PUBLIC__

#pragma once

#include "../rendercommon/r_public.h"
#include "../game/g_sound.h"

typedef enum : uint64_t {
    UI_MENU_INTRO,
    UI_MENU_TITLE,
    UI_MENU_MAIN,
    UI_MENU_PAUSE,
    UI_MENU_NONE,

    NUM_UI_MENUS
} uiMenu_t;

extern "C" void UI_Init( void );
extern "C" void UI_Shutdown( void );
extern "C" void UI_Refresh( int realtime );

#endif