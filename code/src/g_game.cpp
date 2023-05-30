#include "n_shared.h"
#include "m_renderer.h"
#include "g_sound.h"
#include "g_game.h"

Game* Game::gptr;
uint64_t ticcount = 0;

void Game::Init()
{
    gptr = (Game *)Hunk_Alloc(sizeof(Game), "gptr", h_low);
}

Game::~Game()
{
    Snd_Kill();
    R_ShutDown();
}