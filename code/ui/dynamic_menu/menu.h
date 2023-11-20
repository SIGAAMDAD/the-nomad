#ifndef __DYNMENU_MENU__
#define __DYNMENU_MENU__

#pragma once

#include <EASTL/fixed_string.h>
#include <EASTL/fixed_vector.h>
using CUtlString = eastl::fixed_string<char, MAX_STRING_CHARS, true>;
template<typename T>
using CUtlVector = eastl::fixed_vector<T, 1024, true>;

class CDynamicMenu
{
public:
    void Draw( void );
private:
    CUtlString m_Label;
    uint32_t m_nWidth;
    uint32_t m_nHeight;

    CUtlVector<CButton> m_ButtonList;
};

#endif