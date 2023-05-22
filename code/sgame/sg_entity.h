
#ifndef _G_ENTITY_
#define _G_ENITTY_

#pragma once

typedef enum
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
} entityflag_t;

typedef enum
{
    ET_PLAYR = 0,
    ET_MOB,
    ET_BOT,
    ET_MERC,
    ET_AIR,
    ET_ITEM,
    ET_WEAPON,

    ET_NULL
} entitytype_t;

typedef enum armortype_e
{
    ARMOR_STREET,
    ARMOR_MILITART,
    ARMOR_MERC
} armortype_t;

typedef enum
{
    NUMSTATES
} statenum_t;

typedef void(*actionp_t)();
typedef struct state_s
{
    statenum_t id;
    int64_t numticks;
    statenum_t next;
    actionp_t func;
} state_t;

typedef struct entitypos_s
{
    vec2_t hitbox[4];
    vec3_t thrust;
    vec3_t to;
    vec2_t tilemap_coords;
    vec2_t lookangle;
    sprite_t sprite;
} entitypos_t;

typedef struct entity_s
{
    // defined in the array
    const char name[80]={0};
    int health;
    armortype_t armor;
    entitytype_t type;
    entityflag_t flags;
    statenum_t spawnstate;
    statenum_t deadstate;

    // determined at runtime
    entitypos_t pos;
    state_t state;
    int64_t ticker;
    dirtype_t dir;
    struct entity_s* target;
} entity_t;

#endif
