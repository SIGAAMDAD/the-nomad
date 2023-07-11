#include "n_shared.h"
#include "g_bff.h"
#include "../common/n_vm.h"
#include "g_game.h"
#include "g_sound.h"
#include "m_renderer.h"
#include <zlib.h>
#include <bzlib.h>

static void SafeRead(FILE* fp, void *data, size_t size)
{
	if (fread(data, size, 1, fp) == 0) {
		BFF_Error("SafeRead: failed to read %lu bytes from file, errno: %s", size, strerror(errno));
	}
}

template<typename CharT, typename Fn, typename... Args, typename Error>
inline const CharT* Decompress(Error errCheck, CharT **out, CharT *in, uint64_t inlen, uint64_t *outlen, Fn&& fn, Args&&... args)
{
	uint64_t old = *outlen;
	int ret = fn(std::forward<Args>(args)...);
	errCheck(ret);

	if (*outlen != old) {
		void *tmp = Mem_Alloc(*outlen);
		memcpy(tmp, *out, *outlen);
		Mem_Free(*out);
		*out = (CharT *)tmp;
	}

	return *out;
}

template<typename CharT>
inline const CharT* Decompress_BZIP2(CharT *in, uint64_t inlen, uint64_t *outlen)
{
	CharT *out = (CharT *)Mem_Alloc(*outlen);
	return Decompress(
		[=](int ret) {
			if (ret != BZ_OK)
				BFF_Error("Compress_BZIP2: bzip2 failed to compress buffer of size %lu bytes! id: %i", inlen, ret);
		}
	, &out, in, inlen, outlen, BZ2_bzBuffToBuffDecompress, (char *)out, (unsigned int *)outlen, (char *)in, inlen, 0, 1);
}

template<typename CharT>
inline const CharT* Decompress_ZLIB(CharT *in, uint64_t inlen, uint64_t *outlen)
{
	CharT *out = (CharT *)Mem_Alloc(*outlen);
	return Decompress(
		[=](int ret) {
			if (ret != Z_OK)
				BFF_Error("Decompress_ZLIB: zlib failed to decompress buffer of size %lu bytes! id: %i", inlen, ret);
		}
	, &out, in, inlen, outlen, uncompress, (Bytef *)out, (uLongf *)outlen, (const Bytef *)in, (uLong)inlen);
}

