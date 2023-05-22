#include "../src/n_shared.h"
#include "n_vm.h"
#include "vm.h"

#define MAX_VM_NAME 10

typedef struct nomadVM_s
{
    char name[MAX_VM_NAME];
    vm_t vm;
    bool running;
    uint8_t* bytecode;
} nomadVM_t;

#define MAX_ACTIVE_VM 2
static nomadVM_t activeVM[MAX_ACTIVE_VM];
static uint64_t vmCount = 0;

void VM_Init(void)
{
    memset(activeVM, 0, sizeof(activeVM));
    vmCount = 0;

    G_AddVM(, "sgame");
    G_AddVM(, "script");
}

void G_AddVM(bffscript_t* script, const char *name)
{
    if (vmCount >= MAX_ACTIVE_VM) {
        N_Error("G_AddVM: vmCount >= MAX_ACTIVE_VM");
    }
    memset(activeVM[vmCount].name, 0, MAX_VM_NAME);
    strncpy(activeVM[vmCount].name, name, MAX_VM_NAME - 1);
    vm_t* vm = &activeVM[vmCount].vm;
    vmCount++;
    VM_Create(vm, NULL, bytecode, codelen, VM_SystemCalls);
}

void G_RemoveVM(uint64_t index)
{
    if (index >= MAX_ACTIVE_VM) {
        N_Error("G_RemoveVM: index >= MAX_ACTIVE_VM");
    }
    nomadVM_t* vm = &activeVM[index];
    Z_ChangeTag(vm->bytecode, TAG_CACHE);
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
        // free the cache of data
        VM_Free(&activeVM[i].vm);

        // re-initialize the vm memory
        if (VM_Create(&activeVM[i].vm, activeVM[i].name, activeVM[i].bytecode, G_SystemCalls) == -1) {
            N_Error("G_RestartVM: failed to restart vm %s", activeVM[i].name):
        }
    }
}

void Com_free(void *p, vm_t* vm, vmMallocType_t type)
{
    (void)vm;
    (void)type;
    if (p == NULL) {
        Com_Error(VM_INVALID_POINTER, "null pointer (Com_free)");
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
    LOG_ERROR("{}", error);
}