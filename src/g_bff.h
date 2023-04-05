#ifndef _G_BFF_
#define _G_BFF_

#pragma once

constexpr uint16_t MAP_MAX_Y = 480;
constexpr uint16_t MAP_MAX_X = 480;
constexpr uint8_t SECTOR_MAX_Y = 120;
constexpr uint8_t SECTOR_MAX_X = 120;

typedef struct bff_spawner_s
{
    std::string name;
    uint8_t type;
    coord_t where;
    void *heap;
} bff_spawner_t;

typedef struct bff_level_s
{
    uint32_t numspawners;
    bool active;
    linked_list<Mob*> *m_Active;
    std::string lvl_name;
    std::string lvl_id;
    std::vector<bff_spawner_t> spawners;
    std::vector<std::string> intro_screen;
    std::vector<std::string> exit_screen;
    sprite_t lvl_map[MAP_MAX_Y][MAP_MAX_X];
} bff_level_t;

class BFF
{
public:
    json *data;
    std::string name;
    std::vector<bff_level_t> levels;
    std::vector<bff_spawner_t> spawners;
public:
    BFF(json* const _data);
    ~BFF();

    void BFF_LoadLevels();
    void BFF_LoadSpawners();
};

void I_CacheLevels(const std::shared_ptr<BFF>& bff);

#endif