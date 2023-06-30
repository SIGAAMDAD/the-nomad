#include "../src/n_shared.h"
#include "sg_local.h"

void G_EntityTic(void)
{
    int i;
    sgentity_t *e;

    for (i = 0; i < sg_world.numEntites; i++) {
        e = &sg_world.entities[i];
        e->ticcount--;

        if (e->flags & EF_DEAD) {
            continue; // skip if dead
        }

        
    }
}