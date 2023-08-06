#include "n_shared.h"
#include "g_bff.h"
#include "../rendergl/rgl_public.h"
#include "g_game.h"
#include "n_map.h"

#include <cjson/cJSON.h>
#include <zstd/zstd.h>
#include <zlib.h>
#include <gzstream.h>

#define MAP_CACHE_MAGIC 0x55dfa1

static int64_t currentBFFIndex;

typedef struct
{
    uint32_t magic;     // should be MAP_CACHE_MAGIC
    uint32_t endian;    // LE: 0x1, BE: 0x0
    uint32_t numMaps;   // total number of cached maps in the file
} mapCacheHeader_t;

typedef struct
{
    uint32_t hash;
} mapInCache_t;

static uint64_t numLevels;
static nmap_t *mapCache;
static boost::mutex mapLoad;

static void Map_LoadCheckpoints(nmap_t *mapData, const json& data)
{
    for (const auto& i : data.at("objects")) {
        const json_string& type = data.at("type");
        if (type != "checkpoint") { // skip any non-checkpoint objects
            continue;
        }

        mapData->numCheckpoints++;
    }
    uint64_t c = 0;
    mapData->checkpoints = (checkpoint_t *)Hunk_Alloc(sizeof(*mapData->checkpoints) * mapData->numCheckpoints, "checkpoints", h_low);
    for (const auto& i : data.at("objects")) {
        const json_string& type = data.at("type");
        if (type != "checkpoint") { // skip any non-checkpoint objects
            continue;
        }

        mapData->checkpoints[c][0] = data.at("x");
        mapData->checkpoints[c][1] = data.at("y");
        c++;
    }
}

static void Map_LoadSprites(const json_string& source, nmap_t *mapData)
{
    json_string path = source;
    Con_Printf(DEBUG, "tileset path: %s", path.c_str());

    // submit the tileset to the rendering engine    
    RE_SubmitMapTilesheet(path.c_str(), BFF_FetchInfo());

    // parse the level tsj buffer
    try {
        mapData->tsj = json::parse(mapData->lvl->tsjBuffers[0]);
    } catch (const json::exception& e) {
        N_Error("Map_LoadSprites: nlohmann exception occurred when parsing tileset %s.\n\tId: %i\n\tWhat: %s", path.c_str(), e.id, e.what());
    }

    const uint64_t numTiles = mapData->tileCountX * mapData->tileCountY + mapData->firstGid;
    uint32_t *tiles = mapData->tilesetData;
    for (uint64_t i = mapData->firstGid; i < numTiles; ++i)
        tiles[i] = i;
}

static void Map_GenTextureCoords(const glm::vec2& sheetDims, const glm::vec2& spriteDims, const glm::vec2& coords, texcoord_t tex)
{
    glm::vec2 min = { (coords.x * spriteDims.x) / sheetDims.x, (coords.y * spriteDims.y) / sheetDims.x };
    glm::vec2 max = { ((coords.x + 1) * spriteDims.x) / sheetDims.y, ((coords.y + 1) * spriteDims.y) / sheetDims.y };

    tex[0] = { min.x, min.y };
    tex[1] = { max.x, max.y };
    tex[2] = { max.x, max.y };
    tex[3] = { min.x, max.y };
}

static void Map_LoadTileset(nmap_t *mapData)
{
    uint64_t tileIndex = 0;
    for (uint64_t y = 0; y < mapData->tileCountY; y++) {
        for (uint64_t x = 0; x < mapData->tileCountX; x++) {
            Map_GenTextureCoords({ mapData->imageWidth, mapData->imageHeight }, { mapData->tileWidth, mapData->tileHeight },
                { x, y }, mapData->tileCoords[tileIndex]);
            tileIndex++;
        }
    }
}

