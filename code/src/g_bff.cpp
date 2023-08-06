#include "n_shared.h"
#include "g_bff.h"
#include "../common/n_vm.h"
#include "g_game.h"
#include "g_sound.h"
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

static void LVL_LoadTMJBuffer(bfflevel_t *lvl, char *ptr, int64_t buflen, int compression)
{
	const char *tmp;
	char *buffer;
	uint64_t mapBufferLen;

	mapBufferLen = lvl->mapBufferLen;

	buffer = (char *)Z_Malloc(mapBufferLen, TAG_STATIC, &buffer, "tmp");
	// first read the compressed buffer
	memcpy(buffer, ptr, mapBufferLen);
//	FS_Read(buffer, buflen, fd);
	ptr += mapBufferLen;
	switch (compression) {
	case COMPRESSION_BZIP2:
		tmp = Decompress_BZIP2(buffer, buflen, &mapBufferLen);
		break;
	case COMPRESSION_ZLIB:
		tmp = Decompress_ZLIB(buffer, buflen, &mapBufferLen);
		break;
	case COMPRESSION_NONE:
		tmp = buffer;
		break;
	};
	lvl->mapBufferLen = mapBufferLen;

	lvl->tmjBuffer = (char *)Z_Malloc(lvl->mapBufferLen, TAG_STATIC, &lvl->tmjBuffer, "TMJbuffer");
	memcpy(lvl->tmjBuffer, tmp, lvl->mapBufferLen);
	if (tmp != buffer)
		Mem_Free((void *)tmp);

	Z_ChangeTag(buffer, TAG_PURGELEVEL);
}

static void LVL_LoadTSJBuffers(bfflevel_t *lvl, char *ptr, int compression)
{
	int64_t buflen, real_size;
	uint64_t mapBufferLen;
	const char *tmp;
	char *buffer;

	lvl->tsjBuffers = (char **)Z_Malloc(sizeof(char *) * lvl->numTilesets, TAG_STATIC, &lvl->tsjBuffers, "TSJbuffers");
	for (int64_t i = 0; i < lvl->numTilesets; i++) {
		buflen = *(int64_t *)ptr;
		ptr += sizeof(int64_t);
		//FS_Read(&real_size, sizeof(int64_t), fd);
		//FS_Read(&buflen, sizeof(int64_t), fd);

		buffer = (char *)Z_Malloc(buflen, TAG_STATIC, &buffer, "tmp");
		memcpy(buffer, ptr, buflen);
		ptr += buflen;
		mapBufferLen = buflen;
		switch (compression) {
		case COMPRESSION_BZIP2:
			tmp = Decompress_BZIP2(buffer, buflen, &mapBufferLen);
			break;
		case COMPRESSION_ZLIB:
			tmp = Decompress_ZLIB(buffer, buflen, &mapBufferLen);
			break;
		case COMPRESSION_NONE:
			tmp = buffer;
			break;
		};

		real_size = mapBufferLen;
		lvl->tsjBuffers[i] = (char *)Z_Malloc(buflen, TAG_STATIC, &lvl->tsjBuffers[i], "TSJbuffer");
		memcpy(lvl->tsjBuffers[i], tmp, mapBufferLen);
		ptr += mapBufferLen;
		Z_ChangeTag(buffer, TAG_PURGELEVEL);
	}
}

static void LVL_LoadBase(bfflevel_t *data, file_t f, uint64_t *buflen)
{
	FS_Read(&data->levelNumber, sizeof(int64_t), f);
	FS_Read(&data->numTilesets, sizeof(int64_t), f);
	FS_Read(&data->mapBufferLen, sizeof(int64_t), f);
	FS_Read(buflen, sizeof(int64_t), f);
}

void BFF_ReadLevel(bfflevel_t *lvl, const bff_chunk_t *chunk, file_t f)
{
	int64_t len;

	lvl->tmjBuffer = (char *)Hunk_Alloc(lvl->mapBufferLen, "TMJbuffer", h_low);
	FS_Read(lvl->tmjBuffer, lvl->mapBufferLen, f);

	lvl->tsjBuffers = (char **)Hunk_Alloc(sizeof(char *) * lvl->numTilesets, "TSJbuffers", h_low);
    for (int64_t i = 0; i < lvl->numTilesets; i++) {
		FS_Read(&len, sizeof(int64_t), f);
        len++;

        lvl->tsjBuffers[i] = (char *)Hunk_Alloc(len, "TSJbuffer", h_low);
		FS_Read(lvl->tsjBuffers[i], len, f);
    }
}

