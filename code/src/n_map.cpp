#include "n_shared.h"
#include "g_bff.h"
#include "../rendergl/rgl_public.h"
#include "g_game.h"
#include "n_map.h"

#include <zstd/zstd.h>
#include <zlib.h>
#include <gzstream.h>

static eastl::vector<GDRMap *> mapCache;
static boost::mutex mapLoad;

static uint64_t Map_LayerSize(const char* typeStr)
{
    if (!N_stricmpn("tilelayer", typeStr, sizeof("tilelayer"))) return sizeof(GDRTileLayer);
    else if (!N_stricmpn("objectgroup", typeStr, sizeof("objectgroup"))) return sizeof(GDRObjectGroup);
    else if (!N_stricmpn("imagelayer", typeStr, sizeof("imagelayer"))) return sizeof(GDRImageLayer);
    else if (!N_stricmpn("group", typeStr, sizeof("group"))) return sizeof(GDRGroupLayer);

    N_Error("Map_LayerSize: invalid layer string type '%s'", typeStr);
}

void Map_LoadLayers(GDRMap *mapData, const json& data)
{
    GDRMapLayer *layer = mapData->getLayers();
    for (const auto& i : data.at("layers")) {
        construct(layer);

        const std::string& name = i.at("name");
        const std::string& type = i.at("type");

        if (type == "tilelayer")
            layer->setType(MAP_LAYER_TILE);
        else if (type == "objectgroup")
            layer->setType(MAP_LAYER_OBJECT);
        else if (type == "imagelayer")
            layer->setType(MAP_LAYER_IMAGE);
        else if (type == "group")
            layer->setType(MAP_LAYER_GROUP);
        
        layer->ParseBase(mapData, i, type.c_str());
        layer->allocLayer(Z_Malloc(Map_LayerSize(type.c_str()), TAG_STATIC, NULL, "mapLayer"));
        layer->Parse(i);
        layer++;
    }
}

static uint64_t Map_ObjectSize(const char *typeStr)
{
    if (!N_stricmpn("polygon", typeStr, sizeof("polygon"))) return sizeof(GDRPolygonObject);
    else if (!N_stricmpn("polyline", typeStr, sizeof("polyline"))) return sizeof(GDRPolylineObject);
    else if (!N_stricmpn("text", typeStr, sizeof("text"))) return sizeof(GDRTextObject);
    else if (!N_stricmpn("point", typeStr, sizeof("point"))) return sizeof(GDRPointObject);
    else if (!N_stricmpn("rect", typeStr, sizeof("rect"))) return sizeof(GDRRectObject);
    
    N_Error("Map_ObjectSize: invalid object string type '%s'", typeStr);
}

void Map_LoadObjects(GDRMap *mapData, const json& data)
{
    {
        if (!json_contains("objects")) // not strictly required
            return;
    }

    GDRMapObject *object = mapData->getObjects();
    for (const auto& i : data.at("objects")) {
        construct(object);
        const char *typeStr;
        
        if (i.contains("polygon")) {
            object->setType(MAP_OBJECT_POLYGON);
            typeStr = "polygon";
        }
        else if (i.contains("polyline")) {
            object->setType(MAP_OBJECT_POLYLINE);
            typeStr = "polyline";
        }
        else if (i.contains("text")) {
            object->setType(MAP_OBJECT_TEXT);
            typeStr = "text";
        }
        else { // point or rectangle
            uint64_t height = i.at("height");
            uint64_t width = i.at("width");
            if (!width && !height) {
                object->setType(MAP_OBJECT_POINT);
                typeStr = "point";
            }
            else {
                object->setType(MAP_OBJECT_RECT);
                typeStr = "rect";
            }
        }

        object->ParseBase(mapData, i, typeStr);
        object->allocObject(Z_Malloc(Map_ObjectSize(typeStr), TAG_STATIC, NULL, "mapObj"));
        object->Parse(mapData, i);
        object++;
    }
}

