#include "rgl_local.h"

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
    out = ri.Hunk_Alloc( sizeof(*out) * count, h_low );

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
    out = ri.Hunk_Alloc( sizeof(*out) * count, h_low );

    r_worldData.tiles = out;
    r_worldData.numTiles = count;

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
    out = ri.Hunk_Alloc( sizeof(*out) * count, h_low );

    r_worldData.sprites = out;
    r_worldData.numTilesetSprites = count;

    memcpy(out, in, count*sizeof(*out));
}

void R_GenerateDrawData(void)
{
    maptile_t *tile;
    uint32_t numSurfs, i;
    uint32_t offset;
    vertexAttrib_t *attribs;

    r_worldData.numIndices = r_worldData.width * r_worldData.height * 6;
    r_worldData.numVertices = r_worldData.width * r_worldData.height * 4;

    r_worldData.indices = ri.Hunk_Alloc( sizeof(glIndex_t) * r_worldData.numIndices, h_low );
    r_worldData.vertices = ri.Hunk_Alloc( sizeof(drawVert_t) * r_worldData.numVertices, h_low );

    r_worldData.buffer = R_AllocateBuffer( "worldDrawBuffer", NULL, r_worldData.numVertices * sizeof(drawVert_t), NULL,
                                        r_worldData.numIndices * sizeof(glIndex_t), BUFFER_FRAME );
    attribs = r_worldData.buffer->attribs;

    attribs[ATTRIB_INDEX_POSITION].enabled		= qtrue;
	attribs[ATTRIB_INDEX_TEXCOORD].enabled		= qtrue;
	attribs[ATTRIB_INDEX_COLOR].enabled			= qtrue;
	attribs[ATTRIB_INDEX_NORMAL].enabled		= qfalse;
    attribs[ATTRIB_INDEX_WORLDPOS].enabled      = qtrue;

	attribs[ATTRIB_INDEX_POSITION].count		= 3;
	attribs[ATTRIB_INDEX_TEXCOORD].count		= 2;
	attribs[ATTRIB_INDEX_COLOR].count			= 4;
	attribs[ATTRIB_INDEX_NORMAL].count			= 4;
    attribs[ATTRIB_INDEX_WORLDPOS].count        = 3;

	attribs[ATTRIB_INDEX_POSITION].type			= GL_FLOAT;
	attribs[ATTRIB_INDEX_TEXCOORD].type			= GL_FLOAT;
	attribs[ATTRIB_INDEX_COLOR].type			= GL_UNSIGNED_SHORT;
	attribs[ATTRIB_INDEX_NORMAL].type			= GL_SHORT;
    attribs[ATTRIB_INDEX_WORLDPOS].type         = GL_FLOAT;

	attribs[ATTRIB_INDEX_POSITION].index		= ATTRIB_INDEX_POSITION;
	attribs[ATTRIB_INDEX_TEXCOORD].index		= ATTRIB_INDEX_TEXCOORD;
	attribs[ATTRIB_INDEX_COLOR].index			= ATTRIB_INDEX_COLOR;
	attribs[ATTRIB_INDEX_NORMAL].index			= ATTRIB_INDEX_NORMAL;
    attribs[ATTRIB_INDEX_WORLDPOS].index        = ATTRIB_INDEX_WORLDPOS;

	attribs[ATTRIB_INDEX_POSITION].normalized	= GL_FALSE;
	attribs[ATTRIB_INDEX_TEXCOORD].normalized	= GL_FALSE;
	attribs[ATTRIB_INDEX_COLOR].normalized		= GL_TRUE;
	attribs[ATTRIB_INDEX_NORMAL].normalized		= GL_TRUE;
    attribs[ATTRIB_INDEX_WORLDPOS].normalized   = GL_FALSE;

	attribs[ATTRIB_INDEX_POSITION].offset		= offsetof( drawVert_t, xyz );
	attribs[ATTRIB_INDEX_TEXCOORD].offset		= offsetof( drawVert_t, uv );
	attribs[ATTRIB_INDEX_COLOR].offset			= offsetof( drawVert_t, color );
	attribs[ATTRIB_INDEX_NORMAL].offset			= 0;
    attribs[ATTRIB_INDEX_WORLDPOS].offset       = offsetof( drawVert_t, worldPos );

	attribs[ATTRIB_INDEX_POSITION].stride		= sizeof(drawVert_t);
	attribs[ATTRIB_INDEX_TEXCOORD].stride		= sizeof(drawVert_t);
	attribs[ATTRIB_INDEX_COLOR].stride			= sizeof(drawVert_t);
	attribs[ATTRIB_INDEX_NORMAL].stride			= sizeof(drawVert_t);
    attribs[ATTRIB_INDEX_WORLDPOS].stride       = sizeof(drawVert_t);

    // cache the indices so that we aren't calculating these every frame (there could be thousands)
    for ( i = 0, offset = 0; i < r_worldData.numIndices; i += 6, offset += 4 ) {
        r_worldData.indices[i + 0] = offset + 0;
        r_worldData.indices[i + 1] = offset + 1;
        r_worldData.indices[i + 2] = offset + 2;

        r_worldData.indices[i + 3] = offset + 3;
        r_worldData.indices[i + 4] = offset + 2;
        r_worldData.indices[i + 5] = offset + 0;
    }

    // copy the pre-calculated texture coordinates over
    for ( uint32_t y = 0; y < r_worldData.height; y++ ) {
        for ( uint32_t x = 0; x < r_worldData.width; x++ ) {
            VectorCopy2( r_worldData.vertices[(y * r_worldData.width + x) + 0].uv, r_worldData.tiles[y * r_worldData.width + x].texcoords[0] );
            VectorCopy2( r_worldData.vertices[(y * r_worldData.width + x) + 1].uv, r_worldData.tiles[y * r_worldData.width + x].texcoords[1] );
            VectorCopy2( r_worldData.vertices[(y * r_worldData.width + x) + 2].uv, r_worldData.tiles[y * r_worldData.width + x].texcoords[2] );
            VectorCopy2( r_worldData.vertices[(y * r_worldData.width + x) + 3].uv, r_worldData.tiles[y * r_worldData.width + x].texcoords[3] );
        }
    }
}

