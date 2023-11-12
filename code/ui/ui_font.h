#ifndef __UI_FONT__
#define __UI_FONT__

#pragma once

class CUIFont
{
public:
    CUIFont( void );
    ~CUIFont();

    bool LoadFile( const char *path );
    void Bind( void );
    void Unbind( void ) const;
private:
    char name[MAX_GDR_PATH];
    ImFont *fontHandle;
};

#define MAX_FONTS 256

class CUIFontManager
{
public:
    CUIFontManager( void );
    ~CUIFontManager();

    CUIFont *AddFont( const char *path );
    void BuildAltas( void ) const;
private:
    CUIFont *fonts[MAX_FONTS];
    uint64_t numFonts;
};

extern CUIFontManager *fontManager;

#endif