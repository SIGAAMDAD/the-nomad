#include "ui_lib.h"

#define MAX_DAILY_TIPS 1024

#define ID_TITLE       0
#define ID_HELP        1
#define ID_RESUME      2
#define ID_SETTINGS    3
#define ID_CHECKPOINT  4
#define ID_EXIT        5

typedef struct {
    menuframework_t menu;

    menutext_t help;
    menutext_t resume;
    menutext_t settings;
    menutext_t checkpoint;
    menutext_t exitToMainMenu;

    int64_t oldVolume;
} pausemenu_t;

#define PAUSEMENU_VOLUME_CAP 2

static const char *dailyTips[] = {
    "You can parry anything that's a projectile, that includes bullets, flying corpses, blades, etc.",
    "The grappling hook deals a little bit of damage every time it hooks onto an enemy",
};

// PAUSE. REWIND. PLAY.
static pausemenu_t s_pauseMenu;

static void PauseMenu_EventCallback( void *ptr, int event )
{
    if ( event != EVENT_ACTIVATED ) {
        return;
    }

    switch ( ( (menucommon_t *)ptr )->id ) {
    case ID_RESUME:
        UI_SetActiveMenu( UI_MENU_NONE );
        break;
    case ID_CHECKPOINT:
        // rewind the checkpoint
        Cbuf_ExecuteText( EXEC_APPEND, "sgame.rewind_to_last_checkpoint\n" );

        UI_SetActiveMenu( UI_MENU_NONE );
        break;
    case ID_HELP:
        break;
    case ID_SETTINGS:
        UI_SettingsMenu();
        break;
    case ID_EXIT:
        UI_SetActiveMenu( UI_MENU_MAIN );
        gi.mapLoaded = qfalse;
        g_pModuleLib->ModuleCall( sgvm, ModuleOnLevelEnd, 0 );
        Cbuf_ExecuteText( EXEC_APPEND, "unloadworld\n" );
        break;
    default:
        break;
    };
}

void PauseMenu_Cache( void )
{
    const stringHash_t *titleString;
    const stringHash_t *helpString;
    const stringHash_t *resumeString;
    const stringHash_t *settingsString;
    const stringHash_t *checkpointString;
    const stringHash_t *exitToMainMenuString;

    memset( &s_pauseMenu, 0, sizeof( s_pauseMenu ) );

    titleString = strManager->ValueForKey( "MENU_PAUSE_TITLE" );
    resumeString = strManager->ValueForKey( "MENU_PAUSE_RESUME" );
    checkpointString = strManager->ValueForKey( "MENU_PAUSE_CHECKPOINT" );
    exitToMainMenuString = strManager->ValueForKey( "MENU_PAUSE_ETMM" );
    helpString = strManager->ValueForKey( "MENU_PAUSE_HELP" );

    s_pauseMenu.menu.width = ui->gpuConfig.vidWidth;
    s_pauseMenu.menu.height = ui->gpuConfig.vidHeight;
    s_pauseMenu.menu.fullscreen = qfalse;
    s_pauseMenu.menu.name = titleString->value;
    s_pauseMenu.menu.titleFontScale = 3.5f;
    s_pauseMenu.menu.textFontScale = 1.5f;

    s_pauseMenu.resume.generic.type = MTYPE_TEXT;
    s_pauseMenu.resume.generic.id = ID_RESUME;
    s_pauseMenu.resume.generic.eventcallback = PauseMenu_EventCallback;
    s_pauseMenu.resume.generic.font = FontCache()->AddFontToCache( "RobotoMono", "Bold" );
    s_pauseMenu.resume.text = resumeString->value;
    s_pauseMenu.resume.color = color_white;

    s_pauseMenu.checkpoint.generic.type = MTYPE_TEXT;
    s_pauseMenu.checkpoint.generic.id = ID_CHECKPOINT;
    s_pauseMenu.checkpoint.generic.eventcallback = PauseMenu_EventCallback;
    s_pauseMenu.checkpoint.generic.font = FontCache()->AddFontToCache( "RobotoMono", "Bold" );
    s_pauseMenu.checkpoint.text = checkpointString->value;
    s_pauseMenu.checkpoint.color = color_white;

    s_pauseMenu.help.generic.type = MTYPE_TEXT;
    s_pauseMenu.help.generic.id = ID_EXIT;
    s_pauseMenu.help.generic.eventcallback = PauseMenu_EventCallback;
    s_pauseMenu.help.generic.font = FontCache()->AddFontToCache( "RobotoMono", "Bold" );
    s_pauseMenu.help.text = helpString->value;
    s_pauseMenu.help.color = color_white;

    s_pauseMenu.exitToMainMenu.generic.type = MTYPE_TEXT;
    s_pauseMenu.exitToMainMenu.generic.id = ID_EXIT;
    s_pauseMenu.exitToMainMenu.generic.eventcallback = PauseMenu_EventCallback;
    s_pauseMenu.exitToMainMenu.generic.font = FontCache()->AddFontToCache( "RobotoMono", "Bold" );
    s_pauseMenu.exitToMainMenu.text = exitToMainMenuString->value;
    s_pauseMenu.exitToMainMenu.color = color_white;

    s_pauseMenu.oldVolume = Cvar_VariableFloat( "snd_musicvol" );
    Cvar_Set( "snd_musicvol", va( "%i", PAUSEMENU_VOLUME_CAP ) );

    Menu_AddItem( &s_pauseMenu.menu, &s_pauseMenu.resume );
    Menu_AddItem( &s_pauseMenu.menu, &s_pauseMenu.checkpoint );
    Menu_AddItem( &s_pauseMenu.menu, &s_pauseMenu.help );
    Menu_AddItem( &s_pauseMenu.menu, &s_pauseMenu.exitToMainMenu );
}

void UI_PauseMenu( void )
{
    // force as top level menu
    UI_ForceMenuOff();
    Key_ClearStates();
    Key_SetCatcher( KEYCATCH_UI );
    Snd_PlaySfx( ui->sfx_select );

    UI_PushMenu( &s_pauseMenu.menu );
}
