#include "sg_local.h"

const weapon_t weaponinfo[NUMWEAPONTYPES] = {

};

const item_t iteminfo[NUMITEMS] = {

};

const mobj_t mobinfo[NUMMOBS] = {

};

const uint32_t ammoCaps[NUMAMMOTYPES] = {
    25, // AM_SHELL
    50, // AM_BULLET
    10, // AM_ROCKET
};

/*
statenum_t id;
statenum_t next;
actionp_t action;
spritenum_t sprite;
int32_t ticcount;
*/
state_t stateinfo[NUMSTATES] = {
    { ST_NULL,          ST_NULL,        {NULL},         SPR_NULL,       0 },
    { ST_IDLE,          ST_NULL,        {NULL},         SPR_NULL,       0 },
    { ST_KNOCKBACK,     ST_NULL,        {NULL},         SPR_NULL,       0 },
    { ST_DEAD,          ST_NULL,        {NULL},         SPR_NULL,       0 },

    { ST_PLAYR_IDLE,    ST_PLAYR_IDLE,  {P_Thinker},    SPR_PLAYR_IDLE, 0 },
};
