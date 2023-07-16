#include "../src/n_shared.h"
#include "vm.h"
#include "vm_syscalls.h"
#include "n_vm.h"

#define VM_COUNT 1
#define VM_SGAME 0
#define VM_SCRIPT 1

static vm_t vmTable[VM_COUNT];

static vm_t *sgvm, *scriptvm;

void VM_Init(void)
{
    uint32_t i;
    const bffscript_t *sgame;

    Z_CheckHeap();
    Con_Printf("VM_Init: initializing virtual bytecode interpreters");

    sgame = BFF_FetchScript("NM_SGAME");
    sgvm = &vmTable[VM_SGAME];

    VM_Create(sgvm, "NM_SGAME", sgame->bytecode, sgame->codelen, SG_VM_SysCalls);
}

static uint64_t active_vm = 0;
uint32_t vm_command;
uint32_t vm_args[12];

static void VM_RunMain(void)
{
    intptr_t result = VM_Call(NULL, 12, vm_command,
        vm_args[0], vm_args[1], vm_args[2], vm_args[3], vm_args[4], vm_args[5],
        vm_args[6], vm_args[7], vm_args[8], vm_args[9], vm_args[10], vm_args[11]);
}


intptr_t VM_Stop(uint64_t index)
{
}

void VM_Run(uint64_t index)
{
    intptr_t result = VM_Call(&vmTable[index], 12, vm_command,
        vm_args[0], vm_args[1], vm_args[2], vm_args[3], vm_args[4], vm_args[5],
        vm_args[6], vm_args[7], vm_args[8], vm_args[9], vm_args[10], vm_args[11]);
}
