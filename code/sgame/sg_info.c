#include "sg_local.h"

/*
const weapon_t weaponinfo[NUMWEAPONTYPES] = {

};

const item_t iteminfo[NUMITEMS] = {

};
*/

const mobj_t mobinfo[NUMMOBS] = {
    // ent, target, type, sight range, melee range, missile range
    { NULL, NULL, MT_GRUNT, 26, 10, 0 },
};

const int ammoCaps[NUMAMMOTYPES] = {
    25, // AM_SHELL
    50, // AM_BULLET
    10, // AM_ROCKET
};

// these functions should never get called directly, only
// by the state manager/game looper
extern void P_IdleThink( sgentity_t *self );
extern void P_Thinker( sgentity_t *self );
extern void P_DeadThink( sgentity_t *self );

state_t stateinfo[NUMSTATES] = {
    { 0,                            0, 0,   {NULL},         S_NULL },
    { SPR_PLAYR_IDLE_R,             0, 35,  {P_Thinker},    S_PLAYR_IDLE },
    { SPR_PLAYR_MOVE0_R,            4, 35,  {P_Thinker},    S_PLAYR_MOVE },
    { SPR_PLAYR_DASH_R,             0, 35,  {P_Thinker},    S_PLAYR_DASH },
    { SPR_PLAYR_PARRY0_R,           2, 16,  {P_Thinker},    S_PLAYR_MELEE },
    { SPR_PLAYR_PARRY2_HIT0_R,      5, 5,   {P_Thinker},    S_PLAYR_PARRY },
    { SPR_MURSTAR_IDLE_R,           0, 0,   {P_Thinker},    S_MURSTAR_IDLE },
    { SPR_MURSTAR_FIRE_R,           0, 35,  {P_Thinker},    S_MURSTAR_FIRE },
    { SPR_MURSTAR_RLD0_R,           2, 35,  {P_Thinker},    S_MURSTAR_RLD },
};
