#ifndef _N_MAP_
#define _N_MAP_

#include "g_bff.h"

#define json_contains(key) data.contains(key)

// a simple wrapper around json tilemaps

#define GDR_MAX_MAP_PROPS 1024

void Com_CacheMaps(void);

enum : uint64_t {
    TEXT_ITALIC = 0,
    TEXT_UNDERLINE = 2,
    TEXT_WRAP = 4,
    TEXT_STRIKEOUT = 8,
    TEXT_BOLD = 16
};

enum : uint64_t {
    HORZ_ALIGN_CENTER = 0,
    HORZ_ALIGN_LEFT,
    HORZ_ALIGN_RIGHT,
    HORZ_ALIGN_JUSTIFY
};

enum : uint64_t {
    VERT_ALIGN_CENTER = 0,
    VERT_ALIGN_BOTTOM,
    VERT_ALIGN_TOP
};

enum GDRMapPropertyType {
    MAP_PROP_STRING,
    MAP_PROP_INT,
    MAP_PROP_FLOAT,
    MAP_PROP_BOOL,
    MAP_PROP_COLOR,
    MAP_PROP_FILE
};

enum : uint64_t {
    MAP_FMT_CSV, // plain ol' csv
    MAP_FMT_BASE64_UNC, // uncompressed base64 data
    MAP_FMT_BASE64_ZLIB, // compressed (with zlib) base64 data
    MAP_FMT_BASE64_GZIP, // compressed (with gzip) base64 data
    MAP_FMT_BASE64_ZSTD // compressed (with zstd (the facebook thingy)) base64 data
};

enum : uint64_t {
    MAP_OBJECT_TEXT,
    MAP_OBJECT_RECT,
    MAP_OBJECT_POLYLINE,
    MAP_OBJECT_POLYGON,
    MAP_OBJECT_POINT
};

enum : uint64_t {
    MAP_LAYER_TILE,
    MAP_LAYER_OBJECT,
    MAP_LAYER_GROUP,
    MAP_LAYER_IMAGE
};

enum : uint32_t {
    TILE_FLIPPED_HORZ = 0x80000000,
    TILE_FLIPPED_VERT = 0x40000000,
    TILE_FLIPPED_DIAG = 0x20000000
};

struct GDRTile
{
    constexpr inline GDRTile()
        : tilesetIndex(0), id(0), gid(0), flags(0)
    {
    }
    inline GDRTile(uint64_t tileset, int32_t _gid, int64_t tilesetFirstGID)
        : tilesetIndex(tileset),
        id(_gid & ~(TILE_FLIPPED_HORZ | TILE_FLIPPED_VERT | TILE_FLIPPED_DIAG))
    {
        gid = id;
        id -= tilesetFirstGID;
        flags = 0;

        // set the flags
        if (_gid & TILE_FLIPPED_HORZ)
            flags |= TILE_FLIPPED_HORZ;
        if (_gid & TILE_FLIPPED_DIAG)
            flags |= TILE_FLIPPED_DIAG;
        if (_gid & TILE_FLIPPED_VERT)
            flags |= TILE_FLIPPED_VERT;
    }
    inline GDRTile(const GDRTile &) = default;
    inline GDRTile(GDRTile &&) = default;
    ~GDRTile() = default;

    inline GDRTile& operator=(const GDRTile& other)
    {
        memcpy(this, eastl::addressof(other), sizeof(*this));
        return *this;
    }

    inline void init(uint64_t tileset, int32_t _gid, int64_t tilesetFirstGID)
    {
        tilesetIndex = tileset;
        id = _gid & ~(TILE_FLIPPED_HORZ | TILE_FLIPPED_VERT | TILE_FLIPPED_DIAG);

        gid = id;
        id -= tilesetFirstGID;
        flags = 0;

        // set the flags
        if (_gid & TILE_FLIPPED_HORZ)
            flags |= TILE_FLIPPED_HORZ;
        if (_gid & TILE_FLIPPED_DIAG)
            flags |= TILE_FLIPPED_DIAG;
        if (_gid & TILE_FLIPPED_VERT)
            flags |= TILE_FLIPPED_VERT;
    }
    inline bool flippedHorizontally(void) const
    { return (flags & TILE_FLIPPED_HORZ); }
    inline bool flippedVertically(void) const
    { return (flags & TILE_FLIPPED_VERT); }
    inline bool flippedDiagonally(void) const
    { return (flags & TILE_FLIPPED_DIAG); }

