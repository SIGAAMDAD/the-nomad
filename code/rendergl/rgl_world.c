#include "rgl_local.h"

//static world_t rg.world->
static byte *fileBase;

static void R_LoadLights( const lump_t *lights, world_t *world )
{
	uint32_t count;
	maplight_t *in, *out;

	in = (maplight_t *)(fileBase + lights->fileofs);
	if ( !lights->length ) { // not strictly required
		return;
	}
	if ( lights->length % sizeof( *in ) ) {
		ri.Error( ERR_DROP, "RE_LoadWorldMap: funny lump size (lights) in %s", world->name );
	}
	
	count = lights->length / sizeof(*in);
	out = ri.Hunk_Alloc( sizeof(*out) * count, h_low );

	world->lights = out;
	world->numLights = count;

	memcpy(out, in, count*sizeof(*out));
}

static void R_LoadTiles( const lump_t *tiles, world_t *world )
{
	uint32_t count;
	maptile_t *in, *out;

	in = (maptile_t *)(fileBase + tiles->fileofs);
	if ( tiles->length % sizeof( *in ) ) {
		ri.Error( ERR_DROP, "RE_LoadWorldMap: funny lump size (tiles) in %s", world->name );
	}
	
	count = tiles->length / sizeof(*in);
	out = ri.Hunk_Alloc( sizeof(*out) * count, h_low );

	world->tiles = out;
	world->numTiles = count;

	memcpy(out, in, count*sizeof(*out));
}

static spriteCoord_t *R_LoadTileset( const lump_t *sprites, const tile2d_header_t *theader, world_t *world )
{
	uint32_t count, i;
	spriteCoord_t *in, *out;

	in = (spriteCoord_t *)( fileBase + sprites->fileofs );
	if ( sprites->length % sizeof( *out ) ) {
		ri.Error( ERR_DROP, "RE_LoadWorldMap: funny lump size (tileset) in %s", world->name );
	}
	
	count = sprites->length / sizeof(*in);
	out = ri.Hunk_AllocateTempMemory( sizeof( *out ) * count );

	memcpy( out, in, sizeof( *out ) * count );
	
	return out;
}

static void R_CalcSpriteTextureCoords( uint32_t x, uint32_t y, uint32_t spriteWidth, uint32_t spriteHeight,
	uint32_t sheetWidth, uint32_t sheetHeight, spriteCoord_t *texCoords )
{
	const vec2_t min = { (((float)x + 1) * (float)spriteWidth) / (float)sheetWidth,
		(((float)y + 1) * (float)spriteHeight) / (float)sheetHeight };
	const vec2_t max = { ((float)x * (float)spriteWidth) / (float)sheetWidth,
		((float)y * (float)spriteHeight) / (float)sheetHeight };

	(*texCoords)[0][0] = min[0];
	(*texCoords)[0][1] = max[1];

	(*texCoords)[1][0] = min[0];
	(*texCoords)[1][1] = min[1];

	(*texCoords)[2][0] = max[0];
	(*texCoords)[2][1] = min[1];
	
	(*texCoords)[3][0] = max[0];
	(*texCoords)[3][1] = max[1];
}

