#include "ui_lib.h"
#include "../game/g_archive.h"

enum
{
    kbMoveForward,

    NUMBINDS
};

typedef enum
{
    STATE_MAIN,
        STATE_SINGLEPLAYER,
            STATE_NEWGAME,
            STATE_LOADGAME,
            STATE_PLAYMISSION,

    STATE_SETTINGS,
        STATE_GRAPHICS,
        STATE_CONTROLS,
        STATE_AUDIO,
    
    STATE_CREDITS,
} menustate_t;

typedef struct {
    uint32_t key;
    const char *binding;
} keybind_t;

typedef struct
{
    CUIMenu menu;

    keybind_t keybinds[NUMBINDS];

    bool mouseAccelerate;
    bool mouseInvert;

    uint32_t rebindIndex;
    qboolean rebinding;

    bool musicOn;
    bool sfxOn;
    float musicVol;
    float sfxVol;

    renderapi_t api;

    qboolean confirmation;
    qboolean modified;
} settingsmenu_t;

typedef struct {
    char name[MAX_GDR_PATH];
    fileStats_t stats;
    uint64_t index;
    gamedata_t gd;
    qboolean valid;
} saveinfo_t;

typedef struct {
    CUIMenu menu;
    CUIMenu newgame;
    CUIMenu loadgame;

    // load game data
    saveinfo_t *saveList;
    uint64_t numSaves;

    // new game data
    char name[MAX_GDR_PATH];
    gamedif_t diff;
    uint64_t hardestIndex;

    const stringHash_t *newGame;
    const stringHash_t *loadGame;

    CGameArchive sv;
} singleplayer_t;

typedef struct
{
    CUIMenu menu;

    settingsmenu_t settings;
    singleplayer_t sp;

    nhandle_t background0;
    nhandle_t background1;
    nhandle_t ambience;

    const stringHash_t *spString;
    const stringHash_t *settingsString;

    float drawScale;
    int menuWidth;
    int menuHeight;

    menustate_t state;

    qboolean noMenu; // do we just want the scenery?
} mainmenu_t;

static mainmenu_t menu;

typedef struct {
    const char *name;
    const char *tooltip;
} dif_t;


// TODO: make this loadable from a file
static const char *difHardestTitles[] = {
    "POV: Kazuma",
    "Dark Souls",
    "Writing in C++",
    "Metal Goose Rising: REVENGEANCE",
    "Hell Itself",
    "Suicidal Encouragement",
    "Cope, Seethe, Repeat",
    "Sounds Like a U Problem",
    "GIT GUD",
    "THE MEMES",
    "Deal With It",
    "Just A Minor Inconvenience",
    "YOU vs God",
    "The Ultimate Bitch-Slap",
    "GIT REKT",
    "GET PWNED",
    "Wish U Had A BFG?"
};

static const char *creditsString =
"As Always, I would not have gotten to this point without the help of many\n"
"I would like to take the time to thank the following people for contributing\n"
"to this massive project\n";

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

static const dif_t difficultyTable[NUMDIFS] = {
    {
        "Noob",

        "The easiest difficulty by far, you will encounter extremely weak and unaggressive enemies."
        "Play this if you're new to fighting games and/or want a stress-free experience."
    },
    {
        "Rookie",
        
        "The game will take it easy on you, but it will still have a challenge every now and then."
        "Play this if you are new and/or want a stress-free experience but it won't pull all it's punches."
    },
    {
        "Mercenary",

        "The normal difficulty. Play this mode if you want a challenging but fair experience with your game."
    },
    {
        "Nomad",

        "One of the hardest difficulties, enemies will act more aggressive, deal more damage, and be a pain your ass."
        "Play this if you want even more of a challenge than Mercenary."
    },
    {
        "The Blackdeath",
        
        "The hardest difficulty. This mode is only for this most hardened players; enemies will attack and kill you without mercy, most attacks one-shot."
        "Speed, accuracy, and skill are your best friends. Play this if you feel like ascending into god gamerhood."
        "TLDR: this is the difficulty that is used in the canon story."
    },
    {
        NULL,

        "PAIN."
    }
};

static void SettingsMeun_DrawConfirmation( void )
{

}

