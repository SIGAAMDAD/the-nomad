#include "rgl_local.h"

uint64_t r_firstSceneDrawSurf;
uint64_t r_numEntities;
uint64_t r_firstSceneEntity;
uint64_t r_numPolys;
uint64_t r_firstScenePoly;
uint64_t r_numPolyVerts;

/*
====================
R_InitNextFrame

====================
*/
void R_InitNextFrame( void ) {
	backendData->commandList.usedBytes = 0;

	r_firstSceneDrawSurf = 0;

	r_numEntities = 0;
	r_firstSceneEntity = 0;

	r_numPolys = 0;
	r_firstScenePoly = 0;

	r_numPolyVerts = 0;
}


/*
====================
RE_ClearScene

====================
*/
void RE_ClearScene( void ) {
	r_firstSceneEntity = r_numEntities;
	r_firstScenePoly = r_numPolys;
}

void R_DrawElements(uint32_t numIndices, uintptr_t offset)
{
	nglDrawElements(GL_TRIANGLES, numIndices, GLN_INDEX_TYPE, (const void *)offset);
}

void RB_CheckOverflow( uint32_t verts, uint32_t indexes )
{
	if (backend->dbuf.numVertices + verts < MAX_BATCH_VERTICES
		&& backend->dbuf.numIndices + indexes < MAX_BATCH_INDICES) {
		return;
	}

	RB_EndSurface();

	if ( verts >= MAX_BATCH_VERTICES ) {
		ri.Error(ERR_DROP, "RB_CheckOverflow: verts > MAX (%d > %d)", verts, MAX_BATCH_VERTICES );
	}
	if ( indexes >= MAX_BATCH_INDICES ) {
		ri.Error(ERR_DROP, "RB_CheckOverflow: indices > MAX (%d > %d)", indexes, MAX_BATCH_INDICES );
	}

	RB_BeginSurface(backend->dbuf.shader);
}

static void RB_CheckVao(vertexBuffer_t *buf)
{
	if (buf != glState.currentVao) {
		RB_EndSurface();
		RB_BeginSurface(backend->dbuf.shader);

		VBO_Bind(buf);
	}
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

void GDR_EXPORT RE_AddPolyToScene( nhandle_t hShader, const polyVert_t *verts, uint32_t numVerts )
{
	srfPoly_t *poly;

	if (!rg.registered) {
		return;
	}
	if (hShader < -1) {
		hShader = 0; // default shader
	}

	if (!verts) { // should never happen
		ri.Error(ERR_FATAL, "RE_AddPolyToScene: invalid vertices");
	}
	if (r_numPolys >= r_maxPolys->i || r_numPolyVerts + numVerts >= r_maxPolyVerts->i) {
		ri.Printf(PRINT_DEVELOPER, "WARNING: RE_AddPolyToScene: r_max_polys or r_max_polyverts reached\n");
		return;
	}

	poly = &backendData->polys[r_numPolys];
	poly->surfaceType = SF_POLY;
	poly->hShader = hShader;
	poly->numVerts = numVerts;
	poly->verts = &backendData->polyVerts[r_numPolyVerts];
	memcpy(poly->verts, verts, numVerts * sizeof(*verts));

	// done.
	r_numPolyVerts += numVerts;
	r_numPolys++;
}

void GDR_EXPORT RE_AddPolyListToScene( const poly_t *polys, uint32_t numPolys )
{
	const poly_t *p;
	uint32_t i;

	if (!rg.registered) {
		return;
	}

	if (!polys) { // should never happen
		ri.Error(ERR_FATAL, "RE_AddPolyListToScene: bad polylist!");
	}
	if (!numPolys) {
		ri.Printf(PRINT_DEVELOPER, COLOR_YELLOW "RE_AddPolyListToScene: no polys\n");
		return;
	}

	for (i = 0, p = polys; i < numPolys; i++, p++) {
		RE_AddPolyToScene(p->hShader, p->verts, p->numVerts);
	}
}

static void R_ConvertCoords(drawVert_t verts[4], vec3_t pos)
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
    Mat4Multiply(glState.modelViewProjectionMatrix, model, mvp);

	Mat4Transform(mvp, positions[0], p);
	VectorCopy(verts[0].xyz, p);
	Mat4Transform(mvp, positions[1], p);
	VectorCopy(verts[1].xyz, p);
	Mat4Transform(mvp, positions[2], p);
	VectorCopy(verts[2].xyz, p);
	Mat4Transform(mvp, positions[3], p);
	VectorCopy(verts[3].xyz, p);
}

