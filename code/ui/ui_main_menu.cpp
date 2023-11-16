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

typedef struct
{
    CUIMenu menu;

    const char *texdetailString;
    const char *texfilterString;

    // temp cvar data
    keybind_t keybinds[NUMBINDS];
    bool mouseAccelerate;
    bool mouseInvert;
    float mouseSensitivity;
    uint32_t rebindIndex;
    qboolean rebinding;
    bool musicOn;
    bool sfxOn;
    float musicVol;
    float sfxVol;
    renderapi_t api;
    bool fullscreen;
    bool extensions;
    textureFilter_t texfilter;
    textureDetail_t texdetail;
    uint32_t texquality;
    uint32_t geometrydetail;
    int32_t videoMode;
    uint32_t lighting;
    int customWidth;
    int customHeight;
    uint32_t anisotropicFilteringTmp;
    uint32_t anisotropicFiltering;

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
    qboolean nameIssuePopup;

    const stringHash_t *newGame;
    const stringHash_t *loadGame;

    CGameArchive sv;
} singleplayer_t;

typedef struct
{
    CUIMenu menu;
    CUIMenu creditsMenu;

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

    CToggleKey noMenuToggle;

    menustate_t state;

    qboolean escapeToggle;
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

typedef struct {
    const char *label;
    const vidmode_t *mode;
} vidmodeSetting_t;

static const char *vidmodeSettings[NUMVIDMODES+2] = {
    { "Native Resolution (%ix%i)",  }, //NULL },
    { "Custom Resolution (%ix%i)",  }, //NULL },
    { "320x240",                    }, //&r_vidModes[ VIDMODE_320x240 ] },
    { "640x480",                    }, //&r_vidModes[ VIDMODE_640x480 ] },
    { "800x600",                    }, //&r_vidModes[ VIDMODE_800x600 ] },
    { "1024x768",                   }, //&r_vidModes[ VIDMODE_1024x768 ] },
    { "2048x1536",                  }, //&r_vidModes[ VIDMODE_2048x1536 ] },
    { "1280x720",                   }, //&r_vidModes[ VIDMODE_1280x720 ] },
    { "1600x900",                   }, //&r_vidModes[ VIDMODE_1600x900 ] },
    { "1920x1080",                  }, //&r_vidModes[ VIDMODE_1920x1080 ] },
    { "3840x2160",                  }, //&r_vidModes[ VIDMODE_3840x2160 ] },
};

static const char *anisotropicFilters[] = {
    "2x",
    "4x",
    "8x",
    "16x",
    "32x"
};

static void SettingsMeun_DrawConfirmation( void )
{

}

const char *RenderAPIString( renderapi_t api )
{
    switch (api) {
    case R_OPENGL: return "OpenGL";
    case R_SDL2: return "SDL2 (Software Rendering)";
    case R_VULKAN: return "Vulkan";
    default:
        break;
    };
    N_Error( ERR_FATAL, "Invalid render api %i", (int)api );

    // silence compiler warning
    return NULL;
}

static void SettingsMenu_ApplyChanges( void )
{
    const ImVec2 mousePos = ImGui::GetCursorScreenPos();
    char *comingSoonList[256];
    uint32_t comingSoonListSize;

    if (!menu.settings.modified) {
        ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4( 0.1f, 0.1f, 0.1f, 1.0f ));
    }

    comingSoonListSize = 0;
    ImGui::SetCursorScreenPos(ImVec2( 0, (float)ui->GetConfig().vidHeight - 20 ));
    if (ImGui::Button( "APPLY CHANGES" )) {
        if (menu.settings.modified) {
            Cvar_Set( "r_mode", va("%i", menu.settings.videoMode) );
            switch (menu.settings.videoMode) {
            case -2:
                Cvar_Set( "r_customWidth", va( "%i", gi.desktopWidth ) );
                Cvar_Set( "r_customHeight", va( "%i", gi.desktopHeight ) );
                break;
            case -1:
                Cvar_Set( "r_customWidth", va( "%i", menu.settings.customWidth ) );
                Cvar_Set( "r_customHeight", va( "%i", menu.settings.customHeight ) );
                break;
            default:
                Cvar_Set( "r_customWidth", va( "%i", r_vidModes[ menu.settings.videoMode ].width ) );
                Cvar_Set( "r_customWidth", va( "%i", r_vidModes[ menu.settings.videoMode ].height ) );
                break;
            };
        }
    }
    ImGui::SetCursorScreenPos( mousePos );

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
    if (ImGui::ArrowButton( va("##BACK%s", label), ImGuiDir_Left )) {
        return true;
    }
    ImGui::SameLine();
    ImGui::TextUnformatted( "BACK" );

    ImGui::SetWindowFontScale( font_scale * 3.75f * menu.drawScale );
    ImGui::TextUnformatted( label );
    ImGui::SetWindowFontScale( font_scale * 1.5f * menu.drawScale );

    return false;
}

