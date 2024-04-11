#include "ui_lib.h"

typedef struct {
    menuframework_t menu;
    
    menufield_t saveName;
    menubutton_t beginGame;

    menuarrow_t difArrowLeft;
    menuarrow_t difArrowRight;
    menulist_t difList;
    menutable_t difTable;

    gamedif_t diff;

    char **hardestStrings;
    int64_t numHardestStrings;
    int64_t hardestIndex;

    const stringHash_t *title;
    const stringHash_t *s_newGameSaveNamePrompt;
    const stringHash_t *s_newGameBegin;
} newGameMenu_t;

#define ID_BEGINGAME        1
#define ID_SAVENAMEPROMPT   2
#define ID_DIFFICULTY       3

static newGameMenu_t *s_newGame;

static void BeginNewGame( void )
{
    memset( s_newGame->saveName.buffer, 0, sizeof( s_newGame->saveName.buffer ) );

    UI_ForceMenuOff();
    ui->menusp = 0;
    ui->activemenu = NULL;

    gi.state = GS_LEVEL;

    Cvar_Set( "sgame_SaveName", s_newGame->saveName.buffer );
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
        s_newGame->diff = (gamedif_t)s_newGame->difList.curitem;
        break;
    };
}

static void NewGameMenu_Draw( void )
{
    float font_scale;

    Menu_Draw( &s_newGame->menu );

    FontCache()->SetActiveFont( RobotoMono );

    ImGui::Begin( "DifficultyDescription", NULL, MENU_DEFAULT_FLAGS );
    font_scale = ImGui::GetFont()->Scale;
    ImGui::SetWindowFontScale( font_scale * 3.75f );
    ImGui::TextUnformatted( "Difficulty Description" );
    ImGui::SetWindowFontScale( font_scale * 2.75f );
    ImGui::TextWrapped( "%s", difficultyTable[(int32_t)s_newGame->diff].tooltip );
    ImGui::End();
}

int32_t count_fields( const char *line );
char **parse_csv( const char *line );

static void NewGameMenu_ArrowLeft( void *ptr, int event )
{
    int item;

    if ( event != EVENT_ACTIVATED ) {
        return;
    }

    item = ( (menulist_t *)ptr )->curitem;
    if ( item == 0 ) {
        item = ( (menulist_t *)ptr )->numitems - 1;
    } else {
        item--;
    }

    ( (menulist_t *)ptr )->curitem = item;
}

static void NewGameMenu_ArrowRight( void *ptr, int event )
{
    int item;

    if ( event != EVENT_ACTIVATED ) {
        return;
    }

    item = ( (menulist_t *)ptr )->curitem;
    if ( item == ( (menulist_t *)ptr )->numitems - 1 ) {
        item = 0;
    } else {
        item++;
    }

    ( (menulist_t *)ptr )->curitem = item;
}

void NewGameMenu_Cache( void )
{
    int i;
    const stringHash_t *hardest;

    hardest = strManager->ValueForKey( "SP_DIFF_THE_MEMES" );

    if ( !ui->uiAllocated ) {
        s_newGame = (newGameMenu_t *)Hunk_Alloc( sizeof( *s_newGame ), h_high );
        memset( s_newGame, 0, sizeof( *s_newGame ) );
        s_newGame->hardestStrings = parse_csv( hardest->value );
        s_newGame->numHardestStrings = count_fields( hardest->value );
    }

    srand( Sys_Milliseconds() );
    s_newGame->hardestIndex = rand() % s_newGame->numHardestStrings;
    static const char *difficulties[NUMDIFS] = {
        difficultyTable[ DIF_NOOB ].name,
        difficultyTable[ DIF_RECRUIT ].name,
        difficultyTable[ DIF_MERC ].name,
        difficultyTable[ DIF_NOMAD ].name,
        difficultyTable[ DIF_BLACKDEATH ].name,
        s_newGame->hardestStrings[ s_newGame->hardestIndex ]
    };

    s_newGame->title = strManager->ValueForKey( "SP_NEWGAME" );
    s_newGame->s_newGameSaveNamePrompt = strManager->ValueForKey( "SP_SAVE_NAME_PROMPT" );
    s_newGame->s_newGameBegin = strManager->ValueForKey( "SP_BEGIN_NEWGAME" );

    s_newGame->menu.fullscreen = qtrue;
    s_newGame->menu.draw = NewGameMenu_Draw;
    s_newGame->menu.flags = MENU_DEFAULT_FLAGS;
    s_newGame->menu.width = ui->gpuConfig.vidWidth * 0.75f;
    s_newGame->menu.height = ui->gpuConfig.vidHeight;
    s_newGame->menu.titleFontScale = 3.5f;
    s_newGame->menu.textFontScale = 1.5f;
    s_newGame->menu.x = 0;
    s_newGame->menu.y = 0;
    s_newGame->menu.name = s_newGame->title->value;

    s_newGame->saveName.generic.name = StringDup( s_newGame->s_newGameSaveNamePrompt, "SaveNamePrompt" );
    s_newGame->saveName.generic.eventcallback = NewGameMenu_EventCallback;
    s_newGame->saveName.generic.type = MTYPE_FIELD;
    s_newGame->saveName.generic.id = ID_SAVENAMEPROMPT;
    s_newGame->saveName.maxchars = MAX_NPATH;

    s_newGame->beginGame.generic.name = StringDup( s_newGame->s_newGameBegin, "BeginNewGame" );
    s_newGame->beginGame.generic.eventcallback = NewGameMenu_EventCallback;
    s_newGame->beginGame.generic.type = MTYPE_BUTTON;
    s_newGame->beginGame.generic.id = ID_BEGINGAME;

    s_newGame->difArrowLeft.generic.id = ID_DIFFICULTY;
    s_newGame->difArrowLeft.generic.eventcallback = MenuEvent_ArrowLeft;
    s_newGame->difArrowLeft.generic.name = "##DifficultySinglePlayerMenuLeft";
    s_newGame->difArrowLeft.direction = ImGuiDir_Left;

    s_newGame->difArrowLeft.generic.id = ID_DIFFICULTY;
    s_newGame->difArrowLeft.generic.eventcallback = MenuEvent_ArrowRight;
    s_newGame->difArrowLeft.generic.name = "##DifficultySinglePlayerMenuRight";
    s_newGame->difArrowLeft.direction = ImGuiDir_Right;

    s_newGame->difList.generic.name = "Difficulty##SinglePlayerMenuDifficultyConfig";
    s_newGame->difList.generic.eventcallback = NewGameMenu_EventCallback;
    s_newGame->difList.generic.type = MTYPE_LIST;
    s_newGame->difList.generic.id = ID_DIFFICULTY;
    s_newGame->difList.numitems = (int)NUMDIFS;
    s_newGame->difList.itemnames = difficulties;

    Menu_AddItem( &s_newGame->menu, &s_newGame->saveName );
    Menu_AddItem( &s_newGame->menu, &s_newGame->difTable );
 
    Table_AddItem( &s_newGame->difTable, &s_newGame->difArrowLeft );
    Table_AddItem( &s_newGame->difTable, &s_newGame->difList );
    Table_AddItem( &s_newGame->difTable, &s_newGame->difArrowRight );

    Menu_AddItem( &s_newGame->menu, &s_newGame->beginGame );
}

void UI_NewGameMenu( void )
{
    NewGameMenu_Cache();
    UI_PushMenu( &s_newGame->menu );
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
