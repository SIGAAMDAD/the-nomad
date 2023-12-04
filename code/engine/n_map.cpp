#include "n_shared.h"
#include "../game/g_bff.h"
#include "../rendergl/rgl_public.h"
#include "../game/g_game.h"
#include "n_map.h"

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

static nmap_t *mapCache;
nlevel_t *level;

static void Map_LoadCheckpoints(nmap_t *mapData, const json* data)
{
    uint64_t c;
    for (const auto& i : data->at("objects")) {
        const json_string& type = data->at("type");
        if (type != "checkpoint") { // skip any non-checkpoint objects
            continue;
        }

        mapData->numCheckpoints++;
    }
    
    c = 0;
    mapData->checkpoints = (checkpoint_t *)Hunk_Alloc(sizeof(*mapData->checkpoints) * mapData->numCheckpoints, "checkpoints", h_low);
    for (const auto& i : data->at("objects")) {
        const json_string& type = data->at("type");
        if (type != "checkpoint") { // skip any non-checkpoint objects
            continue;
        }

        mapData->checkpoints[c][0] = data->at("x");
        mapData->checkpoints[c][1] = data->at("y");
        c++;
    }
}

static qboolean Map_LoadSprites(const json_string& source, nmap_t *mapData, json *tsj)
{
    uint64_t bufLen;
    char *buffer;

    json_string path = source;
    Con_Printf(DEBUG, "tileset path: %s", path.c_str());

    // submit the tileset to the rendering engine
    mapData->texHandle = RE_RegisterTexture(level->textureName);
    if (mapData->texHandle == -1) {
        N_Error("Failed to load texture %s", level->textureName);
    }

    bufLen = FS_LoadFile(path.c_str(), (void **)&buffer);
    if (!buffer) {
        Con_Printf(WARNING, "Couldn't load tsj buffer %s", path.c_str());
        return qfalse;
    }

    // parse the level tsj buffer
    try {
        *tsj = json::parse(buffer);
    } catch (const json::exception& e) {
        FS_FreeFile(buffer);
        Z_FreeTags(TAG_LEVEL, TAG_LEVEL);
        Con_Error(false,
            "Map_LoadSprites: nlohmann exception occurred when parsing tileset %s.\n"
            "\tId: %i\n\tWhat: %s", path.c_str(), e.id, e.what());
        return qfalse;
    }
    FS_FreeFile(buffer);

    uint64_t tileIndex = 0;
    for (uint32_t y = 0; y < mapData->tileCountY; y++) {
        for (uint32_t x = 0; x < mapData->tileCountX; x++) {
            mapData->tilesetData[y * mapData->tileCountX + x] = tileIndex;
            printf("%i ", mapData->tilesetData[y * mapData->tileCountX + x]);
            tileIndex++;
        }
        printf("\n");
    }
    
    return qtrue;
}


static void Map_GenTextureCoords(const glm::vec2& sheetDims, const glm::vec2& spriteDims, const glm::vec2& coords, texcoord_t *tex)
{
    const float size = sheetDims.x * sheetDims.y;
    const glm::vec2 min = { (coords.x * spriteDims.x) / sheetDims.x, (coords.y * spriteDims.y) / sheetDims.x };
    const glm::vec2 max = { ((coords.x + 1) * spriteDims.x) / sheetDims.y, ((coords.y + 1) * spriteDims.y) / sheetDims.y };

    tex->v[0][0] = min.x;
    tex->v[0][1] = min.y;

    tex->v[1][0] = max.x;
    tex->v[1][1] = min.y;
    
    tex->v[2][0] = max.x;
    tex->v[2][1] = max.y;

    tex->v[3][0] = min.x;
    tex->v[3][1] = max.y;
}

static void Map_LoadTileset(nmap_t *mapData)
{
    uint64_t tileIndex;
    
    tileIndex = 0;
    for (uint64_t y = 0; y < mapData->tileCountY; y++) {
        for (uint64_t x = 0; x < mapData->tileCountX; x++) {
            Map_GenTextureCoords({ mapData->imageWidth, mapData->imageHeight }, { mapData->tileWidth, mapData->tileHeight },
                { x, y }, &mapData->tileCoords[y * mapData->tileCountX + x]);
        }
    }
}