static bool Menu_Option( const char *label )
{
    bool retn;

    ImGui::TableNextColumn();
    ImGui::TextUnformatted( label );
    ImGui::TableNextColumn();
    ImGui::SameLine();
    retn = ImGui::ArrowButton( label, ImGuiDir_Right );

    return retn;
}

const char *TexDetailString( textureDetail_t detail )
{
    switch (detail) {
    case TexDetail_MSDOS: return "MS-DOS (VGA)";
    case TexDetail_IntegratedGPU: return "Integrated GPU";
    case TexDetail_Normie: return "Normie";
    case TexDetail_ExpensiveShitWeveGotHere: return "Expensive Shit We've Got Here";
    case TexDetail_GPUvsGod: return "GPU vs God";
    default:
        break;
    };
    N_Error( ERR_FATAL, "Invalid texture detail %i", (int)detail );

    // silence compiler warning
    return NULL;
}

const char *TexFilterString( textureFilter_t filter )
{
    switch (filter) {
    case TexFilter_Linear: return "Linear";
    case TexFilter_Nearest: return "Nearest";
    case TexFilter_Bilinear: return "Bilinear";
    case TexFilter_Trilinear: return "Trilinear";
    default:
        break;
    };
    N_Error( ERR_FATAL, "Invalid texture filter %i", (int)filter );
    
    // silence compiler warning
    return NULL;
}

static void EscapeMenuToggle( menustate_t newstate )
{
    if (Key_IsDown( KEY_ESCAPE )) {
        if (menu.escapeToggle) {
            menu.escapeToggle = qfalse;
            menu.state = newstate;

            const char *menuName;

            switch (newstate) {
            case STATE_MAIN:
                menuName = "MAIN";
                break;
            case STATE_SINGLEPLAYER:
                menuName = "SINGLEPLAYER";
                break;
            case STATE_NEWGAME:
                menuName = "NEWGAME";
                break;
            case STATE_LOADGAME:
                menuName = "LOADGAME";
                break;
            case STATE_PLAYMISSION:
                menuName = "PLAYMISSION";
                break;
            case STATE_SETTINGS:
                menuName = "SETTINGS";
                break;
            case STATE_GRAPHICS:
                menuName = "GRAPHICS";
                break;
            case STATE_CONTROLS:
                menuName = "CONTROLS";
                break;
            case STATE_AUDIO:
                menuName = "AUDIO";
                break;
            case STATE_CREDITS:
                menuName = "CREDITS";
                break;
            };
            Con_Printf( "Setting menu state to %s\n", menuName );
        }
    }
    else {
        menu.escapeToggle = qtrue;
    }
}

