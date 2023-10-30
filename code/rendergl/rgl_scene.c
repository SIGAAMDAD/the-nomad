// rgl_scene.c -- only one scene per frame. All rendering data submissions must come through here or rgl_cmd.c

#include "rgl_local.h"

uint64_t r_numEntities;
uint64_t r_firstSceneEntity;

uint64_t r_numDLights;
uint64_t r_firstSceneDLight;

uint64_t r_numPolys;
uint64_t r_firstScenePoly;

uint64_t r_numPolyVerts;

uint64_t r_firstSceneDrawSurf;

void R_InitNextFrame(void)
{
    backendData->commandList.usedBytes = 0;

    r_firstSceneDrawSurf = 0;

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
    uint32_t i;
    polyVert_t *vt;

    if (!rg.registered) {
        return;
    }

    if (r_numPolyVerts + numVerts >= r_maxPolyVerts->i) {
        ri.Printf(PRINT_DEVELOPER, "RE_AddPolyToScene: r_maxPolyVerts hit, dropping %i vertices\n", numVerts);
        return;
    }

    vt = &backendData->polyVerts[r_numPolyVerts];
    memcpy(vt, verts, sizeof(*vt) * numVerts);

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
        ri.Printf(PRINT_DEVELOPER, "RE_AddPolyListToScene: r_maxPolys hit, dropping %i polys\n", numPolys);
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

    rg.refdef.x = fd->x;
    rg.refdef.y = fd->y;
    rg.refdef.width = fd->width;
    rg.refdef.height = fd->height;
    rg.refdef.flags = fd->flags;

    rg.refdef.numDLights = r_numDLights;
    rg.refdef.dlights = r_numDLights;
    
    rg.refdef.numEntities = r_numEntities;
    rg.refdef.entities = backendData->entities;

    rg.refdef.drawn = qfalse;
}

GDR_EXPORT void RE_ClearScene(void)
{
    r_firstSceneDLight = r_numDLights;
    r_firstSceneEntity = r_numEntities;
    r_firstScenePoly = r_numPolys;
}

GDR_EXPORT void RE_EndScene(void)
{
    r_firstSceneDLight = rg.refdef.numDLights;
    r_firstSceneEntity = r_numEntities;
    r_firstScenePoly = r_numPolys;
}

GDR_EXPORT void RE_RenderScene(const renderSceneRef_t *fd)
{
    viewData_t parms;
    uint64_t startTime;

    if (!rg.registered) {
        return;
    }

    startTime = ri.Milliseconds();

    if (!rg.world && !(fd->flags & RSF_NOWORLDMODEL)) {
        ri.Error(ERR_DROP, "RE_RenderScene: no world loaded");
    }

    RE_BeginScene(fd);

    memset(&parms, 0, sizeof(parms));
    parms.viewportX = rg.refdef.x;
    parms.viewportY = rg.refdef.y;
    parms.viewportWidth = rg.refdef.width;
    parms.viewportHeight = rg.refdef.height;

    R_RenderView(&parms);

    RE_EndScene();
}


void R_DrawElements(uint32_t numIndices, uintptr_t offset)
{
	nglDrawElements(GL_TRIANGLES, numIndices, GL_UNSIGNED_INT, NULL);
}

void RB_CheckOverflow( uint32_t verts, uint32_t indexes )
{
	if (drawBuf.numVertices + verts < MAX_BATCH_VERTICES
		&& drawBuf.numIndices + indexes < MAX_BATCH_INDICES) {
		return;
	}

	RB_EndSurface();

	if ( verts >= MAX_BATCH_VERTICES ) {
		ri.Error(ERR_DROP, "RB_CheckOverflow: verts > MAX (%d > %d)", verts, MAX_BATCH_VERTICES );
	}
	if ( indexes >= MAX_BATCH_INDICES ) {
		ri.Error(ERR_DROP, "RB_CheckOverflow: indices > MAX (%d > %d)", indexes, MAX_BATCH_INDICES );
	}

	RB_BeginSurface(drawBuf.shader);
}

static void RB_CheckVao(vertexBuffer_t *buf)
{
	if (buf != glState.currentVao) {
		RB_EndSurface();
		RB_BeginSurface(drawBuf.shader);

		VBO_Bind(buf);
	}
	if (buf != drawBuf.buf)
		drawBuf.useInternalVao = qfalse;
}

