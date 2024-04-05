#include "ui_lib.h"
#include "../game/g_archive.h"

#define MAIN_MENU_BACKGROUND "menuback"

class CToggleKey
{
public:
    CToggleKey( void )
        : toggleOn( qtrue )
    {
    }
    ~CToggleKey() { }

    void Toggle( uint32_t key, qboolean& toggleVar ) {
        if (Key_IsDown( key )) {
            if (toggleOn) {
                toggleOn = qfalse;
                toggleVar = ~toggleVar;
            }
        }
        else {
            toggleOn = qtrue;
        }
    }
    void Toggle( uint32_t key ) {
        if (Key_IsDown( key )) {
            if (toggleOn) {
                toggleOn = qfalse;
            }
        }
        else {
            toggleOn = qtrue;
        }
    }
    qboolean On( void ) const { return toggleOn; }
    void Set( qboolean toggle ) { toggleOn = toggle; }
private:
    qboolean toggleOn;
};

typedef struct {
    CUIMenu handle;
    char message[MAXPRINTMSG];
} errorMessage_t;

typedef struct
{
    CUIMenu handle;

    ImFont *font;

    nhandle_t background0;
    nhandle_t background1;
    sfxHandle_t ambience;

    const stringHash_t *spString;
    const stringHash_t *settingsString;

    int32_t menuWidth;
    int32_t menuHeight;

    CToggleKey noMenuToggle;

    qboolean noMenu; // do we just want the scenery?
} mainmenu_t;

ImFont *PressStart2P;
static errorMessage_t errorMenu;
static mainmenu_t menu;

static const char *creditsString =
"As always, I would not have gotten to this point without the help of many\n"
"I would like to take the time to thank the following people for contributing\n"
"to this massive project\n";

static const char *signingOffString =
"Sincerely,\nYour Resident Fiend,\nNoah Van Til";

typedef struct {
    const char *name;
    const char *reason;
} collaborator_t;

static const collaborator_t collaborators[] = {
    { "Ben Pavlovic", "Some weapon ideas, created the name of the hardest difficulty: \"Just A Minor Inconvience\"" },
    { "Tucker Kemnitz", "Art, ideas for some NPCs" },
    { "Alpeca Grenade", "A music piece" },
    { "Jack Rosenthal", "A couple of ideas" },
    { "My Family & Friends", "Helping me get through some tough times" },
    { "My Father", "Giving me feedback, tips and tricks for programming when I was struggling, and helped test the first working version" },
};

