// rgl_scene.c -- only one scene per frame. All rendering data submissions must come through here or rgl_cmd.c

#include "rgl_local.h"

uint64_t r_numEntities;
uint64_t r_firstSceneEntity;

uint64_t r_numDLights;
uint64_t r_firstSceneDLight;

uint64_t r_numPolys;
uint64_t r_firstScenePoly;

uint64_t r_numPolyVerts;

void R_InitNextFrame(void)
{
    backendData->commandList.usedBytes = 0;

    r_firstSceneDLight = 0;
    r_numDLights = 0;

    r_firstSceneEntity = 0;
    r_numEntities = 0;

    r_firstScenePoly = 0;
    r_numPolys = 0;

    r_numPolyVerts = 0;
}

GDR_EXPORT void RE_AddPolyToScene(nhandle_t hShader, const polyVert_t *verts, uint32_t numVerts)
{
    uint32_t i, offset;
    polyVert_t *vt;

    if (!rg.registered) {
        return;
    }

    if (r_numPolyVerts + numVerts >= r_maxPolys->i * 4) {
        ri.Printf(PRINT_DEVELOPER, "RE_AddPolyToScene: r_maxPolyVerts hit, dropping %i vertices\n", numVerts);
        return;
    }

    vt = &backendData->polyVerts[r_numPolyVerts];
    memcpy(vt, verts, sizeof(*vt) * numVerts);

    // generate fan indexes into the buffer
    for (i = 0; i < numVerts - 2; i++) {
        backendData->indices[backendData->numIndices + 0] = r_numPolyVerts;
        backendData->indices[backendData->numIndices + 1] = r_numPolyVerts + i + 1;
        backendData->indices[backendData->numIndices + 2] = r_numPolyVerts + i + 2;
        backendData->numIndices += 3;
    }

    r_numPolyVerts += numVerts;
    r_numPolys++;
}

GDR_EXPORT void RE_AddPolyListToScene(const poly_t *polys, uint32_t numPolys)
{
    uint32_t i;

    if (!rg.registered) {
        return;
    }

    if (r_numPolys + numPolys >= r_maxPolys->i) {
        ri.Printf(PRINT_DEVELOPER, "RE_AddPolyListToScene: r_maxPolys hit, dropping %i polygons\n", numPolys);
        return;
    }

    for (i = 0; i < numPolys; i++) {
        RE_AddPolyToScene(polys[i].hShader, polys[i].verts, polys[i].numVerts);
    }
}

GDR_EXPORT void RE_AddEntityToScene( const renderEntityRef_t *ent )
{
    if (!rg.registered) {
        return;
    }

    if (r_numEntities >= MAX_RENDER_ENTITIES) {
        ri.Printf(PRINT_DEVELOPER, "RE_AddEntityToScene: MAX_RENDER_ENTITIES hit, dropping entity\n");
        return;
    }
    if ( N_isnan(ent->origin[0]) || N_isnan(ent->origin[1]) || N_isnan(ent->origin[2]) ) {
		static qboolean firstTime = qtrue;
		if (firstTime) {
			firstTime = qfalse;
			ri.Printf( PRINT_INFO, COLOR_YELLOW "RE_AddEntityToScene passed a refEntity which has an origin with a NaN component\n");
		}
		return;
	}

    backendData->entities[r_numEntities].e = *ent;

    r_numEntities++;
}

GDR_EXPORT void RE_BeginScene(const renderSceneRef_t *fd)
{
    assert(fd);

    if (!rg.world && !(fd->flags & RSF_NOWORLDMODEL)) {
        ri.Error(ERR_DROP, "RE_RenderScene: no world loaded");
    }

    backend.refdef.x = fd->x;
    backend.refdef.y = fd->y;
    backend.refdef.width = fd->width;
    backend.refdef.height = fd->height;
    backend.refdef.flags = fd->flags;

    backend.refdef.numDLights = r_firstSceneDLight - r_numDLights;
    backend.refdef.dlights = backendData->dlights;
    
    backend.refdef.numEntities = r_firstSceneEntity - r_numEntities;
    backend.refdef.entities = &backendData->entities[r_firstSceneEntity];

    backend.refdef.drawn = qfalse;
}

GDR_EXPORT void RE_ClearScene(void)
{
    r_firstSceneDLight = r_numDLights;
    r_firstSceneEntity = r_numEntities;
    r_firstScenePoly = r_numPolys;
}

GDR_EXPORT void RE_EndScene(void)
{
    r_firstSceneDLight = r_numDLights;
    r_firstSceneEntity = r_numEntities;
    r_firstScenePoly = r_numPolys;
}

GDR_EXPORT void RE_RenderScene( const renderSceneRef_t *fd )
{
    viewData_t parms;
    uint64_t startTime;

    if (!rg.registered) {
        return;
    }

    startTime = ri.Milliseconds();

    if (!rg.world && !(fd->flags & RSF_NOWORLDMODEL)) {
        ri.Error(ERR_FATAL, "RE_RenderScene: no world loaded");
    }

    RE_BeginScene(fd);

    memset(&parms, 0, sizeof(parms));
    parms.viewportX = backend.refdef.x;
    parms.viewportY = backend.refdef.y;
    parms.viewportWidth = backend.refdef.width;
    parms.viewportHeight = backend.refdef.height;
    parms.camera = glState.viewData.camera;

    R_RenderView( &parms );

    RE_EndScene();
}


