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

#ifndef __UI_STRING_MANAGER__
#define __UI_STRING_MANAGER__

#pragma once

//#define USE_MAP_HASH

#include "ui_strings.h"
#include <EASTL/unordered_map.h>
#include <EASTL/vector_map.h>

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
    
    void LoadLanguage( const char *filename );

    qboolean LanguageLoaded( language_t lang ) const;
    uint64_t NumLangsLoaded( void ) const;
    const stringHash_t *ValueForKey( const char *name );
private:
    const stringHash_t *AllocErrorString( const char *key );
    int LoadTokenList( const char **text, language_t lang );

#ifdef USE_MAP_HASH
    eastl::unordered_map<const char *, stringHash_t *> stringHash[NUMLANGS];
#else
    stringHash_t **stringHash[NUMLANGS];
#endif
    uint64_t numLanguages;
};

extern CUIStringManager *strManager;

#endif