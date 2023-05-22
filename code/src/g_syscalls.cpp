#include "n_shared.h"
#include "../common/vm.h"
#include "g_game.h"
#include "g_public.h"
#include "n_scf.h"
#include "../sgame/sg_public.h"

#define NUM_SYSTEM_CALLS NUM_GAME_IMPORTS

typedef struct 
{
    const int id;
    intptr_t(*sysFunc)(vm_t *, intptr_t *);
} vmSystemCall_t;

static intptr_t Sys_Con_Printf(vm_t *vm, intptr_t *args)
{
    Con_Printf("%s", (const char*)VMA(1, vm));
    return args[1];
}
static intptr_t Sys_Con_Error(vm_t *vm, intptr_t *args)
{
    Con_Error("%s", (const char*)VMA(1, vm));
    return args[1];
}
static intptr_t Sys_Con_Flush(vm_t *vm, intptr_t *args)
{
    Con_Flush();
    return 0;
}
static intptr_t Sys_G_GetTilemap(vm_t *vm, intptr_t *args)
{
    memcpy((sprite_t **)VMA(1, vm), Game::Get()->c_map, MAP_MAX_Y * MAP_MAX_X * sizeof(sprite_t));
    return 0;
}
static intptr_t Sys_G_UpdateConfig(vm_t *vm, intptr_t *args)
{
    memcpy((vmCvar_t *)VMA(1, vm), G_GetCvars(), G_NumCvars() * sizeof(vmCvar_t));
    return 0;
}
static intptr_t Sys_N_SaveGame(vm_t *vm, intptr_t *args)
{
//    G_SaveGame(*(uint32_t *)VMA(1, vm));
    return 0;
}
static intptr_t Sys_N_LoadGame(vm_t *vm, intptr_t *args)
{
//    G_LoadGame(*(uint32_t *)VMA(1, vm));
    return 0;
}

const vmSystemCall_t systemCalls[] = {
    {(const int)-SYS_CON_PRINTF,  Sys_Con_Printf},
    {(const int)-SYS_CON_ERROR,   Sys_Con_Error},
    {(const int)-CON_FLUSH,       Sys_Con_Flush},
    {(const int)-G_GETTILEMAP,    Sys_G_GetTilemap},
    {(const int)-G_UPDATECONFIG,  Sys_G_UpdateConfig},
    {(const int)-N_SAVEGAME,      Sys_N_SaveGame},
    {(const int)-N_LOADGAME,      Sys_N_LoadGame},
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
    LOG_ERROR("invalid VM system call id given to G_SystemCalls: {}", id);
    
    return -1;
}