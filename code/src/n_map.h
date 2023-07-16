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
    TILE_FLIPPED_HORZ = 0,
    TILE_FLIPPED_VERT = 2,
    TILE_FLIPPED_DIAG = 4
};

struct GDRTile
{
    constexpr inline GDRTile()
        : tilesetIndex(0), id(0), gid(0), flags(0)
    {
    }
    inline GDRTile(uint32_t tileset, uint32_t _gid, int32_t tilesetFirstGID)
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
        memcpy(this, eastl::addressof(other), sizeof(uint32_t) * 4);
        return *this;
    }

    inline bool flippedHorizontally(void) const
    { return (flags & TILE_FLIPPED_HORZ); }
    inline bool flippedVertically(void) const
    { return (flags & TILE_FLIPPED_VERT); }
    inline bool flippedDiagonally(void) const
    { return (flags & TILE_FLIPPED_DIAG); }

    uint32_t tilesetIndex;
    uint32_t id;
    uint32_t gid;
    uint32_t flags;
};

struct GDRMapProperty
{
    inline GDRMapProperty(GDRMapPropertyType _type, const std::string& _value, const std::string& _name)
        : type(_type), value(_value), name(_name)
    {
    }
    constexpr inline GDRMapProperty(void) = default;
    ~GDRMapProperty() = default;

    inline const std::string& getName(void) const
    { return name; }

    inline const std::string& getString(const std::string& def = "") const
    {
        if (getType() != MAP_PROP_STRING)
            return def;
        
        return value;
    }
    inline const char *getString(const char *def = "") const
    {
        if (getType() != MAP_PROP_STRING)
            return def;
        
        return value.c_str();
    }
    inline int64_t getInt(int64_t def = 0) const
    {
        if (getType() != MAP_PROP_INT)
            return def;
        
        return N_atoi(value.c_str());
    }
    inline double getFloat(double def = 0.0f) const
    {
        if (getType() != MAP_PROP_FLOAT)
            return def;
        
        return atof(value.c_str());
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
        uint64_t color = strtol((value.c_str() + 1), NULL, 16);

        // If the input has the short format #RRGGBB without alpha channel we set it to 255
        if (value.size() == 7) color |= 0xff000000;

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
    inline void setValue(const std::string& newval)
    { value = newval; }
private:
    GDRMapPropertyType type{};
    std::string value{};
    std::string name{};
};

class GDRMap;
class GDRTileset;
class GDRMapLayer;
class GDRMapObject;

class GDRMapPropertyList
{
public:
    inline GDRMapPropertyList(void)
        : m_numProps(0)
    { memset(m_propertyList.data(), NULL, sizeof(GDRMapProperty) * m_propertyList.size()); }
    ~GDRMapPropertyList() = default;

    void LoadProps(const json& data);

    bool getBoolean(const std::string& name, bool def = false) const;
    const std::string& getString(const std::string& name, const std::string& def = "") const;
    double getFloat(const std::string& name, double def = 0.0f) const;
    int64_t getInt(const std::string& name, int64_t def = 0) const;
    const glm::vec4& getColor(const std::string& name, const glm::vec4& def = glm::vec4(0.0f)) const;
    
    inline void addProp(const char *name, bool value)
    { pushProp(name, (value ? "true" : "false"), MAP_PROP_BOOL); }
    inline void addProp(const char *name, const std::string& value)
    { pushProp(name, value, MAP_PROP_STRING); }
    inline void addProp(const char *name, double value)
    { pushProp(name, va("%f", value), MAP_PROP_FLOAT); }
    inline void addProp(const char *name, int64_t value)
    { pushProp(name, va("%li", value), MAP_PROP_INT); }

    inline bool hasValue(const char *name) const
    { return (getProp(name) != NULL); }
    inline const GDRMapProperty* getProp(const char *name) const
    { return getProp(name); }
    inline const eastl::array<GDRMapProperty*, GDR_MAX_MAP_PROPS>& getProperties(void) const
    { return m_propertyList; }
private:
    inline GDRMapProperty* getProp(const char *name)
    {
        const uint64_t hash = Com_GenerateHashValue(name, GDR_MAX_MAP_PROPS);
        if (m_propertyList[hash])
            return m_propertyList[hash];
        
        return NULL;
    }

