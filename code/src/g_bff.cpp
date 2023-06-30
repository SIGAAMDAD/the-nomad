#include "n_shared.h"
#include "g_bff.h"
#include "../common/n_vm.h"
#include "g_game.h"
#include "g_sound.h"
#include "m_renderer.h"

static void SafeRead(FILE* fp, void *data, size_t size)
{
	if (fread(data, size, 1, fp) == 0) {
		BFF_Error("SafeRead: failed to read %lu bytes from file, errno: %s", size, strerror(errno));
	}
}

static void CopyLevelChunk(const bff_chunk_t* chunk, bffinfo_t* info)
{
	bfflevel_t* data;
	const char *ptr;
	
	data = &info->levels[Com_GenerateHashValue(chunk->chunkName, MAX_LEVEL_CHUNKS)];
	strncpy(data->name, chunk->chunkName, MAX_BFF_CHUNKNAME);
	ptr = chunk->chunkBuffer;
	Con_Printf("Loading level chunk %s", data->name);
	
	data->numSpawns = *(bff_int_t *)ptr;
	ptr += sizeof(bff_int_t);
	data->numLights = *(bff_int_t *)ptr;
	ptr += sizeof(bff_int_t);
	data->levelNumber = *(bff_int_t *)ptr;
	ptr += sizeof(bff_int_t);
	data->tilesetIndex = *(bff_int_t *)ptr;
	ptr += sizeof(bff_int_t);
		
	memcpy(data->mapBuffer, ptr, data->mapBufferLen);
	ptr += data->mapBufferLen;
	memcpy(data->spawns, ptr, sizeof(data->spawns));
	ptr += sizeof(mapspawn_t) * data->numSpawns;
	memcpy(data->lights, ptr, sizeof(data->lights));
	ptr += sizeof(maplight_t) * data->numLights;

	Con_Printf("Done loading level chunk %s", data->name);

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
	data->fileBuffer = (char *)Z_Malloc(data->fileSize, TAG_STATIC, &data->fileBuffer, "sndfile");
	
	memcpy(data->fileBuffer, ptr, data->fileSize);
	Con_Printf("Done loading audio chunk %s", data->name);
	
	info->numSounds++;
}

static void CopyScriptChunk(const bff_chunk_t* chunk, bffinfo_t* info)
{
	bffscript_t* data;
	const char* ptr;

	data = &info->scripts[Com_GenerateHashValue(chunk->chunkName, MAX_SCRIPT_CHUNKS)];
	strncpy(data->name, chunk->chunkName, MAX_BFF_CHUNKNAME);
	ptr = chunk->chunkBuffer;
	Con_Printf("Loading script chunk %s", data->name);

	data->codelen = chunk->chunkSize;
	
	data->bytecode = (uint8_t *)Z_Malloc(sizeof(uint8_t) * data->codelen, TAG_STATIC, &data->bytecode, "QVM_BYTES");
	memcpy(data->bytecode, ptr, data->codelen * sizeof(uint8_t));
	Con_Printf("Done loading script chunk %s", data->name);
	
	info->numScripts++;
}

static void CopyTextureChunk(const bff_chunk_t* chunk, bffinfo_t* info)
{
	bfftexture_t* data;
	const char* ptr;

	data = &info->textures[info->numTextures];
	strncpy(data->name, chunk->chunkName, MAX_BFF_CHUNKNAME);
	ptr = chunk->chunkBuffer;
	Con_Printf("Loading texture chunk %s", data->name);

	data->fileSize = chunk->chunkSize;

	data->fileBuffer = (unsigned char *)Z_Malloc(data->fileSize, TAG_STATIC, &data->fileBuffer, "texCache");
	memcpy(data->fileBuffer, ptr, data->fileSize);
	Con_Printf("Done loading texture chunk %s", data->name);

	info->numTextures++;
}

static uint64_t infomark;

bffinfo_t* BFF_GetInfo(bff_t* archive)
{
	if (!archive) {
		BFF_Error("BFF_GetInfo: null archive");
	}
	
	infomark = Hunk_HighMark();
	bffinfo_t* info = (bffinfo_t *)Hunk_Alloc(sizeof(bffinfo_t), "BFFinfo", h_low);
	memset(info, 0, sizeof(bffinfo_t));
	info->numLevels = 0;
	info->numSounds = 0;
	info->numScripts = 0;
	info->numTextures = 0;

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
		case TEXTURE_CHUNK:
			CopyTextureChunk(&archive->chunkList[i], info);
			break;
		};
	}
	return info;
}
void BFF_FreeInfo(bffinfo_t* info)
{
	for (uint32_t i = 0; i < info->numSounds; ++i) {
		Z_ChangeTag(info->sounds[i].fileBuffer, TAG_CACHE);
	}
	for (uint32_t i = 0; i < info->numTextures; ++i) {
		Z_ChangeTag(info->textures[i].fileBuffer, TAG_CACHE);
	}
}

static bffinfo_t* bffinfo;


// this makes it so that only a single bff archive can be in ram at a time
static uint64_t bffmark;

bff_t* BFF_OpenArchive(const GDRStr& filepath)
{
    FILE* fp = Sys_FOpen(filepath.c_str(), "rb");
    if (!fp) {
        N_Error("BFF_OpenArchive: failed to open bff %s", filepath.c_str());
    }

	bff_t* archive = (bff_t *)Z_Malloc(sizeof(bff_t), TAG_STATIC, &archive, "BFFfile");
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
	archive->chunkList = (bff_chunk_t *)Z_Malloc(sizeof(bff_chunk_t) * header.numChunks, TAG_STATIC, &archive->chunkList, "BFFchunks");

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

		archive->chunkList[i].chunkBuffer = (char *)Z_Malloc(archive->chunkList[i].chunkSize, TAG_STATIC, &archive->chunkList[i].chunkBuffer,
			"BFFcache");
		SafeRead(fp, archive->chunkList[i].chunkBuffer, archive->chunkList[i].chunkSize);
	}
	fclose(fp);
    return archive;
}

void BFF_CloseArchive(bff_t* archive)
{
	if (!archive) {
		BFF_Error("BFF_CloseArchive: null archive");
	}
	for (bff_int_t i = 0; i < archive->numChunks; i++) {
		Z_ChangeTag(archive->chunkList[i].chunkBuffer, TAG_CACHE);
	}
	Z_ChangeTag(archive->chunkList, TAG_CACHE);
	Z_ChangeTag(archive, TAG_CACHE);
}

bffscript_t *BFF_FetchScript(const char *name)
{
	return &bffinfo->scripts[Com_GenerateHashValue(name, MAX_SCRIPT_CHUNKS)];
}

bfflevel_t *BFF_FetchLevel(const char *name)
{
	return &bffinfo->levels[Com_GenerateHashValue(name, MAX_LEVEL_CHUNKS)];
}

bffinfo_t *BFF_FetchInfo(void)
{
	return bffinfo;
}

void G_LoadBFF(const GDRStr& bffname)
{
	FS_Init();

	file_t archive = FS_OpenBFF(0);
	bffinfo = BFF_GetInfo((bff_t*)FS_GetBFFData(archive));

	FS_FClose(archive);

    VM_Init(bffinfo->scripts);
}