void Map_LoadTilesets(GDRMap *mapData, const json& data)
{
    mapData->allocTSJBuffers();
    GDRTileset *tileset = mapData->getTilesets();
    uint64_t tilesetIndex = 0;

    for (const auto& i : data.at("tilesets")) {
        construct(tileset);

        tileset->Parse(tilesetIndex, mapData, i, mapData->getTSJBuffers());
        tileset++;
        tilesetIndex++;
    }
}

void Map_LoadMap(const bfflevel_t *lvl)
{
    GDRMap *mapData = (GDRMap *)Hunk_Alloc(sizeof(GDRMap), "mapData", h_low);
    construct(mapData);

    try {
        mapData->Parse(json::parse(lvl->tmjBuffer), lvl);
    } catch (const json::exception& e) {
        N_Error("Map_LoadMap: json exception occurred. Id: %i, String: %s", e.id, e.what());
    }

    Map_LoadTilesets(mapData, mapData->getTMJBuffer());
    Map_LoadObjects(mapData, mapData->getTMJBuffer());
    Map_LoadLayers(mapData, mapData->getTMJBuffer());

//    for (uint64_t i = 0; i < mapData->numTilesets(); i++)
//        mapData->getTilesets()[i].updateSpriteData();

//    boost::unique_lock<boost::mutex> lock{mapLoad};
    mapCache.emplace_back(mapData);
}

void Com_CacheMaps(void)
{
//    eastl::vector<boost::thread> threads;
//    threads.reserve(BFF_FetchInfo()->numLevels);

    // give the map loading a 'boost' by multithreading it, not a pool, just multiple threads
    for (uint32_t levelCount = 0; levelCount < BFF_FetchInfo()->numLevels; ++levelCount) {
        char name[MAX_BFF_CHUNKNAME];
        stbsp_snprintf(name, sizeof(name), "NMLVL%i", levelCount);
        Map_LoadMap(BFF_FetchLevel(name));
//        threads.emplace_back(Map_LoadMap, BFF_FetchLevel(name));
    }
//    for (auto& i : threads)
//        i.join();
//    
    Con_Printf("Successfully cached all maps");
    Game::Get()->c_map = mapCache.front();
}

bool GDRMapPropertyList::getBoolean(const char *name, bool def) const
{
    auto it = m_propertyList.find(name);
    if (it == m_propertyList.end())
        return def;
    
    return it->second.getBoolean();
}

const char *GDRMapPropertyList::getString(const char *name, const char *def) const
{
    auto it = m_propertyList.find(name);
    if (it == m_propertyList.end())
        return def;
    
    return it->second.getString();
}

double GDRMapPropertyList::getFloat(const char *name, double def) const
{
    auto it = m_propertyList.find(name);
    if (it == m_propertyList.end())
        return def;
    
    return it->second.getFloat();
}

int64_t GDRMapPropertyList::getInt(const char *name, int64_t def) const
{
    auto it = m_propertyList.find(name);
    if (it == m_propertyList.end())
        return def;
    
    return it->second.getInt();
}

const glm::vec4& GDRMapPropertyList::getColor(const char *name, const glm::vec4& def) const
{
    auto it = m_propertyList.find(name);
    if (it == m_propertyList.end())
        return def;
    
    return it->second.getColor();
}

template<typename T>
static inline bool load(const std::string& key, T const& value, const json& data, bool required = false)
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


void GDRMapPropertyList::LoadProps(const json& data)
{
    GDRMapPropertyType typeId;

    if (!json_contains("properties")) // no properties to load
        return;

    m_propertyList.reserve(data.at("properties").size());
    for (auto& i : data.at("properties")) {
#define check_json(key) if (!data.contains(key)) N_Error("Map_LoadPropertyList: tmj file is invalid, reason: missing required parm '" key "'")
        check_json("name");
        check_json("type");
        check_json("value");
#undef check_json

        const std::string& name = i.at("name");
        const std::string& typeStr = i.at("type");
        const std::string& value = i.at("value");

        if (typeStr == "string")
            typeId = MAP_PROP_STRING;
        else if (typeStr == "int")
            typeId = MAP_PROP_INT;
        else if (typeStr == "float")
            typeId = MAP_PROP_FLOAT;
        else if (typeStr == "color")
            typeId = MAP_PROP_COLOR;
        else if (typeStr == "bool")
            typeId = MAP_PROP_BOOL;
        else if (typeStr == "file")
            typeId = MAP_PROP_FILE;
        
        m_propertyList.try_emplace(name.c_str(), typeId, value, name);
    }
}

