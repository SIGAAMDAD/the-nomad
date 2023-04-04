#ifndef _P_PLAYR_
#define _P_PLAYR_

#pragma once

typedef enum armortype_e : uint_fast8_t
{
    ARMOR_STREET,
    ARMOR_MILITART,
    ARMOR_MERC
} armortype_t;

typedef struct playr_s
{
    char name[256];
    int_fast16_t health;
    armortype_t armor;
    
    model_t* model;
    
    coord_t pos;
    uint_fast8_t pdir;

    uint_fast16_t level = 0;
    uint_fast64_t xp = 0;

    weapon_t P_wpns[PLAYR_MAX_WPNS];
    weapon_t *c_wpn = NULL;
    weapon_t *swap = NULL;
    cvector_vector_type(item_t) inv;
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

#endif