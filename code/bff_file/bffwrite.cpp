#include "g_bff.h"

#ifndef _NOMAD_VERSION

static void GetInfo(const json& data, bffinfo_t* info, const char *path)
{
	if (!data.contains("info")) {
		BFF_Error("WriteBFF: an info json node is required for bff compilation");
	}
	if (strlen(path) > MAX_BFF_PATH - 1) {
		Con_Printf("WARNING: file path for bff output file given is longer than %i characters, truncating", MAX_BFF_PATH - 1);
	}
	strncpy(info->bffPathname, path, MAX_BFF_PATH - 1);

	if (data["info"].contains("name")) {
		const std::string gamename = data["info"]["name"];
		if (gamename.size() > sizeof(info->bffGamename) - 1) {
			Con_Printf("WARNING: bffGamename provided is longer than %lu characters, truncating", sizeof(info->bffGamename) - 1);
		}
		memset(info->bffGamename, 0, sizeof(info->bffGamename));
		strncpy(info->bffGamename, gamename.c_str(), sizeof(info->bffGamename) - 1);
	}
	else {
		Con_Printf("WARNING: no name provided in info section, using default bffGamename value of %s", DEFAULT_BFF_GAMENAME);
        strcpy(info->bffGamename, DEFAULT_BFF_GAMENAME);
	}
}

#ifndef O_BINARY
#define O_BINARY 0
#endif

static void SafeWrite(const void *data, size_t size, std::vector<char>& buffer)
{
	buffer.insert(buffer.end(), (char *)data, (char *)data + size);
}
static void SafeWrite(const char* filepath, const void *data, size_t size)
{
	int handle = open(filepath, O_WRONLY | O_BINARY);
	if (handle == -1) {
		BFF_Error("SafeWrite: failed to open file handle for file %s, errno: %s", filepath, strerror(errno));
	}
	if (write(handle, data, size) == -1) {
		BFF_Error("SafeWrite: failed to write %lu bytes to file %s", size, filepath);
	}
	close(handle);
}
static void SafeWrite(FILE* fp, const void *data, size_t size)
{
	if (fwrite(data, size, 1, fp) == 0) {
		BFF_Error("SafeWrite: failed to write %lu bytes", size);
	}
}

