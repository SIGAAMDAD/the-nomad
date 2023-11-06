#include "../game/g_game.h"
#include "ui_public.h"
#include "ui_menu.h"
#include "ui_lib.h"
#include "ui_window.h"
#include "ui_string_manager.h"

CUILib *ui;
cvar_t *ui_language;
cvar_t *ui_printStrings;

const char *UI_LangToString( int32_t lang )
{
    switch ((language_t)lang) {
    case LANG_ENGLISH:
        return "english";
    default:
        break;
    };
    return "Invalid";
}

static void UI_RegisterCvars( void )
{
    ui_language = Cvar_Get( "ui_language", "0", CVAR_LATCH | CVAR_SAVE );
    Cvar_SetDescription( ui_language,
                        "Sets the game's language:\n"
                        "  0 - English\n"
                        "  1 - Spanish (Not Supported Yet)\n" );
    Cvar_CheckRange( ui_language, va("%lu", LANG_ENGLISH), va("%lu", NUMLANGS), CVT_INT );

    ui_printStrings = Cvar_Get( "ui_printStrings", "1", CVAR_LATCH | CVAR_SAVE | CVAR_PRIVATE);
    Cvar_SetDescription( ui_printStrings, "Print value strings set by the language ui file" );
    Cvar_CheckRange( ui_printStrings, "0", "1", CVT_INT );
}

void UI_UpdateCvars( void )
{
    if (ui_language->modified) {
        if (!strManager->LanguageLoaded((language_t)ui_language->i)) {
            strManager->LoadFile(va("scripts/ui_strings_%s.txt", UI_LangToString(ui_language->i)));
        }
        ui_language->modified = qfalse;
    }
}

/*
=================
UI_Cache
=================
*/
static void UI_Cache_f( void )
{
    TitleMenu_Cache();
    SettingsMenu_Cache();
}

extern "C" void UI_Shutdown( void )
{
    if (ui) {
        ui->Shutdown();
    }
    if (strManager) {
        strManager->Shutdown();
    }

    Cmd_RemoveCommand( "ui_cache" );
}

// FIXME: call UI_Shutdown instead
void G_ShutdownUI( void ) {
    UI_Shutdown();
}

extern "C" void UI_Init( void )
{
    // register cvars
    UI_RegisterCvars();

    // init the library
    ui = (CUILib *)Hunk_Alloc(sizeof(*ui), h_low);
    memset(ui, 0, sizeof(*ui));
    ui->Init(); // we could call ::new

    // init the string manager
    strManager = (CUIStringManager *)Hunk_Alloc(sizeof(*strManager), h_low);
    memset(strManager, 0, sizeof(*strManager));
    strManager->Init();

    // load the language string file
    strManager->LoadFile(va("scripts/ui_strings_%s.txt", UI_LangToString(ui_language->i)));
    if (!strManager->NumLangsLoaded()) {
        N_Error(ERR_DROP, "UI_Init: no language loaded");
    }

    ui->SetActiveMenu( UI_MENU_TITLE );

    // add commands
    Cmd_AddCommand( "ui_cache", UI_Cache_f );
}

void Menu_Cache( void )
{
    ui->charset = re.RegisterShader( "gfx/bigchars" );
    ui->rb_on = re.RegisterShader( "gfx/rb_on" );
    ui->rb_off = re.RegisterShader( "gfx/rb_off" );

    ui->whiteShader = re.RegisterShader( "white" );
    ui->menubackShader = re.RegisterShader( "menuback" );
}

/*
=================
UI_Refresh
=================
*/
extern "C" void UI_Refresh( int realtime )
{
	ui->SetFrameTime( ui->GetRealTime() - realtime );
	ui->SetRealTime( realtime );

	if ( !( Key_GetCatcher() & KEYCATCH_UI ) ) {
		return;
	}

	UI_UpdateCvars();

	if ( ui->GetCurrentMenu() ) {
		if (ui->GetCurrentMenu()->fullscreen) {
            ui->DrawHandlePic( 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, ui->menubackShader );
		}

		if (ui->GetCurrentMenu()->Draw)
			ui->GetCurrentMenu()->Draw();
		else
			Menu_Draw( ui->GetCurrentMenu() );

		if( ui->GetFirstDraw() ) {
			ui->MouseEvent( 0, 0 );
			ui->SetFirstDraw( qfalse );
		}
	}

	// draw cursor
//	ui->SetColor( NULL );
//	ui->DrawHandlePic( ui->GetCursorX() - 16, ui->GetCursorY() - 16, 32, 32, cursor);

#ifdef _NOMAD_DEBUG
	if (ui->IsDebug()) {
		// cursor coordinates
		ui->DrawString( 0, 0, va("(%d,%d)", ui->GetCursorX(), ui->GetCursorY()), UI_LEFT|UI_SMALLFONT, color_red );
	}
#endif

	// delay playing the enter sound until after the
	// menu has been drawn, to avoid delay while
	// caching images
	if (m_entersound) {
		Snd_PlaySfx( menu_in_sound );
		m_entersound = qfalse;
	}
}