void MainMenu_Draw( void )
{
    uint64_t i;
    float x, y, w, h;
    const int windowFlags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse |
                            ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoBackground;
    renderSceneRef_t refdef;

    menu.noMenuToggle.Toggle( KEY_F2, menu.noMenu );


    x = 0;
    y = 0;
    w = ui->GetConfig().vidWidth;
    h = ui->GetConfig().vidHeight;

    memset( &refdef, 0, sizeof( refdef ) );
    refdef.x = 0;
    refdef.y = 0;
    refdef.width = w;
    refdef.height = h;
    refdef.time = ui->GetFrameTime();
    refdef.flags = RSF_NOWORLDMODEL | RSF_ORTHO_TYPE_SCREENSPACE;

    re.ClearScene();
    re.DrawImage( 0, 0, refdef.width, refdef.height, 1, 0, 0, 1, menu.background0 );
    re.RenderScene( &refdef );

//    ImGui::Begin( "MainMenuBackground", NULL, windowFlags & ~( ImGuiWindowFlags_NoResize ) );
//    ImGui::SetWindowPos( ImVec2( 0.0f, 0.0f ) );
//    ImGui::Image( (void *)(intptr_t)menu.background0, ImGui::GetWindowSize(), ImVec2( 1, 0 ), ImVec2( 0, 1 ) );
//    ImGui::End();
    if ( ui->GetState() != STATE_MODS ) {
        Snd_SetLoopingTrack( menu.ambience );
    }

    if ( menu.font ) {
        FontCache()->SetActiveFont( menu.font );
    }

    // show the user WTF just happened
    if ( errorMenu.message[0] ) {
        Sys_MessageBox( "Game Error", errorMenu.message, false );
        Snd_PlaySfx( ui->sfx_null );
        memset( &errorMenu, 0, sizeof( errorMenu ) );
        Cvar_Set( "com_errorMessage", "" );
        return;
    }

    if ( menu.noMenu ) {
        return; // just the scenery & the music (a bit like Halo 3: ODST, check out halome.nu)...
    }

    if (ui->GetState() == STATE_MAIN) {
        ImGui::Begin( "##MainMenu", NULL, windowFlags );
        ImGui::SetWindowPos( ImVec2( 0, 0 ) );
        ImGui::SetWindowSize( ImVec2( (float)menu.menuWidth, (float)menu.menuHeight ) );
        FontCache()->SetActiveFont( menu.font );
        const float fontScale = ImGui::GetFont()->Scale;
        ImGui::SetWindowFontScale( ( fontScale * 10.5f ) * ui->scale );
        ui->Menu_Title( "MAIN MENU" );
        ImGui::SetWindowFontScale( ( fontScale * 1.5f ) * ui->scale );

        const ImVec2 mousePos = ImGui::GetCursorScreenPos();
        ImGui::SetCursorScreenPos( ImVec2( mousePos.x, mousePos.y + 10 ) );

        FontCache()->SetActiveFont( PressStart2P );

        ImGui::BeginTable( "##MainMenuTable", 2 );
        if (ui->Menu_Option( "Single Player" )) {
            ui->SetState( STATE_SINGLEPLAYER );
        }
        ImGui::TableNextRow();
        if ( ui->Menu_Option( "Tales Around the Campfire" ) ) {
            ui->SetState( STATE_MODS );
        }
        ImGui::TableNextRow();
        if (ui->Menu_Option( "Settings" )) {
            ui->SetState( STATE_SETTINGS );
        }
        ImGui::TableNextRow();
        if ( ui->Menu_Option( "Legal Stuff" ) ) {
            ui->SetState( STATE_LEGAL );
        }
        ImGui::TableNextRow();
        if (ui->Menu_Option( "Credits" )) {
            ui->SetState( STATE_CREDITS );
        }
        ImGui::TableNextRow();
        if (ui->Menu_Option( "Exit To Title Screen" )) {
            ui->PopMenu();
        }
        ImGui::TableNextRow();
        if (ui->Menu_Option( "Exit To Desktop" )) {
            // TODO: possibly add in a DOOM-like exit popup?
            Sys_Exit( 1 );
        }
        ImGui::EndTable();
        ImGui::SetWindowFontScale( fontScale );

        ImGui::End();
    }
    else if ( ui->GetState() == STATE_LEGAL ) {
        ImGui::Begin( "MainMenu", NULL, windowFlags );
        ImGui::SetWindowPos( ImVec2( 0, 0 ) );
        ImGui::SetWindowSize( ImVec2( (float)menu.menuWidth, (float)menu.menuHeight ) );
        LegalMenu_Draw();
        ImGui::End();
    }
    else if (ui->GetState() >= STATE_SINGLEPLAYER && ui->GetState() <= STATE_PLAYMISSION) {
        ImGui::Begin( "MainMenu", NULL, windowFlags );
        ImGui::SetWindowPos( ImVec2( 0, 0 ) );
        ImGui::SetWindowSize( ImVec2( (float)menu.menuWidth, (float)menu.menuHeight ) );
        SinglePlayerMenu_Draw();
        ImGui::End();
    }
    else if ( ui->GetState() == STATE_MODS ) {
        ImGui::Begin( "MainMenu", NULL, windowFlags );
        ImGui::SetWindowPos( ImVec2( 0, 0 ) );
        ImGui::SetWindowSize( ImVec2( (float)menu.menuWidth, (float)menu.menuHeight ) );
        ModsMenu_Draw();
        ImGui::End();
    }
    else if (ui->GetState() >= STATE_SETTINGS && ui->GetState() <= STATE_GAMEPLAY) {
        ImGui::Begin( "MainMenu", NULL, windowFlags );
        ImGui::SetWindowPos( ImVec2( 0, 0 ) );
        ImGui::SetWindowSize( ImVec2( (float)menu.menuWidth * 0.75f, (float)menu.menuHeight * 0.75f ) );
        SettingsMenu_Draw();
        ImGui::End();
    }
    else if (ui->GetState() == STATE_CREDITS) {
        ImGui::Begin( "MainMenu", NULL, windowFlags );
        ImGui::SetWindowPos( ImVec2( 0, 0 ) );
        ImGui::SetWindowSize( ImVec2( (float)menu.menuWidth, (float)menu.menuHeight ) );
        ui->EscapeMenuToggle( STATE_MAIN );
        if (ui->Menu_Title( "CREDITS" )) {
            ui->SetState( STATE_MAIN );
        }
        else {
            ImGui::TextWrapped( "%s", creditsString );
            ImGui::NewLine();
            for (const auto& it : collaborators) {
                ImGui::Bullet();
                ImGui::TextWrapped( "%-24s %s", it.name, it.reason );
            }
            ImGui::TextUnformatted( signingOffString );

            ImGui::End();
        }
    }
    else {
        N_Error( ERR_FATAL, "Invalid UI State" ); // should NEVER happen
    }
}

