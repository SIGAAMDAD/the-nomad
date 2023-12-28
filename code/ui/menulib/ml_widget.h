#ifndef __ML_WIDGET__
#define __ML_WIDGET__

#pragma once

#include "ml_menu.h"

typedef enum {
    WIDGET_TABLE,
    WIDGET_MENU,
    WIDGET_ITEM,
    WIDGET_TEXT,
    WIDGET_BUTTON,
} widgetType_t;

class IMenuWidget
{
public:
    IMenuWidget( void );
    virtual ~IMenuWidget();

    virtual void Draw( void ) = 0;
    virtual void Load( const json& data ) = 0;
protected:
    void LoadBase( const json& data );

    eastl::string m_Title;
    int32_t m_nWidth;
    int32_t m_nHeight;
    int32_t m_nX;
    int32_t m_nY;
    widgetType_t m_Type;
};

#endif