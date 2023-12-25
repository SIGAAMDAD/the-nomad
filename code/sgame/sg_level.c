#include "sg_local.h"
#include "sg_imgui.h"

typedef struct
{
    int timeStart;
    int timeEnd;

    int stylePoints;

    int numDeaths;
    int numKills;
} levelstats_t;

typedef struct
{
    int32_t index;

    // save data
    levelstats_t stats;
    int checkpointIndex;
} levelinfo_t;

static levelinfo_t level;

//
// SG_SpawnLevelEntities
//
static void SG_SpawnLevelEntities(void)
{
    int i;
    const mapspawn_t *spawn;

    spawn = sg.mapInfo.spawns;
    for ( i = 0; i < sg.mapInfo.numSpawns; i++, spawn++ ) {
        switch ( spawn->entitytype ) {
        case ET_MOB:
            break;
        case ET_PLAYR:
            break;
        case ET_ITEM:
        case ET_WEAPON:
            break;
        };
    }
}

qboolean SG_InitLevel( int levelIndex )
{
    vec2_t cameraPos;
    float zoom;

    G_Printf( "Starting up level at index %i\n", levelIndex );
    G_Printf( "Loadng resources...\n" );

    // clear the old level
    memset( &level, 0, sizeof(level) );

    G_Printf( "Loading map from internal cache...\n" );

    if ( !G_LoadMap( levelIndex, &sg.mapInfo, sg.soundBits, &sg.activeEnts ) ) {
        SG_Printf( COLOR_RED "SG_InitLevel: failed to load map file at index %i\n", levelIndex );
        return qfalse;
    }

    G_Printf( "Loading map %s...\n", sg.mapInfo.name );

    G_Printf( "All done.\n" );

    SG_InitPlayer();

    // spawn everything
    SG_SpawnLevelEntities();

    sg.state = SG_IN_LEVEL;

    if ( sg_printLevelStats.i ) {
        G_Printf( "\n---------- Level Info ----------\n" );
        G_Printf( "Map Name: %s\n", sg.mapInfo.name );
        G_Printf( "Checkpoint Count: %i\n", sg.mapInfo.numCheckpoints );
        G_Printf( "Spawn Count: %i\n", sg.mapInfo.numSpawns );
        G_Printf( "Map Width: %i\n", sg.mapInfo.width );
        G_Printf( "Map Height: %i\n", sg.mapInfo.height );
    }

    RE_LoadWorldMap( va( "maps/%s", sg.mapInfo.name ) );

    Cvar_Set( "sg_levelIndex", va( "%i", levelIndex ) );

    level.stats.timeStart = trap_Milliseconds();

    VectorCopy2( cameraPos, sg.mapInfo.spawns[0].xyz );
    zoom = 1.6f;

//    G_SetCameraData( cameraPos, zoom, 0.0f );

    return qtrue;
}

void SG_SaveLevelData( void )
{
}

void SG_DrawLevelStats( void )
{
    float font_scale;
    vec2_t cursorPos;

    font_scale = ImGui_GetFontScale();

    if ( ImGui_BeginWindow( "EndLevel", NULL, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar ) ) {
        ImGui_SetWindowFontScale(font_scale * 6);
        ImGui_TextUnformatted("Level Statistics");
        ImGui_SetWindowFontScale(font_scale * 3.5f);
        ImGui_NewLine();

        ImGui_GetCursorScreenPos( &cursorPos.x, &cursorPos.y );

        ImGui_SetCursorScreenPos( cursorPos.x, cursorPos.y + 20);
    }
    ImGui_EndWindow();
}

int SG_EndLevel( void )
{
    level.stats.timeEnd = trap_Milliseconds();

    sg.state = SG_SHOW_LEVEL_STATS;

    return 1;
}
