// rgl_scene.c -- there can be multiple scenes per frame, all rendering data submissions must come through here or rgl_cmd.c

#include "rgl_local.h"

uint64_t r_numEntities;
uint64_t r_firstSceneEntity;

uint64_t r_numDLights;
uint64_t r_firstSceneDLight;

uint64_t r_numPolys;
uint64_t r_firstScenePoly;

uint64_t r_numPolyVerts;
uint64_t r_firstSceneVert;

uint64_t r_numQuads;

void R_InitNextFrame( void )
{
	if ( !sys_forceSingleThreading->i ) {
		// use the other buffers next frame, because another CPU
		// may still be rendering into the current ones
		rg.smpFrame ^= 1;
	} else {
		rg.smpFrame = 0;
	}
	backendData[ rg.smpFrame ]->commandList.usedBytes = 0;

	r_firstSceneDLight = 0;
	r_numDLights = 0;

	r_firstSceneEntity = 0;
	r_numEntities = 0;

	r_firstScenePoly = 0;
	r_numPolys = 0;

	r_firstSceneVert = 0;
	r_numPolyVerts = 0;
}

void RE_AddSpriteToScene( const vec3_t origin, nhandle_t hShader )
{
	srfPoly_t *poly;
	polyVert_t *vtx;
	srfVert_t *vt;
	vec3_t pos;
	uint32_t i;

	if ( !rg.registered ) {
		return;
	}
	if ( !rg.world ) {
		ri.Error( ERR_DROP, "RE_AddSpriteToScene: no world loaded\n" );
	}

	if ( r_numPolyVerts + 4 >= r_maxPolys->i * 4 ) {
		ri.Printf( PRINT_DEVELOPER, "RE_AddSpriteToScene: r_maxPolyVerts hit, dropping %i vertices\n", 4 );
		return;
	}

	poly = &backendData[ rg.smpFrame ]->polys[ r_numPolys ];
	vtx = &backendData[ rg.smpFrame ]->polyVerts[ r_numPolyVerts ];

	pos[0] = origin[0];
	pos[1] = rg.world->height - origin[1];
	pos[2] = origin[2];

	poly->verts = vtx;
	poly->numVerts = 4;
	poly->hShader = hShader;

	VectorSet2( vtx[0].uv, 0, 0 );
	VectorSet2( vtx[1].uv, 1, 0 );
	VectorSet2( vtx[2].uv, 1, 1 );
	VectorSet2( vtx[3].uv, 0, 1 );

	VectorCopy( vtx[0].worldPos, pos );
	VectorCopy( vtx[1].worldPos, pos );
	VectorCopy( vtx[2].worldPos, pos );
	VectorCopy( vtx[3].worldPos, pos );

	r_numPolyVerts += 4;
	r_numPolys++;
}

void RE_AddPolyToScene( nhandle_t hShader, const polyVert_t *verts, uint32_t numVerts )
{
	uint32_t i, offset;
	polyVert_t *vt;
	srfPoly_t *poly;

	if ( !rg.registered ) {
		return;
	}

	if ( r_numPolyVerts + numVerts >= r_maxPolys->i * 4 ) {
		ri.Printf( PRINT_DEVELOPER, "RE_AddPolyToScene: r_maxPolyVerts hit, dropping %i vertices\n", numVerts );
		return;
	}

	if ( hShader >= rg.numShaders || hShader < 0 ) {
		ri.Printf( PRINT_WARNING, "RE_AddPolyToScene: out of range hShader '%i'\n", hShader );
		return;
	}

	poly = &backendData[ rg.smpFrame ]->polys[r_numPolys];
	vt = &backendData[ rg.smpFrame ]->polyVerts[r_numPolyVerts];

	poly->verts = vt;
	poly->hShader = hShader;
	poly->numVerts = numVerts;

	memcpy( vt, verts, sizeof( *vt ) * numVerts );

	r_numPolyVerts += numVerts;
	r_numPolys++;
}