static void CopyLevelChunk(const bff_chunk_t *chunk, bffinfo_t* info)
{
	bfflevel_t *data;
	file_t f;
	
	data = &info->levels[Com_GenerateHashValue(chunk->chunkName, MAX_LEVEL_CHUNKS)];
	N_strncpyz(data->name, chunk->chunkName, MAX_BFF_CHUNKNAME);

	f = FS_FOpenRead(data->name);
	if (f == FS_INVALID_HANDLE) {
		N_Error("BFF_GetInfo: failed to load level chunk %s, FS_INVALID_HANDLE", data->name);
	}

	Con_Printf(DEV, "Loading level chunk %s", data->name);

	FS_Read(&data->levelNumber, sizeof(int64_t), f);
	FS_Read(&data->numTilesets, sizeof(int64_t), f);
	FS_Read(&data->mapBufferLen, sizeof(int64_t), f);
	BFF_ReadLevel(data, chunk, f);

	FS_FClose(f);
	
	Con_Printf(DEV, "Done loading level chunk %s", data->name);

	info->numLevels++;
}

static void CopySoundChunk(const bff_chunk_t* chunk, bffinfo_t* info)
{
	bffsound_t* data;
	file_t f;
	
	data = &info->sounds[info->numSounds];
	N_strncpyz(data->name, chunk->chunkName, MAX_BFF_CHUNKNAME);

	f = FS_FOpenRead(data->name);
	if (f == FS_INVALID_HANDLE) {
		N_Error("Failed to load sound chunk %s, FS_INVALID_HANDLE", data->name);
	}

	Con_Printf(DEV, "Loading audio chunk %s", data->name);

	data->fileSize = chunk->chunkSize;
	data->fileBuffer = (char *)Hunk_Alloc(data->fileSize, "sndfile", h_low);

	FS_Read(data->fileBuffer, data->fileSize, f);
	FS_FClose(f);

	Con_Printf(DEV, "Done loading audio chunk %s", data->name);
	
	info->numSounds++;
}

static void CopyScriptChunk(const bff_chunk_t* chunk, bffinfo_t* info)
{
	bffscript_t* data;
	file_t f;

	data = &info->scripts[Com_GenerateHashValue(chunk->chunkName, MAX_SCRIPT_CHUNKS)];
	N_strncpyz(data->name, chunk->chunkName, MAX_BFF_CHUNKNAME);

	f = FS_FOpenRead(data->name);
	if (f == FS_INVALID_HANDLE) {
		N_Error("Failed to load script chunk %s, FS_INVALID_HANDLE", data->name);
	}

	Con_Printf(DEV, "Loading script chunk %s", data->name);

	data->codelen = chunk->chunkSize;
	data->bytecode = (uint8_t *)Hunk_Alloc(data->codelen, "bytecode", h_low);

	FS_Read(data->bytecode, data->codelen * sizeof(uint8_t), f);
	FS_FClose(f);

	Con_Printf(DEV, "Done loading script chunk %s", data->name);
	
	info->numScripts++;
}

static void CopyTextureChunk(const bff_chunk_t* chunk, bffinfo_t* info)
{
	bfftexture_t* data;
	file_t f;

	data = &info->textures[Com_GenerateHashValue(chunk->chunkName, MAX_TEXTURE_CHUNKS)];
	N_strncpyz(data->name, chunk->chunkName, MAX_BFF_CHUNKNAME);

	f = FS_FOpenRead(data->name);
	if (f == FS_INVALID_HANDLE) {
		N_Error("Failed to load texture chunk %s, FS_INVALID_HANDLE", data->name);
	}

	Con_Printf(DEV, "Loading texture chunk %s", data->name);

	data->fileSize = chunk->chunkSize;
	data->fileBuffer = (unsigned char *)Hunk_Alloc(data->fileSize, "texbuffer", h_low);

	FS_Read(data->fileBuffer, data->fileSize, f);
	FS_FClose(f);

	Con_Printf(DEV, "Done loading texture chunk %s", data->name);

	info->numTextures++;
}