    inline void pushProp(const std::string& name, const std::string& str_val, GDRMapPropertyType type)
    {
        if (m_numProps >= GDR_MAX_MAP_PROPS)
            N_Error("GDRMapPropertyList::addProp: too many map properties");

        const uint64_t hash = Com_GenerateHashValue(name.c_str(), GDR_MAX_MAP_PROPS);
        if (m_propertyList[hash])
            m_propertyList[hash]->setValue(str_val);
        else {
            m_propertyList[hash] = (GDRMapProperty *)allocator.allocate(sizeof(GDRMapProperty));
            ::new ((void *)m_propertyList[hash]) GDRMapProperty(type, name, str_val);
            ++m_numProps;
        }
    }
    eastl::array<GDRMapProperty*, GDR_MAX_MAP_PROPS> m_propertyList;
    uint64_t m_numProps;
    GDRAllocator<GDRMapProperty> allocator;
};


struct GDRMapPoint {
    double x;
    double y;

    inline constexpr GDRMapPoint()
        : x(0), y(0) { }
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
    GDRTileSheet(const GDRTileset* tileset);
    ~GDRTileSheet() = default;

    inline const glm::vec2* getSpriteCoords(int32_t gid) const
    {
        for (eastl::vector<GDRSprite>::const_iterator it = m_sheetData.cbegin(); it != m_sheetData.cend(); ++it) {
            if (it->gid == gid)
                return it->coords;
        }
        N_Error("GDRTileSheet::getSpriteCoords: bad gid");
    }

    inline const eastl::vector<GDRSprite>& getSprites(void) const
    { return m_sheetData; }
    inline uint64_t numSprites(void) const
    { return m_sheetData.size(); }
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
    eastl::vector<GDRSprite> m_sheetData;

    uint64_t m_sheetWidth;
    uint64_t m_sheetHeight;
    uint64_t m_tileWidth;
    uint64_t m_tileHeight;
    
    int32_t m_firstGid;
};

class GDRTileset
{
public:
    inline GDRTileset() = default;
    ~GDRTileset() = default;

    void Parse(eastl::shared_ptr<GDRMap>& mapData, const json& data, const eastl::vector<json>& tilesets);

    inline const eastl::vector<eastl::vector<GDRTile>>& getTiles(void) const
    { return m_tiles; }
    inline const eastl::shared_ptr<GDRMap>& getMap(void) const
    { return m_mapData; }
    inline eastl::shared_ptr<GDRMap>& getMap(void)
    { return m_mapData; }
    inline const eastl::shared_ptr<GDRTileSheet>& getSpriteData(void) const
    { return m_spriteData; }
    inline eastl::shared_ptr<GDRTileSheet>& getSpriteData(void)
    { return m_spriteData; }
    inline const std::string& getSource(void) const
    { return m_source; }
    inline const std::string& getName(void) const
    { return m_name; }
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
    eastl::shared_ptr<GDRMap> m_mapData;
    eastl::shared_ptr<GDRTileSheet> m_spriteData;
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

class GDRMapObject
{
public:
    GDRMapObject() = default;
    virtual ~GDRMapObject() = default;

    virtual void ParseBase(eastl::shared_ptr<GDRMap>& mapData, const json& data, const std::string& typeStr);
    virtual void Parse(eastl::shared_ptr<GDRMap>& mapData, const json& data) = 0;
    virtual uint64_t getType(void) const = 0;

    virtual inline const eastl::shared_ptr<GDRMap>& getMap(void) const
    { return m_mapData; }
    virtual inline eastl::shared_ptr<GDRMap>& getMap(void)
    { return m_mapData; }
    virtual inline const std::string& getName(void) const
    { return m_name; }
    virtual inline int64_t getId(void) const
    { return m_id; }
    virtual inline int64_t getGid(void) const
    { return m_gid; }
    virtual inline double getWidth(void) const
    { return m_width; }
    virtual inline double getHeight(void) const
    { return m_height; }
    virtual inline double getRotation(void) const
    { return m_rotation; }
    virtual inline double getX(void) const
    { return m_x; }
    virtual inline double getY(void) const
    { return m_y; }
    virtual inline bool isVisible(void) const
    { return m_visible; }
protected:
    GDRMapPropertyList m_propertyList;
    eastl::shared_ptr<GDRMap> m_mapData;
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

class GDRRectObject : public GDRMapObject
{
public:
    virtual void Parse(eastl::shared_ptr<GDRMap>& mapData, const json& data) override;
    virtual inline uint64_t getType(void) const override
    { return MAP_OBJECT_RECT; }
};

class GDRPointObject : public GDRMapObject
{
public:
    virtual void Parse(eastl::shared_ptr<GDRMap>& mapData, const json& data) override;
    virtual inline uint64_t getType(void) const override
    { return MAP_OBJECT_POINT; }

