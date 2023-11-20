#include "../game/g_game.h"
#include "ui_public.h"
#include "ui_menu.h"
#include "ui_lib.h"
#include "ui_string_manager.h"

CUIStringManager *strManager;

void CUIStringManager::Init( void )
{
    numLanguages = 0;
    memset(stringHash, 0, sizeof(stringHash));
}

void CUIStringManager::Shutdown( void )
{
    uint64_t i;

    for (i = 0; i < NUMLANGS; i++) {
        if (stringHash[i]) {
            Z_Free(stringHash[i]);
        }
    }
}

static language_t StringToLanguage( const char *tok )
{
    if (!N_stricmp(tok, "English")) {
        Con_Printf("Language selected: English\n");
        return LANGUAGE_ENGLISH;
    } else if (!N_stricmp(tok, "Spanish")) {
        Con_Printf("Language selected: Spanish\n");
        return LANGUAGE_SPANISH;
    }

    COM_ParseWarning("unknown language parameter '%s', setting default of english", tok);
    return LANGUAGE_ENGLISH;
}

static int IsOnStringList( const char *tok )
{
    for (uint64_t i = 0; i < NUM_UI_STRINGS; i++) {
        if (!N_stricmp(uiStrings[i], tok)) {
            return qtrue;
        }
    }
    return qfalse;
}

int CUIStringManager::LoadTokenList( const char **text, language_t lang )
{
    stringHash_t *hashTable, *str;
    uint64_t hash, i;
    const char *tok;

    hashTable = (stringHash_t *)Z_Malloc(sizeof(*hashTable) * NUM_UI_STRINGS, TAG_STATIC);
    memset(hashTable, 0, sizeof(*hashTable));

    for (i = 0; i < NUM_UI_STRINGS; i++) {
        tok = COM_ParseComplex( text, qtrue );
        if (tok[0] == 0) {
            COM_ParseError("unexpected end of token list");
            return -1;
        }
        else if (!IsOnStringList(tok)) {
            COM_ParseError("unknown token name '%s', skipping", tok);
            continue;
        }

        // bug, so we're doing THIS for now
        str = &hashTable[i];

        tok = COM_ParseExt( text, qtrue );
        if (!tok[0]) {
            COM_ParseError("got invalid token value string");
            return -1;
        }
        else if (strlen(tok) >= sizeof(str->value)) {
            COM_ParseError("token value '%s' is too long (%lu > %lu characters)", tok, strlen(tok), sizeof(str->value));
            return -1;
        }

        str->name = uiStrings[i];
        N_strncpyz(str->value, tok, sizeof(str->value));
        str->lang = lang;
    }
    stringHash[lang] = hashTable;

    return 1;
}

void CUIStringManager::LoadFile( const char *filename )
{
    const char *tok, *t1, **text;
    uint64_t fileLength;
    language_t lang = NUMLANGS;
    union {
        void *v;
        char *b;
    } f;
    
    fileLength = FS_LoadFile(filename, &f.v);
    if (!fileLength || !f.v) {
        Con_Printf(COLOR_YELLOW "WARNING: failed to load ui string file '%s'\n", filename);
        return;
    }

    t1 = f.b;
    text = &t1;

    Con_Printf("CUIStringManager::LoadFile: loading language file '%s'\n", filename);

    COM_BeginParseSession( filename );

    tok = COM_ParseExt( text, qtrue );
    if (tok[0] != '{') {
        COM_ParseWarning("expected '{', got '%s'", tok);
        return;
    }

    while (1) {
        tok = COM_ParseExt(text, qtrue);
        if (!tok[0]) {
            COM_ParseWarning("no concluding '}' in ui file");
            return;
        }

        // end of ui file
        if (tok[0] == '}') {
            break;
        }
        //
        // language <language>
        //
        else if (!N_stricmp( tok, "language" )) {
            tok = COM_ParseExt( text, qtrue );
            if (!tok[0]) {
                COM_ParseError("missing parameter for language in ui file");
                return;
            }

            lang = StringToLanguage( tok );
        }
        //
        // tokens ...
        //
        else if (!N_stricmp( tok, "tokens" )) {
            tok = COM_ParseExt( text, qtrue );
            if (tok[0] != '{') {
                COM_ParseError( "missing opening '{' for tokens in ui file" );
                return;
            }

            if (lang == NUMLANGS) {
                COM_ParseError( "no language specified" );
                return;
            }

            if (!LoadTokenList( text, lang )) {
                return;
            }
        }
    }
    numLanguages++;

    if (ui_printStrings->i) {
        Con_Printf("\n---------- UI Strings: %s ----------\n", UI_LangToString((int32_t)lang));
        for (uint64_t i = 0; i < NUM_UI_STRINGS; i++) {
            if (stringHash[lang][i].name) {
                Con_Printf("\"%s\" = \"%s\"\n", stringHash[lang][i].name, stringHash[lang][i].value);
            }
        }
    }
}

qboolean CUIStringManager::LanguageLoaded( language_t lang ) const {
    return (qboolean)(stringHash[lang] != NULL);
}

uint64_t CUIStringManager::NumLangsLoaded( void ) const {
    return numLanguages;
}

const stringHash_t *CUIStringManager::ValueForKey( const char *name ) const
{
    if (!name || !name[0]) {
        N_Error(ERR_FATAL, "ValueForKey: NULL or empty name");
    }

    for (uint64_t i = 0; i < NUM_UI_STRINGS; i++) {
        if (!N_stricmp(stringHash[ui_language->i][i].name, name)) {
            return &stringHash[ui_language->i][i];
        }
    }
    return NULL;
}
