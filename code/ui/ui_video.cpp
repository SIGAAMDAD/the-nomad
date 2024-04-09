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
} videoOptionsInfo_t;

static videoOptionsInfo_t s_videoOptionsInfo;

static void VideoSettingsMenu_EventCallback( void *ptr, int event )
{
    if ( event != EVENT_ACTIVATED ) {
        return;
    }

    switch ( ( (menucommon_t *)ptr )->id ) {
    case ID_VIDEO:
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
        UI_PopMenu();
        UI_GameplaySettingsMenu();
        break;
    default:
        break;
    };
}

void VideoSettingsMenu_Cache( void )
{
    static const char *difficulties[] = {
        difficultyTable[ DIF_NOOB ].name,
        difficultyTable[ DIF_RECRUIT ].name,
        difficultyTable[ DIF_MERC ].name,
        difficultyTable[ DIF_NOMAD ].name,
        difficultyTable[ DIF_BLACKDEATH ].name,
        "Just A Minor Inconvenience"
    };

    memset( &s_videoOptionsInfo, 0, sizeof( s_videoOptionsInfo ) );

    s_videoOptionsInfo.menu.fullscreen = qtrue;
    s_videoOptionsInfo.menu.width = ui->gpuConfig.vidWidth;
    s_videoOptionsInfo.menu.height = ui->gpuConfig.vidHeight;
    s_videoOptionsInfo.menu.x = 0;
    s_videoOptionsInfo.menu.y = 0;
    s_videoOptionsInfo.menu.name = "Video##SettingsMenu";

    s_videoOptionsInfo.tabs.generic.type = MTYPE_TAB;
    s_videoOptionsInfo.tabs.generic.name = "##SettingsMenuTabs";
    s_videoOptionsInfo.tabs.generic.eventcallback = VideoSettingsMenu_EventCallback;
    s_videoOptionsInfo.tabs.numitems = ID_TABLE;
    s_videoOptionsInfo.tabs.items[0] = (menucommon_t *)&s_videoOptionsInfo.video;
    s_videoOptionsInfo.tabs.items[1] = (menucommon_t *)&s_videoOptionsInfo.performance;
    s_videoOptionsInfo.tabs.items[2] = (menucommon_t *)&s_videoOptionsInfo.audio;
    s_videoOptionsInfo.tabs.items[3] = (menucommon_t *)&s_videoOptionsInfo.controls;
    s_videoOptionsInfo.tabs.items[4] = (menucommon_t *)&s_videoOptionsInfo.gameplay;
}

void UI_VideoSettingsMenu( void )
{
    VideoSettingsMenu_Cache();
    UI_PushMenu( &s_videoOptionsInfo.menu );
}
