#include "ui_lib.h"

#define ID_VIDEO        0
#define ID_PERFORMANCE  1
#define ID_AUDIO        2
#define ID_CONTROLS     3
#define ID_GAMEPLAY     4
#define ID_TABLE        5

typedef struct {
    menuframework_t menu;

    menutext_t video;
    menutext_t performance;
    menutext_t audio;
    menutext_t controls;
    menutext_t gameplay;

    menutab_t tabs;
    menutable_t table;
} performanceOptionsInfo_t;

static performanceOptionsInfo_t s_performanceOptionsInfo;

static void PerformanceSettingsMenu_EventCallback( void *ptr, int event )
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
        UI_PopMenu();
        UI_GameplaySettingsMenu();
        break;
    default:
        break;
    };
}

void PerformanceSettingsMenu_Cache( void )
{
    static const char *difficulties[] = {
        difficultyTable[ DIF_NOOB ].name,
        difficultyTable[ DIF_RECRUIT ].name,
        difficultyTable[ DIF_MERC ].name,
        difficultyTable[ DIF_NOMAD ].name,
        difficultyTable[ DIF_BLACKDEATH ].name,
        "Just A Minor Inconvenience"
    };

    memset( &s_performanceOptionsInfo, 0, sizeof( s_performanceOptionsInfo ) );

    s_performanceOptionsInfo.menu.fullscreen = qtrue;
    s_performanceOptionsInfo.menu.width = ui->gpuConfig.vidWidth;
    s_performanceOptionsInfo.menu.height = ui->gpuConfig.vidHeight;
    s_performanceOptionsInfo.menu.x = 0;
    s_performanceOptionsInfo.menu.y = 0;
    s_performanceOptionsInfo.menu.name = "Performance##SettingsMenu";

    s_performanceOptionsInfo.tabs.generic.type = MTYPE_TAB;
    s_performanceOptionsInfo.tabs.generic.name = "##SettingsMenuTabs";
    s_performanceOptionsInfo.tabs.generic.eventcallback = PerformanceSettingsMenu_EventCallback;
    s_performanceOptionsInfo.tabs.numitems = ID_TABLE;
    s_performanceOptionsInfo.tabs.items[0] = (menucommon_t *)&s_performanceOptionsInfo.video;
    s_performanceOptionsInfo.tabs.items[1] = (menucommon_t *)&s_performanceOptionsInfo.performance;
    s_performanceOptionsInfo.tabs.items[2] = (menucommon_t *)&s_performanceOptionsInfo.audio;
    s_performanceOptionsInfo.tabs.items[3] = (menucommon_t *)&s_performanceOptionsInfo.controls;
    s_performanceOptionsInfo.tabs.items[4] = (menucommon_t *)&s_performanceOptionsInfo.gameplay;
}

void UI_PerformanceSettingsMenu( void )
{
    PerformanceSettingsMenu_Cache();
    UI_PushMenu( &s_performanceOptionsInfo.menu );
}
