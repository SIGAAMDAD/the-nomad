#include "rgl_local.h"

static CRenderWorld r_worldData;
static byte *fileBase;

GDR_EXPORT void CRenderWorld::LoadLights(const lump_t *lights)
{
    uint32_t count;
    maplight_t *in, *out;

    in = (maplight_t *)(fileBase + lights->fileofs);
    if (lights->length % sizeof(*in))
        ri.Error(ERR_DROP, "RE_LoadWorldMap: funny lump size in %s", m_szName);
    
    count = lights->length / sizeof(*in);
    out = (maplight_t *)ri.Hunk_Alloc(count * sizeof(*out), h_low);

    m_pLights = out;
    m_nLights = count;

    memcpy(out, in, count*sizeof(*out));
}

GDR_EXPORT void CRenderWorld::LoadTiles(const lump_t *tiles)
{
    uint32_t count;
    maptile_t *in, *out;

    in = (maptile_t *)(fileBase + tiles->fileofs);
    if (tiles->length % sizeof(*in))
        ri.Error(ERR_DROP, "RE_LoadWorldMap: funny lump size in %s", m_szName);
    
    count = tiles->length / sizeof(*in);
    out = (maptile_t *)ri.Hunk_Alloc(count * sizeof(*out), h_low);

    m_pTiles = out;
    m_nTiles = count;

    memcpy(out, in, count*sizeof(*out));
}

/*
GDR_EXPORT void CRenderWorld::LoadCheckpoints(const lump_t *c)
{
    uint32_t count;
    mapcheckpoint_t *in, *out;

    in = (mapcheckpoint_t *)(fileBase + c->fileofs);
    if (c->length % sizeof(*in))
        ri.Error(ERR_DROP, "RE_LoadWorldMap: funny lump size in %s", m_szName);
    
    count = c->length / sizeof(*in);
    out = ri.Hunk_Alloc(count * sizeof(*out), h_low);

    m_pCheckpoints = out;
    m_nCheckpoints = count;

    memcpy(out, in, count*sizeof(*out));
}

GDR_EXPORT void CRenderWorld::LoadSpawns(const lump_t *s)
{
    uint32_t count;
    mapspawn_t *in, *out;

    in = (mapspawn_t *)(fileBase + s->fileofs);
    if (s->length % sizeof(*in))
        ri.Error(ERR_DROP, "RE_LoadWorldMap: funny lump size in %s", m_szName);
    
    count = s->length / sizeof(*in);
    out = ri.Hunk_Alloc(count * sizeof(*out), h_low);

    m_pSpawns = out;
    m_nSpawns = count;

    memcpy(out, in, count*sizeof(*out));
}
*/

GDR_EXPORT void CRenderWorld::LoadTileset(const lump_t *sprites, const tile2d_header_t *theader)
{
    uint32_t count;
    tile2d_sprite_t *in, *out;

    in = (tile2d_sprite_t *)(fileBase + sprites->fileofs);
    if (sprites->length % sizeof(*in))
        ri.Error(ERR_DROP, "RE_LoadWorldMap: funny lump size in %s", m_szName);
    
    count = sprites->length / sizeof(*in);
    out = (tile2d_sprite_t *)ri.Hunk_Alloc(count * sizeof(*out), h_low);

    m_pSprites = out;
    m_nTilesetSprites = count;

    memcpy(out, in, count*sizeof(*out));
}

GDR_EXPORT void CRenderWorld::GenerateDrawData(void)
{
    maptile_t *tile;
    uint32_t i, offset;

    m_nIndices = m_nWidth * m_nHeight * 6;
    m_nVertices = m_nWidth * m_nHeight * 4;

    m_pIndices = (glIndex_t *)ri.Hunk_Alloc( sizeof(glIndex_t) * m_nIndices, h_low );
    m_pVertices = (drawVert_t *)ri.Hunk_Alloc( sizeof(drawVert_t) * m_nVertices, h_low );

    // TODO: perhaps make it an option to make individual buffer usages? As the index data for the world
    // cache will never change
    m_pDrawBuffer = R_AllocateBuffer( "worldVtxBuffer", sizeof(drawVert_t) * m_nVertices, NULL, sizeof(glIndex_t) * m_nIndices, NULL, BUFFER_FRAME );
    m_pDrawBuffer->SetAttributes( {
        vertexAttrib_t(
            ATTRIB_INDEX_POSITION,
            3,
            GL_FLOAT,
            qtrue,
            GL_FALSE,
            sizeof(drawVert_t),
            offsetof(drawVert_t, xyz)
        ),
        vertexAttrib_t(
            ATTRIB_INDEX_TEXCOORD,
            2,
            GL_FLOAT,
            qtrue,
            GL_FALSE,
            sizeof(drawVert_t),
            offsetof(drawVert_t, uv)
        ),
        vertexAttrib_t(
            ATTRIB_INDEX_COLOR,
            4,
            GL_UNSIGNED_SHORT,
            qtrue,
            GL_TRUE,
            sizeof(drawVert_t),
            offsetof(drawVert_t, color)
        ),
        vertexAttrib_t(
            ATTRIB_INDEX_NORMAL,
            4,
            GL_SHORT,
            qfalse,
            GL_TRUE,
            sizeof(drawVert_t),
            offsetof(drawVert_t, normal)
        )
    } );

    for (i = 0, offset = 0; i < m_nIndices; i += 6, offset += 4) {
        m_pIndices[i + 0] = offset + 0;
        m_pIndices[i + 1] = offset + 1;
        m_pIndices[i + 2] = offset + 2;

        m_pIndices[i + 3] = offset + 3;
        m_pIndices[i + 4] = offset + 2;
        m_pIndices[i + 5] = offset + 0;
    }

    for (uint32_t y = 0; y < m_nHeight; y++) {
        for (uint32_t x = 0; x < m_nWidth; x++) {
            VectorCopy2( m_pVertices[(y * m_nWidth + x) + 0].uv, m_pTiles[y * m_nWidth + x].texcoords[0] );
            VectorCopy2( m_pVertices[(y * m_nWidth + x) + 1].uv, m_pTiles[y * m_nWidth + x].texcoords[1] );
            VectorCopy2( m_pVertices[(y * m_nWidth + x) + 2].uv, m_pTiles[y * m_nWidth + x].texcoords[2] );
            VectorCopy2( m_pVertices[(y * m_nWidth + x) + 3].uv, m_pTiles[y * m_nWidth + x].texcoords[3] );
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
    r_worldData.SetName( filename );

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

    r_worldData.SetSize( mheader->mapWidth, mheader->mapHeight );

    // load into heap
    r_worldData.LoadTiles(&mheader->lumps[LUMP_TILES]);
///    r_worldData.LoadCheckpoints(&mheader->lumps[LUMP_CHECKPOINTS]);
///    r_worldData.LoadSpawns(&mheader->lumps[LUMP_SPAWNS]);
    r_worldData.LoadLights(&mheader->lumps[LUMP_LIGHTS]);
    r_worldData.LoadTileset(&mheader->lumps[LUMP_SPRITES], theader);

    r_worldData.GenerateDrawData();

    rg.world = &r_worldData;

    COM_StripExtension(theader->info.texture, texture, sizeof(texture));
    if (texture[ strlen(texture) - 1 ] == '.') {
        texture[ strlen(texture) - 1 ] = '\0';
    }

    rg.world->m_pShader = R_FindShader(texture);
    if (rg.world->m_pShader == rg.defaultShader) {
        ri.Error(ERR_DROP, "RE_LoadWorldMap: failed to load shader for '%s'", filename);
    }

    ri.FS_FreeFile( buffer.v );
}
