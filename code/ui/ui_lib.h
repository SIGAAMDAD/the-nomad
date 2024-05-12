#ifndef __UI_LIB__
#define __UI_LIB__

#pragma once

#include "../engine/n_shared.h"
#include "../game/g_game.h"
#include "ui_menu.h"
#include "ui_string_manager.h"
#include "ui_window.h"
#include "ui_font.h"
#include <new>
#include <fstream>
#include "colors.h"

typedef enum : uint64_t
{
    STATE_MAIN,
        STATE_SINGLEPLAYER,
            STATE_NEWGAME,
            STATE_LOADGAME,
            STATE_PLAYMISSION,

    STATE_LEGAL,

    STATE_MODS,

    STATE_SETTINGS,
        STATE_PERFORMANCE,
        STATE_GRAPHICS,
        STATE_CONTROLS,
        STATE_AUDIO,
        STATE_GAMEPLAY,
    
    STATE_CREDITS,

    STATE_PAUSE,
        STATE_HELP,
            STATE_HELP_SHOW,
    
    STATE_THANK_YOU_FOR_PLAYING_DEMO,

    STATE_NONE,
    STATE_ERROR
} menustate_t;

#define MAX_MENU_DEPTH 8

#define RCOLUMN_OFFSET			( BIGCHAR_WIDTH )
#define LCOLUMN_OFFSET			(-BIGCHAR_WIDTH )

#define SLIDER_RANGE			10

#include "ui_menu.h"

// cvars
extern cvar_t *ui_language;
extern cvar_t *ui_printStrings;
extern cvar_t *ui_active;

extern const char *UI_LangToString( int32_t lang );

extern void			Menu_Cache( void );

extern vec4_t colorGold;
//extern sfxHandle_t	menu_in_sound;
//extern sfxHandle_t	menu_move_sound;
//extern sfxHandle_t	menu_out_sound;
//extern sfxHandle_t	menu_buzz_sound;
//extern sfxHandle_t	menu_null_sound;
//extern sfxHandle_t	weaponChangeSound;
//extern vec4_t		menu_text_color;
//extern vec4_t		menu_grayed_color;
//extern vec4_t		menu_dark_color;
//extern vec4_t		menu_highlight_color;
//extern vec4_t		menu_red_color;
//extern vec4_t		menu_black_color;
//extern vec4_t		menu_dim_color;
extern vec4_t		color_black;
extern vec4_t		color_white;
extern vec4_t		color_yellow;
extern vec4_t		color_blue;
extern vec4_t		color_orange;
extern vec4_t		color_red;
extern vec4_t		color_dim;
//extern vec4_t		name_color;
//extern vec4_t		list_color;
//extern vec4_t		listbar_color;
//extern vec4_t		text_color_disabled; 
//extern vec4_t		text_color_normal;
//extern vec4_t		text_color_highlight;

#define VIRT_KEYBOARD_ASCII     0
#define VIRT_KEYBOARD_SYMBOLS   1
#define NUM_VIRT_KEYBOARD_MODES 2

typedef struct {
    char *pBuffer;
    int bufTextLen;
    int bufMaxLen;

    int mode;
    int caps;
    qboolean open;

    qboolean capsToggle;
    qboolean backspaceToggle;
    qboolean spaceToggle;
    qboolean doneToggle;
    qboolean modeToggle;
} virtualKeyboard_t;

