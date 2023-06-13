#include "n_shared.h"
#include "../common/vm.h"
#include "g_game.h"
#include "g_public.h"
#include "n_scf.h"
#include "../sgame/sg_public.h"

#define NUM_SYSTEM_CALLS NUM_GAME_IMPORTS
typedef struct 
{
    const int32_t id;
    intptr_t(*sysFunc)(vm_t *, intptr_t *);
} vmSystemCall_t;

static intptr_t Sys_Con_Printf(vm_t *vm, intptr_t *args);
static intptr_t Sys_Con_Error(vm_t *vm, intptr_t *args);
static intptr_t G_GetTilemap(vm_t *vm, intptr_t *args);
static intptr_t G_KeyIsPressed(vm_t *vm, intptr_t *args);
static intptr_t G_ModIsPressed(vm_t *vm, intptr_t *args);

const vmSystemCall_t systemCalls[] = {
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

static intptr_t Sys_Con_Printf(vm_t *vm, intptr_t *args)
{
    Con_Printf("%s", (const char *)VMA(1, vm));
    return 0;
}

static intptr_t Sys_Con_Error(vm_t *vm, intptr_t *args)
{
    Con_Error("%s", (const char *)VMA(1, vm));
    return 0;
}

static intptr_t G_GetTilemap(vm_t *vm, intptr_t *args)
{
    memcpy((sprite_t **)VMA(1, vm), Game::Get()->c_map, sizeof(Game::Get()->c_map));
    return 0;
}

static intptr_t G_GetKeyboardState(vm_t *vm, intptr_t *args)
{
    memcpy((qboolean *)VMA(1, vm), evState.kbstate, sizeof(qboolean) * NUMKEYS);
}