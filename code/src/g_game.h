#ifndef _G_GAME_
#define _G_GAME_

#pragma once

#include "n_scf.h"
#include "g_bff.h"

#ifndef O_BINARY
#define O_BINARY 0
#endif

#define CLOCK_TO_MILLISECONDS(ticks) (((ticks)/(double)CLOCKS_PER_SEC)*1000.0)

#ifdef __unix__
#define sleepfor(x) usleep((x)*1000)
#elif defined(_WIN32)
#define sleepfor(x) Sleep(x)
#endif

typedef enum : uint8_t
{
    GS_MENU,
    GS_LEVEL,
    GS_PAUSE,
    GS_SETTINGS,

    NUMGAMESTATES
} gamestate_t;

enum : uint8_t
{
    OCC_NORMY,
    OCC_EASTER,
    OCC_CHRISTMAS,
    OCC_HALLOWEEN,
    OCC_THANKSGIVING
};

enum : uint8_t
{
    DIF_NOOB,
    DIF_RECRUIT,
    DIF_MERC,
    DIF_NOMAD,
    DIF_BLACKDEATH,
    DIF_MINORINCONVENIECE,

    DIF_HARDEST = DIF_MINORINCONVENIECE
};

inline bool bff_mode = false;
extern uint64_t ticcount;

class Game
{
private:
    static Game* gptr;
public:
    sprite_t c_map[MAP_MAX_X][MAP_MAX_Y];
    char bffname[256];
    char scfname[256];
    char svfile[256];
    SDL_Window* window;
    ImGuiContext* context;
    
    gamestate_t gamestate;
    uint8_t difficulty;
    glm::ivec3 cameraPos;
public:
    Game() = default;
    ~Game();

    Game(Game &&) = default;
    Game(const Game &) = delete;

    static void Init();
    static inline Game* Get() { return gptr; }
};

void G_SaveGame();
void G_LoadGame();

inline coord_t E_GetDir(byte dir)
{
    switch (dir) {
    case D_NORTH: return {-1, 0};
    case D_WEST: return {0, -1};
    case D_SOUTH: return {1, 0};
    case D_EAST: return {0, 1};
    };
    Con_Printf("E_GetDir: dir given is not a valid dir, returning default value of D_NORTH");
    return {-1, 0};
}

void ImGui_Init();
void ImGui_ShutDown();

void I_NomadInit(int argc, char** argv);
void N_MainLoop();

#endif