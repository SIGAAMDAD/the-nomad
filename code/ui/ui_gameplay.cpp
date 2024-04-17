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
    menubutton_t save;
    menubutton_t setDefaults;

    menutext_t difficulty;
    menutext_t debug;
} gameplayOptionsInfo_t;

static gameplayOptionsInfo_t *s_gameplayOptionsInfo;

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
        UI_ControlsSettingsMenu();
        break;
    case ID_GAMEPLAY:
        break;
    default:
        break;
    };
}

void GameplaySettingsMenu_Save( void )
{
}

void GameplaySettingsMenu_SetDefaults( void )
{
}

void GameplaySettingsMenu_Cache( void )
{
    if ( !ui->uiAllocated ) {
        s_gameplayOptionsInfo = (gameplayOptionsInfo_t *)Hunk_Alloc( sizeof( *s_gameplayOptionsInfo ), h_high );
    }
    memset( s_gameplayOptionsInfo, 0, sizeof( *s_gameplayOptionsInfo ) );

    s_gameplayOptionsInfo->video.generic.type = MTYPE_TEXT;
    s_gameplayOptionsInfo->video.generic.id = ID_VIDEO;
    s_gameplayOptionsInfo->video.generic.eventcallback = GameplaySettingsMenu_EventCallback;
    s_gameplayOptionsInfo->video.generic.font = AlegreyaSC;
    s_gameplayOptionsInfo->video.text = "Video##VideoSettingsMenuTabBar";
    s_gameplayOptionsInfo->video.color = color_white;
    
    s_gameplayOptionsInfo->performance.generic.type = MTYPE_TEXT;
    s_gameplayOptionsInfo->performance.generic.id = ID_PERFORMANCE;
    s_gameplayOptionsInfo->performance.generic.eventcallback = GameplaySettingsMenu_EventCallback;
    s_gameplayOptionsInfo->performance.generic.font = AlegreyaSC;
    s_gameplayOptionsInfo->performance.text = "Performance##PerformanceSettingsMenuTabBar";
    s_gameplayOptionsInfo->performance.color = color_white;
    
    s_gameplayOptionsInfo->audio.generic.type = MTYPE_TEXT;
    s_gameplayOptionsInfo->audio.generic.id = ID_AUDIO;
    s_gameplayOptionsInfo->audio.generic.eventcallback = GameplaySettingsMenu_EventCallback;
    s_gameplayOptionsInfo->audio.generic.font = AlegreyaSC;
    s_gameplayOptionsInfo->audio.text = "Audio##AudioSettingsMenuTabBar";
    s_gameplayOptionsInfo->audio.color = color_white;
    
    s_gameplayOptionsInfo->controls.generic.type = MTYPE_TEXT;
    s_gameplayOptionsInfo->controls.generic.id = ID_CONTROLS;
    s_gameplayOptionsInfo->controls.generic.eventcallback = GameplaySettingsMenu_EventCallback;
    s_gameplayOptionsInfo->controls.generic.font = AlegreyaSC;
    s_gameplayOptionsInfo->controls.text = "Controls##ControlsSettingsMenuTabBar";
    s_gameplayOptionsInfo->controls.color = color_white;
    
    s_gameplayOptionsInfo->gameplay.generic.type = MTYPE_TEXT;
    s_gameplayOptionsInfo->gameplay.generic.id = ID_GAMEPLAY;
    s_gameplayOptionsInfo->gameplay.generic.eventcallback = GameplaySettingsMenu_EventCallback;
    s_gameplayOptionsInfo->gameplay.generic.font = AlegreyaSC;
    s_gameplayOptionsInfo->gameplay.text = "Gameplay##GameplaySettingsMenuTabBar";
    s_gameplayOptionsInfo->gameplay.color = color_white;

    s_gameplayOptionsInfo->menu.fullscreen = qtrue;
    s_gameplayOptionsInfo->menu.width = ui->gpuConfig.vidWidth;
    s_gameplayOptionsInfo->menu.height = ui->gpuConfig.vidHeight;
    s_gameplayOptionsInfo->menu.textFontScale = 1.5f;
    s_gameplayOptionsInfo->menu.titleFontScale = 3.5f;
    s_gameplayOptionsInfo->menu.flags = MENU_DEFAULT_FLAGS;
    s_gameplayOptionsInfo->menu.x = 0;
    s_gameplayOptionsInfo->menu.y = 0;
    s_gameplayOptionsInfo->menu.name = "Gameplay";

    Menu_AddItem( &s_gameplayOptionsInfo->menu, &s_gameplayOptionsInfo->tabs );

    Menu_AddItem( &s_gameplayOptionsInfo->menu, &s_gameplayOptionsInfo->setDefaults );
    Menu_AddItem( &s_gameplayOptionsInfo->menu, &s_gameplayOptionsInfo->save );
}

void UI_GameplaySettingsMenu( void )
{
    GameplaySettingsMenu_Cache();
    UI_PushMenu( &s_gameplayOptionsInfo->menu );
}
