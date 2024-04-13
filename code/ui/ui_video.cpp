#include "ui_lib.h"

#define ID_VIDEO        0
#define ID_PERFORMANCE  1
#define ID_AUDIO        2
#define ID_CONTROLS     3
#define ID_GAMEPLAY     4
#define ID_TABLE        5
#define ID_SETDEFAULTS  6
#define ID_SAVECONFIG   7

#define ID_WINDOWMODE   0
#define ID_WINDOWSIZE   1
#define ID_VSYNC        2
#define ID_GAMMA        3
#define ID_EXPOSURE     4
#define ID_WINDOWWIDTH  5
#define ID_WINDOWHEIGHT 6

typedef struct {
    menuframework_t menu;

    menutext_t video;
    menutext_t performance;
    menutext_t audio;
    menutext_t controls;
    menutext_t gameplay;

    menutab_t tabs;
    menutable_t table;

    menutext_t gamma;
    menutext_t windowMode;
    menutext_t windowWidth;
    menutext_t windowHeight;
    menutext_t windowSize;
    menutext_t vsync;
    menutext_t exposure;

    menubutton_t save;
    menubutton_t setDefaults;

    menufield_t windowWidthInput;
    menufield_t windowHeightInput;
    
    menulist_t windowModeList;
    menulist_t windowSizeList;
    
    menuslider_t gammaSlider;
    menuslider_t exposureSlider;
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

static void VideoMenu_SetHint( void *ptr, int event )
{
    if ( event != EVENT_ACTIVATED ) {
        return;
    }
}

void VideoSettingsMenu_Save( void )
{
}

void VideoSettingsMenu_SetDefaults( void )
{
}

void VideoSettingsMenu_Cache( void )
{
    static vec4_t tabColor = { 1.0f, 1.0f, 1.0f, 0.0f };
    static vec4_t tabColorActive = { 0.0f, 1.0f, 0.0f, 1.0f };
    static vec4_t tabColorFocused = { 0.0f, 1.0f, 0.0f, 1.0f };

	static const char *windowModes[] = {
		"Windowed",
		"Fullscreen",
		"Borderless Windowed",
		"Borderless Fullscreen"
	};
	static const char *windowSizes[] = {
		"Custom Resolution",
		"Native Resolution",
		"1024x768",
        "1280x720",
        "1600x900",
        "1920x1080",
        "2048x1536",
        "3840x2160"
	};
	static const char *vsync[] = {
		"Adaptive",
		"Disabled",
		"Enabled"
	};

    if ( !ui->uiAllocated ) {
        s_videoOptionsInfo = (videoOptionsInfo_t *)Hunk_Alloc( sizeof( *s_videoOptionsInfo ), h_high );
    }

    memset( s_videoOptionsInfo, 0, sizeof( *s_videoOptionsInfo ) );

    s_videoOptionsInfo->video.generic.type = MTYPE_TEXT;
    s_videoOptionsInfo->video.generic.id = ID_VIDEO;
    s_videoOptionsInfo->video.generic.eventcallback = VideoSettingsMenu_EventCallback;
    s_videoOptionsInfo->video.generic.font = AlegreyaSC;
    s_videoOptionsInfo->video.text = "Video";
    s_videoOptionsInfo->video.color = color_white;
    
    s_videoOptionsInfo->performance.generic.type = MTYPE_TEXT;
    s_videoOptionsInfo->performance.generic.id = ID_PERFORMANCE;
    s_videoOptionsInfo->performance.generic.eventcallback = VideoSettingsMenu_EventCallback;
    s_videoOptionsInfo->performance.generic.font = AlegreyaSC;
    s_videoOptionsInfo->performance.text = "Performance";
    s_videoOptionsInfo->performance.color = color_white;
    
    s_videoOptionsInfo->audio.generic.type = MTYPE_TEXT;
    s_videoOptionsInfo->audio.generic.id = ID_AUDIO;
    s_videoOptionsInfo->audio.generic.eventcallback = VideoSettingsMenu_EventCallback;
    s_videoOptionsInfo->audio.generic.font = AlegreyaSC;
    s_videoOptionsInfo->audio.text = "Audio";
    s_videoOptionsInfo->audio.color = color_white;
    
    s_videoOptionsInfo->controls.generic.type = MTYPE_TEXT;
    s_videoOptionsInfo->controls.generic.id = ID_CONTROLS;
    s_videoOptionsInfo->controls.generic.eventcallback = VideoSettingsMenu_EventCallback;
    s_videoOptionsInfo->controls.generic.font = AlegreyaSC;
    s_videoOptionsInfo->controls.text = "Controls";
    s_videoOptionsInfo->controls.color = color_white;
    
    s_videoOptionsInfo->gameplay.generic.type = MTYPE_TEXT;
    s_videoOptionsInfo->gameplay.generic.id = ID_GAMEPLAY;
    s_videoOptionsInfo->gameplay.generic.eventcallback = VideoSettingsMenu_EventCallback;
    s_videoOptionsInfo->gameplay.generic.font = AlegreyaSC;
    s_videoOptionsInfo->gameplay.text = "Gameplay";
    s_videoOptionsInfo->gameplay.color = color_white;

    s_videoOptionsInfo->menu.fullscreen = qtrue;
    s_videoOptionsInfo->menu.flags = MENU_DEFAULT_FLAGS;
    s_videoOptionsInfo->menu.width = ui->gpuConfig.vidWidth * 0.75f;
    s_videoOptionsInfo->menu.height = ui->gpuConfig.vidHeight;
    s_videoOptionsInfo->menu.titleFontScale = 3.5f;
    s_videoOptionsInfo->menu.textFontScale = 1.5f;
    s_videoOptionsInfo->menu.x = 0;
    s_videoOptionsInfo->menu.y = 0;
    s_videoOptionsInfo->menu.name = "Video";

    s_videoOptionsInfo->tabs.generic.type = MTYPE_TAB;
    s_videoOptionsInfo->tabs.generic.name = "##SettingsMenuTabBar";
	s_videoOptionsInfo->tabs.generic.font = FontCache()->AddFontToCache( "AlegreyaSC", "Bold" );
    s_videoOptionsInfo->tabs.tabColor = tabColor;
    s_videoOptionsInfo->tabs.tabColorActive = tabColorActive;
    s_videoOptionsInfo->tabs.tabColorFocused = tabColorFocused;
	s_videoOptionsInfo->tabs.numitems = ID_TABLE;
	s_videoOptionsInfo->tabs.items[0] = (menucommon_t *)&s_videoOptionsInfo->video;
	s_videoOptionsInfo->tabs.items[1] = (menucommon_t *)&s_videoOptionsInfo->performance;
	s_videoOptionsInfo->tabs.items[2] = (menucommon_t *)&s_videoOptionsInfo->audio;
	s_videoOptionsInfo->tabs.items[3] = (menucommon_t *)&s_videoOptionsInfo->controls;
	s_videoOptionsInfo->tabs.items[4] = (menucommon_t *)&s_videoOptionsInfo->gameplay;

	s_videoOptionsInfo->save.generic.type = MTYPE_BUTTON;
	s_videoOptionsInfo->save.generic.id = ID_SAVECONFIG;
	s_videoOptionsInfo->save.generic.flags = QMF_HIGHLIGHT_IF_FOCUS;
	s_videoOptionsInfo->save.generic.name = "Save##SettingsMenuSaveConfig";
	s_videoOptionsInfo->save.generic.font = FontCache()->AddFontToCache( "RobotoMono", "Bold" );

	s_videoOptionsInfo->setDefaults.generic.type = MTYPE_BUTTON;
	s_videoOptionsInfo->setDefaults.generic.id = ID_SETDEFAULTS;
	s_videoOptionsInfo->setDefaults.generic.flags = QMF_HIGHLIGHT_IF_FOCUS;
	s_videoOptionsInfo->setDefaults.generic.name = "Set Defaults##SettingsMenuSetDefaultsConfig";
	s_videoOptionsInfo->setDefaults.generic.font = FontCache()->AddFontToCache( "RobotoMono", "Bold" );

    s_videoOptionsInfo->windowWidth.generic.type = MTYPE_TEXT;
    s_videoOptionsInfo->windowWidth.generic.id = ID_WINDOWWIDTH;
    s_videoOptionsInfo->windowWidth.generic.flags = QMF_HIGHLIGHT_IF_FOCUS;
    s_videoOptionsInfo->windowWidth.generic.eventcallback = VideoMenu_SetHint;
    s_videoOptionsInfo->windowWidth.generic.font = AlegreyaSC;
    s_videoOptionsInfo->windowWidth.text = "Window Width";
    s_videoOptionsInfo->windowWidth.color = color_white;
    
    s_videoOptionsInfo->windowHeight.generic.type = MTYPE_TEXT;
    s_videoOptionsInfo->windowHeight.generic.id = ID_WINDOWHEIGHT;
    s_videoOptionsInfo->windowHeight.generic.flags = QMF_HIGHLIGHT_IF_FOCUS;
    s_videoOptionsInfo->windowHeight.generic.eventcallback = VideoMenu_SetHint;
    s_videoOptionsInfo->windowHeight.generic.font = AlegreyaSC;
    s_videoOptionsInfo->windowHeight.text = "Window Height";
    s_videoOptionsInfo->windowHeight.color = color_white;
    
    s_videoOptionsInfo->windowMode.generic.type = MTYPE_TEXT;
    s_videoOptionsInfo->windowMode.generic.id = ID_WINDOWMODE;
    s_videoOptionsInfo->windowMode.generic.flags = QMF_HIGHLIGHT_IF_FOCUS;
    s_videoOptionsInfo->windowMode.generic.eventcallback = VideoMenu_SetHint;
    s_videoOptionsInfo->windowMode.generic.font = AlegreyaSC;
    s_videoOptionsInfo->windowMode.text = "Window Mode";
    s_videoOptionsInfo->windowMode.color = color_white;
    
    s_videoOptionsInfo->windowSize.generic.type = MTYPE_TEXT;
    s_videoOptionsInfo->windowSize.generic.id = ID_WINDOWSIZE;
    s_videoOptionsInfo->windowSize.generic.flags = QMF_HIGHLIGHT_IF_FOCUS;
    s_videoOptionsInfo->windowSize.generic.eventcallback = VideoMenu_SetHint;
    s_videoOptionsInfo->windowSize.generic.font = AlegreyaSC;
    s_videoOptionsInfo->windowSize.text = "Window Size";
    s_videoOptionsInfo->windowSize.color = color_white;
    
    s_videoOptionsInfo->vsync.generic.type = MTYPE_TEXT;
    s_videoOptionsInfo->vsync.generic.id = ID_VSYNC;
    s_videoOptionsInfo->vsync.generic.flags = QMF_HIGHLIGHT_IF_FOCUS;
    s_videoOptionsInfo->vsync.generic.eventcallback = VideoMenu_SetHint;
    s_videoOptionsInfo->vsync.generic.font = AlegreyaSC;
    s_videoOptionsInfo->vsync.text = "VSync";
    s_videoOptionsInfo->vsync.color = color_white;
    
    s_videoOptionsInfo->gamma.generic.type = MTYPE_TEXT;
    s_videoOptionsInfo->gamma.generic.id = ID_GAMMA;
    s_videoOptionsInfo->gamma.generic.flags = QMF_HIGHLIGHT_IF_FOCUS;
    s_videoOptionsInfo->gamma.generic.eventcallback = VideoMenu_SetHint;
    s_videoOptionsInfo->gamma.generic.font = AlegreyaSC;
    s_videoOptionsInfo->gamma.text = "Gamma";
    s_videoOptionsInfo->gamma.color = color_white;
    
    s_videoOptionsInfo->exposure.generic.type = MTYPE_TEXT;
    s_videoOptionsInfo->exposure.generic.id = ID_EXPOSURE;
    s_videoOptionsInfo->exposure.generic.flags = QMF_HIGHLIGHT_IF_FOCUS;
    s_videoOptionsInfo->exposure.generic.eventcallback = VideoMenu_SetHint;
    s_videoOptionsInfo->exposure.generic.font = AlegreyaSC;
    s_videoOptionsInfo->exposure.text = "Exposure";
    s_videoOptionsInfo->exposure.color = color_white;
    
    s_videoOptionsInfo->windowModeList.generic.type = MTYPE_LIST;
    s_videoOptionsInfo->windowModeList.generic.id = ID_WINDOWMODE;
    s_videoOptionsInfo->windowModeList.generic.name = "##WindowModeVideoSettingsMenuConfigList";
    s_videoOptionsInfo->windowModeList.generic.font = RobotoMono;
    s_videoOptionsInfo->windowModeList.numitems = arraylen( windowModes );
    s_videoOptionsInfo->windowModeList.itemnames = windowModes;
    s_videoOptionsInfo->windowModeList.curitem = Cvar_VariableInteger( "r_fullscreen" );
    if ( Cvar_VariableInteger( "r_noborder" ) ) {
    	s_videoOptionsInfo->windowModeList.curitem += 2;
    }
    
    s_videoOptionsInfo->windowSizeList.generic.type = MTYPE_LIST;
    s_videoOptionsInfo->windowSizeList.generic.id = ID_WINDOWSIZE;
    s_videoOptionsInfo->windowSizeList.generic.name = "##WindowSizeVideoSettingsMenuConfigList";
    s_videoOptionsInfo->windowSizeList.generic.font = RobotoMono;
    s_videoOptionsInfo->windowSizeList.numitems = arraylen( windowSizes );
    s_videoOptionsInfo->windowSizeList.itemnames = windowSizes;
    s_videoOptionsInfo->windowSizeList.curitem = Cvar_VariableInteger( "r_mode" );
    if ( s_videoOptionsInfo->windowSizeList.curitem < 0 ) {
    	// custom or desktop resolution
    	s_videoOptionsInfo->windowSizeList.curitem += 2;
    }
    
    s_videoOptionsInfo->windowWidthInput.generic.type = MTYPE_FIELD;
    s_videoOptionsInfo->windowWidthInput.generic.id = ID_WINDOWWIDTH;
    s_videoOptionsInfo->windowWidthInput.generic.flags = QMF_NUMBERSONLY;
    s_videoOptionsInfo->windowWidthInput.generic.name = "##WindowWidthInputVideoSettingsMenuConfigInput";
    s_videoOptionsInfo->windowWidthInput.generic.font = RobotoMono;
    s_videoOptionsInfo->windowWidthInput.maxchars = 24;
    s_videoOptionsInfo->windowWidthInput.color = color_white;
    
    s_videoOptionsInfo->windowHeightInput.generic.type = MTYPE_FIELD;
    s_videoOptionsInfo->windowHeightInput.generic.id = ID_WINDOWHEIGHT;
    s_videoOptionsInfo->windowHeightInput.generic.flags = QMF_NUMBERSONLY;
    s_videoOptionsInfo->windowHeightInput.generic.name = "##WindowHeightInputVideoSettingsMenuConfigInput";
    s_videoOptionsInfo->windowHeightInput.generic.font = RobotoMono;
    s_videoOptionsInfo->windowHeightInput.maxchars = 24;
    s_videoOptionsInfo->windowHeightInput.color = color_white;

    s_videoOptionsInfo->table.generic.type = MTYPE_TABLE;
    s_videoOptionsInfo->table.generic.name = "##VideoSettingsMenuConfigTable";
    s_videoOptionsInfo->table.columns = 2;

    Menu_AddItem( &s_videoOptionsInfo->menu, &s_videoOptionsInfo->tabs );
    Menu_AddItem( &s_videoOptionsInfo->menu, &s_videoOptionsInfo->table );

    Table_AddRow( &s_videoOptionsInfo->table );
    Table_AddItem( &s_videoOptionsInfo->table, &s_videoOptionsInfo->windowMode );
//    Table_AddItem( &s_videoOptionsInfo->table, &s_videoOptionsInfo->windowModeLeft );
    Table_AddItem( &s_videoOptionsInfo->table, &s_videoOptionsInfo->windowModeList );
//    Table_AddItem( &s_videoOptionsInfo->table, &s_videoOptionsInfo->windowModeRight );

    Table_AddRow( &s_videoOptionsInfo->table );
    Table_AddItem( &s_videoOptionsInfo->table, &s_videoOptionsInfo->windowSize );
//    Table_AddItem( &s_videoOptionsInfo->table, &s_videoOptionsInfo->windowSizeLeft );
    Table_AddItem( &s_videoOptionsInfo->table, &s_videoOptionsInfo->windowSizeList );
//    Table_AddItem( &s_videoOptionsInfo->table, &s_videoOptionsInfo->windowSizeRight );

//    Menu_AddItem( &s_videoOptionsInfo->menu, &s_videoOptionsInfo->setDefaults );
//    Menu_AddItem( &s_videoOptionsInfo->menu, &s_videoOptionsInfo->save );
}

void UI_VideoSettingsMenu( void )
{
    UI_PushMenu( &s_videoOptionsInfo->menu );
}