    uint64_t tilesetIndex;
    uint64_t id;
    int32_t gid;
    uint64_t flags;
};

struct GDRMapProperty
{
    inline GDRMapProperty(GDRMapPropertyType _type, const char *_value, const char *_name)
        : type(_type), value(_value), name(_name) { }
    inline GDRMapProperty(GDRMapPropertyType _type, const std::string& _value, const std::string& _name)
        : type(_type), value(_value.c_str()), name(_name.c_str()) { }
    inline GDRMapProperty(void) = default;
    ~GDRMapProperty() = default;

    inline const char *getName(void) const
    { return name; }
    inline const char *getString(const char *def = "") const
    {
        if (getType() != MAP_PROP_STRING)
            return def;
        
        return value;
    }
    inline int64_t getInt(int64_t def = 0) const
    {
        if (getType() != MAP_PROP_INT)
            return def;
        
        return N_atoi(value);
    }
    inline double getFloat(double def = 0.0f) const
    {
        if (getType() != MAP_PROP_FLOAT)
            return def;
        
        return atof(value);
    }
    inline bool getBoolean(bool def = false) const
    {
        if (getType() != MAP_PROP_BOOL)
            return def;
        
        return (value == "true" ? true : false);
    }
    inline const glm::vec4& getColor(const glm::vec4& def = glm::vec4(0.0f)) const
    {
        if (getType() != MAP_PROP_COLOR)
            return def;
    
        // We skip the first # character and then read directly the hexadecimal value
        uint64_t color = strtol((value + 1), NULL, 16);

        // If the input has the short format #RRGGBB without alpha channel we set it to 255
        if (strlen(value) == 7) color |= 0xff000000;

        const static glm::vec4 v = {
            (color & 0x00ff0000) >> 16,
            (color & 0x0000ff00) >> 8,
            (color & 0x000000ff),
            (color & 0xff000000) >> 24
        };
        return v;
    }

    inline GDRMapPropertyType getType(void) const
    { return type; }
    inline void setValue(const char *newval)
    { value = newval; }
private:
    GDRMapPropertyType type{};
    const char *value{};
    const char *name{};
};

class GDRMap;
class GDRTileset;
class GDRMapLayer;
class GDRMapObject;

class GDRMapPropertyList
{
public:
    inline GDRMapPropertyList(void)
        : m_numProps(0) { }
    ~GDRMapPropertyList() = default;

    void LoadProps(const json& data);

    const char *getString(const char *name, const char *def = "") const;
    bool getBoolean(const char *name, bool def = false) const;
    double getFloat(const char *name, double def = 0.0f) const;
    int64_t getInt(const char *name, int64_t def = 0) const;
    const glm::vec4& getColor(const char *name, const glm::vec4& def = glm::vec4(0.0f)) const;
    
    inline void addProp(const char *name, bool value)
    { pushProp(name, (value ? "true" : "false"), MAP_PROP_BOOL); }
    inline void addProp(const char *name, const char *value)
    { pushProp(name, value, MAP_PROP_STRING); }
    inline void addProp(const char *name, double value)
    { pushProp(name, va("%f", value), MAP_PROP_FLOAT); }
    inline void addProp(const char *name, int64_t value)
    { pushProp(name, va("%li", value), MAP_PROP_INT); }

    inline bool hasValue(const char *name) const
    { return (getProp(name) != NULL); }
    inline const GDRMapProperty* getProp(const char *name) const
    { return getProp(name); }
    inline const eastl::unordered_map<const char *, GDRMapProperty>& getProperties(void) const
    { return m_propertyList; }
private:
    inline GDRMapProperty* getProp(const char *name)
    {
        auto it = m_propertyList.find(name);
        if (it != m_propertyList.end())
            return &it->second;
        
        return NULL;
    }

