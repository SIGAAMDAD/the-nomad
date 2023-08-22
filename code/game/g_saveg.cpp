#include "n_shared.h"
#include "../sgame/sg_local.h"
#include "../sgame/sg_mob.h"
#include "../sgame/sg_playr.h"
#include "../sgame/sg_game.h"

void G_SaveGame(uint32_t slot)
{
    char savepath[256];
    stbsp_snprintf(savepath, sizeof(savepath) - 1, "Files/gamedata/SVDATA/nomadsv%i.ngd", slot);
    FILE* fp = fopen(savepath, "wb");
    if (!fp) {
        LOG_WARN("failed to load file {} is wb mode (G_SaveGame)", filepath);
        return;
    }
    for (uint32_t i = 0; i < MAX_PLAYR_COUNT; i++) {
        playr_t* p = &sg_world.playrs[i];
        
        fwrite(p->name, sizeof(p->name), 1, fp);
        fwrite(p->inventory, sizeof(*p->inventory), MAX_PLAYR_INVENTORY, fp);
        fwrite(p->thrust, sizeof(vec2_t), 1, fp);
        fwrite(p->to, sizeof(vec2_t), 1, fp);
        fwrite(p->pos, sizeof(vec2_t), 1, fp);
        fwrite(&p->alive, sizeof(qboolean), 1, fp);
        fwrite(&p->health, sizeof(int), 1, fp);
        fwrite(&p->dir, sizeof(dirtype_t), 1, fp);
    }
    for (uint32_t i = 0; i < MAX_MOBS_ACTIVE; i++) {
        mobj_t* m = &sg_world->mobs[i];

        fwrite(m->name, sizeof(m->name), 1, fp);
        fwrite(m->to, sizeof(vec2_t), 1, fp);
        fwrite(m->pos, sizeof(vec2_t), 1, fp);
        fwrite(m->thrust, sizeof(vec2_t), 1, fp);
        fwrite(&m->health, sizeof(int), 1, fp);
        fwrite(&m->type, sizeof(mobtype_t), 1, fp);
        fwrite(&m->alive, sizeof(qboolean), 1, fp);
        fwrite(&m->dir, sizeof(dirtype_t), 1, fp);
    }
    fwrite(sg_world->tilemap, sizeof(sprite_t), MAP_MAX_Y * MAP_MAX_X, fp);
    fwrite(&sg_world->state, sizeof(gamestate_t), 1, fp);
    fclose(fp);
}