#ifndef _G_GAME_
#define _G_GAME_

#pragma once


inline void N_DebugWindowClear();
inline void N_DebugWindowDraw();

#include "g_zone.h"
#include "n_scf.h"
#include "m_renderer.h"
#include "g_rng.h"
#include "g_map.h"
#include "g_items.h"
#include "p_playr.h"
#include "g_sound.h"
#include "g_mob.h"
#include "g_bff.h"

inline std::chrono::system_clock::time_point start_time;
inline std::chrono::system_clock::time_point end_time;
inline std::chrono::duration<double, std::milli> work_time, sleep_time;

#define CLOCK_TO_MILLISECONDS(ticks) (((ticks)/(double)CLOCKS_PER_SEC)*1000.0)

inline void N_DebugWindowClear()
{
    R_ClearScreen();
    ImGui_ImplSDL2_NewFrame();
    ImGui_ImplSDLRenderer_NewFrame();
    ImGui::NewFrame();

    start_time = std::chrono::system_clock::now();
    work_time = start_time - end_time;
    if (work_time.count() < scf::renderer::framerate) {
        std::chrono::duration<double, std::milli> delta_ms(scf::renderer::framerate - work_time.count());
        auto delta_ms_duration = std::chrono::duration_cast<std::chrono::milliseconds>(delta_ms);
        std::this_thread::sleep_for(std::chrono::milliseconds(delta_ms_duration.count()));
    }
}
inline void N_DebugWindowDraw()
{
    end_time = std::chrono::system_clock::now();
    if (scf::renderer::drawfps) {
        sleep_time = end_time - start_time;
        ImGui::Begin("FPS", NULL,
            (IMGUI_STANDARD_FLAGS | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoDecoration));
        ImGui::SetWindowPos(ImVec2(SCREEN_WIDTH - 50, 10));
        ImGui::Text("%f", (work_time + sleep_time).count());
        ImGui::End();
    }
    ImGui::Render();
    ImGui_ImplSDLRenderer_RenderDrawData(ImGui::GetDrawData());
    R_FlushBuffer();
}

inline void N_DrawFPS()
{
}

typedef enum : uint8_t
{
    GS_MENU,
    GS_LEVEL,
    GS_PAUSE,
    GS_LOAD,
    GS_SAVE,
    GS_SETTINGS,

    NUMGAMESTATES
} gamestate_t;

template <auto fn>
struct zone_deleter {
    template <typename T>
    constexpr void operator()(T* arg) const { Z_Free(arg); }
};

template<typename T>
using zone_ptr = std::unique_ptr<T, zone_deleter<T>>;
template<typename T>
inline zone_ptr<T> make_ptr(int tag, void *user)
{ return std::unique_ptr<T, zone_deleter<T>>((T *)Z_Malloc(sizeof(T), tag, user)); }

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
    playr_t* playrs = NULL; // for when multiplayer is added
    playr_t* playr; // player on the current machine
    gamestate_t gamestate;
    linked_list<Mob*> m_Active;
    item_t* i_Active;

    std::shared_ptr<BFF> bff;
    bff_level_t* level = NULL;
public:
    Game() = default;
    ~Game();

    Game(Game &&) = default;
    Game(const Game &) = delete;

    static void Init();
    static inline Game* Get() { return gptr; }
    static inline linked_list<Mob*>& GetMobs() { return gptr->m_Active; }
    static inline playr_t* GetPlayr() { return &gptr->playrs[0]; }
    static inline playr_t* GetPlayers() { return gptr->playrs; }
};

void

void G_SaveGame(const char* svfile);
void G_LoadGame(const char* svfile);
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

#include "g_zone.h"

void I_NomadInit();
void N_MainLoop();

#endif