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

#if 0
byte *buffer;
byte *save_p;
size_t buffer_size;
size_t buffer_index;

void *n_realloc(void *ptr, size_t nsize)
{
	void *p = malloc(nsize);
	if (ptr) {
		memmove(p, ptr, nsize);
		free(ptr);
	}
	return p;
}

// hacky overload
ssize_t write(const void *buf, const size_t elemsize, const size_t nelem)
{
	buffer = (byte *)realloc(buffer, buffer_size + (elemsize * nelem));
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
#endif

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

	uint16_t numentities = game->entities.size();

	FILE* fp = fopen(svfile.c_str(), "wb");
	if (!fp) {
		LOG_ERROR("failed to create a save file named {}, aborting.", svfile);
		return;
	}
	assert(fp);

	fwrite(versionstring, sizeof(char), 80, fp);
	fwrite(&version_major, sizeof(uint16_t), 1, fp);
	fwrite(&version_update, sizeof(uint32_t), 1, fp);
	fwrite(&version_patch, sizeof(uint64_t), 1, fp);
	
	fwrite(&magic, sizeof(uint64_t), 1, fp);
	fwrite(&numentities, sizeof(uint16_t), 1, fp);
	fwrite(&game->difficulty, sizeof(uint8_t), 1, fp);
	fwrite(&game->gamestate, sizeof(gamestate_t), 1, fp);
	fwrite(&ticcount, sizeof(uint64_t), 1, fp);

	fwrite(&playr->level, sizeof(uint64_t), 1, fp);
	fwrite(&playr->xp, sizeof(uint64_t), 1, fp);
	fwrite(playr->P_wpns, sizeof(weapon_t), PLAYR_MAX_WPNS, fp);
	fwrite(playr->inv, sizeof(item_t), PLAYR_MAX_ITEMS, fp);

	linked_list<entity_t>::iterator entities = game->entities.begin();
	for (uint16_t i = 0; i < numentities; ++i) {
		fwrite(&entities->val, sizeof(entity_t) - sizeof(entitypos_t) + sizeof(state_t) + (sizeof(void *) * 2) + 1, 1, fp);
		fwrite(&entities->val.state, sizeof(state_t), 1, fp);
		fwrite(&entities->val.dir, sizeof(uint8_t), 1, fp);
		fwrite(&entities->val.ticker, sizeof(uint64_t), 1, fp);
		fwrite(&entities->val.pos.coords, sizeof(glm::vec3), 1, fp);
		fwrite(&entities->val.pos.lookangle, sizeof(glm::vec2), 1, fp);
		fwrite(entities->val.pos.hitbox, sizeof(vec2_t), 4, fp);
		fwrite(&entities->val.pos.to, sizeof(vec3_t), 1, fp);
		fwrite(&entities->val.pos.thrust, sizeof(vec3_t), 1, fp);
		entities = entities->next;
	}
	fwrite(game->c_map, sizeof(sprite_t), MAP_MAX_Y*MAP_MAX_X, fp);
	fclose(fp);
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

	FILE* fp = fopen(svfile.c_str(), "rb");
	if (!fp) {
		LOG_ERROR("failed to fopen save file {} for loading", svfile);
		return;
	}
	assert(fp);

	fread(versionstring, sizeof(char), 80, fp);
	fread(&version_major, sizeof(uint16_t), 1, fp);
	fread(&version_update, sizeof(uint32_t), 1, fp);
	fread(&version_patch, sizeof(uint64_t), 1, fp);
	
	fread(&magic, sizeof(uint64_t), 1, fp);
	fread(&numentities, sizeof(uint16_t), 1, fp);
	fread(&game->difficulty, sizeof(uint8_t), 1, fp);
	fread(&game->gamestate, sizeof(gamestate_t), 1, fp);
	fread(&ticcount, sizeof(uint64_t), 1, fp);

	fread(&playr->level, sizeof(uint64_t), 1, fp);
	fread(&playr->xp, sizeof(uint64_t), 1, fp);
	fread(playr->P_wpns, sizeof(weapon_t), PLAYR_MAX_WPNS, fp);
	fread(playr->inv, sizeof(item_t), PLAYR_MAX_ITEMS, fp);

	game->entities.clear();

	for (uint16_t i = 0; i < numentities; ++i) {
		game->entities.emplace_back();
		entity_t* const entity = &game->entities.back();
		fread(entity, sizeof(entity_t) - sizeof(entitypos_t) + sizeof(state_t) + (sizeof(void *) * 2) + 1, 1, fp);
		fread(&entity->state, sizeof(state_t), 1, fp);
		fread(&entity->dir, sizeof(uint8_t), 1, fp);
		fread(&entity->ticker, sizeof(uint64_t), 1, fp);
		fread(&entity->pos.coords, sizeof(glm::vec3), 1, fp);
		fread(&entity->pos.lookangle, sizeof(glm::vec2), 1, fp);
		fread(entity->pos.hitbox, sizeof(vec2_t), 4, fp);
		fread(&entity->pos.to, sizeof(vec3_t), 1, fp);
		fread(&entity->pos.thrust, sizeof(vec3_t), 1, fp);
	}
	fread(game->c_map, sizeof(sprite_t), MAP_MAX_Y*MAP_MAX_X, fp);
	fclose(fp);
}