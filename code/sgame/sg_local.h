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

enum
{
    // ignore the EF_SOLID flag if entity has this one, cheat code
    EF_NOCLIP = 1,
    // merc/mob specific, deaf flag, cheat code
    EF_DEAF = 2,
    // merc/mob specific, blind flag, cheat code
    EF_BLIND = 4,
    // if it can and WILL be blocked, this applies to all attacks, yes ALL ATTACKS
    EF_PROJECTILE = 8,
    // self explanatory
    EF_DEAD = 16,
    // static and/or destructable environment piece
    EF_SOLID = 32,
    // mob/merc specific, entity cannot move
    EF_SENTRY = 64,
    // iframes.
    EF_INVUL = 128,
    // can be false for anything that we don't want dead just yet
    EF_KILLABLE = 256,
    // mob/merc specific, buffed stats
    EF_LEADER = 528,
};

typedef enum
{

} statenum_t;

typedef struct sgentity_s sgentity_t;
typedef void (*actionp_t)(sgentity_t *);
typedef struct
{
    statenum_t id;
    statenum_t next, prev;
    int duration; // number of 'frames' it takes for the state to complete
    int sprite; // used for bff texture lookups
    actionp_t think; // if null, just wait until ticcount == 0
} state_t;

struct sgentity_s
{
    void *e; // entity-specific data

    vec2_t pos;
    vec2_t thrust;
    dirtype_t facing;
    state_t state;

    int flags; // general flags
    int ticker; // the count of how many frames into a state the entity is

    struct sgentity_s* target; // only really applies to mobs and homing attacks
};

#endif
