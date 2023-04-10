#include "n_shared.h"
#include "g_game.h"

json& N_GetSaveJSon()
{
    LOG_INFO("loading svdata.json for reading");
    std::ifstream file("Files/gamedata/SAVES/svdata.json", std::ios::in | std::ios::binary);
    if (file.fail()) {
        N_Error("N_GetSaveJSon: failed to load Files/gamedata/SAVES/svdata.json, what did you do with, it, huh?");
    }
    static json data = json::from_bjdata(file);
    file.close();
    LOG_INFO("success");

    for (int i = 0; i < MAXSAVES; ++i) {
        std::string node_name = "sv_"+std::to_string(i);
        const std::string svname = data[node_name]["name"];
        LOG_INFO("save file {} name: {}", i, svname.c_str());
    }
    return data;
}

void G_SaveGame()
{
    std::string svfile = "nomadsv.ngd";
    int arg = I_GetParm("-save");
    if (arg != -1) {
        svfile = myargv[arg + 1];
    }
    FILE* fp = fopen(svfile.c_str(), "wb");
    if (!fp) {
        LOG_WARN("G_SaveGame: failed to open a writeonly filestream for savefile {}, aborting.", svfile);
        return;
    }

    Game* const game = Game::Get();
    size_t nummobs = game->m_Active.size();
    size_t numitems = game->i_Active.size();
    fwrite(&game->difficulty, sizeof(uint8_t), 1, fp);
    fwrite(&game->gamestate, sizeof(gamestate_t), 1, fp);
    fwrite(&nummobs, sizeof(size_t), 1, fp);
    fwrite(&numitems, sizeof(size_t), 1, fp);
    
    playr_t* const playr = Game::GetPlayr();
    fwrite(playr->name, sizeof(char), 256, fp);
    fwrite(&playr->health, sizeof(int_fast16_t), 1, fp);
    fwrite(&playr->armor, sizeof(armortype_t), 1, fp);
    fwrite(&playr->level, sizeof(uint_fast16_t), 1, fp);
    fwrite(&playr->xp, sizeof(uint_fast64_t), 1, fp);
    fwrite(&playr->pos, sizeof(coord_t), 1, fp);
    fwrite(&playr->pdir, sizeof(uint_fast8_t), 1, fp);
    size_t invsize = playr->inv.size();
    fwrite(&invsize, sizeof(size_t), 1, fp);
    for (size_t i = 0; i < playr->inv.size(); ++i)
        fwrite(&playr->inv[i], sizeof(item_t), 1, fp);
    
    fwrite(playr->P_wpns, sizeof(weapon_t), PLAYR_MAX_WPNS, fp);
    uint8_t c_wpn_index = 0;
    for (int i = 0; i < arraylen(playr->P_wpns); ++i) {
        if (N_memcmp(&playr->P_wpns[i], playr->c_wpn, sizeof(item_t))) {
            c_wpn_index = i;
            break;
        }
    }
    fwrite(&c_wpn_index, sizeof(uint8_t), 1, fp);

    if (nummobs >= 1) {
        linked_list<Mob*>::iterator it = game->m_Active.begin();
        for (size_t i = 0; i < nummobs; ++i) {
            Mob* const mob = it->val;
            fwrite(&mob->health, sizeof(int16_t), 1, fp);
            fwrite(&mob->flags, sizeof(uint32_t), 1, fp);
            fwrite(&mob->mpos, sizeof(coord_t), 1, fp);
            fwrite(&mob->mdir, sizeof(uint8_t), 1, fp);
            fwrite(&mob->c_mob, sizeof(mobj_t), 1, fp);
            it = it->next;
        }
    }
    if (numitems >= 1) {
        linked_list<item_t*>::iterator it = game->i_Active.begin();
        for (size_t i = 0; i < numitems; ++i) {
            fwrite(it->val, sizeof(item_t), 1, fp);
        }
    }
    fclose(fp);
}

void G_LoadGame()
{
    std::string svfile = "nomadsv.ngd";
    int arg = I_GetParm("-save");
    if (arg != -1) {
        svfile = myargv[arg + 1];
    }
    FILE* fp = fopen(svfile.c_str(), "rb");
    if (!fp) {
        LOG_WARN("G_LoadGame: failed to open a readonly filestream for savefile {}, aborting.", svfile);
        return;
    }
    Game* const game = Game::Get();
    size_t nummobs, numitems;
    fread(&game->difficulty, sizeof(uint8_t), 1, fp);
    fread(&game->gamestate, sizeof(gamestate_t), 1, fp);
    fread(&nummobs, sizeof(size_t), 1, fp);
    fread(&numitems, sizeof(size_t), 1, fp);
    
    playr_t* const playr = Game::GetPlayr();
    fread(playr->name, sizeof(char), 256, fp);
    fread(&playr->health, sizeof(int_fast16_t), 1, fp);
    fread(&playr->armor, sizeof(armortype_t), 1, fp);
    fread(&playr->level, sizeof(uint_fast16_t), 1, fp);
    fread(&playr->xp, sizeof(uint_fast64_t), 1, fp);
    fread(&playr->pos, sizeof(coord_t), 1, fp);
    fread(&playr->pdir, sizeof(uint_fast8_t), 1, fp);
    size_t invsize;
    playr->inv.clear();
    fread(&invsize, sizeof(size_t), 1, fp);
    if (invsize >= 1) {
        playr->inv.resize(invsize);
        fread(playr->inv.data(), sizeof(item_t), invsize, fp);
    }
    fread(playr->P_wpns, sizeof(weapon_t), PLAYR_MAX_WPNS, fp);
    uint8_t c_wpn_index = 0;
    fread(&c_wpn_index, sizeof(uint8_t), 1, fp);
    playr->c_wpn = &playr->P_wpns[c_wpn_index];

    if (nummobs >= 1) {
        if (Game::GetMobs().size() >= 1) {
            linked_list<Mob*>::iterator it = Game::GetMobs().begin();
            for (size_t i = 0; i < Game::GetMobs().size(); ++i) {
                Z_Free(it->val);
                it = it->next;
            }
        }
        Game::GetMobs().clear();
        for (size_t i = 0; i < nummobs; ++i) {
            Game::GetMobs().emplace_back();
            Game::GetMobs().back() = (Mob *)Z_Malloc(sizeof(Mob), TAG_STATIC, &Game::GetMobs().back());
            Mob* const mob = Game::GetMobs().back();
            fread(&mob->health, sizeof(int16_t), 1, fp);
            fread(&mob->flags, sizeof(uint32_t), 1, fp);
            fread(&mob->mpos, sizeof(coord_t), 1, fp);
            fread(&mob->mdir, sizeof(uint8_t), 1, fp);
            fread(&mob->c_mob, sizeof(mobj_t), 1, fp);
        }
    }
    if (numitems >= 1) {
        if (Game::Get()->i_Active.size() >= 1) {
            linked_list<item_t*>::iterator it = Game::Get()->i_Active.begin();
            for (size_t i = 0; i < Game::Get()->i_Active.size(); ++i) {
                Z_Free(it->val);
                it = it->next;
            }
        }
        for (size_t i = 0; i < numitems; ++i) {
            Game::Get()->i_Active.emplace_back();
            fread(Game::Get()->i_Active.back(), sizeof(item_t), 1, fp);
        }
    }
    fclose(fp);
}