static void SettingsMenu_ApplyChanges( void )
{
    ImGui::NewLine();
    ImGui::NewLine();
    ImGui::NewLine();

    if (!menu.settings.modified) {
        ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4( 0.1f, 0.1f, 0.1f, 1.0f ));
    }

    if (ImGui::Button( "APPLY CHANGES" )) {
        if (menu.settings.modified) {
            if (Cvar_VariableInteger( "g_mouseAcceleration" ) != (int32_t)menu.settings.mouseAccelerate) {
                Cvar_Set( "g_mouseAcceleration", va( "%i", menu.settings.mouseAccelerate ) );
            }
            if (Cvar_VariableInteger( "g_mouseInvert" ) != (int32_t)menu.settings.mouseInvert) {
                Cvar_Set( "g_mouseInvert", va( "%i", menu.settings.mouseInvert ) );
            }
            if (Cvar_VariableInteger( "r_textureFiltering" ) != (int32_t)3) {
                Cvar_Set( "r_textureFiltering", va( "%i", 3 ) );
            }
        }
    }

    if (!menu.settings.modified) {
        ImGui::PopStyleColor();
    }
}

static const char *KeyToString( uint32_t key )
{
    switch (key) {
    case KEY_LSHIFT: return "Left-Shift";
    case KEY_RSHIFT: return "Right-Shift";
    default:
        break;
    };
    Con_Printf("WARNING: unknown key %i\n", key);
    return "";
}

