#include "ui_lib.h"

typedef struct {
    menuframework_t menu;

    nhandle_t background;

    menucustom_t progressbar;

    uint64_t lastDotTime;
    int dotCounter;
} loadMenu_t;

static loadMenu_t *s_loadMenu;

static void LoadMenu_Cache( void )
{
    if ( !ui->uiAllocated ) {
        s_loadMenu = (loadMenu_t *)Hunk_Alloc( sizeof( *s_loadMenu ), h_high );
    }
    memset( s_loadMenu, 0, sizeof( *s_loadMenu ) );

    s_loadMenu->menu.flags = MENU_DEFAULT_FLAGS;
    s_loadMenu->menu.fullscreen = qtrue;
    s_loadMenu->menu.width = ui->gpuConfig.vidWidth;
    s_loadMenu->menu.height = ui->gpuConfig.vidHeight;
    s_loadMenu->menu.textFontScale = 2.5f;
    s_loadMenu->menu.titleFontScale = 3.5f;
    s_loadMenu->menu.name = strManager->ValueForKey( "MENU_LOGO_STRING" )->value;
    s_loadMenu->menu.track = Snd_RegisterTrack( "music/title.ogg" );
    s_loadMenu->menu.x = 0;
    s_loadMenu->menu.y = 0;

    s_loadMenu->progressbar.generic.ownerdraw = LoadMenu_ProgressBar;
    s_loadMenu->progressbar.generic.type = MTYPE_CUSTOM;
    s_loadMenu->progressbar.generic.flags = QMF_OWNERDRAW;

    s_loadMenu->background = re.RegisterShader( "menu/mainbackground" );
    ui->menubackShader = s_loadMenu->background;

    Menu_AddItem( &s_loadMenu->menu, &s_loadMenu->progressbar );
}

void UI_LoadAsset()
{
    
}

void UI_LoadMenu( void )
{
    LoadMenu_Cache();
    UI_PushMenu( &s_loadMenu->menu );
}