GDR_EXPORT void RE_LoadWorldMap(const char *filename)
{
    bmf_t *header;
    mapheader_t *mheader;
    tile2d_header_t *theader;
    char texture[MAX_GDR_PATH];
    union {
        byte *b;
        void *v;
    } buffer;

    ri.Printf(PRINT_INFO, "------ RE_LoadWorldMap(%s) ------\n", filename);

    if (strlen(filename) >= MAX_GDR_PATH) {
        ri.Error(ERR_DROP, "RE_LoadWorldMap: name '%s' too long", filename);
    }

    if (rg.worldMapLoaded) {
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

    rg.worldMapLoaded = qtrue;

    memset(&r_worldData, 0, sizeof(r_worldData));
    N_strncpyz(r_worldData.name, filename, sizeof(r_worldData.name));
    N_strncpyz(r_worldData.baseName, COM_SkipPath(filename), sizeof(r_worldData.baseName));

    COM_StripExtension(r_worldData.baseName, r_worldData.baseName, sizeof(r_worldData.baseName));

    header = (bmf_t *)buffer.b;
    if (LittleInt(header->version) != LEVEL_VERSION) {
        ri.Error(ERR_DROP, "RE_LoadWorldMap: %s has the wrong version number (%i should be %i)",
            filename, LittleInt(header->version), LEVEL_VERSION);
    }

    fileBase = (byte *)header;

    mheader = &header->map;
    theader = &header->tileset;

    // swap all the lumps
    for (uint32_t i = 0; i < sizeof(bmf_t)/4; i++) {
        ((int32_t *)header)[i] = LittleInt( ((int32_t *)header)[i] );
    }

    r_worldData.width = mheader->mapWidth;
    r_worldData.height = mheader->mapHeight;
    r_worldData.numTiles = r_worldData.width * r_worldData.height;

    // load into heap
    R_LoadTiles(&mheader->lumps[LUMP_TILES]);
    R_LoadLights(&mheader->lumps[LUMP_LIGHTS]);

    R_LoadTileset(&mheader->lumps[LUMP_SPRITES], theader);

    R_GenerateDrawData();

    rg.world = &r_worldData;

    COM_StripExtension(theader->info.texture, texture, sizeof(texture));
    if (texture[ strlen(texture) - 1 ] == '.') {
        texture[ strlen(texture) - 1 ] = '\0';
    }

    rg.world->shader = R_FindShader( texture );
    if ( rg.world->shader == rg.defaultShader ) {
        ri.Error(ERR_DROP, "RE_LoadWorldMap: failed to load shader for '%s'", filename);
    }

    ri.FS_FreeFile( buffer.v );
}
