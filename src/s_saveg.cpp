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

#define UINTPTR_C(x)      ((uintptr_t)(UINTMAX_C(x)))
#define PADSAVEP()	save_p = (byte*)(((uintptr_t)save_p + UINTPTR_C(3)) & (~UINTPTR_C(3)))

void G_SaveGame()
{
	playr_t* const playr = Game::GetPlayr();
	Game* const game = Game::Get();
	
    std::string svfile = "nomadsv.ngd";
    int arg = I_GetParm("-save");
    if (arg != -1) {
        svfile = myargv[arg + 1];
    }
	FILE* fp = fopen(svfile.c_str(), "wb");
	if (!fp) {
		N_Error("G_SaveGame: failed to create a save file named %s", svfile.c_str());
	}
	
	uint64_t version[3] = { _NOMAD_VERSION, _NOMAD_VERSION_UPDATE, _NOMAD_VERSION_PATCH };
	uint64_t magic = 0x5f3759df;
	uint64_t nummobs = game->m_Active.size();
	uint64_t numitems = game->i_Active.size();
	uint64_t player_inv_size = playr->inv.size();
	uint64_t player_wpns_size = playr->P_wpns.size();
	
	// header
	fwrite(&magic, sizeof(uint64_t), 1, fp);
	fwrite(version, sizeof(uint64_t), arraylen(version), fp);
	fwrite(&game->gamestate, sizeof(gamestate_t), 1, fp);
	fwrite(&game->difficulty, sizeof(uint8_t), 1, fp);
	fwrite(&game->ticcount, sizeof(uint64_t), 1, fp);
	fwrite(game->c_map, sizeof(sprite_t), MAP_MAX_Y * MAP_MAX_X, fp);
	
	// players
	fwrite(playr->name, sizeof(char), 256, fp);
	fwrite(&playr->health, sizeof(uint64_t), 1, fp);
	fwrite(&playr->armor, sizeof(armortype_t), 1, fp);
	fwrite(&playr->pdir, sizeof(uint8_t), 1, fp);
	fwrite(&playr->xp, sizeof(uint64_t), 1, fp);
	fwrite(&playr->level, sizeof(uint64_t), 1, fp);
	fwrite(&player_inv_size, sizeof(uint64_t), 1, fp);
	fwrite(&player_wpns_size, sizeof(uint64_t), 1, fp);
    if (playr->inv.size() > 0) {
    	fwrite(playr->inv.data(), sizeof(item_t), playr->inv.size(), fp);
    }
    if (playr->P_wpns.size() > 0) {
    	fwrite(playr->P_wpns.data(), sizeof(weapon_t), playr->P_wpns.size(), fp);
    }
	fwrite(&playr->pos, sizeof(entitypos_t), 1, fp);
	
	fwrite(&nummobs, sizeof(uint64_t), 1, fp);
	if (game->m_Active.size() > 0) {
		for (linked_list<Mob*>::iterator it = game->m_Active.begin(); it != game->m_Active.end(); it = it->next) {
			Mob* const mob = it->val;
			fwrite(&mob->health, sizeof(int16_t), 1, fp);
			fwrite(&mob->mdir, sizeof(uint8_t), 1, fp);
			fwrite(&mob->mpos, sizeof(entitypos_t), 1, fp);
			fwrite(&mob->flags, sizeof(uint32_t), 1, fp);
			fwrite(&mob->c_mob, sizeof(mobj_t), 1, fp);
		}
	}
	fwrite(&numitems, sizeof(uint64_t), 1, fp);
	if (game->i_Active.size() > 0) {
		for (linked_list<item_t*>::iterator it = game->i_Active.begin(); it != game->i_Active.end(); it = it->next) {
			item_t* const item = it->val;
			fwrite(&item->cost, sizeof(uint16_t), 1, fp);
		}
	}
	fclose(fp);
}

