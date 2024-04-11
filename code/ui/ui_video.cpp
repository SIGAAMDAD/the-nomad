#include "ui_lib.h"

#define ID_VIDEO        0
#define ID_PERFORMANCE  1
#define ID_AUDIO        2
#define ID_CONTROLS     3
#define ID_GAMEPLAY     4
#define ID_TABLE        5
#define ID_SETDEFAULTS  6
#define ID_SAVECONFIG   7

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

static videoOptionsInfo_t *s_videoOptionsInfo;
extern menutab_t settingsTabs;
extern menubutton_t settingsResetDefaults;
extern menubutton_t settingsSave;

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
        UI_ControlsSettingsMenu();
        break;
    case ID_GAMEPLAY:
        UI_PopMenu();
        UI_GameplaySettingsMenu();
        break;
    default:
        break;
    };
}

void VideoSettingsMenu_Save( void )
{
}

void VideoSettingsMenu_SetDefaults( void )
{
}

void VideoSettingsMenu_Cache( void )
{
    if ( !ui->uiAllocated ) {
        s_videoOptionsInfo = (videoOptionsInfo_t *)Hunk_Alloc( sizeof( *s_videoOptionsInfo ), h_high );
    }

    memset( s_videoOptionsInfo, 0, sizeof( *s_videoOptionsInfo ) );

    s_videoOptionsInfo->menu.fullscreen = qtrue;
    s_videoOptionsInfo->menu.width = ui->gpuConfig.vidWidth;
    s_videoOptionsInfo->menu.height = ui->gpuConfig.vidHeight;
    s_videoOptionsInfo->menu.x = 0;
    s_videoOptionsInfo->menu.y = 0;
    s_videoOptionsInfo->menu.name = "Video##SettingsMenu";

    Menu_AddItem( &s_videoOptionsInfo->menu, &settingsTabs );

    Menu_AddItem( &s_videoOptionsInfo->menu, &settingsResetDefaults );
    Menu_AddItem( &s_videoOptionsInfo->menu, &settingsSave );
}

void UI_VideoSettingsMenu( void )
{
    VideoSettingsMenu_Cache();
    UI_PushMenu( &s_videoOptionsInfo->menu );
}
