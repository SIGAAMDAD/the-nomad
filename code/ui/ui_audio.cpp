#include "ui_lib.h"

#define ID_VIDEO        0
#define ID_PERFORMANCE  1
#define ID_AUDIO        2
#define ID_CONTROLS     3
#define ID_GAMEPLAY     4
#define ID_TABLE        5
#define ID_SETDEFAULTS  6
#define ID_SAVECONFIG   7

#define ID_SFXON        0
#define ID_MUSICON      1
#define ID_SFXVOLUME    2
#define ID_MUSICVOLUME  3
#define ID_MASTERVOLUME 4

typedef struct {
    menuframework_t menu;

    menutext_t video;
    menutext_t performance;
    menutext_t audio;
    menutext_t controls;
    menutext_t gameplay;

    menutab_t tabs;
    menutable_t table;

    menuswitch_t sfxOn;
    menuswitch_t musicOn;
    menuslider_t sfxVol;
    menuslider_t musicVol;
    menuslider_t masterVol;

    menutext_t mastervol;
    menutext_t sfxvol;
    menutext_t musicvol;
    menutext_t musicon;
    menutext_t sfxon;
} audioOptionsInfo_t;

static audioOptionsInfo_t s_audioOptionsInfo;

static void AudioSettingsMenu_EventCallback( void *ptr, int event )
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

void AudioSettingsMenu_Save( void )
{
    Cvar_SetIntegerValue( "snd_sfxon", s_audioOptionsInfo.sfxOn.curvalue );
    Cvar_SetIntegerValue( "snd_musicon", s_audioOptionsInfo.musicOn.curvalue );
    Cvar_SetIntegerValue( "snd_sfxvol", s_audioOptionsInfo.sfxVol.curvalue );
    Cvar_SetIntegerValue( "snd_musicvol", s_audioOptionsInfo.musicVol.curvalue );
    Cvar_SetIntegerValue( "snd_mastervol", s_audioOptionsInfo.masterVol.curvalue );
}

void AudioSettingsMenu_SetDefaults( void )
{
}

