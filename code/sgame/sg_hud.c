#include "sg_local.h"
#include "sg_imgui.h"

void SG_HudDraw( void )
{
    const sgentity_t *data;
    vec4_t color;

    data = sg.playr.ent;

    ImGui_BeginWindow( "HEALTH", NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_AlwaysAutoResize );
    ImGui_SetWindowPos( 16.0f * sg.scale, 16.0f * sg.scale );

    if ( data->health >= 50 ) {
        color.r = 0;
        color.g = 1;
        color.b = 0;
        color.a = 1;
        ImGui_PushColor( ImGuiCol_FrameBg, &color );
    }
    else if ( data->health < 50 ) {
        color.r = 1;
        color.g = 1;
        color.b = 0;
        color.a = 1;

        ImGui_PushColor( ImGuiCol_FrameBg, &color );
    }
    ImGui_ProgressBar( (float)data->health );
    ImGui_PopColor();
    
    ImGui_EndWindow();
}
