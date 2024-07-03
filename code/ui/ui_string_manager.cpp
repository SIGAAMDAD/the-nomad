/*
===========================================================================
Copyright (C) 2023-2024 GDR Games

This file is part of The Nomad source code.

The Nomad source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

The Nomad source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Foobar; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/

#include "../game/g_game.h"
#include "ui_public.hpp"
#include "ui_menu.h"
#include "ui_lib.h"
#include "ui_string_manager.h"

CUIStringManager *strManager;

void CUIStringManager::Init( void ) {
    numLanguages = 0;
    memset( stringHash, 0, sizeof( stringHash ) );
}

void CUIStringManager::Shutdown( void ) {
    numLanguages = 0;
    memset( stringHash, 0, sizeof( stringHash ) );
}

static language_t StringToLanguage( const char *tok )
{
    if ( !N_stricmp( tok, "English" ) ) {
        Con_Printf( "Language selected: English\n" );
        return LANGUAGE_ENGLISH;
    } else if ( !N_stricmp( tok, "Spanish" ) ) {
        Con_Printf( "Language selected: Spanish\n" );
        return LANGUAGE_SPANISH;
    }

    COM_ParseWarning( "unknown language parameter '%s', setting default of english", tok );
    return LANGUAGE_ENGLISH;
}

int CUIStringManager::LoadTokenList( const char **text, language_t lang )
{
    stringHash_t *str, **hashTable;
    uint64_t hash, i;
    uint64_t size;
    char name[MAX_STRING_CHARS], value[MAX_STRING_CHARS];
    const char *tok;
    qboolean ignore;

    // we may be just reloading the language file, so clear it out
    if ( !stringHash[ lang ] ) {
        stringHash[ lang ] = (stringHash_t **)Hunk_Alloc( sizeof( stringHash_t ** ) * ui_maxLangStrings->i, h_high );
    }

    for ( i = 0; i < ui_maxLangStrings->i; i++ ) {
        ignore = qfalse;

        memset( name, 0, sizeof( name ) );
        memset( value, 0, sizeof( value ) );

        tok = COM_ParseComplex( text, qtrue );
        if ( tok[0] == ' ' || tok[0] == '\n' ) {
            continue;
        } else if ( tok[0] == '}' ) {
            // end of token list
            break;
        } else if ( tok[0] == 0 ) {
            COM_ParseError( "unexpected end of string token list" );
            return -1;
        } else if ( strlen( tok ) >= sizeof( name ) ) {
            COM_ParseError( "string token name '%s' is too long", tok );
            continue;
        }

        hash = Com_GenerateHashValue( tok, ui_maxLangStrings->i );
        for ( str = stringHash[lang][hash]; str; str = str->next ) {
            if ( !N_stricmp( str->name, tok ) ) {
                COM_ParseWarning( "found duplicate string token for '%s', ignoring it.\n", tok );
                ignore = qtrue;
                break;
            }
        }
        if ( ignore ) {
            continue;
        }

        N_strncpyz( name, tok, sizeof( name ) );

        tok = COM_ParseExt( text, qtrue );
        if ( strlen( tok ) >= sizeof( value ) ) {
            COM_ParseError( "string token value '%s' is too long", tok );
            return -1;
        }

        N_strncpyz( value, tok, sizeof( value ) );

        size = 0;
        size += PAD( sizeof( *str ), sizeof( uintptr_t ) );
        size += PAD( strlen( name ) + 1, sizeof( uintptr_t ) );
        size += PAD( strlen( value ) + 1, sizeof( uintptr_t ) );

        str = (stringHash_t *)Hunk_Alloc( sizeof( *str ), h_high );
        str->name = (char *)Hunk_Alloc( strlen( name ) + 1, h_high );
        str->value = (char *)Hunk_Alloc( strlen( value ) + 1, h_high );
//        str->name = (char *)( str + 1 );
//        str->value = (char *)( str->name + strlen( name ) + 1 );

        str->next = stringHash[lang][hash];
        stringHash[lang][hash] = str;

        strcpy( str->name, name );
        strcpy( str->value, value );

        str->lang = lang;
    }

    return 1;
}

void CUIStringManager::LoadLanguage( const char *filename )
{
    const char *tok, *t1, **text;
    uint64_t fileLength;
    language_t lang = NUMLANGS;
    union {
        void *v;
        char *b;
    } f;
    char **fileList;
    const char *language;
    uint64_t nFiles, i;

    language = filename;
    fileList = FS_ListFiles( va( "scripts/langs/%s", language ), ".txt", &nFiles );

    for ( i = 0; i < nFiles; i++ ) {
        filename = va( "scripts/langs/%s/%s", language, fileList[i] );
        fileLength = FS_LoadFile( filename, &f.v );
        if ( !fileLength || !f.v ) {
            Con_Printf( COLOR_YELLOW "WARNING: failed to load ui string file '%s'\n", filename );
            return;
        }

        t1 = f.b;
        text = &t1;

        Con_Printf( "CUIStringManager::LoadFile: loading language file '%s'\n", filename );

        COM_BeginParseSession( filename );

        tok = COM_ParseExt( text, qtrue );
        if ( tok[0] != '{' ) {
            COM_ParseWarning( "expected '{', got '%s'", tok );
            continue;
        }

        while ( 1 ) {
            tok = COM_ParseExt( text, qtrue );
            if ( !tok[0] ) {
                COM_ParseWarning( "no concluding '}' in ui file" );
                continue;
            }

            // end of ui file
            if ( tok[0] == '}' ) {
                break;
            }
            //
            // language <language>
            //
            else if ( !N_stricmp( tok, "language" ) ) {
                tok = COM_ParseExt( text, qtrue );
                if ( !tok[0] ) {
                    COM_ParseError( "missing parameter for language in ui file" );
                    continue;
                }

                lang = StringToLanguage( tok );
            }
            //
            // tokens { ... }
            //
            else if ( !N_stricmp( tok, "tokens" ) ) {
                tok = COM_ParseExt( text, qtrue );
                if ( tok[0] != '{' ) {
                    COM_ParseError( "missing opening '{' for tokens in ui file" );
                    continue;
                }

                if ( lang == NUMLANGS ) {
                    COM_ParseError( "no language specified" );
                    continue;
                }

                if ( !LoadTokenList( text, lang ) ) {
                    continue;
                }
            }
        }
        FS_FreeFile( f.v );
    }
    FS_FreeFileList( fileList );
    numLanguages++;

    if ( ui_printStrings->i ) {
        Con_Printf( "\n---------- UI Strings: %s ----------\n", UI_LangToString( (int32_t)lang ) );
        for ( i = 0; i < ui_maxLangStrings->i; i++ ) {
            if ( stringHash[lang][i] ) {
                Con_Printf( "\"%s\" = \"%s\" (HASH: %lu)\n", stringHash[lang][i]->name, stringHash[lang][i]->value, i );
            }
        }
    }
}

qboolean CUIStringManager::StringExists( const char *name ) const {
    uint64_t hash;
    const stringHash_t *str;

    hash = Com_GenerateHashValue( name, ui_maxLangStrings->i );
    for ( str = stringHash[ ui_language->i ][hash]; str; str = str->next ) {
        if ( !N_stricmp( str->name, name ) ) {
            return qtrue;
        }
    }
    return qfalse;
}

qboolean CUIStringManager::LanguageLoaded( language_t lang ) const {
    return (qboolean)( stringHash[lang] != NULL );
}

uint64_t CUIStringManager::NumLangsLoaded( void ) const {
    return numLanguages;
}

const stringHash_t *CUIStringManager::AllocErrorString( const char *key ) {
    stringHash_t *str;
    uint64_t size, hash;
    const char *value;

    value = va( "ERROR: %s variable has not been set before.", key );

    size = 0;
    size += PAD( sizeof( *str ), sizeof( uintptr_t ) );
    size += PAD( strlen( key ) + 1, sizeof( uintptr_t ) );
    size += PAD( strlen( value ) + 1, sizeof( uintptr_t ) );

    str = (stringHash_t *)Hunk_Alloc( size, h_high );
    str->name = (char *)( str + 1 );
    str->value = (char *)( str->name + strlen( key ) + 1 );
    str->lang = (language_t)ui_language->i;

    N_strncpyz( str->name, key, MAX_STRING_CHARS );
    N_strncpyz( str->value, value, MAX_STRING_CHARS );

    hash = Com_GenerateHashValue( key, ui_maxLangStrings->i );

    str->next = stringHash[ ui_language->i ][hash];
    stringHash[ ui_language->i ][hash] = str;

    return str;
}

const stringHash_t *CUIStringManager::ValueForKey( const char *name )
{
    uint64_t hash;
    const stringHash_t *str;

    if ( !name || !name[0] ) {
        N_Error( ERR_FATAL, "CUIStringManager::ValueForKey: NULL or empty name" );
    }

    hash = Com_GenerateHashValue( name, ui_maxLangStrings->i );
    for ( str = stringHash[ui_language->i][hash]; str; str = str->next ) {
        if ( !N_stricmp( name, str->name ) ) {
            return str;
        }
    }

    return AllocErrorString( name );
}