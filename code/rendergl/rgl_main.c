#include "rgl_local.h"

// this is here to that functions in n_shared.c can link

void GDR_DECL N_Error( errorCode_t code, const char *err, ... )
{
	va_list argptr;
	char msg[MAXPRINTMSG];

	va_start( argptr, err );
	N_vsnprintf( msg, sizeof( msg ) - 1, err, argptr );
	va_end( argptr );

	ri.Error( code, "%s", msg );
}

void GDR_DECL Con_Printf( const char *fmt, ... )
{
	va_list argptr;
	char msg[MAXPRINTMSG];

	va_start( argptr, fmt );
	N_vsnprintf( msg, sizeof( msg ) - 1, fmt, argptr );
	va_end( argptr );

	ri.Printf( PRINT_INFO, "%s", msg );
}

void RB_MakeViewMatrix( void )
{
	float aspect;
	uint32_t viewFlags;
	vec4_t ortho;

	aspect = glConfig.vidWidth / glConfig.vidHeight;

	glState.viewData.zFar = -1.0f;
	glState.viewData.zNear = 1.0f;
	viewFlags = glState.viewData.flags & RSF_ORTHO_BITS;

	switch ( viewFlags ) {
	case RSF_ORTHO_TYPE_SCREENSPACE:
		ortho[0] = 0;
		ortho[1] = glConfig.vidWidth;
		ortho[2] = glConfig.vidHeight;
		ortho[3] = 0;

		VectorClear( glState.viewData.camera.origin );
		break;
	case RSF_ORTHO_TYPE_WORLD:
		ortho[0] = 0.0f;
		ortho[1] = rg.world->width;
		ortho[2] = 0.0f;
		ortho[3] = rg.world->height;
		break;
	case RSF_ORTHO_TYPE_CORDESIAN:
		ortho[0] = -aspect;
		ortho[1] = aspect;
		ortho[2] = -aspect;
		ortho[3] = aspect;
		break;
	default:
		ri.Error( ERR_DROP, "R_RenderView: invalid orthographic matrix type" );
	};

	ri.GLM_MakeVPM( ortho, &glState.viewData.camera.zoom, glState.viewData.zNear, glState.viewData.zFar, glState.viewData.camera.origin,
		glState.viewData.camera.viewProjectionMatrix, glState.viewData.camera.projectionMatrix, glState.viewData.camera.viewMatrix,
		viewFlags );
}

/*
===============
R_Radix
===============
*/
static GDR_INLINE void R_Radix( int32_t byte, uint32_t size, const srfPoly_t *source, srfPoly_t *dest )
{
	uint32_t       count[ 256 ] = { 0 };
	uint32_t       index[ 256 ];
	uint32_t       i;
	uint8_t        *sortKey;
	uint8_t        *end;

	sortKey = ( (uint8_t *)&source[ 0 ].hShader ) + byte;
	end = sortKey + ( size * sizeof( srfPoly_t ) );
	for ( ; sortKey < end; sortKey += sizeof( srfPoly_t ) ) {
		++count[ *sortKey ];
	}

	index[ 0 ] = 0;

	for ( i = 1; i < 256; ++i ) {
		index[ i ] = index[ i - 1 ] + count[ i - 1 ];
	}

	sortKey = ( (uint8_t *)&source[ 0 ].hShader ) + byte;
	for ( i = 0; i < size; ++i, sortKey += sizeof( srfPoly_t ) ) {
		dest[ index[ *sortKey ]++ ] = source[ i ];
	}
}


/*
===============
R_RadixSort

Radix sort with 4 byte size buckets
===============
*/
static void R_RadixSort( srfPoly_t *source, uint32_t size )
{
	srfPoly_t *scratch = (srfPoly_t *)alloca( sizeof( *scratch ) * r_maxPolys->i );
#ifdef GDR_LITTLE_ENDIAN
	R_Radix( 0, size, source, scratch );
	R_Radix( 1, size, scratch, source );
	R_Radix( 2, size, source, scratch );
	R_Radix( 3, size, scratch, source );
#else
	R_Radix( 3, size, source, scratch );
	R_Radix( 2, size, scratch, source );
	R_Radix( 1, size, source, scratch );
	R_Radix( 0, size, scratch, source );
#endif //GDR_LITTLE_ENDIAN
}

extern uint64_t r_numEntities;
extern uint64_t r_firstSceneEntity;

