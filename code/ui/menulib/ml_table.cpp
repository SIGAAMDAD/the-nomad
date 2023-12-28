#include "../ui_lib.h"
#include "ml_table.h"


CUITable::CUITable( void ) {
}

CUITable::~CUITable() {
    m_Title.clear();
    m_Widgets.clear();
}

void CUITable::Draw( void )
{
    int32_t y, x;
    uint64_t i;

    ImGui::BeginTable( m_Title.c_str(), m_nColumns );

    for ( y = 0; y < m_nRows; y++ ) {
        if ( y != m_nRows - 1 ) {
            ImGui::TableNextRow();
        }
        for ( x = 0; x < m_nColumns; x++ ) {
            if ( x != m_nColumns - 1 ) {
                ImGui::TableNextColumn();
            }
            for ( i = 0; i < m_Widgets.size(); i++ ) {
                if ( m_Widgets[i].m_nRow != y || m_Widgets[i].m_nColumn != x ) {
                    continue;
                }
                m_Widgets[i].m_pBase->Draw();
            }
        }
    }

    ImGui::EndTable();
}

void CUITable::Load( const json& data )
{
    LoadBase( data );

    if ( !data.contains( "columns" ) ) {
        N_Error( ERR_DROP, "IMenuWidget::Load: table widgets must contain a column count" );
    }
    if ( !data.contains( "rows" ) ) {
        N_Error( ERR_DROP, "IMenuWidget::Load: table widgets must contain a row count" );
    }

    m_nColumns = data.at( "columns" );
    m_nRows = data.at( "rows" );

    if ( !data.contains( "widgets" ) ) {
        return;
    }

    const std::vector<json>& widgets = data.at( "widgets" );

    m_Widgets.reserve( widgets.size() );

    for ( const auto& it : widgets ) {
        CUITableWidget& w = m_Widgets.emplace_back();
        const eastl::string& type = it.at( "type" );

        if ( !N_stricmp( type.c_str(), "table" ) ) {
            w.m_pBase = (CUITable *)Hunk_Alloc( sizeof(CUITable), h_low );
        } else if ( !N_stricmp( type.c_str(), "button" ) ) {

        }

        w.m_pBase->Load( it );
    }
}
