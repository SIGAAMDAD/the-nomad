#ifndef _G_GAME_
#define _G_GAME_

#pragma once


inline void N_DebugWindowClear();
inline void N_DebugWindowDraw();

#include "g_entity.h"
#include "g_zone.h"
#include "n_scf.h"
#include "m_renderer.h"
#include "g_rng.h"
#include "g_mob.h"
#include "g_bff.h"
#include "g_map.h"
#include "g_items.h"
#include "p_playr.h"
#include "g_sound.h"


inline clock_t start = clock(), end_time = clock();
inline uint64_t delta;
inline uint64_t sleep_time;
inline uint64_t work;

#ifndef O_BINARY
#define O_BINARY 0
#endif


#define CLOCK_TO_MILLISECONDS(ticks) (((ticks)/(double)CLOCKS_PER_SEC)*1000.0)

#ifdef __unix__
#define sleepfor(x) usleep((x)*1000)
#elif defined(_WIN32)
#define sleepfor(x) Sleep(x)
#endif

bool N_WriteFile(const char* name, const void *buffer, const size_t count);
size_t N_ReadFile(const char* name, char **buffer);

inline std::vector<float> vertices;

inline void N_DebugWindowClear()
{
//    R_ClearScreen();
//    ImGui_ImplSDL2_NewFrame();
//    ImGui_ImplSDLRenderer_NewFrame();
//    ImGui::NewFrame();
    glClear(GL_COLOR_BUFFER_BIT);
}
inline void N_DebugWindowDraw()
{
    /*if (scf::renderer::drawfps) {
        sleep_time = end_time - start;
        ImGui::Begin("FPS", NULL,
            (IMGUI_STANDARD_FLAGS | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoDecoration));
        ImGui::SetWindowPos(ImVec2(SCREEN_WIDTH - 50, 10));
        ImGui::Text("%ld", work + sleep_time);
        ImGui::End();
    } */
//    ImGui::Render();
//    ImGui_ImplSDLRenderer_RenderDrawData(ImGui::GetDrawData());

    SDL_GL_SwapWindow(renderer->window);
    //R_FlushBuffer();

    sleepfor(scf::renderer::ticrate);
}

inline void N_DrawFPS()
{
}

typedef enum : uint8_t
{
    GS_MENU,
    GS_LEVEL,
    GS_PAUSE,
    GS_SETTINGS,

    NUMGAMESTATES
} gamestate_t;

#if 0
template<typename T>
using zone_ptr = std::unique_ptr<T, zone_deleter<T>>;
template<typename T>
inline zone_ptr<T> make_ptr(int tag, void *user)
{ return std::unique_ptr<T, zone_deleter<T>>((T *)Z_Malloc(sizeof(T), tag, user)); }
#endif

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
inline uint64_t ticcount = 0;

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
    uint16_t numplayers;
    playr_t* playrs = NULL; // for when multiplayer is added
    playr_t* playr; // player on the current machine
    gamestate_t gamestate;
    linked_list<entity_t> entities;
    uint8_t difficulty;

    bff_file_t* file = NULL;
    bff_level_t* level = NULL;
public:
    Game() = default;
    ~Game();

    Game(Game &&) = default;
    Game(const Game &) = delete;

    static void Init();
    static inline Game* Get() { return gptr; }
    static inline playr_t* GetPlayr() { return &gptr->playrs[0]; }
    static inline playr_t* GetPlayers() { return gptr->playrs; }
};

inline bool imgui_on = false;

void G_SaveGame();
void G_LoadGame();
json& N_GetSaveJSon();
void G_SpawnItem(item_t item);
void G_SpawnItem(const item_t& item);

inline coord_t E_GetDir(byte dir)
{
    switch (dir) {
    case D_NORTH: return {-1, 0};
    case D_WEST: return {0, -1};
    case D_SOUTH: return {1, 0};
    case D_EAST: return {0, 1};
    };
    LOG_WARN("E_GetDir: dir given is not a valid dir, returning default value of D_NORTH");
    return {-1, 0};
}

void ImGui_Init();
void ImGui_ShutDown();

#include "g_bff.h"

void I_NomadInit(int argc, char** argv);
void N_MainLoop();

#endif