    inline void pushProp(const char *name, const char *str_val, GDRMapPropertyType type)
    {
        if (m_numProps >= GDR_MAX_MAP_PROPS)
            N_Error("GDRMapPropertyList::addProp: too many map properties");

        auto it = m_propertyList.find(name);
        if (it != m_propertyList.end())
            it->second.setValue(str_val);
        else {
            m_propertyList.try_emplace(name, type, name, str_val);
            ++m_numProps;
        }
    }
    eastl::unordered_map<const char *, GDRMapProperty> m_propertyList;
    uint64_t m_numProps;
};


struct GDRMapPoint {
    double x;
    double y;

    inline constexpr GDRMapPoint(void)
        : x(0), y(0) { }
    inline constexpr GDRMapPoint(double _x, double _y)
        : x(_x), y(_y) { }
    template<typename T>
    inline GDRMapPoint(T _x, T _y)
        : x((double)_x), y((double)_y) { }
    ~GDRMapPoint() = default;
};

struct GDRSprite
{
    glm::vec2 coords[4]; // opengl texture coordinates
    int32_t gid;
};

class GDRTileSheet
{
public:
    GDRTileSheet(const GDRTileset *tileset);
    ~GDRTileSheet() = default;

    inline const glm::vec2* getSpriteCoords(int32_t gid) const
    {
        for (uint64_t i = 0; i < m_numSprites; i++) {
            if (m_sheetData[i].gid == gid) {
                return m_sheetData[i].coords;
            }
        }
        N_Error("GDRTileSheet::getSpriteCoords: bad gid");
    }

    inline const GDRSprite *getSprites(void) const
    { return m_sheetData; }
    inline GDRSprite *getSprites(void)
    { return m_sheetData; }
    inline uint64_t numSprites(void) const
    { return m_numSprites; }
    inline uint64_t getSheetWidth(void) const
    { return m_sheetWidth; }
    inline uint64_t getSheetHeight(void) const
    { return m_sheetHeight; }
    inline uint64_t getTileWidth(void) const
    { return m_tileWidth; }
    inline uint64_t getTileHeight(void) const
    { return m_tileHeight; }
    inline int32_t getFirstGID(void) const
    { return m_firstGid; }
private:
    GDRSprite* m_sheetData;
    uint64_t m_numSprites;

    uint64_t m_sheetWidth;
    uint64_t m_sheetHeight;
    uint64_t m_tileWidth;
    uint64_t m_tileHeight;
    
    int32_t m_firstGid;
};

class GDRTileset
{
public:
    inline GDRTileset(void) = default;
    ~GDRTileset() = default;

    void Parse(GDRMap *mapData, const json& data, const eastl::vector<json>& tilesets);
    void updateSpriteData(void);
    inline void updateSpriteData(const GDRTile *tile, uint64_t coords)
    {
        m_spriteData->getSprites()[coords].gid = tile->gid;
    }

    inline const eastl::vector<eastl::vector<GDRTile>>& getTiles(void) const
    { return m_tiles; }
    inline const GDRMap *getMap(void) const
    { return m_mapData; }
    inline GDRMap *getMap(void)
    { return m_mapData; }
    inline const GDRTileSheet* getSpriteData(void) const
    { return m_spriteData; }
    inline GDRTileSheet* getSpriteData(void)
    { return m_spriteData; }
    inline const char *getSource(void) const
    { return m_source.c_str(); }
    inline const char *getName(void) const
    { return m_name.c_str(); }
    inline uint64_t getTextureHash(void) const
    { return m_textureHash; }
    inline uint64_t getImageWidth(void) const
    { return m_imageWidth; }
    inline uint64_t getImageHeight(void) const
    { return m_imageHeight; }
    inline uint64_t getMargin(void) const
    { return m_margin; }
    inline uint64_t getSpacing(void) const
    { return m_spacing; }
    inline uint64_t getColumns(void) const
    { return m_columns; }
    inline uint64_t getTileCount(void) const
    { return m_tileCount; }
    inline uint64_t getTileWidth(void) const
    { return m_tileWidth; }
    inline uint64_t getTileHeight(void) const
    { return m_tileHeight; }
    inline int64_t getFirstGID(void) const
    { return m_firstGid; }
private:
    GDRMapPropertyList m_properties;

