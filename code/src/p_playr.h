#ifndef _P_PLAYR_
#define _P_PLAYR_

#pragma once


inline std::vector<std::array<uint64_t, 4>> xp_to_stats = {
    {0, PLAYR_MAX_HEALTH, PLAYR_MAX_WPNS, PLAYR_MAX_ITEMS},      // level 0
};

typedef struct playr_s
{
    entity_t* p;
    uint64_t level = 0;
    uint64_t xp = 0;

    uint64_t *max_health = &xp_to_stats[0][1];
    uint64_t *max_weapons = &xp_to_stats[0][2];
    uint64_t *max_items = &xp_to_stats[0][3];
    model_t* model;
    weapon_t P_wpns[PLAYR_MAX_WPNS];
    item_t inv[PLAYR_MAX_ITEMS];
    weapon_t *c_wpn = &P_wpns[0];
    weapon_t *swap = c_wpn;
} playr_t;

void P_MoveN();
void P_MoveS();
void P_MoveW();
void P_MoveE();
void P_DashN();
void P_DashW();
void P_DashS();
void P_DashE();
void P_SlideN();
void P_SlideW();
void P_SlideS();
void P_SlideE();
void P_UseWeapon();
void P_SwapWeapon1();
void P_SwapWeapon2();
void P_SwapWeapon3();
void P_SwapWeapon4();
void P_SwapWeapon5();
void P_SwapWeapon6();
void P_SwapWeapon7();
void P_SwapWeapon8();
void P_SwapWeapon9();
void P_SwapWeapon10();
void P_NextWeapon();
void P_PrevWeapon();
void P_QuickSwap();
void P_ChangeDirL();
void P_ChangeDirR();

#endif