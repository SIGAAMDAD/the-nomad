#include "../engine/n_shared.h"
#include "../engine/n_scf.h"
#include "sg_local.h"

world_t sg_world;
playr_t playrs[MAX_PLAYR_COUNT];
mobj_t mobs[MAX_MOBS_ACTIVE];

int SG_Init(void);
int SG_Shutdown(void);
int SG_RunLoop(void);

int vmMain(int command, int arg0, int arg1, int arg2, int arg3, int arg4, int arg5, int arg6, int arg7,
    int arg8, int arg9, int arg10)
{
    switch (command) {
    case SGAME_INIT:
        return SG_Init();
    default:
        SG_Error("vmMain: invalid command id: %i", command);
        break;
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

    trap_Print(msg);
}

void GDR_DECL SG_Error(const char *fmt, ...)
{
    va_list argptr;
    char msg[1024];

    va_start(argptr, fmt);
    vsprintf(msg, fmt, argptr);
    va_end(argptr);

    trap_Error(msg);
}

const mobj_t mobinfo[NUMMOBS] = {
    {"Zurgut Hulk"},
    {"Ravager"},
    {"Grunt"},
    {"Shotty"},
};

int SG_Init(void)
{
    playr_t* p;

    SG_Printf("SG_Init: initializing solo-player campaign variables");

    sg_world.state = GS_LEVEL;
    
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

    SG_InitMem();

    sg_world.mapwidth = 0;
    sg_world.mapheight = 0;
    sg_world.spritemap = (sprite_t **)SG_AllocMem(sizeof(sprite_t *) * sg_world.mapwidth);

    if (!trap_Key_GetCatcher() & KEYCATCH_SGAME) {
        trap_Key_SetCatcher(trap_Key_GetCatcher() & KEYCATCH_SGAME);
    }

    return 0;
}

int SG_Shutdown(void)
{
    SG_ClearMem();

    if (trap_Key_GetCatcher() & KEYCATCH_SGAME) {
        trap_Key_SetCatcher(trap_Key_GetCatcher() & ~KEYCATCH_SGAME);
    }

    return 0;
}
