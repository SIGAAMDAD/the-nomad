#include "n_shared.h"
#include "g_bff.h"
#include "../rendergl/rgl_public.h"
#include "g_game.h"
#include "n_map.h"

#include <zstd/zstd.h>
#include <zlib.h>
#include <gzstream.h>

static std::mutex mapLoad;

void Map_LoadLayers(eastl::shared_ptr<GDRMap>& mapData, const json& data)
{
    for (const auto& i : data.at("layers")) {
//        std::unique_lock<std::mutex> lock{mapLoad};
        eastl::shared_ptr<GDRMapLayer> layer;

        const std::string& name = i.at("name");
        const std::string& type = i.at("type");

        if (type == "tilelayer")
            layer = eastl::make_shared<GDRTileLayer>();
        else if (type == "objectgroup")
            layer = eastl::make_shared<GDRObjectGroup>();
        else if (type == "imagelayer")
            layer = eastl::make_shared<GDRImageLayer>();
        else if (type == "group")
            layer = eastl::make_shared<GDRGroupLayer>();
        
        layer->ParseBase(mapData, i, type);
        layer->Parse(mapData, i);
        mapData->AddLayer(layer);
    }
}

void Map_LoadObjects(eastl::shared_ptr<GDRMap>& mapData, const json& data)
{
    if (!json_contains("objects")) // not strictly required
        return;

    for (const auto& i : data.at("objects")) {
//        std::unique_lock<std::mutex> lock{mapLoad};
        eastl::shared_ptr<GDRMapObject> object;
        std::string typeStr;
        
        if (i.contains("polygon")) {
            object = eastl::make_shared<GDRPolygonObject>();
            typeStr = "polygon";
        }
        else if (i.contains("polyline")) {
            object = eastl::make_shared<GDRPolylineObject>();
            typeStr = "polyline";
        }
        else if (i.contains("text")) {
            object = eastl::make_shared<GDRTextObject>();
            typeStr = "text";
        }
        else { // point or rectangle
            uint64_t height = i.at("height");
            uint64_t width = i.at("width");
            if (!width && !height) {
                object = eastl::make_shared<GDRPointObject>();
                typeStr = "point";
            }
            else {
                object = eastl::make_shared<GDRRectObject>();
                typeStr = "rect";
            }
        }

        object->ParseBase(mapData, i, typeStr);
        object->Parse(mapData, i);
        mapData->AddObject(object);
    }
}

void Map_LoadTilesets(eastl::shared_ptr<GDRMap>& mapData, const json& data)
{
    eastl::vector<json> tilesets;
    tilesets.resize(mapData->getLevelData()->numTilesets);
    for (uint32_t i = 0; i < mapData->getLevelData()->numTilesets; ++i) {
        try {
            tilesets[i] = json::parse(mapData->getLevelData()->tsjBuffers[i]);
        } catch (const json::exception& e) {
            N_Error("Map_LoadTilesets: json exception occurred. Id: %i, String: %s", e.id, e.what());
        }
    }

    for (const auto& i : data.at("tilesets")) {
//        std::unique_lock<std::mutex> lock{mapLoad};
        eastl::shared_ptr<GDRTileset> tileset = eastl::make_shared<GDRTileset>();

        tileset->Parse(mapData, i, tilesets);
        mapData->AddTileset(tileset);
    }
}

void Map_LoadMap(const bfflevel_t *lvl/*, boost::asio::thread_pool& pool*/)
{
    json data;

    try {
        data = json::parse(lvl->tmjBuffer);
    } catch (const json::exception& e) {
        N_Error("Map_LoadMap: json exception occurred. Id: %i, String: %s", e.id, e.what());
    }

    eastl::shared_ptr<GDRMap> mapData = eastl::make_shared<GDRMap>();
    mapData->Parse(data, lvl);
#if 0
    boost::asio::post(pool,
        [&] { Map_LoadTilesets(mapData, data); }
    );
    boost::asio::post(pool,
        [&] { Map_LoadObjects(mapData, data); }
    );
#endif
    Map_LoadTilesets(mapData, data);
    Map_LoadObjects(mapData, data);
    Map_LoadLayers(mapData, data);
}

