#include "n_shared.h"
#include "g_game.h"

/*
char s[64];
float f;
int32_t i;
qboolean b;
*/

keybind_t kb_binds[NUMBINDS] = {
    {"Move Forward",        SDLK_w,         KMOD_NONE,  SDL_KEYDOWN, kbMoveUp},
    {"Move Backwards",      SDLK_s,         KMOD_NONE,  SDL_KEYDOWN, kbMoveDown},
    {"Move Left",           SDLK_a,         KMOD_NONE,  SDL_KEYDOWN, kbMoveLeft},
    {"Move Right",          SDLK_d,         KMOD_NONE,  SDL_KEYDOWN, kbMoveRight},

    {"Camera Forward",      SDLK_UP,        KMOD_NONE,  SDL_KEYDOWN, kbCameraUp},
    {"Camera Backwards",    SDLK_DOWN,      KMOD_NONE,  SDL_KEYDOWN, kbCameraDown},
    {"Camera Left",         SDLK_LEFT,      KMOD_NONE,  SDL_KEYDOWN, kbCameraLeft},
    {"Camera Right",        SDLK_RIGHT,     KMOD_NONE,  SDL_KEYDOWN, kbCameraRight},
    {"Zoom In",             SDLK_m,         KMOD_NONE,  SDL_KEYDOWN, kbZoomIn},
    {"Zoom Out",            SDLK_n,         KMOD_NONE,  SDL_KEYDOWN, kbZoomOut},

    {"Shoot",               (SDL_KeyCode)0, KMOD_NONE,  SDL_KEYDOWN, kbShoot},
    {"Jump",                SDLK_SPACE,     KMOD_NONE,  SDL_KEYDOWN, kbJump},
    {"Crouch",              SDLK_LCTRL,     KMOD_NONE,  SDL_KEYDOWN, kbCrouch},
    
    {"Open Console",        SDLK_BACKQUOTE, KMOD_NONE,  SDL_KEYDOWN, kbConsole},
    {"Exit",                SDLK_ESCAPE,    KMOD_NONE,  SDL_KEYDOWN, kbExit}
};

//cvar_t g_vert_fov          = {"g_vert_fov",        "", 0.0f, 24, qfalse,   TYPE_INT, CVG_ENGINE, CVAR_SAVE};
//cvar_t g_horz_fov          = {"g_horz_fov",        "", 0.0f, 64, qfalse,   TYPE_INT, CVG_ENGINE, CVAR_SAVE};
//cvar_t g_pspeed            = {"g_pspeed",          "", 1.0f, 0,  qfalse, TYPE_FLOAT, CVG_ENGINE, CVAR_SAVE};
//cvar_t g_gravity           = {"g_gravity",         "", 1.5f, 0,  qfalse, TYPE_FLOAT, CVG_ENGINE, CVAR_SAVE};

//cvar_t snd_musicvol        = {"snd_musicvol",      "", 0.7f, 0,  qfalse, TYPE_FLOAT, CVG_ENGINE, CVAR_SAVE};
//cvar_t snd_sfxvol          = {"snd_sfxvol",        "", 1.0f, 0,  qfalse, TYPE_FLOAT, CVG_ENGINE, CVAR_SAVE};
//cvar_t snd_musicon         = {"snd_musicon",       "", 0.0f, 0,  qtrue,   TYPE_BOOL, CVG_ENGINE, CVAR_SAVE};
//cvar_t snd_sfxon           = {"snd_sfxon",         "", 0.0f, 0,  qtrue,   TYPE_BOOL, CVG_ENGINE, CVAR_SAVE};

//cvar_t c_fastmobs1         = {"c_fastmobs1",       "", 0.0f, 0, qfalse, TYPE_BOOL, CVG_ENGINE, CVAR_SAVE | CVAR_CHEAT};
//cvar_t c_fastmobs2         = {"c_fastmobs2",       "", 0.0f, 0, qfalse, TYPE_BOOL, CVG_ENGINE, CVAR_SAVE | CVAR_CHEAT};
//cvar_t c_fastmobs3         = {"c_fastmobs3",       "", 0.0f, 0, qfalse, TYPE_BOOL, CVG_ENGINE, CVAR_SAVE | CVAR_CHEAT};
//cvar_t c_deafmobs          = {"c_deafmobs",        "", 0.0f, 0, qfalse, TYPE_BOOL, CVG_ENGINE, CVAR_SAVE | CVAR_CHEAT};
//cvar_t c_blindmobs         = {"c_blindmobs",       "", 0.0f, 0, qfalse, TYPE_BOOL, CVG_ENGINE, CVAR_SAVE | CVAR_CHEAT};
//cvar_t c_nosmell           = {"c_nosmell",         "", 0.0f, 0, qfalse, TYPE_BOOL, CVG_ENGINE, CVAR_SAVE | CVAR_CHEAT};
//cvar_t c_nomobs            = {"c_nomobs",          "", 0.0f, 0, qfalse, TYPE_BOOL, CVG_ENGINE, CVAR_SAVE | CVAR_CHEAT};
//cvar_t c_godmode           = {"c_godmode",         "", 0.0f, 0, qfalse, TYPE_BOOL, CVG_ENGINE, CVAR_SAVE | CVAR_CHEAT};
//cvar_t c_infinite_ammo     = {"c_infinite_ammo",   "", 0.0f, 0, qfalse, TYPE_BOOL, CVG_ENGINE, CVAR_SAVE | CVAR_CHEAT};
//cvar_t c_bottomless_clip   = {"c_bottomless_clip", "", 0.0f, 0, qfalse, TYPE_BOOL, CVG_ENGINE, CVAR_SAVE | CVAR_CHEAT};
#ifdef _NOMAD_DEBUG
//cvar_t c_devmode           = {"c_devmode",         "", 0.0f, 0,  qtrue, TYPE_BOOL, CVG_ENGINE, CVAR_SAVE | CVAR_CHEAT};
//cvar_t c_cheatsallowed     = {"c_cheatsallowed",   "", 0.0f, 0,  qtrue, TYPE_BOOL, CVG_ENGINE, CVAR_SAVE};
#else
//cvar_t c_devmode           = {"c_devmode",         "", 0.0f, 0, qfalse, TYPE_BOOL, CVG_ENGINE, CVAR_SAVE | CVAR_CHEAT};
//cvar_t c_cheatsallowed     = {"c_cheatsallowed",   "", 0.0f, 0, qfalse, TYPE_BOOL, CVG_ENGINE, CVAR_SAVE};
#endif

static void SCF_ParseFile(void)
{
    int parm = I_GetParm("-config");
    if (parm != -1) {

    }
}

#define NOMAD_CONFIG "default.cfg"

/*
Com_LoadConfig: loads the default configuration file
*/
void Com_LoadConfig(void)
{
    Cmd_ExecuteText("exec " NOMAD_CONFIG);
}

void G_LoadSCF(void){
    std::ifstream stream("gamedata/default.scf", std::ios::in);
    if (stream.fail()) {
        N_Error("G_LoadSCF: failed to open default.scf");
    }

    json data;
    try {
        data = json::parse(stream);
    } catch (const json::exception& e) {
        N_Error("G_LoadSCF: json error occurred, error id: %i, error message: %s", e.id, e.what());
    }
}