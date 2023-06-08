#include "../bff_file/g_bff.h"
#define LOG_WARN(...) Con_Printf(__VA_ARGS__)
#define N_Error BFF_Error
#include "zone.h"
#include <bzlib.h>
#include <zlib.h>


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
	}
}

#ifndef O_BINARY
#define O_BINARY 0
#endif

#define BUFFER_SIZE (4*1024)

void SafeWrite(const void *data, size_t size, std::vector<char>& buffer)
{
	buffer.insert(buffer.end(), (char *)data, (char *)data + size);
}
void SafeWrite(FILE *fp, const void *data, size_t size)
{
	if (fwrite(data, size, 1, fp) == 0) {
		BFF_Error("WriteBFF: failed to write %lu bytes to file", size);
	}
}
void SafeWrite(char *buf_p, const void *data, size_t size)
{
	memcpy(buf_p, data, size);
	buf_p += size;
}

void CopyBuffer(char *bufferPtr, const void *data, bff_int_t size)
{
	const char *d = (const char *)data;
	while (size--) {
		*bufferPtr++ = *d++ ^ MAGIC_XOR;
	}
}

void WriteBFF(const char* outfile, const char* jsonfile)
{
//	Z_Init();
	bff_t* archive;
	FILE* fp;
	
	bffinfo_t* info = (bffinfo_t *)SafeMalloc(sizeof(bffinfo_t), "info");
	
	FILE* jsonfp = SafeOpen(jsonfile, "r");
	json data = json::parse(jsonfp);
	fclose(jsonfp);
	
	GetInfo(data, info, outfile);
	GetLevels(data, info);
	GetScripts(data, info);
	GetSounds(data, info);
	GetTextures(data, info);
	
	archive = (bff_t *)SafeMalloc(sizeof(bff_t), "archive");
	
	bff_int_t totalChunks = info->numLevels + info->numSounds + info->numScripts + info->numTextures;
	archive->numChunks = totalChunks;
	archive->chunkList = (bff_chunk_t *)SafeMalloc(sizeof(bff_chunk_t) * archive->numChunks, "chunkList");
	bff_chunk_t* chunk = archive->chunkList;

	for (bff_int_t i = 0; i < info->numLevels; i++) {
		char *ptr;
		strncpy(chunk->chunkName, info->levels[i].name, MAX_BFF_CHUNKNAME);
		chunk->chunkSize = sizeof(bfflevel_t);
		chunk->chunkType = LEVEL_CHUNK;
		chunk->chunkBuffer = (char *)SafeMalloc(sizeof(bfflevel_t), "levelChunk");
		ptr = chunk->chunkBuffer;
		
		*(bff_int_t *)ptr = info->levels[i].numSpawns;
		ptr += sizeof(bff_int_t);
		*(bff_int_t *)ptr = info->levels[i].numLights;
		ptr += sizeof(bff_int_t);
		*(bff_int_t *)ptr = info->levels[i].levelNumber;
		ptr += sizeof(bff_int_t);
		
		memcpy(ptr, info->levels[i].tilemap, sizeof(info->levels[i].tilemap));
		ptr += sizeof(info->levels[i].tilemap);
		memcpy(ptr, info->levels[i].spawns, sizeof(mapspawn_t) * info->levels[i].numSpawns);
		ptr += sizeof(mapspawn_t) * info->levels[i].numSpawns;
		memcpy(ptr, info->levels[i].lights, sizeof(maplight_t) * info->levels[i].numLights);
		ptr += sizeof(maplight_t) * info->levels[i].numLights;
		
		chunk++;
	}
	for (bff_int_t i = 0; i < info->numTextures; i++) {
		char *ptr;
		strncpy(chunk->chunkName, info->textures[i].name, MAX_BFF_CHUNKNAME);
		chunk->chunkSize = info->textures[i].fileSize;
		chunk->chunkBuffer = (char *)SafeMalloc(chunk->chunkSize, "texChunk");
		ptr = chunk->chunkBuffer;
		chunk->chunkType = TEXTURE_CHUNK;

		memcpy(ptr, info->textures[i].fileBuffer, info->textures[i].fileSize);
		free(info->textures[i].fileBuffer);

		chunk++;
	}
	for (bff_int_t i = 0; i < info->numSounds; i++) {
		char *ptr;
		strncpy(chunk->chunkName, info->sounds[i].name, MAX_BFF_CHUNKNAME);
		chunk->chunkSize = sizeof(bff_int_t) + sizeof(bff_short_t) + info->sounds[i].fileSize;
		chunk->chunkBuffer = (char *)SafeMalloc(chunk->chunkSize, "soundChunk");
		ptr = chunk->chunkBuffer;
		chunk->chunkType = SOUND_CHUNK;
		
		*(bff_int_t *)ptr = info->sounds[i].fileSize;
		ptr += sizeof(bff_int_t);
		*(bff_short_t *)ptr = info->sounds[i].fileType;
		ptr += sizeof(bff_short_t);
		
		memcpy(ptr, info->sounds[i].fileBuffer, info->sounds[i].fileSize);
		free(info->sounds[i].fileBuffer);
		
		chunk++;
	}
	for (bff_int_t i = 0; i < info->numScripts; i++) {
		char *ptr;
		strncpy(chunk->chunkName, info->scripts[i].name, MAX_BFF_CHUNKNAME);
		chunk->chunkSize = (info->scripts[i].codelen * sizeof(uint8_t));
		chunk->chunkType = SCRIPT_CHUNK;
		chunk->chunkBuffer = (char *)SafeMalloc(chunk->chunkSize, "scriptChunk");
		ptr = chunk->chunkBuffer;

		Con_Printf("qvm bytecode length: %lu", info->scripts[i].codelen);
		memcpy(ptr, info->scripts[i].bytecode, sizeof(uint8_t) * info->scripts[i].codelen);
		free(info->scripts[i].bytecode);

		chunk++;
	}
	free(info);
	
	bffheader_t header = {
		.ident = BFF_IDENT,
		.magic = HEADER_MAGIC,
		.numChunks = archive->numChunks
	};

	fp = SafeOpen(outfile, "wb");
	SafeWrite(fp, &header, sizeof(bffheader_t));
	SafeWrite(fp, archive->bffPathname, sizeof(archive->bffPathname));
	SafeWrite(fp, archive->bffGamename, sizeof(archive->bffGamename));
	for (bff_int_t i = 0; i < archive->numChunks; i++) {
		size_t offset = ftell(fp);
		Con_Printf(
			"<-------- CHUNK %lu -------->\n"
			"size: %lu\n"
			"type: %i\n"
			"name: %s\n"
			"file offset: %lu\n",
		i, archive->chunkList[i].chunkSize, archive->chunkList[i].chunkType, archive->chunkList[i].chunkName, offset);
		SafeWrite(fp, archive->chunkList[i].chunkName, MAX_BFF_CHUNKNAME);
		SafeWrite(fp, &archive->chunkList[i].chunkSize, sizeof(bff_int_t));
		SafeWrite(fp, &archive->chunkList[i].chunkType, sizeof(bff_int_t));
		SafeWrite(fp, archive->chunkList[i].chunkBuffer, archive->chunkList[i].chunkSize);
//		free(archive->chunkList[i].chunkBuffer);
	}
	fclose(fp);
	exit(1);
}