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

    char **hardestStrings;
    int32_t numHardestStrings;

    const stringHash_t *difficultyDescriptions[NUMDIFS];
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

static dif_t difficultyTable[NUMDIFS];

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
                        if ( ImGui::MenuItem( difficultyTable[ i ].name ) ) {
                            sp.diff = (gamedif_t)i;
                            ui->PlaySelected();
                        }
                    }
                    else {
                        if ( ImGui::MenuItem( sp.hardestStrings[ sp.hardestIndex ] ) ) {
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
            
            Cvar_Set( "mapname", gi.mapCache.infoList[0].info.name );
            g_pModuleLib->ModuleCall( sgvm, ModuleOnLevelStart, 1, 0 ); // start a new game
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
        if ( sp.numSaves ) {
            ImGui::BeginTable( "Save Slots", 6 );
            // TODO: add key here
            for (i = 0; i < sp.numSaves; i++) {
                ImGui::TableNextColumn();
                if ( ImGui::Button( "LOAD" ) ) {
                    Cvar_Set( "sg_savename", sp.saveList[i].name );
                    Cvar_Set( "mapname", gi.mapCache.infoList[i].info.name );
                    g_pModuleLib->ModuleCall( sgvm, ModuleOnLoadGame, 0 );
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

char **parse_csv( const char *line );
int32_t count_fields( const char *line );

void SinglePlayerMenu_Cache( void )
{
    saveinfo_t *info;
    const char **fileList;
    uint64_t i;
    const stringHash_t *hardest;

    memset( &sp, 0, sizeof(sp) );
    
    //
    // init strings
    //
    difficultyTable[DIF_NOOB].name = strManager->ValueForKey( "SP_DIFF_VERY_EASY" )->value;
    difficultyTable[DIF_NOOB].tooltip = strManager->ValueForKey( "SP_DIFF_0_DESC" )->value;

    difficultyTable[DIF_RECRUIT].name = strManager->ValueForKey( "SP_DIFF_EASY" )->value;
    difficultyTable[DIF_RECRUIT].tooltip = strManager->ValueForKey( "SP_DIFF_1_DESC" )->value;

    difficultyTable[DIF_MERC].name = strManager->ValueForKey( "SP_DIFF_MEDIUM" )->value;
    difficultyTable[DIF_MERC].tooltip = strManager->ValueForKey( "SP_DIFF_2_DESC" )->value;

    difficultyTable[DIF_NOMAD].name = strManager->ValueForKey( "SP_DIFF_HARD" )->value;
    difficultyTable[DIF_NOMAD].tooltip = strManager->ValueForKey( "SP_DIFF_3_DESC" )->value;

    difficultyTable[DIF_BLACKDEATH].name = strManager->ValueForKey( "SP_DIFF_VERY_HARD" )->value;
    difficultyTable[DIF_BLACKDEATH].tooltip = strManager->ValueForKey( "SP_DIFF_4_DESC" )->value;

    sp.newGame = strManager->ValueForKey( "SP_NEWGAME" );
    sp.loadGame = strManager->ValueForKey( "SP_LOADGAME" );
    hardest = strManager->ValueForKey( "SP_DIFF_THE_MEMES" );
    sp.hardestStrings = parse_csv( hardest->value );
    sp.numHardestStrings = count_fields( hardest->value );

    //
    // init savefiles
    //

    sp.numSaves = 0;
    fileList = g_pArchiveHandler->GetSaveFiles( &sp.numSaves );

    if ( sp.numSaves ) {
        Cvar_Set( "sg_numSaves", va( "%i", (int32_t)sp.numSaves ) );

        sp.saveList = (saveinfo_t *)Hunk_Alloc( sizeof(saveinfo_t) * sp.numSaves, h_high );
        info = sp.saveList;

        for ( i = 0; i < sp.numSaves; i++, info++ ) {
            N_strncpyz( info->name, fileList[i], sizeof(info->name) );

            info->index = i;
            if ( !Sys_GetFileStats( &info->stats, info->name ) ) { // this should never fail
                N_Error( ERR_DROP, "Failed to stat savefile '%s' even though it exists", info->name );
            }

            if ( !g_pArchiveHandler->LoadPartial( info->name, &info->gd ) ) { // just get the header and basic game information
                Con_Printf( COLOR_YELLOW "WARNING: Failed to get valid header data from savefile '%s'\n", info->name );
                info->valid = qfalse;
            }
            else {
                info->valid = qtrue;
            }
        }
    }
}

//
// a small csv parser for c, credits to semitrivial for this
// https://github.com/semitrivial/csv_parser.git
//

void free_csv_line( char **parsed ) {
    char **ptr;

    for ( ptr = parsed; *ptr; ptr++ ) {
        Z_Free( *ptr );
    }

    Z_Free( parsed );
}

int32_t count_fields( const char *line ) {
    const char *ptr;
    int32_t cnt, fQuote;

    for ( cnt = 1, fQuote = 0, ptr = line; *ptr; ptr++ ) {
        if ( fQuote ) {
            if ( *ptr == '\"' ) {
                fQuote = 0;
            }
            continue;
        }

        switch( *ptr ) {
            case '\"':
                fQuote = 1;
                continue;
            case ',':
                cnt++;
                continue;
            default:
                continue;
        }
    }

    if ( fQuote ) {
        return -1;
    }

    return cnt;
}

static char *CopyUIString( const char *str ) {
    char *out;
    uint64_t len;

    len = strlen( str ) + 1;
    out = (char *)Z_Malloc( len, TAG_GAME );
    N_strncpyz( out, str, len );

    return out;
}

/*
 *  Given a string containing no linebreaks, or containing line breaks
 *  which are escaped by "double quotes", extract a NULL-terminated
 *  array of strings, one for every cell in the row.
 */
char **parse_csv( const char *line ) {
    char **buf, **bptr, *tmp, *tptr;
    const char *ptr;
    int32_t fieldcnt, fQuote, fEnd;

    fieldcnt = count_fields( line );

    if ( fieldcnt == -1 ) {
        return NULL;
    }

    buf = (char **)Z_Malloc( sizeof(char *) * ( fieldcnt + 1 ), TAG_GAME );
    tmp = (char *)Hunk_AllocateTempMemory( strlen( line ) + 1 );

    bptr = buf;

    for ( ptr = line, fQuote = 0, *tmp = '\0', tptr = tmp, fEnd = 0; ; ptr++ ) {
        if ( fQuote ) {
            if ( !*ptr ) {
                break;
            }

            if ( *ptr == '\"' ) {
                if ( ptr[1] == '\"' ) {
                    *tptr++ = '\"';
                    ptr++;
                    continue;
                }
                fQuote = 0;
            }
            else {
                *tptr++ = *ptr;
            }

            continue;
        }

        switch( *ptr ) {
            case '\"':
                fQuote = 1;
                continue;
            case '\0':
                fEnd = 1;
            case ',':
                *tptr = '\0';
                *bptr = CopyUIString( tmp );

                if ( !*bptr ) {
                    for ( bptr--; bptr >= buf; bptr-- ) {
                        Z_Free( *bptr );
                    }
                    Z_Free( buf );
                    Hunk_FreeTempMemory( tmp );

                    return NULL;
                }

                bptr++;
                tptr = tmp;

                if ( fEnd ) {
                    break;
                } else {
                    continue;
                }

            default:
                *tptr++ = *ptr;
                continue;
        }

        if ( fEnd ) {
            break;
        }
    }

    *bptr = NULL;
    Hunk_FreeTempMemory( tmp );
    return buf;
}
