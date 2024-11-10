#include "ui_lib.h"

typedef struct {
	menuframework_t menu;

	nhandle_t background;

	menucustom_t progressbar;
	uint64_t nTotalAssets;
	uint64_t nCompletedAssets;

	pthread_mutex_t lock;
} loadMenu_t;

static loadMenu_t *s_loadMenu;

static void LoadMenu_ProgressBar( void *self )
{
	/*
	const uint64_t nTotal = s_loadMenu->szLoadGroups[ LOAD_SCRIPTS ][1] + s_loadMenu->szLoadGroups[ LOAD_SHADERS ][1]
		+ s_loadMenu->szLoadGroups[ LOAD_TEXTURES ][1];
	const uint64_t nCompleted = s_loadMenu->szLoadGroups[ LOAD_SCRIPTS ][0] + s_loadMenu->szLoadGroups[ LOAD_SHADERS ][-]
		+ s_loadMenu->szLoadGroups[ LOAD_TEXTURES ][0];

	ImGui::SetCursorScreenPos( ImVec2( 16 * ui->scale + ui->bias, 600 * ui->scale ) );
	if ( !s_loadMenu->szLoadGroups[0] ) {
		ImGui::Text( "Loading Scripts..." );
	} else if ( s_loadMenu->szLoadGroups[1] ) {
		ImGui::Text( "Loading Shaders..." );
	} else if ( s_loadMenu->szLoadGroups[2] ) {
		ImGui::Text( "Loading Textures..." );
	}
	*/
	ImGui::TextUnformatted( "Loading Resources..." );
	ImGui::ProgressBar( (float)( s_loadMenu->nCompletedAssets / s_loadMenu->nTotalAssets ) );
}

void UI_FinishResource( void )
{
	if ( !s_loadMenu || !ui->uiAllocated ) {
		return;
	}
	pthread_mutex_lock( &s_loadMenu->lock );
	s_loadMenu->nCompletedAssets++;
	pthread_mutex_unlock( &s_loadMenu->lock );
}

void UI_PushResource( void )
{
	if ( !s_loadMenu || !ui->uiAllocated ) {
		return;
	}
	pthread_mutex_lock( &s_loadMenu->lock );
	s_loadMenu->nTotalAssets++;
	pthread_mutex_unlock( &s_loadMenu->lock );
}

void LoadMenu_Cache( void )
{
	if ( !ui->uiAllocated ) {
		s_loadMenu = (loadMenu_t *)Hunk_Alloc( sizeof( *s_loadMenu ), h_high );
	}
	if ( s_loadMenu->menu.name ) {
		pthread_mutex_destroy( &s_loadMenu->lock );	
	}
	memset( s_loadMenu, 0, sizeof( *s_loadMenu ) );

	s_loadMenu->menu.flags = MENU_DEFAULT_FLAGS;
	s_loadMenu->menu.fullscreen = qtrue;
	s_loadMenu->menu.width = ui->gpuConfig.vidWidth;
	s_loadMenu->menu.height = ui->gpuConfig.vidHeight;
	s_loadMenu->menu.textFontScale = 2.5f;
	s_loadMenu->menu.titleFontScale = 3.5f;
	s_loadMenu->menu.name = strManager->ValueForKey( "MENU_LOGO_STRING" )->value;
	s_loadMenu->menu.track = Snd_RegisterTrack( "event:/music/title" );
	s_loadMenu->menu.x = 0;
	s_loadMenu->menu.y = 0;

	s_loadMenu->progressbar.generic.ownerdraw = LoadMenu_ProgressBar;
	s_loadMenu->progressbar.generic.type = MTYPE_CUSTOM;
	s_loadMenu->progressbar.generic.flags = QMF_OWNERDRAW;

	s_loadMenu->background = re.RegisterShader( "menu/mainbackground" );
	ui->menubackShader = s_loadMenu->background;

	pthread_mutex_init( &s_loadMenu->lock, NULL );

	Menu_AddItem( &s_loadMenu->menu, &s_loadMenu->progressbar );
}

void UI_LoadMenu( void )
{
	LoadMenu_Cache();
	UI_PushMenu( &s_loadMenu->menu );
}
