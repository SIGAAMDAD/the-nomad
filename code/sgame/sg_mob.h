#ifndef _SG_MOB_
#define _SG_MOB_

#pragma once

typedef enum
{
    M_HULK,
    M_RAVAGER,
    M_GRUNT,
    M_SHOTTY,

    NUMMOBS
} mobtype_t;

typedef struct mobj_s
{
    char name[80];
    int health;

    vec2_t pos;
    vec2_t thrust;
    vec2_t to;

    dirtype_t dir;
    mobtype_t type;
    qboolean alive;
    void (*think)(struct mobj_s *mob);
} mobj_t;

extern const mobj_t mobinfo[NUMMOBS];
extern mobj_t mobs[MAX_MOBS_ACTIVE];

#endif
