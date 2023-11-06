#ifndef __UI_WINDOW__
#define __UI_WINDOW__

#pragma once

#include "../rendercommon/imgui.h"

typedef struct {
    const char *name;
    const char *hint;
} menuListItem_t;

typedef struct {
    const char *menuName;
    const menuListItem_t *items;
    uint64_t nitems;
} menuList_t;

class CUIWindow
{
public:
    CUIWindow( const char *name );
    ~CUIWindow();

    // utility
    void SetDimensions( int w, int h );
    glm::ivec2 GetDimensions( void ) const;

    void SetPosition( int xPos, int yPos );
    glm::ivec2 GetPosition( void ) const;

    void SetFontScale( float scale );
    float GetFontScale( void ) const;

    void SameLine( void ) const;
    void NewLine( void ) const;

    int DrawMenuList( const menuList_t *list ) const;

    void DrawString( const char *str ) const;
    void DrawStringCentered( const char *str ) const;

    // input functions
    void TextInput( const char *label, char *buffer, uint64_t size ) const;
    void TextInputWithCompletion( const char *label, mfield_t *buffer ) const;
private:
    const char *name;

    int x, y;
    int width, height;
    float fontScale;
};


#endif