static void R_GenerateTexCoords( tile2d_info_t *info )
{
	uint32_t y, x;
	uint32_t i;
	float scaleWidth, scaleHeight;
	uint32_t sheetWidth, sheetHeight;
	const texture_t *image;
	char texture[MAX_NPATH];
	vec3_t tmp1, tmp2;
	drawVert_t *vtx;
	spriteCoord_t *sprites;
	vec2_t *xyz;
	texCoord_t *uv;
	color4ub_t *color;
	worldPos_t *worldPos;

	COM_StripExtension( info->texture, texture, sizeof( texture ) );
	if ( texture[ strlen( texture ) - 1 ] == '.' ) {
		texture[ strlen( texture ) - 1 ] = '\0';
	}

	R_FindImageFile( info->texture, IMGTYPE_COLORALPHA, IMGFLAG_CLAMPTOEDGE );
	rg.world->shader = R_GetShaderByHandle( RE_RegisterShader( texture ) );
	if ( rg.world->shader == rg.defaultShader ) {
		ri.Error( ERR_DROP, "RE_LoadWorldMap: failed to load shader for '%s'", rg.world->name );
	}

	image = rg.world->shader->stages[0]->bundle[0].image[0];

	// we might be getting higher quality textures, so scale coordinates appropriately
	scaleWidth = image->width / info->imageWidth;
	scaleHeight = image->height / info->imageHeight;

	info->tileCountX = info->imageWidth / info->tileWidth;
	info->tileCountY = info->imageHeight / info->tileHeight;

	info->tileWidth *= scaleWidth;
	info->tileHeight *= scaleHeight;

	sheetWidth = info->tileCountX * info->tileWidth;
	sheetHeight = info->tileCountY * info->tileHeight;

	ri.Printf( PRINT_DEVELOPER, "Generating worldData tileset %ux%u:%ux%u, %u sprites, (%ux%u tiles, %0.02fx%0.02f scale)\n", info->imageWidth, info->imageHeight,
		info->tileWidth, info->tileHeight, info->numTiles, info->tileCountX, info->tileCountY, scaleWidth, scaleHeight );

	sprites = (spriteCoord_t *)ri.Hunk_AllocateTempMemory( sizeof( *sprites ) * info->tileCountX * info->tileCountY );

	for ( y = 0; y < info->tileCountY; y++ ) {
		for ( x = 0; x < info->tileCountX; x++ ) {
			R_CalcSpriteTextureCoords( x, y, info->tileWidth, info->tileHeight, sheetWidth, sheetHeight,
				&sprites[ y * info->tileCountX + x ] );
		}
	}

	xyz = rg.world->xyz;
	uv = rg.world->uv;
	worldPos = rg.world->worldPos;
	color = rg.world->color;
	for ( y = 0; y < rg.world->height; y++ ) {
		for ( x = 0; x < rg.world->width; x++ ) {
		#if 0
			VectorCopy2( uv[0], sprites[ rg.world->tiles[ y * rg.world->width + x ].index ][0] );
			VectorCopy2( uv[1], sprites[ rg.world->tiles[ y * rg.world->width + x ].index ][1] );
			VectorCopy2( uv[2], sprites[ rg.world->tiles[ y * rg.world->width + x ].index ][2] );
			VectorCopy2( uv[3], sprites[ rg.world->tiles[ y * rg.world->width + x ].index ][3] );
		#else
			memcpy( uv, sprites[ rg.world->tiles[ y * rg.world->width + x ].index ], sizeof( spriteCoord_t ) );
		#endif

			VectorSet2( worldPos[0], x, y );
			VectorSet2( worldPos[1], x, y );
			VectorSet2( worldPos[2], x, y );
			VectorSet2( worldPos[3], x, y );

			VectorCopy4( color[0].rgba, colorWhite );
			VectorCopy4( color[1].rgba, colorWhite );
			VectorCopy4( color[2].rgba, colorWhite );
			VectorCopy4( color[3].rgba, colorWhite );
			
			xyz += 4;
			uv += 4;
			worldPos += 4;
			color += 4;
		}
	}
//	R_SetupTileLighting();
	ri.Hunk_FreeTempMemory( sprites );
}


float stsvco_valenceScore( const uint32_t numTris ) {
    return 2 * powf( numTris, -0.5f );
}

#define LRU_CACHE_SIZE 64

