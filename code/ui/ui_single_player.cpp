#include "../game/g_game.h"
#include "ui_public.hpp"
#include "ui_menu.h"
#include "ui_lib.h"
#include "ui_string_manager.h"

#define ID_NEWGAME      0
#define ID_LOADGAME     1
#define ID_PLAMISSION   2

typedef struct {
	const stringHash_t *titleString;
    const stringHash_t *newGameString;
    const stringHash_t *loadGameString;
    const stringHash_t *playMissionString;

    menuframework_t menu;

    menutext_t newGame;
    menutext_t loadGame;
    menutext_t playMission;
} singleplayer_t;

extern ImFont *RobotoMono;
extern ImFont *PressStart2P;
static singleplayer_t sp;

static void SinglePlayerMenu_EventCallback( void *ptr, int event )
{
    if ( event != EVENT_ACTIVATED ) {
        return;
    }

    switch ( ( (menucommon_t *)ptr )->id ) {
    case ID_NEWGAME:
        UI_SinglePlayerMenu();
        break;
    case ID_LOADGAME:
        UI_LoadGameMenu();
        break;
    case ID_PLAMISSION:
        break;
    default:
        break;
    };
}

char **parse_csv( const char *line );
int32_t count_fields( const char *line );

void SinglePlayerMenu_Cache( void )
{
    memset( &sp, 0, sizeof( sp ) );

	sp.titleString = strManager->ValueForKey( "SP_MENU_TITLE" );	
    sp.newGameString = strManager->ValueForKey( "SP_NEWGAME" );
    sp.loadGameString = strManager->ValueForKey( "SP_LOADGAME" );
    sp.playMissionString = strManager->ValueForKey( "SP_PLAY_MISSION" );

    sp.menu.name = "SinglePlayerMenu##MainMenuSinglePlayerConfig";
    sp.menu.flags = MENU_DEFAULT_FLAGS;
    sp.menu.x = 0;
    sp.menu.y = 0;
    sp.menu.width = ui->gpuConfig.vidWidth;
    sp.menu.height = ui->gpuConfig.vidHeight;
    sp.menu.fullscreen = qtrue;

    sp.newGame.generic.name = StringDup( sp.newGameString, "SinglePlayerNewGameOption" );
    sp.newGame.generic.type = MTYPE_TEXT;
    sp.newGame.generic.id = ID_NEWGAME;
    sp.newGame.generic.flags = QMF_HIGHLIGHT_IF_FOCUS;
    sp.newGame.generic.eventcallback = SinglePlayerMenu_EventCallback;
    sp.newGame.text = sp.newGameString->value;
    VectorCopy4( sp.newGame.color, colorWhite );

    sp.loadGame.generic.name = StringDup( sp.loadGameString, "SinglePlayerLoadGameOption" );
    sp.loadGame.generic.type = MTYPE_TEXT;
    sp.loadGame.generic.id = ID_LOADGAME;
    sp.loadGame.generic.flags = QMF_HIGHLIGHT_IF_FOCUS;
    sp.loadGame.generic.eventcallback = SinglePlayerMenu_EventCallback;
    sp.loadGame.text = sp.loadGameString->value;
    VectorCopy4( sp.loadGame.color, colorWhite );

    sp.playMission.generic.name = StringDup( sp.playMissionString, "SinglePlayerPlayMissionOption" );
    sp.playMission.generic.type = MTYPE_TEXT;
    sp.playMission.generic.id = ID_PLAMISSION;
    sp.playMission.generic.flags = QMF_HIGHLIGHT_IF_FOCUS;
    sp.playMission.generic.eventcallback = SinglePlayerMenu_EventCallback;
    sp.playMission.text = sp.playMissionString->value;
    VectorCopy4( sp.playMission.color, colorWhite );

    Menu_AddItem( &sp.menu, &sp.newGame );
    Menu_AddItem( &sp.menu, &sp.loadGame );
    Menu_AddItem( &sp.menu, &sp.playMission );
}

void UI_SinglePlayerMenu( void )
{
    UI_PushMenu( &sp.menu );
    Key_SetCatcher( KEYCATCH_UI );
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