typedef struct {
    int frametime;
    int realtime;
    int menusp;

    menuframework_t *activemenu;
    menuframework_t *stack[MAX_MENU_DEPTH];

    ImFont *currentFont;

    gpuConfig_t gpuConfig;
    qboolean debug;

    nhandle_t whiteShader;
    nhandle_t menubackShader;

    sfxHandle_t sfx_null;
    sfxHandle_t sfx_scroll;
    sfxHandle_t sfx_back;
    sfxHandle_t sfx_select;
    qboolean sfx_scroll_toggle;

    nhandle_t rb_on;
    nhandle_t rb_off;
    nhandle_t arrow_horz_left;
    nhandle_t arrow_horz_right;
    nhandle_t back_0;
    nhandle_t back_1;
    nhandle_t arrows_vert_0;
    nhandle_t arrow_vert_bot;
    nhandle_t arrow_vert_top;

    // xbox specific
    nhandle_t controller_x;
    nhandle_t controller_y;
    nhandle_t controller_a;
    nhandle_t controller_b;

    nhandle_t controller_start;
    nhandle_t controller_back;
    nhandle_t controller_dpad_up;
    nhandle_t controller_dpad_down;
    nhandle_t controller_dpad_left;
    nhandle_t controller_dpad_right;
    nhandle_t controller_left_trigger;
    nhandle_t controller_left_button;
    nhandle_t controller_right_trigger;
    nhandle_t controller_right_button;

    qboolean uiAllocated;
    virtualKeyboard_t virtKeyboard;

    float scale;
    float bias;

    nhandle_t backdrop;

//    menustate_t state;
    uiMenu_t menustate;
    qboolean escapeToggle;
    qboolean backHovered;
} uiGlobals_t;

extern uiGlobals_t *ui;

typedef struct {
    const char *name;
    const char *tooltip;
} dif_t;

extern dif_t difficultyTable[NUMDIFS];

extern void UI_EscapeMenuToggle( void );
extern qboolean UI_MenuTitle( const char *label, float scale = 3.75f );
extern qboolean UI_MenuOption( const char *label );
extern void UI_DrawNamedPic( float x, float y, float width, float height, const char *name );
extern void UI_DrawHandlePic( float x, float y, float width, float height, nhandle_t hShader );
extern qboolean UI_IsFullscreen( void );
extern void UI_SetActiveMenu( uiMenu_t menu );
extern void UI_ForceMenuOff( void );
extern void UI_PushMenu( menuframework_t *menu );
extern void UI_PopMenu( void );
extern int UI_TextInput( const char *label, char *pBuffer, size_t bufSize, ImGuiInputTextFlags flags = ImGuiInputTextFlags_EnterReturnsTrue );
extern int UI_VirtualKeyboard( const char *pName, char *pBuffer, size_t nBufSize );

//
// ui_confirm.cpp
//
extern void         UI_ConfirmMenu( const char *question, void (*draw)( void ), void (*action)( qboolean result ) );

//
// ui_loadgame.cpp
//
extern void         UI_LoadGameMenu( void );
extern void         LoadGameMenu_Cache( void );

//
// ui_newgame.cpp
//
extern void         UI_NewGameMenu( void );
extern void         NewGameMenu_Cache( void );

//
// ui_credits.cpp
//
extern void         UI_CreditsMenu( void );
extern void         CreditsMenu_Cache( void );

//
// ui_pause.cpp
//
extern void         UI_PauseMenu( void );
extern void         PauseMenu_Cache( void );

//
// ui_title.cpp
//
extern void         UI_TitleMenu( void );
extern void         TitleMenu_Cache( void );

//
// ui_demo.cpp
//
extern void         UI_DemoMenu( void );
extern void         DemoMenu_Cache( void );

//
// ui_intro.cpp
//
extern void         UI_IntroMenu( void );
extern void         IntroMenu_Cache( void );

//
// ui_main.cpp
//
extern void         UI_MainMenu( void );
extern void         MainMenu_Cache( void );
extern void         MainMenu_Draw( void );

//
// ui_settings.cpp
//
extern void         UI_SettingsMenu( void );
extern void         SettingsMenu_Cache( void );

//
// ui_legal.cpp
//
extern void         LegalMenu_Cache( void );
extern void         LegalMenu_Draw( void );

//
// ui_mods.cpp
//
extern void         UI_ModsMenu( void );
extern void         ModsMenu_Cache( void );
extern void         ModsMenu_Draw( void );
extern qboolean     ModsMenu_IsModuleActive( const char *pName );

//
// ui_single_player.cpp
//
extern void         UI_SinglePlayerMenu( void );
extern void         SinglePlayerMenu_Cache( void );
extern void         SinglePlayerMenu_Draw( void );

#endif