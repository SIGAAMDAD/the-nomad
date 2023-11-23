#include "sg_local.h"
#include "sg_imgui.h"

typedef struct
{
    uint32_t timeStart;
    uint32_t timeEnd;

    uint32_t stylePoints;

    uint32_t numDeaths;
    uint32_t numKills;
} levelstats_t;

typedef struct
{
    int32_t index;

    // save data
    levelstats_t stats;
    uint32_t checkpointIndex;
} levelinfo_t;

static levelinfo_t *linfo;

//
// SG_SpawnLevelEntities
//
static void SG_SpawnLevelEntities( void )
{
    uint32_t i;
    const mapspawn_t *spawn;

    spawn = sg.mapInfo.spawns;
    for (i = 0; i < sg.mapInfo.numSpawns; i++, spawn++) {
        switch (spawn->entitytype) {
        case ET_MOB:
            SG_SpawnMobOnMap( spawn->entityid, spawn->xyz[0], spawn->xyz[1], spawn->xyz[2] );
            break;
        case ET_PLAYR:
        case ET_ITEM:
        case ET_WEAPON:
            break;
        };
    }
}

qboolean SG_InitLevel( int32_t levelIndex )
{
    G_Printf( "Starting up level at index %i\n", levelIndex );
    G_Printf( "Allocating resources...\n" );

    linfo = SG_MemAlloc( sizeof(*linfo) );
    if (!linfo) {
        trap_Error( "SG_InitLevel: not enough vm memory" );
    }
    assert( linfo );

    memset( linfo, 0, sizeof(*linfo) );

    G_Printf( "Loading map from internal cache...\n" );

    if (!G_LoadMap( levelIndex, &sg.mapInfo )) {
        SG_Printf( "SG_InitLevel: failed to load map file at index %i\n", levelIndex );
        return qfalse;
    }

    G_Printf( "All done.\n" );

    // spawn everything
    SG_SpawnLevelEntities();

    sg.state = SGAME_IN_LEVEL;

    return qtrue;
}

typedef struct {
    ImGuiWindow window;
} endlevelScreen_t;

static endlevelScreen_t endLevel;

static void SG_DrawLevelStats( void )
{
    float font_scale;
    vec2_t cursorPos;

    font_scale = ImGui_GetFontScale();

    if (ImGui_BeginWindow( &endLevel.window )) {
        ImGui_SetWindowFontScale( font_scale * 6 );
        ImGui_TextUnformatted( "Level Statistics" );
        ImGui_SetWindowFontScale( font_scale * 3.5f );
        ImGui_NewLine();

        ImGui_GetCursorScreenPos( &cursorPos[0], &cursorPos[1] );

        ImGui_SetCursorScreenPos( cursorPos[0], cursorPos[1] + 20 );

    }
    ImGui_EndWindow();
}

int32_t SG_EndLevel( void )
{
    // setup the window
    memset( &endLevel, 0, sizeof(endLevel) );

    endLevel.window.m_Flags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar;
    endLevel.window.m_pTitle = "endLevel";
    endLevel.window.m_bOpen = qtrue;
    endLevel.window.m_bClosable = qfalse;

    // set the state
    sg.state = SGAME_SHOW_LEVEL_STATS;

    return 1;
}
