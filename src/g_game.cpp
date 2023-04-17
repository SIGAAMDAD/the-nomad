#include "n_shared.h"
#include "g_game.h"

std::shared_ptr<spdlog::logger> Log::m_Instance;
Console con;
Game* Game::gptr;

bool N_WriteFile(const char* name, const void *buffer, const ssize_t count)
{
    int handle;
    ssize_t size;
#ifdef __unix__
    handle = open(name, O_WRONLY | O_CREAT | O_TRUNC | O_BINARY, (mode_t)0666);
    if (handle == -1)
        return false;
    size = write(handle, buffer, count);
    if (size < count || size == -1)
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
ssize_t N_ReadFile(const char* name, char **buffer)
{
    assert(buffer);
#ifdef __unix__
    int handle = open(name, O_RDONLY | O_BINARY, (mode_t)0666);
    if (handle == -1)
        N_Error("N_ReadFile: failed to open() file %s", name);
    struct stat fdata;
    if (fstat(handle, &fdata) == -1)
        N_Error("N_ReadFile: failed to fstat() file %s", name);
#elif defined(_WIN32)
    int handle = _open(name, O_RDONLY | O_BINARY, (mode_t)0666);
    if (handle == -1)
        N_Error("N_ReadFile: failed to open() file %s", name);
    struct _stati64 fdata;
    if (_fstati64(handle, &fdata) == -1)
        N_Error("N_ReadFile: failed to fstat() file %s", name);
#endif
    
    void *buf = malloc(fdata.st_size);
    if (!buf)
        N_Error("N_ReadFile: malloc() failed");
#ifdef __unix__
    ssize_t size = read(handle, buf, fdata.st_size);
    if (size < fdata.st_size || size == -1)
        N_Error("N_ReadFile: failed to read() file %s", name);
    close(handle);
#elif defined(_WIN32)
    ssize_t size = _read(handle, buf, fdata.st_size);
    if (size < fdata.st_size || size == -1)
        N_Error("N_ReadFile: failed to read() file %s", name);
    _close(handle);
#endif
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

    Game::Get()->playrs = (playr_t *)Z_Malloc(sizeof(playr_t) * 1, TAG_STATIC, &Game::Get()->playrs);
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
        ImGui_ShutDown();
        Snd_Kill();
        R_ShutDown();
    }
}