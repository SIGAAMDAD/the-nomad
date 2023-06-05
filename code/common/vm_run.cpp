#include "../src/n_shared.h"
#include "../src/g_bff.h"
#include "vm.h"
#include "n_vm.h"

#define MAX_VM_NAME 10

typedef struct nomadVM_s
{
    char name[MAX_VM_NAME];
    vm_t vm;
    bool running;
    uint8_t* bytecode;
    uint32_t codelen;
    pthread_t thread;

    intptr_t result;
} nomadVM_t;

static nomadVM_t activeVM[MAX_ACTIVE_VM];
static uint64_t vmCount = 0;

void VM_Init(bffscript_t *scripts)
{
    memset(activeVM, 0, sizeof(activeVM));
    vmCount = 0;

    G_AddVM(&scripts[0], "sgame");
    G_AddVM(&scripts[1], "script");
}

uint64_t VM_GetIndex(const char* name)
{
    for (uint64_t i = 0; i < arraylen(activeVM); i++) {
        if (N_strncmp(activeVM[i].name, name, MAX_VM_NAME)) {
            return i;
        }
    }
    return INVALID_VM;
}
uint64_t VM_GetIndex(vm_t *vm)
{
    for (uint64_t i = 0; i < arraylen(activeVM); i++) {
        if (vm == &activeVM[i].vm) {
            return i;
        }
    }
    return INVALID_VM;
}

void G_AddVM(bffscript_t* script, const char *name)
{
    if (vmCount >= MAX_ACTIVE_VM) {
        N_Error("G_AddVM: vmCount >= MAX_ACTIVE_VM");
    }
    memset(activeVM[vmCount].name, 0, MAX_VM_NAME);
    N_strncpy(activeVM[vmCount].name, name, MAX_VM_NAME);
    activeVM[vmCount].bytecode = script->bytecode;
    activeVM[vmCount].codelen = script->codelen;
    vm_t* vm = &activeVM[vmCount].vm;
    vmCount++;
    VM_Create(vm, name, script->bytecode, script->codelen, G_SystemCalls);
}

void G_RemoveVM(uint64_t index)
{
    if (index >= MAX_ACTIVE_VM) {
        N_Error("G_RemoveVM: index >= MAX_ACTIVE_VM");
    }
    nomadVM_t* vm = &activeVM[index];
    Z_ChangeTag(vm->bytecode, TAG_PURGELEVEL);
    VM_Free(&vm->vm);
    vm->running = false;
}

void G_RestartVM(void)
{
    for (uint32_t i = 0; i < arraylen(activeVM); i++) {
        if (!activeVM[i].bytecode) {
            N_Error("G_RestartVM: vm data for vm %s freed prematurely", activeVM[i].name);
        }
        if (activeVM[i].running) {
            N_Error("G_RestartVM: called on running vm");
        }

        // restart the memory
        //TODO: THIS SHIT
    }
}

static uint64_t active_vm = 0;
uint32_t vm_command;
uint32_t vm_args[12];

static void* VM_RunMain(void *unused)
{
    activeVM[active_vm].result = VM_Call(&activeVM[active_vm].vm, vm_command,
        vm_args[0], vm_args[1], vm_args[2], vm_args[3], vm_args[4], vm_args[5],
        vm_args[6], vm_args[7], vm_args[8], vm_args[9], vm_args[10], vm_args[11]);
    return NULL;
}


intptr_t VM_Stop(uint64_t index)
{
    if (!activeVM[index].running) {
        return -1;
    }
    pthread_join(activeVM[index].thread, (void **)NULL);
    intptr_t result = activeVM[index].result;
    return result;
}

void VM_Run(uint64_t index)
{
    active_vm = index;
    activeVM[index].running = true;
    pthread_create(&activeVM[index].thread, NULL, VM_RunMain, NULL);
}

void Com_free(void *p, vm_t* vm, vmMallocType_t type)
{
    (void)vm;
    (void)type;
    if (p == NULL) {
        Con_Error("null pointer (Com_free)");
        return;
    }
}
void* Com_malloc(size_t size, vm_t* vm, vmMallocType_t type)
{
    (void)vm;
    (void)type;
    return Z_Malloc(size, TAG_LEVEL, NULL, "vmMem");
}
void Com_Error(vmErrorCode_t level, const char* error)
{
    (void)level;
    Con_Error("%s", error);
}