    eastl::vector<eastl::vector<GDRTile>> m_tiles;
    GDRMap *m_mapData;
    GDRTileSheet* m_spriteData;
    std::string m_name;
    std::string m_source;

    uint64_t m_textureHash;
    uint64_t m_imageWidth;
    uint64_t m_imageHeight;
    uint64_t m_margin;
    uint64_t m_spacing;
    uint64_t m_columns;
    uint64_t m_tileCount;
    uint64_t m_tileWidth;
    uint64_t m_tileHeight;

    int64_t m_firstGid;
};

class GDRRectObject
{
public:
    inline constexpr GDRRectObject(void) {}
    ~GDRRectObject() = default;

    void Parse(GDRMap *mapData, const json& data);
};

class GDRPointObject
{
public:
    inline GDRPointObject(void) = default;
    ~GDRPointObject() = default;

    void Parse(GDRMap *mapData, const json& data);

    inline const GDRMapPoint& getPoint(void) const
    { return m_point; }
    inline GDRMapPoint& getPoint(void)
    { return m_point; }
    inline double getX(void) const
    { return m_point.x; }
    inline double getY(void) const
    { return m_point.y; }
private:
    GDRMap *m_mapData;
    GDRMapPoint m_point;
};

class GDRPolylineObject
{
public:
    inline GDRPolylineObject(void) = default;
    ~GDRPolylineObject() = default;

    void Parse(GDRMap *mapData, const json& data);

    inline const GDRMapPoint *getPoints(void) const
    { return m_pointList; }
    inline GDRMapPoint *getPoints(void)
    { return m_pointList; }
    inline uint64_t getNumPoints(void) const
    { return m_numPoints; }
private:
    GDRMap *m_mapData;
    uint64_t m_numPoints;
    GDRMapPoint *m_pointList;
};

class GDRPolygonObject
{
public:
    inline GDRPolygonObject(void) = default;
    ~GDRPolygonObject() = default;

    void Parse(GDRMap *mapData, const json& data);

    inline const GDRMapPoint *getPoints(void) const
    { return m_pointList; }
    inline GDRMapPoint *getPoints(void)
    { return m_pointList; }
    inline uint64_t getNumPoints(void) const
    { return m_numPoints; }
private:
    GDRMap *m_mapData;
    uint64_t m_numPoints;
    GDRMapPoint *m_pointList;
};

class GDRTextObject
{
public:
    inline GDRTextObject(void) = default;
    ~GDRTextObject() = default;

    void Parse(GDRMap *mapData, const json& data);

    inline const char *getText(void) const
    { return m_texBuffer.c_str(); }
    inline const char *getFont(void) const
    { return m_fontFamily.c_str(); }
    inline uint64_t getFlags(void) const
    { return m_flags; }
    inline uint64_t getPixelSize(void) const
    { return m_pixelSize; }
    inline uint64_t getHAlign(void) const
    { return m_horzAlign; }
    inline uint64_t getVAlign(void) const
    { return m_vertAlign; }
    inline const glm::vec4& getColor(void) const
    { return m_color; }

    inline bool isBold(void) const
    { return (m_flags & TEXT_BOLD); }
    inline bool isUnderlined(void) const
    { return (m_flags & TEXT_UNDERLINE); }
    inline bool isStrikeout(void) const
    { return (m_flags & TEXT_STRIKEOUT); }
    inline bool isItalics(void) const
    { return (m_flags & TEXT_ITALIC); }
    inline bool isWrap(void) const
    { return (m_flags & TEXT_WRAP); }
private:
    GDRMap *m_mapData;
    std::string m_texBuffer;
    std::string m_fontFamily;