extern uint64_t r_numDLights;
extern uint64_t r_firstSceneDLight;

extern uint64_t r_numPolys;
extern uint64_t r_firstScenePoly;

extern uint64_t r_numPolyVerts;
extern uint64_t r_firstSceneVert;

static int SortPoly( const void *a, const void *b ) {
	return (int)( ( (srfPoly_t *)a )->hShader - ( (srfPoly_t *)b )->hShader );
}

void R_WorldToGL( drawVert_t *verts, vec3_t pos )
{
	vec3_t xyz[4];
	int i;
	vec2_t scale;

	VectorSet2( scale, 1.0f, 1.0f );

	ri.GLM_TransformToGL( pos, xyz, scale, 0.0f, glState.viewData.camera.viewProjectionMatrix );

	for ( i = 0; i < 4; ++i ) {
		VectorCopy( verts[i].xyz, xyz[i] );
	}
}

void R_WorldToGL2( polyVert_t *verts, vec3_t pos, uint32_t numVerts )
{
	vec3_t xyz[4];
	int i;
	vec2_t scale;

	VectorSet2( scale, 1.0f, 1.0f );

	ri.GLM_TransformToGL( pos, xyz, scale, 0.0f, glState.viewData.camera.viewProjectionMatrix );

	for ( i = 0; i < numVerts; ++i ) {
		VectorCopy( verts[i].xyz, xyz[i] );
	}
}

void R_ScreenToGL( polyVert_t *verts )
{
	vec3_t xyz[4];
	vec3_t pos;
	int i;
	vec2_t scale;

	VectorCopy( pos, verts[3].xyz );
	VectorSet2( scale, 1, 1 );

	ri.GLM_TransformToGL( pos, xyz, scale, 0.0f, glState.viewData.camera.viewProjectionMatrix );

	for ( i = 0; i < 4; i++ ) {
		VectorCopy( verts[i].xyz, xyz[i] );
	}
}

qboolean R_ClipTile( const vec3_t origin )
{
	const bbox_t cameraBounds = {
		.mins = {
			ceil( glState.viewData.camera.origin[0] ) - 7.0f,
			ceil( glState.viewData.camera.origin[1] ) - 7.0f
		},
		.maxs = {
			ceil( glState.viewData.camera.origin[0] ) + 7.0f,
			ceil( glState.viewData.camera.origin[1] ) + 7.0f
		}
	};

	return BoundsIntersectPoint( &cameraBounds, origin );
}

