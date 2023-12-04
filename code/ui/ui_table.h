#ifndef __UI_TABLE__
#define __UI_TABLE__

#pragma once

#include <EASTL/functional.h>

class CUITable
{
public:
    CUITable( void );
    ~CUITable();

    void AddConfigOption( const char *pName, const eastl::function<void( void )>& fn );
};

GDR_INLINE CUITable::CUITable( void )
{
    ImGui::BeginTable( " ", 2 );
}

GDR_INLINE CUITable::~CUITable()
{
    ImGui::TableNextColumn();
    ImGui::TableNextColumn();
    ImGui::TableNextRow();

    ImGui::EndTable();
}

GDR_INLINE void CUITable::AddConfigOption( const char *pName, const eastl::function<void( void )>& fn )
{
    ImGui::TableNextColumn();
    ImGui::TextUnformatted( pName );
    ImGui::TableNextColumn();
    fn();

    ImGui::TableNextRow();
}

#endif