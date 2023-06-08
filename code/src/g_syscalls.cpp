#include "n_shared.h"
#include "../common/vm.h"
#include "g_game.h"
#include "g_public.h"
#include "n_scf.h"
#include "../sgame/sg_public.h"
#include "../sgame/sg_local.h"

typedef enum
{
    W_ADB, // asturion double-barrel

    NUMWEAPONS
} weapontype_t;

typedef struct
{
    const char name[80];

    weapontype_t id;

    int32_t damage;
    int32_t range;
} weapon_t;

typedef enum
{
    I_SWORD,

    I_HEALTH_SMALL,
    I_HEALTH_MEDIUM,
    I_HEALTH_LARGE,

    NUMITEMS
} itemtype_t;

typedef struct
{
    const char name[80];

    itemtype_t id;

    int32_t cost;
} item_t;

typedef enum
{
    M_HULK,
    M_RAVAGER,
    M_GRUNT,
    M_SHOTTY,

    NUMMOBS
} mobtype_t;

struct mobj_s;
typedef struct mobj_t;

typedef void(*thinkerfunc_t)(mobj_t *);
struct mobj_s
{
    char name[80];
    int32_t health;

    vec2_t pos;
    vec2_t thrust;
    vec2_t to;

    dirtype_t dir;
    mobtype_t type;
    qboolean alive;
    thinkerfunc_t think;
};

typedef struct
{
    char *name;
    int32_t health;

    vec2_t pos;
    vec2_t thrust;
    vec2_t to;
    dirtype_t dir;
    qboolean alive;
    item_t inventory[MAX_PLAYR_INVENTORY];
} playr_t;

typedef enum
{
    SG_INACTIVE,
    SG_IN_MENU,
    SG_IN_LEVEL
} sg_gamestate_t;

typedef struct
{
    sprite_t tilemap[MAP_MAX_Y][MAP_MAX_X];

    playr_t* playr;
    sg_gamestate_t state;

    int32_t numPlayrs;
    int32_t numMobs;
    playr_t* playrs;
    mobj_t* mobs;
} world_t;

#define NUM_SYSTEM_CALLS NUM_GAME_IMPORTS

typedef struct 
{
    const int32_t id;
    intptr_t(*sysFunc)(vm_t *, intptr_t *);
} vmSystemCall_t;

static intptr_t Sys_Con_Printf(vm_t *vm, intptr_t *args)
{
    Com_Printf("%s", (const char*)VMA(1, vm));
    return args[1];
}
static intptr_t Sys_Con_Error(vm_t *vm, intptr_t *args)
{
    Com_Error("%s", (const char*)VMA(1, vm));
    return args[1];
}
static intptr_t Sys_G_GetTilemap(vm_t *vm, intptr_t *args)
{
    memcpy((sprite_t **)VMA(1, vm), Game::Get()->c_map, MAP_MAX_Y * MAP_MAX_X * sizeof(sprite_t));
    return 0;
}
static intptr_t Sys_G_SaveGame(vm_t *vm, intptr_t *args)
{
    world_t* w = (world_t *)VMA(1, vm);
//    G_SaveGame(*(uint32_t *)VMA(1, vm));
    return 0;
}
static intptr_t Sys_G_LoadGame(vm_t *vm, intptr_t *args)
{
    world_t* w = (world_t *)VMA(1, vm);
//    G_LoadGame(*(uint32_t *)VMA(1, vm));
    return 0;
}
static intptr_t Sys_G_GetEvents(vm_t *vm, intptr_t *args)
{
    memcpy(VMA(1, vm), &evState, sizeof(eventState_t));
    return 0;
}

const vmSystemCall_t systemCalls[] = {
    {(const int)-SYS_CON_PRINTF,  Sys_Con_Printf},
    {(const int)-SYS_CON_ERROR,   Sys_Con_Error},
    {(const int)-G_GETTILEMAP,    Sys_G_GetTilemap},
    {(const int)-G_SAVEGAME,      Sys_G_SaveGame},
    {(const int)-G_LOADGAME,      Sys_G_LoadGame},
    {(const int)-G_GETEVENTS,     Sys_G_GetEvents}
};

intptr_t G_SystemCalls(vm_t *vm, intptr_t *args)
{
    const int id = -1 - args[0];
    int i;
    for (i = 0; i < arraylen(systemCalls); i++) {
        if (id == systemCalls[i].id) {
            return systemCalls[i].sysFunc(vm, args);
        }
    }
    Con_Error("invalid VM system call id given to G_SystemCalls: %i", id);
    
    return -1;
}