/*
* R_DrawPolys: draws all sprites and polygons submitted into the current scene
*/
void R_DrawPolys( void )
{
	uint64_t i, j;
	uint64_t firstIndex;
	srfPoly_t *poly;
	nhandle_t oldShader;
	uint64_t numVerts;
	vec3_t normal, edge1, edge2;
	vec3_t xyz[4];
	
	// no polygon submissions this frame
	if ( !r_numPolys && !r_numPolyVerts ) {
		return;
	}
	ri.ProfileFunctionBegin( "R_DrawPolys" );

	rg.world->drawing = qtrue;

	RE_ProcessDLights();

	RB_SetBatchBuffer( backend.drawBuffer, backendData[ 0 ]->verts, sizeof( srfVert_t ),
		backendData[ 0 ]->indices, sizeof( glIndex_t ) );

	// sort the polys to be more efficient with our shaders
	R_RadixSort( backend.refdef.polys, backend.refdef.numPolys );

	poly = backend.refdef.polys;
	oldShader = poly->hShader;
	backend.drawBatch.shader = R_GetShaderByHandle( oldShader );
	rg.world->drawing = qtrue;
	backend.drawBatch.instanced = qtrue;

	assert( backend.refdef.polys );

	for ( i = 0; i < backend.refdef.numPolys; i++ ) {
		if ( oldShader != backend.refdef.polys[i].hShader ) {
			// if we have a new shader, flush the current batch
			RB_FlushBatchBuffer();
			oldShader = backend.refdef.polys[i].hShader;
			backend.drawBatch.shader = R_GetShaderByHandle( backend.refdef.polys[i].hShader );
			backend.drawBatch.instanceCount = 0;
		}
		
		if ( backend.drawBatch.vtxOffset + backend.refdef.polys[i].numVerts >= r_maxPolys->i * 4
			|| backend.drawBatch.idxOffset + ( (int64_t)( backend.refdef.polys[i].numVerts ) - 2 ) >= r_maxPolys->i * 6 )
		{
			RB_FlushBatchBuffer();
		}
		
		// generate fan indexes into the buffer
		for ( j = 0; j < backend.refdef.polys[i].numVerts - 2; j++ ) {
			backendData[ rg.smpFrame ]->indices[ backend.drawBatch.idxOffset + 0 ] = backend.drawBatch.vtxOffset;
			backendData[ rg.smpFrame ]->indices[ backend.drawBatch.idxOffset + 1 ] = backend.drawBatch.vtxOffset + j + 1;
			backendData[ rg.smpFrame ]->indices[ backend.drawBatch.idxOffset + 2 ] = backend.drawBatch.vtxOffset + j + 2;
			backend.drawBatch.idxOffset += 3;
		}
		
		for ( j = 0; j < backend.refdef.polys[i].numVerts; j++ ) {
			VectorCopy2( backendData[ rg.smpFrame ]->verts[ backend.drawBatch.vtxOffset ].xyz, backend.refdef.polys[i].verts[j].xyz );
			VectorCopy2( backendData[ rg.smpFrame ]->verts[ backend.drawBatch.vtxOffset ].st, backend.refdef.polys[i].verts[j].uv );
			VectorCopy2( backendData[ rg.smpFrame ]->verts[ backend.drawBatch.vtxOffset ].worldPos, backend.refdef.polys[i].verts[j].worldPos );
			backendData[ rg.smpFrame ]->verts[ backend.drawBatch.vtxOffset ].color.u32 = backend.refdef.polys[i].verts[j].modulate.u32;

			backend.drawBatch.vtxOffset++;
		}
		backend.drawBatch.instanceCount++;
	}
	
	// flush out anything remaining
	RB_FlushBatchBuffer();
	rg.world->drawing = qfalse;

	ri.ProfileFunctionEnd();
}

void R_DrawWorld( void )
{
	uint32_t y, x;
	uint32_t i;
	uint64_t j;
	vec3_t pos;
	vec3_t origin;
	drawVert_t *vtx;
	drawVert_t verts[4];
	vec2_t *xyz;
	vec3_t edge1, edge2;
	vec2_t deltaUV1, deltaUV2;
	texCoord_t *uv;
	vec3_t *tangent, *bitangent;
	vec3_t *normal;
	float f;
	static vec3_t cameraPos;
	const renderEntityDef_t *refEntity;

	if ( ( backend.refdef.flags & RSF_NOWORLDMODEL ) ) {
		// nothing to draw
		return;
	}

	ri.ProfileFunctionBegin( "R_DrawWorld" );

	if ( !rg.world ) {
		ri.Error( ERR_FATAL, "R_DrawWorld: no world model loaded" );
	}

	// prepare the batch
	RB_SetBatchBuffer( rg.world->buffer, rg.world->vertices, sizeof( vec2_t ), rg.world->indices, sizeof( glIndex_t ) );
	GL_CheckErrors();

	backend.drawBatch.shader = rg.world->shader;
	rg.world->drawing = qtrue;
	backend.drawBatch.instanceCount = 1;
	backend.drawBatch.instanced = qtrue;

	xyz = rg.world->xyz;
	
	// if we haven't moved at all, we don't need to do any redundant calculations
	if ( !( ri.Cvar_VariableInteger( "g_paused" ) || VectorCompare( cameraPos, glState.viewData.camera.origin ) ) ) {
		for ( y = 0; y < rg.world->height; y++ ) {
			for ( x = 0; x < rg.world->width; x++ ) {
				pos[0] = x;
				pos[1] = rg.world->height - y;
				pos[2] = 0.0f;

				// convert the local world coordinates to OpenGL screen coordinates
				R_WorldToGL( verts, pos );

				VectorCopy2( xyz[ 0 ], verts[0].xyz );
				VectorCopy2( xyz[ 1 ], verts[1].xyz );
				VectorCopy2( xyz[ 2 ], verts[2].xyz );
				VectorCopy2( xyz[ 3 ], verts[3].xyz );

				xyz += 4;
			}
		}
	}
	VectorCopy( cameraPos, glState.viewData.camera.origin );

	backend.drawBatch.idxOffset = rg.world->numIndices;
	backend.drawBatch.vtxOffset = rg.world->numVertices;

	// flush it we have anything left in there
	RB_FlushBatchBuffer();
	rg.world->drawing = qfalse;

	backend.drawBatch.instanced = qfalse;

	ri.ProfileFunctionEnd();
}