    glm::vec4 m_color;

    uint64_t m_pixelSize;
    uint64_t m_horzAlign;
    uint64_t m_vertAlign;
    uint64_t m_flags;
};

typedef union {
    GDRTextObject *text;
    GDRPolylineObject *polyline;
    GDRPolygonObject *polygon;
    GDRRectObject *object;
    GDRPointObject *point;
    GDRRectObject *rect;
    void *data;
} GDRMapObjectData;

class GDRMapObject
{
public:
    inline GDRMapObject(void) = default;
    ~GDRMapObject() = default;

    void ParseBase(GDRMap *mapData, const json& data, const char *typeStr);
    void Parse(GDRMap *mapData, const json& data);

    inline void setType(uint64_t type)
    { m_type = type; }
    inline void allocObject(void *p)
    {
        m_data.data = p;
        switch (m_type) {
        case MAP_OBJECT_POINT: construct(m_data.point); break;
        case MAP_OBJECT_POLYGON: construct(m_data.polygon); break;
        case MAP_OBJECT_POLYLINE: construct(m_data.polyline); break;
        case MAP_OBJECT_RECT: construct(m_data.rect); break;
        case MAP_OBJECT_TEXT: construct(m_data.text); break;
        };
    }

    inline const GDRMap *getMap(void) const
    { return m_mapData; }
    inline GDRMap *getMap(void)
    { return m_mapData; }
    inline const char *getName(void) const
    { return m_name.c_str(); }
    inline int64_t getId(void) const
    { return m_id; }
    inline int64_t getGid(void) const
    { return m_gid; }
    inline double getWidth(void) const
    { return m_width; }
    inline double getHeight(void) const
    { return m_height; }
    inline double getRotation(void) const
    { return m_rotation; }
    inline double getX(void) const
    { return m_x; }
    inline double getY(void) const
    { return m_y; }
    inline bool isVisible(void) const
    { return m_visible; }
    inline uint64_t getType(void) const
    { return m_type; }
    inline const void *data(void) const
    { return m_data.data; }
    inline void *data(void)
    { return m_data.data; }
private:
    GDRMapObjectData m_data;
    GDRMapPropertyList m_propertyList;
    GDRMap *m_mapData;
    std::string m_name;

    int64_t m_id;
    int64_t m_gid;
    uint64_t m_type;

    double m_width;
    double m_height;
    double m_rotation;
    double m_x;
    double m_y;

    bool m_visible;
};


class GDRImageLayer
{
public:
    inline GDRImageLayer(void) = default;
    ~GDRImageLayer() = default;

    void Parse(GDRMap *mapData, const json& data);

    inline const char *getImage(void) const
    { return m_image.c_str(); }
    inline uint64_t getTextureHash(void) const
    { return m_texHash; }
private:
    GDRMap *m_mapData;
    std::string m_image;
    uint64_t m_texHash;
};

// also an objectlayer
class GDRObjectGroup
{
public:
    inline GDRObjectGroup(void) = default;
    ~GDRObjectGroup() = default;

    void Parse(GDRMap *mapData, const json& data);

    inline const GDRMapObject *getObjects(void) const
    { return m_objectList; }
    inline uint64_t numObjects(void) const
    { return m_numObjects; }
    inline uint64_t getDrawOrder(void) const
    { return m_drawOrder; }
private:
    GDRMap *m_mapData;
    uint64_t m_numObjects;
    GDRMapObject *m_objectList;

    uint64_t m_drawOrder;
};

class GDRGroupLayer
{
public:
    inline GDRGroupLayer(void) = default;
    ~GDRGroupLayer() = default;

    void Parse(GDRMap *mapData, const json& data);

