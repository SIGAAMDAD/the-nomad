#include "rgl_local.h"

#define R_SETFLOAT4( name, v ) { R_SetFloat4(backend.frameShader,va("%s" name,(buf)),(v)); }
#define R_SETFLOAT3( name, v ) { R_SetFloat3(backend.frameShader,va("%s" name,(buf)),(v)); }
#define R_SETFLOAT( name, f ) { R_SetFloat(backend.frameShader,va("%s" name,(buf)),(f)); }

extern "C" void RE_AddDLight(const dlight_t *ref)
{
    char buf[128];

    snprintf(buf, sizeof(buf), "dlights[%i].", backend.numDLights);

    R_SETFLOAT("ambient", ref->ambient);
    R_SETFLOAT("brightness", ref->brightness);
    R_SETFLOAT("diffuse", ref->diffuse);
    R_SETFLOAT4("color", ref->color);
    R_SETFLOAT3("pos", ref->pos);
    R_SETFLOAT("specular", ref->specular);
    R_SETFLOAT("ltype", (float)ref->ltype);
}

extern "C" void RE_BuildTileSurf(void)
{
    int64_t y, x;
    int64_t startX, startY;
    int64_t endX, endY;

    // surface we're rendering to
    tileSurf_t *surf;

    surf = rg.tileSurf;
    surf->numTilesX = r_fovWidth->i;
    surf->numTilesY = r_fovHeight->i;

    startX = (uint32_t)rg.plRef->worldPos[0] - r_fovWidth->i;
    startY = (uint32_t)rg.plRef->worldPos[1] - r_fovHeight->i;

    endY = (uint32_t)rg.plRef->worldPos[0] + r_fovWidth->i;
    endX = (uint32_t)rg.plRef->worldPos[1] + r_fovHeight->i;

    surf->tiles = (tile_t **)R_FrameAlloc(sizeof(*surf->tiles) * r_fovWidth->i * r_fovHeight->i);
    for (y = startY; y <= endX; ++y) {
        for (x = startX; x <= endX; ++x) {
            if ((y < 0 || y >= rg.mapData->mapHeight) || (x < 0 || x >= rg.mapData->mapWidth)) {
                surf->tiles[y * rg.mapData->mapWidth + x] = NULL;
            }
            else {
                surf->tiles[y * rg.mapData->mapWidth + x] = &rg.mapData->tilemapData[y * rg.mapData->mapWidth + x];
            }
        }
    }
}

