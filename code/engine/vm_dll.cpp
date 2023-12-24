#include "n_shared.h"
#include <squirrel.h>
#include "vm_local.h"

static squall::VMStd nutVM;

void VM_InitSquirrel( vm_t *vm )
{
    HSQUIRRELVM hVMInstance;
    SQRESULT res;

    hVMInstance = sq_open( PROGRAM_STACK_SIZE );

    vm->dataAlloc = ~0U;
    vm->dataMask = ~0U;
    vm->dataBase = 0;

    vm->squirrelVMInstance = hVMInstance;
}

void VM_ShutdownSquirrel( vm_t *vm )
{
    sq_close( (HSQUIRRELVM)vm->squirrelVMInstance );
}

void VM_CallSquirrel( vm_t *vm, int32_t command, uint32_t numCommands, ... )
{
    int32_t args[MAX_VMMAIN_ARGS];
    va_list argptr;

    args[0] = command;
    va_start( argptr, numCommands );
    for ( uint32_t i = 0; i < numCommands; i++ ) {
        args[i+1] = va_arg( argptr, int32_t );
    }
    va_end( argptr );
}


