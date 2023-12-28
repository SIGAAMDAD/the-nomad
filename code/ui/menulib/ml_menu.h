#ifndef __ML_MENU__
#define __ML_MENU__

#pragma once

#include "code/engine/n_allocator.h"
#include "code/ui/menulib/nlohmann/json.hpp"

#include "ml_widget.h"

using json = nlohmann::json;

template<typename T>
using UtlVector = eastl::vector<T, CZoneAllocator<TAG_GAME>>;

class CDynamicMenu
{
public:
    CDynamicMenu( void );
    ~CDynamicMenu();

    void Load( const json& data );
private:
};

void UI_LoadMenus( void );

#endif