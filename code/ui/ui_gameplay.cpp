#include "ui_lib.h"

#define ID_VIDEO        0
#define ID_PERFORMANCE  1
#define ID_AUDIO        2
#define ID_CONTROLS     3
#define ID_GAMEPLAY     4
#define ID_TABLE        5

typedef struct {
    menuframework_t menu;

    menutable_t table;

    menutext_t difficulty;
    menutext_t debug;
} gameplayOptionsInfo_t;

static gameplayOptionsInfo_t s_gameplayOptionsInfo;

extern menutab_t settingsTabs;
extern menubutton_t settingsResetDefaults;
extern menubutton_t settingsSave;
static void GameplaySettingsMenu_EventCallback( void *ptr, int event )
{
    if ( event != EVENT_ACTIVATED ) {
        return;
    }

    switch ( ( (menucommon_t *)ptr )->id ) {
    case ID_VIDEO:
        UI_PopMenu();
        UI_VideoSettingsMenu();
        break;
    case ID_PERFORMANCE:
        UI_PopMenu();
        UI_PerformanceSettingsMenu();
        break;
    case ID_AUDIO:
        UI_PopMenu();
        UI_AudioSettingsMenu();
        break;
    case ID_CONTROLS:
        UI_PopMenu();
        UI_ContolsSettingsMenu();
        break;
    case ID_GAMEPLAY:
        break;
    default:
        break;
    };
}

void GameplaySettingsMenu_Cache( void )
{
    memset( &s_gameplayOptionsInfo, 0, sizeof( s_gameplayOptionsInfo ) );

    s_gameplayOptionsInfo.menu.fullscreen = qtrue;
    s_gameplayOptionsInfo.menu.width = ui->gpuConfig.vidWidth;
    s_gameplayOptionsInfo.menu.height = ui->gpuConfig.vidHeight;
    s_gameplayOptionsInfo.menu.textFontScale = 1.5f;
    s_gameplayOptionsInfo.menu.titleFontScale = 3.5f;
    s_gameplayOptionsInfo.menu.x = 0;
    s_gameplayOptionsInfo.menu.y = 0;
    s_gameplayOptionsInfo.menu.name = "Audio##SettingsMenu";

    Menu_AddItem( &s_gameplayOptionsInfo.menu, &settingsTabs );

    Menu_AddItem( &s_gameplayOptionsInfo.menu, &settingsSave );
    Menu_AddItem( &s_gameplayOptionsInfo.menu, &settingsResetDefaults );
}

void UI_GameplaySettingsMenu( void )
{
    GameplaySettingsMenu_Cache();
    UI_PushMenu( &s_gameplayOptionsInfo.menu );
}