void R_AddPolySurfs(void)
{
	const srfPoly_t *poly;
	uint32_t i;
	shader_t *sh;
	
	for (i = 0, poly = backendData->polys; i < backendData->numPolys; i++, poly++) {
		sh = R_GetShaderByHandle(poly->hShader);
		R_AddDrawSurf((void *)poly, sh);
	}
}

void R_ConvertCoords(vec3_t verts[4], vec3_t pos)
{
	mat4_t model, mvp;
	vec4_t p;

	const vec4_t positions[4] = {
		{  0.5f, -0.5f, 0.0f, 1.0f },
		{ -0.5f, -0.5f, 0.0f, 1.0f },
		{ -0.5f,  0.5f, 0.0f, 1.0f },
		{ -0.5f,  0.5f, 0.0f, 1.0f }
	};

    Mat4Identity(model);
    Mat4Translation(pos, model);
    Mat4Multiply(rg.viewData.camera.transformMatrix, model, mvp);

	Mat4Transform(mvp, positions[0], p);
	VectorCopy(verts[0], p);
	Mat4Transform(mvp, positions[1], p);
	VectorCopy(verts[1], p);
	Mat4Transform(mvp, positions[2], p);
	VectorCopy(verts[2], p);
	Mat4Transform(mvp, positions[3], p);
	VectorCopy(verts[3], p);
}

void RB_AddQuad(const drawVert_t verts[4])
{
	uint32_t idx;

	idx = drawBuf.numVertices;

	// triangle indices for a simple quad
	drawBuf.indices[drawBuf.numIndices] = idx;
	drawBuf.indices[drawBuf.numIndices + 1] = idx + 1;
	drawBuf.indices[drawBuf.numIndices + 2] = idx + 2;

	drawBuf.indices[drawBuf.numIndices + 3] = idx + 3;
	drawBuf.indices[drawBuf.numIndices + 4] = idx + 2;
	drawBuf.indices[drawBuf.numIndices + 5] = idx + 0;

	VectorCopy(drawBuf.xyz[idx], verts[0].xyz);
	VectorCopy(drawBuf.xyz[idx+1], verts[1].xyz);
	VectorCopy(drawBuf.xyz[idx+2], verts[2].xyz);
	VectorCopy(drawBuf.xyz[idx+3], verts[3].xyz);

	VectorCopy2(drawBuf.texCoords[idx], verts[0].uv);
	VectorCopy2(drawBuf.texCoords[idx+1], verts[1].uv);
	VectorCopy2(drawBuf.texCoords[idx+2], verts[2].uv);
	VectorCopy2(drawBuf.texCoords[idx+3], verts[3].uv);

	drawBuf.numVertices += 4;
	drawBuf.numIndices += 6;
}

/*
==============
RB_InstantQuad

based on Tess_InstantQuad from xreal
==============
*/
void RB_InstantQuad2(vec4_t quadVerts[4], vec2_t texCoords[4])
{
//	GLimp_LogComment("--- RB_InstantQuad2 ---\n");

	drawBuf.numVertices = 0;
	drawBuf.numIndices = 0;
	drawBuf.firstIndex = 0;

	VectorCopy(drawBuf.xyz[drawBuf.numVertices], quadVerts[0]);
	VectorCopy2(drawBuf.texCoords[drawBuf.numVertices], texCoords[0]);
	drawBuf.numVertices++;

	VectorCopy(drawBuf.xyz[drawBuf.numVertices], quadVerts[1]);
	VectorCopy2(drawBuf.texCoords[drawBuf.numVertices], texCoords[1]);
	drawBuf.numVertices++;

	VectorCopy(drawBuf.xyz[drawBuf.numVertices], quadVerts[2]);
	VectorCopy2(drawBuf.texCoords[drawBuf.numVertices], texCoords[2]);
	drawBuf.numVertices++;

	VectorCopy(drawBuf.xyz[drawBuf.numVertices], quadVerts[3]);
	VectorCopy2(drawBuf.texCoords[drawBuf.numVertices], texCoords[3]);
	drawBuf.numVertices++;

	drawBuf.indices[drawBuf.numIndices++] = 0;
	drawBuf.indices[drawBuf.numIndices++] = 1;
	drawBuf.indices[drawBuf.numIndices++] = 2;
	drawBuf.indices[drawBuf.numIndices++] = 0;
	drawBuf.indices[drawBuf.numIndices++] = 2;
	drawBuf.indices[drawBuf.numIndices++] = 3;

	RB_UpdateCache(ATTRIB_POSITION | ATTRIB_TEXCOORD);

	R_DrawElements(drawBuf.numIndices, drawBuf.firstIndex);

	drawBuf.numIndices = 0;
	drawBuf.numVertices = 0;
	drawBuf.firstIndex = 0;
}

