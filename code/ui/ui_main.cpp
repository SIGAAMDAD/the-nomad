#include "../game/g_game.h"
#include "ui_public.h"
#include "ui_menu.h"
#include "ui_lib.h"
#include "ui_window.h"
#include "../rendercommon/imgui.h"

CUILib *ui;

extern "C" char *CopyString(const char *str) {
    return Z_Strdup(str);
}

static void UI_RegisterCvars( void )
{

}

void UI_UpdateCvars( void )
{

}

/*
=================
UI_Cache
=================
*/
static void UI_Cache_f( void )
{
}

extern "C" void UI_Shutdown( void )
{
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
    ui->Init(); // we could call ::new, but whatever...

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
extern "C" void UI_Refresh( uint64_t realtime )
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

static void UI_WindowDefault( const char *name )
{
}


typedef struct {
    CUIMenu menu;

    mtext_t thenomad;
} titlemenu_t;

#define MAIN_MENU_VERTICAL_SPACING 34

static titlemenu_t title;

static void TitleMenu_Draw( void )
{
    // setup window
    ImGuiWindow window( "TitleMenu" );
    window.SetFontScale(2);

    ImGui::SeparatorText("The Nomad");
}

void UI_TitleMenu( void )
{
    int y;
    int style = UI_CENTER | UI_DROPSHADOW;

    memset(&title, 0, sizeof(title));

    title.menu.Draw = TitleMenu_Draw;
    title.menu.fullscreen = qtrue;
    title.menu.wrapAround = qtrue;

    y = 0;
    title.thenomad.generic.type         = MTYPE_PTEXT;
    title.thenomad.generic.flags        = QMF_CENTER_JUSTIFY | QMF_PULSEIFFOCUS;
    title.thenomad.generic.x            = 320;
    title.thenomad.generic.y            = y;
    title.thenomad.generic.id           = ID_THENOMAD;
    title.thenomad.string               = CopyString("THE NOMAD");
    title.thenomad.style                = style;
    VectorCopy4(title.thenomad.color, color_red);

    ui->PushMenu(&title.menu);
}