static void R_OptimizeVertexCache( void )
{
	glIndex_t *pIndices;
	drawVert_t *pVerts;
	uint64_t i, j, t, v;

	typedef struct {
		int numAdjecentTris;
		int numTrisLeft;
		int triListIndex;
		int cacheIndex;
	} vertex_t;

	typedef struct {
		int vertices[3];
		qboolean drawn;
	} triangle_t;

	vertex_t *vertices;
	triangle_t *triangles;
	uint32_t *vertToTri;

	const uint32_t numTriangles = rg.world->numIndices / 3;

	pIndices = rg.world->indices;
	pVerts = rg.world->vertices;

	vertices = ri.Hunk_AllocateTempMemory( rg.world->numVertices * sizeof( *vertices ) );
	triangles = ri.Hunk_AllocateTempMemory( numTriangles * sizeof( *triangles ) );
	
	for ( i = 0; i < rg.world->numVertices; ++i ) {
		vertices[i].numAdjecentTris = 0;
		vertices[i].numTrisLeft = 0;
		vertices[i].triListIndex = 0;
		vertices[i].cacheIndex = -1;
	}
	
	for ( i = 0; i < numTriangles; ++i ) {
		for ( j = 0; j < 3; ++j ) {
			triangles[i].vertices[j] = pIndices[ i * 3 + j ];
			++vertices[ triangles[i].vertices[j] ].numAdjecentTris;
		}
		triangles[i].drawn = false;
	}
	
	// Loop through and find index for the tri list for vertex->tri
	for ( i = 1; i < rg.world->numVertices; ++i ) {
		vertices[i].triListIndex = vertices[ i - 1 ].triListIndex+vertices[ i - 1 ].numAdjecentTris;
	}
	
	const int numVertToTri = vertices[ rg.world->numVertices - 1 ].triListIndex+vertices[ rg.world->numVertices - 1 ].numAdjecentTris;
	vertToTri = ri.Hunk_AllocateTempMemory( numVertToTri * sizeof( *vertToTri ) );
	
	for ( i = 0; i < numTriangles; ++i ) {
		for ( j = 0; j < 3; ++j ) {
			const int index = triangles[ i ].vertices[ j ];
			const int triListIndex = vertices[ index ].triListIndex + vertices[index].numTrisLeft;
			vertToTri[ triListIndex ] = i;
			++vertices[index].numTrisLeft;
		}
	}
	
	// Make LRU cache
	const int LRUCacheSize = 64;
	int *LRUCache = alloca( LRUCacheSize * sizeof( *LRUCache ) );
	float *scoring = alloca( LRUCacheSize * sizeof( *scoring ) );

	for ( i = 0; i < LRUCacheSize; ++i ) {
		LRUCache[i] = -1;
		scoring[i] = -1.0f;
	}
	
	int numIndicesDone = 0;
	while ( numIndicesDone != rg.world->numIndices ) {
		// update vertex scoring
		for ( i = 0; i < LRUCacheSize && LRUCache[i] >= 0; ++i ) {
			int vertexIndex = LRUCache[i];
			if ( vertexIndex != -1 ) {
				// Do scoring based on cache position
				if ( i < 3 ) {
					scoring[i] = 0.75f;
				} else {
					const float scaler = 1.0f / ( LRUCacheSize - 3 );
					const float scoreBase = 1.0f - ( i - 3 ) * scaler;
					scoring[i] = powf ( scoreBase, 1.5f );
				}
				// Add score based on tris left for vertex (valence score)
				const int numTrisLeft = vertices[ vertexIndex ].numTrisLeft;
				scoring[i] += stsvco_valenceScore(numTrisLeft);
			}
		}
		// find triangle to draw based on score
		// Update score for all triangles with vertexes in cache
		int triangleToDraw = -1;
		float bestTriScore = 0.0f;
		for ( i = 0; i < LRUCacheSize && LRUCache[i] >= 0; ++i ) {
			const int vIndex = LRUCache[i];
			if ( vertices[vIndex].numTrisLeft > 0 ) {
				for ( t = 0; t < vertices[vIndex].numAdjecentTris; ++t ) {
					const int tIndex = vertToTri[ vertices[vIndex].triListIndex + t];
					if ( !triangles[ tIndex ].drawn ) {
						float triScore = .0f;
						for ( v = 0; v < 3; ++v ) {
							const int cacheIndex = vertices[ triangles[ tIndex ].vertices[v] ].cacheIndex;
							if ( cacheIndex >= 0 ) {
								triScore += scoring[ cacheIndex ];
							}
						}
						if ( triScore > bestTriScore ) {
							triangleToDraw = tIndex;
							bestTriScore = triScore;
						}
					}
				}
			}
		}
		
		if ( triangleToDraw < 0 ) {
			// No triangle can be found by heuristic, simply choose first and best
			for ( t = 0; t < numTriangles; ++t ) {
				if ( !triangles[t].drawn ) {
					//compute valence for each vertex
					float triScore = .0f;
					for ( v = 0; v < 3; ++v ) {
						const uint32_t vertexIndex = triangles[t].vertices[v];
						// Add score based on tris left for vertex (valence score)
						const int numTrisLeft = vertices[ vertexIndex ].numTrisLeft;
						triScore += stsvco_valenceScore( numTrisLeft );
					}
					if ( triScore >= bestTriScore ) {
						triangleToDraw = t;
						bestTriScore = triScore;
					}
				}
			}
		}
		
		// update cache
		int cacheIndex = 3;
		int numVerticesFound = 0;
		while ( LRUCache[numVerticesFound] >= 0 && numVerticesFound < 3 && cacheIndex < LRUCacheSize ) {
			qboolean topOfCacheInTri = qfalse;
			// Check if index is in triangle
			for ( i = 0; i < 3; ++i ) {
				if( triangles[triangleToDraw].vertices[i] == LRUCache[numVerticesFound]) {
					++numVerticesFound;
					topOfCacheInTri = qtrue;
					break;
				}
			}
			
			if ( !topOfCacheInTri ) {
				int topIndex = LRUCache[ numVerticesFound ];
				for ( int j = numVerticesFound; j < 2; ++j) {
					LRUCache[j] = LRUCache[j+1];
				}
				LRUCache[2] = LRUCache[cacheIndex];
				if ( LRUCache[2] >= 0 ) {
					vertices[ LRUCache[2] ].cacheIndex = -1;
				}
				
				LRUCache[cacheIndex] = topIndex;
				if ( topIndex >= 0 ) {
					vertices[ topIndex ].cacheIndex = cacheIndex;
				}
				++cacheIndex;
			}
		}
		
		// Set triangle as drawn
		for ( v = 0; v < 3; ++v ) {
			const int index = triangles[triangleToDraw].vertices[v];
			
			LRUCache[ v ] = index;
			vertices[ index ].cacheIndex = v;
			
			--vertices[index].numTrisLeft;
			
			pIndices[ numIndicesDone ] = index;
			++numIndicesDone;
		}
		
		
		triangles[ triangleToDraw ].drawn = qtrue;
	}
	// Memory cleanup
	ri.Hunk_FreeTempMemory( vertToTri );
	ri.Hunk_FreeTempMemory( triangles );
	ri.Hunk_FreeTempMemory( vertices );
}

