#include "../bff_file/g_bff.h"
#define LOG_WARN(...) Con_Printf(__VA_ARGS__)
#define N_Error BFF_Error
#include "zone.h"
#include <bzlib.h>
#include <zlib.h>
#include <SDL2/SDL_endian.h>

static const char *defName = DEFAULT_BFF_GAMENAME;

void strncpyz(char *dst, const char *src, size_t n)
{
	strncpy(dst, src, n);
	dst[n - 1] = '\0';
}

static void GetInfo(const json& data, bffinfo_t* info, const char *path)
{
}

#ifndef O_BINARY
#define O_BINARY 0
#endif

#define BUFFER_SIZE (4*1024)

void SafeWrite(const void *data, size_t size, std::vector<char>& buffer)
{
	buffer.insert(buffer.end(), (char *)data, (char *)data + size);
}

void SafeWrite(FILE *fp, const std::vector<char>& buffer)
{
	if (fwrite(buffer.data(), sizeof(char), buffer.size(), fp) == 0) {
		BFF_Error("WriteBFF: failed to write %li bytes to file", buffer.size());
	}
}

void SafeWrite(FILE *fp, const void *data, size_t size)
{
	if (fwrite(data, size, 1, fp) == 0) {
		BFF_Error("WriteBFF: failed to write %li bytes to file", size);
	}
}
void SafeWrite(char *buf_p, const void *data, size_t size)
{
	memcpy(buf_p, data, size);
	buf_p += size;
}

void CopyBuffer(char *bufferPtr, const void *data, int64_t size)
{
	const char *d = (const char *)data;
	while (size--) {
		*bufferPtr++ = *d++ ^ MAGIC_XOR;
	}
}

static void SwapData(void *data, uint64_t size)
{
	size <<= 2;

	for (uint64_t i = 0; i < size; i++)
		((int64_t *)data)[i] = SDL_SwapLE32(((int64_t *)data)[i]);
}
void BFF_WriteLevel(const bfflevel_t *lvl, bff_chunk_t *chunk)
{
	int64_t totalSize;
	char *buf_p;

	totalSize = lvl->mapBufferLen + lvl->levelBufferLen;
	for (int64_t i = 0; i < lvl->numTilesets; i++) {
		totalSize += strlen(lvl->tsjBuffers[i].c_str()) + 1 + sizeof(int64_t);
	}
	totalSize += sizeof(int64_t) * 4 + sizeof(lvl->name);
	strncpy(chunk->chunkName, lvl->name, MAX_BFF_CHUNKNAME);

	chunk->chunkSize = totalSize;
	chunk->chunkBuffer = new char[totalSize];
	buf_p = chunk->chunkBuffer;

	*(int64_t *)buf_p = lvl->levelNumber;
	buf_p += sizeof(int64_t);
	*(int64_t *)buf_p = lvl->levelBufferLen;
	buf_p += sizeof(int64_t);
	*(int64_t *)buf_p = lvl->numTilesets;
	buf_p += sizeof(int64_t);
	*(int64_t *)buf_p = lvl->mapBufferLen;
	buf_p += sizeof(int64_t);

	memcpy(buf_p, lvl->tmjBuffer.c_str(), lvl->mapBufferLen);
	buf_p += lvl->mapBufferLen;

	memcpy(buf_p, lvl->levelBuffer.c_str(), lvl->levelBufferLen);
	buf_p += lvl->levelBufferLen;

	for (int64_t i = 0; i < lvl->numTilesets; i++) {
		*(int64_t *)buf_p = strlen(lvl->tsjBuffers[i].c_str()) + 1;
		buf_p += sizeof(int64_t);

		memcpy(buf_p, lvl->tsjBuffers[i].c_str(), strlen(lvl->tsjBuffers[i].c_str()) + 1);
		buf_p += strlen(lvl->tsjBuffers[i].c_str()) + 1;
	}
}

template<typename CharT>
static const CharT* CompressBuffer(CharT *buffer, uint64_t inlen, uint64_t *outlen, int compression)
{
	const CharT *buf;
	switch (compression) {
	case COMPRESSION_BZIP2:
		buf = Compress_BZIP2(buffer, inlen, outlen);
		break;
	case COMPRESSION_ZLIB:
		buf = Compress_ZLIB(buffer, inlen, outlen);
		break;
	case COMPRESSION_NONE:
		buf = buffer;
		*outlen = inlen;
		break;
	};
	return buf;
}