static uint32_t CalcGID(uint32_t gid, nmap_t *m, uint32_t y, uint32_t x)
{
    const uint32_t index = y * m->mapWidth + x;

    // get the flags
    bool flippedHorz = gid & TILE_FLIPPED_HORZ;
    bool flippedDiag = gid & TILE_FLIPPED_DIAG;
    bool flippedVert = gid & TILE_FLIPPED_VERT;

    // clear the flags
    gid &= ~(TILE_FLIPPED_HORZ | TILE_FLIPPED_DIAG | TILE_FLIPPED_VERT);
    m->tilemapData[index][0] = gid;

    m->tilemapData[index][1] = 0;
    if (flippedHorz)
        m->tilemapData[index][1] |= TILE_FLIPPED_HORZ;
    if (flippedDiag)
        m->tilemapData[index][1] |= TILE_FLIPPED_DIAG;
    if (flippedVert)
        m->tilemapData[index][1] |= TILE_FLIPPED_VERT;
    
    return gid;
}

static qboolean Map_LoadTiles(nmap_t *mapData, const json *tilelayer, const json *tileset)
{
    uint32_t *tileData, tileIndex;
    uint32_t tileCountX, tileCountY;
    uint32_t mapWidth, mapHeight;
    uint64_t size;
    json *tsj;

    tsj = (json *)Hunk_Alloc(sizeof(json), "JSONtsj", h_low);

    mapData->firstGid = tileset->at("firstgid");
    mapData->mapWidth = tilelayer->at("width");
    mapData->mapHeight = tilelayer->at("height");

    // load the tileset json buffer
    const json_string& tmpName = tileset->at("source").get<json_string>();

    mapData->tilesetName = Z_StrdupTag(tmpName.c_str(), TAG_LEVEL);
    Con_Printf("tileset: %s", mapData->tilesetName);
    if (!Map_LoadSprites(mapData->tilesetName, mapData, tsj))
        return qfalse;
    
    mapData->tsj = tsj;

    // set the pointer to the newly parsed json buffer
    tileset = mapData->tsj;
    mapData->imageWidth = tileset->at("imagewidth");
    mapData->imageHeight = tileset->at("imageheight");
    mapData->tileCountX = mapData->imageWidth / mapData->tileWidth;
    mapData->tileCountY = mapData->imageHeight / mapData->tileHeight;  

    mapData->tilemapData = (tile_t *)Hunk_Alloc(sizeof(*mapData->tilemapData) * mapData->mapWidth * mapData->mapHeight, "tilemapData", h_low);
    mapData->tilesetData = (uint32_t *)Hunk_Alloc(sizeof(*mapData->tilesetData) * mapData->tileCountX * mapData->tileCountY, "tilesetData", h_low);
    mapData->tileCoords = (texcoord_t *)Hunk_Alloc(sizeof(*mapData->tileCoords) * mapData->tileCountX * mapData->tileCountY, "tileCoords", h_low);

    //
    // load the tiles
    //

    // determine the format of the tiles
    if (!tilelayer->contains("encoding")) { // plain 'ol csv
        tileData = Map_LoadCSV(*tilelayer, mapData->mapHeight, mapData->mapWidth);
        mapData->tileDataFmt = MAP_FMT_CSV;
    }
    else {
        tileData = Map_LoadBase64(*tilelayer, mapData->mapWidth, mapData->mapHeight, &mapData->tileDataFmt);
    }

    // copy it over to the tile data
    tileIndex = 0;
    for (uint64_t y = 0; y < mapData->mapHeight; y++) {
        for (uint64_t x = 0; x < mapData->mapWidth; x++) {
            uint32_t gid = CalcGID(tileData[y * mapData->mapWidth + x], mapData, y, x);
            printf("%i ", gid);
        }
        printf("\n");
    }
    // give it back to the zone
    Hunk_FreeTempMemory(tileData);

    Con_Printf(
        "imageWidth: %lu\n"
        "imageHeight: %lu\n"
        "tileWidth: %lu\n"
        "tileHeight: %lu\n",
    mapData->imageWidth, mapData->imageHeight, mapData->tileWidth, mapData->tileHeight);

    return qtrue;
}

