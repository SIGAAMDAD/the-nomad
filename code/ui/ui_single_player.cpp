#include "../game/g_game.h"
#include "ui_public.hpp"
#include "ui_menu.h"
#include "ui_lib.h"
#include "ui_window.h"
#include "ui_string_manager.h"
#include "../game/g_archive.h"

typedef struct {
    char name[MAX_GDR_PATH];
    fileStats_t stats;
    uint64_t index;
    gamedata_t gd;
    qboolean valid;
} saveinfo_t;

typedef struct {
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

extern ImFont *RobotoMono;
static singleplayer_t sp;

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
    "Wish U Had A BFG?",
    "Skill Issue",
    "DAKKA",
    "OOOOF",
    "So sad, too bad."
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

void NewGame_DrawNameIssue( void )
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
        , sizeof(sp.name) - 1);

        if (ImGui::Button( "OK" )) {
            ui->PlaySelected();
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}

void SinglePlayerMenu_Draw( void )
{
    ImVec2 mousePos;
    uint64_t i;
    float font_scale;

    switch (ui->GetState()) {
    case STATE_SINGLEPLAYER:
        ui->EscapeMenuToggle( STATE_MAIN );
        if (ui->GetState() != STATE_SINGLEPLAYER) {
            break;
        }
        else if (ui->Menu_Title( "SINGLE PLAYER" )) {
            ui->SetState( STATE_MAIN );
            break;
        }
        mousePos = ImGui::GetCursorScreenPos();
        ImGui::SetCursorScreenPos( ImVec2( mousePos.x, mousePos.y + 10 ) );
        
        ImGui::BeginTable( " ", 2 );
        if (ui->Menu_Option( "New Game" )) {
            ui->SetState( STATE_NEWGAME );
            sp.hardestIndex = rand() % arraylen(difHardestTitles);
        }
        ImGui::TableNextRow();
        if (ui->Menu_Option( "Load Game" )) {
            ui->SetState( STATE_LOADGAME );
        }
        ImGui::TableNextRow();
        if (ui->Menu_Option( "Play Mission (COMING SOON!)" )) { // play any mission found inside the current BFF loaded
//               ui->SetState( STATE_PLAYMISSION;
        }
        ImGui::EndTable();
        break;
    case STATE_NEWGAME: {

        ImGui::SetWindowSize( ImVec2( (float)ui->GetConfig().vidWidth * 0.75f, ImGui::GetWindowSize().y ) );

        ui->EscapeMenuToggle( STATE_SINGLEPLAYER );
        const char *difName;
        if (ui->GetState() != STATE_NEWGAME) {
            // reset to 0
            memset( sp.name, 0, sizeof(sp.name) );
            sp.diff = DIF_NOOB;
            sp.hardestIndex = 0;
            break;
        }
        else if (ui->Menu_Title( "NEW GAME" )) {
            // reset to 0
            memset( sp.name, 0, sizeof(sp.name) );
            sp.diff = DIF_NOOB;
            sp.hardestIndex = 0;
            ui->SetState( STATE_SINGLEPLAYER );
            break;
        }
        mousePos = ImGui::GetCursorScreenPos();
        ImGui::SetCursorScreenPos( ImVec2( mousePos.x, mousePos.y + 10 ) );
        if (sp.diff == DIF_HARDEST) {
            difName = difHardestTitles[ sp.hardestIndex ];
        }
        else {
            difName = difficultyTable[ sp.diff ].name;
        }
        ImGui::BeginTable( " ", 2 );
        {
            ImGui::TableNextColumn();
            ImGui::TextUnformatted( "Save Name" );
            ImGui::TableNextColumn();
            if (ImGui::InputText( " ", sp.name, sizeof(sp.name), ImGuiInputTextFlags_EscapeClearsAll | ImGuiInputTextFlags_EnterReturnsTrue )) {
                ui->PlaySelected();
                // make sure it's an absolute path
                if ( strchr( sp.name, '/' ) || strchr( sp.name, '\\' ) ) {
                    N_strncpyz( sp.name, COM_SkipPath( sp.name ), sizeof(sp.name) );
                }
                // make sure its a unique name, so we don't get filename collisions
                for (i = 0; i < sp.numSaves; i++) {
                    if (!N_stricmp( sp.name, sp.saveList[i].name )) {
                        ui->SetState( STATE_NAMEISSUE );
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
                            sp.diff = (gamedif_t)i;
                            ui->PlaySelected();
                        }
                    }
                    else {
                        if (ImGui::MenuItem( difHardestTitles[ sp.hardestIndex ] )) {
                            sp.diff = (gamedif_t)i;
                            ui->PlaySelected();
                        }
                    }
                }
                ImGui::EndMenu();
            }
        }
        ImGui::EndTable();

        ImGui::NewLine();

        if ( ImGui::Button( "Open To a Fresh Chapter" ) ) {
            ui->PlaySelected();
            ui->SetState( STATE_NONE );
            ui->SetActiveMenu( UI_MENU_NONE );
            gi.state = GS_LEVEL;
            Key_SetCatcher( 0 );
            Key_ClearStates();

            if ( N_stricmp( Cvar_VariableString( "sg_savename" ), sp.name ) ) {
                Cvar_Set( "sg_savename", sp.name );
            }
            
            VM_Call( sgvm, 0, SGAME_INIT );
            VM_Call( sgvm, 1, SGAME_LOADLEVEL, 0 ); // start a new game
        }

        ImGui::NewLine();
        ImGui::NewLine();

        FontCache()->SetActiveFont( RobotoMono );

        font_scale = ImGui::GetFont()->Scale;
        ImGui::SetWindowFontScale( font_scale * 3.75f );
        ImGui::TextUnformatted( "Difficulty Description" );
        ImGui::SetWindowFontScale( font_scale * 2.75f );
        ImGui::TextWrapped( "%s", difficultyTable[(int32_t)sp.diff].tooltip );
        break; }
    case STATE_LOADGAME: {
        ui->EscapeMenuToggle( STATE_SINGLEPLAYER );
        if (ui->GetState() != STATE_LOADGAME) {
            break;
        }
        else if (ui->Menu_Title( "LOAD GAME" )) {
            ui->SetState( STATE_SINGLEPLAYER );
            break;
        }
        mousePos = ImGui::GetCursorScreenPos();
        ImGui::SetCursorScreenPos( ImVec2( mousePos.x, mousePos.y + 10 ) );
        if (sp.numSaves) {
            ImGui::BeginTable( "Save Slots", 6 );
            // TODO: add key here
            for (i = 0; i < sp.numSaves; i++) {
                ImGui::TableNextColumn();
                if ( ImGui::Button( "LOAD" ) ) {
                    Cvar_Set( "sg_savename", sp.saveList[i].name );
                }
                ImGui::TableNextColumn();
                ImGui::Text( "%s", sp.saveList[i].name );
                ImGui::TableNextColumn();
                ImGui::Text( "%lu", sp.saveList[i].stats.ctime ); // creation time
                ImGui::TableNextColumn();
                ImGui::Text( "%lu", sp.saveList[i].stats.mtime ); // last used time
                ImGui::TableNextColumn();
                ImGui::Text( "%s", sp.saveList[i].gd.bffName );
                ImGui::TableNextColumn();
                ImGui::Text( "%s", difficultyTable[ sp.saveList[i].gd.diff ].name );
                ImGui::TableNextRow();
            }
            ImGui::EndTable();
        }
        else {
            ImGui::TextUnformatted( "No Saves" );
        }
        break; }
    };

    ImGui::End();
}

void SinglePlayerMenu_Cache( void )
{
    char **fileList;
    saveinfo_t *info;

    memset( &sp, 0, sizeof(sp) );

    fileList = FS_ListFiles( "savedata/", ".ngd", &sp.numSaves );

    if (sp.numSaves) {
        sp.saveList = (saveinfo_t *)Hunk_Alloc( sizeof(saveinfo_t) * sp.numSaves, h_low );
        memset( sp.saveList, 0, sizeof(saveinfo_t) * sp.numSaves );
        info = sp.saveList;
    }

    for (uint64_t i = 0; i < sp.numSaves; i++, info++) {
        N_strncpyz( info->name, fileList[i], sizeof(info->name) );

        info->index = i;
        if (!Sys_GetFileStats( &info->stats, info->name )) { // this should never fail
            N_Error(ERR_DROP, "Failed to stat savefile '%s' even though it exists", info->name);
        }

        if (!sp.sv.LoadPartial( info->name, &info->gd )) { // just get the header and basic game information
            Con_Printf( COLOR_YELLOW "WARNING: Failed to get valid header data from savefile '%s'\n", info->name );
            info->valid = qfalse;
        }
        else {
            info->valid = qtrue;
        }
    }

    Sys_FreeFileList( fileList );
}