extern "C" void RE_DrawTileSurf(const tileSurf_t *surf)
{
    uint32_t y, x, i;
    uint32_t gid;
    drawVert_t vertices[6], *v, *vert;
    qboolean warning = qfalse;
    const texcoord_t *texcoords;

    R_BindTexture(rg.textures[rg.mapData->texHandle]);
    const uint32_t indices[] = {
        0, 1, 2,
        2, 3, 0
    };

    /*
    DONT MODIFY THIS!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    */
#if 0 // works
    drawVert_t vertices[4] = {
        {{0.0f, 0.0f, 0.0f, 0.0f}, { 0.5f,  0.5f, 0.0f}, {1.0f, 1.0f}, {0.0f, 0.0f}},
        {{0.0f, 0.0f, 0.0f, 0.0f}, { 0.5f, -0.5f, 0.0f}, {0.0f, 1.0f}, {0.0f, 0.0f}},
        {{0.0f, 0.0f, 0.0f, 0.0f}, {-0.5f, -0.5f, 0.0f}, {0.0f, 0.0f}, {0.0f, 0.0f}},
        {{0.0f, 0.0f, 0.0f, 0.0f}, {-0.5f,  0.5f, 0.0f}, {1.0f, 0.0f}, {0.0f, 0.0f}},
    };
//#else
    drawVert_t vertices[4] = {
        {{0.0f, 0.0f, 0.0f, 0.0f}, { 0.5f,  0.5f, 0.0f}, {1.0f, 0.0f}, {0.0f, 0.0f}},
        {{0.0f, 0.0f, 0.0f, 0.0f}, { 0.5f, -0.5f, 0.0f}, {1.0f, 1.0f}, {0.0f, 0.0f}},
        {{0.0f, 0.0f, 0.0f, 0.0f}, {-0.5f, -0.5f, 0.0f}, {0.0f, 1.0f}, {0.0f, 0.0f}},
        {{0.0f, 0.0f, 0.0f, 0.0f}, {-0.5f,  0.5f, 0.0f}, {0.0f, 0.0f}, {0.0f, 0.0f}},
    };
#endif

    nglBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    backend.frameCache->indices = backend.indices;
    v = (drawVert_t *)R_FrameAlloc(sizeof(*v) * rg.mapData->mapWidth * rg.mapData->mapHeight * 4);
    vert = v;

    R_BindCache(backend.frameCache);
    for (y = 0; y < rg.mapData->mapHeight; ++y) {
        for (x = 0; x < rg.mapData->mapWidth; ++x) {
            // convert the world/tilemap coordinates to OpenGL screen coordinates
            RB_ConvertCoords(vert, glm::vec3( x - (rg.mapData->mapWidth * 0.5f), rg.mapData->mapHeight - y, 0.0f ), 4);

            // fetch the gid
//            gid = rg.mapData->tilemapData[y * rg.mapData->mapWidth + x][0];


//            texcoords = ri.Map_GetSpriteCoords(gid);
//            if (!texcoords) { // uh oh...
//                for (i = 0; i < 4; ++i) {
//                    memset(vert[i].texcoords, 0, sizeof(vert[i].texcoords));
//                }
//                Con_Printf(WARNING, "Bad gid: %i", gid);
//              if (r_debug)
//                  Con_Printf(WARNING, "Bad gid: %i", rg.);
//            }
//            else {
                for (i = 0; i < 4; ++i) {
                    memcpy(vert[i].texcoords, rg.mapData->tileCoords[rg.mapData->tilemapData[y * rg.mapData->mapWidth + x]].v[i], sizeof(vec2_t));
                }
//            }
            vert += 4;
        }
    }
    nglBufferSubDataARB(GL_ARRAY_BUFFER_ARB, 0, sizeof(*v) * rg.mapData->mapWidth * rg.mapData->mapHeight * 4, v);
    nglBufferSubDataARB(GL_ELEMENT_ARRAY_BUFFER_ARB, 0, sizeof(uint32_t) * rg.mapData->mapWidth * rg.mapData->mapHeight * 6, backend.indices);
    nglDrawElements(GL_TRIANGLES, rg.mapData->mapHeight * rg.mapData->mapWidth * 6, GL_UNSIGNED_INT, NULL);
    R_UnbindCache();
    R_UnbindTexture();
}

extern "C" void RE_RenderTilemap(void)
{
    rg.tileSurf = (tileSurf_t *)R_FrameAlloc(sizeof(*rg.tileSurf));
    RE_BuildTileSurf();
    RE_DrawTileSurf(rg.tileSurf);
}

extern "C" void RB_ConvertCoords(drawVert_t *v, const glm::vec3& pos, uint32_t count)
{
    uint32_t i;
    glm::mat4 model, mvp, identity = glm::mat4(1.0f);
    const glm::vec4 *positions;

    constexpr glm::vec4 p[4] = {
        { 0.5f,  0.5f, 0.0f, 1.0f},
        { 0.5f, -0.5f, 0.0f, 1.0f},
        {-0.5f, -0.5f, 0.0f, 1.0f},
        {-0.5f,  0.5f, 0.0f, 1.0f},
    };
    positions = p;

    model = glm::translate(identity, pos);
    mvp = rg.camera.vpm * mvp;

    for (i = 0; i < count; ++i) {
        const glm::vec3 p = mvp * positions[i];
        VectorCopy(v[i].pos, p);
    }
}
