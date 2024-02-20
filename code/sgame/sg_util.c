#include "sg_local.h"
#include "sg_util.h"

struct vector_s
{
    int nElementSize;
    int nSize;
    int nAllocated;
    nhandle_t hBuffer;
};

void vector_init( vector_t *vec, int itemSize )
{
    memset( vec, 0, sizeof(*vec) );
    vec->nElementSize = itemSize;
    vec->hBuffer = ML_CreateBuffer();
}

void vector_shutdown( vector_t *vec )
{
    ML_ReleaseBuffer( vec->hBuffer );
    memset( vec, 0, sizeof(*vec) );
}

void vector_at( vector_t *vec, int index, void *dest )
{
    if ( !vec ) {
        return;
    }
    ML_GetBufferData( vec->hBuffer, vec->nElementSize * index, vec->nElementSize, dest );
}

void vector_append( vector_t *vec, const void *data, int count )
{
    if ( !vec ) {
        return;
    }
    ML_BufferAppend( vec->hBuffer, vec->nElementSize * count, data );
}

/*
* a handy CSV parser for c, I didn't write this, credits to this go to semitrivial
* https://github.com/semitrivial/csv_parser.git
*/

int count_fields( const char *line ) {
    const char *ptr;
    int cnt, fQuote;

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

/*
 *  Given a string containing no linebreaks, or containing line breaks
 *  which are escaped by "double quotes", extract a NULL-terminated
 *  array of strings, one for every cell in the row.
 */
int parse_csv( const char *line, char *strings[MAX_STRING_CHARS], int maxStrings ) {
    char **bptr, *tptr;
    char tmp[MAX_STRING_CHARS];
    const char *ptr;
    int fieldcnt, fQuote, fEnd;
    int total;

    fieldcnt = count_fields( line );

    if ( fieldcnt == -1 ) {
        return -1;
    }

    memset( tmp, 0, sizeof(tmp) );

    bptr = strings;

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
            N_strncpyz( *bptr, tmp, sizeof(*strings) );
            if ( !*bptr ) {
                G_Printf( "bad csv parse!\n" );
                return -1;
            }
            if ( total >= maxStrings ) {
                G_Printf( COLOR_YELLOW "WARNING: too many csv values.\n" );
                return maxStrings;
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
        };

        if ( fEnd ) {
            break;
        }
    }

    return total;
}
