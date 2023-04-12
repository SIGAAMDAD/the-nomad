#include "n_shared.h"
#include "g_game.h"

bool sdl_on = false;
static bool exited = false;
int myargc;
char** myargv;

#ifndef _WIN32
#define LoadLibraryA(x) dlopen((x), RTLD_NOW | RTLD_LOCAL)
#define GetProcAddress(a,b) dlsym((a),(b))
#define FreeLibrary(x) dlclose((x))
using HMODULE = void*;
#endif

HMODULE G_LoadLibrary(const char *lib)
{
    HMODULE handle;
#ifdef _WIN32
    if ((handle = GetModuleHandleA(lib)) != NULL)
        return (void *)NULL;
#elif defined(__unix__)
    if (*lib == '\0')
        return (void *)NULL;
#endif
    handle = LoadLibraryA(lib);
    return handle;
}

void *G_LoadSym(HMODULE handle, const char *name)
{
    return (void *)GetProcAddress((HMODULE)handle, name);
}

#define LOAD(ptr,name) \
{ \
    *((void **)&ptr) = G_LoadSym(handle,name); \
    if (!ptr) N_Error("failed to load library symbol %s", name); \
}

void G_LoadBZip2()
{
    constexpr const char* libname = "Files/deps/libbz2.so.1.0.4";
    HMODULE handle = G_LoadLibrary(libname);
    
    LOAD(bzip2_bufcompress, "BZ2_bzBuffToBuffCompress");
    LOAD(bzip2_bufdecompress, "BZ2_bzBuffToBuffDecompress");
}

void G_LoadSndFile()
{
    constexpr const char* libname = "Files/deps/libsndfile.so.1.0.31";
    HMODULE handle = G_LoadLibrary(libname);

    LOAD(sndfile_open, "sf_open");
    LOAD(sndfile_close, "sf_close");
    LOAD(sndfile_readshort, "sf_read_short");
}

int I_GetParm(const char* name)
{
    if (myargc < 2)
        return -1;
    for (int i = 1; i < myargc; ++i) {
        if (N_strncmp(name, myargv[i], N_strlen(name))) {
            return i;
        }
    }
    return -1;
}

void N_Error(const char *err, ...)
{
    char msg[1024];
    N_memset(msg, 0, sizeof(msg));
    va_list argptr;
    va_start(argptr, err);
    vsnprintf(msg, sizeof(msg), err, argptr);
    va_end(argptr);
    if (sdl_on) {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Engine Error (Fatal)", msg, Game::Get()->window);
    }
    con.ConError("Error: {}", msg);
    con.ConFlush();
    Game::Get()->~Game();
    Log::GetLogger()->flush();
    exit(EXIT_FAILURE);
}


void ImGui_Init()
{
    con.ConPrintf("ImGui_Init: initializing and allocating an ImGui_SDLRenderer context");
    IMGUI_CHECKVERSION();

    Game::Get()->context = ImGui::CreateContext();
    ImGui::SetCurrentContext(Game::Get()->context);
    ImGui_ImplSDL2_InitForSDLRenderer(renderer->SDL_window, R_GetRenderer());
    ImGui_ImplSDLRenderer_Init(R_GetRenderer());
    imgui_on = true;
}

void I_NomadInit(int argc, char** argv)
{
    myargc = argc;
    myargv = argv;

    G_LoadBZip2();
    G_LoadSndFile();

    Z_Init();

    con.ConPrintf("setting up logger");
    Log::Init();

    int i = I_GetParm("-bff=write");
    if (i != -1) {
        write_bff_mode = true;
        G_WriteBFF(myargv[i + 1], myargv[i + 2]);
    }

    Game::Init();

    fprintf(stdout,
        "+===========================================================+\n"
         "\"The Nomad\" is free software distributed under the terms\n"
         "of both the GNU General Public License v2.0 and Apache License\n"
         "v2.0\n"
         "+==========================================================+\n"
    );

    con.ConPrintf("G_LoadSCF: parsing scf file");
    G_LoadSCF();

    con.ConPrintf("G_LoadBFF: loading bff file");
    G_LoadBFF();

    LOG_INFO("initializing renderer");
    R_Init();

    LOG_INFO("setting up imgui");
    ImGui_Init();

    con.ConFlush();

    LOG_INFO("running main gameplay loop");
    N_MainLoop();
}