#ifndef _SG_GAME_
#define _SG_GAME_

#pragma once

typedef unsigned int sprite_t;

typedef enum
{
    SG_INACTIVE,
    SG_IN_MENU,
    SG_IN_LEVEL
} gamestate_t;

typedef struct
{
    int mapwidth;
    int mapheight;
    sprite_t **spritemap;

    playr_t *playr;
    gamestate_t state;

    int numPlayrs;
    int numMobs;
    playr_t *playrs;
    mobj_t *mobs;
} world_t;

extern world_t sg_world;

void G_SpawnMob(mobtype_t type, const vec2_t pos);

#endif
