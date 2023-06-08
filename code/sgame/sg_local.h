#ifndef _SG_LOCAL_
#define _SG_LOCAL_

#pragma once

#include "sg_public.h"

// everything is globally or statically allocated within the vm, unless its using the G_AllocMem stuff, but the vm doesn't like it
// (reasonably, raw pointers + vm bytecode = exploits) when you pass pointers back and forth from the vm and native bytecode, so non of that'll happen
#define MAX_PLAYR_COUNT 10
#define MAX_MOBS_ACTIVE 150
#define MAX_PLAYR_INVENTORY 20

void* G_AllocMem(int size);
void G_FreeMem(void *ptr);
void G_InitMem(void);

#endif