float R_CalcCacheEfficiency( void )
{
	uint32_t numCacheMisses;
	glIndex_t index;
	int cache[ LRU_CACHE_SIZE ];
	int i, c;
	qboolean foundInCache;

	numCacheMisses = 0;

    for ( i = 0; i < LRU_CACHE_SIZE; ++i ) {
		cache[i] = -1;
	}
    
    for ( i = 0; i < rg.world->numIndices; ++i ) {
    	index = rg.world->indices[i];

        // check if vertex in cache
        foundInCache = qfalse;
        for ( c = 0; c < LRU_CACHE_SIZE && cache[c] >= 0 && !foundInCache; ++c) {
            if ( cache[ c ] == index ) {
				foundInCache = qtrue;
			}
        }
        
        if ( !foundInCache ) {
            ++numCacheMisses;
            for ( c = LRU_CACHE_SIZE - 1; c  >= 1; --c ) {
                cache[c] = cache[ c-1 ];
            }
            cache[0] = index;
        }
    }
    
    return (float)numCacheMisses / (float)( rg.world->numIndices / 3 );
}

static void R_ProcessLights( void )
{
	shaderLight_t *lights;
	const maplight_t *data;
	uint32_t i, x, y;
	dirtype_t dir;
	char extradefines[1024];
	uint32_t attribs;

	extern const char *fallbackShader_tile_vp;
	extern const char *fallbackShader_tile_fp;
	extern const char *fallbackShader_sprite_vp;
	extern const char *fallbackShader_sprite_fp;

	ri.Printf( PRINT_DEVELOPER, "Processing %u lights\n", rg.world->numLights );

	if ( r_dynamiclight->i ) {
		rg.lightData = GLSL_InitUniformBuffer( "u_LightBuffer", NULL, sizeof( shaderLight_t ) * ( rg.world->numLights + r_maxDLights->i ), qfalse );
	} else {
		rg.lightData = GLSL_InitUniformBuffer( "u_LightBuffer", NULL, sizeof( shaderLight_t ) * rg.world->numLights, qfalse );
	}

	attribs = ATTRIB_POSITION | ATTRIB_TEXCOORD | ATTRIB_WORLDPOS | ATTRIB_COLOR;

	extradefines[0] = '\0';
	N_strcat( extradefines, sizeof( extradefines ) - 1, "#define USE_LIGHT\n" );
	N_strcat( extradefines, sizeof( extradefines ) - 1, va( "#define MAX_TEXTURES %i\n", MAX_TEXTURES ) );
	N_strcat( extradefines, sizeof( extradefines ) - 1, va( "#define MAX_MAP_LIGHTS %i\n", rg.world->numLights ) );
	if ( r_dynamiclight->i ) {
		N_strcat( extradefines, sizeof( extradefines ) - 1, va( "#define MAX_LIGHTS %i\n", rg.world->numLights + r_maxDLights->i ) );
	} else {
		N_strcat( extradefines, sizeof( extradefines ) - 1, va( "#define MAX_LIGHTS %i\n", rg.world->numLights ) );
	}
	N_strcat( extradefines, sizeof( extradefines ) - 1, va( "#define POINT_LIGHT %i\n", LIGHT_POINT ) );
	N_strcat( extradefines, sizeof( extradefines ) - 1, va( "#define DIRECTION_LIGHT %i\n", LIGHT_DIRECTIONAL ) );
	/*
	if ( r_normalMapping->i ) {
		N_strcat( extradefines, sizeof( extradefines ) - 1, "#define USE_NORMALMAP\n" );
	}
	if ( r_specularMapping->i ) {
		N_strcat( extradefines, sizeof( extradefines ) - 1, "#define USE_SPECULARMAP\n" );
	}
	if ( r_parallaxMapping->i ) {
		N_strcat( extradefines, sizeof( extradefines ) - 1, "#define USE_PARALLAXMAP\n" );
	}
	*/
	ri.Printf( PRINT_INFO, "Compiling tile shader...\n" );
	if ( !GLSL_InitGPUShader( &rg.tileShader, "tile", attribs, qtrue, extradefines, qtrue, fallbackShader_tile_vp, fallbackShader_tile_fp ) ) {
		ri.Error( ERR_FATAL, "Could not load tile shader!" );
	}

	GLSL_LinkUniformToShader( &rg.tileShader, UNIFORM_LIGHTDATA, rg.lightData, qfalse, 0 );

	GLSL_InitUniforms( &rg.tileShader );
	GLSL_FinishGPUShader( &rg.tileShader );

	ri.Printf( PRINT_INFO, "Compiling sprite shader...\n" );
	if ( !GLSL_InitGPUShader( &rg.spriteShader, "sprite", attribs, qtrue, extradefines, qtrue, fallbackShader_sprite_vp, fallbackShader_sprite_fp ) ) {
		ri.Error( ERR_FATAL, "Could not load sprite shader!" );
	}

	GLSL_LinkUniformToShader( &rg.spriteShader, UNIFORM_LIGHTDATA, rg.lightData, qfalse, 0 );

	GLSL_InitUniforms( &rg.spriteShader );
	GLSL_FinishGPUShader( &rg.spriteShader );

	lights = (shaderLight_t *)rg.lightData->data;
	data = rg.world->lights;

	for ( i = 0; i < rg.world->numLights; i++ ) {
		VectorCopy4( lights[i].color, data[i].color );
		VectorCopy2( lights[i].origin, data[i].origin );
		lights[i].brightness = data[i].brightness;
		lights[i].range = data[i].range;
		lights[i].constant = data[i].constant;
		lights[i].linear = data[i].linear;
		lights[i].quadratic = data[i].quadratic;
		lights[i].type = data[i].type;
	}

	GLSL_UseProgram( &rg.tileShader );
	GLSL_SetUniformVec3( &rg.tileShader, UNIFORM_AMBIENTLIGHT, rg.world->ambientLightColor );
	if ( !r_dynamiclight->i ) {
		GLSL_SetUniformInt( &rg.tileShader, UNIFORM_NUM_LIGHTS, rg.world->numLights );
		GLSL_ShaderBufferData( &rg.tileShader, UNIFORM_LIGHTDATA, rg.lightData, sizeof( *lights ) * rg.world->numLights, 0, qfalse );
	}

	GLSL_UseProgram( &rg.spriteShader );
	GLSL_SetUniformVec3( &rg.spriteShader, UNIFORM_AMBIENTLIGHT, rg.world->ambientLightColor );
	if ( !r_dynamiclight->i ) {
		GLSL_SetUniformInt( &rg.spriteShader, UNIFORM_NUM_LIGHTS, rg.world->numLights );
		GLSL_ShaderBufferData( &rg.spriteShader, UNIFORM_LIGHTDATA, rg.lightData, sizeof( *lights ) * rg.world->numLights, 0, qfalse );
	}
}

