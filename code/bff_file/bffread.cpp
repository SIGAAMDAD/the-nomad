#include "../src/n_shared.h"
#include "g_bff.h"
#include "../src/g_zone.h"

bff_t* BFF_OpenArchive(const eastl::string& filepath)
{
	if (filepath.size() > MAX_BFF_PATH) {
		BFF_Error("BFF_OpenArchive: filepath character length exceeds maximum filepath length of %i bytes", (int)MAX_BFF_PATH);
	}
	FILE* fp = SafeOpen(filepath.c_str(), "rb");
	
	bff_t* archive = (bff_t *)SafeMalloc(sizeof(bff_t), "bffArchive");
	bffheader_t header;
	
	SafeRead(&header, sizeof(bffheader_t), 1, fp);
	if (header.ident != BFF_IDENT) {
		BFF_Error("BFF_OpenArchive: file isn't a bff archive");
	}
	if (header.magic != HEADER_MAGIC) {
		BFF_Error("BFF_OpenArchive: header magic number is not correct");
	}
	if (!header.numChunks) {
		BFF_Error("BFF_OpenArchive: bad chunk count");
	}
	
	archive->numChunks = header.numChunks;
	archive->chunkList = (bff_chunk_t *)SafeMalloc(sizeof(bff_chunk_t) * archive->numChunks);
	
	// register all the chunks
	for (uint64_t i = 0; i < archive->numChunks; i++) {
		bff_chunk_t* chunk = &archive->chunkList[i];
		
		SafeRead(&chunk->chunkSize, sizeof(uint64_t), 1, fp);
		SafeRead(&chunk->chunkType, sizeof(uint32_t), 1, fp);
		
		chunk->chunkBuffer = (char *)SafeMalloc(chunk->chunkSize);
		
		SafeRead(chunk->chunkBuffer, sizeof(char), chunk->chunkSize, fp);
	}
	fclose(fp);
	
	return archive;
}

static void CopyLevelChunk(const bff_chunk_t* chunk, bffinfo_t* info)
{
	bfflevel_t* data;
	const char* ptr;
	
	data = &info->levels[info->numLevels];
	memset(data->spawns, 0, sizeof(data->spawns));
	memset(data->lights, 0, sizeof(data->lights));
	
	ptr = (const char *)chunk->chunkBuffer;
	data->numSpawns = *(uint64_t *)ptr;
	ptr += sizeof(uint64_t);
	data->numLights = *(uint64_t *)ptr;
	ptr += sizeof(uint64_t);
	data->levelNumber = *(uint64_t *)ptr;
	ptr += sizeof(uint64_t);
	
	memcpy(data->tilemap, ptr, sizeof(data->tilemap));
	ptr += sizeof(data->tilemap);
	memcpy(data->spawns, ptr, data->numSpawns * sizeof(mapspawn_t));
	ptr += data->numSpawns * sizeof(mapspawn_t);
	memcpy(data->lights, ptr, data->numLights * sizeof(maplight_t));
	ptr += data->numLights * sizeof(maplight_t);
	
	info->numLevels++;
}

static void CopySoundChunk(const bff_chunk_t* chunk, bffinfo_t* info)
{
	bffsound_t* data;
	const char* ptr;
	
	data = &info->sounds[info->numSounds];
	ptr = (const char *)chunk->chunkBuffer;
	data->filesize = *(uint64_t *)ptr;
	ptr += sizeof(uint64_t);
	data->filetype = *(uint16_t *)ptr;
	ptr += sizeof(uint16_t);
	data->filebuffer = (char *)SafeMalloc(data->filesize); // doesn't need zone heap
	
	memcpy(data->filebuffer, ptr, data->filesize);
	
	info->numSounds++;
}

static void CopyScriptChunk(const bff_chunk_t* chunk, bffinfo_t* info)
{
	bffscript_t* data;
	const char* ptr;
	uint64_t levelIndex;
	uint64_t musicIndex;
	
	data = &info->scripts[info->numScripts];
	ptr = (const char *)chunk->chunkBuffer;
	levelIndex = *(uint64_t *)ptr;
	ptr += sizeof(uint64_t);
	musicIndex = *(uint64_t *)ptr;
	ptr += sizeof(uint64_t);
	data->codelen = *(uint64_t *)ptr;
	ptr += sizeof(uint64_t);
	
	data->level = &info->levels[levelIndex];
	data->music = &info->sounds[musicIndex];
	data->bytecode = (uint8_t *)Z_Malloc(sizeof(uint8_t) * data->codelen, TAG_STATIC, &data->bytecode, "QVM_BYTES");
	
	memcpy(data->bytecode, ptr, sizeof(uint8_t) * data->codelen);
	
	info->numScripts++;
}

