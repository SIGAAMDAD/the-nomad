#ifndef __G_FONT_H__
#define __G_FONT_H__

#pragma once

#define MAX_UI_FONTS 1024

#include "../rendercommon/imgui.h"
#include <EASTL/unordered_map.h>
#include "../module_lib/module_public.h"

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
	uiFont_t *m_FontList[ MAX_UI_FONTS ];
    ImFont *m_pCurrentFont;
};

extern CUIFontCache *g_pFontCache;

inline CUIFontCache *FontCache( void ) {
    return g_pFontCache;
}

#endif