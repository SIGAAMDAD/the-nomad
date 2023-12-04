#include "sg_local.h"
#include "sg_imgui.h"

typedef struct {
    ImGuiWindow health;
    ImGuiWindow weapons;
} hudData_t;

static hudData_t *hud;

void Hud_Draw( void )
{
    ImGui_BeginWindow( &hud->health );
    ImGui_SetWindowPos( 16 * sg.scale, 16 * sg.scale );
    ImGui_Text( "Health: %lu", sg.playr->ent->health );
    ImGui_EndWindow();

    ImGui_BeginWindow( &hud->weapons );
    ImGui_EndWindow();
}

void Hud_Init( void )
{
    const int statusWindowFlags =
        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoMouseInputs | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar;

    hud = (hudData_t *)SG_MemAlloc( sizeof(*hud) );
    memset( hud, 0, sizeof(*hud) );

    hud->health.m_bClosable = qfalse;
    hud->health.m_bOpen = qtrue;
    hud->health.m_Flags = statusWindowFlags;
    hud->health.m_pTitle = "HealthStatus";

    hud->weapons.m_bClosable = qfalse;
    hud->weapons.m_bClosable = qfalse;
    hud->weapons.m_bOpen = qtrue;
    hud->weapons.m_Flags = statusWindowFlags;
    hud->weapons.m_pTitle = "WeaponStatus";
}