    const GDRMapLayer *getChild(uint64_t index) const;
    GDRMapLayer *getChild(uint64_t index);
    inline const GDRMapLayer *getChildren(void) const
    { return m_layerList; }
    inline GDRMapLayer *getChildren(void)
    { return m_layerList; }
    inline uint64_t getNumChildren(void) const
    { return m_numLayers; }
    inline uint64_t getOffsetX(void) const
    { return m_offsetX; }
    inline uint64_t getOffsetY(void) const
    { return m_offsetY; }
private:
    void ParseLayer(const json& data);
    GDRMap *m_mapData;
    uint64_t m_numLayers;
    GDRMapLayer *m_layerList;

    uint64_t m_offsetX;
    uint64_t m_offsetY;
};

class GDRMapLayer;

class GDRTileLayer
{
public:
    inline GDRTileLayer(void) = default;
    ~GDRTileLayer() = default;

    void Parse(const GDRMapLayer *layerData, GDRMap *mapData, const json& data);

    inline uint64_t getDataFormat(void) const
    { return m_dataFmt; }
    inline const GDRTile* getTiles(void) const
    { return m_tileData; }
    inline const GDRTile* getTile(uint64_t index) const
    { return &m_tileData[index]; }
private:
    uint32_t* LoadBase64(const json& data);
    uint32_t* LoadCSV(const json& data);
    void ParseData(const json& data);

    GDRMap *m_mapData;
    GDRTile *m_tileData;

    uint64_t m_dataFmt;
    uint64_t m_width;
    uint64_t m_height;
};

typedef union {
    GDRImageLayer *image;
    GDRObjectGroup *object;
    GDRGroupLayer *group;
    GDRTileLayer *tile;
    void *data;
} GDRMapLayerData;

class GDRMapLayer
{
public:
    inline GDRMapLayer(void) = default;
    ~GDRMapLayer() = default;

    void ParseBase(GDRMap *mapData, const json& data, const char *typeStr);
    void Parse(const json& data);

    inline void allocLayer(void *p)
    {
        m_data.data = p;
        switch (m_type) {
        case MAP_LAYER_IMAGE: construct(m_data.image); break;
        case MAP_LAYER_GROUP: construct(m_data.group); break;
        case MAP_LAYER_TILE: construct(m_data.tile); break;
        case MAP_LAYER_OBJECT: construct(m_data.object); break;
        };
    }
    inline void setType(uint64_t type)
    { m_type = type; }

    inline const void *data(void) const
    { return m_data.data; }
    inline void *data(void)
    { return m_data.data; }
    inline uint64_t getType(void) const
    { return m_type; }
    inline const GDRMap *getMap(void) const
    { return m_mapData; }
    inline GDRMap *getMap(void)
    { return m_mapData; }
    inline const char *getName(void) const
    { return m_name.c_str(); }
    inline const GDRMapPropertyList& getProperties(void) const
    { return m_properties; }
    inline bool isVisible(void) const
    { return m_visible; }
    inline uint64_t getWidth(void) const
    { return m_width; }
    inline uint64_t getHeight(void) const
    { return m_height; }
    inline uint64_t getX(void) const
    { return m_x; }
    inline uint64_t getY(void) const
    { return m_y; }
    inline double getOpacity(void) const
    { return m_opacity; }
private:
    GDRMapLayerData m_data;
    GDRMapPropertyList m_properties;
    GDRMap *m_mapData;
    std::string m_name;

    uint64_t m_width;
    uint64_t m_height;
    uint64_t m_y;
    uint64_t m_x;
    uint64_t m_type;
    double m_opacity;
    bool m_visible;
};

inline const GDRMapLayer *GDRGroupLayer::getChild(uint64_t index) const
{ return &m_layerList[index]; }
inline GDRMapLayer *GDRGroupLayer::getChild(uint64_t index)
{ return &m_layerList[index]; }

class GDRMap
{
public:
    inline GDRMap(void) = default;
    ~GDRMap() = default;

