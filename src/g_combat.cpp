#include "n_shared.h"
#include "g_game.h"

void P_ShootSingle(weapon_t* const wpn)
{
    int32_t range = wpn->range;
    uint16_t dmg = wpn->dmg;

    coord_t pos = E_GetDir(Game::Get()->playr->pdir.load());
    coord_t endpoint{};
    switch (Game::Get()->playr->pdir.load()) {
    case D_NORTH:
        endpoint.y.store(Game::Get()->playr->pos.y.load() - range);
        endpoint.x.store(Game::Get()->playr->pos.x.load());
        break;
    case D_WEST:
        endpoint.y.store(Game::Get()->playr->pos.y.load());
        endpoint.x.store(Game::Get()->playr->pos.x.load() - range);
        break;
    case D_SOUTH:
        endpoint.y.store(Game::Get()->playr->pos.y.load() + range);
        endpoint.x.store(Game::Get()->playr->pos.x.load());
        break;
    case D_EAST:
        endpoint.y.store(Game::Get()->playr->pos.y.load());
        endpoint.x.store(Game::Get()->playr->pos.x.load() + range);
        break;
    };
    Mob* hitmob = NULL;
    for (int32_t y = Game::Get()->playr->pos.y.load(); y != endpoint.y.load(); ++y) {
        for (int32_t x = Game::Get()->playr->pos.x.load(); x != endpoint.x.load(); ++x) {
            for (linked_list<Mob*>::iterator it = Game::Get()->m_Active.begin(); it != Game::Get()->m_Active.end();
            it = it->next) {
                if (it->val->mpos == coord_t(y, x)) {
                    hitmob = it->val;
                    goto hit;
                }
            }
        }
    }
hit:
    hitmob->health.fetch_sub(dmg);
}