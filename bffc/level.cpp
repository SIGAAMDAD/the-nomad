#include "../bff_file/g_bff.h"

// from Quake3e
void StripExtension(const char *in, char* out, uint32_t destsize)
{
	const char *dot = strrchr(in, '.'), *slash;

	if (dot && ((slash = strrchr(in, '/')) == NULL || slash < dot))
		destsize = (destsize < dot-in+1 ? destsize : dot-in+1);

	if ( in == out && destsize > 1 )
		out[destsize - 1] = '\0';
	else
		strncpy(out, in, destsize);
}


static void GetMapData(bfflevel_t *lvl, const std::string& mapfile, const std::string& levelfile,
	const std::vector<std::string>& tilesets)
{
	FILE *fp;
	uint64_t fsize;
	{
		fp = fopen(mapfile.c_str(), "r");
		if (!fp) {
			BFF_Error("WriteBFF: failed to open mapfile %s", mapfile.c_str());
		}
		fseek(fp, 0L, SEEK_END);
		fsize = ftell(fp);
		fseek(fp, 0L, SEEK_SET);

		lvl->tmjBuffer.resize(fsize);
		lvl->mapBufferLen = lvl->tmjBuffer.size() + 1;

		fread(lvl->tmjBuffer.data(), sizeof(char), fsize, fp);
		fclose(fp);
	}

	{
		fp = fopen(levelfile.c_str(), "r");
		if (!fp) {
			BFF_Error("WriteBFF: failed to open levelfile %s", levelfile.c_str());
		}
		fseek(fp, 0L, SEEK_END);
		fsize = ftell(fp);
		fseek(fp, 0L, SEEK_SET);

		lvl->levelBuffer.resize(fsize);
		lvl->levelBufferLen = lvl->levelBuffer.size() + 1;

		fread(lvl->levelBuffer.data(), sizeof(char), fsize, fp);
		fclose(fp);
	}

	lvl->tsjBuffers.reserve(tilesets.size());
	for (const auto& i : tilesets) {
		fp = fopen(i.c_str(), "r");
		if (!fp) {
			BFF_Error("WriteBFF: failed to open tileset file %s", i.c_str());
		}
		fseek(fp, 0L, SEEK_END);
		fsize = ftell(fp);
		fseek(fp, 0L, SEEK_SET);

		lvl->tsjBuffers.emplace_back();
		lvl->tsjBuffers.back().resize(fsize);
		fread(lvl->tsjBuffers.back().data(), sizeof(char), fsize, fp);
		fclose(fp);
	}
	lvl->numTilesets = lvl->tsjBuffers.size();
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
	for (int32_t i = 0; i < info->numLevels; i++) {
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

		if (!data["levels"][levelnode].contains("mapfile")) {
			BFF_Error("WriteBFF: level at index %lu (%s) doesn't contain a mapfile", i, levelnode.c_str());
		}
		if (!data["levels"][levelnode].contains("tilesets")) {
			BFF_Error("WriteBFF: level at index %lu (%s) doesn't contain a list of tilesets", i, levelnode.c_str());
		}
		if (!data["levels"][levelnode].contains("levelfile")) {
			BFF_Error("WriteBFF: level at index %lu (%s) doesn't contain a levelfile", i, levelnode.c_str());
		}
		
		const std::string levelfile = data["levels"][levelnode]["levelfile"];
		const std::string mapfile = data["levels"][levelnode]["mapfile"];
		const std::vector<std::string> tilesets = data["levels"][levelnode]["tilesets"];
		GetMapData(level, mapfile, levelfile, tilesets);
	}
}