static void Map_LoadTiles(nmap_t *mapData, const json *tilelayer, const json *tileset)
{
    uint32_t *tileData, tileIndex;

    mapData->firstGid = tileset->at("firstgid");
    mapData->mapWidth = tilelayer->at("width");
    mapData->mapHeight = tilelayer->at("height");

    if (!tilelayer->contains("encoding")) { // plain 'ol csv
        tileData = Map_LoadCSV(*tilelayer, mapData->mapHeight, mapData->mapWidth);
        mapData->tileDataFmt = MAP_FMT_CSV;
    }
    else {
        tileData = Map_LoadBase64(*tilelayer, mapData->mapWidth, mapData->mapHeight, &mapData->tileDataFmt);
    }

    // load the tileset json buffer
    const json_string& tmpName = tileset->at("source").get<json_string>();
    json_string tilesetName = tmpName;

    if (tmpName.find(".tsj")) {
        tilesetName.resize(tmpName.size() - 4);
        COM_StripExtension(tmpName.c_str(), tilesetName.data(), tilesetName.size());
    }

    mapData->tilesetName = Z_Strdup(tilesetName.c_str());
    Con_Printf("tileset: %s", mapData->tilesetName);
    Map_LoadSprites(mapData->tilesetName, mapData);

    // set the pointer to the newly parsed json buffer
    tileset = &mapData->tsj;
    mapData->imageWidth = tileset->at("imagewidth");
    mapData->imageHeight = tileset->at("imageheight");
    mapData->tileCountX = mapData->imageWidth / mapData->tileWidth;
    mapData->tileCountY = mapData->imageHeight / mapData->tileHeight;

    mapData->tilemapData = (tile_t *)Hunk_Alloc(sizeof(*mapData->tilemapData) * mapData->mapWidth * mapData->mapHeight, "tilemapData", h_low);
    mapData->tilesetData = (uint32_t *)Hunk_Alloc(sizeof(*mapData->tilesetData) * mapData->tileCountX * mapData->tileCountY, "tilesetData", h_low);
    mapData->tileCoords = (texcoord_t *)Hunk_Alloc(sizeof(texcoord_t) * mapData->tileCountX * mapData->tileCountY, "tileCoords", h_low);

    // copy it over to the tile data
    tileIndex = 0;
    for (uint64_t y = 0; y < mapData->mapHeight; y++) {
        for (uint64_t x = 0; x < mapData->mapWidth; x++) {
            uint32_t gid = tileData[y * mapData->mapWidth + x] - mapData->firstGid;

            // get the flags
            bool flippedHorz = gid & TILE_FLIPPED_HORZ;
            bool flippedDiag = gid & TILE_FLIPPED_DIAG;
            bool flippedVert = gid & TILE_FLIPPED_VERT;

            // clear the flags
            gid &= ~(TILE_FLIPPED_HORZ | TILE_FLIPPED_DIAG | TILE_FLIPPED_VERT);
            mapData->tilemapData[tileIndex][0] = gid;

            mapData->tilemapData[tileIndex][1] = 0;
            if (flippedHorz)
                mapData->tilemapData[tileIndex][1] |= TILE_FLIPPED_HORZ;
            if (flippedDiag)
                mapData->tilemapData[tileIndex][1] |= TILE_FLIPPED_DIAG;
            if (flippedVert)
                mapData->tilemapData[tileIndex][1] |= TILE_FLIPPED_VERT;

            tileIndex++;
            printf("%i ", gid);
        }
        printf("\n");
    }
    Map_LoadTileset(mapData);

    Con_Printf(
        "imageWidth: %lu\n"
        "imageHeight: %lu\n"
        "tileWidth: %lu\n"
        "tileHeight: %lu\n",
    mapData->imageWidth, mapData->imageHeight, mapData->tileWidth, mapData->tileHeight);

    // give it back to the zone
    Z_Free(tileData);
}