bffinfo_t* BFF_GetInfo(bff_t* archive)
{
	bffinfo_t* info;
	
	info = (bffinfo_t *)Hunk_Alloc(sizeof(bffinfo_t), "BFFinfo", h_low);
	info->numLevels = 0;
	info->numSounds = 0;
	info->numScripts = 0;
	info->numTextures = 0;
	info->compression = archive->header.compression;

	for (int64_t i = 0; i < archive->numChunks; i++) {
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

/*
TODO: put this in n_files and add safety procedures
*/
bff_t* BFF_OpenArchive(const char *filepath)
{
	FILE* fd;
	bff_t *archive;
	bffheader_t *header;

	fd = Sys_FOpen(filepath, "rb");
    if (!fd) {
        N_Error("BFF_OpenArchive: failed to open bff %s", filepath);
    }

	archive = (bff_t *)Mem_Alloc(sizeof(bff_t));
	if (!archive) {
		N_Error("BFF_OpenArchive: Mem_Alloc() failed");
	}
	SafeRead(fd, &archive->header, sizeof(bffheader_t));

	header = &archive->header;
	header->ident = LittleInt(header->ident);
	header->magic = LittleInt(header->magic);
	header->numChunks = LittleInt(header->numChunks);
	header->version = LittleShort(header->version);
	if (header->ident != BFF_IDENT) {
		N_Error("BFF_OpenArchive: file isn't a bff archive");
	}
	if (header->magic != HEADER_MAGIC) {
		N_Error("BFF_OpenArchive: header magic number is not correct");
	}
	if (!header->numChunks) {
		N_Error("BFF_OpenArchive: bad chunk count");
	}
	if (header->version != BFF_VERSION) {
		Con_Printf("========== WARNING: version in bff file %s isn't a supported version ==========", filepath);
	}

	SafeRead(fd, archive->bffPathname, sizeof(archive->bffPathname));
	SafeRead(fd, archive->bffGamename, sizeof(archive->bffGamename));

	Con_Printf(
		"\n<-------- HEADER -------->\n"
		"total chunks: %li\n"
		"compression: %s\n"
		"version: %hu\n",
	header->numChunks, BFF_CompressionString(header->compression), header->version);

	archive->numChunks = header->numChunks;
	archive->chunkList = (bff_chunk_t *)Mem_Alloc(sizeof(bff_chunk_t) * header->numChunks);
	if (!archive->chunkList) {
		N_Error("BFF_OpenArchive: Mem_Alloc() failed");
	}

	for (int64_t i = 0; i < archive->numChunks; i++) {
		size_t offset = ftell(fd);

		SafeRead(fd, archive->chunkList[i].chunkName, MAX_BFF_CHUNKNAME);
		SafeRead(fd, &archive->chunkList[i].chunkSize, sizeof(int64_t));
		SafeRead(fd, &archive->chunkList[i].chunkType, sizeof(int64_t));

		archive->chunkList[i].chunkSize = LittleInt(archive->chunkList[i].chunkSize);
		archive->chunkList[i].chunkType = LittleInt(archive->chunkList[i].chunkType);

		Con_Printf(
			"<-------- CHUNK %li -------->\n"
			"size: %3.03f KiB\n"
			"type: %li\n"
			"name: %s\n"
			"file offset: %lu\n",
		i, ((float)archive->chunkList[i].chunkSize / 1024), archive->chunkList[i].chunkType, archive->chunkList[i].chunkName, offset);

		archive->chunkList[i].chunkBuffer = (char *)Mem_Alloc(archive->chunkList[i].chunkSize);
		if (!archive->chunkList[i].chunkBuffer) {
			N_Error("BFF_OpenArchive: Mem_Alloc() failed");
		}
		SafeRead(fd, archive->chunkList[i].chunkBuffer, archive->chunkList[i].chunkSize);
	}
	fclose(fd);
    return archive;
}

void BFF_CloseArchive(bff_t* archive)
{
	if (!archive) {
		N_Error("BFF_CloseArchive: null archive");
	}
	for (int64_t i = 0; i < archive->numChunks; i++) {
		Mem_Free(archive->chunkList[i].chunkBuffer);
	}
	Mem_Free(archive->chunkList);
	Mem_Free(archive);
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

	for (uint64_t i = 0; i < info->numTextures; ++i) {
		if (info->textures[i].fileBuffer)
			textures.emplace_back(&info->textures[i]);
	}
	return textures;
}

const eastl::vector<const bfflevel_t*>& BFF_OrderLevels(const bffinfo_t *info)
{
	static eastl::vector<const bfflevel_t*> levels;
	levels.reserve(info->numLevels);

	for (uint64_t i = 0; i < info->numLevels; ++i) {
		if (strlen(info->levels[i].name)) // name wont be empty if its a valid level
			levels.emplace_back(&info->levels[i]);
	}
	return levels;
}

#if 0

typedef void* bffHandle;
typedef void* bffChunk;
typedef int bffError;

typedef struct
{
	char *name;
	uint64_t size;
	uint64_t realSize;
	uint64_t compression;
} bffChunkInfo;

#define BFF_NOERROR			(0)
#define BFF_INVALID_CHUNK	(-1)
#define BFF_BAD_HEADER		(-2)

bffError bffLoadFileRead(bffHandle *handle, const char *filename);
bffError bffLoadFileWrite(bffHandle *handle, const char *filename);
bffError bffGetChunkInfo(bffChunk chunk, bffChunkInfo *info);
bffError bffReadFromChunk(bfFChunk chunk, void *buffer, uint64_t size);

#endif