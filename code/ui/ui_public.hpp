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
    void SetActiveFont( ImFont *font );
    uiFont_t *GetFont( const char *fileName );

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

    NUM_UI_MENUS
} uiMenu_t;

extern "C" void UI_GetHashString( const char *name, char *value );
extern "C" void UI_DrawDiagnostics( void );
extern "C" void UI_ShowDemoMenu( void );
extern "C" void UI_Init( void );
extern "C" void UI_Shutdown( void );
extern "C" void UI_Refresh( int32_t realtime );

// commonly used fonts in the UI system
extern ImFont *AlegreyaSC;
extern ImFont *RobotoMono;

#endif