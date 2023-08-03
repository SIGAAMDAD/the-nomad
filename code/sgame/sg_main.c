#include "../src/n_shared.h"
#include "../src/n_scf.h"
#include "sg_local.h"

world_t sg_world;
playr_t playrs[MAX_PLAYR_COUNT];
mobj_t mobs[MAX_MOBS_ACTIVE];

static qboolean *kbstate[NUMBINDS];

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
        Com_Error("vmMain: invalid command id: %i", command);
    };
    return -1;
}

void GDR_DECL SG_Printf(const char *fmt, ...)
{
    va_list argptr;
    char msg[1024];

    va_start(argptr, fmt);
    vsprintf(msg, fmt, argptr);
    va_end(argptr);

    VM_Com_Printf(msg);
}

void GDR_DECL SG_Error(const char *fmt, ...)
{
    va_list argptr;
    char msg[1024];

    va_start(argptr, fmt);
    vsprintf(msg, fmt, argptr);
    va_end(argptr);

    VM_Com_Error(msg);
}

const mobj_t mobinfo[NUMMOBS] = {
    {"Zurgut Hulk"},
    {"Ravager"},
    {"Grunt"},
    {"Shotty"},
};

int G_RunLoop(void)
{
    G_GetKeyboardState(kbstate, NUMBINDS);
    
    return 0;
}

int G_Init(void)
{    
    int i;
    playr_t* p;

    G_Printf("SG_Init: initializing solo-player campaign variables");

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

    sg_world.mapwidth = 0;
    sg_world.mapheight = 0;
    sg_world.spritemap = (sprite_t **)G_AllocMem(sizeof(sprite_t *) * sg_world.mapwidth);

    G_InitMem();

    return 0;
}

int G_Shutdown(void)
{


    return 0;
}
