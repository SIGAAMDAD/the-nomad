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

    menubutton_t save;
    menubutton_t setDefaults;
} videoOptionsInfo_t;

static videoOptionsInfo_t *s_videoOptionsInfo;

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

    s_videoOptionsInfo->video.generic.type = MTYPE_TEXT;
    s_videoOptionsInfo->video.generic.id = ID_VIDEO;
    s_videoOptionsInfo->video.generic.eventcallback = VideoSettingsMenu_EventCallback;
    s_videoOptionsInfo->video.generic.font = AlegreyaSC;
    s_videoOptionsInfo->video.text = "Video##VideoSettingsMenuTabBar";
    s_videoOptionsInfo->video.color = color_white;
    
    s_videoOptionsInfo->performance.generic.type = MTYPE_TEXT;
    s_videoOptionsInfo->performance.generic.id = ID_PERFORMANCE;
    s_videoOptionsInfo->performance.generic.eventcallback = VideoSettingsMenu_EventCallback;
    s_videoOptionsInfo->performance.generic.font = AlegreyaSC;
    s_videoOptionsInfo->performance.text = "Performance##PerformanceSettingsMenuTabBar";
    s_videoOptionsInfo->performance.color = color_white;
    
    s_videoOptionsInfo->audio.generic.type = MTYPE_TEXT;
    s_videoOptionsInfo->audio.generic.id = ID_AUDIO;
    s_videoOptionsInfo->audio.generic.eventcallback = VideoSettingsMenu_EventCallback;
    s_videoOptionsInfo->audio.generic.font = AlegreyaSC;
    s_videoOptionsInfo->audio.text = "Audio##AudioSettingsMenuTabBar";
    s_videoOptionsInfo->audio.color = color_white;
    
    s_videoOptionsInfo->controls.generic.type = MTYPE_TEXT;
    s_videoOptionsInfo->controls.generic.id = ID_CONTROLS;
    s_videoOptionsInfo->controls.generic.eventcallback = VideoSettingsMenu_EventCallback;
    s_videoOptionsInfo->controls.generic.font = AlegreyaSC;
    s_videoOptionsInfo->controls.text = "Controls##ControlsSettingsMenuTabBar";
    s_videoOptionsInfo->controls.color = color_white;
    
    s_videoOptionsInfo->gameplay.generic.type = MTYPE_TEXT;
    s_videoOptionsInfo->gameplay.generic.id = ID_GAMEPLAY;
    s_videoOptionsInfo->gameplay.generic.eventcallback = VideoSettingsMenu_EventCallback;
    s_videoOptionsInfo->gameplay.generic.font = AlegreyaSC;
    s_videoOptionsInfo->gameplay.text = "Gameplay##GameplaySettingsMenuTabBar";
    s_videoOptionsInfo->gameplay.color = color_white;

    s_videoOptionsInfo->menu.fullscreen = qtrue;
    s_videoOptionsInfo->menu.flags = MENU_DEFAULT_FLAGS;
    s_videoOptionsInfo->menu.width = ui->gpuConfig.vidWidth;
    s_videoOptionsInfo->menu.height = ui->gpuConfig.vidHeight;
    s_videoOptionsInfo->menu.x = 0;
    s_videoOptionsInfo->menu.y = 0;
    s_videoOptionsInfo->menu.name = "Video";

    s_videoOptionsInfo->tabs.generic.type = MTYPE_TAB;
	s_videoOptionsInfo->tabs.generic.font = FontCache()->AddFontToCache( "AlegreyaSC", "Bold" );
	s_videoOptionsInfo->tabs.numitems = ID_TABLE;
	s_videoOptionsInfo->tabs.items[0] = (menucommon_t *)&s_videoOptionsInfo->video;
	s_videoOptionsInfo->tabs.items[1] = (menucommon_t *)&s_videoOptionsInfo->performance;
	s_videoOptionsInfo->tabs.items[2] = (menucommon_t *)&s_videoOptionsInfo->audio;
	s_videoOptionsInfo->tabs.items[3] = (menucommon_t *)&s_videoOptionsInfo->controls;
	s_videoOptionsInfo->tabs.items[4] = (menucommon_t *)&s_videoOptionsInfo->gameplay;

	s_videoOptionsInfo->save.generic.type = MTYPE_BUTTON;
	s_videoOptionsInfo->save.generic.id = ID_SAVECONFIG;
	s_videoOptionsInfo->save.generic.eventcallback = VideoSettingsMenu_EventCallback;
	s_videoOptionsInfo->save.generic.flags = QMF_HIGHLIGHT_IF_FOCUS;
	s_videoOptionsInfo->save.generic.name = "Save##SettingsMenuSaveConfig";
	s_videoOptionsInfo->save.generic.font = FontCache()->AddFontToCache( "RobotoMono", "Bold" );

	s_videoOptionsInfo->setDefaults.generic.type = MTYPE_BUTTON;
	s_videoOptionsInfo->setDefaults.generic.id = ID_SETDEFAULTS;
	s_videoOptionsInfo->setDefaults.generic.eventcallback = VideoSettingsMenu_EventCallback;
	s_videoOptionsInfo->setDefaults.generic.flags = QMF_HIGHLIGHT_IF_FOCUS;
	s_videoOptionsInfo->setDefaults.generic.name = "Set Defaults##SettingsMenuSetDefaultsConfig";
	s_videoOptionsInfo->setDefaults.generic.font = FontCache()->AddFontToCache( "RobotoMono", "Bold" );

    Menu_AddItem( &s_videoOptionsInfo->menu, &s_videoOptionsInfo->tabs );

    Menu_AddItem( &s_videoOptionsInfo->menu, &s_videoOptionsInfo->setDefaults );
    Menu_AddItem( &s_videoOptionsInfo->menu, &s_videoOptionsInfo->save );
}

void UI_VideoSettingsMenu( void )
{
    VideoSettingsMenu_Cache();
    UI_PushMenu( &s_videoOptionsInfo->menu );
}
