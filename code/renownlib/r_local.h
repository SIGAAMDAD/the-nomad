#ifndef __R_LOCAL__
#define __R_LOCAL__

#pragma once

#include "r_public.h"
#include "../game/g_game.h"

typedef struct
{
    char name[64];
    uint32_t entityType;

    float cruelty;
    float kindness;
    float honor;
    uint32_t renown;
} renown_entity_t;

typedef struct
{
    char name[64];
    int32_t health;
    ivec2_t detectionRange;
} mobinfo_t;

typedef struct
{
    char **botInfos;
    int32_t numBots;
    uint32_t numLevelBots;

    char **mobInfos;
    int32_t numMobs;
    uint32_t numLevelMobs;

    char **itemInfos;
    int32_t numItems;

    char **weaponInfos;
    int32_t numWeapons;

    renown_entity_t *entityData;
    int32_t numEvents;
} renownGlobals_t;

extern renownImport_t ri;
extern renownGlobals_t rg;

extern uint32_t r_numMobConfigs;
extern uint32_t r_numBotConfigs;
extern uint32_t r_numLocationConfigs;
extern uint32_t r_numFactionConfigs;

extern cvar_t *r_renownDebug;
extern cvar_t *r_mobsFile;
extern cvar_t *r_botsFile;
extern cvar_t *r_itemsFile;
extern cvar_t *r_weaponsFile;
extern cvar_t *r_locationsFile;
extern cvar_t *r_factionsFile;
extern cvar_t *r_maxEventBuffer;
extern cvar_t *r_eventShuffle;

void Renown_InitEvents( void );

#endif