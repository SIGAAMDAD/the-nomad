#include "n_shared.h"
#include "g_game.h"

cvar_t g_vert_fov = {"g_vert_fov","24",TYPE_INT,qtrue};
cvar_t g_horz_fov = {"g_horz_fov","64",TYPE_INT,qtrue};
cvar_t g_pspeed = {"g_pspeed","1.0",TYPE_FLOAT,qtrue};
cvar_t g_gravity = {"g_gravity","1.5",TYPE_FLOAT,qtrue};

cvar_t snd_musicvol = {"snd_musicvol","0.7",TYPE_FLOAT,qtrue};
cvar_t snd_sfxvol = {"snd_sfxvol","1.0",TYPE_FLOAT,qtrue};
cvar_t snd_musicon = {"snd_musicon","true",TYPE_BOOL,qtrue};
cvar_t snd_sfxon = {"snd_sfxon","true",TYPE_BOOL,qtrue};

cvar_t r_ticrate = {"r_ticrate","35",TYPE_INT,qtrue};
cvar_t r_texture_magfilter = {"r_texture_magfilter","GL_NEAREST",TYPE_STRING,qtrue};
cvar_t r_texture_minfilter = {"r_texture_minfilter","GL_LINEAR_MIPMAP_LINEAR",TYPE_STRING,qtrue};
cvar_t r_screenheight = {"r_screenheight","720",TYPE_INT,qtrue};
cvar_t r_screenwidth = {"r_screenwidth","1024",TYPE_INT,qtrue};
cvar_t r_vsync = {"r_vsync","true",TYPE_BOOL,qtrue};
cvar_t r_fullscreen = {"r_fullscreen","false",TYPE_BOOL,qtrue};
cvar_t r_native_fullscreen = {"r_native_fullscreen","false",TYPE_BOOL,qtrue};
cvar_t r_hidden = {"r_hidden","false",TYPE_BOOL,qtrue};
cvar_t r_drawFPS = {"r_drawFPS","false",TYPE_BOOL,qtrue};
cvar_t r_renderapi = {"r_renderapi","R_OPENGL",TYPE_STRING,qtrue};
cvar_t r_msaa_amount = {"r_msaa_amount","OFF",TYPE_STRING,qtrue};

eastl::vector<const byte*> api_extensions;

cvar_t c_fastmobs1 = {"c_fastmobs1","false",TYPE_BOOL,qtrue};
cvar_t c_fastmobs2 = {"c_fastmobs2","false",TYPE_BOOL,qtrue};
cvar_t c_fastmobs3 = {"c_fastmobs3","false",TYPE_BOOL,qtrue};
cvar_t c_deafmobs = {"c_deafmobs","false",TYPE_BOOL,qtrue};
cvar_t c_blindmobs = {"c_blindmobs","false",TYPE_BOOL,qtrue};
cvar_t c_nosmell = {"c_nosmell","false",TYPE_BOOL,qtrue};
cvar_t c_nomobs = {"c_nomobs","false",TYPE_BOOL,qtrue};
cvar_t c_godmode = {"c_godmode","false",TYPE_BOOL,qtrue};
cvar_t c_infinite_ammo = {"c_infinite_ammo","false",TYPE_BOOL,qtrue};
cvar_t c_bottomless_clip = {"c_bottomless_clip","false",TYPE_BOOL,qtrue};
cvar_t c_devmode = {"c_devmode","false",TYPE_BOOL,qtrue};

static cvar_t *cvars[] = {
    &g_vert_fov,
    &g_horz_fov,
    &snd_musicvol,
    &snd_sfxvol,
    &snd_musicon,
    &snd_sfxon,
    &r_ticrate,
    &r_texture_magfilter,
    &r_texture_minfilter,
    &r_screenheight,
    &r_screenwidth,
    &r_vsync,
    &r_fullscreen,
    &r_native_fullscreen,
    &r_hidden,
    &r_drawFPS,
    &r_renderapi,
    &r_msaa_amount,
    &c_fastmobs1,
    &c_fastmobs2,
    &c_fastmobs3,
    &c_deafmobs,
    &c_blindmobs,
    &c_nosmell,
    &c_godmode,
    &c_infinite_ammo,
    &c_bottomless_clip,
    &c_devmode,
    &fs_gamedir,
    &fs_numArchives,
};

#define PRINTARG(x) fmt::arg(#x, x)

uint32_t G_NumCvars(void)
{
    return arraylen(cvars);
}

cvar_t** G_GetCvars(void)
{
    return cvars;
}

static void Cvar_Load(const json& data, const std::string& name, cvar_t* cvar)
{
    if (!cvar->save) {
        return;
    }
    if (cvar->name[0] == 'c' && !data.contains(name)) { // cheats are not required in the config file
        return;
    }
    if (!data.contains(name)) {
        N_Error("Cvar_Load: cvar %s required in configuration file to run the game", name.c_str());
    }
    const std::string value = data[name];

    if (!N_strcmp(cvar->value, value.c_str())) {
        N_strncpy(cvar->value, value.c_str(), (value.size() >= sizeof(cvar->value) ? sizeof(cvar->value) - 1 : value.size()));
    }

    Con_Printf("Initialized cvar %s with value %s", cvar->name, cvar->value);
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

    for (uint32_t i = 0; i < arraylen(cvars); i++) {
        Cvar_Load(data, cvars[i]->name, cvars[i]);
    }
}