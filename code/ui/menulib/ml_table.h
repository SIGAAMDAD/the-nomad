#ifndef __ML_TABLE__
#define __ML_TABLE__

#pragma once

#include "ml_widget.h"

struct CUITableWidget
{
    IMenuWidget *m_pBase;
    int32_t m_nColumn;
    int32_t m_nRow;
};

class CUITable : public IMenuWidget
{
public:
    virtual void Draw( void ) override;
    virtual void Load( const json& data ) override;
private:
    int32_t m_nColumns;
    int32_t m_nRows;
    UtlVector<CUITableWidget> m_Widgets;
};

#endif