bffinfo_t* BFF_GetInfo(bff_t* archive)
{
	if (!archive) {
		BFF_Error("BFF_SortArchive: null archive");
	}
	
	bffinfo_t* info = (bffinfo_t *)Z_Malloc(sizeof(bffinfo_t), TAG_STATIC, &info, "BFFinfo");
	memset(info->levels, 0, sizeof(info->levels));
	memset(info->sounds, 0, sizeof(info->sounds));
	memset(info->scripts, 0, sizeof(info->scripts));
	info->numLevels = 0;
	info->numSounds = 0;
	info->numScripts = 0;
	
	for (uint64_t i = 0; i < archive->numChunks; i++) {
		switch (archive->chunkList[i].chunkType) {
		case LEVEL_CHUNK:
			CopyLevelChunk(&archive->chunkList[i], info);
			break;
		case SOUND_CHUNK:
			CopySoundChunk(&archive->chunkList[i], info);
			break;
		case SCRIPT_CHUNK:
			CopyScriptChunk(&archive->chunkList[i], info);
			break;
		};
	}
	return info;
}
void BFF_FreeInfo(bffinfo_t* info)
{
	for (uint64_t i = 0; i < info->numScripts; i++) {
		Z_ChangeTag(info->scripts[i].bytecode, TAG_CACHE);
	}
	Z_ChangeTag(info, TAG_CACHE);	
}

void BFF_CloseArchive(bff_t* archive)
{
	for (uint64_t i = 0; i < archive->numChunks; i++) {
		free(archive->chunkList[i].chunkBuffer);
	}
	free(archive);
}

#ifndef _NOMAD_VERSION
void DecompileBFF(const char *filepath)
{
	FILE* fp = SafeOpen(filepath, "rb");

	bff_t* archive = (bff_t *)SafeMalloc(sizeof(bff_t), "bffArchive");
	bffheader_t header;
	SafeRead(&header, sizeof(bffheader_t), 1, fp);
	if (header.ident != BFF_IDENT) {
		BFF_Error("DecompileBFF: file isn't a bff archive");
	}
	if (header.magic != HEADER_MAGIC) {
		BFF_Error("DecompileBFF: header magic number is not correct");
	}
	if (!header.numChunks) {
		BFF_Error("DecompileBFF: bad chunk count");
	}

	SafeRead(archive->bffPathname, sizeof(archive->bffPathname), 1, fp);
	SafeRead(archive->bffGamename, sizeof(archive->bffGamename), 1, fp);

	Con_Printf(
		"<-------- HEADER -------->\n"
		"total chunks: %lu\n",
	header.numChunks);

	archive->numChunks = header.numChunks;
	archive->chunkList = (bff_chunk_t *)SafeMalloc(sizeof(bff_chunk_t) * header.numChunks, "chunkList");

	for (uint64_t i = 0; i < archive->numChunks; i++) {
		size_t offset = ftell(fp);
		SafeRead(archive->chunkList[i].chunkName, sizeof(char), MAX_BFF_CHUNKNAME, fp);
		SafeRead(&archive->chunkList[i].chunkSize, sizeof(uint64_t), 1, fp);
		SafeRead(&archive->chunkList[i].chunkType, sizeof(uint32_t), 1, fp);
		Con_Printf(
			"<-------- CHUNK %lu -------->\n"
			"size: %lu\n"
			"type: %i\n"
			"name: %s\n"
			"file offset: %lu\n",
		i, archive->chunkList[i].chunkSize, archive->chunkList[i].chunkType, archive->chunkList[i].chunkName, offset);

		archive->chunkList[i].chunkBuffer = (char *)SafeMalloc(archive->chunkList[i].chunkSize, "chunkBuffer");
		SafeRead(archive->chunkList[i].chunkBuffer, sizeof(char), archive->chunkList[i].chunkSize, fp);
	}
	fclose(fp);

	for (uint64_t i = 0; i < archive->numChunks; i++) {
		free(archive->chunkList[i].chunkBuffer);
	}
	free(archive->chunkList);
	free(archive);
}
#endif