void R_RenderView( const viewData_t *parms )
{
	uint32_t i;
	rg.viewCount++;

	memcpy( &glState.viewData, parms, sizeof( *parms ) );

	// setup the correct matrices
	RB_MakeViewMatrix();

	// draw the world
	RE_AddDrawWorldCmd();

	// draw any queued up images
	R_IssuePendingRenderCommands();
}


static void R_CalcSpriteTextureCoords( uint32_t x, uint32_t y, uint32_t spriteWidth, uint32_t spriteHeight,
	uint32_t sheetWidth, uint32_t sheetHeight, spriteCoord_t texCoords )
{
	const vec2_t min = { ( ( (float)x + 1 ) * spriteWidth ) / sheetWidth, ( ( (float)y + 1 ) * spriteHeight ) / sheetHeight };
	const vec2_t max = { ( (float)x * spriteWidth ) / sheetWidth, ( (float)y * spriteHeight ) / sheetHeight };

	texCoords[0][0] = min[0];
	texCoords[0][1] = max[1];

	texCoords[1][0] = min[0];
	texCoords[1][1] = min[1];

	texCoords[2][0] = max[0];
	texCoords[2][1] = min[1];
	
	texCoords[3][0] = max[0];
	texCoords[3][1] = max[1];
}

/*
void R_ScreenToGL(vec3_t *xyz)
{
	xyz[0][0] = 2.0f * xyz[0][0] / glConfig.vidWidth - 1.0f;
	xyz[0][1] = 1.0f - 2.0f * xyz[0][1] / glConfig.vidHeight;

	xyz[1][0] = 2.0f * xyz[1][0] / glConfig.vidWidth - 1.0f;
	xyz[1][1] = 1.0f - 2.0f * xyz[1][1] / glConfig.vidHeight;

	xyz[2][0] = 2.0f * xyz[2][0] / glConfig.vidWidth - 1.0f;
	xyz[2][1] = 1.0f - 2.0f * xyz[2][1] / glConfig.vidHeight;

	xyz[3][0] = 2.0f * xyz[3][0] / glConfig.vidWidth - 1.0f;
	xyz[3][1] = 1.0f - 2.0f * xyz[3][1] / glConfig.vidHeight;
}
*/

nhandle_t RE_RegisterSpriteSheet( const char *npath, uint32_t sheetWidth, uint32_t sheetHeight, uint32_t spriteWidth, uint32_t spriteHeight )
{
	spriteSheet_t *sheet;
	uint64_t len;
	uint64_t size;
	uint64_t i;
	nhandle_t handle;
	uint32_t numSprites, spriteCountY, spriteCountX;
	sprite_t *sprite;

	len = strlen( npath );

	if ( !( ( (uint32_t)sheetWidth % 2 ) == 0 && ( (uint32_t)sheetHeight % 2 ) == 0 )
		|| !( ( (uint32_t)spriteWidth % 2 ) == 0 && ( (uint32_t)spriteHeight % 2 ) == 0 ) )
	{
		ri.Error( ERR_DROP, "RE_RegisterSpriteSheet: please ensure your sprite dimensions and sheet dimensions are powers of two" );
	}
	if ( len >= MAX_NPATH ) {
		ri.Error( ERR_DROP, "RE_RegisterSpriteSheet: name '%s' too long", npath );
	}

	//
	// check if we already have it
	//
	for ( i = 0; i < rg.numSpriteSheets; i++ ) {
		if ( !rg.sheets[i] ) {
			continue;
		}
		if ( !N_stricmp( npath, rg.sheets[i]->name ) ) {
			return (nhandle_t)i;
		}
	}

	numSprites = ( sheetWidth / spriteWidth ) * ( sheetHeight / spriteHeight );

	size = 0;
	size += PAD( sizeof( *sheet ), sizeof( uintptr_t ) );
	size += PAD( sizeof( *sheet->sprites ) * numSprites, sizeof( uintptr_t ) );
	size += PAD( len + 1, sizeof( uintptr_t ) );

	handle = rg.numSpriteSheets;
	sheet = rg.sheets[rg.numSpriteSheets] = (spriteSheet_t *)ri.Hunk_Alloc( size, h_low );
	memset( sheet, 0, size );
	sheet->name = (char *)( sheet + 1 );
	sheet->sprites = (sprite_t *)( sheet->name + len + 1 );

	strcpy( sheet->name, npath );

	sheet->numSprites = numSprites;
	sheet->sheetWidth = sheetWidth;
	sheet->sheetHeight = sheetHeight;
	sheet->spriteWidth = spriteWidth;
	sheet->spriteHeight = spriteHeight;

	spriteCountX = sheetWidth / spriteWidth;
	spriteCountY = sheetHeight / spriteHeight;

	ri.Printf( PRINT_DEVELOPER, "Generate sprite sheet, [ %u, %u ]:[ %u, %u ], %u sprites\n", sheetWidth, sheetHeight, spriteWidth, spriteHeight,
		numSprites );

	sheet->hShader = RE_RegisterShader( npath );
	if ( sheet->hShader == FS_INVALID_HANDLE ) {
		ri.Printf( PRINT_WARNING, "RE_RegisterSpriteSheet: failed to find shader '%s'.\n", npath );
		return FS_INVALID_HANDLE;
	}

	sprite = sheet->sprites;
	for ( uint32_t y = 0; y < spriteCountY; y++ ) {
		for ( uint32_t x = 0; x < spriteCountX; x++ ) {
			R_CalcSpriteTextureCoords( x, y, spriteWidth, spriteHeight, sheetWidth, sheetHeight, sheet->sprites[y * spriteCountX + x].texCoords );
		}
	}

	rg.numSpriteSheets++;

	return handle;
}

