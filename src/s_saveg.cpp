#include "n_shared.h"
#include "g_game.h"
#include <cpuid.h>

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

byte *buffer;
byte *save_p;
size_t buffer_size;
size_t buffer_index;

// hacky overload
ssize_t write(const void *buf, const size_t elemsize, const size_t nelem)
{
	buffer = (byte *)realloc(buffer, buffer_size + (elemsize * nelem) + 16);
	if (!buffer) {
		N_Error("G_SaveGame: n_realloc() failed");
	}
	assert(buffer);
	memmove(&buffer[buffer_index], buf, elemsize * nelem);
	buffer_size += elemsize * nelem + 16;
	buffer_index += elemsize * nelem;
	save_p = &buffer[buffer_index];
	return elemsize * nelem;
}
ssize_t read(void *buf, const size_t elemsize, const size_t nelem)
{
	memmove(buf, save_p, elemsize * nelem);
	save_p += elemsize * nelem;
	return elemsize * nelem;
}

void G_SaveGame()
{
	playr_t* const playr = Game::GetPlayr();
	Game* const game = Game::Get();
	
    std::string svfile = "nomadsv.ngd";
    int arg = I_GetParm("-save");
    if (arg != -1) {
        svfile = myargv[arg + 1];
    }
	uint64_t magic = 0x5f3759df;
	uint32_t player_size = sizeof(playr_t) - ((sizeof(void *) * 6) + (24 << 1));
	uint16_t version_major = _NOMAD_VERSION;
	uint32_t version_update = _NOMAD_VERSION_UPDATE;
	uint64_t version_patch = _NOMAD_VERSION_PATCH;
	char versionstring[80];
	memset(versionstring, 0, sizeof(versionstring));
	stbsp_snprintf(versionstring, 80, "thenomad-v%hu.%i.%ld", version_major, version_update, version_patch);

	buffer_size = buffer_index = 0;
	uint16_t numentities = game->entities.size();

	write(versionstring, sizeof(char), 80);
	write(&version_major, sizeof(uint16_t), 1);
	write(&version_update, sizeof(uint32_t), 1);
	write(&version_patch, sizeof(uint64_t), 1);
	
	PADSAVEP();
	write(&magic, sizeof(uint64_t), 1);
	write(&numentities, sizeof(uint16_t), 1);
	write(&game->difficulty, sizeof(uint8_t), 1);
	write(&game->gamestate, sizeof(gamestate_t), 1);
	write(&game->ticcount, sizeof(uint64_t), 1);

	PADSAVEP();
	write(&playr->level, sizeof(uint64_t), 1);
	write(&playr->xp, sizeof(uint64_t), 1);
	write(playr->P_wpns, sizeof(weapon_t), PLAYR_MAX_WPNS);
	write(playr->inv, sizeof(item_t), PLAYR_MAX_ITEMS);

	linked_list<entity_t>::iterator entities = game->entities.begin();
	for (uint16_t i = 0; i < numentities; ++i) {
		PADSAVEP();
		write(&entities->val, sizeof(entity_t) - sizeof(entitypos_t) + sizeof(state_t) + (sizeof(void *) * 2) + 1, 1);
		write(&entities->val.state, sizeof(state_t), 1);
		write(&entities->val.dir, sizeof(uint8_t), 1);
		write(&entities->val.ticker, sizeof(uint64_t), 1);
		write(&entities->val.pos.coords, sizeof(glm::vec3), 1);
		write(&entities->val.pos.lookangle, sizeof(glm::vec2), 1);
		write(entities->val.pos.hitbox, sizeof(vec2_t), 4);
		write(entities->val.pos.to, sizeof(vec3_t), 1);
		write(entities->val.pos.thrust, sizeof(vec3_t), 1);
		entities = entities->next;
	}
	PADSAVEP();
	write(game->c_map, sizeof(sprite_t), MAP_MAX_Y*MAP_MAX_X);

	N_WriteFile(svfile.c_str(), buffer, buffer_size);
	free(buffer);
}

void G_LoadGame()
{
    playr_t* const playr = Game::GetPlayr();
	Game* const game = Game::Get();
	
    std::string svfile = "nomadsv.ngd";
    int arg = I_GetParm("-save");
    if (arg != -1) {
        svfile = myargv[arg + 1];
    }
	uint64_t magic = 0x5f3759df;
	uint16_t numentities = game->entities.size();
	uint64_t player_size = sizeof(playr_t) - ((sizeof(void *) * 6) + (24 << 1));
	uint16_t version_major = 0;
	uint32_t version_update = 0;
	uint64_t version_patch = 0;
	char versionstring[80];
	memset(versionstring, 0, sizeof(versionstring));

	N_ReadFile(svfile.c_str(), (char **)&buffer);
	save_p = buffer;

	read(versionstring, sizeof(char), 80);
	read(&version_major, sizeof(uint16_t), 1);
	read(&version_update, sizeof(uint32_t), 1);
	read(&version_patch, sizeof(uint64_t), 1);
	
	PADSAVEP();
	read(&magic, sizeof(uint64_t), 1);
	read(&numentities, sizeof(uint16_t), 1);
	read(&game->difficulty, sizeof(uint8_t), 1);
	read(&game->gamestate, sizeof(gamestate_t), 1);
	read(&game->ticcount, sizeof(uint64_t), 1);

	PADSAVEP();
	read(&playr->level, sizeof(uint64_t), 1);
	read(&playr->xp, sizeof(uint64_t), 1);
	read(playr->P_wpns, sizeof(weapon_t), PLAYR_MAX_WPNS);
	read(playr->inv, sizeof(item_t), PLAYR_MAX_ITEMS);

	game->entities.clear();

	for (uint16_t i = 0; i < numentities; ++i) {
		PADSAVEP();
		game->entities.emplace_back();
		entity_t* const entity = &game->entities.back();
		read(entity, sizeof(entity_t) - sizeof(entitypos_t) + sizeof(state_t) + (sizeof(void *) * 2) + 1, 1);
		read(&entity->state, sizeof(state_t), 1);
		read(&entity->dir, sizeof(uint8_t), 1);
		read(&entity->ticker, sizeof(uint64_t), 1);
		read(&entity->pos.coords, sizeof(glm::vec3), 1);
		read(&entity->pos.lookangle, sizeof(glm::vec2), 1);
		read(entity->pos.hitbox, sizeof(vec2_t), 4);
		read(entity->pos.to, sizeof(vec3_t), 1);
		read(entity->pos.thrust, sizeof(vec3_t), 1);
	}
	PADSAVEP();
	read(game->c_map, sizeof(sprite_t), MAP_MAX_Y*MAP_MAX_X);

	free(buffer);
}