static void R_AddWorldSurfs(void)
{
	drawVert_t verts[4];
	const maptile_t *tiles;
	vec3_t pos;
	uint32_t i;

	if (!r_drawWorld->i) {
		return;
	}

	VectorClear(pos);
	GL_BindTexture(GL_TEXTURE3, rg.world->tileset);
	tiles = rg.world->tiles;

	for (i = 0; i < rg.world->numSurfs; i++) {
		VectorCopy(pos, tiles[i].pos);
		R_ConvertCoords(verts, pos);
		R_AddDrawSurf((void *)rg.world->tileSurfs, rg.world->shader);
	}
}

void R_GenerateDrawSurfs(void)
{
	R_AddWorldSurfs();

	R_AddPolySurfs();

	R_SortDrawSurfs(backendData->drawSurfs, backendData->numDrawSurfs);
}

void R_FinishBatch(const drawBuffer_t *input)
{
	// setup state
	GL_BindTexture(GL_TEXTURE0, input->shader->texture);
	GLSL_UseProgram(&rg.basicShader);
	GLSL_SetUniformMatrix4(&rg.basicShader, UNIFORM_MODELVIEWPROJECTION, glState.modelViewProjectionMatrix);
	GLSL_SetUniformInt(&rg.basicShader, UNIFORM_DIFFUSE_MAP, input->shader->texture->id);
	GL_State(input->shader->stateBits);

	//
	// draw
	//
	R_DrawElements(input->numIndices, input->firstIndex);
}

void RB_AddQuad(const drawVert_t verts[4])
{
	uint32_t idx;

	idx = backend->dbuf.numVertices;

	// triangle indices for a simple quad
	backend->dbuf.indices[backend->dbuf.numIndices] = idx;
	backend->dbuf.indices[backend->dbuf.numIndices + 1] = idx + 1;
	backend->dbuf.indices[backend->dbuf.numIndices + 2] = idx + 2;

	backend->dbuf.indices[backend->dbuf.numIndices + 3] = idx + 3;
	backend->dbuf.indices[backend->dbuf.numIndices + 4] = idx + 2;
	backend->dbuf.indices[backend->dbuf.numIndices + 5] = idx + 0;

	VectorCopy(backend->dbuf.xyz[idx], verts[0].xyz);
	VectorCopy(backend->dbuf.xyz[idx+1], verts[1].xyz);
	VectorCopy(backend->dbuf.xyz[idx+2], verts[2].xyz);
	VectorCopy(backend->dbuf.xyz[idx+3], verts[3].xyz);

	VectorCopy2(backend->dbuf.texCoords[idx], verts[0].uv);
	VectorCopy2(backend->dbuf.texCoords[idx+1], verts[1].uv);
	VectorCopy2(backend->dbuf.texCoords[idx+2], verts[2].uv);
	VectorCopy2(backend->dbuf.texCoords[idx+3], verts[3].uv);

	backend->dbuf.numVertices += 4;
	backend->dbuf.numIndices += 6;
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

	backend->dbuf.numVertices = 0;
	backend->dbuf.numIndices = 0;
	backend->dbuf.firstIndex = 0;

	VectorCopy(backend->dbuf.xyz[backend->dbuf.numVertices], quadVerts[0]);
	VectorCopy2(backend->dbuf.texCoords[backend->dbuf.numVertices], texCoords[0]);
	backend->dbuf.numVertices++;

	VectorCopy(backend->dbuf.xyz[backend->dbuf.numVertices], quadVerts[1]);
	VectorCopy2(backend->dbuf.texCoords[backend->dbuf.numVertices], texCoords[1]);
	backend->dbuf.numVertices++;

	VectorCopy(backend->dbuf.xyz[backend->dbuf.numVertices], quadVerts[2]);
	VectorCopy2(backend->dbuf.texCoords[backend->dbuf.numVertices], texCoords[2]);
	backend->dbuf.numVertices++;

	VectorCopy(backend->dbuf.xyz[backend->dbuf.numVertices], quadVerts[3]);
	VectorCopy2(backend->dbuf.texCoords[backend->dbuf.numVertices], texCoords[3]);
	backend->dbuf.numVertices++;

	backend->dbuf.indices[backend->dbuf.numIndices++] = 0;
	backend->dbuf.indices[backend->dbuf.numIndices++] = 1;
	backend->dbuf.indices[backend->dbuf.numIndices++] = 2;
	backend->dbuf.indices[backend->dbuf.numIndices++] = 0;
	backend->dbuf.indices[backend->dbuf.numIndices++] = 2;
	backend->dbuf.indices[backend->dbuf.numIndices++] = 3;

	RB_UpdateCache(ATTRIB_POSITION | ATTRIB_TEXCOORD);

	R_DrawElements(backend->dbuf.numIndices, backend->dbuf.firstIndex);

	backend->dbuf.numIndices = 0;
	backend->dbuf.numVertices = 0;
	backend->dbuf.firstIndex = 0;
}

