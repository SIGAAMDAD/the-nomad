#ifndef _SG_ITEM_
#define _SG_ITEM_

#pragma once

typedef enum
{
    W_ADB, // asturion double-barrel

    NUMWEAPONS
} weapontype_t;

typedef struct
{
    const char name[80];

    weapontype_t id;

    int damage;
    int range;
} weapon_t;

typedef enum
{
    I_SWORD,

    I_HEALTH_SMALL,
    I_HEALTH_MEDIUM,
    I_HEALTH_LARGE,

    NUMITEMS
} itemtype_t;

typedef struct
{
    const char name[80];

    itemtype_t id;

    int cost;
} item_t;

extern const item_t iteminfo[NUMITEMS];

#endif
