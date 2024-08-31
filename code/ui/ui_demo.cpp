// ui_demo.cpp -- a wee little thank you popup for anyone playing the early versions
// of the game and/or the demos of the full release

#include "ui_lib.h"
#include <SDL2/SDL.h>

typedef struct {
	menuframework_t menu;
} demoMenu_t;

static demoMenu_t *s_demo;

// NOTE: this will change with each major version
#define DEMO_STRING \
	"Thank you very much for playing this demo of \"The Nomad\"! :)\n" \
	"More levels and a full alpha/early-acccess versions are in\n" \
	"development. If you would like to stay update, check out\n" \
	"the discord for updates or the dev blog.\n"

static void DemoMenu_Draw( void )
{
	const int windowFlags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize
		| ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoCollapse;
	bool done;

	done = true;

	ImGui::Begin( "##DemoMenu", NULL, windowFlags );
	ImGui::SetWindowFontScale( 2.5f * ui->scale );
	ImGui::SetWindowPos( ImVec2( 100 * ui->scale, 200 * ui->scale ) );

	ImGui::TextUnformatted( DEMO_STRING );
	ImGui::NewLine();

	if ( ImGui::Button( "DISCORD SERVER" ) ) {
		Snd_PlaySfx( ui->sfx_select );
		done = false;
		// TODO:
	}

	ImGui::SameLine( 528.0f * ui->scale );

	if ( ImGui::Button( "WEBSITE" ) ) {
		Snd_PlaySfx( ui->sfx_select );
		SDL_OpenURL( "https://sites.google.com/view/gdrgames" );
		done = false;
	}

	ImGui::NewLine();

	done = ImGui::Button( "DONE" );

	ImGui::End();

	if ( done ) {
		UI_PopMenu();
		UI_ForceMenuOff();
		ui->menusp = 0;

		Cbuf_ExecuteText( EXEC_APPEND, "setmap\n" );
		UI_SetActiveMenu( UI_MENU_MAIN );
		return;
	}
}

void DemoMenu_Cache( void )
{
	if ( !ui->uiAllocated ) {
		s_demo = (demoMenu_t *)Hunk_Alloc( sizeof( *s_demo ), h_high );
	}
	memset( s_demo, 0, sizeof( *s_demo ) );

	s_demo->menu.draw = DemoMenu_Draw;
	s_demo->menu.fullscreen = qtrue;
	ui->menubackShader = re.RegisterShader( "menu/mainbackground" );
}

void UI_DemoMenu( void )
{
	DemoMenu_Cache();
	UI_PushMenu( &s_demo->menu );
}