void GDRMapLayer::ParseBase(GDRMap *mapData, const json& data, const char *typeStr)
{
    m_mapData = mapData;
    m_properties.LoadProps(data);

#define check_json(key) if (!data.contains(key)) N_Error("Map_LoadMapLayer: tmj file is invalid, reason: missing required parm '" key "'")
    check_json("name");
    check_json("width");
    check_json("height");
    check_json("y");
    check_json("x");
    check_json("opacity");
    check_json("visible");
#undef check_json

    m_name = data.at("name");
    
    if (!N_stricmpn("tilelayer", typeStr, sizeof("tilelayer")))
        m_type = MAP_LAYER_TILE;
    else if (!N_stricmpn("objectgroup", typeStr, sizeof("objectgroup")))
        m_type = MAP_LAYER_OBJECT;
    else if (!N_stricmpn("imagelayer", typeStr, sizeof("imagelayer")))
        m_type = MAP_LAYER_IMAGE;
    else if (!N_stricmpn("group", typeStr, sizeof("group")))
        m_type = MAP_LAYER_GROUP;
    
    m_width = data.at("width");
    m_height = data.at("height");
    m_y = data.at("y");
    m_x = data.at("x");
    m_opacity = data.at("opacity");
    m_visible = data.at("visible");
}

#if 0 // broken
static void Map_UpdateTilesets(const GDRTile *tileMap, const uint64_t width, const uint64_t height, GDRMap *mapData)
{
    const GDRTile *tile;
    GDRTileset *tileset;

    // set the tileset stuff
    for (uint64_t x = 0; x < width; x++) {
        for (uint64_t y = 0; y < height; y++) {
            tile = &tileMap[y * width + x];
            
            if (tile->tilesetIndex < mapData->numTilesets()) {
                tileset = mapData->getTilesets()[tile->tilesetIndex];
//                tileset->updateSpriteData(tile, y * width + x);
            }
        }
    }
}
#endif

void GDRTileLayer::ParseData(const json& data)
{
    // if the map has already been loaded (maps are reloaded either on command or every time a non-fatal error occurs)
    if (!m_tileData)
        m_tileData = (GDRTile *)Hunk_Alloc(sizeof(GDRTile) * (m_width * m_height), "tileData", h_low);
    
    // reset it all regardless of whether its begin reloaded or not
    memset(m_tileData, 0, sizeof(GDRTile) * (m_width * m_height));

    // convert the gids to map tiles
    tileIndex = 0;
    for (uint64_t y = 0; y < m_height; y++) {
        for (uint64_t x = 0; x < m_width; x++) {
            gid = [tileIndex];

            // get the gid's flags
            bool flippedHorz = gid & TILE_FLIPPED_HORZ;
            bool flippedDiag = gid & TILE_FLIPPED_DIAG;
            bool flippedVert = gid & TILE_FLIPPED_VERT;

            // clear them all
            gid &= ~(TILE_FLIPPED_HORZ | TILE_FLIPPED_VERT | TILE_FLIPPED_DIAG);

            for (uint64_t i = m_mapData->numTilesets() - 1; i >= 0; --i) {
                const GDRTileset *tileset = &m_mapData->getTilesets()[i];

                if (tileset->getFirstGID() <= gid) {
                    m_tileData[y * m_width + x] = tileset->getTiles()[gid - tileset->getFirstGID()];
                }
            }

            tileIndex++;
        }
    }

    // give it back to the zone
    Z_ChangeTag(tileData, TAG_PURGELEVEL);
}