void MainMenu_Cache( void )
{
    extern ImFont *RobotoMono;

    memset( &menu, 0, sizeof( menu ) );
    memset( &errorMenu, 0, sizeof( errorMenu ) );
//    menu = (mainmenu_t *)Hunk_Alloc( sizeof( *menu ), h_high );
//    errorMenu = (errorMessage_t *)Z_Malloc( sizeof( *errorMenu ), h_high );

    // only use of rand() is determining DIF_HARDEST title
    srand( time( NULL ) );

    // setup base values
    Cvar_Get( "g_mouseAcceleration", "0", CVAR_LATCH | CVAR_SAVE );
    Cvar_Get( "g_mouseInvert", "0", CVAR_LATCH | CVAR_SAVE );

    // check for errors
    Cvar_VariableStringBuffer( "com_errorMessage", errorMenu.message, sizeof(errorMenu.message) );
    if ( errorMenu.message[0] ) {
        errorMenu.handle.Draw = MainMenu_Draw;

        ui->SetState( STATE_ERROR );

        Key_SetCatcher( KEYCATCH_UI );
        ui->ForceMenuOff();
        ui->PushMenu( &errorMenu.handle );
        return;
    }

    menu.handle.Draw = MainMenu_Draw;

    menu.ambience = Snd_RegisterTrack( "music/title.ogg" );
    menu.background0 = re.RegisterShader( MAIN_MENU_BACKGROUND );

    menu.settingsString = strManager->ValueForKey( "MENU_MAIN_SETTINGS" );
    menu.spString = strManager->ValueForKey( "MENU_MAIN_SINGLEPLAYER" );

    PressStart2P = FontCache()->AddFontToCache( "PressStart2P" );
    menu.font = FontCache()->AddFontToCache( "AlegreyaSC", "Bold" );
    RobotoMono = FontCache()->AddFontToCache( "RobotoMono", "Bold" );

    ui->menu_background = menu.background0;

    menu.noMenu = qfalse;
    menu.menuHeight = ui->GetConfig().vidHeight;
    menu.menuWidth = ui->GetConfig().vidWidth;
    ui->SetState( STATE_MAIN );
}

void UI_MainMenu( void )
{
    MainMenu_Cache();

    ui->PushMenu( &menu.handle );
}