    inline const GDRMapPoint& getPoint(void) const
    { return m_point; }
    inline GDRMapPoint& getPoint(void)
    { return m_point; }
    inline double getX(void) const
    { return m_point.x; }
    inline double getY(void) const
    { return m_point.y; }
private:
    GDRMapPoint m_point;
};

class GDRPolylineObject : public GDRMapObject
{
public:
    typedef eastl::vector<GDRMapPoint>::iterator point_iterator;
    typedef eastl::vector<GDRMapPoint>::const_iterator const_point_iterator;
    typedef eastl::vector<GDRMapPoint>::reverse_iterator reverse_point_iterator;
    typedef eastl::vector<GDRMapPoint>::const_reverse_iterator const_reverse_point_iterator;
public:
    virtual void Parse(eastl::shared_ptr<GDRMap>& mapData, const json& data) override;
    virtual inline uint64_t getType(void) const override
    { return MAP_OBJECT_POLYLINE; }

    inline const eastl::vector<GDRMapPoint>& getPoints(void) const
    { return m_pointList; }
    inline eastl::vector<GDRMapPoint>& getPoints(void)
    { return m_pointList; }
    inline uint64_t getNumPoints(void) const
    { return m_pointList.size(); }
    
    inline point_iterator begin(void)
    { return m_pointList.begin(); }
    inline point_iterator end(void)
    { return m_pointList.end(); }
    inline const_point_iterator cbegin(void) const
    { return m_pointList.cbegin(); }
    inline const_point_iterator cend(void) const
    { return m_pointList.cend(); }
    inline reverse_point_iterator rbegin(void)
    { return m_pointList.rbegin(); }
    inline reverse_point_iterator rend(void)
    { return m_pointList.rend(); }
    inline const_reverse_point_iterator crbegin(void) const
    { return m_pointList.crbegin(); }
    inline const_reverse_point_iterator crend(void) const
    { return m_pointList.crend(); }
private:
    eastl::vector<GDRMapPoint> m_pointList;
};

class GDRPolygonObject : public GDRMapObject
{
public:
    typedef eastl::vector<GDRMapPoint>::iterator point_iterator;
    typedef eastl::vector<GDRMapPoint>::const_iterator const_point_iterator;
    typedef eastl::vector<GDRMapPoint>::reverse_iterator reverse_point_iterator;
    typedef eastl::vector<GDRMapPoint>::const_reverse_iterator const_reverse_point_iterator;
public:
    virtual void Parse(eastl::shared_ptr<GDRMap>& mapData, const json& data) override;
    virtual inline uint64_t getType(void) const override
    { return MAP_OBJECT_POLYGON; }

    inline const eastl::vector<GDRMapPoint>& getPoints(void) const
    { return m_pointList; }
    inline eastl::vector<GDRMapPoint>& getPoints(void)
    { return m_pointList; }
    inline uint64_t getNumPoints(void) const
    { return m_pointList.size(); }
    
    inline point_iterator begin(void)
    { return m_pointList.begin(); }
    inline point_iterator end(void)
    { return m_pointList.end(); }
    inline const_point_iterator cbegin(void) const
    { return m_pointList.cbegin(); }
    inline const_point_iterator cend(void) const
    { return m_pointList.cend(); }
    inline reverse_point_iterator rbegin(void)
    { return m_pointList.rbegin(); }
    inline reverse_point_iterator rend(void)
    { return m_pointList.rend(); }
    inline const_reverse_point_iterator crbegin(void) const
    { return m_pointList.crbegin(); }
    inline const_reverse_point_iterator crend(void) const
    { return m_pointList.crend(); }
private:
    eastl::vector<GDRMapPoint> m_pointList;
};

class GDRTextObject : public GDRMapObject
{
public:
    virtual void Parse(eastl::shared_ptr<GDRMap>& mapData, const json& data) override;
    virtual inline uint64_t getType(void) const override
    { return MAP_OBJECT_TEXT; }

