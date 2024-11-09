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

#ifndef __UI_PUBLIC__
#define __UI_PUBLIC__

#pragma once

#include <EASTL/fixed_vector.h>
#include "../rendercommon/r_public.h"
#include "../rendercommon/imgui.h"
#include "../engine/n_allocator.h"
#include "../game/g_sound.h"

#define MAX_UI_FONTS 1024

typedef struct uiFont_s {
    char m_szName[64];
    ImFont *m_pFont;
    uint64_t m_nFileSize;
    struct uiFont_s *m_pNext;
    struct uiFont_s *m_pPrev;
} uiFont_t;

/*
* CUIFontCache
* class for loading and caching all font data
*/
class CUIFontCache
{
public:
    CUIFontCache( void );
    ~CUIFontCache();

    void ClearCache( void );
    ImFont *AddFontToCache( const char *filename, const char *variant = "Regular", float scale = 1.0f );
    void Finalize( void );
    void SetActiveFont( ImFont *pFont );
    void SetActiveFont( nhandle_t hFont );
    uiFont_t *GetFont( const char *fileName );
    nhandle_t RegisterFont( const char *filename, const char *variant, float scale );

    static void ListFonts_f( void );
private:
    uiFont_t *m_FontList[MAX_UI_FONTS];
    ImFont *m_pCurrentFont;
};

extern CUIFontCache *g_pFontCache;

inline CUIFontCache *FontCache( void ) {
    return g_pFontCache;
}

typedef enum : uint64_t {
    UI_MENU_INTRO,
    UI_MENU_TITLE,
    UI_MENU_MAIN,
    UI_MENU_PAUSE,
    UI_MENU_NONE,
    UI_MENU_DEMO,
    UI_MENU_SPLASH,

    NUM_UI_MENUS
} uiMenu_t;

extern "C" void UI_GetHashString( const char *name, char *value );
extern "C" void UI_DrawDiagnostics( void );
extern "C" void UI_ShowDemoMenu( void );
extern "C" void UI_Init( void );
extern "C" void UI_Shutdown( void );
extern "C" void UI_Refresh( int32_t realtime );
extern "C" void UI_DrawText( const char *txt );
extern "C" void UI_SetLoadingStatus( uint64_t nCompiledScripts, uint64_t nTotalScripts, uint64_t nCompiledShaders, uint64_t nTotalShaders,
    uint64_t nLoadedTextures, uint64_t nTotalTextures );
extern void UI_SetActiveMenu( uiMenu_t menu );

// commonly used fonts in the UI system
extern ImFont *AlegreyaSC;
extern ImFont *RobotoMono;

#endif