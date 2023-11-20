#include "sg_local.h"
#include "sg_imgui.h"

void GDR_ATTRIBUTE((format(printf, 1, 2))) GDR_DECL ImGui_SetItemTooltip( const char *fmt, ... )
{
    va_list argptr;
    char msg[4096];
    int length;

    va_start( argptr, fmt );
    length = vsprintf( msg, fmt, argptr );
    va_end( argptr );

    if (length >= sizeof(msg)) {
        trap_Error( "ImGui_SetItemTooltip: buffer overflow" );
    }

    ImGui_SetItemTooltipUnformatted( msg );
}

void GDR_ATTRIBUTE((format(printf, 1, 2))) GDR_DECL ImGui_Text( const char *fmt, ... )
{
    va_list argptr;
    char msg[4096];
    int length;

    va_start( argptr, fmt );
    length = vsprintf( msg, fmt, argptr );
    va_end( argptr );

    if (length >= sizeof(msg)) {
        trap_Error( "ImGui_Text: buffer overflow" );
    }

    ImGui_TextUnformatted( msg );
}

void GDR_ATTRIBUTE((format(printf, 2, 3))) GDR_DECL ImGui_ColoredText( const vec4_t pColor, const char *fmt, ... )
{
    va_list argptr;
    char msg[4096];
    int length;

    va_start( argptr, fmt );
    length = vsprintf( msg, fmt, argptr );
    va_end( argptr );

    if (length >= sizeof(msg)) {
        trap_Error( "ImGui_Text: buffer overflow" );
    }

    ImGui_ColoredTextUnformatted( pColor, msg );
}
