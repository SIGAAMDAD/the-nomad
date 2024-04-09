#include "ui_lib.h"
#include "../game/g_archive.h"

#define ID_SINGEPLAYER      1
#define ID_MODS             2
#define ID_SETTINGS         3
#define ID_CREDITS          4
#define ID_EXIT             5
#define ID_TABLE            6

typedef struct {
    menuframework_t menu;
    char message[MAXPRINTMSG];
} errorMessage_t;

typedef struct {
    menuframework_t menu;

    menutable_t table;

    menutext_t singleplayer;
    menutext_t mods;
    menutext_t settings;
    menutext_t credits;
    menutext_t exitGame;
    
    menuarrow_t spArrow;
    menuarrow_t modsArrow;
    menuarrow_t settingsArrow;
    menuarrow_t creditsArrow;
    menuarrow_t exitArrow;

    ImFont *font;

    nhandle_t background0;
    nhandle_t background1;
    sfxHandle_t ambience;

    const stringHash_t *logoString;
    const stringHash_t *spString;
    const stringHash_t *modsString;
    const stringHash_t *settingsString;
    const stringHash_t *exitString;

    int32_t menuWidth;
    int32_t menuHeight;

    qboolean toggleKey;
    qboolean noMenu; // do we just want the scenery?
} mainmenu_t;

ImFont *PressStart2P;
static errorMessage_t errorMenu;
static mainmenu_t s_main;

static void MainMenu_EventCallback( void *item, int event )
{
    const menucommon_t *self;

    if ( event != EVENT_ACTIVATED ) {
        return;
    }

    self = (const menucommon_t *)item;

    switch ( self->id ) {
    case ID_SINGEPLAYER:
        UI_SinglePlayerMenu();
        break;
    case ID_MODS:
        UI_ModsMenu();
        break;
    case ID_SETTINGS:
        UI_SettingsMenu();
        break;
    case ID_CREDITS:
        UI_CreditsMenu();
        break;
    case ID_EXIT:
        Cbuf_ExecuteText( EXEC_APPEND, "quit\n" );
        break;
    case ID_TABLE:
        break;
    default:
        N_Error( ERR_DROP, "MainMeu_EventCallback: unknown item id %i", self->id );
    };
}

static void MainMenu_ToggleMenu( void ) {
    if ( Key_IsDown( KEY_F2 ) ) {
        if ( s_main.noMenu ) {
            s_main.toggleKey = qfalse;
            s_main.noMenu = !s_main.noMenu;
        }
    }
    else {
        s_main.toggleKey = qtrue;
    }
}

void MainMenu_Draw( void )
{
    uint64_t i;
    const int windowFlags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse |
                            ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoBackground;

    MainMenu_ToggleMenu();

    if ( s_main.font ) {
        FontCache()->SetActiveFont( s_main.font );
    }

    if ( s_main.noMenu || Key_GetCatcher() & KEYCATCH_CONSOLE ) {
        return; // just the scenery & the music (a bit like Halo 3: ODST, check out halome.nu)...
    }

    // show the user WTF just happened
    if ( errorMenu.message[0] ) {
        Sys_MessageBox( "Game Error", errorMenu.message, false );
        Cvar_Set( "com_errorMessage", "" );
        UI_MainMenu();
        Snd_PlaySfx( ui->sfx_null );
    } else {
        Menu_Draw( &s_main.menu );
    }
}