void RB_InstantQuad(vec4_t quadVerts[4])
{
	vec2_t texCoords[4];

	VectorSet2(texCoords[0], 0.0f, 0.0f);
	VectorSet2(texCoords[1], 1.0f, 0.0f);
	VectorSet2(texCoords[2], 1.0f, 1.0f);
	VectorSet2(texCoords[3], 0.0f, 1.0f);

	GLSL_UseProgram(&rg.basicShader);
	
	GLSL_SetUniformMatrix4(&rg.basicShader, UNIFORM_MODELVIEWPROJECTION, glState.modelViewProjectionMatrix);
	GLSL_SetUniformInt(&rg.basicShader, UNIFORM_DIFFUSE_MAP, backend->dbuf.shader->texture->id);
	GLSL_SetUniformVec4(&rg.basicShader, UNIFORM_COLOR, colorWhite);

	RB_InstantQuad2(quadVerts, texCoords);
}

static qboolean RB_SurfaceVaoCached(uint32_t numVerts, drawVert_t *verts, uint32_t numIndices, glIndex_t *indices)
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
		RB_BeginSurface(backend->dbuf.shader);
	}

	if (recycleVertexBuffer)
		VaoCache_RecycleVertexBuffer();
	
	if (recycleIndexBuffer)
		VaoCache_RecycleIndexBuffer();
	
	if (!backend->dbuf.numVertices)
		VaoCache_InitQueue();
	
	VaoCache_AddSurface(verts, numVerts, indices, numIndices);

	backend->dbuf.numIndices += numIndices;
	backend->dbuf.numVertices += numVerts;

	return qtrue;
}

static void RB_SurfacePolychain(const srfPoly_t *p)
{
	uint32_t i, numv;

	RB_CheckVao(backend->dbuf.buf);
	RB_CheckOverflow(p->numVerts, 3*(p->numVerts - 2));
	
	// fan triangles into the draw buffer
	numv = backend->dbuf.numVertices;
	for (i = 0; i < p->numVerts; i++) {
		VectorCopy(backend->dbuf.xyz[numv], p->verts[i].xyz);

		backend->dbuf.texCoords[numv][0] = p->verts[i].uv[0];
		backend->dbuf.texCoords[numv][1] = p->verts[i].uv[1];
//		tess.color[numv][0] = (int)p->verts[i].modulate.rgba[0] * 257;
//		tess.color[numv][1] = (int)p->verts[i].modulate.rgba[1] * 257;
//		tess.color[numv][2] = (int)p->verts[i].modulate.rgba[2] * 257;
//		tess.color[numv][3] = (int)p->verts[i].modulate.rgba[3] * 257;

		numv++;
	}

	// generate fan indices into the draw buffer
	for (i = 0; i < p->numVerts; i++) {
		backend->dbuf.indices[backend->dbuf.numIndices + 0] = backend->dbuf.numVertices;
		backend->dbuf.indices[backend->dbuf.numIndices + 1] = backend->dbuf.numVertices + i + 1;
		backend->dbuf.indices[backend->dbuf.numIndices + 2] = backend->dbuf.numVertices + i + 2;
		backend->dbuf.numIndices += 3;
	}

	backend->dbuf.numVertices = numv;
}

static void RB_SurfaceTile(const srfTile_t *tile)
{
	RB_SurfaceVaoCached(4, tile->verts, 6, tile->indices);
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
