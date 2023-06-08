#include "../src/n_shared.h"
#include "g_bff.h"

static void SafeRead(FILE* fp, void *data, size_t size)
{
	if (fread(data, size, 1, fp) == 0) {
		BFF_Error("SafeRead: failed to read %lu bytes from file, errno: %s", size, strerror(errno));
	}
}

bff_t* BFF_OpenArchive(const std::string& filepath)
{
	FILE* fp = SafeOpen(filepath.c_str(), "rb");

	bff_t* archive = (bff_t *)Hunk_Alloc(sizeof(bff_t), "bffArchive", h_low);
	bffheader_t header;
	SafeRead(fp, &header, sizeof(bffheader_t));
	if (header.ident != BFF_IDENT) {
		BFF_Error("BFF_OpenArchive: file isn't a bff archive");
	}
	if (header.magic != HEADER_MAGIC) {
		BFF_Error("BFF_OpenArchive: header magic number is not correct");
	}
	if (!header.numChunks) {
		BFF_Error("BFF_OpenArchive: bad chunk count");
	}

	SafeRead(fp, archive->bffPathname, sizeof(archive->bffPathname));
	SafeRead(fp, archive->bffGamename, sizeof(archive->bffGamename));

	Con_Printf(
		"<-------- HEADER -------->\n"
		"total chunks: %lu\n",
	header.numChunks);

	archive->numChunks = header.numChunks;
	archive->chunkList = (bff_chunk_t *)Z_Malloc(sizeof(bff_chunk_t) * header.numChunks, TAG_STATIC, &archive->chunkList, "chunkList");

	for (bff_int_t i = 0; i < archive->numChunks; i++) {
		size_t offset = ftell(fp);
		SafeRead(fp, archive->chunkList[i].chunkName, MAX_BFF_CHUNKNAME);
		SafeRead(fp, &archive->chunkList[i].chunkSize, sizeof(bff_int_t));
		SafeRead(fp, &archive->chunkList[i].chunkType, sizeof(bff_int_t));
		Con_Printf(
			"<-------- CHUNK %lu -------->\n"
			"size: %lu\n"
			"type: %i\n"
			"name: %s\n"
			"file offset: %lu\n",
		i, archive->chunkList[i].chunkSize, archive->chunkList[i].chunkType, archive->chunkList[i].chunkName, offset);

		archive->chunkList[i].chunkBuffer = (char *)Hunk_Alloc(archive->chunkList[i].chunkSize, "chunkBuffer", h_low);
		SafeRead(fp, archive->chunkList[i].chunkBuffer, archive->chunkList[i].chunkSize);
	}
	fclose(fp);

	return archive;
}

static void CopyLevelChunk(const bff_chunk_t* chunk, bffinfo_t* info)
{
	bfflevel_t* data;
	const char *ptr;
	
	data = &info->levels[info->numLevels];
	strncpy(data->name, chunk->chunkName, MAX_BFF_CHUNKNAME);
	ptr = chunk->chunkBuffer;
	
	data->numSpawns = *(bff_int_t *)ptr;
	ptr += sizeof(bff_int_t);
	data->numLights = *(bff_int_t *)ptr;
	ptr += sizeof(bff_int_t);
	data->levelNumber = *(bff_int_t *)ptr;
	ptr += sizeof(bff_int_t);
	
	memcpy(data->tilemap, ptr, sizeof(data->tilemap));
	ptr += sizeof(data->tilemap);
	memcpy(data->spawns, ptr, sizeof(data->spawns));
	ptr += sizeof(mapspawn_t) * data->numSpawns;
	memcpy(data->lights, ptr, sizeof(data->lights));
	ptr += sizeof(maplight_t) * data->numLights;

	info->numLevels++;
}

static void CopySoundChunk(const bff_chunk_t* chunk, bffinfo_t* info)
{
	bffsound_t* data;
	const char* ptr;
	
	data = &info->sounds[info->numSounds];
	strncpy(data->name, chunk->chunkName, MAX_BFF_CHUNKNAME);
	Con_Printf("Loading audio chunk %s", data->name);

	ptr = chunk->chunkBuffer;

	data->fileSize = *(bff_int_t *)ptr;
	ptr += sizeof(bff_int_t);
	data->fileType = *(bff_short_t *)ptr;
	ptr += sizeof(bff_short_t);
	data->fileBuffer = (char *)Z_Malloc(data->fileSize, TAG_STATIC, &data->fileBuffer, "sndbuffer");
	
	memcpy(data->fileBuffer, ptr, data->fileSize);
	Con_Printf("done loading audio chunk %s", data->name);
	
	info->numSounds++;
}

static void CopyScriptChunk(const bff_chunk_t* chunk, bffinfo_t* info)
{
	bffscript_t* data;
	const char* ptr;

	data = &info->scripts[info->numScripts];
	strncpy(data->name, chunk->chunkName, MAX_BFF_CHUNKNAME);
	ptr = chunk->chunkBuffer;

	data->codelen = *(bff_int_t *)ptr;
	ptr += sizeof(bff_int_t);
	
	data->bytecode = (uint8_t *)Z_Malloc(sizeof(uint8_t) * data->codelen, TAG_STATIC, &data->bytecode, "QVM_BYTES");
	memcpy(data->bytecode, ptr, data->codelen * sizeof(uint8_t));
	
	info->numScripts++;
}

bffinfo_t* BFF_GetInfo(bff_t* archive)
{
	if (!archive) {
		BFF_Error("BFF_GetInfo: null archive");
	}
	
	bffinfo_t* info = (bffinfo_t *)Hunk_Alloc(sizeof(bffinfo_t), "bffinfo", h_low);
	memset(info, 0, sizeof(bffinfo_t));
	info->numLevels = 0;
	info->numSounds = 0;
	info->numScripts = 0;
	
	for (bff_int_t i = 0; i < archive->numChunks; i++) {
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
}

void BFF_CloseArchive(bff_t* archive)
{
	if (!archive) {
		BFF_Error("BFF_CloseArchive: null archive");
	}
	// cache the chunks
	for (bff_int_t i = 0; i < archive->numChunks; i++) {
		Z_ChangeTag(archive->chunkList[i].chunkBuffer, TAG_CACHE);
	}
	Z_ChangeTag(archive, TAG_CACHE);
}