#if 0
void GDRTileset::updateSpriteData(void)
{
    const GDRMapLayer *it = m_mapData->getLayers();

    for (uint64_t i = 0; i < m_mapData->numLayers(); ++i) {
        if (it->getType() != MAP_LAYER_TILE) // skip if not a tilelayer
            continue;
        
        const GDRTile *tileData = ((const GDRTileLayer *)it->data())->getTiles();
        const GDRTile *tile;
        for (uint64_t x = 0; x < it->getWidth(); x++) {
            for (uint64_t y = 0; y < it->getHeight(); y++) {
                tile = &tileData[y * it->getWidth() + x];

                // is it the index?
                if (tile->tilesetIndex == eastl::distance(m_mapData->getTilesets(), this))
                    m_spriteData->getSprites()[y * it->getWidth() + x].gid = tile->gid;
            }
        }
        ++it;
    }
}
#endif

void GDRTileLayer::Parse(const GDRMapLayer *layerData, GDRMap *mapData, const json& data)
{
#define check_json(key) if (!data.contains(key)) N_Error("Map_LoadTileLayer: tmj file is invalid, reason: missing required parm '" key "'")
    check_json("data");
#undef check_json

    m_width = layerData->getWidth();
    m_height = layerData->getHeight();
    m_mapData = mapData;

    // get the tile data
    ParseData(data);
}

static const std::string base64_chars = 
             "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
             "abcdefghijklmnopqrstuvwxyz"
             "0123456789+/";

static inline bool is_base64(uint8_t c)
{
    return (isalnum(c) || (c == '+') || (c == '/'));
}