static void R_InitWorldBuffer( tile2d_header_t *theader )
{
	maptile_t *tile;
	uint32_t numSurfs, i;
	uint32_t y, x;
	uint32_t offset;
	vertexAttrib_t attribs[ATTRIB_INDEX_COUNT];
	vec3_t pos;
	vec3_t edge1, edge2;
	vec2_t deltaUV1, deltaUV2;
	vec2_t *uv;
	vec3_t *tangent, *bitangent;
	vec3_t *normal;
	float f;

	rg.world->numIndices = rg.world->width * rg.world->height * 6;
	rg.world->numVertices = rg.world->width * rg.world->height * 4;

	memset( attribs, 0, sizeof( attribs ) );

	attribs[ATTRIB_INDEX_POSITION].enabled		= qtrue;
	attribs[ATTRIB_INDEX_TEXCOORD].enabled		= qtrue;
	attribs[ATTRIB_INDEX_COLOR].enabled			= qtrue;
	attribs[ATTRIB_INDEX_WORLDPOS].enabled		= qtrue;
	attribs[ATTRIB_INDEX_TANGENT].enabled		= qfalse;
	attribs[ATTRIB_INDEX_BITANGENT].enabled		= qfalse;
	attribs[ATTRIB_INDEX_NORMAL].enabled		= qfalse;

	attribs[ATTRIB_INDEX_POSITION].count		= 2;
	attribs[ATTRIB_INDEX_TEXCOORD].count		= 2;
	attribs[ATTRIB_INDEX_COLOR].count			= 4;
	attribs[ATTRIB_INDEX_WORLDPOS].count		= 2;
	attribs[ATTRIB_INDEX_TANGENT].count			= 3;
	attribs[ATTRIB_INDEX_BITANGENT].count		= 3;
	attribs[ATTRIB_INDEX_NORMAL].count			= 3;

	attribs[ATTRIB_INDEX_POSITION].type			= GL_FLOAT;
	attribs[ATTRIB_INDEX_TEXCOORD].type			= GL_FLOAT;
	attribs[ATTRIB_INDEX_COLOR].type			= GL_UNSIGNED_BYTE;
	attribs[ATTRIB_INDEX_WORLDPOS].type			= GL_FLOAT;
	attribs[ATTRIB_INDEX_TANGENT].type			= GL_FLOAT;
	attribs[ATTRIB_INDEX_BITANGENT].type		= GL_FLOAT;
	attribs[ATTRIB_INDEX_NORMAL].type			= GL_FLOAT;

	attribs[ATTRIB_INDEX_POSITION].index		= ATTRIB_INDEX_POSITION;
	attribs[ATTRIB_INDEX_TEXCOORD].index		= ATTRIB_INDEX_TEXCOORD;
	attribs[ATTRIB_INDEX_COLOR].index			= ATTRIB_INDEX_COLOR;
	attribs[ATTRIB_INDEX_WORLDPOS].index		= ATTRIB_INDEX_WORLDPOS;
	attribs[ATTRIB_INDEX_TANGENT].index			= ATTRIB_INDEX_TANGENT;
	attribs[ATTRIB_INDEX_BITANGENT].index		= ATTRIB_INDEX_BITANGENT;
	attribs[ATTRIB_INDEX_NORMAL].index			= ATTRIB_INDEX_NORMAL;

	attribs[ATTRIB_INDEX_POSITION].normalized	= GL_FALSE;
	attribs[ATTRIB_INDEX_TEXCOORD].normalized	= GL_FALSE;
	attribs[ATTRIB_INDEX_COLOR].normalized		= GL_TRUE;
	attribs[ATTRIB_INDEX_WORLDPOS].normalized	= GL_FALSE;
	attribs[ATTRIB_INDEX_TANGENT].normalized	= GL_FALSE;
	attribs[ATTRIB_INDEX_BITANGENT].normalized	= GL_FALSE;
	attribs[ATTRIB_INDEX_NORMAL].normalized		= GL_FALSE;

	attribs[ATTRIB_INDEX_POSITION].usage		= BUFFER_STREAM;
	attribs[ATTRIB_INDEX_TEXCOORD].usage		= BUFFER_STATIC;
	attribs[ATTRIB_INDEX_COLOR].usage			= BUFFER_STATIC;
	attribs[ATTRIB_INDEX_WORLDPOS].usage		= BUFFER_STATIC;
	attribs[ATTRIB_INDEX_TANGENT].usage			= BUFFER_STATIC;
	attribs[ATTRIB_INDEX_COLOR].usage			= BUFFER_STATIC;

	if ( glContext.directStateAccess ) {
		attribs[ATTRIB_INDEX_WORLDPOS].size			= sizeof( worldPos_t ) * rg.world->numVertices;
		attribs[ATTRIB_INDEX_POSITION].size			= sizeof( vec2_t ) * rg.world->numVertices;
		attribs[ATTRIB_INDEX_TEXCOORD].size			= sizeof( texCoord_t ) * rg.world->numVertices;
		attribs[ATTRIB_INDEX_COLOR].size			= sizeof( color4ub_t ) * rg.world->numVertices;
	} else {
		attribs[ATTRIB_INDEX_WORLDPOS].offset		= 0;
		attribs[ATTRIB_INDEX_POSITION].offset		= attribs[ ATTRIB_INDEX_WORLDPOS ].offset + ( sizeof( worldPos_t ) * rg.world->numVertices );
		attribs[ATTRIB_INDEX_TEXCOORD].offset		= attribs[ ATTRIB_INDEX_POSITION ].offset + ( sizeof( vec2_t ) * rg.world->numVertices );
		attribs[ATTRIB_INDEX_COLOR].offset			= attribs[ ATTRIB_INDEX_TEXCOORD ].offset + ( sizeof( texCoord_t ) * rg.world->numVertices );
	}

	attribs[ATTRIB_INDEX_POSITION].stride		= sizeof( vec2_t );
	attribs[ATTRIB_INDEX_TEXCOORD].stride		= sizeof( texCoord_t );
	attribs[ATTRIB_INDEX_COLOR].stride			= sizeof( color4ub_t );
	attribs[ATTRIB_INDEX_WORLDPOS].stride		= sizeof( worldPos_t );
	attribs[ATTRIB_INDEX_TANGENT].stride		= sizeof( vec3_t );
	attribs[ATTRIB_INDEX_BITANGENT].stride		= sizeof( vec3_t );
	attribs[ATTRIB_INDEX_NORMAL].stride			= sizeof( vec3_t );

	rg.world->buffer = R_AllocateBuffer( "worldDrawBuffer", NULL, sizeof( *rg.world->vertices ) * rg.world->numVertices, NULL,
										sizeof( glIndex_t ) * rg.world->numIndices, BUFFER_STREAM, attribs );

	VBO_Bind( rg.world->buffer );
	if ( glContext.directStateAccess ) {
		rg.world->buffer->vertex[ ATTRIB_INDEX_POSITION ].size = sizeof( vec2_t ) * rg.world->numVertices;
		rg.world->buffer->vertex[ ATTRIB_INDEX_TEXCOORD ].size = sizeof( texCoord_t ) * rg.world->numVertices;
		rg.world->buffer->vertex[ ATTRIB_INDEX_WORLDPOS ].size = sizeof( worldPos_t ) * rg.world->numVertices;
		rg.world->buffer->vertex[ ATTRIB_INDEX_COLOR ].size = sizeof( color4ub_t ) * rg.world->numVertices;

		rg.world->buffer->vertex[ ATTRIB_INDEX_POSITION ].target = GL_ARRAY_BUFFER;
		rg.world->buffer->vertex[ ATTRIB_INDEX_TEXCOORD ].target = GL_ARRAY_BUFFER;
		rg.world->buffer->vertex[ ATTRIB_INDEX_WORLDPOS ].target = GL_ARRAY_BUFFER;
		rg.world->buffer->vertex[ ATTRIB_INDEX_COLOR ].target = GL_ARRAY_BUFFER;

		rg.world->buffer->index.target = GL_ELEMENT_ARRAY_BUFFER;

		VBO_MapBuffers( &rg.world->buffer->vertex[ ATTRIB_INDEX_POSITION ], qfalse );
		VBO_MapBuffers( &rg.world->buffer->vertex[ ATTRIB_INDEX_TEXCOORD ], qtrue );
		VBO_MapBuffers( &rg.world->buffer->vertex[ ATTRIB_INDEX_WORLDPOS ], qtrue );
		VBO_MapBuffers( &rg.world->buffer->vertex[ ATTRIB_INDEX_COLOR ], qtrue );
	} else {
		rg.world->buffer->vertex[0].size = sizeof( *rg.world->vertices ) * rg.world->numVertices;
		VBO_MapBuffers( &rg.world->buffer->vertex[0], qfalse );
	}
	VBO_MapBuffers( &rg.world->buffer->index, qtrue );
	GL_CheckErrors();

	rg.world->indices = rg.world->buffer->index.data;

	// cache the indices so that we aren't calculating these every frame (there could be thousands)
	for ( i = 0, offset = 0; i < rg.world->numIndices; i += 6, offset += 4 ) {
		rg.world->indices[ i + 0 ] = offset + 0;
		rg.world->indices[ i + 1 ] = offset + 1;
		rg.world->indices[ i + 2 ] = offset + 2;

		rg.world->indices[ i + 3 ] = offset + 3;
		rg.world->indices[ i + 4 ] = offset + 2;
		rg.world->indices[ i + 5 ] = offset + 0;
	}

	ri.Printf( PRINT_INFO, "Optimizing vertex cache... (Current cache misses: %f)\n", R_CalcCacheEfficiency() );
	R_OptimizeVertexCache();
	ri.Printf( PRINT_INFO, "Optimized cache misses: %f\n", R_CalcCacheEfficiency() );

	if ( glContext.directStateAccess ) {
		rg.world->worldPos = (worldPos_t *)rg.world->buffer->vertex[ ATTRIB_INDEX_WORLDPOS ].data;
		rg.world->xyz = (vec2_t *)( rg.world->buffer->vertex[ ATTRIB_INDEX_POSITION ].data );
		rg.world->uv = (texCoord_t *)( rg.world->buffer->vertex[ ATTRIB_INDEX_TEXCOORD ].data );
		rg.world->color = (color4ub_t *)( rg.world->buffer->vertex[ ATTRIB_INDEX_COLOR ].data );
	} else {
		rg.world->vertices = rg.world->buffer->vertex[0].data;
		rg.world->worldPos = (worldPos_t *)rg.world->vertices;
		rg.world->xyz = (vec2_t *)( rg.world->worldPos + rg.world->numVertices );
		rg.world->uv = (texCoord_t *)( rg.world->xyz + rg.world->numVertices );
		rg.world->color = (color4ub_t *)( rg.world->uv + rg.world->numVertices );
	}

	R_GenerateTexCoords( &theader->info );
	R_ProcessLights();
	R_SetupTileLighting();
	
	ri.Printf( PRINT_DEVELOPER, "Flushing mapped world buffer regions...\n" );

	VBO_BindNull();

	VBO_Bind( rg.world->buffer );

	if ( glContext.directStateAccess ) {
//		nglFlushMappedNamedBufferRange( rg.world->buffer->vertex[ ATTRIB_INDEX_WORLDPOS ].id, 0, sizeof( worldPos_t ) * rg.world->numVertices );
//		nglFlushMappedNamedBufferRange( rg.world->buffer->vertex[ ATTRIB_INDEX_TEXCOORD ].id, 0, sizeof( texCoord_t ) * rg.world->numVertices );
//		nglFlushMappedNamedBufferRange( rg.world->buffer->vertex[ ATTRIB_INDEX_COLOR ].id, 0, sizeof( color4ub_t ) * rg.world->numVertices );
//		nglFlushMappedNamedBufferRange( rg.world->buffer->index.id, 0, sizeof( glIndex_t ) * rg.world->numIndices );

		nglUnmapNamedBuffer( rg.world->buffer->vertex[ ATTRIB_INDEX_WORLDPOS ].id );
		nglUnmapNamedBuffer( rg.world->buffer->vertex[ ATTRIB_INDEX_TEXCOORD ].id );
		nglUnmapNamedBuffer( rg.world->buffer->vertex[ ATTRIB_INDEX_COLOR ].id );
		nglUnmapNamedBuffer( rg.world->buffer->index.id );
	} else {
		nglFlushMappedBufferRange( GL_ARRAY_BUFFER, attribs[ ATTRIB_INDEX_WORLDPOS ].offset, sizeof( worldPos_t ) * rg.world->numVertices );
		nglFlushMappedBufferRange( GL_ARRAY_BUFFER, attribs[ ATTRIB_INDEX_TEXCOORD ].offset, sizeof( vec2_t ) * rg.world->numVertices );
		nglFlushMappedBufferRange( GL_ARRAY_BUFFER, attribs[ ATTRIB_INDEX_COLOR ].offset, sizeof( color4ub_t ) * rg.world->numVertices );
		nglFlushMappedBufferRange( GL_ELEMENT_ARRAY_BUFFER, 0, sizeof( glIndex_t ) * rg.world->numIndices );
	}
	
	VBO_BindNull();
}

