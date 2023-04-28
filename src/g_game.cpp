#include "n_shared.h"
#include "g_game.h"

std::shared_ptr<spdlog::logger> Log::m_Instance;
Console con;
Game* Game::gptr;

bool N_WriteFile(const char* name, const void *buffer, const size_t count)
{
    assert(buffer);
    FILE* fp;
    size_t size;

    fp = fopen(name, "wb");
    if (!fp)
        return false;
    assert(fp);
    size = fwrite(buffer, sizeof(char), count, fp);
    if (size < count || size == 0)
        return false;
    fclose(fp);
    return true;
}

size_t N_ReadFile(const char* name, char **buffer)
{
    assert(buffer);
    FILE* fp;
    size_t size, fsize;
    void *buf;
    
    fp = fopen(name, "rb");
    if (!fp)
        N_Error("N_ReadFile: failed to fopen() file %s", name);
    assert(fp);
    fseek(fp, 0L, SEEK_END);
    fsize = ftell(fp);
    fseek(fp, 0L, SEEK_SET);
    buf = malloc(fsize);
    if (!buf)
        N_Error("N_ReadFile: %s failed", "malloc()");
    size = fread(buf, sizeof(char), fsize, fp);
    if (size < fsize)
        N_Error("N_ReadFile: failed to fread() file %s", name);
    fclose(fp);
    *buffer = (char *)buf;
    return size;
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
    memset(Game::Get()->bffname, 0, sizeof(Game::Get()->bffname));
    memset(Game::Get()->scfname, 0, sizeof(Game::Get()->scfname));
    memset(Game::Get()->svfile, 0, sizeof(Game::Get()->svfile));
    memset(Game::Get()->c_map, SPR_FLOOR_OUTSIDE, sizeof(Game::Get()->c_map));

    strncpy(Game::Get()->bffname, "nomadmain.bff", sizeof(Game::Get()->bffname));
    strncpy(Game::Get()->scfname, "default.scf", sizeof(Game::Get()->scfname));
    strncpy(Game::Get()->svfile, "nomadsv.ngd", sizeof(Game::Get()->svfile));
    Game::Get()->gamestate = GS_MENU;

    Game::Get()->playrs = (playr_t *)Z_Malloc(sizeof(playr_t) * 10, TAG_STATIC, &Game::Get()->playrs);
    assert(Game::Get()->playrs);
    Game::Get()->playr = &gptr->playrs[0];
    assert(Game::Get()->playr);
    playr_t* const playr = Game::GetPlayr();

    memset(playr->P_wpns, 0, sizeof(weapon_t) * arraylen(playr->P_wpns));
    memset(playr->inv, 0, sizeof(item_t) * arraylen(playr->inv));
}

Game::~Game()
{
    Log::GetLogger()->flush();
    if (!bff_mode) {
//        ImGui_ShutDown();
        Snd_Kill();
        R_ShutDown();
    }
}