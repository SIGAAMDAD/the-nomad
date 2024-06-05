#ifndef __UI_STRING_MANAGER__
#define __UI_STRING_MANAGER__

#pragma once

#include "ui_strings.h"
#include <EASTL/unordered_map.h>

#define MAX_UI_STRINGS 4096

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
    char *name;
    char *value;
    language_t lang;
    struct stringHash_s *next;
    
    bool operator==( const stringHash_s *other ) const {
        return this == other;
    }
} stringHash_t;

namespace eastl {
	template<> struct hash<stringHash_t *> {
		size_t operator()( const stringHash_t *str ) const {
			const unsigned char *p = (const unsigned char *)str->value; // To consider: limit p to at most 256 chars.
			unsigned int c, result = 2166136261U; // We implement an FNV-like string hash.
			while((c = *p++) != 0) // Using '!=' disables compiler warnings.
				result = (result * 16777619) ^ c;
			return (size_t)result;
		}
	};
};

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
    const stringHash_t *ValueForKey( const char *name );
private:
    const stringHash_t *AllocErrorString( const char *key );
    int LoadTokenList( const char **text, language_t lang );

    stringHash_t **stringHash[NUMLANGS];
    uint64_t numLanguages;
};

extern CUIStringManager *strManager;

#endif