void RE_LoadWorldMap( const char *filename )
{
	bmf_t *header;
	mapheader_t *mheader;
	tile2d_header_t *theader;
	spriteCoord_t *sprites;
	static world_t r_worldData;
	int i;
	char texture[MAX_NPATH];
	union {
		byte *b;
		void *v;
	} buffer;

	ri.Printf( PRINT_INFO, "------ RE_LoadWorldMap( %s ) ------\n", filename );

	if ( strlen( filename ) >= MAX_NPATH ) {
		ri.Error(ERR_DROP, "RE_LoadWorldMap: name '%s' too long", filename );
	}

	if ( rg.worldMapLoaded ) {
		ri.Error( ERR_DROP, "attempted to reduntantly load world map" );
	}

	// load it
	ri.FS_LoadFile( filename, &buffer.v );
	if ( !buffer.v ) {
		ri.Error( ERR_DROP, "RE_LoadWorldMap: %s not found", filename );
	}

	// clear rg.world so if the level fails to load, the next
	// try will not look at the partially loaded version
	rg.world = NULL;

	rg.worldMapLoaded = qtrue;

	memset( &r_worldData,0, sizeof( r_worldData ) );
	N_strncpyz( r_worldData.name, filename, sizeof( r_worldData.name ) );
	N_strncpyz( r_worldData.baseName, COM_SkipPath( r_worldData.name ), sizeof( r_worldData.baseName ) );

	COM_StripExtension( r_worldData.baseName, r_worldData.baseName, sizeof( r_worldData.baseName ) );

	header = (bmf_t *)buffer.b;
	if ( LittleInt( header->version ) != LEVEL_VERSION ) {
		ri.Error( ERR_DROP, "RE_LoadWorldMap: %s has the wrong version number (%i should be %i)",
			filename, LittleInt( header->version ), LEVEL_VERSION );
	}

	fileBase = (byte *)header;

	mheader = &header->map;
	theader = &header->tileset;

	// swap all the lumps
	for ( i = 0; i < ( sizeof( bmf_t ) / 4 ); i++ ) {
		( (int32_t *)header )[i] = LittleInt( ( (int32_t *)header )[i] );
	}

	VectorCopy( r_worldData.ambientLightColor, mheader->ambientLightColor );

	r_worldData.width = mheader->mapWidth;
	r_worldData.height = mheader->mapHeight;
	r_worldData.numTiles = r_worldData.width * r_worldData.height;

	// load into heap
	ri.G_GetMapData( &r_worldData.tiles, &r_worldData.numTiles );
	R_LoadLights( &mheader->lumps[LUMP_LIGHTS], &r_worldData );

	rg.world = &r_worldData;

	R_InitWorldBuffer( theader );

	ri.FS_FreeFile( buffer.v );
}

typedef struct {
	size_t minOutsideRoot;
	size_t maxOutsideRoot;
	
} quadtree_t;