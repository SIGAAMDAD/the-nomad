#include "../src/n_shared.h"
#include "../bff_file/g_bff.h"
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

#define MAX_ACTIVE_VM 2
static nomadVM_t activeVM[MAX_ACTIVE_VM];
static uint64_t vmCount = 0;

void VM_Init(bffscript_t *scripts)
{
    memset(activeVM, 0, sizeof(activeVM));
    vmCount = 0;

    G_AddVM(&scripts[0], "sgame");
    G_AddVM(&scripts[1], "script");
}

void G_AddVM(bffscript_t* script, const char *name)
{
    if (vmCount >= MAX_ACTIVE_VM) {
        N_Error("G_AddVM: vmCount >= MAX_ACTIVE_VM");
    }
    memset(activeVM[vmCount].name, 0, MAX_VM_NAME);
    N_strncpy(activeVM[vmCount].name, name, MAX_VM_NAME - 1);
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

static void* VM_RunMain(void *in_args)
{
    uint64_t index = ((uint64_t *)in_args)[0];
    int command = ((int *)in_args)[3];
    int* args = &((int *)in_args)[4];

    activeVM[index].result = VM_Call(&activeVM[index].vm, command, args[0], args[1], args[2], args[3], args[4], args[5], args[6], args[7], args[8],
        args[9], args[10], args[11]);
    return NULL;
}

void VM_Stop(uint64_t index)
{
    if (!activeVM[index].running) {
        return;
    }
    pthread_join(activeVM[index].thread, (void **)NULL);
    intptr_t result = activeVM[index].result;

    if (result == -1) {
        Con_Error("vmMain for vm {} returned -1", activeVM[index].name);
    }
}

void VM_Run(uint64_t index, int command, const nomadvector<int>& vm_args)
{
    int args[15];
    memset(args, 0, sizeof(args));

    *(uint64_t *)args = index;
    args[3] = command;
    memcpy(args+4, vm_args.data(), vm_args.size() * sizeof(int));

    activeVM[index].running = true;
    pthread_create(&activeVM[index].thread, NULL, VM_RunMain, (void *)args);
}

void Com_free(void *p, vm_t* vm, vmMallocType_t type)
{
    (void)vm;
    (void)type;
    if (p == NULL) {
        Con_Error("null pointer (Com_free)");
        return;
    }
    xfree(p);
}
void* Com_malloc(size_t size, vm_t* vm, vmMallocType_t type)
{
    (void)vm;
    (void)type;
    return malloc_aligned(32, size);
}
void Com_Error(vmErrorCode_t level, const char* error)
{
    (void)level;
    Con_Error("%s", error);
}