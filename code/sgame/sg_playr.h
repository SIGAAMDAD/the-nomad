#ifndef _SG_PLAYR_
#define _SG_PLAYR_

#pragma once

#include "sg_item.h"

typedef struct
{
    char *name;
    int health;

    vec2_t pos;
    vec2_t thrust;
    vec2_t to;
    dirtype_t dir;
    qboolean alive;
    item_t inventory[MAX_PLAYR_INVENTORY];
} playr_t;

extern playr_t playrs[MAX_PLAYR_COUNT];

void P_Teleport();

#endif
