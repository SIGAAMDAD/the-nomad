#include "../src/n_shared.h"
#include "sg_local.h"
#include "sg_playr.h"
#include "sg_mob.h"
#include "sg_game.h"

world_t sg_world;
playr_t playrs[MAX_PLAYR_COUNT];
mobj_t mobs[MAX_MOBS_ACTIVE];

eventState_t events;

int G_Init(void);
int G_Shutdown(void);
int G_RunLoop(void);
int G_StartLevel(void);
int G_EndLevel(void);

int vmMain(int command, int arg0, int arg1, int arg2, int arg3, int arg4, int arg5, int arg6, int arg7,
    int arg8, int arg9, int arg10)
{
    switch (command) {
    case SGAME_INIT:
        return G_Init();
    case SGAME_RUNTIC:
        return G_RunLoop();
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

int G_RunLoop(void)
{
    G_GetEvents(&events);

    return 0;
}

int G_Init(void)
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

    G_GetTilemap(sg_world.tilemap);
    G_InitMem();

    return 0;
}

int G_Shutdown(void)
{


    return 0;
}
