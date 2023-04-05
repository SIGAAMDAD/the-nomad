#include "n_shared.h"
#include "g_game.h"

std::shared_ptr<spdlog::logger> Log::m_Instance;
Console con;
Game* Game::gptr;

void ImGui_ShutDown()
{
    if (imgui_on) {
        imgui_on = false;
        con.ConPrintf("ImGui_ShutDown: deallocating ImGui context");
        ImGui_ImplSDLRenderer_Shutdown();
        ImGui_ImplSDL2_Shutdown();
        ImGui::DestroyContext();
    }
}

void Log::Init()
{
    std::vector<spdlog::sink_ptr> sinks;
    sinks.emplace_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());
    sinks.emplace_back(std::make_shared<spdlog::sinks::basic_file_sink_mt>("Files/debug/debug.log", true));

    sinks[0]->set_pattern("%^[%l]: %v%$");
    sinks[1]->set_pattern("[%l]: %v");

    m_Instance = std::make_shared<spdlog::logger>("GAME", begin(sinks), end(sinks));
    spdlog::register_logger(m_Instance);
    spdlog::set_default_logger(m_Instance);
    m_Instance->set_level(spdlog::level::trace);
    m_Instance->flush_on(spdlog::level::trace);
}

void Game::Init()
{
    gptr = (Game *)Z_Malloc(sizeof(Game), TAG_STATIC, &gptr);
    assert(gptr);
    N_memset(Game::Get()->bffname, 0, sizeof(Game::Get()->bffname));
    N_memset(Game::Get()->scfname, 0, sizeof(Game::Get()->scfname));
    N_memset(Game::Get()->svfile, 0, sizeof(Game::Get()->svfile));

    N_strncpy(Game::Get()->bffname, "nomadmain.bff", sizeof(Game::Get()->bffname));
    N_strncpy(Game::Get()->scfname, "default.scf", sizeof(Game::Get()->scfname));
    N_strncpy(Game::Get()->svfile, "nomadsv.ngd", sizeof(Game::Get()->svfile));
    Game::Get()->gamestate = GS_MENU;

    Game::Get()->playrs = (playr_t *)Z_Malloc(sizeof(playr_t) * 1, TAG_STATIC, &Game::Get()->playrs);
    assert(Game::Get()->playrs);
    Game::Get()->playr = &gptr->playrs[0];
    assert(Game::Get()->playr);
    Game::Get()->playr->pdir = D_NORTH;
}

Game::~Game()
{
    Log::GetLogger()->flush();
    ImGui_ShutDown();
    R_ShutDown();
}