static void CopyLevelChunk(const bff_chunk_t* chunk, bffinfo_t* info)
{
	bfflevel_t* data;
	const char *ptr;
	
	data = &info->levels[Com_GenerateHashValue(chunk->chunkName, MAX_LEVEL_CHUNKS)];
	strncpy(data->name, chunk->chunkName, MAX_BFF_CHUNKNAME);
	ptr = chunk->chunkBuffer;
	Con_Printf("Loading level chunk %s", data->name);
	
	data->levelNumber = *(int32_t *)ptr;
	ptr += sizeof(int32_t);
	data->numTilesets = *(int32_t *)ptr;
	ptr += sizeof(int32_t);
	data->mapBufferLen = *(int32_t *)ptr;
	ptr += sizeof(int32_t);
	
	int32_t buflen = *(int32_t *)ptr;
	uint64_t mapBufferLen = data->mapBufferLen;
	const char *tmp;
	switch (info->compression) {
	case COMPRESSION_BZIP2:
		tmp = Decompress_BZIP2((char *)ptr, buflen, &mapBufferLen);
		break;
	case COMPRESSION_ZLIB:
		tmp = Decompress_ZLIB((char *)ptr, buflen, &mapBufferLen);
		break;
	case COMPRESSION_NONE:
		tmp = ptr;
		break;
	};

	data->mapBufferLen = mapBufferLen;
	data->tmjBuffer = (char *)Z_Malloc(data->mapBufferLen, TAG_STATIC, &data->tmjBuffer, "TMJbuffer");
	memcpy(data->tmjBuffer, tmp, data->mapBufferLen);
	ptr += data->mapBufferLen;

	data->tsjBuffers = (char **)Z_Malloc(sizeof(char *) * data->numTilesets, TAG_STATIC, &data->tsjBuffers, "TSJbuffers");
	for (int32_t i = 0; i < data->numTilesets; i++) {
		int32_t real_size = *(int32_t *)ptr;
		ptr += sizeof(int32_t);
		int32_t bufferLen = *(int32_t *)ptr;
		ptr += sizeof(int32_t);

		const char *tmp;
		uint64_t mapBufferLen = real_size;
		switch (info->compression) {
		case COMPRESSION_BZIP2:
			tmp = Decompress_BZIP2((char *)ptr, buflen, &mapBufferLen);
			break;
		case COMPRESSION_ZLIB:
			tmp = Decompress_ZLIB((char *)ptr, buflen, &mapBufferLen);
			break;
		case COMPRESSION_NONE:
			tmp = ptr;
			break;
		};

		real_size = mapBufferLen;
		data->tsjBuffers[i] = (char *)Z_Malloc(real_size, TAG_STATIC, &data->tsjBuffers[i], "TSJbuffer");
		memcpy(data->tsjBuffers[i], tmp, real_size);
		ptr += bufferLen;
	}
	
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

	data->fileSize = *(int32_t *)ptr;
	ptr += sizeof(int32_t);
	data->fileType = *(int32_t *)ptr;
	ptr += sizeof(int32_t);
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
	
	// kept on the hunk so that restarting a vm won't be impossible if the zone starts binging and purging cached blocks
	data->bytecode = (uint8_t *)Hunk_Alloc(sizeof(uint8_t) * data->codelen, "QVM_BYTES", h_low); 
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
	info->compression = archive->header.compression;

	for (int32_t i = 0; i < archive->numChunks; i++) {
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
	for (uint32_t i = 0; i < info->numSounds; ++i)
		Z_ChangeTag(info->sounds[i].fileBuffer, TAG_CACHE);
	for (uint32_t i = 0; i < info->numTextures; ++i)
		Z_ChangeTag(info->textures[i].fileBuffer, TAG_CACHE);
	for (uint32_t i = 0; i < info->numLevels; ++i) {
		Z_ChangeTag(info->levels[i].tmjBuffer, TAG_CACHE);
		for (uint32_t t = 0; t < info->levels[i].numTilesets; ++t)
			Z_ChangeTag(info->levels[i].tsjBuffers[t], TAG_CACHE);
	}
}

static bffinfo_t* bffinfo;

static inline const char *BFF_CompressionString(int compression)
{
	switch (compression) {
	case COMPRESSION_BZIP2: return "bzip2";
	case COMPRESSION_ZLIB: return "zlib";
	};
	return "None";
}

// this makes it so that only a single bff archive can be in ram at a time
static uint64_t bffmark;

bff_t* BFF_OpenArchive(const GDRStr& filepath)
{
    FILE* fp = Sys_FOpen(filepath.c_str(), "rb");
    if (!fp) {
        N_Error("BFF_OpenArchive: failed to open bff %s", filepath.c_str());
    }

	bff_t* archive = (bff_t *)Z_Malloc(sizeof(bff_t), TAG_STATIC, &archive, "BFFfile");
	SafeRead(fp, &archive->header, sizeof(bffheader_t));

	bffheader_t& header = archive->header;
	header.ident = LittleInt(header.ident);
	header.magic = LittleInt(header.magic);
	header.numChunks = LittleInt(header.numChunks);
	header.version = LittleShort(header.version);
	if (header.ident != BFF_IDENT) {
		N_Error("BFF_OpenArchive: file isn't a bff archive");
	}
	if (header.magic != HEADER_MAGIC) {
		N_Error("BFF_OpenArchive: header magic number is not correct");
	}
	if (!header.numChunks) {
		N_Error("BFF_OpenArchive: bad chunk count");
	}
	if (header.version != BFF_VERSION) {
		Con_Printf("========== WARNING: version in bff file %s isn't a supported version ==========", filepath.c_str());
	}

	SafeRead(fp, archive->bffPathname, sizeof(archive->bffPathname));
	SafeRead(fp, archive->bffGamename, sizeof(archive->bffGamename));

	Con_Printf(
		"<-------- HEADER -------->\n"
		"total chunks: %lu\n"
		"compression: %s\n"
		"version: %hu\n",
	header.numChunks, BFF_CompressionString(header.compression), header.version);

	archive->numChunks = header.numChunks;
	archive->chunkList = (bff_chunk_t *)Z_Malloc(sizeof(bff_chunk_t) * header.numChunks, TAG_STATIC, &archive->chunkList, "BFFchunks");

	for (int32_t i = 0; i < archive->numChunks; i++) {
		size_t offset = ftell(fp);

		SafeRead(fp, archive->chunkList[i].chunkName, MAX_BFF_CHUNKNAME);
		SafeRead(fp, &archive->chunkList[i].chunkSize, sizeof(int32_t));
		SafeRead(fp, &archive->chunkList[i].chunkType, sizeof(int32_t));

		archive->chunkList[i].chunkSize = LittleInt(archive->chunkList[i].chunkSize);
		archive->chunkList[i].chunkType = LittleInt(archive->chunkList[i].chunkType);

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
	for (int32_t i = 0; i < archive->numChunks; i++) {
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

bfftexture_t *BFF_FetchTexture(const char *name)
{
	return &bffinfo->textures[Com_GenerateHashValue(name, MAX_TEXTURE_CHUNKS)];

}

bffinfo_t *BFF_FetchInfo(void)
{
	return bffinfo;
}

const eastl::vector<const bfftexture_t*>& BFF_OrderTextures(const bffinfo_t *info)
{
	static eastl::vector<const bfftexture_t*> textures;
	textures.reserve(info->numTextures);

	for (uint32_t i = 0; i < info->numTextures; ++i) {
		if (info->textures[i].fileBuffer)
			textures.emplace_back(&info->textures[i]);
	}
	return textures;
}

const eastl::vector<const bfflevel_t*>& BFF_OrderLevels(const bffinfo_t *info)
[
	static eastl::vector<const bfflevel_t*> levels;
	levels.reserve(info->numLevels);

	for (uint32_t i = 0; i < info->numLevels; ++i) {
		if (strlen(info->levels[i].name)) // name wont be empty if its a valid level
			levels.emplace_back(&info->levels[i]);
	}
	return levels;
]


void G_LoadBFF(const GDRStr& bffname)
{
	FS_Init();

	file_t archive = FS_OpenBFF(0);
	bffinfo = BFF_GetInfo((bff_t*)FS_GetBFFData(archive));

	FS_FClose(archive);

    VM_Init(bffinfo->scripts);
}