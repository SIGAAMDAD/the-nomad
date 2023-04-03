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
    data["header"] = {
        {"version", NOMAD_VERSION},
        {"version.update", NOMAD_VERSION_UPDATE},
        {"version.patch", NOMAD_VERSION_PATCH},
        {"bffname", Game::Get()->bffname},
        {"scfname", Game::Get()->scfname}
    };
    data["playr"] = {
        {"name", Game::GetPlayr()->name},
        {"health", Game::GetPlayr()->health},
        {"armor", Game::GetPlayr()->armor},
        {"pos.y", Game::GetPlayr()->pos.y},
        {"pos.x", Game::GetPlayr()->pos.x},
        {"pdir", Game::GetPlayr()->pdir},
        {"level", Game::GetPlayr()->level},
        {"xp", Game::GetPlayr()->xp}
    };
    uint32_t nummobs = Game::GetMobs().size();
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

    std::array<uint64_t, 3> version;
    version[0] = data["header"]["version"];
    version[1] = data["header"]["version.update"];
    version[2] = data["header"]["version.patch"];

    if (version[0] != _NOMAD_VERSION) {
        bool open = true;
        char inbuf[180];
        LOG_WARN("version[0] != _NOMAD_VERSION, checking for user validation");
        while (open) {
            N_DebugWindowClear();
            ImGui::Begin("NOTICE", &open, IMGUI_STANDARD_FLAGS);
            ImGui::Text("This save file's version major is different from the current version, are you sure you want to continue?");
            ImGui::InputTextWithHint();
            ImGui::End();
            N_DebugWindowDraw();
        }
    }
}