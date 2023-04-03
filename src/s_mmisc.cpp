#include "n_shared.h"
#include "g_game.h"

Mob::Mob()
    : health(0), flags(0), mpos(0, 0), mdir(P_Random() & 3)
{
}

void M_RunThinker(linked_list<Mob*>::iterator it)
{
    Mob* const actor = it->val;
}