void AudioSettingsMenu_Cache( void )
{
    memset( &s_audioOptionsInfo, 0, sizeof( s_audioOptionsInfo ) );

    s_audioOptionsInfo.menu.fullscreen = qtrue;
    s_audioOptionsInfo.menu.width = ui->gpuConfig.vidWidth;
    s_audioOptionsInfo.menu.height = ui->gpuConfig.vidHeight;
    s_audioOptionsInfo.menu.x = 0;
    s_audioOptionsInfo.menu.y = 0;
    s_audioOptionsInfo.menu.name = "Audio##SettingsMenu";
    s_audioOptionsInfo.menu.flags = MENU_DEFAULT_FLAGS;
    s_audioOptionsInfo.menu.textFontScale = 1.5f;
    s_audioOptionsInfo.menu.titleFontScale = 3.5f;

    s_audioOptionsInfo.tabs.generic.type = MTYPE_TAB;
    s_audioOptionsInfo.tabs.generic.name = "##SettingsMenuTabs";
    s_audioOptionsInfo.tabs.generic.eventcallback = AudioSettingsMenu_EventCallback;
    s_audioOptionsInfo.tabs.numitems = ID_TABLE;
    s_audioOptionsInfo.tabs.items[0] = (menucommon_t *)&s_audioOptionsInfo.video;
    s_audioOptionsInfo.tabs.items[1] = (menucommon_t *)&s_audioOptionsInfo.performance;
    s_audioOptionsInfo.tabs.items[2] = (menucommon_t *)&s_audioOptionsInfo.audio;
    s_audioOptionsInfo.tabs.items[3] = (menucommon_t *)&s_audioOptionsInfo.controls;
    s_audioOptionsInfo.tabs.items[4] = (menucommon_t *)&s_audioOptionsInfo.gameplay;

    s_audioOptionsInfo.mastervol.generic.type = MTYPE_TEXT;
    s_audioOptionsInfo.mastervol.generic.id = ID_MASTERVOLUME;
    s_audioOptionsInfo.mastervol.generic.flags = QMF_HIGHLIGHT_IF_FOCUS;
    s_audioOptionsInfo.mastervol.text = "Master Volume";
    s_audioOptionsInfo.mastervol.color = color_white;

    s_audioOptionsInfo.musicvol.generic.type = MTYPE_TEXT;
    s_audioOptionsInfo.musicvol.generic.id = ID_MUSICVOLUME;
    s_audioOptionsInfo.musicvol.generic.flags = QMF_HIGHLIGHT_IF_FOCUS;
    s_audioOptionsInfo.musicvol.text = "Music Volume";
    s_audioOptionsInfo.musicvol.color = color_white;

    s_audioOptionsInfo.sfxvol.generic.type = MTYPE_TEXT;
    s_audioOptionsInfo.sfxvol.generic.id = ID_SFXVOLUME;
    s_audioOptionsInfo.sfxvol.generic.flags = QMF_HIGHLIGHT_IF_FOCUS;
    s_audioOptionsInfo.sfxvol.text = "Sound Effects Volume";
    s_audioOptionsInfo.sfxvol.color = color_white;

    s_audioOptionsInfo.musicon.generic.type = MTYPE_TEXT;
    s_audioOptionsInfo.musicon.generic.id = ID_MUSICON;
    s_audioOptionsInfo.musicon.generic.flags = QMF_HIGHLIGHT_IF_FOCUS;
    s_audioOptionsInfo.musicon.text = "Music On";
    s_audioOptionsInfo.musicon.color = color_white;

    s_audioOptionsInfo.sfxon.generic.type = MTYPE_TEXT;
    s_audioOptionsInfo.sfxon.generic.id = ID_SFXON;
    s_audioOptionsInfo.sfxon.generic.flags = QMF_HIGHLIGHT_IF_FOCUS;
    s_audioOptionsInfo.sfxon.text = "Sfx On";
    s_audioOptionsInfo.sfxon.color = color_white;
    
    s_audioOptionsInfo.sfxOn.generic.type = MTYPE_RADIOBUTTON;
    s_audioOptionsInfo.sfxOn.generic.id = ID_SFXON;
    s_audioOptionsInfo.sfxOn.generic.name = "##SoundEffectsAudioSettingsMenuConfigButton";
    s_audioOptionsInfo.sfxOn.curvalue = Cvar_VariableInteger( "snd_sfxon" );
    s_audioOptionsInfo.sfxOn.color = color_white;

    s_audioOptionsInfo.musicOn.generic.type = MTYPE_RADIOBUTTON;
    s_audioOptionsInfo.musicOn.generic.id = ID_MUSICON;
    s_audioOptionsInfo.musicOn.generic.name = "##MusicAudioSettingsMenuConfigButton";
    s_audioOptionsInfo.musicOn.curvalue = Cvar_VariableInteger( "snd_musicon" );
    s_audioOptionsInfo.musicOn.color = color_white;

    s_audioOptionsInfo.musicVol.generic.type = MTYPE_SLIDER;
    s_audioOptionsInfo.musicVol.generic.id = ID_MUSICVOLUME;
    s_audioOptionsInfo.musicVol.generic.name = "##MusicAudioSettingsMenuConfigSlider";
    s_audioOptionsInfo.musicVol.minvalue = 0;
    s_audioOptionsInfo.musicVol.maxvalue = 100;
    s_audioOptionsInfo.musicVol.curvalue = Cvar_VariableFloat( "snd_musicvol" );
    s_audioOptionsInfo.musicVol.isIntegral = qtrue;

    s_audioOptionsInfo.sfxVol.generic.type = MTYPE_SLIDER;
    s_audioOptionsInfo.sfxVol.generic.id = ID_SFXVOLUME;
    s_audioOptionsInfo.sfxVol.generic.name = "##SoundEffectsAudioSettingsMenuConfigSlider";
    s_audioOptionsInfo.sfxVol.minvalue = 0;
    s_audioOptionsInfo.sfxVol.maxvalue = 100;
    s_audioOptionsInfo.sfxVol.curvalue = Cvar_VariableFloat( "snd_sfxvol" );
    s_audioOptionsInfo.sfxVol.isIntegral = qtrue;

    s_audioOptionsInfo.masterVol.generic.type = MTYPE_SLIDER;
    s_audioOptionsInfo.masterVol.generic.id = ID_MASTERVOLUME;
    s_audioOptionsInfo.masterVol.generic.name = "##MasterAudioSettingsMenuConfigSlider";
    s_audioOptionsInfo.masterVol.minvalue = 0;
    s_audioOptionsInfo.masterVol.maxvalue = 100;
    s_audioOptionsInfo.masterVol.curvalue = Cvar_VariableFloat( "snd_mastervol" );
    s_audioOptionsInfo.masterVol.isIntegral = qtrue;

    s_audioOptionsInfo.table.columns = 2;
    s_audioOptionsInfo.table.generic.type = MTYPE_TABLE;
    s_audioOptionsInfo.table.generic.id = ID_TABLE;
    s_audioOptionsInfo.table.generic.name = "##AudioSettingsMenuConfigTable";

    Menu_AddItem( &s_audioOptionsInfo.menu, &s_audioOptionsInfo.tabs );
    Menu_AddItem( &s_audioOptionsInfo.menu, &s_audioOptionsInfo.table );

    Table_AddRow( &s_audioOptionsInfo.table );
    Table_AddItem( &s_audioOptionsInfo.table, &s_audioOptionsInfo.mastervol );
    Table_AddItem( &s_audioOptionsInfo.table, &s_audioOptionsInfo.masterVol );

    Table_AddRow( &s_audioOptionsInfo.table );
    Table_AddItem( &s_audioOptionsInfo.table, &s_audioOptionsInfo.musicvol );
    Table_AddItem( &s_audioOptionsInfo.table, &s_audioOptionsInfo.musicVol );

    Table_AddRow( &s_audioOptionsInfo.table );
    Table_AddItem( &s_audioOptionsInfo.table, &s_audioOptionsInfo.sfxvol );
    Table_AddItem( &s_audioOptionsInfo.table, &s_audioOptionsInfo.sfxVol );

    Table_AddRow( &s_audioOptionsInfo.table );
    Table_AddItem( &s_audioOptionsInfo.table, &s_audioOptionsInfo.musicon );
    Table_AddItem( &s_audioOptionsInfo.table, &s_audioOptionsInfo.musicOn );

    Table_AddRow( &s_audioOptionsInfo.table );
    Table_AddItem( &s_audioOptionsInfo.table, &s_audioOptionsInfo.sfxon );
    Table_AddItem( &s_audioOptionsInfo.table, &s_audioOptionsInfo.sfxOn );
}

void UI_AudioSettingsMenu( void )
{
    AudioSettingsMenu_Cache();
    UI_PushMenu( &s_audioOptionsInfo.menu );
}