    inline void Parse(const json& data, const bfflevel_t *lvl)
    {
        boost::unique_lock<boost::mutex> lock{m_dataLock};
        m_levelData = lvl;
        m_tmjBuffer = data;
        m_name = lvl->name;
        m_width.store((uint64_t)data.at("width"));
        m_height.store((uint64_t)data.at("height"));

        if (!json_contains("tilesets"))
            N_Error("Map_LoadMap: invalid tmj file, no tilesets found");
        if (!json_contains("layers"))
            N_Error("Map_LoadMap: invalid tmj file, no layers found");
        
        m_numTilesets = data.at("tilesets").size();
        m_tilesets = (GDRTileset *)Hunk_Alloc(sizeof(GDRTileset) * m_numTilesets, "mapTilesets", h_low);
        m_numLayers = data.at("layers").size();
        m_layers = (GDRMapLayer *)Hunk_Alloc(sizeof(GDRMapLayer) * m_numLayers, "mapLayers", h_low);

        if (!json_contains("objects")) {
            Con_Printf("WARNING: tmj file %s has no objects", m_name.c_str());
            return;
        }
        m_objects = (GDRMapObject *)Hunk_Alloc(sizeof(GDRMapObject) * data.at("objects").size(), "mapObjs", h_low);
        m_numObjects = data.at("objects").size();
    }

    inline void allocTSJBuffers(void)
    {
        boost::unique_lock<boost::mutex> lock{m_dataLock};
        m_tsjBuffers.resize(m_levelData->numTilesets);
        for (int64_t i = 0; i < m_levelData->numTilesets; ++i) {
            try {
                m_tsjBuffers[i] = json::parse(m_levelData->tsjBuffers[i]);
            } catch (const json::exception& e) {
                N_Error("Map_LoadTilesets: json exception occurred. Id: %i, String: %s", e.id, e.what());
            }
        }
    }

    inline const eastl::vector<json>& getTSJBuffers(void) const
    { return m_tsjBuffers; }
    inline uint64_t numTSJBuffers(void) const
    { return m_levelData->numTilesets; }
    inline const json& getTMJBuffer(void) const
    { return m_tmjBuffer; }
    inline const GDRMapLayer *getLayers(void) const
    { return m_layers; }
    inline const GDRMapObject *getObjects(void) const
    { return m_objects; }
    inline const GDRTileset *getTilesets(void) const
    { return m_tilesets; }
    inline GDRMapLayer *getLayers(void)
    { return m_layers; }
    inline GDRMapObject *getObjects(void)
    { return m_objects; }
    inline GDRTileset *getTilesets(void)
    { return m_tilesets; }

    inline const char *getName(void) const
    { return m_name.c_str(); }
    inline const bfflevel_t *getLevelData(void) const
    { return m_levelData; }

    inline uint64_t numLayers(void) const
    { return m_numLayers; }
    inline uint64_t numObjects(void) const
    { return m_numObjects; }
    inline uint64_t numTilesets(void) const
    { return m_numTilesets; }
    inline uint64_t getTilesetIndex(int32_t gid) const
    {
        // cleanup the gid bit flags
        gid &= ~(TILE_FLIPPED_VERT | TILE_FLIPPED_DIAG | TILE_FLIPPED_HORZ);
        
        for (uint64_t i = 0; i < m_numTilesets; ++i) {
            if (gid >= m_tilesets[i].getFirstGID()) {
                return i;
            }
        }

        return m_numTilesets;
    }

    inline uint64_t getWidth(void) const
    { return m_width.load(); }
    inline uint64_t getHeight(void) const
    { return m_height.load(); }
    inline bool isInfinite(void) const
    { return m_infinite.load(); }
private:
    // kept for map reloading
    json m_tmjBuffer;
    eastl::vector<json> m_tsjBuffers;

    uint64_t m_numLayers;
    uint64_t m_numObjects;
    uint64_t m_numTilesets;

    GDRMapLayer *m_layers;
    GDRMapObject *m_objects;
    GDRTileset *m_tilesets;
    std::string m_name;

    const bfflevel_t *m_levelData;
    boost::mutex m_dataLock;

    eastl::atomic<uint64_t> m_width;
    eastl::atomic<uint64_t> m_height;
    eastl::atomic<bool> m_infinite;
};

#endif