void RE_AddDynamicLightToScene( const vec3_t origin, float range, float constant, float linear, float quadratic,
	float brightness, const vec3_t color )
{
	dlight_t *dl;

	if ( !rg.registered ) {
		return;
	}
	if ( r_numDLights >= r_maxDLights->i || !r_dynamiclight->i ) {
		return;
	}
	if ( brightness <= 0.0f ) {
		return;
	}

	dl = &backendData[ rg.smpFrame ]->dlights[ r_numDLights ];
	VectorCopy( dl->origin, origin );
	VectorCopy( dl->color, color );
	dl->brightness = brightness;
	dl->range = range;
	dl->constant = constant;
	dl->linear = linear;
	dl->quadratic = quadratic;

	r_numDLights++;
}

void RE_AddPolyListToScene( const poly_t *polys, uint32_t numPolys )
{
	uint32_t i;

	if ( !rg.registered ) {
		return;
	}

	if ( r_numPolys + numPolys >= r_maxPolys->i ) {
		ri.Printf( PRINT_DEVELOPER, "RE_AddPolyListToScene: r_maxPolys hit, dropping %i polygons\n", numPolys );
		return;
	}

	for ( i = 0; i < numPolys; i++ ) {
		RE_AddPolyToScene( polys[i].hShader, polys[i].verts, polys[i].numVerts );
	}
}

void RE_AddEntityToScene( const renderEntityRef_t *ent )
{
	if ( !rg.registered ) {
		return;
	}

	if ( r_numEntities >= MAX_RENDER_ENTITIES ) {
		ri.Printf( PRINT_DEVELOPER, "RE_AddEntityToScene: MAX_RENDER_ENTITIES hit, dropping entity\n" );
		return;
	}
	if ( !rg.world ) {
		ri.Error( ERR_DROP, "RE_AddEntityToScene: only use when a world is loaded" );
	}
	if ( ent->sheetNum != -1 && ent->spriteId >= rg.sheets[ ent->sheetNum ]->numSprites ) {
		ri.Printf( PRINT_DEVELOPER, "RE_AddEntityToScene: invalid spriteId\n" );
		return;
	}
	if ( N_isnan( ent->origin[0] ) || N_isnan( ent->origin[1] ) || N_isnan( ent->origin[2] ) ) {
		static qboolean firstTime = qtrue;
		if ( firstTime ) {
			firstTime = qfalse;
			ri.Printf( PRINT_INFO, COLOR_YELLOW "RE_AddEntityToScene passed a refEntity which has an origin with a NaN component\n");
		}
		return;
	}

	backendData[ rg.smpFrame ]->entities[ r_numEntities ].e = *ent;

	r_numEntities++;
}

void RE_ProcessDLights( void )
{
	dlight_t *dlight;
	shaderLight_t *gpuLight;
	uint64_t i;
	uint32_t numLights;

	numLights = rg.world->numLights;
	if ( r_dynamiclight->i ) {
		numLights += backend.refdef.numDLights;
	}

	if ( r_dynamiclight->i && backend.refdef.numDLights && ( backend.refdef.flags & RSF_ORTHO_BITS ) == RSF_ORTHO_TYPE_WORLD ) {
		gpuLight = (shaderLight_t *)rg.lightData->data + rg.world->numLights;
		dlight = backend.refdef.dlights;

		for ( i = 0; i < backend.refdef.numDLights; i++ ) {
			if ( r_numDLights >= r_maxDLights->i ) {
				ri.Printf( PRINT_DEVELOPER, "R_ProcessDLights: too many lights, dropping %lu lights\n", backend.refdef.numDLights - i );
			}

			VectorSet2( gpuLight[i].origin, dlight->origin[0], dlight->origin[1] );
			VectorCopy( gpuLight[i].color, dlight->color );

			gpuLight[i].color[3] = 1.0f;

			gpuLight[i].brightness = dlight->brightness;
			gpuLight[i].range = dlight->range;
			gpuLight[i].type = LIGHT_POINT;
			gpuLight[i].constant = dlight->constant;
			gpuLight[i].quadratic = dlight->quadratic;
			gpuLight[i].linear = dlight->linear;

			dlight++;
		}
	}
	if ( r_dynamiclight->i ) {
		GLSL_UseProgram( &rg.tileShader );
		GLSL_SetUniformInt( &rg.tileShader, UNIFORM_NUM_LIGHTS, numLights );
		GLSL_ShaderBufferData( &rg.tileShader, UNIFORM_LIGHTDATA, rg.lightData, sizeof( *gpuLight ) * numLights, 0, qfalse );

		GLSL_UseProgram( &rg.spriteShader );
		GLSL_SetUniformInt( &rg.spriteShader, UNIFORM_NUM_LIGHTS, numLights );
	}
}

