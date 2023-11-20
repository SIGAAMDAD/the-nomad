#ifndef __DYNMENU_BUTTON__
#define __DYNMENU_BUTTON__

#pragma once

#include "menu.h"

class CButton
{
public:
    CButton( const char **pText );
    ~CButton();

    bool Load( const char **pText );
    bool Draw( void ) const;
private:
    CUtlString m_Label;
    int32_t m_PosX;
    int32_t m_PosY;
    vec4_t m_pColor;
    vec2_t m_pSize;
};

#endif