const std::string& base64_encode(uint8_t *out, uint32_t in_len)
{
    static std::string ret;
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

const std::string& base64_decode(const std::string& in)
{
    uint64_t in_len = in.size();
    int32_t i = 0;
    int32_t j = 0;
    int32_t in_ = 0;
    uint8_t char_array_4[4], char_array_3[3];
    static std::string ret;

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
static inline void Map_TrimBase64(std::string& data)
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

uint32_t* GDRTileset::LoadCSV(const json& data)
{
    uint32_t *data32;
    const uint64_t *csv_ptr;

    // a simple array thing
    const std::vector<uint64_t>& csv = data.at("data");
    data32 = (uint32_t *)Z_Malloc(sizeof(uint32_t) * (m_width * m_height * 4), TAG_STATIC, &data32, "tileTempData");

    for (uint64_t y = 0; y < m_height; y++) {
        for (uint64_t x = 0; x < m_width; x++) {
            data32[y * m_width + x] = csv[y * m_width + x];
        }
    }
    return data32;
}

uint32_t* GDRTileset::LoadBase64(const json& data)
{
    const std::string& compression = data.at("compression");
    std::string str = data.at("data");
    Map_TrimBase64(str);
    const std::string& base64Data = base64_decode(str);
    uint64_t dataLen;
    uint32_t *data32;

    // no compression (empty string)
    if (!compression.size()) {
        m_dataFmt = MAP_FMT_BASE64_UNC;
        data32 = (uint32_t *)Z_Malloc(base64Data.size(), TAG_STATIC, &data32, "tileTempData");
        memcpy(data32, base64Data.c_str(), base64Data.size());
    }
    else if (compression == "zlib") {
        m_dataFmt = MAP_FMT_BASE64_ZLIB;
        uLongf outlen = m_width * m_height * 4;
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
        m_dataFmt = MAP_FMT_BASE64_GZIP;
        data32 = (uint32_t *)Map_DecompressGZIP(base64Data.c_str(), base64Data.size(), &dataLen, m_width * m_height * 4);
    }
    else if (compression == "zstd")
        N_Error("Map_LoadMap: zstd compression isn't supported yet. If you want compression, use either gzip or zlib");
    
    return data32;
}


void GDRTileset::Parse(uint64_t index, GDRMap *mapData, const json& data, const eastl::vector<json>& tsj)
{
    m_mapData = mapData;
    m_properties.LoadProps(data);

#define check_json(key) if (!data.contains(key)) N_Error("Map_LoadTileset: tmj file is invalid, reason: missing required parm '" key "'")
    check_json("source");
    check_json("firstgid");
#undef check_json

    const std::string& source = data.at("source");
    m_firstGid = (int32_t)data.at("firstgid");

    // strip the .tsj part if its there (the source will be used for bff texture lookups)
    if (source.find(".tsj") != std::string::npos) {
        m_source.resize(source.size() - 4);
        COM_StripExtension(source.c_str(), m_source.data(), m_source.size());
    }
    else {
        m_source = source;
    }

    // hash it
    m_textureHash = Com_GenerateHashValue(m_source.c_str(), MAX_TEXTURE_CHUNKS);
    
    // check if the texture is actually valid
    if (!BFF_FetchInfo()->textures[m_textureHash].fileBuffer)
        N_Error("Map_LoadTilesets: texture source %s is invalid", m_source.c_str());
    
    // find the json chunk within the tilesets
    const json* tsjData;
    for (uint64_t i = 0; i < tsj.size(); ++i) {
        const std::string& name = tsj[i].at("name");

        if (name == m_source) {
            tsjData = &tsj[i];
            break;
        }
    }
    if (!tsjData)
        N_Error("Map_LoadTileset: failed to find tileset source for '%s'", m_source.c_str());

#define check_json(key) if (!tsjData->contains(key)) N_Error("Map_LoadTileset: tsj file is invalid, reason: missing required parm '" key "'")
    check_json("tilewidth");
    check_json("tileheight");
    check_json("imagewidth");
    check_json("imageheight");
    check_json("spacing");
    check_json("margin");
    check_json("columns");
    check_json("tilecount");
    check_json("name");
#undef check_json
    m_tileCount = tsjData->at("tilecount");
    m_name = tsjData->at("name");
    m_spacing = tsjData->at("spacing");
    m_margin = tsjData->at("margin");
    m_columns = tsjData->at("columns");
    m_tileWidth = tsjData->at("tilewidth");
    m_tileHeight = tsjData->at("tileheight");
    m_imageWidth = tsjData->at("imagewidth");
    m_imageHeight = tsjData->at("imageheight");

    RE_SubmitMapTilesheet(m_source.c_str(), BFF_FetchInfo());

    // generate the texture coordinates
    auto genCoords = [&](const glm::vec2& sheetDims, const glm::vec2& spriteDims, const glm::vec2& coords, glm::vec2 tex[4]) {
        glm::vec2 min = { (coords.x * spriteDims.x) / sheetDims.x, (coords.y * spriteDims.y) / sheetDims.y };
        glm::vec2 max = { ((coords.x + 1) * spriteDims.x) / sheetDims.x, ((coords.y + 1) * spriteDims.y) / sheetDims.y };

        tex[0] = { min.x, min.y };
        tex[1] = { max.x, min.y };
        tex[2] = { max.x, max.y };
        tex[3] = { min.x, max.y };
    };

    const uint64_t tileCountX = m_imageWidth / m_tileWidth;
    const uint64_t tileCountY = m_imageHeight / m_tileHeight;
    const double tileVertexWidth = floor(m_imageWidth / m_tileWidth);
    const uint32_t lastgid = tileVertexWidth / floor(m_imageHeight / m_tileHeight) * m_firstGid - 1;
    uint32_t tileIndex = 0;

    m_tileData = (GDRTile *)Hunk_Alloc(sizeof(GDRTile) * (tileCountX * tileCountY), "tilesetData", h_low);
    const glm::vec2 imageDims = { m_imageWidth, m_imageHeight };
    const glm::vec2 tileDims = { m_tileWidth, m_tileHeight };
    
    for (uint64_t x = 0; x < tileCountX; ++x) {
        for (uint64_t y = 0; x < tileCountY; ++y) {
            m_tileData[y * tileCountX + x].tilesetIndex = index;
            m_tileData[y * tileCOuntX + x].gid = m_firstGid + tileIndex;

            uint32_t gid = m_tileData[y * tileCountX]
            // clear the flags
            gid &= ~(TILE_FLIPPED_HORZ | TILE_FLIPPED_DIAG | TILE_FLIPPED_VERT);
            
            // set the flags
            if (_gid & TILE_FLIPPED_HORZ)
                flags |= TILE_FLIPPED_HORZ;
            if (_gid & TILE_FLIPPED_DIAG)
                flags |= TILE_FLIPPED_DIAG;
            if (_gid & TILE_FLIPPED_VERT)
                flags |= TILE_FLIPPED_VERT;
            
            construct(&m_tileData[y * tileCountX + x], index, tileIndex + m_firstGid);
            genCoords(imageDims, tileDims, glm::vec2(x, y), m_tileData[y * tileCountX + x].coords);
            tileIndex++;
        }
    }

    Con_Printf(DEV,
        "\n"
        "[Tileset Info]\n"
        "tilewidth: %li\n"
        "tileheight: %li\n"
        "spacing; %li\n"
        "columns: %li\n"
        "imagewidth: %li\n"
        "imageheight: %li\n"
        "tilecount: %li\n"
        "source: %s\n"
        "name: %s\n",
    m_tileWidth, m_tileHeight, m_spacing, m_columns, m_imageWidth, m_imageHeight, m_tileCount, m_source.c_str(), m_name.c_str());
}

void GDRImageLayer::Parse(GDRMap *mapData, const json& data)
{
#define check_json(key) if (!data.contains(key)) N_Error("Map_LoadImageLayer: tmj file is invalid, reason: missing required parm '" key "'")
    check_json("source");
#undef check_json

    m_image = data.at("source");
    m_texHash = Com_GenerateHashValue(m_image.c_str(), MAX_TEXTURE_CHUNKS);

    // this'll be used with the hash later
    RE_SubmitMapTilesheet(m_image.c_str(), BFF_FetchInfo());
}

void GDRPolylineObject::Parse(GDRMap *mapData, const json& data)
{
#define check_json(key) if (!data.contains(key)) N_Error("Map_LoadPolylineObject: tmj file is invalid, reason: missing required parm '" key "'")
    check_json("polyline");
#undef check_json

    m_numPoints = data.at("polyline").size();
    m_pointList = (GDRMapPoint *)Z_Malloc(sizeof(GDRMapPoint) * m_numPoints, TAG_STATIC, &m_pointList, "mapPoints");

    GDRMapPoint *point = m_pointList;
    for (const auto& i : data.at("polyline")) {
        const glm::ivec2 coords = { (int)i.at("x"), (int)i.at("y") };
        construct( point, coords.x, coords.y );
        point++;
    }
}

void GDRPolygonObject::Parse(GDRMap *mapData, const json& data)
{
#define check_json(key) if (!data.contains(key)) N_Error("Map_LoadPolygonObject: tmj file is invalid, reason: missing required parm '" key "'")
    check_json("polygon");
#undef check_json

    m_numPoints = data.at("polygon").size();
    m_pointList = (GDRMapPoint *)Z_Malloc(sizeof(GDRMapPoint) * m_numPoints, TAG_STATIC, &m_pointList, "mapPoints");

    GDRMapPoint *point = m_pointList;
    for (const auto& i : data.at("polygon")) {
        const glm::ivec2 coords = { (int)i.at("x"), (int)i.at("y") };
        construct(point, coords.x, coords.y);
        point++;
    }
}

void GDRMapLayer::Parse(const json& data)
{
    switch (m_type) {
    case MAP_LAYER_IMAGE:
        m_data.image->Parse(m_mapData, data);
        break;
    case MAP_LAYER_TILE:
        m_data.tile->Parse(this, m_mapData, data);
        break;
    case MAP_LAYER_GROUP:
        m_data.group->Parse(m_mapData, data);
        break;
    case MAP_LAYER_OBJECT:
        m_data.object->Parse(m_mapData, data);
        break;
    };
}

void GDRGroupLayer::Parse(GDRMap *mapData, const json& data)
{
    m_numLayers = data.at("layers").size();
    m_layerList = (GDRMapLayer *)Hunk_Alloc(sizeof(GDRMapLayer) * m_numLayers, "mapLayers", h_low);

    GDRMapLayer *layer = m_layerList;
    for (const auto& i : data.at("layers")) {
        construct(layer);
        const std::string& name = data.at("name");
        const std::string& type = data.at("type");

        if (type == "tilelayer")
            layer->setType(MAP_LAYER_TILE);
        else if (type == "objectgroup")
            layer->setType(MAP_LAYER_OBJECT);
        else if (type == "imagelayer")
            layer->setType(MAP_LAYER_IMAGE);
        else if (type == "group")
            layer->setType(MAP_LAYER_GROUP);
    
        layer->ParseBase(m_mapData, data, type.c_str());
        layer->allocLayer(Z_Malloc(Map_LayerSize(type.c_str()), TAG_STATIC, NULL, "mapLayer"));
        layer->Parse(data);
        layer++;
    }
}

void GDRMapObject::ParseBase(GDRMap *mapData, const json& data, const char *typeStr)
{
    m_name = data.at("name");
    m_mapData = mapData;
}

void GDRMapObject::Parse(GDRMap *mapData, const json& data)
{
    switch (m_type) {
    case MAP_OBJECT_POINT:
        m_data.point->Parse(m_mapData, data);
        break;
    case MAP_OBJECT_POLYGON:
        m_data.polygon->Parse(m_mapData, data);
        break;
    case MAP_OBJECT_POLYLINE:
        m_data.polyline->Parse(m_mapData, data);
        break;
    case MAP_OBJECT_RECT:
        m_data.rect->Parse(m_mapData, data);
        break;
    case MAP_OBJECT_TEXT:
        m_data.text->Parse(m_mapData, data);
        break;
    };
}

void GDRRectObject::Parse(GDRMap *mapData, const json& data)
{
    // nothing to parse
}

void GDRPointObject::Parse(GDRMap *mapData, const json& data)
{
    m_mapData = mapData;
    m_point.x = data.at("x");
    m_point.y = data.at("y");
}

void GDRTextObject::Parse(GDRMap *mapData, const json& data)
{
    m_mapData = mapData;
    const json& text = data.at("text");
    const std::string& halign = data.at("halign");
    const std::string& valign = data.at("valign");
    m_texBuffer = data.at("text");
    m_fontFamily = data.at("fontfamily");

    // convert the string into a color by using the property struct one
    GDRMapProperty p(MAP_PROP_COLOR, data.at("color"), "color");
    m_color = p.getColor();
    
    // clear, then set the flags
    m_flags = 0;
    if ((bool)data.at("wrap"))
        m_flags |= TEXT_WRAP;
    if ((bool)data.at("bold"))
        m_flags |= TEXT_BOLD;
    if ((bool)data.at("strikeout"))
        m_flags |= TEXT_STRIKEOUT;
    if ((bool)data.at("italic"))
        m_flags |= TEXT_ITALIC;
    if ((bool)data.at("underline"))
        m_flags |= TEXT_UNDERLINE;
}

void GDRObjectGroup::Parse(GDRMap *mapData, const json& data)
{
    m_numObjects = data.at("objects").size();
    m_objectList = (GDRMapObject *)Hunk_Alloc(sizeof(GDRMapObject) * m_numObjects, "mapObjs", h_low);

    GDRMapObject *object = m_objectList;
    for (const auto& i : data.at("objects")) {
        construct(object);
        const char *typeStr;
        
        if (i.contains("polygon")) {
            object->setType(MAP_OBJECT_POLYGON);
            typeStr = "polygon";
        }
        else if (i.contains("polyline")) {
            object->setType(MAP_OBJECT_POLYLINE);
            typeStr = "polyline";
        }
        else if (i.contains("text")) {
            object->setType(MAP_OBJECT_TEXT);
            typeStr = "text";
        }
        else { // point or rectangle
            uint64_t height = i.at("height");
            uint64_t width = i.at("width");
            if (!width && !height) {
                object->setType(MAP_OBJECT_POINT);
                typeStr = "point";
            }
            else {
                object->setType(MAP_OBJECT_RECT);
                typeStr = "rect";
            }
        }

        object->ParseBase(mapData, i, typeStr);
        object->allocObject(Z_Malloc(Map_ObjectSize(typeStr), TAG_STATIC, NULL, "mapObj"));
        object->Parse(mapData, i);
        object++;
    }
}