void RE_ProcessEntities( void )
{
	renderEntityDef_t *refEntity;
	vec3_t xyz[4];
	vec3_t origin;
	polyVert_t *verts;
	srfPoly_t *poly;
	uint64_t i, j;
	uint32_t stage, bundle;
	uint64_t maxVerts;
	float texIndex;
	qboolean done;
	const shader_t *shader;

	if ( !r_numEntities || !backend.refdef.numEntities || ( backend.refdef.flags & RSF_ORTHO_BITS ) != RSF_ORTHO_TYPE_WORLD ) {
		return;
	}

	refEntity = backend.refdef.entities;
	maxVerts = r_maxPolys->i * 4;
	poly = &backend.refdef.polys[ backend.refdef.numPolys ];
	verts = &backendData[ rg.smpFrame ]->polyVerts[ r_numPolyVerts ];
	texIndex = 0.0f;

	static const vec2_t texCoords[4] = {
		{ 1.0f, 0.0f },
		{ 1.0f, 1.0f },
		{ 0.0f, 1.0f },
		{ 0.0f, 0.0f }
	};

	for ( i = 0; i < backend.refdef.numEntities; i++ ) {
		if ( r_numPolys >= r_maxPolys->i || r_numPolyVerts >= maxVerts ) {
			ri.Printf( PRINT_DEVELOPER, "R_ProcessEntities: too many entities, dropping %lu entities\n", backend.refdef.numEntities - i );
			break;
		}
		
		origin[0] = refEntity->e.origin[0];
		origin[1] = rg.world->height - ( refEntity->e.origin[1] - refEntity->e.origin[2] );
		origin[2] = 0.0f;

		poly->verts = verts;
		if ( refEntity->e.sheetNum == -1 || !rg.sheets[ refEntity->e.sheetNum ] ) {
			poly->hShader = refEntity->e.spriteId;
		} else {
			poly->hShader = rg.sheets[ refEntity->e.sheetNum ]->hShader;
		}
		poly->numVerts = 4;

//		R_LightEntity( refEntity );

		ri.GLM_TransformToGL( origin, xyz, refEntity->e.scale, refEntity->e.rotation, glState.viewData.camera.viewProjectionMatrix );

		if ( refEntity->e.sheetNum == -1 ) {
			*(double *)verts[0].uv = *(double *)texCoords[0];
			*(double *)verts[1].uv = *(double *)texCoords[1];
			*(double *)verts[2].uv = *(double *)texCoords[2];
			*(double *)verts[3].uv = *(double *)texCoords[3];

			shader = R_GetShaderByHandle( refEntity->e.spriteId );
		} else {
			VectorCopy2( verts[ 0 ].uv, rg.sheets[ refEntity->e.sheetNum ]->sprites[ refEntity->e.spriteId ].texCoords[0] );
			VectorCopy2( verts[ 1 ].uv, rg.sheets[ refEntity->e.sheetNum ]->sprites[ refEntity->e.spriteId ].texCoords[1] );
			VectorCopy2( verts[ 2 ].uv, rg.sheets[ refEntity->e.sheetNum ]->sprites[ refEntity->e.spriteId ].texCoords[2] );
			VectorCopy2( verts[ 3 ].uv, rg.sheets[ refEntity->e.sheetNum ]->sprites[ refEntity->e.spriteId ].texCoords[3] );

			shader = R_GetShaderByHandle( rg.sheets[ refEntity->e.sheetNum ]->hShader );
		}

		VectorSet2( verts[0].worldPos, refEntity->e.origin[0], refEntity->e.origin[1] + refEntity->e.origin[2] );
		VectorCopy2( verts[0].xyz, xyz[0] );
		verts[0].modulate.u32 = refEntity->e.shader.u32;
		verts[0].modulate.u32 += refEntity->ambientLightInt;

		VectorSet2( verts[1].worldPos, refEntity->e.origin[0], refEntity->e.origin[1] + refEntity->e.origin[2] );
		VectorCopy2( verts[1].xyz, xyz[1] );
		verts[1].modulate.u32 = refEntity->e.shader.u32;
		verts[1].modulate.u32 += refEntity->ambientLightInt;

		VectorSet2( verts[2].worldPos, refEntity->e.origin[0], refEntity->e.origin[1] + refEntity->e.origin[2] );
		VectorCopy2( verts[2].xyz, xyz[2] );
		verts[2].modulate.u32 = refEntity->e.shader.u32;
		verts[2].modulate.u32 += refEntity->ambientLightInt;

		VectorSet2( verts[3].worldPos, refEntity->e.origin[0], refEntity->e.origin[1] + refEntity->e.origin[2] );
		VectorCopy2( verts[3].xyz, xyz[3] );
		verts[3].modulate.u32 = refEntity->e.shader.u32;
		verts[3].modulate.u32 += refEntity->ambientLightInt;

		verts += 4;
		r_numPolyVerts += 4;

		refEntity++;
		poly++;
		backend.refdef.numPolys++;
	}
}