void Map_LoadMap(uint64_t index)
{
    nmap_t *mapData = &mapCache[index];

    try {
        mapData->tmj = json::parse(lvl->tmjBuffer);
    } catch (const json::exception& e) {
        N_Error("Map_LoadMap: json exception occurred.\n\tId: %i\n\tWhat: %s", e.id, e.what());
    }

    if ((bool)mapData->tmj.at("infinite")) {
        N_Error("Map_LoadMap: this game does not support infinite tile maps");
    }
    if (mapData->tmj.at("tilesets").size() > 1) {
        Con_Printf(WARNING, "Maximum tilesets per map is 1, only loading the first tileset");
    }

    mapData->tileWidth = mapData->tmj.at("tilewidth");
    mapData->tileHeight = mapData->tmj.at("tileheight");

    uint64_t numTileLayers = 0;
    const json* tilelayer;
    for (const auto& i : mapData->tmj.at("layers")) {
        const json_string& type = i.at("type");

        if (type == "tilelayer") {
            if (numTileLayers)
                N_Error("Map_LoadMap: too many tile layers");
            else {
                numTileLayers++;
                tilelayer = eastl::addressof(i);
            }
        }
    }
    uint64_t numTilesets = 0;
    const json* tileset;
    for (const auto& i : mapData->tmj.at("tilesets")) {
        if (numTilesets)
            continue;
        else {
            numTilesets++;
            tileset = eastl::addressof(i);
        }
    }

    Map_LoadTiles(mapData, tilelayer, tileset);
}

const glm::vec2* Map_GetSpriteCoords(uint32_t gid)
{
    const uint64_t totalSprites = Game::Get()->c_map->tileCountX * Game::Get()->c_map->tileCountY;
    return gid > totalSprites || gid < Game::Get()->c_map->firstGid ? NULL /* invalid gid */ : Game::Get()->c_map->tileCoords[gid];
}

template<typename T>
static inline bool load(const json_string& key, T const& value, const json& data, bool required = false)
{
    if (!json_contains(key)) {
        if (required)
            N_Error("Map_LoadMap: tmj file is invalid, reason: missing required parm %s", key.c_str());
        else
            return false;
    }
    
    value = data.at(key);
    return true;
}

static const json_string base64_chars = 
             "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
             "abcdefghijklmnopqrstuvwxyz"
             "0123456789+/";

static inline bool is_base64(uint8_t c)
{
    return (isalnum(c) || (c == '+') || (c == '/'));
}

const json_string& base64_encode(uint8_t *out, uint32_t in_len)
{
    static json_string ret;
    int32_t i = 0;
    int32_t j = 0;
    uint8_t char_array_3[3];
    uint8_t char_array_4[4];
    const uint8_t *o = out;

    while (in_len--) {
        char_array_3[i++] = *(o++);
        if (i == 3) {
            char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
            char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
            char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
            char_array_4[3] = char_array_3[2] & 0x3f;
            
            for (i = 0; i < 4; i++)
                ret += base64_chars[char_array_4[i]];
        
            i = 0;
        }
    }

    if (i) {
        for (j = i; j < 3; j++)
            char_array_3[j] = '\0';

        char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
        char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
        char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
        char_array_4[3] = char_array_3[2] & 0x3f;

        for (j = 0; (j < i + 1); j++)
            ret += base64_chars[char_array_4[j]];

        while (i++ < 3)
            ret += '=';
    }

    return ret;
}

const json_string& base64_decode(const json_string& in)
{
    uint64_t in_len = in.size();
    int32_t i = 0;
    int32_t j = 0;
    int32_t in_ = 0;
    uint8_t char_array_4[4], char_array_3[3];
    static json_string ret;

    while (in_len-- && (in[in_] != '=') && is_base64(in[in_])) {
        char_array_4[i++] = in[in_]; in_++;
        if (i == 4) {
            for (i = 0; i < 4; i++)
                char_array_4[i] = base64_chars.find(char_array_4[i]);

            char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
            char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
            char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

            for (i = 0; i < 3; i++)
                ret += char_array_3[i];
        
            i = 0;
        }
    }

    if (i) {
        for (j = i; j <4; j++)
            char_array_4[j] = 0;

        for (j = 0; j < 4; j++)
            char_array_4[j] = base64_chars.find(char_array_4[j]);

        char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
        char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
        char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

        for (j = 0; j < i - 1; j++)
            ret += char_array_3[j];
    }
    
    return ret;
}

