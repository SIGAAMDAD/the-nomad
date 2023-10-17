#include "rgl_local.h"

/*
bmf files get really funky
*/
typedef struct {
    uint32_t ident;
    uint32_t version;
    tile2d_header_t tileHeader;
    mapheader_t mapData;
} lvlheader_t;

static world_t r_worldData;
static byte *fileBase;

static void R_LoadLights(const lump_t *lights)
{
    uint32_t count;
    maplight_t *in, *out;

    in = (maplight_t *)(fileBase + lights->fileofs);
    if (lights->length % sizeof(*in))
        ri.Error(ERR_DROP, "RE_LoadWorldMap: funny lump size in %s", r_worldData.name);
    
    count = lights->length / sizeof(*in);
    out = ri.Hunk_Alloc(count * sizeof(*out), h_low);

    r_worldData.lights = out;
    r_worldData.numLights = count;

    memcpy(out, in, count*sizeof(*out));
}

static void R_LoadTiles(const lump_t *tiles)
{
    uint32_t count;
    maptile_t *in, *out;

    in = (maptile_t *)(fileBase + tiles->fileofs);
    if (tiles->length % sizeof(*in))
        ri.Error(ERR_DROP, "RE_LoadWorldMap: funny lump size in %s", r_worldData.name);
    
    count = tiles->length / sizeof(*in);
    out = ri.Hunk_Alloc(count * sizeof(*out), h_low);

    r_worldData.tiles = out;
    r_worldData.numTiles = count;

    memcpy(out, in, count*sizeof(*out));
}

static void R_LoadCheckpoints(const lump_t *c)
{
    uint32_t count;
    mapcheckpoint_t *in, *out;

    in = (mapcheckpoint_t *)(fileBase + c->fileofs);
    if (c->length % sizeof(*in))
        ri.Error(ERR_DROP, "RE_LoadWorldMap: funny lump size in %s", r_worldData.name);
    
    count = c->length / sizeof(*in);
    out = ri.Hunk_Alloc(count * sizeof(*out), h_low);

    r_worldData.checkpoints = out;
    r_worldData.numCheckpoints = count;

    memcpy(out, in, count*sizeof(*out));
}

static void R_LoadSpawns(const lump_t *s)
{
    uint32_t count;
    mapspawn_t *in, *out;

    in = (mapspawn_t *)(fileBase + s->fileofs);
    if (s->length % sizeof(*in))
        ri.Error(ERR_DROP, "RE_LoadWorldMap: funny lump size in %s", r_worldData.name);
    
    count = s->length / sizeof(*in);
    out = ri.Hunk_Alloc(count * sizeof(*out), h_low);

    r_worldData.spawns = out;
    r_worldData.numSpawns = count;

    memcpy(out, in, count*sizeof(*out));
}

static void R_LoadTileset(const lump_t *sprites, const tile2d_header_t *theader)
{
    uint32_t count;
    tile2d_sprite_t *in, *out;

    in = (tile2d_sprite_t *)(fileBase + sprites->fileofs);
    if (sprites->length % sizeof(*in))
        ri.Error(ERR_DROP, "RE_LoadWorldMap: funny lump size in %s", r_worldData.name);
    
    count = sprites->length / sizeof(*in);
    out = ri.Hunk_Alloc(count * sizeof(*out), h_low);

    r_worldData.sprites = out;
    r_worldData.numTilesetSprites = count;

    memcpy(out, in, count*sizeof(*out));

    r_worldData.tileset;
}

