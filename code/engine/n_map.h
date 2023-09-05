#ifndef _N_MAP_
#define _N_MAP_

#pragma once

// 4 gpu-style texture coordinates
typedef struct
{
    vec2_t v[4];
} texcoord_t;

// gid, flags
typedef uint32_t tile_t[2];
// x, y
typedef uint32_t checkpoint_t[2];
// x, y, entity type, specific entity id
typedef uint32_t spawn_t[4];
// an entity id
typedef uint32_t sprite_t;

#ifdef __cplusplus

#include "../game/g_bff.h"

class GDRMap;
class GDRMapLayer;
class GDRMapObject;

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


enum : uint32_t {
    TILE_FLIPPED_HORZ = 0x80000000,
    TILE_FLIPPED_VERT = 0x40000000,
    TILE_FLIPPED_DIAG = 0x20000000
};

#define json_contains(key) data.contains(key)

// a simple wrapper around json tilemaps

void Com_CacheMaps(void);

uint32_t* Map_LoadBase64(const json& data, uint64_t width, uint64_t height, uint64_t *m_dataFmt);
uint32_t* Map_LoadCSV(const json& data, uint64_t width, uint64_t height);
static char *Map_DecompressGZIP(const char *data, uint64_t inlen, uint64_t *outlen, const uint64_t expectedLen);
const eastl::string& base64_decode(const eastl::string& in);
const eastl::string& base64_encode(uint8_t *out, uint32_t in_len);

typedef struct
{
#ifdef __cplusplus
    const json *tsj, *tmj;
#endif
    char *name;

    tile_t *tilemapData;
    texcoord_t *tileCoords;
    uint32_t *tilesetData;

    nhandle_t texHandle;

    uint64_t numCheckpoints;
    checkpoint_t *checkpoints;

    uint64_t numSpawns;
    spawn_t *spawns;

    char *tilesetName;
    bfftexture_t *tileset;

    uint64_t tileDataFmt;
    uint64_t tileWidth;
    uint64_t tileHeight;
    uint64_t tileCountX;
    uint64_t tileCountY;
    uint64_t imageWidth;
    uint64_t imageHeight;
    uint64_t mapWidth;
    uint64_t mapHeight;

    uint32_t firstGid;
} nmap_t;


typedef struct
{
    uint32_t startTime;
    uint32_t endTime;
    uint32_t kills;
    uint32_t longestCombo;
} levelStats_t;

typedef struct
{
    levelStats_t stats;     // level-by-level statistics
    nmap_t *mapData;        // a pointer to the cached map data
    char *mapName;          // the tmj chunk name
    char *levelName;        // the chunk name
    char *longName;         // the name on screen
    char *textureName;      // name of the texture file to use when rendering tiles
    vec4_t background;

    int levelIndex;
} nlevel_t;

extern nlevel_t *level;

const texcoord_t* Map_GetSpriteCoords(uint32_t gid);
void LVL_Shutdown(void);

#endif

#endif
