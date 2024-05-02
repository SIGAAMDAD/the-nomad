#include "module_public.h"

void CModuleLib::CrashWindow( void )
{
    const int windowFlags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse;

    ImGui::Begin( "##CrashWindow", (bool *)&m_bCrashWindow, windowFlags );
    ImGui::End();
}