void RB_InstantQuad(vec4_t quadVerts[4])
{
	vec2_t texCoords[4];

	VectorSet2(texCoords[0], 0.0f, 0.0f);
	VectorSet2(texCoords[1], 1.0f, 0.0f);
	VectorSet2(texCoords[2], 1.0f, 1.0f);
	VectorSet2(texCoords[3], 0.0f, 1.0f);

	GLSL_UseProgram(&rg.basicShader);
	
	GLSL_SetUniformMatrix4(&rg.basicShader, UNIFORM_MODELVIEWPROJECTION, glState.modelviewProjection);
	GLSL_SetUniformVec4(&rg.basicShader, UNIFORM_COLOR, colorWhite);

	RB_InstantQuad2(quadVerts, texCoords);
}

#if 0
qboolean RB_SurfaceVaoCached(uint32_t numVerts, drawVert_t *verts, uint32_t numIndices, glIndex_t *indices)
{
	qboolean recycleVertexBuffer = qfalse;
	qboolean recycleIndexBuffer = qfalse;
	qboolean endSurface = qfalse;

	if (!numIndices || !numVerts)
		return qfalse;
	
	VaoCache_BindVao();

	VaoCache_CheckAdd(&endSurface, &recycleVertexBuffer, &recycleIndexBuffer, numVerts, numIndices);

	if (endSurface) {
		RB_EndSurface();
		RB_BeginSurface(drawBuf.shader);
	}

	if (recycleVertexBuffer)
		VaoCache_RecycleVertexBuffer();
	
	if (recycleIndexBuffer)
		VaoCache_RecycleIndexBuffer();
	
	if (!drawBuf.numVertices)
		VaoCache_InitQueue();
	
	VaoCache_AddSurface(verts, numVerts, indices, numIndices);

	drawBuf.numIndices += numIndices;
	drawBuf.numVertices += numVerts;
	drawBuf.useCacheVao = qtrue;
	drawBuf.useInternalVao = qfalse;

	return qtrue;
}
#endif

static void RB_SurfacePolychain(const srfPoly_t *p)
{
	uint64_t i, numv, numIdx, idxCount, vtxCount;

	RB_CheckVao(drawBuf.buf);
	RB_CheckOverflow(p->numVerts, 3*(p->numVerts - 2));
	
	// fan triangles into the draw buffer
	numv = drawBuf.numVertices;
	vtxCount = 0;
	for (i = 0; i < p->numVerts; i++) {
		VectorCopy(drawBuf.xyz[numv], p->verts[i].xyz);

		drawBuf.texCoords[numv][0] = p->verts[i].uv[0];
		drawBuf.texCoords[numv][1] = p->verts[i].uv[1];
		drawBuf.color[numv][0] = (int)p->verts[i].modulate.rgba[0] * 257;
		drawBuf.color[numv][1] = (int)p->verts[i].modulate.rgba[1] * 257;
		drawBuf.color[numv][2] = (int)p->verts[i].modulate.rgba[2] * 257;
		drawBuf.color[numv][3] = (int)p->verts[i].modulate.rgba[3] * 257;

		numv++;
		vtxCount++;
	}

	// generate fan indices into the draw buffer
	numIdx = drawBuf.numIndices;
	for (i = 0; i < p->numVerts; i++) {
		drawBuf.indices[numIdx + 0] = drawBuf.numVertices;
		drawBuf.indices[numIdx + 1] = drawBuf.numVertices + i + 1;
		drawBuf.indices[numIdx + 2] = drawBuf.numVertices + i + 2;
		numIdx += 3;
		idxCount++;
	}

	drawBuf.numIndices = numIdx;
	drawBuf.numVertices = numv;
}

static void RB_SurfaceTile(const srfTile_t *tile)
{
//	RB_SurfaceVaoCached(4, tile->verts, 6, tile->indices);
}

static void RB_SurfaceBad(void *) {
	ri.Printf(PRINT_INFO, COLOR_YELLOW "Bad surface processed\n");
}

static void RB_SurfaceSkip(void *) {
}

void (*rb_surfaceTable[SF_NUM_SURFACE_TYPES])(void *) = {
	(void(*)(void*))RB_SurfaceBad,
	(void(*)(void*))RB_SurfaceSkip,
	(void(*)(void*))RB_SurfacePolychain,
	(void(*)(void*))RB_SurfaceTile
};


