#include "../game/g_game.h"
#include "ui_public.hpp"
#include "ui_menu.h"
#include "ui_lib.h"
#include "ui_window.h"
#include "../engine/n_allocator.h"
#include "ui_string_manager.h"
#include "ui_table.h"
#include "../rendergl/ngl.h"
#include "../rendercommon/imgui_impl_opengl3.h"
#include "../rendercommon/imgui.h"

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
} settings_t;

static settings_t *initial;
static settings_t *settings;
menutab_t settingsTabs;
menubutton_t settingsResetDefaults;
menubutton_t settingsSave;

static void SettingsMenu_EventCallback( void *ptr, int event )
{
	if ( event != EVENT_ACTIVATED ) {
		return;
	}

	switch ( ( (menucommon_t *)ptr )->id ) {
	case ID_SAVECONFIG:
		VideoSettingsMenu_Save();
		PerformanceSettingsMenu_Save();
		AudioSettingsMenu_Save();
		ControlsSettingsMenu_Save();
		GameplaySettingsMenu_Save();
		break;
	case ID_SETDEFAULTS:
		VideoSettingsMenu_SetDefaults();
		PerformanceSettingsMenu_SetDefaults();
		AudioSettingsMenu_SetDefaults();
		ControlsSettingsMenu_SetDefaults();
		GameplaySettingsMenu_SetDefaults();
		break;
	};
}

static void SettingsMenu_InitPresets( void )
{
}

void SettingsMenu_Cache( void )
{
	if ( !ui->uiAllocated ) {
		settings = (settings_t *)Hunk_Alloc( sizeof( *settings ), h_high );
		initial = (settings_t *)Hunk_Alloc( sizeof( *initial ), h_high );
	}
	memset( settings, 0, sizeof( *settings ) );
	memset( initial, 0, sizeof( *initial ) );
	
	settings->menu.fullscreen = qtrue;
	settings->menu.x = 0;
	settings->menu.y = 0;
	settings->menu.width = ui->gpuConfig.vidWidth;
	settings->menu.height = ui->gpuConfig.vidHeight;
	settings->menu.name = "Settings##SettingsMenu";
	settings->menu.titleFontScale = 3.5f;
	settings->menu.textFontScale = 1.5f;

	settingsTabs.generic.type = MTYPE_TAB;
	settingsTabs.generic.font = FontCache()->AddFontToCache( "AlegreyaSC", "Bold" );
	settingsTabs.numitems = ID_TABLE;
	settingsTabs.items[0] = (menucommon_t *)&settings->video;
	settingsTabs.items[1] = (menucommon_t *)&settings->performance;
	settingsTabs.items[2] = (menucommon_t *)&settings->audio;
	settingsTabs.items[3] = (menucommon_t *)&settings->controls;
	settingsTabs.items[4] = (menucommon_t *)&settings->gameplay;

	settingsSave.generic.type = MTYPE_BUTTON;
	settingsSave.generic.id = ID_SAVECONFIG;
	settingsSave.generic.eventcallback = SettingsMenu_EventCallback;
	settingsSave.generic.flags = QMF_HIGHLIGHT_IF_FOCUS;
	settingsSave.generic.name = "##SettingsMenuSaveConfig";
	settingsSave.generic.font = FontCache()->AddFontToCache( "RobotoMono", "Bold" );

	settingsResetDefaults.generic.type = MTYPE_BUTTON;
	settingsResetDefaults.generic.id = ID_SETDEFAULTS;
	settingsResetDefaults.generic.eventcallback = SettingsMenu_EventCallback;
	settingsResetDefaults.generic.flags = QMF_HIGHLIGHT_IF_FOCUS;
	settingsResetDefaults.generic.name = "##SettingsMenuSetDefaultsConfig";
	settingsResetDefaults.generic.font = FontCache()->AddFontToCache( "RobotoMono", "Bold" );

	VideoSettingsMenu_Cache();
	PerformanceSettingsMenu_Cache();
	AudioSettingsMenu_Cache();
	ControlsSettingsMenu_Cache();
	GameplaySettingsMenu_Cache();
}

void UI_SettingsMenu( void )
{
	UI_VideoSettingsMenu();
}
