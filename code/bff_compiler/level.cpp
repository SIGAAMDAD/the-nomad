#include "../bff_file/g_bff.h"

void GetLightData(bfflevel_t* level, bffinfo_t* info)
{
	level->numLights = 0;
	// get the total lights in the level
	for (uint32_t s = 0; s < NUMSECTORS; s++) {
		for (uint32_t y = 0; y < SECTOR_MAX_Y; y++) {
			for (uint32_t x = 0; x < SECTOR_MAX_X; x++) {
				switch (level->tilemap[s][y][x]) {
				case SPR_LAMP:
					level->numLights++;
					break;
				default: break; // ignore everything else
				};
			}
		}
	}
	if (level->numLights >= MAX_MAP_LIGHTS) {
		BFF_Error("level map lights count exceeded limit of %i lights", MAX_MAP_LIGHTS);
	}
	
	// process the light data
	for (uint32_t i = 0; i < level->numLights; i++) {
		maplight_t* light = &level->lights[i];
		for (uint32_t s = 0; s < NUMSECTORS; s++) {
			for (uint32_t y = 0; y < SECTOR_MAX_Y; y++) {
				for (uint32_t x = 0; x < SECTOR_MAX_X; x++) {
					switch (level->tilemap[s][y][x]) {
					case SPR_LAMP:
						light->y = y;
						light->x = x;
						light->sector = s;
						break;
					};
				}
			}
		}
		
		// sanity checks
		if (light->y >= SECTOR_MAX_Y) {
			Con_Printf("WARNING: light->y >= SECTOR_MAX_Y, setting to SECTOR_MAX_Y - 1");
			light->y = SECTOR_MAX_Y - 1;
		}
		if (light->x >= SECTOR_MAX_X) {
			Con_Printf("WARNING: light->x >= SECTOR_MAX_X, setting to SECTOR_MAX_X - 1");
			light->x = SECTOR_MAX_X - 1;
		}
		if (light->sector >= NUMSECTORS) {
			BFF_Error("invalid light because sector index isn't valid");
		}
		
		light->intensity = info->config.lightIntensity;
		light->aoe = info->config.lightAOE;
	}
}

void GetSpawnData(bfflevel_t* level)
{
	level->numSpawns = 0;
	// get the total spawners in the level
	for (uint32_t s = 0; s < NUMSECTORS; s++) {
		for (uint32_t y = 0; y < SECTOR_MAX_Y; y++) {
			for (uint32_t x = 0; x < SECTOR_MAX_X; x++) {
				switch (level->tilemap[s][y][x]) {
				case SPR_SPAWNER:
					level->numSpawns++;
					break;
				default: break; // ignore everything else
				};
			}
		}
	}
	if (level->numSpawns >= MAX_MAP_SPAWNS) {
		BFF_Error("level map spawner count exceeded limit of %i spawners", MAX_MAP_SPAWNS);
	}
	
	// process the spawner locations
	for (uint32_t i = 0; i < level->numSpawns; i++) {
		mapspawn_t* spawn = &level->spawns[i];
		for (uint32_t s = 0; s < NUMSECTORS; s++) {
			for (uint32_t y = 0; y < SECTOR_MAX_Y; y++) {
				for (uint32_t x = 0; x < SECTOR_MAX_X; x++) {
					switch (level->tilemap[s][y][x]) {
					case SPR_SPAWNER:
						spawn->y = y;
						spawn->x = x;
						spawn->sector = s;
						break;
					default: break;
					};
				}
			}
		}
		// sanity checks
		if (spawn->y >= SECTOR_MAX_Y) {
			Con_Printf("WARNING: spawn->y >= SECTOR_MAX_Y, setting to SECTOR_MAX_Y - 1");
			spawn->y = SECTOR_MAX_Y - 1;
		}
		if (spawn->x >= SECTOR_MAX_X) {
			Con_Printf("WARNING: spawn->x >= SECTOR_MAX_X, setting to SECTOR_MAX_X - 1");
			spawn->x = SECTOR_MAX_X - 1;
		}
		if (spawn->sector >= NUMSECTORS) {
			BFF_Error("invalid spawner because sector index isn't valid");
		}
	}
}

void GetSectorMapData(char tilemap[SECTOR_MAX_Y][SECTOR_MAX_X], const std::string& mapfile)
{
	std::ifstream file(mapfile.c_str(), std::ios::in);
	if (file.fail()) {
		BFF_Error("WriteBFF: failed to open mapfile %s", mapfile.c_str());
	}
	file.seekg(0L, std::ios_base::end);
	size_t fsize = file.tellg();
	file.seekg(0L, std::ios_base::beg);
	std::vector<std::string> strbuf;
	std::string linebuf;
	strbuf.reserve(fsize);
	while (std::getline(file, linebuf)) {
		strbuf.emplace_back(linebuf);
	}
	file.close();
	
	for (uint32_t y = 0; y < SECTOR_MAX_Y; y++) {
		for (uint32_t x = 0; x < SECTOR_MAX_X; x++) {
			tilemap[y][x] = strbuf[y][x];
		}
	}
}

void GetSectorMaps(bfflevel_t* level, const json& data, const std::string& levelnode, uint32_t index)
{
	for (uint32_t i = 0; i < NUMSECTORS; i++) {
		const std::string sectornode = "sector_"+std::to_string(i);
		Con_Printf("loading level sector %s for level %s", sectornode.c_str(), levelnode.c_str());
		if (!data["levels"][levelnode].contains(sectornode)) {
			BFF_Error("WriteBFF: levels must have exactly %hu sectors, level only has %iu sectors, level index: %i", NUMSECTORS, i, index);
		}
		if (!data["levels"][levelnode][sectornode].contains("mapfile")) {
			BFF_Error("WriteBFF: level sector must have a mapfile, sector: %hu, level: %i", i, index);
		}
		const std::string mapfile = data["levels"][levelnode][sectornode]["mapfile"];
		
		GetSectorMapData(level->tilemap[i], mapfile);
	}
}

void GetLevels(const json& data, bffinfo_t* info)
{
	if (!data.contains("levels")) {
		BFF_Error("WriteBFF: each bff file must have at least one level");
	}
	info->numLevels = data["levels"].size();
	if (info->numLevels >= MAX_LEVEL_CHUNKS) {
		BFF_Error("WriteBFF: the number of levels in entries file exceeds the limit of %i levels", MAX_LEVEL_CHUNKS);
	}
	memset(info->levels, 0, sizeof(info->levels));
	for (bff_int_t i = 0; i < info->numLevels; i++) {
		bfflevel_t* level = &info->levels[i];
		const std::string levelnode = "level_"+std::to_string(i);

		if (data["levels"][levelnode].contains("name")) {
			const std::string name = data["levels"][levelnode]["name"];
			Con_Printf("loading level %s", name.c_str());
			if (name.size() > MAX_BFF_CHUNKNAME - 1) {
				Con_Printf("WARNING: level name at index %lu is greater than %i characters, truncating", i, MAX_BFF_CHUNKNAME - 1);
			}
			strncpy(level->name, name.c_str(), MAX_BFF_CHUNKNAME - 1);
		}
		else {
			Con_Printf("WARNING: level at index %lu doesn't have an assigned name, using default name of %s", i, levelnode.c_str());
			strncpy(level->name, levelnode.c_str(), MAX_BFF_CHUNKNAME - 1);
		}
		
		memset(level->lights, 0, sizeof(level->lights));
		memset(level->spawns, 0, sizeof(level->spawns));
		GetSectorMaps(level, data, levelnode, i);
		GetSpawnData(level);
		GetLightData(level, info);
	}
}