static void SettingsMenu_RebindKey( void )
{
    if (ImGui::BeginPopupModal( "Rebind Key" )) {
        ImGui::TextUnformatted( "Press Any Key..." );

        for (uint32_t i = 0; i < NUMKEYS; i++) {
            if (Key_IsDown( i )) {
                menu.settings.rebinding = qtrue;
                menu.settings.rebindIndex = i;
                ImGui::CloseCurrentPopup();
            }
        }
        if (ImGui::Button("CANCEL")) {
            menu.settings.rebinding = qfalse;
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}

static bool Menu_Title( const char *label )
{
    const float font_scale = ImGui::GetFont()->Scale;

    ImGui::SetWindowFontScale( font_scale * menu.drawScale );
    if (ImGui::ArrowButton( "##BACK", ImGuiDir_Left )) {
        ui->PopMenu();
        return true;
    }
    ImGui::SameLine();
    ImGui::TextUnformatted( "BACK" );

    ImGui::SetWindowFontScale( font_scale * 3.75f * menu.drawScale );
    ImGui::TextUnformatted( label );
    ImGui::SetWindowFontScale( font_scale * 1.5f * menu.drawScale );

    ImGui::NewLine();
    ImGui::NewLine();

    return false;
}

static bool Menu_Option( const char *label )
{
    ImGui::TextUnformatted( label );
    ImGui::SameLine();
    return ImGui::ArrowButton( label, ImGuiDir_Right );
}

static void SettingsMenu_Draw( void )
{
    if (ImGui::BeginTabBar( " " )) {
        if (ImGui::BeginTabItem( "GRAPHICS" )) {
            menu.state = STATE_GRAPHICS;
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem( "AUDIO" )) {
            menu.state = STATE_AUDIO;
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }
    switch (menu.state) {
    case STATE_GRAPHICS:
        ImGui::BeginTable( " ", 2 );
        {
            ImGui::TableNextColumn();
            ImGui::TextUnformatted( "Anti-Aliasing" );
            ImGui::TableNextColumn();
            if (ImGui::BeginMenu( "Select Anti-Aliasing Type" )) {
                ImGui::EndMenu();
            }

            ImGui::TableNextRow();
            
            ImGui::TableNextColumn();
            ImGui::TextUnformatted( "Texture Filtering" );
            ImGui::TableNextColumn();
            if (ImGui::BeginMenu( "Select Texture Filtering" )) {
                if (ImGui::MenuItem( "Linear" )) {
                }
                if (ImGui::MenuItem( "Nearest" )) {
                }
                if (ImGui::MenuItem( "Bilinear" )) {
                }
                if (ImGui::MenuItem( "Trilinear" )) {
                }
                ImGui::EndMenu();
            }
        }
        ImGui::EndTable();
        break;
    case STATE_AUDIO:
        ImGui::SeparatorText( "Sound Effects" );
        if (ImGui::Checkbox( "ON##SfxOn", &menu.settings.sfxOn )) {
            menu.settings.modified = qtrue;
        }
        ImGui::SameLine();
        if (ImGui::SliderFloat( "VOLUME##SfxVolume", &menu.settings.sfxVol, 0.0f, 100.0f )) {
            menu.settings.modified = qtrue;
        }

        ImGui::SeparatorText( "Music" );
        if (ImGui::Checkbox( "ON##MusicOn", &menu.settings.musicOn )) {
            menu.settings.modified = qtrue;
        }
        ImGui::SameLine();
        if (ImGui::SliderFloat( "VOLUME##MusicVolume", &menu.settings.musicVol, 0.0f, 100.0f )) {
            menu.settings.modified = qtrue;
        }
        break;
    case STATE_CONTROLS:
        break;
    };

    SettingsMenu_ApplyChanges();
}

static void MainMenu_Draw( void )
{
    uint64_t i;
    const int windowFlags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse |
                            ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBackground;

    if (Key_IsDown( KEY_F2 )) {
        menu.noMenu = qtrue;
    }

    Snd_SetLoopingTrack( menu.ambience );

    if (menu.noMenu) {
        return; // just the scenery & the music (a bit like Halo 3: ODST, check out halome.nu)...
    }

    ImGui::Begin( "MainMenu", NULL, windowFlags );
    ImGui::SetWindowPos( ImVec2( 0, 0 ) );
    ImGui::SetWindowSize( ImVec2( (float)menu.menuWidth / 2, (float)menu.menuHeight ) );
    if (menu.state == STATE_MAIN) {
        Menu_Title( "MAIN MENU" );

        if (Menu_Option( "Single Player" )) {
            menu.state = STATE_SINGLEPLAYER;
            ui->PushMenu( &menu.sp.menu );
        }
        if (Menu_Option( "Settings" )) {
            menu.state = STATE_GRAPHICS;
            ui->PushMenu( &menu.settings.menu );
        }
        if (Menu_Option( "Exit To Title Screen" )) {
            ui->PopMenu();
        }
        if (Menu_Option( "Exit To Desktop" )) {
            // TODO: possibly add in a DOOM-like exit popup?
            Sys_Exit( 1 );
        }
    }
    else if (menu.state >= STATE_SINGLEPLAYER && menu.state <= STATE_PLAYMISSION) {
        switch (menu.state) {
        case STATE_SINGLEPLAYER:
            if (Menu_Title( "SINGLE PLAYER" )) {
                menu.state = STATE_MAIN;
            }
            
            if (Menu_Option( "New Game" )) {
                menu.state = STATE_NEWGAME;
                menu.sp.hardestIndex = rand() % arraylen(difHardestTitles);
                ui->PushMenu( &menu.sp.newgame );
            }
            if (Menu_Option( "Load Game" )) {
                menu.state = STATE_LOADGAME;
                ui->PushMenu( &menu.sp.loadgame );
            }
            if (Menu_Option( "Play Mission (COMING SOON!)" )) { // play any mission found inside the current BFF loaded
//                menu.state = STATE_PLAYMISSION;
            }

            break;
        case STATE_NEWGAME: {
            const char *difName;

            if (Menu_Title( "NEW GAME" )) {
                menu.state = STATE_SINGLEPLAYER;
            }

            ImGui::TextUnformatted( "Save Name: " );
            ImGui::SameLine();
            ImGui::InputText( " ", menu.sp.name, sizeof(menu.sp.name) );

            if (strchr( menu.sp.name, '/' ) || strchr( menu.sp.name, '\\' )) {
                N_strncpyz( menu.sp.name, COM_SkipPath( menu.sp.name ), sizeof(menu.sp.name) );
            }

            if (menu.sp.diff == DIF_HARDEST) {
                difName = difHardestTitles[ menu.sp.hardestIndex ];
            }
            else {
                difName = difficultyTable[ menu.sp.diff ].name;
            }

            if (ImGui::BeginMenu( va("Select Difficulty (Current: %s)", difName) )) {
                for (i = 0; i < NUMDIFS; i++) {
                    if (i != DIF_HARDEST) {
                        if (ImGui::MenuItem( difficultyTable[ i ].name )) {
                            menu.sp.diff = (gamedif_t)i;
                        }
                    }
                    else {
                        if (ImGui::MenuItem( difHardestTitles[ menu.sp.hardestIndex ] )) {
                            menu.sp.diff = (gamedif_t)i;
                        }
                    }

                    if (ImGui::IsItemHovered( ImGuiHoveredFlags_AllowWhenDisabled )) {
                        ImGui::SetItemTooltip( difficultyTable[i].tooltip );
                    }
                }

                ImGui::EndMenu();
            }

            break; }
        case STATE_LOADGAME: {
            if (Menu_Title( "LOAD GAME" )) {
                menu.state = STATE_SINGLEPLAYER;
            }

            if (menu.sp.numSaves) {
                ImGui::BeginTable( "Save Slots", 5 );

                // TODO: add key here

                for (i = 0; i < menu.sp.numSaves; i++) {
                    ImGui::TableNextColumn();
                    ImGui::Text( "%-64s", menu.sp.saveList[i].name );
                    ImGui::TableNextColumn();
                    ImGui::Text( "%-8lu", menu.sp.saveList[i].stats.ctime ); // creation time
                    ImGui::TableNextColumn();
                    ImGui::Text( "%-8lu", menu.sp.saveList[i].stats.mtime ); // last used time
                    ImGui::TableNextColumn();
                    ImGui::Text( "%-64s", menu.sp.saveList[i].gd.bffName );
                    ImGui::TableNextColumn();
                    ImGui::Text( "%-12s", difficultyTable[ menu.sp.saveList[i].gd.diff ].name );

                    ImGui::TableNextRow();
                }
                ImGui::EndTable();
            }
            else {
                ImGui::TextUnformatted( "No Saves" );
            }

            break; }
        };
    }
    else if (menu.state >= STATE_SETTINGS && menu.state <= STATE_AUDIO) {
        if (Menu_Title( "SETTINGS" )) {
            menu.state = STATE_MAIN;
        }
        else {
            SettingsMenu_Draw();
        }
    }
    else if (menu.state == STATE_CREDITS) {
        Menu_Title( "CREDITS" );

        ImGui::TextUnformatted( creditsString );
    }
    else {
        N_Error(ERR_DROP, "Inavlid UI State"); // should NEVER happen
    }
    ImGui::End();
}

static void SinglePlayerMenu_Cache( void )
{
    char **fileList;
    saveinfo_t *info;

    fileList = FS_ListFiles( "savedata/", ".ngd", &menu.sp.numSaves );

    if (menu.sp.numSaves) {
        menu.sp.saveList = (saveinfo_t *)Hunk_Alloc( sizeof(saveinfo_t) * menu.sp.numSaves, h_high );
        memset( menu.sp.saveList, 0, sizeof(saveinfo_t) * menu.sp.numSaves );
        info = menu.sp.saveList;
    }

    for (uint64_t i = 0; i < menu.sp.numSaves; i++, info++) {
        N_strncpyz( info->name, fileList[i], sizeof(info->name) );

        info->index = i;
        if (!Sys_GetFileStats( &info->stats, info->name )) { // this should never fail
            N_Error(ERR_DROP, "Failed to stat savefile '%s' even though it exists", info->name);
        }

        if (!menu.sp.sv.LoadPartial( info->name, &info->gd )) { // just get the header and basic game information
            Con_Printf( COLOR_YELLOW "WARNING: Failed to get valid header data from savefile '%s'\n", info->name );
            info->valid = qfalse;
        }
        else {
            info->valid = qtrue;
        }
    }

    Sys_FreeFileList( fileList );
}

void MainMenu_Cache( void )
{
    memset( &menu, 0, sizeof(menu) );

    // only use of rand() is determining DIF_HARDEST title
    srand(time(NULL));

    // setup base values
    Cvar_Get( "g_mouseAcceleration", "0", CVAR_LATCH | CVAR_SAVE );
    Cvar_Get( "g_mouseInvert", "0", CVAR_LATCH | CVAR_SAVE );

    SinglePlayerMenu_Cache();

    menu.menu.Draw = MainMenu_Draw;
    menu.settings.menu.Draw = MainMenu_Draw;
    menu.sp.menu.Draw = MainMenu_Draw;
    menu.sp.loadgame.Draw = MainMenu_Draw;
    menu.sp.newgame.Draw = MainMenu_Draw;

    menu.drawScale = Cvar_VariableFloat( "r_customWidth" ) / Cvar_VariableFloat( "r_customHeight" );

    menu.settings.sfxOn = Cvar_VariableInteger( "snd_sfxon" );
    menu.settings.musicOn = Cvar_VariableInteger( "snd_musicon" );
    menu.settings.sfxVol = Cvar_VariableFloat( "snd_sfxvol" );
    menu.settings.musicVol = Cvar_VariableFloat( "snd_musicvol" );

    menu.spString = strManager->ValueForKey("MENU_MAIN_SP_STRING");
    menu.settingsString = strManager->ValueForKey("MENU_MAIN_SETTINGS_STRING");
    
    menu.settings.api = R_OPENGL;
    menu.settings.mouseAccelerate = Cvar_VariableInteger("g_mouseAcceleration");
    menu.settings.mouseInvert = Cvar_VariableInteger("g_mouseInvert");

    menu.ambience = Snd_RegisterTrack( "track00.ogg" );

    menu.noMenu = qfalse;
    menu.menuHeight = ui->GetConfig().vidHeight;
    menu.menuWidth = ui->GetConfig().vidWidth;
    menu.state = STATE_MAIN;
}

void UI_MainMenu( void )
{
    MainMenu_Cache();

    ui->PushMenu( &menu.menu );
}
