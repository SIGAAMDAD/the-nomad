#include "../src/n_shared.h"
#include "sg_local.h"
#include "sg_playr.h"
#include "sg_mob.h"
#include "sg_game.h"

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

qboolean c_fastmobs1;
qboolean c_fastmobs2;
qboolean c_fastmobs3;
qboolean c_deafmobs;
qboolean c_blindmobs;
qboolean c_nosmell;
qboolean c_nomobs;
qboolean c_godmode;
qboolean c_infinite_ammo;
qboolean c_bottomless_clip;
qboolean c_devmode;

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
    {"c_deafmobs", (char *)&c_deadmobs, TYPE_BOOL},
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

world_t sg_world;
playr_t playrs[MAX_PLAYR_COUNT];
mobj_t mobs[MAX_MOBS_ACTIVE];

int vmMain(int command, int arg0, int arg1, int arg2, int arg3, int arg4, int arg5, int arg6, int arg7,
    int arg8, int arg9, int arg10)
{
    switch (command) {
    case SGAME_INIT:
        return G_Init();
    case SGAME_SHUTDOWN:
        return G_Shutdown();
    default:
        Con_Error("vmMain: invalid command id: %i", command);
    };
    return -1;
}

const mobj_t mobinfo[NUMMOBS] = {
    {"Zurgut Hulk"},
    {"Ravager"},
    {"Grunt"},
    {"Shotty"},
};

int G_Init()
{
    int i;
    playr_t* p;

    sg_world.state = SG_IN_LEVEL;
    
    sg_world.playr = &playrs[0];
    sg_world.numPlayrs = 0;
    sg_world.playrs = playrs;
    p = &playrs[0];
    memset(playrs, 0, MAX_PLAYR_COUNT * sizeof(*playrs));

    p->alive = qtrue;
    p->dir = D_NORTH;
    p->health = 100;
    memset(p->inventory, 0, MAX_PLAYR_INVENTORY * sizeof(*p->inventory));
    VectorCopy(p->to, vec2_origin);
    VectorCopy(p->thrust, vec2_origin);
    VectorCopy(p->pos, vec2_origin);

    sg_world.numMobs = 0;
    sg_world.mobs = mobs;
    memset(mobs, 0, MAX_MOBS_ACTIVE * sizeof(*mobs));

    G_UpdateConfig(cvars);
    G_GetTilemap(sg_world.tilemap);
    G_InitMem();

    return 0;
}

void G_Shutdown()
{


    return 0;
}