void Com_CacheMaps(void)
{
//    boost::asio::thread_pool pool(BFF_FetchInfo()->numLevels << 1);

    for (uint32_t levelCount = 0; levelCount < BFF_FetchInfo()->numLevels; ++levelCount) {
        char name[MAX_BFF_CHUNKNAME];
        stbsp_snprintf(name, sizeof(name), "NMLVL%i", levelCount);
        Map_LoadMap(BFF_FetchLevel(name));
//        boost::asio::post(pool,
//            [&] { Map_LoadMap(BFF_FetchLevel(name), pool); }
//        );
    }
//    pool.join();
    Con_Printf("Successfully cached all maps");
}

bool GDRMapPropertyList::getBoolean(const std::string& name, bool def) const
{
    const uint64_t hash = Com_GenerateHashValue(name.c_str(), GDR_MAX_MAP_PROPS);
    if (!m_propertyList[hash])
        return def;
    
    return m_propertyList[hash]->getBoolean(def);
}

const std::string& GDRMapPropertyList::getString(const std::string& name, const std::string& def) const
{
    const uint64_t hash = Com_GenerateHashValue(name.c_str(), GDR_MAX_MAP_PROPS);
    if (!m_propertyList[hash])
        return def;
    
    return m_propertyList[hash]->getString(def);
}

double GDRMapPropertyList::getFloat(const std::string& name, double def) const
{
    const uint64_t hash = Com_GenerateHashValue(name.c_str(), GDR_MAX_MAP_PROPS);
    if (!m_propertyList[hash])
        return def;
    
    return m_propertyList[hash]->getFloat(def);
}

int64_t GDRMapPropertyList::getInt(const std::string& name, int64_t def) const
{
    const uint64_t hash = Com_GenerateHashValue(name.c_str(), GDR_MAX_MAP_PROPS);
    if (!m_propertyList[hash])
        return def;
    
    return m_propertyList[hash]->getInt(def);
}

const glm::vec4& GDRMapPropertyList::getColor(const std::string& name, const glm::vec4& def) const
{
    const uint64_t hash = Com_GenerateHashValue(name.c_str(), GDR_MAX_MAP_PROPS);
    if (!m_propertyList[hash])
        return def;
    
    return m_propertyList[hash]->getColor(def);
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
        
        const uint64_t hash = Com_GenerateHashValue(name.c_str(), GDR_MAX_MAP_PROPS);
        m_propertyList[hash] = (GDRMapProperty *)allocator.allocate(sizeof(GDRMapProperty));
        ::new ((void *)m_propertyList[hash]) GDRMapProperty(typeId, value, name);
    }
}

void GDRMapLayer::ParseBase(eastl::shared_ptr<GDRMap>& mapData, const json& data, const std::string& typeStr)
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
    
    if (typeStr == "tilelayer")
        m_type = MAP_LAYER_TILE;
    else if (typeStr == "objectgroup")
        m_type = MAP_LAYER_OBJECT;
    else if (typeStr == "imagelayer")
        m_type = MAP_LAYER_IMAGE;
    else if (typeStr == "group")
        m_type = MAP_LAYER_GROUP;
    
    m_width = data.at("width");
    m_height = data.at("height");
    m_y = data.at("y");
    m_x = data.at("x");
    m_opacity = data.at("opacity");
    m_visible = data.at("visible");
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

uint32_t* GDRTileLayer::LoadCSV(const json& data)
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

uint32_t* GDRTileLayer::LoadBase64(const json& data)
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

void GDRTileLayer::ParseData(const json& data)
{
    uint32_t *tileData;

    // its csv
    if (!json_contains("encoding")) {
        m_dataFmt = MAP_FMT_CSV;
        tileData = LoadCSV(data);
    }
    else {
        tileData = LoadBase64(data);
    }
    
    // if the map has already been loaded (maps are reloaded either on command or every time a non-fatal error occurs)
    if (!m_tileData)
        m_tileData = (GDRTile *)Hunk_Alloc(sizeof(GDRTile) * (m_width * m_height), "tileData", h_low);
    
    // convert the gids to map tiles
    for (uint64_t x = 0; x < m_width; x++) {
        for (uint64_t y = 0; y < m_height; y++) {
            uint32_t gid = tileData[y * m_width + x];

            // find the tileset index
            const uint64_t tilesetIndex = m_mapData->getTilesetIndex(gid);

            if (tilesetIndex < m_mapData->numTilesets()) {
                // if valid, set up the map tile with the tileset
                const eastl::shared_ptr<GDRTileset>& tileset = m_mapData->getTilesets()[tilesetIndex];
                
                ::new ((void *)&m_tileData[y * m_width + x]) GDRTile(tilesetIndex, gid, tileset->getFirstGID());
            }
            else {
                // otherwise, make it null
                ::new ((void *)&m_tileData[y * m_width + x]) GDRTile(m_mapData->numTilesets(), gid, 0);
            }
        }
    }

    // give it back to the zone
    Z_ChangeTag(tileData, TAG_PURGELEVEL);
}