void MainMenu_Cache( void )
{
    extern ImFont *RobotoMono;

    memset( &s_main, 0, sizeof( s_main ) );
    memset( &errorMenu, 0, sizeof( errorMenu ) );

    // only use of rand() is determining DIF_HARDEST title
    srand( time( NULL ) );

    // setup base values
    Cvar_Get( "g_mouseAcceleration", "0", CVAR_LATCH | CVAR_SAVE );
    Cvar_Get( "g_mouseInvert", "0", CVAR_LATCH | CVAR_SAVE );

    // check for errors
    Cvar_VariableStringBuffer( "com_errorMessage", errorMenu.message, sizeof(errorMenu.message) );
    if ( errorMenu.message[0] ) {
        Key_SetCatcher( KEYCATCH_UI );

        errorMenu.menu.draw = MainMenu_Draw;
        errorMenu.menu.fullscreen = qtrue;

        UI_ForceMenuOff();
        UI_PushMenu( &errorMenu.menu );

        return;
    }

    PressStart2P = FontCache()->AddFontToCache( "PressStart2P" );
    s_main.font = FontCache()->AddFontToCache( "AlegreyaSC", "Bold" );
    RobotoMono = FontCache()->AddFontToCache( "RobotoMono", "Bold" );

    s_main.noMenu = qfalse;
    s_main.toggleKey = qtrue;
    ui->menubackShader = s_main.background0;

    s_main.menu.titleFontScale = 10.5f;
    s_main.menu.textFontScale = 1.5f;

    s_main.logoString = strManager->ValueForKey( "MENU_LOGO_STRING" );
    s_main.settingsString = strManager->ValueForKey( "MENU_MAIN_SETTINGS" );
    s_main.spString = strManager->ValueForKey( "MENU_MAIN_SINGLEPLAYER" );
    s_main.modsString = strManager->ValueForKey( "MENU_MAIN_MODS" );
    s_main.exitString = strManager->ValueForKey( "MENU_MAIN_EXIT" );

    s_main.table.generic.name = "##MainMenuOptionsTable";
    s_main.table.generic.type = MTYPE_TABLE;
    s_main.table.generic.id = ID_TABLE;
    s_main.table.columns = 2;

    s_main.singleplayer.generic.name = StringDup( s_main.spString, "SinglePlayerMainMenuOption" );
    s_main.singleplayer.generic.type = MTYPE_TEXT;
    s_main.singleplayer.generic.id = ID_SINGEPLAYER;
    s_main.singleplayer.generic.flags = QMF_HIGHLIGHT_IF_FOCUS;
    s_main.singleplayer.generic.eventcallback = MainMenu_EventCallback;
    s_main.singleplayer.text = s_main.spString->value;
    s_main.singleplayer.color = color_white;

    s_main.mods.generic.name = StringDup( s_main.modsString, "ModsMainMenuOption" );
    s_main.mods.generic.type = MTYPE_TEXT;
    s_main.mods.generic.id = ID_MODS;
    s_main.mods.generic.flags = QMF_HIGHLIGHT_IF_FOCUS;
    s_main.mods.generic.eventcallback = MainMenu_EventCallback;
    s_main.mods.text = s_main.modsString->value;
    s_main.mods.color = color_white;

    s_main.settings.generic.name = StringDup( s_main.settingsString, "SettingsMainMenuOption" );
    s_main.settings.generic.type = MTYPE_TEXT;
    s_main.settings.generic.id = ID_SETTINGS;
    s_main.settings.generic.flags = QMF_HIGHLIGHT_IF_FOCUS;
    s_main.settings.generic.eventcallback = MainMenu_EventCallback;
    s_main.settings.text = s_main.settingsString->value;
    s_main.settings.color = color_white;

    s_main.credits.generic.name = "Credits##MainMenuOption";
    s_main.credits.generic.type = MTYPE_TEXT;
    s_main.credits.generic.id = ID_CREDITS;
    s_main.credits.generic.flags = QMF_HIGHLIGHT_IF_FOCUS;
    s_main.credits.generic.eventcallback = MainMenu_EventCallback;
    s_main.credits.text = "Credits";
    s_main.credits.color = color_white;

    s_main.exitGame.generic.name = StringDup( s_main.exitString, "ExitGameMenuOption" );
    s_main.exitGame.generic.type = MTYPE_TEXT;
    s_main.exitGame.generic.id = ID_EXIT;
    s_main.exitGame.generic.flags = QMF_HIGHLIGHT_IF_FOCUS;
    s_main.exitGame.generic.eventcallback = MainMenu_EventCallback;
    s_main.exitGame.text = s_main.exitString->value;
    s_main.exitGame.color = color_white;

    s_main.spArrow.generic.name = StringDup( s_main.spString, "SinglePlayerMainMenuArrow" );
    s_main.spArrow.generic.type = MTYPE_ARROW;
    s_main.spArrow.generic.id = ID_SINGEPLAYER;
    s_main.spArrow.generic.eventcallback = MainMenu_EventCallback;
    s_main.spArrow.direction = ImGuiDir_Right;

    s_main.modsArrow.generic.name = StringDup( s_main.modsString, "ModsMainMenuArrow" );
    s_main.modsArrow.generic.type = MTYPE_ARROW;
    s_main.modsArrow.generic.id = ID_MODS;
    s_main.modsArrow.generic.eventcallback = MainMenu_EventCallback;
    s_main.modsArrow.direction = ImGuiDir_Right;

    s_main.settingsArrow.generic.name = StringDup( s_main.settingsString, "SettingsMainMenuArrow" );
    s_main.settingsArrow.generic.type = MTYPE_ARROW;
    s_main.settingsArrow.generic.id = ID_SETTINGS;
    s_main.settingsArrow.generic.eventcallback = MainMenu_EventCallback;
    s_main.settingsArrow.direction = ImGuiDir_Right;

    s_main.creditsArrow.generic.name = "Credits##CreditsMainMenuArrow";
    s_main.creditsArrow.generic.type = MTYPE_ARROW;
    s_main.creditsArrow.generic.id = ID_CREDITS;
    s_main.creditsArrow.generic.eventcallback = MainMenu_EventCallback;
    s_main.creditsArrow.direction = ImGuiDir_Right;

    s_main.exitArrow.generic.name = StringDup( s_main.exitString, "ExitGameMainMenuArrow" );
    s_main.exitArrow.generic.type = MTYPE_ARROW;
    s_main.exitArrow.generic.id = ID_EXIT;
    s_main.exitArrow.generic.eventcallback = MainMenu_EventCallback;
    s_main.exitArrow.direction = ImGuiDir_Right;

    s_main.ambience = Snd_RegisterTrack( "music/title.ogg" );
    s_main.background0 = re.RegisterShader( "menu/mainbackground" );

    Menu_AddItem( &s_main.menu, &s_main.table );

    Table_AddRow( &s_main.table );
    Table_AddItem( &s_main.table, &s_main.singleplayer );
    Table_AddItem( &s_main.table, &s_main.spArrow );

    Table_AddRow( &s_main.table );
    Table_AddItem( &s_main.table, &s_main.mods );
    Table_AddItem( &s_main.table, &s_main.modsArrow );

    Table_AddRow( &s_main.table );
    Table_AddItem( &s_main.table, &s_main.settings );
    Table_AddItem( &s_main.table, &s_main.settingsArrow );

    Table_AddRow( &s_main.table );
    Table_AddItem( &s_main.table, &s_main.credits );
    Table_AddItem( &s_main.table, &s_main.creditsArrow );

    Table_AddRow( &s_main.table );
    Table_AddItem( &s_main.table, &s_main.exitGame );
    Table_AddItem( &s_main.table, &s_main.exitArrow );

    Key_SetCatcher( KEYCATCH_UI );
    ui->menusp = 0;
    UI_PushMenu( &s_main.menu );
}

void UI_MainMenu( void ) {
    MainMenu_Cache();
}
