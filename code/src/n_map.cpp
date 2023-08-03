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

static eastl::vector<nmap_t *> mapCache;
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
    if (source.find(".tsj")) {
        path.resize(source.size() - 4);
        COM_StripExtension(source.c_str(), path.data(), path.size());
    }

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
    RE_SubmitMapTilesheet(mapData->tilesetName, BFF_FetchInfo());

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
            uint32_t gid = tileData[tileIndex];

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
        }
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

void Map_LoadMap(const bfflevel_t *lvl)
{
    nmap_t *mapData = (nmap_t *)Hunk_Alloc(sizeof(*mapData), "mapData", h_low);
    mapData->lvl = lvl;
    mapData->name = Z_Strdup(lvl->name);

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
    

    mapCache.emplace_back(mapData);
}

void Com_CacheMaps(void)
{
    // give the map loading a 'boost' by multithreading it, not a pool, just multiple threads
    for (uint32_t levelCount = 0; levelCount < BFF_FetchInfo()->numLevels; ++levelCount) {
        char name[MAX_BFF_CHUNKNAME];
        stbsp_snprintf(name, sizeof(name), "NMLVL%i", levelCount);
        Map_LoadMap(BFF_FetchLevel(name));
    }

    Con_Printf("Successfully cached all maps");
    Game::Get()->c_map = mapCache.front();
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