nmap_t *LVL_LoadMap(const char *tmjFile)
{
    nmap_t *mapData;
    char *tmjBuffer, *tsjBuffer;
    uint64_t tmjBufLen, tsjBufLen;
    uint64_t numTilesets, numTileLayers;
    const json *tilelayer, *tileset;
    json *tmj, *tsj;

    tmjBufLen = FS_LoadFile(tmjFile, (void **)&tmjBuffer);
    if (!tmjBuffer) {
        Con_Error(false, "LVL_LoadMap: failed to load tmj file %s", tmjFile);
        return NULL;
    }

    tmj = (json *)Hunk_Alloc(sizeof(json), "JSONtmj", h_low);

    try {
        *tmj = json::parse(tmjBuffer);
    } catch (const json::exception& e) {
        FS_FreeFile(tmjBuffer);
        Z_Free(tmj);
        Con_Error(false, "LVL_LoadMap: json exception occurred.\n\tId: %i\n\tWhat: %s", e.id, e.what());
        return NULL;
    }
    FS_FreeFile(tmjBuffer);

    if ((bool)tmj->at("infinite")) {
        Con_Error(false, "LVL_LoadMap: this game doesn't support infinite tile maps");
    }
    if (tmj->at("tilesets").size() > 1) {
        Con_Printf(WARNING, "Maximum tilesets per map is 1, only loading the first tileset");
    }

    mapData = (nmap_t *)Hunk_Alloc(sizeof(*mapData), "mapData", h_low);

    mapData->tmj = tmj;
    mapData->tileWidth = tmj->at("tilewidth");
    mapData->tileHeight = tmj->at("tileheight");

    numTileLayers = 0;
    for (const auto& i : tmj->at("layers")) {
        const json_string& type = i.at("type");

        if (type == "tilelayer") {
            if (numTileLayers) {
                Con_Error(false, "LVL_LoadMap: too many tile layers");
                return NULL;
            }
            else {
                numTileLayers++;
                tilelayer = eastl::addressof(i);
            }
        }
    }
    numTilesets = 0;
    for (const auto& i : tmj->at("tilesets")) {
        if (numTilesets)
            continue;
        else {
            numTilesets++;
            tileset = eastl::addressof(i);
        }
    }

    if (!Map_LoadTiles(mapData, tilelayer, tileset)) {
        Z_FreeTags(TAG_LEVEL, TAG_LEVEL);
        Hunk_Clear();
        return NULL;
    }
    Map_LoadTileset(mapData);

    return mapData;
}