nhandle_t RE_RegisterSprite( nhandle_t hSpriteSheet, uint32_t index )
{
	spriteSheet_t *sheet;

	if ( hSpriteSheet == FS_INVALID_HANDLE ) {
		ri.Printf( PRINT_WARNING, "RE_RegisterSprite: bad sprite sheet given.\n" );
		return FS_INVALID_HANDLE;
	}

	sheet = rg.sheets[hSpriteSheet];

	if ( index >= sheet->numSprites ) {
		ri.Printf( PRINT_WARNING, "RE_RegisterSprite: invalid index given.\n" );
	}

	return (nhandle_t)index; // nothing too fancy...
}

/*
=============
R_CalcTexDirs

Lengyel, Eric. �Computing Tangent Space Basis Vectors for an Arbitrary Mesh�. Terathon Software 3D Graphics Library, 2001. http://www.terathon.com/code/tangent.html
=============
*/
void R_CalcTexDirs(vec3_t sdir, vec3_t tdir, const vec3_t v1, const vec3_t v2,
				   const vec3_t v3, const vec2_t w1, const vec2_t w2, const vec2_t w3)
{
	float			x1, x2, y1, y2, z1, z2;
	float			s1, s2, t1, t2, r;

	x1 = v2[0] - v1[0];
	x2 = v3[0] - v1[0];
	y1 = v2[1] - v1[1];
	y2 = v3[1] - v1[1];
	z1 = v2[2] - v1[2];
	z2 = v3[2] - v1[2];

	s1 = w2[0] - w1[0];
	s2 = w3[0] - w1[0];
	t1 = w2[1] - w1[1];
	t2 = w3[1] - w1[1];

	r = s1 * t2 - s2 * t1;
	if (r) r = 1.0f / r;

	VectorSet(sdir, (t2 * x1 - t1 * x2) * r, (t2 * y1 - t1 * y2) * r, (t2 * z1 - t1 * z2) * r);
	VectorSet(tdir, (s1 * x2 - s2 * x1) * r, (s1 * y2 - s2 * y1) * r, (s1 * z2 - s2 * z1) * r);
}

/*
=============
R_CalcTangentSpace

Lengyel, Eric. �Computing Tangent Space Basis Vectors for an Arbitrary Mesh�. Terathon Software 3D Graphics Library, 2001. http://www.terathon.com/code/tangent.html
=============
*/

