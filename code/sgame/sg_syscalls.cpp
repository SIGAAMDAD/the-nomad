#include "code/engine/n_shared.h"
#include "code/engine/n_scf.h"
#include "code/common/n_vm.h"
#include "sg_public.h"
#include "code/common/vm_syscalls.h"

SYSCALL(VM_Key_IsDown)
{
    return (qvm_int_t)Key_IsDown(*(uint32_t *)VMA(1));
}

static const vmSystemCall_t syscalls[] = {
    NOMADLIB_FUNCS,
    CALL_ENTRY(VM_G_GETKEYBOARDSTATE, VM_Key_IsDown),
};

intptr_t SG_Syscalls(vm_t *vm, intptr_t *args)
{
    const intptr_t id = args[0] - 1;
    for (const auto& i : syscalls) {
        if (i.id == id)
            return i.call(vm, args);
    }
    VM_Error(vm, "SG_Syscalls: invalid system function call %li", id);
    return -1;
}