void R_GenerateDrawData(void)
{
    srfTile_t *surfs, *surf;
    drawVert_t *vert;
    glIndex_t *idx;
    uint32_t numSurfs, i;
    uint32_t offset;

    r_worldData.numIndices = r_worldData.width * r_worldData.height * 6;
    r_worldData.numVertices = r_worldData.width * r_worldData.height * 4;

    r_worldData.indices = ri.Hunk_Alloc(sizeof(glIndex_t) * r_worldData.numIndices, h_low);
    r_worldData.vertices = ri.Hunk_Alloc(sizeof(drawVert_t) * r_worldData.numVertices, h_low);
    surfs = ri.Hunk_Alloc(sizeof(*surfs) * r_worldData.width * r_worldData.height, h_low);

    idx = r_worldData.indices;
    vert = r_worldData.vertices;

    for (i = 0, offset = 0; i < r_worldData.numIndices; i += 6, offset += 4) {
        idx[i + 0] = offset + 0;
        idx[i + 1] = offset + 1;
        idx[i + 2] = offset + 2;

        idx[i + 3] = offset + 3;
        idx[i + 4] = offset + 2;
        idx[i + 5] = offset + 0;
    }

    for (i = 0, surf = surfs, vert = r_worldData.vertices; i < numSurfs; surf++, i++, idx += 6, vert += 4) {
        surf->surface = SF_TILE;
        surf->indices = idx;
        surf->verts = vert;
    }
}

void GDR_EXPORT RE_LoadWorldMap(const char *filename)
{
    lvlheader_t *header;
    mapheader_t *mheader;
    tile2d_header_t *theader;
    union {
        byte *b;
        void *v;
    } buffer;

    if (strlen(filename) >= MAX_GDR_PATH) {
        ri.Error(ERR_DROP, "RE_LoadWorldMap: name '%s' too long", filename);
    }

    if (rg.worldLoaded) {
        ri.Error(ERR_DROP, "attempted to reduntantly load world map");
    }

    // load it
    ri.FS_LoadFile(filename, &buffer.v);
    if (!buffer.v) {
        ri.Error(ERR_DROP, "RE_LoadWorldMap: %s not found", filename);
    }

    // clear rg.world so if the level fails to load, the next
    // try will not look at the partially loaded version
    rg.world = NULL;

    rg.worldLoaded = qtrue;

    memset(&r_worldData, 0, sizeof(r_worldData));
    N_strncpyz(r_worldData.name, filename, sizeof(r_worldData.name));
    N_strncpyz(r_worldData.baseName, COM_SkipPath(filename), sizeof(r_worldData.baseName));

    COM_StripExtension(r_worldData.baseName, r_worldData.baseName, sizeof(r_worldData.baseName));

    header = (lvlheader_t *)buffer.b;
    if (LittleInt(header->version) != LEVEL_VERSION) {
        ri.Error(ERR_DROP, "RE_LoadWorldMap: %s has the wrong version number (%i should be %i)",
            filename, LittleInt(header->version), LEVEL_VERSION);
    }

    fileBase = (byte *)header;

    // swap all the lumps
    mheader = (mapheader_t *)((byte *)header + offsetof(lvlheader_t, mapData));
    for (uint64_t i = 0; i < sizeof(mapheader_t)/4; i++) {
        ((int *)mheader)[i] = LittleInt(((int *)mheader)[i]);
    }
    theader = (tile2d_header_t *)((byte *)header + offsetof(lvlheader_t, tileHeader));
    for (uint64_t i = 0; i < sizeof(tile2d_header_t)/4; i++) {
        ((int *)theader)[i] = LittleInt(((int *)theader)[i]);
    }

    // load into heap
    R_LoadTiles(&mheader->lumps[LUMP_TILES]);
    R_LoadCheckpoints(&mheader->lumps[LUMP_CHECKPOINTS]);
    R_LoadSpawns(&mheader->lumps[LUMP_SPAWNS]);
    R_LoadLights(&mheader->lumps[LUMP_LIGHTS]);

    R_LoadTileset(&mheader->lumps[LUMP_SPRITES], theader);

    R_GenerateDrawData();

    rg.world->shader = R_FindShader(theader->info.texture);
    if (rg.world->shader == rg.defaultShader) {
        ri.Error(ERR_DROP, "RE_LoadWorldMap: failed to load shader for '%s'", filename);
    }
    rg.world->tileset = rg.world->shader->texture;
}