/*
vec_t R_CalcTangentSpace( vec3_t tangent, vec3_t bitangent, const vec3_t normal, const vec3_t sdir, const vec3_t tdir )
{
	vec3_t n_cross_t;
	vec_t n_dot_t, handedness;

	// Gram-Schmidt orthogonalize
	n_dot_t = DotProduct(normal, sdir);
	VectorMA(sdir, -n_dot_t, normal, tangent);
	VectorNormalize(tangent);

	// Calculate handedness
	CrossProduct(normal, sdir, n_cross_t);
	handedness = (DotProduct(n_cross_t, tdir) < 0.0f) ? -1.0f : 1.0f;

	// Calculate orthogonal bitangent, if necessary
	if (bitangent)
		CrossProduct(normal, tangent, bitangent);

	return handedness;
}

qboolean R_CalcTangentVectors(drawVert_t dv[3])
{
	int             i;
	float           bb, s, t;
	vec3_t          bary;


	// calculate barycentric basis for the triangle
	bb = (dv[1].uv[0] - dv[0].uv[0]) * (dv[2].uv[1] - dv[0].uv[1]) - (dv[2].uv[0] - dv[0].uv[0]) * (dv[1].uv[1] - dv[0].uv[1]);
	if(fabs(bb) < 0.00000001f)
		return qfalse;

	// do each vertex
	for(i = 0; i < 3; i++)
	{
		vec4_t tangent;
		vec3_t normal, bitangent, nxt;

		// calculate s tangent vector
		s = dv[i].uv[0] + 10.0f;
		t = dv[i].uv[1];
		bary[0] = ((dv[1].uv[0] - s) * (dv[2].uv[1] - t) - (dv[2].uv[0] - s) * (dv[1].uv[1] - t)) / bb;
		bary[1] = ((dv[2].uv[0] - s) * (dv[0].uv[1] - t) - (dv[0].uv[0] - s) * (dv[2].uv[1] - t)) / bb;
		bary[2] = ((dv[0].uv[0] - s) * (dv[1].uv[1] - t) - (dv[1].uv[0] - s) * (dv[0].uv[1] - t)) / bb;

		tangent[0] = bary[0] * dv[0].xyz[0] + bary[1] * dv[1].xyz[0] + bary[2] * dv[2].xyz[0];
		tangent[1] = bary[0] * dv[0].xyz[1] + bary[1] * dv[1].xyz[1] + bary[2] * dv[2].xyz[1];
		tangent[2] = bary[0] * dv[0].xyz[2] + bary[1] * dv[1].xyz[2] + bary[2] * dv[2].xyz[2];

		VectorSubtract(tangent, dv[i].xyz, tangent);
		VectorNormalize(tangent);

		// calculate t tangent vector
		s = dv[i].uv[0];
		t = dv[i].uv[1] + 10.0f;
		bary[0] = ((dv[1].uv[0] - s) * (dv[2].uv[1] - t) - (dv[2].uv[0] - s) * (dv[1].uv[1] - t)) / bb;
		bary[1] = ((dv[2].uv[0] - s) * (dv[0].uv[1] - t) - (dv[0].uv[0] - s) * (dv[2].uv[1] - t)) / bb;
		bary[2] = ((dv[0].uv[0] - s) * (dv[1].uv[1] - t) - (dv[1].uv[0] - s) * (dv[0].uv[1] - t)) / bb;

		bitangent[0] = bary[0] * dv[0].xyz[0] + bary[1] * dv[1].xyz[0] + bary[2] * dv[2].xyz[0];
		bitangent[1] = bary[0] * dv[0].xyz[1] + bary[1] * dv[1].xyz[1] + bary[2] * dv[2].xyz[1];
		bitangent[2] = bary[0] * dv[0].xyz[2] + bary[1] * dv[1].xyz[2] + bary[2] * dv[2].xyz[2];

		VectorSubtract(bitangent, dv[i].xyz, bitangent);
		VectorNormalize(bitangent);

		// store bitangent handedness
		R_VaoUnpackNormal(normal, dv[i].normal);
		CrossProduct(normal, tangent, nxt);
		tangent[3] = (DotProduct(nxt, bitangent) < 0.0f) ? -1.0f : 1.0f;

		R_VaoPackTangent(dv[i].tangent, tangent);

		// debug code
		//% Sys_FPrintf( SYS_VRB, "%d S: (%f %f %f) T: (%f %f %f)\n", i,
		//%     stv[ i ][ 0 ], stv[ i ][ 1 ], stv[ i ][ 2 ], ttv[ i ][ 0 ], ttv[ i ][ 1 ], ttv[ i ][ 2 ] );
	}

	return qtrue;
}
*/