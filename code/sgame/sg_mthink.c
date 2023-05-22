#include "../src/n_shared.h"
#include "sg_local.h"
#include "sg_entity.h"
#include "sg_mob.h"
#include "sg_game.h"

void G_SpawnMob(mobtype_t type, const vec2_t pos)
{
    mobj_t* mob;
    int i;

    mob = NULL;
    if (type >= NUMMOBS) {
        Con_Printf("WARNING: attempted to spawn a mob with an invalid type");
        return;
    }
    for (i = 0; i < MAX_MOBS_ACTIVE; i++) {
        if (!sg_world.mobs[i].alive) {
            mob = &sg_world.mobs[i];
            break;
        }
    }
    mob->dir = D_NORTH;
    mob->health = mobinfo[type].health;
    mob->alive = qtrue;
    strncpy(mob->name, mobinfo[type].name, sizeof(mob->name) - 1);
    VectorCopy(mob->pos, pos);
    VectorCopy(mob->thrust, vec2_origin);
    VectorCopy(mob->to, vec2_origin);
}

void M_KillMob(mobj_t* mob)
{
    mob->alive = qfalse;
}

void M_RunThinkers(void)
{
    mobj_t* mob;
    int i;

    for (i = 0; i < MAX_MOBS_ACTIVE; i++) {
        if (sg_world.mobs[i].alive) {
            mob = &sg_world.mobs[i];
            (*mob->think)(mob);
        }
    }
}
