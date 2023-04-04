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
        LOG_INFO("save file %i name: %s", i, svname.c_str());
    }
    return data;
}

void G_SaveGame(const char* svfile)
{
    assert(svfile);
    json data;
    uint32_t nummobs = Game::GetMobs().size();

    data["header"] = {
        {"version", NOMAD_VERSION},
        {"version.update", NOMAD_VERSION_UPDATE},
        {"version.patch", NOMAD_VERSION_PATCH},
        {"bffname", Game::Get()->bffname},
        {"scfname", Game::Get()->scfname},
        {"nummobs", nummobs}
    };
    data["playr"] = {
        {"name", std::string(Game::GetPlayr()->name)},
        {"health", Game::GetPlayr()->health},
        {"armor", Game::GetPlayr()->armor},
        {"pos.y", Game::GetPlayr()->pos.y},
        {"pos.x", Game::GetPlayr()->pos.x},
        {"pdir", Game::GetPlayr()->pdir},
        {"level", Game::GetPlayr()->level},
        {"xp", Game::GetPlayr()->xp}
    };
    linked_list<Mob*>::iterator it = Game::GetMobs().begin();
    for (uint32_t i = 0; i < nummobs; ++i) {
        std::string node_name = "mob_"+std::to_string(i);
        data[node_name] = {
            {"health", it->val->health},
            {"mpos.y", it->val->mpos.y},
            {"mpos.x", it->val->mpos.x},
            {"mdir", it->val->mdir},
            {"flags", it->val->flags},
            {"mobj_index", it->val->getmobjindex()}
        };
        it = it->next;
    }
    std::vector<uint8_t> bin = json::to_bjdata(data);

    std::ofstream file(svfile, std::ios::out | std::ios::binary);
    file << bin.data();
    file.close();
}

void G_LoadGame(const char* svfile)
{
    assert(svfile);
    std::ifstream file(svfile, std::ios::in | std::ios::binary);
    json data = json::from_bjdata(file);
    file.close();

    uint64_t version[3];
    *version = data["header"]["version"];
    version[1] = data["header"]["version.update"];
    version[2] = data["header"]["version.patch"];
    uint32_t nummobs = data["header"]["nummobs"];

    if (version[0] != _NOMAD_VERSION) {
        LOG_WARN("version[0] != _NOMAD_VERSION");
    }

    {
        const std::string name = data["playr"]["name"];
        N_strncpy(Game::GetPlayr()->name, name.c_str(), 256);
        playr->armor = data["playr"]["armor"];
        playr->health = data["playr"]["health"];
        playr->level = data["playr"]["level"];
        playr->xp = data["playr"]["level"];
        playr->pos.y = data["playr"]["pos.y"];
        playr->pos.x = data["playr"]["pos.x"];
    }

    for (linked_list<Mob*>::iterator it = Game::GetMobs().begin(); it != Game::GetMobs().end();
    it = it->next) {
        Z_Free(it->val);
    }
    Game::GetMobs().clear();
    linked_list<Mob*>::iterator it = Game::GetMobs().begin();
    for (uint32_t i = 0; i < nummobs; ++i) {
        const std::string node_name = "mob_"+std::to_string(i);
        Mob* const mob = it->val;
        mob->health = data[node_name]["health"];
        mob->mpos.y = data[node_name]["mpos.y"];
        mob->mpos.x = data[node_name]["mpos.x"];
        mob->c_mob = mobinfo[data["mob"]["mobj_index"]];
        mob->flags = data[node_name]["flags"];
        mob->mdir = data[node_name]["mdir"];
        it = it->next;
    }
}