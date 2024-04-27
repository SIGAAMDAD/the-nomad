#include "ui_lib.h"

typedef struct {
    menuframework_t menu;

    nhandle_t background;

    menucustom_t progressbar;

    uint64_t lastDotTime;
    int dotCounter;
} loadMenu_t;

static loadMenu_t *s_loadMenu;

static void LoadMenu_UpdateThread( void )
{
    CThread *pThread;

    pThread = gi.m_LoadStack.top();

    // 10 KiB stack should be plenty
    if ( !pThread->IsAlive() ) {
        if ( !pThread->Start() ) {
            N_Error( ERR_FATAL, "G_RunLoaderThread: failed to start thread '%s'", pThread->GetName() );
        }
    } else {
        if ( !pThread->Join( 100 ) ) {
            return;
            N_Error( ERR_FATAL, "LoadMenu_UpdateThread: failed to join thread '%s'", pThread->GetName() );
        }
        gi.m_LoadStack.pop();
    }
}

static void LoadMenu_ProgressBar( void *ptr ) {
    if ( !gi.m_LoadStack.size() ) {
        UI_ForceMenuOff();
        ui->menusp = NULL;
        UI_SetActiveMenu( UI_MENU_MAIN );
        return;
    }
    static uint64_t dotTime = 0;
    uint64_t dotDeltaTime;
    int i;
    float progress;

    LoadMenu_UpdateThread();

    ImGui::SetCursorScreenPos( ImVec2( 64 * ui->scale, 600 * ui->scale ) );

    progress = ( gi.m_nLoadStackMaxSize - gi.m_LoadStack.size() ) / 100.0f;
    ImGui::Text( "%s (%0.02f/100.0%%)", gi.m_LoadStack.top()->GetName(), progress );

    ImGui::SameLine();

    ImGui::TextUnformatted( "  LOADING" );
    dotTime = Sys_Milliseconds();
    dotDeltaTime = s_loadMenu->lastDotTime - dotTime;
    if ( dotDeltaTime > 500 ) {
        for ( i = 0; i < s_loadMenu->dotCounter; i++ ) {
            ImGui::SameLine();
            ImGui::TextUnformatted( "." );
        }
        s_loadMenu->dotCounter++;
        if ( s_loadMenu->dotCounter >= 3 ) {
            s_loadMenu->dotCounter = 0;
        }
    }
    s_loadMenu->lastDotTime = dotTime;
    ImGui::NewLine();

    ImGui::ProgressBar( progress, ImVec2( 900 * ui->scale, 72 * ui->scale ) );
}

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

void UI_LoadMenu( void )
{
    LoadMenu_Cache();
    UI_PushMenu( &s_loadMenu->menu );
}
