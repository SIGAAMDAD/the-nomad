#include "sg_local.h"
#include "sg_imgui.h"

void GDR_ATTRIBUTE((format(printf, 1, 2))) GDR_DECL ImGui_SetItemTooltip( const char *fmt, ... )
{
    va_list argptr;
    char msg[8192];
    
    va_start( argptr, fmt );
    vsprintf( msg, fmt, argptr );
    va_end( argptr );

    ImGui_SetItemTooltipUnformatted( msg );
}

void GDR_ATTRIBUTE((format(printf, 1, 2))) GDR_DECL ImGui_Text( const char *fmt, ... )
{
    va_list argptr;
    char msg[8192];

    va_start( argptr, fmt );
    vsprintf( msg, fmt, argptr );
    va_end( argptr );

    ImGui_TextUnformatted( msg );
}

void GDR_ATTRIBUTE((format(printf, 2, 3))) GDR_DECL ImGui_ColoredText( const vec4_t *pColor, const char *fmt, ... )
{
    va_list argptr;
    char msg[8192];

    va_start( argptr, fmt );
    vsprintf( msg, fmt, argptr );
    va_end( argptr );

    ImGui_ColoredTextUnformatted( pColor, msg );
}