// trim out all the white space and uneeded tokens
static inline void Map_TrimBase64(json_string& data)
{
    data.erase(data.begin(), eastl::find_if(data.begin(), data.end(), eastl::not1(eastl::ptr_fun<int, int>(std::isspace))));
}

static const char *zlib_strerror(int err)
{
    switch (err) {
    case Z_BUF_ERROR: return "Z_BUF_ERROR, zlib had an oopsy with the data buffer.";
    case Z_ERRNO: return "Z_ERRNO, pay attention to the errno this time.";
    case Z_STREAM_ERROR: return "Z_STREAM_ERROR, data pointer is NULL";
    case Z_DATA_ERROR: return "Z_DATA_ERROR, data corruption error most likely";
    case Z_MEM_ERROR: return "Z_MEM_ERROR, out of memory error (malloc returned NULL probably)";
    case Z_VERSION_ERROR: return "Z_VERSION_ERROR, bad version, perhaps?";
    };
    return "None... How???";
}

static char *Map_DecompressGZIP(const char *data, uint64_t inlen, uint64_t *outlen, const uint64_t expectedLen)
{
    uint64_t bufferSize = expectedLen;
    int ret;
    z_stream stream;
    char *out = (char *)Z_Malloc(bufferSize, TAG_STATIC, &out, "tileTempData");

    stream.zalloc = Z_NULL;
    stream.zfree = Z_NULL;
    stream.opaque = Z_NULL;
    stream.next_in = (Bytef *)data;
    stream.avail_in = inlen;
    stream.next_out = (Bytef *)out;
    stream.avail_out = bufferSize;

    ret = inflateInit2(&stream, 15 + 32);
    if (ret != Z_OK)
        N_Error("Map_LoadMap: zlib (inflateInit2) failed");

    do {
        ret = inflate(&stream, Z_SYNC_FLUSH);

        switch (ret)  {
        case Z_NEED_DICT:
        case Z_STREAM_ERROR:
            ret = Z_DATA_ERROR;
            break;
        case Z_DATA_ERROR:
        case Z_MEM_ERROR:
            inflateEnd(&stream);
            N_Error("Map_LoadMap: zlib (inflate) failed");
        };

        if (ret != Z_STREAM_END) {
            out = (char *)Z_Realloc(bufferSize * 2, TAG_STATIC, &out, out, "tileTempData");

            stream.next_out = (Bytef *)(out + bufferSize);
            stream.avail_out = bufferSize;
            bufferSize *= 2;
        }
    } while (ret != Z_STREAM_END);

    if (stream.avail_in != 0)
        N_Error("Map_LoadMap: zlib failed to decompress gzip buffer");

    inflateEnd(&stream);

    *outlen = bufferSize;
    Con_Printf("Successfully decompressed %lu bytes with zlib (gzip data)", bufferSize);

    return out;
}

uint32_t* Map_LoadCSV(const json& data, uint64_t width, uint64_t height)
{
    uint32_t *data32;
    const uint64_t *csv_ptr;

    // a simple array thing
    const std::vector<uint64_t>& csv = data.at("data");
    data32 = (uint32_t *)Z_Malloc(sizeof(uint32_t) * (width * height * 4), TAG_STATIC, &data32, "tileTempData");

    for (uint64_t y = 0; y < height; y++) {
        for (uint64_t x = 0; x < width; x++) {
            data32[y * width + x] = csv[y * width + x];
        }
    }
    return data32;
}