    inline const std::string& getText(void) const
    { return m_texBuffer; }
    inline const std::string& getFont(void) const
    { return m_fontFamily; }
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
    eastl::shared_ptr<GDRMap> m_mapData;
    std::string m_texBuffer;
    std::string m_fontFamily;

    glm::vec4 m_color;

    uint64_t m_pixelSize;
    uint64_t m_horzAlign;
    uint64_t m_vertAlign;
    uint64_t m_flags;
};

class GDRMapLayer
{
public:
    GDRMapLayer() = default;
    virtual ~GDRMapLayer() = default;

    virtual void ParseBase(eastl::shared_ptr<GDRMap>& mapData, const json& data, const std::string& typeStr);
    virtual void Parse(eastl::shared_ptr<GDRMap>& mapData, const json& data) = 0;
    virtual uint64_t getType(void) const = 0;

    virtual inline const eastl::shared_ptr<GDRMap>& getMap(void) const
    { return m_mapData; }
    virtual inline eastl::shared_ptr<GDRMap>& getMap(void)
    { return m_mapData; }
    virtual inline const std::string& getName(void) const
    { return m_name; }
    virtual inline const GDRMapPropertyList& getProperties(void) const
    { return m_properties; }
    virtual inline bool isVisible(void) const
    { return m_visible; }
    virtual inline uint64_t getWidth(void) const
    { return m_width; }
    virtual inline uint64_t getHeight(void) const
    { return m_height; }
    virtual inline uint64_t getX(void) const
    { return m_x; }
    virtual inline uint64_t getY(void) const
    { return m_y; }
    virtual inline double getOpacity(void) const
    { return m_opacity; }
protected:
    GDRMapPropertyList m_properties;
    eastl::shared_ptr<GDRMap> m_mapData;
    std::string m_name;

    uint64_t m_width;
    uint64_t m_height;
    uint64_t m_y;
    uint64_t m_x;
    uint64_t m_type;
    double m_opacity;
    bool m_visible;
};

class GDRImageLayer : public GDRMapLayer
{
public:
    virtual void Parse(eastl::shared_ptr<GDRMap>& mapData, const json& data) override;
    virtual inline uint64_t getType(void) const override
    { return MAP_LAYER_IMAGE; }

    inline const std::string& getImage(void) const
    { return m_image; }
    inline uint64_t getTextureHash(void) const
    { return m_texHash; }
private:
    std::string m_image;
    uint64_t m_texHash;
};

// also an objectlayer
class GDRObjectGroup : public GDRMapLayer
{
public:
    virtual void Parse(eastl::shared_ptr<GDRMap>& mapData, const json& data) override;
    virtual inline uint64_t getType(void) const override
    { return MAP_LAYER_OBJECT; }

    inline const eastl::vector<eastl::shared_ptr<GDRMapObject>>& getObjects(void) const
    { return m_objectList; }
    inline uint64_t numObjects(void) const
    { return m_objectList.size(); }
    inline uint64_t getDrawOrder(void) const
    { return m_drawOrder; }
private:
    eastl::vector<eastl::shared_ptr<GDRMapObject>> m_objectList;

    uint64_t m_drawOrder;
};

class GDRGroupLayer : public GDRMapLayer
{
public:
    virtual void Parse(eastl::shared_ptr<GDRMap>& mapData, const json& data) override;
    virtual inline uint64_t getType(void) const override
    { return MAP_LAYER_GROUP; }

    inline void AddLayer(const eastl::shared_ptr<GDRMapLayer>& layerData)
    { m_layerList.emplace_back(layerData); }
    inline void AddChild(const eastl::shared_ptr<GDRMapLayer>& layerData)
    { AddLayer(layerData); }

