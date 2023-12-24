#include "ui_lib.h"
#include "../game/g_archive.h"

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
    CUIMenu menu;
    char message[MAXPRINTMSG];
} errorMessage_t;

static errorMessage_t errorMenu;

typedef struct
{
    CUIMenu menu;

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
    const int windowFlags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse |
                            ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBackground;

    menu.noMenuToggle.Toggle( KEY_F2, menu.noMenu );

    Snd_SetLoopingTrack( menu.ambience );

    if ( ui->GetState() == STATE_ERROR ) {
        if ( ImGui::BeginPopup( "Engine Error", windowFlags & ~( ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBackground ) ) ) {
            ImGui::TextUnformatted( errorMenu.message );
            if ( Key_AnyDown() ) {
                Snd_PlaySfx( ui->sfx_null );
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }
    }

    /*
    ImGui::Begin( "MainMenu##BACKGROUND", NULL, windowFlags | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoMouseInputs | ImGuiWindowFlags_NoBringToFrontOnFocus );
    ImGui::SetWindowPos( ImVec2( 0, 0 ) );
    ImGui::SetWindowSize( ImVec2( (float)menu.menuWidth, (float)menu.menuHeight ) );
    ImGui::Image( re.ImGui_TextureData( menu.background0 ), ImVec2( (float)menu.menuWidth, (float)menu.menuHeight ) );
    ImGui::End();
    */

    if (menu.noMenu) {
        return; // just the scenery & the music (a bit like Halo 3: ODST, check out halome.nu)...
    }

    if (ui->GetState() == STATE_NAMEISSUE) {
        NewGame_DrawNameIssue();
    }

    if (ui->GetState() == STATE_MAIN) {
        ImGui::Begin( "MainMenu", NULL, windowFlags );
        ImGui::SetWindowPos( ImVec2( 0, 0 ) );
        ImGui::SetWindowSize( ImVec2( (float)menu.menuWidth / 2, (float)menu.menuHeight ) );
        ui->Menu_Title( "MAIN MENU" );

        const ImVec2 mousePos = ImGui::GetCursorScreenPos();
        ImGui::SetCursorScreenPos( ImVec2( mousePos.x, mousePos.y + 10 ) );

        ImGui::BeginTable( " ", 2 );
        if (ui->Menu_Option( "Single Player" )) {
            ui->SetState( STATE_SINGLEPLAYER );
        }
        ImGui::TableNextRow();
        if (ui->Menu_Option( "Settings" )) {
            ui->SetState( STATE_SETTINGS );
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

        ImGui::End();
    }
    else if (ui->GetState() >= STATE_SINGLEPLAYER && ui->GetState() <= STATE_PLAYMISSION) {
        ImGui::Begin( "MainMenu", NULL, windowFlags );
        ImGui::SetWindowPos( ImVec2( 0, 0 ) );
        ImGui::SetWindowSize( ImVec2( (float)menu.menuWidth / 2, (float)menu.menuHeight ) );
        SinglePlayerMenu_Draw();
        ImGui::End();
    }
    else if (ui->GetState() >= STATE_SETTINGS && ui->GetState() <= STATE_AUDIO) {
        ImGui::Begin( "MainMenu", NULL, windowFlags );
        ImGui::SetWindowPos( ImVec2( 0, 0 ) );
        ImGui::SetWindowSize( ImVec2( (float)menu.menuWidth / 2, (float)menu.menuHeight ) );
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
            return;
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
        N_Error(ERR_FATAL, "Invalid UI State"); // should NEVER happen
    }
}

void MainMenu_Cache( void )
{
    memset( &menu, 0, sizeof(menu) );
    memset( &errorMenu, 0, sizeof(errorMenu) );

    // only use of rand() is determining DIF_HARDEST title
    srand( time( NULL ) );

    // setup base values
    Cvar_Get( "g_mouseAcceleration", "0", CVAR_LATCH | CVAR_SAVE );
    Cvar_Get( "g_mouseInvert", "0", CVAR_LATCH | CVAR_SAVE );

    // check for errors
    Cvar_VariableStringBuffer( "com_errorMessage", errorMenu.message, sizeof(errorMenu.message) );
    if ( errorMenu.message[0] ) {
        errorMenu.menu.Draw = MainMenu_Draw;

        ui->SetState( STATE_ERROR );

        Key_SetCatcher( KEYCATCH_UI );
        ui->ForceMenuOff();
        ui->PushMenu( &errorMenu.menu );

        ImGui::OpenPopup( "Engine Error" );

        return;
    }

    menu.menu.Draw = MainMenu_Draw;

    menu.ambience = Snd_RegisterTrack( "music/title.ogg" );
    menu.background0 = re.RegisterShader( "textures/desertbkgd.jpg" );

    menu.settingsString = strManager->ValueForKey("MENU_MAIN_SETTINGS");

    menu.noMenu = qfalse;
    menu.menuHeight = ui->GetConfig().vidHeight;
    menu.menuWidth = ui->GetConfig().vidWidth;
    ui->SetState( STATE_MAIN );
}

void UI_MainMenu( void )
{
    MainMenu_Cache();

    ui->PushMenu( &menu.menu );
}