static inline const char *CompressionString(int compression)
{
	switch (compression) {
	case COMPRESSION_BZIP2: return "bzip2";
	case COMPRESSION_ZLIB: return "zlib";
	};
	return "None";
}

static inline void BFF_Report(int64_t index, int64_t size, const char *name)
{
	Con_Printf(
		"<-------- CHUNK %li -------->\n"
		"size: %3.03f KiB\n"
		"name: %s\n",
	index, ((float)size / 1024), name);
}

// from Quake3e
void StripExtension(const char *in, char* out, uint32_t destsize)
{
	const char *dot = strrchr(in, '.'), *slash;

	if (dot && ((slash = strrchr(in, '/')) == NULL || slash < dot))
		destsize = (destsize < dot-in+1 ? destsize : dot-in+1);

	if ( in == out && destsize > 1 )
		out[destsize - 1] = '\0';
	else {
		strncpy(out, in, destsize);
		out[destsize] = '\0';
	}
}

void WriteBFF(const char *outfile, const char *jsonfile, int compression)
{
	bff_t *archive;
	FILE *fp;

	FILE *jsonfp = SafeOpen(jsonfile, "r");
	const json data = json::parse(jsonfp);
	fclose(jsonfp);

	archive = (bff_t *)SafeMalloc(sizeof(*archive), "archive");
	memset(archive->bffGamename, 0, sizeof(archive->bffGamename));
	strncpyz(archive->bffGamename, data.at("name").get<std::string>().c_str(), sizeof(archive->bffGamename));
	archive->numChunks = data.at("files").size();
	archive->chunkList = new bff_chunk_t[archive->numChunks];

	bffheader_t header = {
		.ident = BFF_IDENT,
		.magic = HEADER_MAGIC,
		.numChunks = archive->numChunks,
		.compression = compression,
		.version = BFF_VERSION
	};

	Con_Printf(
		"<-------- HEADER -------->\n"
		"total chunks: %li\n"
		"compression: %s\n"
		"version: %hu\n",
	header.numChunks, CompressionString(compression), header.version);


	fp = SafeOpen(outfile, "wb");

	SafeWrite(fp, &header, sizeof(header));
	SafeWrite(fp, archive->bffGamename, sizeof(archive->bffGamename));

	uint64_t index = 0;
	for (const auto& i : data.at("files")) {
		bff_chunk_t *chunk = &archive->chunkList[index];

		strncpy(chunk->chunkName, i.get<std::string>().c_str(), sizeof(chunk->chunkName));

		FILE *tempfp = SafeOpen(i.get<std::string>().c_str(), "rb");

		fseek(tempfp, 0L, SEEK_END);
		chunk->chunkSize = ftell(tempfp);
		fseek(tempfp, 0L, SEEK_SET);

		chunk->chunkBuffer = new char[chunk->chunkSize];

		fread(chunk->chunkBuffer, chunk->chunkSize, 1, tempfp);
		fclose(tempfp);

		BFF_Report(index, chunk->chunkSize, chunk->chunkName);
		
		index++;
	}
	for (uint64_t i = 0; i < archive->numChunks; i++) {
		bff_chunk_t *chunk = &archive->chunkList[i];

		SafeWrite(fp, chunk, sizeof(*chunk) - sizeof(char *));
		SafeWrite(fp, chunk->chunkBuffer, chunk->chunkSize);

		delete[] chunk->chunkBuffer;
	}

	delete archive->chunkList;
	free(archive);
	fclose(fp);
}

