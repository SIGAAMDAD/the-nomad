#include "../ui_lib.h"
#include "ml_widget.h"

void IMenuWidget::LoadBase( const json& data )
{
    if ( data.contains( "x" ) ) {
        m_nX = data.at( "x" );
    } else {
        m_nX = -1;
    }

    if ( data.contains( "y" ) ) {
        m_nY = data.at( "y" );
    } else {
        m_nY = -1;
    }

    if ( data.contains( "width" ) ) {
        m_nWidth = data.at( "width" );
    } else {
        m_nWidth = -1;
    }

    if ( data.contains( "height" ) ) {
        m_nHeight = data.at( "height" );
    } else {
        m_nHeight = -1;
    }
}
