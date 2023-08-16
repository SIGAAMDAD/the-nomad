#include "../engine/n_shared.h"
#include "../rendergl/rgl_public.h"
#include "../engine/n_sound.h"
#include "g_game.h"

Game* Game::gptr;
uint64_t ticcount = 0;

void Game::Init()
{
    gptr = (Game *)Hunk_Alloc(sizeof(Game), "gptr", h_low);
}

Game::~Game()
{
}