#if 0
void WriteBFF(const char* outfile, const char* jsonfile, int compression)
{
//	Z_Init();
	bff_t* archive;
	FILE* fp;
	
	bffinfo_t* info = (bffinfo_t *)SafeMalloc(sizeof(bffinfo_t), "info");
	
	FILE* jsonfp = SafeOpen(jsonfile, "r");
	json data = json::parse(jsonfp);
	fclose(jsonfp);
	
	GetInfo(data, info, outfile);
	GetTextures(data, info);
	GetLevels(data, info);
	GetScripts(data, info);
	GetSounds(data, info);
	
	archive = (bff_t *)SafeMalloc(sizeof(bff_t), "archive");
	
	int64_t totalChunks = info->numLevels + info->numSounds + info->numScripts + info->numTextures;
	int64_t chunkCount = 0;
	archive->numChunks = totalChunks;
	archive->bffGamename = info->bffGamename;

	bffheader_t header = {
		.ident = BFF_IDENT,
		.magic = HEADER_MAGIC,
		.numChunks = archive->numChunks,
		.compression = compression,
		.version = BFF_VERSION
	};
	fp = SafeOpen(outfile, "wb");

	SafeWrite(fp, &header, sizeof(bffheader_t));
	uint64_t pathLen = strlen(archive->bffPathname);
	uint64_t gameLen = strlen(archive->bffGamename);

	SafeWrite(fp, &gameLen, sizeof(uint64_t));
	SafeWrite(fp, archive->bffGamename, strlen(archive->bffGamename) + 1);

	Con_Printf(
		"<-------- HEADER -------->\n"
		"total chunks: %li\n"
		"compression: %s\n"
		"version: %hu\n",
	header.numChunks, CompressionString(compression), header.version);

	archive->chunkList = new bff_chunk_t[header.numChunks];

	for (int64_t i = 0; i < info->numLevels; i++) {
		const bfflevel_t *lvl = &info->levels[i];
		bff_chunk_t *chunk = &archive->chunkList[i];

		BFF_WriteLevel(lvl, &archive->chunkList[chunkCount]);
		BFF_Report(chunkCount, chunk->chunkSize, chunk->chunkType, chunk->chunkName);

		chunkCount++;
	}
	for (int64_t i = 0; i < info->numTextures; i++) {
		const bfftexture_t *tex = &info->textures[i];
		bff_chunk_t *chunk = &archive->chunkList[chunkCount];

		chunk->chunkBuffer = new char[tex->fileSize];
		chunk->chunkSize = tex->fileSize;
		chunk->chunkType = TEXTURE_CHUNK;

		memcpy(chunk->chunkBuffer, tex->fileBuffer, tex->fileSize);

		strncpy(chunk->chunkName, tex->name, sizeof(chunk->chunkName));
		BFF_Report(chunkCount, chunk->chunkSize, chunk->chunkType, chunk->chunkName);
		free(tex->fileBuffer);

		chunkCount++;
	}
	for (int64_t i = 0; i < info->numSounds; i++) {
		const bffsound_t *snd = &info->sounds[i];
		bff_chunk_t *chunk = &archive->chunkList[chunkCount];

		chunk->chunkBuffer = new char[snd->fileSize];
		chunk->chunkSize = snd->fileSize;
		chunk->chunkType = SOUND_CHUNK;
		
		strncpy(chunk->chunkName, snd->name, sizeof(chunk->chunkName));
		memcpy(chunk->chunkBuffer, snd->fileBuffer, chunk->chunkSize);

		BFF_Report(chunkCount, chunk->chunkSize, chunk->chunkType, chunk->chunkName);
		free(snd->fileBuffer);

		chunkCount++;
	}
	for (int64_t i = 0; i < info->numScripts; i++) {
		const bffscript_t *script = &info->scripts[i];
		bff_chunk_t *chunk = &archive->chunkList[chunkCount];

		chunk->chunkBuffer = new char[script->codelen];
		chunk->chunkSize = script->codelen;
		chunk->chunkType = SCRIPT_CHUNK;

		strncpy(chunk->chunkName, script->name, sizeof(chunk->chunkName));
		memcpy(chunk->chunkBuffer, script->bytecode, script->codelen);
		BFF_Report(chunkCount, chunk->chunkSize, chunk->chunkType, chunk->chunkName);

		free(script->bytecode);
		chunkCount++;
	}
	free(info->bffGamename);
	free(info->bffPathname);
	free(info);

	fp = SafeOpen(outfile, "wb");
	if (!fp) {
		BFF_Error("WriteBFF: failed to open write stream for output file %s", outfile);
	}

	SafeWrite(fp, &header, sizeof(bffheader_t));
	SafeWrite(fp, archive->bffPathname, sizeof(archive->bffPathname));
	SafeWrite(fp, archive->bffGamename, sizeof(archive->bffGamename));

	for (int64_t i = 0; i < archive->numChunks; i++) {
		SafeWrite(fp, archive->chunkList[i].chunkName, MAX_BFF_CHUNKNAME);
		SafeWrite(fp, &archive->chunkList[i].chunkSize, sizeof(int64_t));
		SafeWrite(fp, &archive->chunkList[i].chunkType, sizeof(int64_t));
		SafeWrite(fp, archive->chunkList[i].chunkBuffer, archive->chunkList[i].chunkSize);
		delete[](char *)archive->chunkList[i].chunkBuffer;
	}

	fclose(fp);
	exit(1);
}
#endif
