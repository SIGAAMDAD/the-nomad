#include "n_shared.h"
#include "g_game.h"

std::shared_ptr<spdlog::logger> Log::m_Instance;
Console con;
Game* Game::gptr;

#ifndef O_BINARY
#define O_BINARY 0
#endif

bool N_WriteFile(const char* name, const void *buffer, const size_t count)
{
    int handle;
    ssize_t size;
#ifdef __unix__
    handle = open(name, O_WRONLY | O_CREAT | O_TRUNC | O_BINARY, (mode_t)0666);
    if (handle == -1)
        return false;
    size = write(handle, buffer, count);
    if (size < count)
        return false;
    close(handle);
#elif defined(_WIN32)
    handle = _open(name, O_WRONLY | O_CREAT | O_TRUNC | O_BINARY, (mode_t)0666);
    if (handle == -1)
        return false;
    size = _write(handle, buffer, count);
    if (size < count)
        return false;
    _close(handle);
#endif
    return true;
}
size_t N_ReadFile(const char* name, char **buffer)
{
    assert(buffer);
    FILE* fp = fopen(name, "rb");
    if (!fp)
        N_Error("N_ReadFile: failed to open file %s", name);
    fseek(fp, 0L, SEEK_END);
    size_t size = ftell(fp);
    fseek(fp, 0L, SEEK_SET);
    void *buf = malloc(size);
    size_t count = fread(buf, sizeof(char), size, fp);
    *buffer = (char *)buf;
    fclose(fp);
    return count;
}

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
    N_memset(Game::Get()->c_map, SPR_FLOOR_OUTSIDE, sizeof(Game::Get()->c_map));

    N_strncpy(Game::Get()->bffname, "nomadmain.bff", sizeof(Game::Get()->bffname));
    N_strncpy(Game::Get()->scfname, "default.scf", sizeof(Game::Get()->scfname));
    N_strncpy(Game::Get()->svfile, "nomadsv.ngd", sizeof(Game::Get()->svfile));
    Game::Get()->gamestate = GS_MENU;

    Game::Get()->playrs = (playr_t *)Z_Malloc(sizeof(playr_t) * 1, TAG_STATIC, &Game::Get()->playrs);
    assert(Game::Get()->playrs);
    Game::Get()->playr = &gptr->playrs[0];
    assert(Game::Get()->playr);
    playr_t* const playr = Game::GetPlayr();
    playr->pdir = D_NORTH;
    playr->health = PLAYR_MAX_HEALTH;
    playr->armor = ARMOR_STREET;
    playr->xp = 0;
    playr->level = 0;
    playr->pos.pos = {0, 0};

    playr->P_wpns.resize(PLAYR_MAX_WPNS);
    N_memset(playr->P_wpns.data(), 0, PLAYR_MAX_WPNS * sizeof(weapon_t));
    playr->c_wpn = &playr->P_wpns.front();

    playr->inv.resize(PLAYR_MAX_ITEMS);
    N_memset(playr->inv.data(), 0, PLAYR_MAX_ITEMS * sizeof(item_t));
}

Game::~Game()
{
    Log::GetLogger()->flush();
    ImGui_ShutDown();
    Snd_Kill();
    R_ShutDown();
}