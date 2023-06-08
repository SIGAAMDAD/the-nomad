#ifndef _N_VM_
#define _N_VM_

#pragma once

#include "../bff_file/g_bff.h"
#include "vm.h"

#define MAX_ACTIVE_VM 2
#define INVALID_VM (MAX_ACTIVE_VM+1)

void VM_Init(bffscript_t* scripts);
void G_RemoveVM(uint64_t index);
intptr_t VM_Stop(uint64_t index);
uint64_t VM_GetIndex(const char* name);
uint64_t VM_GetIndex(vm_t *vm);

intptr_t G_SystemCalls(vm_t *vm, intptr_t *args);

#endif