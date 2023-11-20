#include "../ui_lib.h"
#include "button.h"

CButton::CButton( const char **pText )
{
    m_PosX = -1;
    m_PosY = -1;
    memcpy( m_pColor, &ImGui::GetStyle().Colors[ImGuiCol_FrameBg], sizeof(vec4_t) );
    VectorSet2( m_pSize, 0, 0 );

    Load( pText );
}

CButton::~CButton()
{
}

bool CButton::Load( const char **pText )
{
    const char *tok;

    while (1) {
        tok = COM_ParseComplex( pText, qtrue );
        
        if (!tok[0]) {
            COM_ParseError( "unexpected end of button definition, expected '}'" );
            return false;
        }

        // end of definition
        if (tok[0] == '}') {
            break;
        }
        //
        // color ( <r> <g> <b> <a> )
        //
        else if (!N_stricmp( tok, "color" )) {
            Parse1DMatrix( pText, 4, m_pColor );
        }
        //
        // size ( <w> <h> )
        //
        else if (!N_stricmp( tok, "size" )) {
            Parse1DMatrix( pText, 2, m_pSize );
        }
        //
        // posX <position>
        //
        else if (!N_stricmp( tok, "posX" )) {
            tok = COM_ParseExt( pText, qfalse );
            if (!tok[0]) {
                COM_ParseWarning( "missing value for button parameter 'position'" );
                continue;
            }
            m_PosX = atoi( tok );
        }
        //
        // posY <position>
        //
        else if (!N_stricmp( tok, "posY" )) {
            tok = COM_ParseExt( pText, qfalse );
            if (!tok[0]) {
                COM_ParseWarning( "missing value for button parameter 'position'" );
                continue;
            }
            m_PosY = atoi( tok );
        }
        //
        // position <posX> <posY>
        //
        else if (!N_stricmp( tok, "position" )) {
            tok = COM_ParseExt( pText, qfalse );
            if (!tok[0]) {
                COM_ParseWarning( "missing value for button parameter 'position'" );
                continue;
            }
            m_PosX = atoi( tok );
            tok = COM_ParseExt( pText, qfalse );
            if (!tok[0]) {
                COM_ParseWarning( "missing value for button parameter 'position'" );
                continue;
            }
            m_PosY = atoi( tok );
        }
        else {
            COM_ParseWarning( "unrecognized parameter '%s' in button definition", tok );
            continue;
        }
    }
    return true;
}

bool CButton::Draw( void ) const
{
    bool bPushed;

    ImGui::PushStyleColor( ImGuiCol_FrameBg, ImVec4( m_pColor ) );
    bPushed = ImGui::Button( m_Label.c_str(), ImVec2( m_pSize[0], m_pSize[1] ) );
    ImGui::PopStyleColor();
    
    return bPushed;
}
