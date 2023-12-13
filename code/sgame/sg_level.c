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

static levelinfo_t level;

//
// SG_SpawnLevelEntities
//
static void SG_SpawnLevelEntities(void)
{
    uint32_t i;
    const mapspawn_t *spawn;

    spawn = sg.mapInfo.spawns;
    for (i = 0; i < sg.mapInfo.numSpawns; i++, spawn++)
    {
        switch (spawn->entitytype)
        {
        case ET_MOB:
            SG_SpawnMobOnMap(spawn->entityid, spawn->xyz[0], spawn->xyz[1], spawn->xyz[2]);
            break;
        case ET_PLAYR:
            if (sg.playrReady)
            {
                G_Error("SG_InitLevel: there can be only one player spawn per map, make those checkpoints");
            }
            SG_InitPlayer();
            break;
        case ET_ITEM:
        case ET_WEAPON:
            break;
        };
    }
}

qboolean SG_InitLevel(int32_t levelIndex)
{
    G_Printf("Starting up level at index %i\n", levelIndex);
    G_Printf("Allocating resources...\n");

    // clear the old level
    memset(&level, 0, sizeof(level));

    G_Printf("Loading map from internal cache...\n");

    if (!G_LoadMap(levelIndex, &sg.mapInfo))
    {
        SG_Printf("SG_InitLevel: failed to load map file at index %i\n", levelIndex);
        return qfalse;
    }

    G_Printf("Loading map %s...\n", sg.mapInfo.name);

    G_Printf("All done.\n");

    // spawn everything
    SG_SpawnLevelEntities();

    sg.state = SG_IN_LEVEL;

    if (sg_printLevelStats.i)
    {
        G_Printf("\n---------- Level Info ----------\n");
        G_Printf("Map Name: %s\n", sg.mapInfo.name);
        G_Printf("Checkpoint Count: %i\n", sg.mapInfo.numCheckpoints);
        G_Printf("Spawn Count: %i\n", sg.mapInfo.numSpawns);
        G_Printf("Map Width: %i\n", sg.mapInfo.width);
        G_Printf("Map Height: %i\n", sg.mapInfo.height);
    }

    RE_LoadWorldMap(va("maps/%s", sg.mapInfo.name));

    Cvar_Set("sg_levelIndex", va("%i", levelIndex));

    return qtrue;
}

void SG_SaveLevelData(void)
{
}

typedef struct
{
    ImGuiWindow window;
} endlevelScreen_t;

static endlevelScreen_t endLevel;

void SG_DrawLevelStats(void)
{
    float font_scale;
    vec2_t cursorPos;

    font_scale = ImGui_GetFontScale();

    if (ImGui_BeginWindow(&endLevel.window))
    {
        ImGui_SetWindowFontScale(font_scale * 6);
        ImGui_TextUnformatted("Level Statistics");
        ImGui_SetWindowFontScale(font_scale * 3.5f);
        ImGui_NewLine();

        ImGui_GetCursorScreenPos(&cursorPos[0], &cursorPos[1]);

        ImGui_SetCursorScreenPos(cursorPos[0], cursorPos[1] + 20);
    }
    ImGui_EndWindow();
}

//
// SG_DrawAbortMission: returns qtrue if the user wants to end the current level
// via the pause menu
//
int32_t SG_DrawAbortMission(void)
{
    float font_scale;

    font_scale = ImGui_GetFontScale();

    if (ImGui_BeginPopupModal("Abort Mission", endLevel.window.m_Flags))
    {
        ImGui_SetWindowFontScale(font_scale * 3.5f);

        ImGui_TextUnformatted("Are You Sure Want To End The Current Level? Your Most Recent Checkpoint Will Be Saved.");

        if (ImGui_Button("Yes"))
        {
            ImGui_CloseCurrentPopup();
            return qtrue;
        }
        ImGui_SameLine(0.0f);
        if (ImGui_Button("No"))
        {
            ImGui_CloseCurrentPopup();
            return qfalse;
        }

        ImGui_EndPopup();
    }

    return qfalse;
}

int32_t SG_EndLevel(void)
{
    // setup the window
    memset(&endLevel, 0, sizeof(endLevel));

    endLevel.window.m_Flags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar;
    endLevel.window.m_pTitle = "endLevel";
    endLevel.window.m_bOpen = qtrue;
    endLevel.window.m_bClosable = qfalse;

    // are we aborting this mission?
    if (level.checkpointIndex != sg.mapInfo.numCheckpoints - 1)
    {
        sg.state = SG_ABORT_LEVEL;
        ImGui_OpenPopup("Abort Mission");
    }
    else
    {
        sg.state = SG_SHOW_LEVEL_STATS;
    }

    return 1;
}