    inline const eastl::shared_ptr<GDRMapLayer>& getChild(uint64_t index) const
    { return m_layerList[index]; }
    inline eastl::shared_ptr<GDRMapLayer>& getChild(uint64_t index)
    { return m_layerList[index]; }
    inline const eastl::vector<eastl::shared_ptr<GDRMapLayer>>& getChildren(void) const
    { return m_layerList; }
    inline eastl::vector<eastl::shared_ptr<GDRMapLayer>>& getChildren(void)
    { return m_layerList; }
    inline uint64_t getNumChildren(void) const
    { return m_layerList.size(); }
    inline uint64_t getOffsetX(void) const
    { return m_offsetX; }
    inline uint64_t getOffsetY(void) const
    { return m_offsetY; }
private:
    void ParseLayer(const json& data);
    eastl::vector<eastl::shared_ptr<GDRMapLayer>> m_layerList;

    uint64_t m_offsetX;
    uint64_t m_offsetY;
};

class GDRTileLayer : public GDRMapLayer
{
public:
    virtual void Parse(eastl::shared_ptr<GDRMap>& mapData, const json& data) override;
    virtual inline uint64_t getType(void) const override
    { return MAP_LAYER_TILE; }

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

    GDRTile *m_tileData;

    uint64_t m_dataFmt;
};

class GDRMap
{
public:
    GDRMap() = default;
    ~GDRMap() = default;

    inline void Parse(const json& data, const bfflevel_t *lvl)
    {
        m_levelData = lvl;
        m_width.store((uint64_t)data.at("width"));
        m_height.store((uint64_t)data.at("height"));

        if (!json_contains("tilesets"))
            N_Error("Map_LoadMap: invalid tmj file, no tilesets found");
        if (!json_contains("layers"))
            N_Error("Map_LoadMap: invalid tmj file, no layers found");
        
        m_tilesets.reserve((uint64_t)data.at("tilesets").size());
        m_layers.reserve((uint64_t)data.at("layers").size());

        if (!json_contains("objects")) {
            Con_Printf("WARNING: tmj file %s has no objects", m_name.c_str());
            return;
        }
    }

    inline void AddLayer(const eastl::shared_ptr<GDRMapLayer>& layer)
    {
        boost::unique_lock<boost::mutex> lock{m_dataLock};
        m_layers.emplace_back(layer);
    }
    inline void AddObject(const eastl::shared_ptr<GDRMapObject>& object)
    {
        boost::unique_lock<boost::mutex> lock{m_dataLock};
        m_objects.emplace_back(object);
    }
    inline void AddTileset(const eastl::shared_ptr<GDRTileset>& tileset)
    {
        boost::unique_lock<boost::mutex> lock{m_dataLock};
        m_tilesets.emplace_back(tileset);
    }

    inline const eastl::vector<eastl::shared_ptr<GDRMapLayer>>& getLayers(void) const
    { return m_layers; }
    inline const eastl::vector<eastl::shared_ptr<GDRMapObject>>& getObjects(void) const
    { return m_objects; }
    inline const eastl::vector<eastl::shared_ptr<GDRTileset>>& getTilesets(void) const
    { return m_tilesets; }
    inline const std::string& getName(void) const
    { return m_name; }
    inline const bfflevel_t *getLevelData(void) const
    { return m_levelData; }

    inline uint64_t numLayers(void) const
    { return m_layers.size(); }
    inline uint64_t numObjects(void) const
    { return m_objects.size(); }
    inline uint64_t numTilesets(void) const
    { return m_tilesets.size(); }
    inline uint64_t getTilesetIndex(uint32_t gid) const
    {
        // cleanup the gid bit flags
        gid &= ~(TILE_FLIPPED_VERT | TILE_FLIPPED_DIAG | TILE_FLIPPED_HORZ);
        
        for (uint64_t i = 0; i < m_tilesets.size(); ++i) {
            if (gid >= m_tilesets[i]->getFirstGID()) {
                return i;
            }
        }

        return m_tilesets.size();
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
    eastl::vector<json> m_tsjBuffer;

    eastl::vector<eastl::shared_ptr<GDRMapLayer>> m_layers;
    eastl::vector<eastl::shared_ptr<GDRMapObject>> m_objects;
    eastl::vector<eastl::shared_ptr<GDRTileset>> m_tilesets;
    std::string m_name;

    const bfflevel_t *m_levelData;

    boost::mutex m_dataLock;

    eastl::atomic<uint64_t> m_width;
    eastl::atomic<uint64_t> m_height;
    eastl::atomic<bool> m_infinite;
};

#endif