static void NewGame_DrawNameIssue( void )
{
    if (ImGui::BeginPopupModal( "Save Slot Name Issue" )) {
        ImGui::Text(
            "Sorry, but it looks like your save file name is either too long or already exists\n"
            "if its too long, it must be less than or equal to %lu characters in length, please\n"
            "shorten your save's name.\n"
            "if it already exists, then please rename the save slot.\n"
            "\n"
            "Sorry for the inconvenience. :)\n"
            "\n"
            "Your Resident Fiend,\n"
            "Noah Van Til\n"
        , sizeof(menu.sp.name) - 1);

        if (ImGui::Button( "OK" )) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}

static void MainMenu_Draw( void )
{
    uint64_t i;
    const int windowFlags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse |
                            ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBackground;

    menu.noMenuToggle.Toggle( KEY_F2, menu.noMenu );

    Snd_SetLoopingTrack( menu.ambience );

    if (menu.noMenu) {
        return; // just the scenery & the music (a bit like Halo 3: ODST, check out halome.nu)...
    }

    if (menu.sp.nameIssuePopup) {
        NewGame_DrawNameIssue();
    }

    ImGui::Begin( "MainMenu", NULL, windowFlags );
    ImGui::SetWindowPos( ImVec2( 0, 0 ) );
    ImGui::SetWindowSize( ImVec2( (float)menu.menuWidth / 2, (float)menu.menuHeight ) );
    if (menu.state == STATE_MAIN) {
        Menu_Title( "MAIN MENU" );

        const ImVec2 mousePos = ImGui::GetCursorScreenPos();
        ImGui::SetCursorScreenPos( ImVec2( mousePos.x, mousePos.y + 10 ) );

        ImGui::BeginTable( " ", 2 );
        if (Menu_Option( "Single Player" )) {
            menu.state = STATE_SINGLEPLAYER;
        }
        ImGui::TableNextRow();
        if (Menu_Option( "Settings" )) {
            menu.state = STATE_SETTINGS;
        }
        ImGui::TableNextRow();
        if (Menu_Option( "Credits" )) {
            menu.state = STATE_CREDITS;
        }
        ImGui::TableNextRow();
        if (Menu_Option( "Exit To Title Screen" )) {
            ui->PopMenu();
        }
        ImGui::TableNextRow();
        if (Menu_Option( "Exit To Desktop" )) {
            // TODO: possibly add in a DOOM-like exit popup?
            Sys_Exit( 1 );
        }
        ImGui::EndTable();
    }
    else if (menu.state >= STATE_SINGLEPLAYER && menu.state <= STATE_PLAYMISSION) {
        ImVec2 mousePos;

        switch (menu.state) {
        case STATE_SINGLEPLAYER:
            EscapeMenuToggle( STATE_MAIN );

            if (Menu_Title( "SINGLE PLAYER" )) {
                menu.state = STATE_MAIN;
            }

            mousePos = ImGui::GetCursorScreenPos();
            ImGui::SetCursorScreenPos( ImVec2( mousePos.x, mousePos.y + 10 ) );
            
            ImGui::BeginTable( " ", 2 );
            if (Menu_Option( "New Game" )) {
                menu.state = STATE_NEWGAME;
                menu.sp.hardestIndex = rand() % arraylen(difHardestTitles);
            }
            ImGui::TableNextRow();
            if (Menu_Option( "Load Game" )) {
                menu.state = STATE_LOADGAME;
            }
            ImGui::TableNextRow();
            if (Menu_Option( "Play Mission (COMING SOON!)" )) { // play any mission found inside the current BFF loaded
//                menu.state = STATE_PLAYMISSION;
            }
            ImGui::EndTable();

            break;
        case STATE_NEWGAME: {
            EscapeMenuToggle( STATE_SINGLEPLAYER );

            const char *difName;

            if (Menu_Title( "NEW GAME" )) {
                menu.state = STATE_SINGLEPLAYER;
            }

            mousePos = ImGui::GetCursorScreenPos();
            ImGui::SetCursorScreenPos( ImVec2( mousePos.x, mousePos.y + 10 ) );

            if (menu.sp.diff == DIF_HARDEST) {
                difName = difHardestTitles[ menu.sp.hardestIndex ];
            }
            else {
                difName = difficultyTable[ menu.sp.diff ].name;
            }

            ImGui::BeginTable( " ", 2 );
            {
                ImGui::TableNextColumn();
                ImGui::TextUnformatted( "Save Name" );
                ImGui::TableNextColumn();
                if (ImGui::InputText( " ", menu.sp.name, sizeof(menu.sp.name), ImGuiInputTextFlags_EscapeClearsAll )) {
                    // make sure it's an absolute path
                    if (strchr( menu.sp.name, '/' ) || strchr( menu.sp.name, '\\' )) {
                        N_strncpyz( menu.sp.name, COM_SkipPath( menu.sp.name ), sizeof(menu.sp.name) );
                    }

                    // make sure its a unique name, so we don't get filename collisions
                    for (i = 0; i < menu.sp.numSaves; i++) {
                        if (!N_stricmp( menu.sp.name, menu.sp.saveList[i].name )) {
                            menu.sp.nameIssuePopup = qtrue;
                            ImGui::OpenPopup( "Save Slot Name Issue" );
                        }
                    }
                }
                
                ImGui::TableNextRow();

                ImGui::TableNextColumn();
                ImGui::TextUnformatted( "Difficulty" );
                ImGui::TableNextColumn();
                if (ImGui::BeginMenu( va("%s", difName) )) {
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
                            ImGui::BeginTooltip();
                            ImGui::TextUnformatted( difficultyTable[i].tooltip );
                            ImGui::EndTooltip();
                        }
                    }
                    ImGui::EndMenu();
                }
            }
            ImGui::EndTable();

            ImGui::NewLine();
            ImGui::NewLine();
            ImGui::NewLine();
            ImGui::NewLine();
            ImGui::NewLine();

            if (ImGui::Button( "Open To a Fresh Chapter" )) {

            }
            break; }
        case STATE_LOADGAME: {
            EscapeMenuToggle( STATE_SINGLEPLAYER );
            if (Menu_Title( "LOAD GAME" )) {
                menu.state = STATE_SINGLEPLAYER;
            }

            mousePos = ImGui::GetCursorScreenPos();
            ImGui::SetCursorScreenPos( ImVec2( mousePos.x, mousePos.y + 10 ) );

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
        /*
        if (ImGui::BeginTabBar( " " )) {
            if (ImGui::BeginTabItem( "GRAPHICS" )) {
                menu.state = STATE_GRAPHICS;
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem( "AUDIO" )) {
                menu.state = STATE_AUDIO;
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem( "CONTROLS" )) {
                menu.state = STATE_CONTROLS;
                ImGui::EndTabItem();
            }
            ImGui::EndTabBar();
        } */
        switch (menu.state) {
        case STATE_SETTINGS:
            EscapeMenuToggle( STATE_MAIN );
            if (menu.state != STATE_SETTINGS) {
                break;
            }
            else if (Menu_Title( "SETTINGS" )) {
                menu.state = STATE_MAIN;
                break;
            }
            ImGui::BeginTable( " ", 2 );
            ImGui::TableNextRow();
            if (Menu_Option( "Graphics" )) {
                menu.state = STATE_GRAPHICS;
            }
            ImGui::TableNextRow();
            if (Menu_Option( "Audio" )) {
                menu.state = STATE_AUDIO;
            }
            ImGui::TableNextRow();
            if (Menu_Option( "Controls" )) {
                menu.state = STATE_CONTROLS;
            }
            ImGui::EndTable();
            break;
        case STATE_GRAPHICS:
            EscapeMenuToggle( STATE_SETTINGS );
            if (menu.state != STATE_GRAPHICS) {
                break;
            }
            else if (Menu_Title( "GRAPHICS" )) {
                menu.state = STATE_SETTINGS;
                break;
            }

            ImGui::BeginTable( " ", 2 );
            {
                const char *vidMode;

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
                if (ImGui::BeginMenu( menu.settings.texfilterString )) {
                    for (i = 0; i < NumTexFilters; i++) {
                        if (ImGui::MenuItem( TexFilterString( (textureFilter_t)((int)TexFilter_Linear + i) ) )) {
                            menu.settings.modified = qtrue;
                            menu.settings.texfilter = (textureFilter_t)((int)TexFilter_Linear + i);
                        }
                    }
                    ImGui::EndMenu();
                }

                ImGui::TableNextRow();

                ImGui::TableNextColumn();
                ImGui::TextUnformatted( "Texture Detail" );
                ImGui::TableNextColumn();
                if (ImGui::BeginMenu( menu.settings.texdetailString )) {
                    for (i = 0; i < NumTexFilters; i++) {
                        if (ImGui::MenuItem( TexFilterString( (textureFilter_t)((int)TexFilter_Linear + i) ) )) {
                            menu.settings.modified = qtrue;
                            menu.settings.texfilter = (textureFilter_t)((int)TexFilter_Linear + i);
                        }
                    }
                    ImGui::EndMenu();
                }

                ImGui::TableNextRow();

                ImGui::TableNextColumn();
                ImGui::TextUnformatted( "Anisotropic Filtering" );
                ImGui::TableNextColumn();
                if (ImGui::ArrowButton( "##ANIFILTER_LEFT", ImGuiDir_Left )) {
                    menu.settings.anisotropicFiltering = (int32_t)menu.settings.anisotropicFiltering - 1 == -1 ? arraylen(anisotropicFilters) - 1
                        : menu.settings.anisotropicFiltering - 1;
                    if (Cvar_VariableInteger( "r_anistropicFiltering" ) != (int32_t)menu.settings.anisotropicFiltering) {
                        menu.settings.modified = qtrue;
                    }
                }
                ImGui::SameLine();
                ImGui::TextUnformatted( anisotropicFilters[ menu.settings.anisotropicFiltering ] );
                ImGui::SameLine();
                if (ImGui::ArrowButton( "##ANIFILTER_RIGHT", ImGuiDir_Right )) {
                    menu.settings.anisotropicFiltering = menu.settings.anisotropicFiltering + 1 >= arraylen(anisotropicFilters) ? 0 : menu.settings.anisotropicFiltering + 1;
                    if (Cvar_VariableInteger( "r_anistropicFiltering" ) != (int32_t)menu.settings.anisotropicFiltering) {
                        menu.settings.modified = qtrue;
                    }
                }

                ImGui::TableNextRow();

                switch (menu.settings.videoMode) {
                case -2:
                    vidMode = vidmodeSettings[0];
                    break;
                case -1:
                    vidMode = vidmodeSettings[1];
                    break;
                default:
                    vidMode = r_vidModes[ menu.settings.videoMode ].description;
                    break;
                };

                ImGui::TableNextColumn();
                ImGui::TextUnformatted( "Video Mode" );
                ImGui::TableNextColumn();
                if (ImGui::BeginMenu( va("%s##VIDMODE", vidMode) )) {
                    for (i = 0; i < NUMVIDMODES; i++) {
                        if (i == 0 || i == 1) {
                            if (ImGui::MenuItem( va( vidmodeSettings[i], r_customWidth->i, r_customHeight->i ) )) {
                                menu.settings.modified = menu.settings.videoMode != i;
                                menu.settings.videoMode = i;
                            }
                            continue;
                        }
                        if (ImGui::MenuItem( vidmodeSettings[ i ] )) {
                            menu.settings.modified = menu.settings.videoMode != i;
                            menu.settings.videoMode = i;
                        }
                    }
                    ImGui::EndMenu();
                }

                ImGui::TableNextRow();

                ImGui::TableNextColumn();
                ImGui::TextUnformatted( "Rendering API" );
                ImGui::TableNextColumn();
                if (ImGui::BeginMenu( RenderAPIString( menu.settings.api ) )) {
                    if (ImGui::MenuItem( RenderAPIString( R_OPENGL ) )) {
                        menu.settings.modified = menu.settings.api != R_OPENGL;
                        menu.settings.api = R_OPENGL;
                    }
                    if (ImGui::MenuItem( va("%s (COMING SOON!)", RenderAPIString( R_SDL2 )) )) {
                        menu.settings.modified = menu.settings.api != R_SDL2;
                        menu.settings.api = R_SDL2;
                    }
                    if (ImGui::MenuItem( va("%s (COMING SOON!)", RenderAPIString( R_VULKAN )) )) {
                        menu.settings.modified = menu.settings.api != R_VULKAN;
                        menu.settings.api = R_VULKAN;
                    }
                    ImGui::EndMenu();
                }

                ImGui::TableNextRow();

                ImGui::TableNextColumn();
                ImGui::TextUnformatted( "Fullscreen" );
                ImGui::TableNextColumn();
                if (ImGui::Checkbox( menu.settings.fullscreen ? "ON" : "OFF", &menu.settings.fullscreen )) {
                    menu.settings.modified = qtrue;
                }

                ImGui::TableNextRow();

                ImGui::TableNextColumn();
                ImGui::TextUnformatted( "Lighting" );
                ImGui::TableNextColumn();
                ImGui::TextUnformatted( "COMING SOON!" );
            }
            ImGui::EndTable();
            break;
        case STATE_AUDIO:
            EscapeMenuToggle( STATE_SETTINGS );
            if (menu.state != STATE_AUDIO) {
                break;
            }
            else if (Menu_Title( "AUDIO" )) {
                menu.state = STATE_SETTINGS;
                break;
            }

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
        case STATE_CONTROLS: {
            EscapeMenuToggle( STATE_SETTINGS );
            if (menu.state != STATE_CONTROLS) {
                break;
            }
            else if (Menu_Title( "CONTROLS" )) {
                menu.state = STATE_SETTINGS;
                break;
            }

            // mouse options
            ImGui::SeparatorText( "MOUSE" );
            ImGui::BeginTable( " ", 2 );
            {
                ImGui::TableNextColumn();
                ImGui::TextUnformatted( "Sensitivity" );
                ImGui::TableNextColumn();
                if (ImGui::SliderFloat( " ", &menu.settings.mouseSensitivity, 0, 100.0f )) {
                }

                ImGui::TableNextRow();

                ImGui::TableNextColumn();
                ImGui::TextUnformatted( "Invert Mouse" );
                ImGui::TableNextColumn();
                if (ImGui::Checkbox( menu.settings.mouseInvert ? "ON" : "OFF", &menu.settings.mouseInvert )) {
                    menu.settings.modified = qtrue;
                }

                ImGui::TableNextRow();

                ImGui::TableNextColumn();
                ImGui::TextUnformatted( "Mouse Acceleration" );
                ImGui::TableNextColumn();
                if (ImGui::Checkbox( menu.settings.mouseAccelerate ? "ON" : "OFF", &menu.settings.mouseAccelerate )) {
                    menu.settings.modified = qtrue;
                }
            }
            ImGui::EndTable();

            // keybinding options
            break; }
        };

        SettingsMenu_ApplyChanges();
    }
    else if (menu.state == STATE_CREDITS) {
        EscapeMenuToggle( STATE_MAIN );
        if (Menu_Title( "CREDITS" )) {
            menu.state = STATE_MAIN;
        }

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
    menu.creditsMenu.Draw = MainMenu_Draw;

    menu.drawScale = Cvar_VariableFloat( "r_customWidth" ) / Cvar_VariableFloat( "r_customHeight" );

    menu.settings.texdetail = (textureDetail_t)Cvar_VariableInteger( "r_textureDetail" );
    menu.settings.texfilter = (textureFilter_t)Cvar_VariableInteger( "r_textureFiltering" );
    menu.settings.texdetailString = TexDetailString( menu.settings.texdetail );
    menu.settings.texfilterString = TexFilterString( menu.settings.texfilter );
    menu.settings.videoMode = Cvar_VariableInteger( "r_mode" );

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