void GDRTileLayer::Parse(eastl::shared_ptr<GDRMap>& mapData, const json& data)
{
#define check_json(key) if (!data.contains(key)) N_Error("Map_LoadTileLayer: tmj file is invalid, reason: missing required parm '" key "'")
    check_json("data");
#undef check_json

    // get the tile data
    ParseData(data);
}

void GDRTileset::Parse(eastl::shared_ptr<GDRMap>& mapData, const json& data, const eastl::vector<json>& tsj)
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

    // hash it
    m_textureHash = Com_GenerateHashValue(m_source.c_str(), MAX_TEXTURE_CHUNKS);
    
    // check if the texture is actually valid
    if (!BFF_FetchInfo()->textures[m_textureHash].fileBuffer)
        N_Error("Map_LoadTilesets: texture source %s is invalid", m_source.c_str());
    
    // find the json chunk within the tilesets
    const json* tsjData;
#if 0
    for (std::vector<json>::const_iterator it = tsj.cbegin(); it != tsj.cend(); ++it) {
        const std::string& name = it->at("name");

        if (std::string(name).c_str() == m_source) {
            tsjData = it;
            break;
        }
    }
#endif
    for (uint64_t i = 0; i < tsj.size(); i++) {
        const std::string& name = tsj[i].at("name");

        if (name == m_source) {
            tsjData = &tsj[i];
            break;
        }
    }

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

    m_spriteData = eastl::make_shared<GDRTileSheet>(this);
}

GDRTileSheet::GDRTileSheet(const GDRTileset* tileset)
{
    auto genCoords = [&](const glm::vec2& sheetDims, const glm::vec2& spriteDims, const glm::vec2& coords, glm::vec2 tex[4]) {
        glm::vec2 min = { (coords.x * spriteDims.x) / sheetDims.x, (coords.y * spriteDims.y) / sheetDims.y };
        glm::vec2 max = { ((coords.x + 1) * spriteDims.x) / sheetDims.x, ((coords.y + 1) * spriteDims.y) / sheetDims.y };

        tex[0] = { min.x, min.y };
        tex[1] = { max.x, min.y };
        tex[2] = { max.x, max.y };
        tex[3] = { min.x, max.y };
    };

    uint32_t gid;

    m_sheetWidth = tileset->getImageWidth();
    m_sheetHeight = tileset->getImageHeight();
    m_tileWidth = tileset->getTileWidth();
    m_tileHeight = tileset->getTileHeight();
    m_firstGid = tileset->getFirstGID();

    gid = m_firstGid;

    const uint32_t tileCountX = m_sheetWidth / m_tileWidth;
    const uint32_t tileCountY = m_sheetHeight / m_tileHeight;
    const float tileVertexWidth = floor(m_sheetWidth / m_tileWidth);
    const uint32_t lastgid = tileVertexWidth / floor(m_sheetHeight / m_tileHeight) * m_firstGid - 1;

    m_sheetData.resize(tileset->getTileCount());
    eastl::vector<GDRSprite>::iterator spriteIt = m_sheetData.begin();
    for (uint32_t y = 0; y < tileCountY; ++y) {
        for (uint32_t x = 0; x < tileCountX; ++x) {
            gid = 1 - (x * y);

            spriteIt->gid = gid;
            genCoords({ m_sheetWidth, m_sheetHeight }, { m_tileWidth, m_tileHeight }, { x, y }, spriteIt->coords);
            spriteIt++;
        }
    }
}

