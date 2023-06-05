#ifndef _SG_GAME_
#define _SG_GAME_

#pragma once

typedef enum
{
    SG_INACTIVE,
    SG_IN_MENU,
    SG_IN_LEVEL
} sg_gamestate_t;

typedef struct
{
    sprite_t tilemap[MAP_MAX_Y][MAP_MAX_X];

    playr_t* playr;
    sg_gamestate_t state;

    int numPlayrs;
    int numMobs;
    playr_t* playrs;
    mobj_t* mobs;
} world_t;

extern world_t sg_world;

void G_SpawnMob(mobtype_t type, const vec2_t pos);

#endif
