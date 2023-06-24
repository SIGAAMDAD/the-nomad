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

static intptr_t Sys_Com_Printf(vm_t *vm, intptr_t *args);
static intptr_t Sys_Com_Error(vm_t *vm, intptr_t *args);
static intptr_t G_GetTilemap(vm_t *vm, intptr_t *args);
static intptr_t G_GetKeyboardState(vm_t *vm, intptr_t *args);
static intptr_t Sys_memcpy(vm_t *vm, intptr_t *args);
static intptr_t Sys_memmove(vm_t *vm, intptr_t *args);
static intptr_t Sys_memset(vm_t *vm, intptr_t *args);

const vmSystemCall_t systemCalls[] = {
    {SYS_COM_PRINTF,     Sys_Com_Printf},
    {SYS_COM_ERROR,      Sys_Com_Error},
    {G_GETTILEMAP,       G_GetTilemap},
    {G_GETKEYBOARDSTATE, G_GetKeyboardState},
    {SYS_MEMCPY,         Sys_memcpy},
    {SYS_MEMMOVE,        Sys_memmove},
    {SYS_MEMSET,         Sys_memset},
};

intptr_t G_SystemCalls(vm_t *vm, intptr_t *args)
{
    const int32_t id = -1 - args[0];
    for (uint32_t i = 0; i < arraylen(systemCalls); i++) {
        if (id == systemCalls[i].id) {
            return systemCalls[i].sysFunc(vm, args);
        }
    }
    Com_Error("invalid VM system call id given to G_SystemCalls: %i", id);
    
    return -1;
}

#ifdef _NOMAD_DEBUG
#define SEGFAULT(vm,addr,len) \
    if (!VM_MemoryRangeValid(addr,len,vm)) Com_Error(__func__": Segmentation Fault (out-of-range memory access violation)")
#else
#define SEGFAULT(vm,addr,len)
#endif

static intptr_t Sys_Com_Printf(vm_t *vm, intptr_t *args)
{
    Com_Printf("%s", (const char *)VMA(1, vm));
    return 0;
}

static intptr_t Sys_Com_Error(vm_t *vm, intptr_t *args)
{
    Com_Error("%s", (const char *)VMA(1, vm));
    return 0;
}

static intptr_t G_GetTilemap(vm_t *vm, intptr_t *args)
{
    SEGFAULT(vm, args[1], sizeof(Game::Get()->c_map));

    memcpy((sprite_t **)VMA(1, vm), Game::Get()->c_map, sizeof(Game::Get()->c_map));
    return 0;
}

static intptr_t G_GetKeyboardState(vm_t *vm, intptr_t *args)
{
    SEGFAULT(vm, args[1], sizeof(qboolean) * NUMKEYS);

    memcpy((qboolean *)VMA(1, vm), evState.kbstate, sizeof(qboolean) * NUMKEYS);
    return 0;
}

static intptr_t Sys_memcpy(vm_t *vm, intptr_t *args)
{
    SEGFAULT(vm, args[1], args[3]);
    SEGFAULT(vm, args[2], args[3]);

    memcpy(VMA(1, vm), VMA(2, vm), args[3]);
    return args[1];
}

static intptr_t Sys_memmove(vm_t *vm, intptr_t *args)
{
    SEGFAULT(vm, args[1], args[3]);
    SEGFAULT(vm, args[2], args[3]);

    memmove(VMA(1, vm), VMA(2, vm), args[3]);
    return args[1];
}

static intptr_t Sys_memset(vm_t *vm, intptr_t *args)
{
    SEGFAULT(vm, args[1], args[3]);

    memset(VMA(1, vm), args[2], args[3]);
    return args[1];
}