void GDRImageLayer::Parse(eastl::shared_ptr<GDRMap>& mapData, const json& data)
{
#define check_json(key) if (!data.contains(key)) N_Error("Map_LoadImageLayer: tmj file is invalid, reason: missing required parm '" key "'")
    check_json("source");
#undef check_json

    m_image = data.at("source");
    m_texHash = Com_GenerateHashValue(m_image.c_str(), MAX_TEXTURE_CHUNKS);

    // this'll be used with the hash later
    RE_SubmitMapTilesheet(m_image.c_str(), BFF_FetchInfo());
}

void GDRPolylineObject::Parse(eastl::shared_ptr<GDRMap>& mapData, const json& data)
{
#define check_json(key) if (!data.contains(key)) N_Error("Map_LoadPolylineObject: tmj file is invalid, reason: missing required parm '" key "'")
    check_json("polyline");
#undef check_json

    m_pointList.reserve(data.at("polyline").size());
    for (const auto& i : data.at("polyline")) {
        const glm::ivec2 coords = { (int)i.at("x"), (int)i.at("y") };
        m_pointList.emplace_back( coords.x, coords.y );
    }
}

void GDRPolygonObject::Parse(eastl::shared_ptr<GDRMap>& mapData, const json& data)
{
#define check_json(key) if (!data.contains(key)) N_Error("Map_LoadPolygonObject: tmj file is invalid, reason: missing required parm '" key "'")
    check_json("polygon");
#undef check_json

    m_pointList.reserve(data.at("polygon").size());
    for (const auto& i : data.at("polygon")) {
        const glm::ivec2 coords = { (int)i.at("x"), (int)i.at("y") };
        m_pointList.emplace_back(GDRMapPoint( coords.x, coords.y ));
    }
}

void GDRGroupLayer::ParseLayer(const json& data)
{
    eastl::shared_ptr<GDRMapLayer> layer;
    const std::string& name = data.at("name");
    const std::string& type = data.at("type");

    if (type == "tilelayer")
        layer = eastl::make_shared<GDRTileLayer>();
    else if (type == "objectgroup")
        layer = eastl::make_shared<GDRObjectGroup>();
    else if (type == "imagelayer")
        layer = eastl::make_shared<GDRImageLayer>();
    else if (type == "group")
        layer = eastl::make_shared<GDRGroupLayer>();
    
    layer->ParseBase(m_mapData, data, type);
    layer->Parse(m_mapData, data);
    m_layerList.emplace_back(layer);
}

void GDRGroupLayer::Parse(eastl::shared_ptr<GDRMap>& mapData, const json& data)
{
    m_layerList.reserve(data.at("layers").size());
    for (const auto& i : data.at("layers")) {
        ParseLayer(i);
    }
}

void GDRMapObject::ParseBase(eastl::shared_ptr<GDRMap>& mapData, const json& data, const std::string& typeStr)
{
    m_name = data.at("name");

}

void GDRRectObject::Parse(eastl::shared_ptr<GDRMap>& mapData, const json& data)
{
    // nothing to parse
}

void GDRPointObject::Parse(eastl::shared_ptr<GDRMap>& mapData, const json& data)
{
    // nothing to parse
}

void GDRTextObject::Parse(eastl::shared_ptr<GDRMap>& mapData, const json& data)
{
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

void GDRObjectGroup::Parse(eastl::shared_ptr<GDRMap>& mapData, const json& data)
{
    m_objectList.reserve(data.at("objects").size());
    for (const auto& i : data.at("objects")) {
        eastl::shared_ptr<GDRMapObject> object;
        std::string typeStr;
        
        if (i.contains("polygon")) {
            object = eastl::make_shared<GDRPolygonObject>();
            typeStr = "polygon";
        }
        else if (i.contains("polyline")) {
            object = eastl::make_shared<GDRPolylineObject>();
            typeStr = "polyline";
        }
        else if (i.contains("text")) {
            object = eastl::make_shared<GDRTextObject>();
            typeStr = "text";
        }
        else { // point or rectangle
            uint64_t height = i.at("height");
            uint64_t width = i.at("width");
            if (!width && !height) {
                object = eastl::make_shared<GDRPointObject>();
                typeStr = "point";
            }
            else {
                object = eastl::make_shared<GDRRectObject>();
                typeStr = "rect";
            }
        }

        object->ParseBase(mapData, i, typeStr);
        object->Parse(mapData, i);
        m_objectList.emplace_back(object);
    }
}