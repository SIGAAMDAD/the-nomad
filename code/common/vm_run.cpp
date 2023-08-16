#include "../engine/n_shared.h"
#include "vm.h"
#include "n_vm.h"

static vm_t vmTable[VM_COUNT];
static vm_t *sgvm, *scriptvm;

intptr_t SG_Syscalls(vm_t *vm, intptr_t *args);

void VM_Init(void)
{
    uint32_t i;
    uint64_t bytecodeLen;
    uint8_t *bytecode;

    Z_CheckHeap();
    Con_Printf("VM_Init: initializing virtual bytecode interpreters");

    bytecodeLen = FS_LoadFile("sgame.qvm", (void **)&bytecode);
    sgvm = &vmTable[SGAME_VM];

    VM_CreateInterpreted(sgvm, "sgame", bytecode, bytecodeLen, SG_Syscalls);
    FS_FreeFile(bytecode);
}

static uint64_t active_vm = 0;
uint32_t vm_command;
uint32_t vm_args[12];


intptr_t VM_Stop(uint64_t index)
{
    return 0;
}

void VM_Run(uint64_t index)
{
    intptr_t result = VM_CallInterpreted(&vmTable[index], 12, vm_command,
        vm_args[0], vm_args[1], vm_args[2], vm_args[3], vm_args[4], vm_args[5],
        vm_args[6], vm_args[7], vm_args[8], vm_args[9], vm_args[10], vm_args[11]);
}
