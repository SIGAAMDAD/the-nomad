#ifndef _N_VM_
#define _N_VM_

#pragma once

#ifndef Q3_VM

#include "../src/g_bff.h"
#include "vm.h"

#define MAX_ACTIVE_VM 2
#define VM_COUNT 2
#define SGAME_VM 0
#define SCRIPT_VM 1
#define INVALID_VM (MAX_ACTIVE_VM+1)

extern uint32_t vm_command;
extern uint32_t vm_args[12];

void VM_Init(void);
void G_RemoveVM(uint64_t index);
intptr_t VM_Stop(uint64_t index);
void VM_Run(uint64_t index);

#endif

#endif
