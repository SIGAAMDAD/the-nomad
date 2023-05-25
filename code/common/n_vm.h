#ifndef _N_VM_
#define _N_VM_

#pragma once

#include "../bff_file/g_bff.h"
#include "vm.h"

void VM_Init(bffscript_t* scripts);
void G_AddVM(bffscript_t* script, const char* name);
void G_RemoveVM(uint64_t index);
void VM_Shutdown(void);

intptr_t G_SystemCalls(vm_t *vm, intptr_t *args);

#endif