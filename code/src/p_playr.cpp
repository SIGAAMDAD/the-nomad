#include "n_shared.h"
#include "g_game.h"

void P_MoveN()
{
    --Game::GetPlayr()->p->pos.coords.y;
}

void P_MoveW()
{
    --Game::GetPlayr()->p->pos.coords.x;
}

void P_MoveS()
{
    ++Game::GetPlayr()->p->pos.coords.y;
}

void P_MoveE()
{
    ++Game::GetPlayr()->p->pos.coords.x;
}

void P_NextWeapon()
{
    Game::GetPlayr()->swap = Game::GetPlayr()->c_wpn;
    if (Game::GetPlayr()->c_wpn == &Game::GetPlayr()->P_wpns[PLAYR_MAX_WPNS - 1])
        Game::GetPlayr()->c_wpn = &Game::GetPlayr()->P_wpns[0];
    else
        ++Game::GetPlayr()->c_wpn;
}
void P_PrevWeapon()
{
    Game::GetPlayr()->swap = Game::GetPlayr()->c_wpn;
    if (Game::GetPlayr()->c_wpn == &Game::GetPlayr()->P_wpns[PLAYR_MAX_WPNS - 1])
        Game::GetPlayr()->c_wpn = &Game::GetPlayr()->P_wpns[0];
    else
        --Game::GetPlayr()->c_wpn;
}
void P_QuickSwap()
{
    weapon_t* const tmp = Game::GetPlayr()->c_wpn;
    Game::GetPlayr()->c_wpn = Game::GetPlayr()->swap;
    Game::GetPlayr()->swap = tmp;
}

void P_ChangeDirL()
{
    if (Game::GetPlayr()->p->dir == D_EAST)
        Game::GetPlayr()->p->dir = D_NORTH;
    else
        ++Game::GetPlayr()->p->dir;
}

void P_ChangeDirR()
{
    if (Game::GetPlayr()->p->dir == D_NORTH)
        Game::GetPlayr()->p->dir = D_EAST;
    else
        --Game::GetPlayr()->p->dir;
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

void P_ZoomIn()
{

}

void P_ZoomOut()
{
    
}