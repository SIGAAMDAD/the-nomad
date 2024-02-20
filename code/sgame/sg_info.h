#ifndef __SG_INFO__
#define __SG_INFO__

#pragma once

typedef struct {
    int damage[NUMDIFS];
    const char *name;
    int range;
    int ammo;
    int damagetype; // used for causeOfDeath_t
    float accuracy;

    nhandle_t hIconShader;

    sfxHandle_t equipSfx;
    sfxHandle_t altfireSfx;
    sfxHandle_t meleeSfx;
    sfxHandle_t reloadSfx;
} weaponinfo_t;

typedef struct {
    const char *name;
    int cost;
    int stack;
    int moduleIndex;

    nhandle_t hIconShader;

    sfxHandle_t useSfx;
} iteminfo_t;

typedef struct {
    const char *name;

    int moduleIndex;
    int health;
    int movespeed;
    uint32_t flags;

    qboolean hasMelee;
    qboolean hasMissile;

    int soundTolerance;
    int detectionRangeX;
    int detectionRangeY;

    int meleeDamage[NUMDIFS];
    int meleeRange[NUMDIFS];
} mobinfo_t;

typedef struct {
    const char *name;
    int *statToChange;
    int statChangeDelta;
} powerupinfo_t;

typedef struct {
    const char *strings[6];
    int moduleIndex;
    int index;
} damageType_t;

typedef struct {
    int ticks;
} animFrame_t;

typedef struct {
    int numFrames;
} animation_t;

typedef enum
{
    S_NULL,

    S_PLAYR_IDLE,
    S_PLAYR_MOVE,
    S_PLAYR_DASH,
    S_PLAYR_MELEE,
    S_PLAYR_PARRY,

    S_FIREARM_IDLE,
    S_FIREARM_USE,
    S_FIREARM_RELOAD,

    S_MOB_IDLE,
    S_MOB_CHASE,
    S_MOB_FIGHT,
    S_MOB_FLEE,
    S_MOB_,

    NUMSTATES
} statenum_t;

typedef struct {
    spritenum_t sprite;
    uint32_t frames;
    uint32_t tics;
    actionp_t action;
    statenum_t nextstate;
} state_t;

extern state_t stateinfo[NUMSTATES];

#endif
