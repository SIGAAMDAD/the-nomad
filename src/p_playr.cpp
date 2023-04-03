#include "n_shared.h"
#include "g_game.h"

static playr_t* playr;

void P_MoveN()
{
    --playr->pos.y;
}

void P_MoveW()
{
    --playr->pos.x;
}

void P_MoveS()
{
    ++playr->pos.y;
}

void P_MoveE()
{
    ++playr->pos.x;
}

void P_NextWeapon()
{
    playr->swap = playr->c_wpn;
    if (playr->c_wpn == &playr->P_wpns[10])
        playr->c_wpn = playr->P_wpns;
    else
        ++playr->c_wpn;
}
void P_PrevWeapon()
{
    playr->swap = playr->c_wpn;
    if (playr->c_wpn == playr->P_wpns)
        playr->c_wpn = &playr->P_wpns[0];
    else
        --playr->c_wpn;
}
void P_QuickSwap()
{
    weapon_t* const tmp = playr->c_wpn;
    playr->c_wpn = playr->swap;
    playr->swap = tmp;
}

void P_DashN()
{
    
}
void P_DashW()
{
    
}
void P_DashS()
{
    
}
void P_DashE()
{
    
}
void P_SlideN()
{
    
}
void P_SlideW()
{
    
}
void P_SlideS()
{
    
}
void P_SlideE()
{
    
}
void P_UseWeapon()
{
    
}
void P_SwapWeapon1()
{
    
}
void P_SwapWeapon2()
{
    
}
void P_SwapWeapon3()
{
    
}
void P_SwapWeapon4()
{
    
}
void P_SwapWeapon5()
{
    
}
void P_SwapWeapon6()
{
    
}
void P_SwapWeapon7()
{
    
}
void P_SwapWeapon8()
{
    
}
void P_SwapWeapon9()
{
    
}
void P_SwapWeapon10()
{
    
}