uint32_t* Map_LoadBase64(const json& data, uint64_t width, uint64_t height, uint64_t *m_dataFmt)
{
    const json_string& compression = data.at("compression");
    json_string str = data.at("data");
    Map_TrimBase64(str);
    const json_string& base64Data = base64_decode(str);
    uint64_t dataLen;
    uint32_t *data32;

    // no compression (empty string)
    if (!compression.size()) {
        *m_dataFmt = MAP_FMT_BASE64_UNC;
        data32 = (uint32_t *)Z_Malloc(base64Data.size(), TAG_STATIC, &data32, "tileTempData");
        memcpy(data32, base64Data.c_str(), base64Data.size());
    }
    else if (compression == "zlib") {
        *m_dataFmt = MAP_FMT_BASE64_ZLIB;
        uLongf outlen = width * height * 4;
        data32 = (uint32_t *)Z_Malloc(sizeof(uint32_t) * outlen, TAG_STATIC, &data32, "tileTempData");

        int ret = uncompress((Bytef *)data32, &outlen, (const Bytef *)base64Data.c_str(), base64Data.size());
        if (ret != Z_OK) {
            N_Error("Map_LoadMap: zlib failed to decompress tilelayer data of size %lu bytes, reason: %s, errno: %s",
                base64Data.size(), zlib_strerror(ret), strerror(errno));
        }
        Con_Printf("Successfully decompressed %lu bytes with zlib", base64Data.size());
        dataLen = outlen;
    }
    else if (compression == "gzip") {
        *m_dataFmt = MAP_FMT_BASE64_GZIP;
        data32 = (uint32_t *)Map_DecompressGZIP(base64Data.c_str(), base64Data.size(), &dataLen, width * height * 4);
    }
    else if (compression == "zstd")
        N_Error("Map_LoadMap: zstd compression isn't supported yet. If you want compression, use either gzip or zlib");
    
    return data32;
}

#if 0
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
#endif

void Map_LoadLevels(void)
{
    file_t f;
    const int64_t nLevels = FS_NumFilesOfTypeInBFF(currentBFFIndex, LEVEL_CHUNK);
    bfflevel_t *lvl;
    numLevels = nLevels;

    mapCache = (nmap_t *)Hunk_Alloc(sizeof(*mapCache) * numLevels, "mapCache", h_low);
    for (int64_t i = 0; i < nLevels; i++) {
        lvl = (bfflevel_t *)Hunk_Alloc(sizeof(*lvl), "levelData", h_low);
        f = FS_OpenFileInBFF(LEVEL_CHUNK, i, currentBFFIndex);
        if (f == FS_INVALID_HANDLE) {
            N_Error("Map_LoadLevels: Failed to load level chunk %i", i);
        }

        mapCache[i].lvl = lvl;

        FS_Read(&lvl->levelNumver, sizeof(int64_t), f);
        FS_Read(&lvl->numTilesets, sizeof(int64_t), f);
        FS_Read(&lvl->mapBufferLen, sizeof(int64_t), f);

        lvl->tmjBuffer = (char *)Hunk_Alloc(lvl->mapBufferLen, "TMJbuffer", h_low);
        FS_Read(lvl->tmjBuffer, lvl->mapBufferLen, f);

        int64_t len;
        lvl->tsjBuffers = (char **)Hunk_Alloc(sizeof(*lvl->tsjBuffers) * lvl->numTilesets, "TSJbuffers", h_low);
        for (int64_t l = 0; l < lvl.numTilesets; l++) {
            FS_Read(&len, sizeof(int64_t), f);
            len++;

            lvl->tsjBuffers[i] = (char *)Hunk_Alloc(len, "TSJbuffer", h_low);
            FS_Read(lvl->tsjBuffers[i], len, f);
        }

        FS_FClose(f);
    }
}

void CopyLevelChunk(const bff_chunk_t *chunk, bffinfo_t* info)
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

void Com_CacheMaps(void)
{
    Map_LoadLevels();

    for (uint64_t i = 0; i < numLevels; i++) {
        Map_LoadMap();
    }

    Con_Printf("Successfully loaded all maps");
    Game::Get()->c_map = &mapCache[0];
}
