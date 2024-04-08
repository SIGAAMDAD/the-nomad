#include "ui_lib.h"

typedef struct {
    menuframework_t menu;
    
    menufield_t saveName;
    menubutton_t beginGame;
    menulist_t difList;

    gamedif_t diff;

    char **hardestStrings;
    int64_t numHardestStrings;
    int64_t hardestIndex;

    const stringHash_t *title;
    const stringHash_t *newGameSaveNamePrompt;
    const stringHash_t *newGameBegin;
} newGameMenu_t;

#define ID_BEGINGAME        1
#define ID_SAVENAMEPROMPT   2
#define ID_DIFFICULTY       3

static newGameMenu_t newGame;
extern ImFont *RobotoMono;

static void BeginNewGame( void )
{
    memset( newGame.saveName.buffer, 0, sizeof( newGame.saveName.buffer ) );

    UI_ForceMenuOff();
    ui->menusp = 0;
    ui->activemenu = NULL;

    gi.state = GS_LEVEL;

    Cvar_Set( "sgame_SaveName", newGame.saveName.buffer );
    Cvar_SetIntegerValue( "g_levelIndex", 0 );
    Cvar_Set( "mapname", *gi.mapCache.mapList );

    // start a new game
    g_pModuleLib->ModuleCall( sgvm, ModuleOnLevelStart, 0 );
}

static void NewGameMenu_EventCallback( void *ptr, int event )
{
    if ( event != EVENT_ACTIVATED ) {
        return;
    }

    switch ( ( (menucommon_t *)ptr )->id ) {
    case ID_SAVENAMEPROMPT:
        break;
    case ID_BEGINGAME:
        BeginNewGame();
        break;
    case ID_DIFFICULTY:
        newGame.diff = (gamedif_t)newGame.difList.curitem;
        break;
    };
}

static void NewGameMenu_Draw( void )
{
    float font_scale;

    Menu_Draw( &newGame.menu );

    FontCache()->SetActiveFont( RobotoMono );

    ImGui::Begin( "DifficultyDescription", NULL, MENU_DEFAULT_FLAGS );
    font_scale = ImGui::GetFont()->Scale;
    ImGui::SetWindowFontScale( font_scale * 3.75f );
    ImGui::TextUnformatted( "Difficulty Description" );
    ImGui::SetWindowFontScale( font_scale * 2.75f );
    ImGui::TextWrapped( "%s", difficultyTable[(int32_t)newGame.diff].tooltip );
    ImGui::End();
}

int32_t count_fields( const char *line );
char **parse_csv( const char *line );

void NewGameMenu_Cache( void )
{
    int i;
    const stringHash_t *hardest;

    memset( &newGame, 0, sizeof( newGame ) );

    hardest = strManager->ValueForKey( "SP_DIFF_THE_MEMES" );
    newGame.hardestStrings = parse_csv( hardest->value );
    newGame.numHardestStrings = count_fields( hardest->value );

    newGame.title = strManager->ValueForKey( "SP_NEWGAME" );
    newGame.newGameSaveNamePrompt = strManager->ValueForKey( "SP_SAVE_NAME_PROMPT" );
    newGame.newGameBegin = strManager->ValueForKey( "SP_BEGIN_NEWGAME" );

    newGame.menu.fullscreen = qtrue;
    newGame.menu.draw = NewGameMenu_Draw;
    newGame.menu.flags = MENU_DEFAULT_FLAGS;
    newGame.menu.width = ui->gpuConfig.vidWidth * 0.75f;
    newGame.menu.height = ui->gpuConfig.vidHeight;
    newGame.menu.x = 0;
    newGame.menu.y = 0;
    newGame.menu.name = "SinglePlayer##MainMenuSinglePlayerNewGame";

    newGame.saveName.generic.name = StringDup( newGame.newGameSaveNamePrompt, "SaveNamePrompt" );
    newGame.saveName.generic.eventcallback = NewGameMenu_EventCallback;
    newGame.saveName.generic.type = MTYPE_FIELD;
    newGame.saveName.generic.id = ID_SAVENAMEPROMPT;
    newGame.saveName.maxchars = MAX_NPATH;

    newGame.beginGame.generic.name = StringDup( newGame.newGameBegin, "BeginNewGame" );
    newGame.beginGame.generic.eventcallback = NewGameMenu_EventCallback;
    newGame.beginGame.generic.type = MTYPE_BUTTON;
    newGame.beginGame.generic.id = ID_BEGINGAME;

    newGame.difList.generic.name = "Difficulty##SinglePlayerMenuDifficultyConfig";
    newGame.difList.generic.eventcallback = NewGameMenu_EventCallback;
    newGame.difList.generic.type = MTYPE_LIST;
    newGame.difList.generic.id = ID_DIFFICULTY;
    newGame.difList.numitems = (int)NUMDIFS;
    newGame.difList.useTable = qfalse;
    for ( i = 0; i < NUMDIFS; i++ ) {
        newGame.difList.itemnames[i] = difficultyTable[i].name;
    }

    Menu_AddItem( &newGame.menu, &newGame.saveName );
    Menu_AddItem( &newGame.menu, &newGame.difList );
    Menu_AddItem( &newGame.menu, &newGame.beginGame );
}

void UI_NewGameMenu( void )
{

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

        switch ( *ptr ) {
        case '\"':
            fQuote = 1;
            continue;
        case ',':
            cnt++;
            continue;
        default:
            continue;
        };
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

    buf = (char **)Z_Malloc( sizeof( char * ) * ( fieldcnt + 1 ), TAG_GAME );
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

        switch ( *ptr ) {
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
            }
            continue;
        default:
            *tptr++ = *ptr;
            continue;
        };

        if ( fEnd ) {
            break;
        }
    }

    *bptr = NULL;
    Hunk_FreeTempMemory( tmp );
    return buf;
}
