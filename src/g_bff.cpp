#include "n_shared.h"
#include "g_game.h"

#define FILEPATH(x) (std::string("Files/gamedata/BFF/")+std::string(Game::Get()->bffname)+"/"+std::string(x)).c_str()

//
// I_CacheLevel: pre-caches the level data during initialization of the bff directory
//
void I_CacheLevel(const std::shared_ptr<BFF>& bff)
{
    json& data = *bff->data;
    uint32_t index = 0;
    for (auto& i : bff->levels) {
        const std::string& node_name = "level_"+std::to_string(index);
        json level = data[node_name];

        const std::string mapfile = level["mapfile"];
        std::string buffer;
        {
            FILE* fp = fopen(FILEPATH(mapfile), "w");
            if (!fp) {

            }
            char c = getc(fp);
            while (c != EOF) {
                buffer.push_back(c);
                c = getc(fp);
            }
        }
        Lexer lex(buffer.c_str());
        ++index;
    }
}

void G_LoadBFF(const char* filename)
{
    LOG_INFO("opening bff entries file");
    std::ifstream file(FILEPATH(filename), std::ios::in);
    if (file.fail()) {
        N_Error("G_LoadBFF: failed to open file %s", filename);
    }
    assert(file.is_open());
    json data = json::parse(file);
    file.close();
    LOG_INFO("done");
    std::shared_ptr<BFF> bff = std::make_shared<BFF>(&data);

    bff->BFF_LoadLevels();
    Game::Get()->bff = bff;
    LOG_INFO("finished bff loading");
}

BFF::BFF(json* const _data)
    : data(_data)
{
    N_memset(name, 0, sizeof(name));
}

BFF::~BFF()
{
}

void BFF::BFF_LoadLevels()
{
    const json& _header = (*data)["header"];
    uint32_t numlevels = _header["numlevels"];
    levels.reserve(numlevels);
    for (uint32_t i = 0; i < numlevels; ++i) {
        const std::string node_name = "level_"+std::to_string(i);
        LOG_INFO("BFF_LoadLevels: new level loaded, name: %s", ptr->lvl_name);
    }
    LOG_INFO("BFF_LoadLevels: total levels loaded: %iu", numlevels);
}