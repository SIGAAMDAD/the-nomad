#ifndef __UI_STRING_MANAGER__
#define __UI_STRING_MANAGER__

#pragma once

#include "ui_strings.h"

typedef enum : uint64_t
{
    LANGUAGE_ENGLISH,
    
    //
    // these will be added later
    //

    LANGUAGE_SPANISH,
    LANGUAGE_GERMAN,

    NUMLANGS
} language_t;

typedef struct stringHash_s {
    const char *name;
    char value[1024];
    language_t lang;
} stringHash_t;

class CUIStringManager
{
public:
    CUIStringManager( void ) = default;
    ~CUIStringManager() = default;

    void Init( void );
    void Shutdown( void );

    void LoadFile( const char *filename );

    qboolean LanguageLoaded( language_t lang ) const;
    uint64_t NumLangsLoaded( void ) const;
    const stringHash_t *ValueForKey( const char *name ) const;
private:
    int LoadTokenList( const char **text, language_t lang );

    stringHash_t *stringHash[NUMLANGS];
    uint64_t numLanguages;
};

extern CUIStringManager *strManager;

#endif