void WriteBFF(const char* outfile, const char* jsonfile)
{
	bff_t* archive;
	FILE* fp;
	
	bffinfo_t* info = (bffinfo_t *)SafeMalloc(sizeof(bffinfo_t));
	
	FILE* jsonfp = SafeOpen(jsonfile, "r");
	json data = json::parse(jsonfp);
	fclose(jsonfp);
	
	GetInfo(data, info, outfile);
	GetLevels(data, info);
	GetScripts(data, info);
	
	archive = (bff_t *)SafeMalloc(sizeof(bff_t), "bffArchive");
	
	uint64_t totalChunks = info->numLevels + info->numSounds + info->numScripts;
	archive->numChunks = totalChunks;
	archive->chunkList = (bff_chunk_t *)SafeMalloc(sizeof(bff_chunk_t) * archive->numChunks);
	bff_chunk_t* chunk = archive->chunkList;

	for (uint64_t i = 0; i < info->numLevels; i++) {
		char *ptr;
		strncpy(chunk->chunkName, info->levels[i].name, MAX_BFF_CHUNKNAME);
		chunk->chunkSize = sizeof(bfflevel_t);
		chunk->chunkType = LEVEL_CHUNK;
		chunk->chunkBuffer = (char *)SafeMalloc(sizeof(bfflevel_t), "levelChunk");
		ptr = chunk->chunkBuffer;
		
		*(uint64_t *)ptr = info->levels[i].numSpawns;
		ptr += sizeof(uint64_t);
		*(uint64_t *)ptr = info->levels[i].numLights;
		ptr += sizeof(uint64_t);
		*(uint64_t *)ptr = info->levels[i].levelNumber;
		ptr += sizeof(uint64_t);
		
		memmove(ptr, info->levels[i].tilemap, sizeof(info->levels[i].tilemap));
		ptr += sizeof(info->levels[i].tilemap);
		memmove(ptr, info->levels[i].spawns, sizeof(mapspawn_t) * info->levels[i].numSpawns);
		ptr += sizeof(mapspawn_t) * info->levels[i].numSpawns;
		memmove(ptr, info->levels[i].lights, sizeof(maplight_t) * info->levels[i].numLights);
		ptr += sizeof(maplight_t) * info->levels[i].numLights;
		
		chunk++;
	}
	for (uint64_t i = 0; i < info->numSounds; i++) {
		char *ptr;
		strncpy(chunk->chunkName, info->sounds[i].name, MAX_BFF_CHUNKNAME);
		chunk->chunkSize = sizeof(uint64_t) + sizeof(uint16_t) + info->sounds[i].filesize;
		chunk->chunkBuffer = (char *)SafeMalloc(chunk->chunkSize);
		ptr = chunk->chunkBuffer;
		
		*(uint64_t *)ptr = info->sounds[i].filesize;
		ptr += sizeof(uint64_t);
		*(uint16_t *)ptr = info->sounds[i].filetype;
		ptr += sizeof(uint16_t);
		
		memmove(ptr, info->sounds[i].filebuffer, info->sounds[i].filesize);
		free(info->sounds[i].filebuffer);
		
		chunk++;
	}
	for (uint64_t i = 0; i < info->numScripts; i++) {
		char *ptr;
		uint64_t levelIndex, musicIndex;
		strncpy(chunk->chunkName, info->scripts[i].name, MAX_BFF_CHUNKNAME);
		chunk->chunkSize = sizeof(uint64_t) * 2 + info->scripts[i].codelen * sizeof(uint8_t);
		chunk->chunkType = SCRIPT_CHUNK;
		chunk->chunkBuffer = (char *)SafeMalloc(chunk->chunkSize, "scriptChunk");
		ptr = chunk->chunkBuffer;
		
		for (uint64_t s = 0; s < info->numSounds; s++) {
			if (info->scripts[i].music == &info->sounds[s]) {
				musicIndex = s;
				break;
			}
		}
		for (uint64_t s = 0; s < info->numLevels; s++) {
			if (info->scripts[i].level == &info->levels[s]) {
				levelIndex = s;
				break;
			}
		}
		
		*(uint64_t *)ptr = levelIndex;
		ptr += sizeof(uint64_t);
		*(uint64_t *)ptr = musicIndex;
		ptr += sizeof(uint64_t);
		*(uint64_t *)ptr = info->scripts[i].codelen;
		ptr += sizeof(uint64_t);
		
		Con_Printf("qvm bytecode length: %lu", info->scripts[i].codelen);
		memmove(ptr, info->scripts[i].bytecode, sizeof(uint8_t) * info->scripts[i].codelen);

		chunk++;
	}
	
	bffheader_t header = {
		.ident = BFF_IDENT,
		.magic = HEADER_MAGIC,
		.numChunks = archive->numChunks,
		.fileDecompressedSize = 0,
		.fileCompressedSize = 0,
	};

	fp = SafeOpen(outfile, "wb");
	
	SafeWrite(fp, &header, sizeof(bffheader_t));
	SafeWrite(fp, archive->bffPathname, sizeof(archive->bffPathname));
	SafeWrite(fp, archive->bffGamename, sizeof(archive->bffGamename));
	for (uint32_t i = 0; i < archive->numChunks; i++) {
		size_t offset = ftell(fp);
		Con_Printf(
			"<-------- CHUNK %lu -------->\n"
			"size: %lu\n"
			"type: %i\n"
			"name: %s\n"
			"file offset: %lu\n",
		i, archive->chunkList[i].chunkSize, archive->chunkList[i].chunkType, archive->chunkList[i].chunkName, offset);
		SafeWrite(fp, archive->chunkList[i].chunkName, MAX_BFF_CHUNKNAME);
		SafeWrite(fp, &archive->chunkList[i].chunkSize, sizeof(uint64_t));
		SafeWrite(fp, &archive->chunkList[i].chunkType, sizeof(uint32_t));
		SafeWrite(fp, archive->chunkList[i].chunkBuffer, sizeof(char) * archive->chunkList[i].chunkSize);
	}
	fclose(fp);
}

#endif