void G_LoadGame()
{
    Game* const game = Game::Get();
    playr_t* const playr = game->playr;

    std::string svfile = "nomadsv.ngd";
    int arg = I_GetParm("-save");
    if (arg != -1) {
        svfile = myargv[arg + 1];
    }
    FILE* fp = fopen(svfile.c_str(), "rb");
	if (!fp) {
	
	}
	
	uint64_t version[3] = { _NOMAD_VERSION, _NOMAD_VERSION_UPDATE, _NOMAD_VERSION_PATCH };
	uint64_t magic = 0x5f3759df;
	uint64_t nummobs;
	uint64_t numitems;
	uint64_t player_inv_size;
	uint64_t player_wpns_size;
	
	// header
	fread(&magic, sizeof(uint64_t), 1, fp);
    if (magic != HEADER_MAGIC) {
        fclose(fp);
        N_Error("G_LoadGame: invalid header magic number, was %lx, expected %lx", magic, HEADER_MAGIC);
    }

	fread(version, sizeof(uint64_t), arraylen(version), fp);
	fread(&game->gamestate, sizeof(gamestate_t), 1, fp);
	fread(&game->difficulty, sizeof(uint8_t), 1, fp);
	fread(&game->ticcount, sizeof(uint64_t), 1, fp);
	fread(game->c_map, sizeof(sprite_t), MAP_MAX_Y * MAP_MAX_X, fp);
	
	// players
	fread(playr->name, sizeof(char), 256, fp);
	fread(&playr->health, sizeof(uint64_t), 1, fp);
	fread(&playr->armor, sizeof(armortype_t), 1, fp);
	fread(&playr->pdir, sizeof(uint8_t), 1, fp);
	fread(&playr->xp, sizeof(uint64_t), 1, fp);
	fread(&playr->level, sizeof(uint64_t), 1, fp);
	fread(&player_inv_size, sizeof(uint64_t), 1, fp);
	fread(&player_wpns_size, sizeof(uint64_t), 1, fp);

    if (playr->P_wpns.size() != player_wpns_size) {
        playr->P_wpns.resize(player_wpns_size);
    }
    if (playr->inv.size() != player_inv_size) {
        playr->inv.resize(player_inv_size);
    }
    if (player_inv_size > 0) {
        playr->inv.resize(player_inv_size);
        fread(playr->inv.data(), sizeof(item_t), player_inv_size, fp);
    }
    if (player_wpns_size > 0)  {
        playr->P_wpns.resize(player_wpns_size);
        fread(playr->P_wpns.data(), sizeof(weapon_t), player_wpns_size, fp);
    }
	fread(&playr->pos, sizeof(entitypos_t), 1, fp);
	
	fread(&nummobs, sizeof(uint64_t), 1, fp);
    if (nummobs > 0) {
        for (linked_list<Mob*>::iterator it = game->m_Active.begin(); it != game->m_Active.end(); it = it->next) {
            Z_Free(it->val);
        }
        game->m_Active.clear();
        for (uint64_t i = 0; i < nummobs; ++i) {
            game->m_Active.emplace_back();
            game->m_Active.back() = (Mob *)Z_Malloc(sizeof(Mob), TAG_STATIC, &game->m_Active.back());
            Mob* const mob = game->m_Active.back();
			fread(&mob->health, sizeof(int16_t), 1, fp);
			fread(&mob->mdir, sizeof(uint8_t), 1, fp);
			fread(&mob->mpos, sizeof(entitypos_t), 1, fp);
			fread(&mob->flags, sizeof(uint32_t), 1, fp);
			fread(&mob->c_mob, sizeof(mobj_t), 1, fp);
        }
    }
    fread(&numitems, sizeof(uint64_t), 1, fp);
    if (numitems > 0) {
        for (linked_list<item_t*>::iterator it = game->i_Active.begin(); it != game->i_Active.end(); it = it->next) {
            Z_Free(it->val);
        }
        game->i_Active.clear();
        for (uint64_t i = 0; i < numitems; ++i) {
            game->i_Active.emplace_back();
            game->i_Active.back() = (item_t *)Z_Malloc(sizeof(item_t), TAG_STATIC, &game->i_Active.back());
            fread(game->i_Active.back(), sizeof(item_t), 1, fp);
        }
    }
	fclose(fp);
}