#include "n_shared.h"
#include "g_game.h"

int g_vert_fov;
int g_horz_fov;
float g_pspeed;
float g_gravity;

float snd_musicvol;
float snd_sfxvol;
qboolean snd_musicon;
qboolean snd_sfxon;

int r_ticrate;
int r_texture_magfilter;
int r_texture_minfilter;
int r_screenheight;
int r_screenwidth;
int r_renderapi;
qboolean r_vsync;
qboolean r_fullscreen;
qboolean r_hidden;
qboolean r_drawFPS;
int r_renderapi;
int r_msaa_amount;

nomadvector<const byte*> api_extensions;

qboolean c_fastmobs1 = qfalse;
qboolean c_fastmobs2 = qfalse;
qboolean c_fastmobs3 = qfalse;
qboolean c_deafmobs = qfalse;
qboolean c_blindmobs = qfalse;
qboolean c_nosmell = qfalse;
qboolean c_nomobs = qfalse;
qboolean c_godmode = qfalse;
qboolean c_infinite_ammo = qfalse;
qboolean c_bottomless_clip = qfalse;
qboolean c_devmode = qfalse;

static vmCvar_t cvars[] = {
    {"g_vert_fov", (char *)&g_vert_fov, TYPE_INT},
    {"g_horz_fov", (char *)&g_horz_fov, TYPE_INT},
    {"g_pspeed", (char *)&g_pspeed, TYPE_FLOAT},
    {"g_gravity", (char *)&g_gravity, TYPE_FLOAT},
    {"snd_musicvol", (char *)&snd_musicvol, TYPE_FLOAT},
    {"snd_musicon", (char *)&snd_musicon, TYPE_BOOL},
    {"snd_sfxvol", (char *)&snd_sfxvol, TYPE_FLOAT},
    {"snd_sfxon", (char *)&snd_sfxon, TYPE_BOOL},
    {"c_fastmobs1", (char *)&c_fastmobs1, TYPE_BOOL},
    {"c_fastmobs2", (char *)&c_fastmobs2, TYPE_BOOL},
    {"c_fastmobs3", (char *)&c_fastmobs3, TYPE_BOOL},
    {"c_deafmobs", (char *)&c_deafmobs, TYPE_BOOL},
    {"c_blindmobs", (char *)&c_blindmobs, TYPE_BOOL},
    {"c_nosmell", (char *)&c_nosmell, TYPE_BOOL},
    {"c_nomobs", (char *)&c_nomobs, TYPE_BOOL},
    {"c_godmode", (char *)&c_godmode, TYPE_BOOL},
    {"c_infinite_ammo", (char *)&c_infinite_ammo, TYPE_BOOL},
    {"c_bottomless_clip", (char *)&c_bottomless_clip, TYPE_BOOL},
    {"c_devmode", (char *)&c_devmode, TYPE_BOOL},
    {"r_ticrate", (char *)&r_ticrate, TYPE_INT},
    {"r_texture_magfilter", (char *)&r_texture_magfilter, TYPE_INT},
    {"r_texture_minfilter", (char *)&r_texture_minfilter, TYPE_INT},
    {"r_screenheight", (char *)&r_screenheight, TYPE_INT},
    {"r_screenwidth", (char *)&r_screenwidth, TYPE_INT},
    {"r_renderapi", (char *)&r_renderapi, TYPE_INT},
    {"r_vsync", (char *)&r_vsync, TYPE_BOOL},
    {"r_fullscreen", (char *)&r_fullscreen, TYPE_BOOL},
    {"r_hidden", (char *)&r_hidden, TYPE_BOOL}
};


#define PRINTARG(x) fmt::arg(#x, x)

uint32_t G_NumCvars(void)
{
    return arraylen(cvars);
}

vmCvar_t* G_GetCvars(void)
{
    return cvars;
}

void G_LoadSCF()
{
    std::ifstream stream("default.scf", std::ios::in);
    if (stream.fail()) {
        N_Error("G_LoadSCF: failed to open default.scf");
    }
    json data;
    try {
        data = json::parse(stream);
    } catch (const json::exception& e) {
        N_Error("G_LoadSCF: json error occurred, error id: %i, error message: %s", e.id, e.what());
    }
    stream.close();

    std::vector<std::string> vars = data["cvars"];
    for (const auto& v : vars) {
        for (uint32_t i = 0; i < arraylen(cvars); i++) {
            if (i == cvars[i].name) {
                switch (cvars[i].type) {
                case TYPE_INT:
                    *(int *)cvars[i].value = atoi(v.c_str());
                    break;
                case TYPE_BOOL:
                    *(qboolean *)cvars[i].value = v == "true" ? qtrue : qfalse;
                    break;
                case TYPE_FLOAT:
                    *(float *)cvars[i].value = atof(v.c_str());
                    break;
                case TYPE_STRING:
                    strncpy(cvars[i].value, v.c_str(), strlen(cvars[i].value));
                    break;
                };
            }
        }
    }
}