const texcoord_t* Map_GetSpriteCoords(uint32_t gid)
{
    const uint64_t totalSprites = level->mapData->tileCountX * level->mapData->tileCountY;
    return gid > totalSprites || gid < level->mapData->firstGid ? NULL /* invalid gid */ : &level->mapData->tileCoords[gid];
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
    char *out;
    
    out = (char *)Hunk_AllocateTempMemory(bufferSize);

    stream.zalloc = Z_NULL;
    stream.zfree = Z_NULL;
    stream.opaque = Z_NULL;
    stream.next_in = (Bytef *)data;
    stream.avail_in = inlen;
    stream.next_out = (Bytef *)out;
    stream.avail_out = bufferSize;

    ret = inflateInit2(&stream, 15 + 32);
    if (ret != Z_OK)
        N_Error("LVL_LoadMap: zlib (inflateInit2) failed");

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
            N_Error("LVL_LoadMap: zlib (inflate) failed");
        };

        if (ret != Z_STREAM_END) {
            out = (char *)Hunk_ReallocateTempMemory(out, bufferSize * 2);

            stream.next_out = (Bytef *)(out + bufferSize);
            stream.avail_out = bufferSize;
            bufferSize *= 2;
        }
    } while (ret != Z_STREAM_END);

    if (stream.avail_in != 0)
        N_Error("LVL_LoadMap: zlib failed to decompress gzip buffer");

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
    data32 = (uint32_t *)Hunk_AllocateTempMemory(sizeof(uint32_t) * (width * height * 4));

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
        data32 = (uint32_t *)Hunk_AllocateTempMemory(base64Data.size());
        memcpy(data32, base64Data.c_str(), base64Data.size());
    }
    else if (compression == "zlib") {
        *m_dataFmt = MAP_FMT_BASE64_ZLIB;
        uLongf outlen = width * height * 4;
        data32 = (uint32_t *)Hunk_AllocateTempMemory(sizeof(uint32_t) * outlen);

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

// only used if none are provided within the .level file
static const char *defaultLevelName = "tnNoName";
static const char *defaultLongName = "GLNomad No Name";

/*
LVL_Shutdown: clears the entire hunk and frees all zone allocations with TAG_LEVEL
*/
void LVL_Shutdown(void)
{
    Z_FreeTags(TAG_LEVEL, TAG_LEVEL);
}

static qboolean LVL_ParseShader(const char *tok, const char *buffer)
{
    while (1) {
        tok = COM_ParseExt(&buffer, qtrue);

        if (!tok[0]) {
            COM_ParseWarning("no matching '}'");
            return qfalse;
        }

        if (tok[0] == '}') {
            break;
        }
        else if (!N_stricmp(tok, "background_R")) {
            tok = COM_ParseExt(&buffer, qfalse);
            level->background[0] = N_atof(tok);
        }
        else if (!N_stricmp(tok, "background_G")) {
            tok = COM_ParseExt(&buffer, qfalse);
            level->background[1] = N_atof(tok);
        }
        else if (!N_stricmp(tok, "background_B")) {
            tok = COM_ParseExt(&buffer, qfalse);
            level->background[2] = N_atof(tok);
        }
        else if (!N_stricmp(tok, "background_A")) {
            tok = COM_ParseExt(&buffer, qfalse);
            level->background[3] = N_atof(tok);
        }
    }
    return qtrue;
}

static qboolean LVL_ParseGeneral(const char *shaderBuffer)
{
    const char *tok, *buffer;

    buffer = shaderBuffer;

    while (1) {
        tok = COM_ParseExt(&buffer, qtrue);

        if (!tok[0]) {
            COM_ParseWarning("no matching '}' found");
            return qfalse;
        }

        if (tok[0] == '}') {
            break;
        }
        
        //
        // name <name>
        //
        else if (!N_stricmp(tok, "name")) {
            tok = COM_ParseExt(&buffer, qfalse);

            if (!tok[0]) {
                COM_ParseWarning("missing parameter for 'name'");
                return qfalse;
            }

            level->levelName = Z_StrdupTag(tok, TAG_LEVEL);
        }
        //
        // longname <name>
        //
        else if (!N_stricmp(tok, "longname")) {
            tok = COM_ParseExt(&buffer, qfalse);

            if (!tok[0]) {
                COM_ParseWarning("missing parameter for 'longname'");
                return qfalse;
            }
            level->longName = Z_StrdupTag(tok, TAG_LEVEL);
        }
        else if (!N_stricmp(tok, "levelIndex")) {
            tok = COM_ParseExt(&buffer, qfalse);
            level->levelIndex = atoi(tok);

            if (level->levelIndex < 0) {
                COM_ParseError("LVL_ParseGeneral: invalid levelIndex: %i", level->levelIndex);
                // TODO: make an error state for this
            }
        }
        else if (!N_stricmp(tok, "texture")) {
            tok = COM_ParseExt(&buffer, qfalse);
            level->textureName = Z_StrdupTag(tok, TAG_LEVEL);
        }
        else if (!N_stricmp(tok, "map")) {
            tok = COM_ParseExt(&buffer, qfalse);
            level->mapName = Z_StrdupTag(tok, TAG_LEVEL);
        }
        // special textures shader information
        else if (!N_stricmp(tok, "shader")) {
//            LVL_ParseShader(tok, buffer);
        }
    }
    return qtrue;
}

qboolean Com_LoadLevel(const char *name)
{
    uint64_t shaderLen;
    char *shaderBuffer;

    shaderLen = FS_LoadFile(name, (void **)&shaderBuffer);

    if (!shaderBuffer) {
        Con_Error(false, "Com_LoadLevel: Couldn't load file %s", name);
        return qfalse;
    }

    level = (nlevel_t *)Hunk_Alloc(sizeof(*level), "levelData", h_low);
    memset(level, 0, sizeof(*level));

    level->levelName = (char *)defaultLevelName;
    level->longName = (char *)defaultLongName;

    COM_BeginParseSession(name);
    if (!LVL_ParseGeneral(shaderBuffer)) {
        FS_FreeFile(shaderBuffer);
        Z_FreeTags(TAG_LEVEL, TAG_LEVEL);
        Hunk_Clear();
        Con_Error(false, "Failed to parse level %s", name);
        return qfalse;
    }
    FS_FreeFile(shaderBuffer);
    level->mapData = LVL_LoadMap(level->mapName);
    if (!level->mapData) {
        Z_FreeTags(TAG_LEVEL, TAG_LEVEL);
        Hunk_Clear();
        Con_Error(false, "Failed to load map data for %s", name);
        return qfalse;
    }
    return qtrue;
}

void Com_CacheMaps(void)
{
    qboolean success;

    if (!Hunk_TempIsClear()) {
        Con_Printf(WARNING, "Temporary hunk is not cleared during level loading, slight chance of memory leaks on the temp hunk");
    }
    success = Com_LoadLevel("eaglesPk.level");
    if (!success)
        Con_Printf("Failed to load all maps");
    else
        Con_Printf("Successfully loaded all maps");
}
