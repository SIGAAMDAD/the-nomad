#include "ui_lib.h"

CUIFontManager *fontManager;

CUIFont::CUIFont( void )
{
    fontHandle = NULL;
    memset(name,  0, sizeof(name));
}

CUIFont::~CUIFont() {
}

void CUIFont::Bind( void ) {
    ImGui::PushFont( fontHandle );
}

void CUIFont::Unbind( void ) const {
    ImGui::PopFont();
}

bool CUIFont::LoadFile( const char *path )
{
    void *buffer;
    uint64_t length;
    ImFontConfig config;

    Con_DPrintf("CUIFont::LoadFile: loading font file '%s'...\n", path);

    length = FS_LoadFile( path, &buffer );
    if (!length || !buffer) {
        Con_Printf(COLOR_YELLOW "WARNING: Failed to load font file '%s'\n", path);
        return false;
    }

    N_strncpyz( name, path, sizeof(name) );

    config.MergeMode = true;
    fontHandle = ImGui::GetIO().Fonts->AddFontFromMemoryTTF( buffer, length, 18.0f, &config, ImGui::GetIO().Fonts->GetGlyphRangesDefault() );

    return fontHandle != NULL;
}

CUIFontManager::CUIFontManager( void )
{
    numFonts = 0;
    memset(fonts, 0, sizeof(fonts));
}

CUIFontManager::~CUIFontManager() {
}

CUIFont *CUIFontManager::AddFont( const char *path )
{
    CUIFont *font;

    if (numFonts == MAX_FONTS) {
        N_Error(ERR_DROP, "CUIFontManager::AddFont: MAX_FONTS hit");
    }

    font = fonts[numFonts] = (CUIFont *)Z_Malloc( sizeof(*font), TAG_GAME );
    numFonts++;

    // construct
    ::new ((void *)font) CUIFont();

    // load
    if (!font->LoadFile( path )) {
        return NULL;
    }

    return font;
}

void CUIFontManager::BuildAltas( void ) const {
    ImGuiIO& io = ImGui::GetIO();

    if (!io.Fonts->IsBuilt()) {
        io.Fonts->Build();
    }
}