void RE_BeginScene( const renderSceneRef_t *fd )
{
	backend.refdef.x = fd->x;
	backend.refdef.y = fd->y;
	backend.refdef.width = fd->width;
	backend.refdef.height = fd->height;
	backend.refdef.flags = fd->flags;

	backend.refdef.time = fd->time;
	backend.refdef.floatTime = backend.refdef.time * 0.001f; // -EC-: cast to double

	backend.refdef.numDLights = r_numDLights - r_firstSceneDLight;
	backend.refdef.dlights = &backendData[ rg.smpFrame ]->dlights[ r_firstSceneDLight ];

	backend.refdef.numEntities = r_numEntities - r_firstSceneEntity;
	backend.refdef.entities = &backendData[ rg.smpFrame ]->entities[ r_firstSceneEntity ];

	backend.refdef.numPolys = r_numPolys - r_firstScenePoly;
	backend.refdef.polys = &backendData[ rg.smpFrame ]->polys[ r_firstScenePoly ];

	backend.refdef.drawn = qfalse;

	rg.frameSceneNum++;
	rg.frameCount++;
}

void RE_ClearScene( void )
{
	r_firstSceneDLight = r_numDLights;
	r_firstSceneEntity = r_numEntities;
	r_firstScenePoly = r_numPolys;
	r_firstSceneVert = r_numPolyVerts;
}

void RE_EndScene( void )
{
	r_firstSceneDLight = r_numDLights;
	r_firstSceneEntity = r_numEntities;
	r_firstScenePoly = r_numPolys;
	r_firstSceneVert = r_numPolyVerts;
}

void RE_RenderScene( const renderSceneRef_t *fd )
{
	viewData_t parms;
	int64_t startTime;

	if ( !rg.registered ) {
		return;
	}

	startTime = ri.Milliseconds();

	if ( !rg.world && !( fd->flags & RSF_NOWORLDMODEL ) ) {
		ri.Error( ERR_FATAL, "RE_RenderScene: no world loaded" );
	}

	RE_BeginScene( fd );

	memset( &parms, 0, sizeof( parms ) );
	parms.viewportX = backend.refdef.x;
	parms.viewportY = backend.refdef.y;

	parms.flags = fd->flags;

	parms.viewportWidth = backend.refdef.width;
	parms.viewportHeight = backend.refdef.height;
	parms.camera = glState.viewData.camera;
	parms.stereoFrame = backend.refdef.stereoFrame;

	R_RenderView( &parms );

	if ( !( parms.flags & RSF_NOWORLDMODEL ) ) {
		R_AddPostProcessCmd();
	}

	RE_EndScene();

	rg.frontEndMsec += ri.Milliseconds() - startTime;
}


