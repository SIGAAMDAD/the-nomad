#include "../bff_file/g_bff.h"
#define LOG_WARN(...) Con_Printf(__VA_ARGS__)
#define N_Error BFF_Error
#include "zone.h"


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
	Z_Init();
	bff_t* archive;
	FILE* fp;
	
	bffinfo_t* info = (bffinfo_t *)Z_Malloc(sizeof(bffinfo_t), TAG_STATIC, &info, "info");
	
	FILE* jsonfp = SafeOpen(jsonfile, "r");
	json data = json::parse(jsonfp);
	fclose(jsonfp);
	
	GetInfo(data, info, outfile);
	GetLevels(data, info);
	GetScripts(data, info);
	GetSounds(data, info);
	
	archive = (bff_t *)Z_Malloc(sizeof(bff_t), TAG_STATIC, &archive, "archive");
	
	bff_int_t totalChunks = info->numLevels + info->numSounds + info->numScripts;
	archive->numChunks = totalChunks;
	archive->chunkList = (bff_chunk_t *)Z_Malloc(sizeof(bff_chunk_t) * archive->numChunks, TAG_STATIC, &archive->chunkList, "chunkList");
	bff_chunk_t* chunk = archive->chunkList;

	for (bff_int_t i = 0; i < info->numLevels; i++) {
		char *ptr;
		strncpy(chunk->chunkName, info->levels[i].name, MAX_BFF_CHUNKNAME);
		chunk->chunkSize = sizeof(bfflevel_t);
		chunk->chunkType = LEVEL_CHUNK;
		chunk->chunkBuffer = (char *)Z_Malloc(sizeof(bfflevel_t), TAG_STATIC, &chunk->chunkBuffer, "levelChunk");
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
		chunk->chunkSize = sizeof(bff_int_t) + info->textures[i].fileSize;
		chunk->chunkBuffer = (char *)Z_Malloc(chunk->chunkSize, TAG_STATIC, &chunk->chunkBuffer, "texChunk");
		ptr = chunk->chunkBuffer;
		chunk->chunkType = TEXTURE_CHUNK;

		*(bff_int_t *)ptr = info->textures[i].fileSize;
		ptr += sizeof(bff_int_t);

		memcpy(ptr, info->textures[i].fileBuffer, info->textures[i].fileSize);
		Z_ChangeTag(info->textures[i].fileBuffer, TAG_PURGELEVEL);

		chunk++;
	}
	for (bff_int_t i = 0; i < info->numSounds; i++) {
		char *ptr;
		strncpy(chunk->chunkName, info->sounds[i].name, MAX_BFF_CHUNKNAME);
		chunk->chunkSize = sizeof(bff_int_t) + sizeof(bff_short_t) + info->sounds[i].fileSize;
		chunk->chunkBuffer = (char *)Z_Malloc(chunk->chunkSize, TAG_STATIC, &chunk->chunkBuffer, "soundChunk");
		ptr = chunk->chunkBuffer;
		chunk->chunkType = SOUND_CHUNK;
		
		*(bff_int_t *)ptr = info->sounds[i].fileSize;
		ptr += sizeof(bff_int_t);
		*(bff_short_t *)ptr = info->sounds[i].fileType;
		ptr += sizeof(bff_short_t);
		
		memcpy(ptr, info->sounds[i].fileBuffer, info->sounds[i].fileSize);
		Z_ChangeTag(info->sounds[i].fileBuffer, TAG_PURGELEVEL);
		
		chunk++;
	}
	for (bff_int_t i = 0; i < info->numScripts; i++) {
		char *ptr;
		strncpy(chunk->chunkName, info->scripts[i].name, MAX_BFF_CHUNKNAME);
		chunk->chunkSize = sizeof(bff_int_t) + (info->scripts[i].codelen * sizeof(uint8_t));
		chunk->chunkType = SCRIPT_CHUNK;
		chunk->chunkBuffer = (char *)Z_Malloc(chunk->chunkSize, TAG_STATIC, &chunk->chunkBuffer, "scriptChunk");
		ptr = chunk->chunkBuffer;
		
		*(bff_int_t *)ptr = info->scripts[i].codelen;
		ptr += sizeof(bff_int_t);
		
		Con_Printf("qvm bytecode length: %lu", info->scripts[i].codelen);
		memcpy(ptr, info->scripts[i].bytecode, sizeof(uint8_t) * info->scripts[i].codelen);
		Z_ChangeTag(info->scripts[i].bytecode, TAG_PURGELEVEL);

		chunk++;
	}
	Z_ChangeTag(info, TAG_PURGELEVEL);
	
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
		SafeWrite(fp, archive->chunkList[i].chunkBuffer, sizeof(char) * archive->chunkList[i].chunkSize);
		Z_ChangeTag(archive->chunkList[i].chunkBuffer, TAG_PURGELEVEL);
	}
	fclose(fp);

	Z_ChangeTag(archive->chunkList, TAG_PURGELEVEL);
	Z_ChangeTag(archive, TAG_PURGELEVEL);
}
