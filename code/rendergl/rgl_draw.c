#include "rgl_local.h"

void R_DrawElements( uint32_t numElements, uintptr_t nOffset ) {
	backend.pc.c_drawCalls++;

	switch ( r_drawMode->i ) {
	case DRAWMODE_GPU:
	case DRAWMODE_MAPPED:
		nglDrawElements( GL_TRIANGLES, numElements, GLN_INDEX_TYPE, BUFFER_OFFSET( nOffset ) );
		break;
	case DRAWMODE_IMMEDIATE: {
		// immediate mode drawing is the least supported, if there's a bug here, I probably will not try to fix it
		// only use this if you really want a retro feel
		// im not even sure if it'll really work

		drawVert_t *vtx = backend.drawBatch.vertices;

		nglBegin( GL_TRIANGLES );
		for ( uint64_t i = 0; i < numElements; i++ ) {
			nglVertex3f( vtx[ i + nOffset ].xyz[0], vtx[ i + nOffset ].xyz[1], vtx[ i + nOffset ].xyz[2] );
			nglTexCoord2f( vtx[ i + nOffset ].uv[0], vtx[ i + nOffset ].uv[1] );
			nglColor4us( vtx[ i + nOffset ].color[0], vtx[ i + nOffset ].color[1], vtx[ i + nOffset ].color[2],
				vtx[ i + nOffset ].color[3] );
		}
		nglEnd();
		break; }
	case DRAWMODE_CLIENT: {
		nglDrawElements( GL_TRIANGLES, numElements, GLN_INDEX_TYPE, (byte *)backend.drawBatch.indices + nOffset );
		break; }
	default:
		ri.Error( ERR_FATAL, "R_DrawElements: invalid draw mode" );
	};
}

/*
================
DrawTris

Draws triangle outlines for debugging
================
*/
static void DrawTris( void ) {
	GL_BindTexture( TB_COLORMAP, rg.whiteImage );

	GL_State( GLS_POLYMODE_LINE | GLS_DEPTHMASK_TRUE );
	nglDepthRange( 0, 0 );

	{
		shaderProgram_t *sp = &rg.textureColorShader;
		vec4_t color;

		GLSL_UseProgram( sp );
		
		GLSL_SetUniformMatrix4( sp, UNIFORM_MODELVIEWPROJECTION, glState.viewData.camera.viewProjectionMatrix );
		VectorSet4( color, 1, 1, 1, 1 );
		GLSL_SetUniformVec4( sp, UNIFORM_COLOR, color );
		GLSL_SetUniformInt( sp, UNIFORM_ALPHATEST, 0 );
		GLSL_SetUniformInt( sp, UNIFORM_TEXTURE_MAP, TB_DIFFUSEMAP );

		R_DrawElements( backend.drawBatch.idxOffset, 0 );
	}

	nglDepthRange( 0, 1 );
}

static void ComputeTexMods( shaderStage_t *pStage, uint32_t bundleNum, float *outMatrix, float *outOffTurb )
{
	uint32_t tm;
	float matrix[6], currentmatrix[6];
	textureBundle_t *bundle = &pStage->bundle[bundleNum];

	matrix[0] = 1.0f; matrix[2] = 0.0f; matrix[4] = 0.0f;
	matrix[1] = 0.0f; matrix[3] = 1.0f; matrix[5] = 0.0f;

	currentmatrix[0] = 1.0f; currentmatrix[2] = 0.0f; currentmatrix[4] = 0.0f;
	currentmatrix[1] = 0.0f; currentmatrix[3] = 1.0f; currentmatrix[5] = 0.0f;

	outMatrix[0] = 1.0f; outMatrix[2] = 0.0f;
	outMatrix[1] = 0.0f; outMatrix[3] = 1.0f;

	outOffTurb[0] = 0.0f; outOffTurb[1] = 0.0f; outOffTurb[2] = 0.0f; outOffTurb[3] = 0.0f;

	for ( tm = 0; tm < bundle->numTexMods ; tm++ ) {
		switch ( bundle->texMods[tm].type ) {
		case TMOD_NONE:
			tm = TR_MAX_TEXMODS;		// break out of for loop
			break;
		case TMOD_TURBULENT:
			RB_CalcTurbulentFactors(&bundle->texMods[tm].wave, &outOffTurb[2], &outOffTurb[3]);
			break;
		case TMOD_ENTITY_TRANSLATE:
			RB_CalcScrollTexMatrix( backend.currentEntity->e.shaderTexCoord, matrix );
			break;
		case TMOD_SCROLL:
			RB_CalcScrollTexMatrix( bundle->texMods[tm].scroll,
									 matrix );
			break;
		case TMOD_SCALE:
			RB_CalcScaleTexMatrix( bundle->texMods[tm].scale,
								  matrix );
			break;
		case TMOD_STRETCH:
			RB_CalcStretchTexMatrix( &bundle->texMods[tm].wave, 
								   matrix );
			break;
		case TMOD_TRANSFORM:
			RB_CalcTransformTexMatrix( &bundle->texMods[tm],
									 matrix );
			break;
		case TMOD_ROTATE:
			RB_CalcRotateTexMatrix( bundle->texMods[tm].rotateSpeed,
									matrix );
			break;
		default:
			ri.Error( ERR_DROP, "ERROR: unknown texmod '%d' in shader '%s'", bundle->texMods[tm].type, backend.drawBatch.shader->name );
			break;
		};

		switch ( bundle->texMods[tm].type ) {	
		case TMOD_NONE:
		case TMOD_TURBULENT:
		default:
			break;
		case TMOD_ENTITY_TRANSLATE:
		case TMOD_SCROLL:
		case TMOD_SCALE:
		case TMOD_STRETCH:
		case TMOD_TRANSFORM:
		case TMOD_ROTATE:
			outMatrix[0] = matrix[0] * currentmatrix[0] + matrix[2] * currentmatrix[1];
			outMatrix[1] = matrix[1] * currentmatrix[0] + matrix[3] * currentmatrix[1];

			outMatrix[2] = matrix[0] * currentmatrix[2] + matrix[2] * currentmatrix[3];
			outMatrix[3] = matrix[1] * currentmatrix[2] + matrix[3] * currentmatrix[3];

			outOffTurb[0] = matrix[0] * currentmatrix[4] + matrix[2] * currentmatrix[5] + matrix[4];
			outOffTurb[1] = matrix[1] * currentmatrix[4] + matrix[3] * currentmatrix[5] + matrix[5];

			currentmatrix[0] = outMatrix[0];
			currentmatrix[1] = outMatrix[1];
			currentmatrix[2] = outMatrix[2];
			currentmatrix[3] = outMatrix[3];
			currentmatrix[4] = outOffTurb[0];
			currentmatrix[5] = outOffTurb[1];
			break;
		};
	}
}




static void ComputeShaderColors( const shaderStage_t *pStage, vec4_t baseColor, vec4_t vertColor, int32_t blend )
{
	qboolean isBlend = ((blend & GLS_SRCBLEND_BITS) == GLS_SRCBLEND_DST_COLOR)
		|| ((blend & GLS_SRCBLEND_BITS) == GLS_SRCBLEND_ONE_MINUS_DST_COLOR)
		|| ((blend & GLS_DSTBLEND_BITS) == GLS_DSTBLEND_SRC_COLOR)
		|| ((blend & GLS_DSTBLEND_BITS) == GLS_DSTBLEND_ONE_MINUS_SRC_COLOR);

//	qboolean is2DDraw = backend.currentEntity == &backend.entity2D;

	float overbright = ( isBlend ) ? 1.0f : (float)( 1 << rg.overbrightBits );

//	const fog_t *fog;

	baseColor[0] = 
	baseColor[1] =
	baseColor[2] =
	baseColor[3] = 1.0f;

	vertColor[0] =
	vertColor[1] =
	vertColor[2] =
	vertColor[3] = 0.0f;

	//
	// rgbGen
	//
	switch ( pStage->rgbGen ) {
	case CGEN_EXACT_VERTEX:
	case CGEN_EXACT_VERTEX_LIT:
		baseColor[0] = 
		baseColor[1] =
		baseColor[2] = 
		baseColor[3] = 0.0f;
		vertColor[0] =
		vertColor[1] =
		vertColor[2] = overbright;
		vertColor[3] = 1.0f;
		break;
	case CGEN_WAVEFORM:
		baseColor[0] =
		baseColor[1] =
		baseColor[2] = RB_CalcWaveColorSingle( &pStage->rgbWave );
		break;
	case CGEN_CONST:
		baseColor[0] = pStage->constantColor[0] /*/ 255.0f */;
		baseColor[1] = pStage->constantColor[1] /*/ 255.0f */;
		baseColor[2] = pStage->constantColor[2] /*/ 255.0f */;
		baseColor[3] = pStage->constantColor[3] /*/ 255.0f */;
		break;
	case CGEN_VERTEX:
	case CGEN_VERTEX_LIT:
		baseColor[0] =
		baseColor[1] =
		baseColor[2] =
		baseColor[3] = 0.0f;
		vertColor[0] =
		vertColor[1] =
		vertColor[2] =
		vertColor[3] = 1.0f;
		break;
	case CGEN_ONE_MINUS_VERTEX:
		baseColor[0] = 
		baseColor[1] =
		baseColor[2] = 1.0f;
		vertColor[0] =
		vertColor[1] =
		vertColor[2] = -1.0f;
		break;
/*	case CGEN_FOG:
		fog = rg.world->fogs + tess.fogNum;

		baseColor[0] = ((unsigned char *)(&fog->colorInt))[0] / 255.0f;
		baseColor[1] = ((unsigned char *)(&fog->colorInt))[1] / 255.0f;
		baseColor[2] = ((unsigned char *)(&fog->colorInt))[2] / 255.0f;
		baseColor[3] = ((unsigned char *)(&fog->colorInt))[3] / 255.0f;
		break; */
	case CGEN_ENTITY: {
		if (backend.currentEntity)
		{
			baseColor[0] = backend.currentEntity->e.shader.rgba[0] / 255.0f;
			baseColor[1] = backend.currentEntity->e.shader.rgba[1] / 255.0f;
			baseColor[2] = backend.currentEntity->e.shader.rgba[2] / 255.0f;
			baseColor[3] = backend.currentEntity->e.shader.rgba[3] / 255.0f;
		}
		break; }
	case CGEN_ONE_MINUS_ENTITY: {
		if ( backend.currentEntity ) {
			baseColor[0] = 1.0f - backend.currentEntity->e.shader.rgba[0] / 255.0f;
			baseColor[1] = 1.0f - backend.currentEntity->e.shader.rgba[1] / 255.0f;
			baseColor[2] = 1.0f - backend.currentEntity->e.shader.rgba[2] / 255.0f;
			baseColor[3] = 1.0f - backend.currentEntity->e.shader.rgba[3] / 255.0f;
		}
		break; }
	case CGEN_IDENTITY:
	case CGEN_LIGHTING_DIFFUSE:
		baseColor[0] =
		baseColor[1] =
		baseColor[2] = overbright;
		break;
	case CGEN_IDENTITY_LIGHTING:
	case CGEN_BAD:
		break;
	};

	//
	// alphaGen
	//
	switch ( pStage->alphaGen ) {
	case AGEN_SKIP:
		break;
	case AGEN_CONST:
		baseColor[3] = pStage->constantColor[3] / 255.0f;
		vertColor[3] = 0.0f;
		break;
	case AGEN_WAVEFORM:
		baseColor[3] = RB_CalcWaveAlphaSingle( &pStage->alphaWave );
		vertColor[3] = 0.0f;
		break;
	case AGEN_ENTITY: {
		if ( backend.currentEntity ) {
			baseColor[3] = backend.currentEntity->e.shader.rgba[3] / 255.0f;
		}
		vertColor[3] = 0.0f;
		break; }
	case AGEN_ONE_MINUS_ENTITY: {
		if ( backend.currentEntity ) {
			baseColor[3] = 1.0f - backend.currentEntity->e.shader.rgba[3] / 255.0f;
		}
		vertColor[3] = 0.0f;
		break; }
	case AGEN_VERTEX:
		baseColor[3] = 0.0f;
		vertColor[3] = 1.0f;
		break;
	case AGEN_ONE_MINUS_VERTEX:
		baseColor[3] = 1.0f;
		vertColor[3] = -1.0f;
		break;
	case AGEN_IDENTITY:
	case AGEN_LIGHTING_SPECULAR:
        break;
	};

	// FIXME: find some way to implement this.
#if 0
	// if in greyscale rendering mode turn all color values into greyscale.
	if(r_greyscale->integer)
	{
		int scale;
		
		for(i = 0; i < tess.numVertexes; i++)
		{
			scale = (tess.svars.colors[i][0] + tess.svars.colors[i][1] + tess.svars.colors[i][2]) / 3;
			tess.svars.colors[i][0] = tess.svars.colors[i][1] = tess.svars.colors[i][2] = scale;
		}
	}
#endif
}

static void ComputeDeformValues(int *deformGen, float deformParams[5])
{
	// u_DeformGen
	*deformGen = DGEN_NONE;
	if(!ShaderRequiresCPUDeforms(backend.drawBatch.shader))
	{
		deformStage_t  *ds;

		// only support the first one
		ds = &backend.drawBatch.shader->deforms[0];

		switch ( ds->deformation ) {
		case DEFORM_WAVE:
			*deformGen = ds->deformationWave.func;

			deformParams[0] = ds->deformationWave.base;
			deformParams[1] = ds->deformationWave.amplitude;
			deformParams[2] = ds->deformationWave.phase;
			deformParams[3] = ds->deformationWave.frequency;
			deformParams[4] = ds->deformationSpread;
			break;
		case DEFORM_BULGE:
			*deformGen = DGEN_BULGE;

			deformParams[0] = 0;
			deformParams[1] = ds->bulgeHeight; // amplitude
			deformParams[2] = ds->bulgeWidth;  // phase
			deformParams[3] = ds->bulgeSpeed;  // frequency
			deformParams[4] = 0;
			break;
		default:
			break;
		};
	}
}


/*
* RB_DrawShaderStages: RB_IterateShaderStages but for imgui textures
*/
void RB_DrawShaderStages( nhandle_t hShader, uint32_t nElems, uint32_t type, const void *offset, int32_t baseVertex )
{
	shader_t *shader;
	uint32_t i;
	shaderStage_t *stageP;
	shaderProgram_t *sp;

	sp = &rg.imguiShader;
	shader = R_GetShaderByHandle( hShader );

	if ( backend.drawBatch.shader ) {
		backend.drawBatch.shaderTime = backend.refdef.floatTime - backend.drawBatch.shader->timeOffset;
		if ( backend.drawBatch.shader->clampTime && backend.drawBatch.shaderTime >= backend.drawBatch.shader->clampTime ) {
			backend.drawBatch.shaderTime = backend.drawBatch.shader->clampTime;
		}
	}

	for ( i = 0; i < MAX_SHADER_STAGES; i++ ) {
		stageP = shader->stages[i];

		vec4_t texMatrix;
		vec4_t texOffTurb;

        if ( !stageP ) {
            break;
        }

        GL_State( stageP->stateBits );
        if ( ( stageP->stateBits & GLS_ATEST_BITS ) == GLS_ATEST_GT_0 ) {
			GLSL_SetUniformInt( sp, UNIFORM_ALPHATEST, 1 );
		}
		else if ( ( stageP->stateBits & GLS_ATEST_BITS ) == GLS_ATEST_LT_80 ) {
			GLSL_SetUniformInt( sp, UNIFORM_ALPHATEST, 2 );
		}
		else if ( ( stageP->stateBits & GLS_ATEST_BITS ) == GLS_ATEST_GE_80 ) {
			GLSL_SetUniformInt( sp, UNIFORM_ALPHATEST, 3 );
	    }
		else {
			GLSL_SetUniformInt( sp, UNIFORM_ALPHATEST, 0 );
		}
		
		if ( r_lightmap->i ) {
			vec4_t v;
			VectorSet4( v, 1.0f, 0.0f, 0.0f, 1.0f );
			GLSL_SetUniformVec4( sp, UNIFORM_DIFFUSE_TEXMATRIX, v );
			VectorSet4( v, 0.0f, 0.0f, 0.0f, 0.0f );
			GLSL_SetUniformVec4( sp, UNIFORM_DIFFUSE_TEXOFFTURB, v );

			GLSL_SetUniformInt( sp, UNIFORM_TCGEN0, TCGEN_LIGHTMAP );
		}
		else
		{
			ComputeTexMods( stageP, TB_DIFFUSEMAP, texMatrix, texOffTurb );
			GLSL_SetUniformVec4( sp, UNIFORM_DIFFUSE_TEXMATRIX, texMatrix );
			GLSL_SetUniformVec4( sp, UNIFORM_DIFFUSE_TEXOFFTURB, texOffTurb );

			GLSL_SetUniformInt( sp, UNIFORM_TCGEN0, stageP->bundle[0].tcGen );
			if ( stageP->bundle[0].tcGen == TCGEN_VECTOR ) {
				vec3_t vec;

				VectorCopy( stageP->bundle[0].tcGenVectors[0], vec );
				GLSL_SetUniformVec3( sp, UNIFORM_TCGEN0VECTOR0, vec );
				VectorCopy( stageP->bundle[0].tcGenVectors[1], vec );
				GLSL_SetUniformVec3( sp, UNIFORM_TCGEN0VECTOR1, vec );
			}
		}

        {
			vec4_t baseColor;
			vec4_t vertColor;

			ComputeShaderColors( stageP, baseColor, vertColor, stageP->stateBits );

			GLSL_SetUniformVec4( sp, UNIFORM_BASECOLOR, baseColor );
			GLSL_SetUniformVec4( sp, UNIFORM_VERTCOLOR, vertColor );
		}

        GLSL_SetUniformInt( sp, UNIFORM_COLORGEN, stageP->rgbGen );
        GLSL_SetUniformInt( sp, UNIFORM_ALPHAGEN, stageP->alphaGen );
		if ( rg.world && rg.worldMapLoaded ) {
			GLSL_SetUniformInt( sp, UNIFORM_GAMEPAUSED, ri.Cvar_VariableInteger( "g_paused" ) );
		}
		GLSL_SetUniformFloat( sp, UNIFORM_SHARPENING, r_imageSharpenAmount->f );
		GLSL_SetUniformInt( sp, UNIFORM_ANTIALIASING, r_multisampleType->i );
		
		{
			vec2_t screenSize;
			VectorSet2( screenSize, glConfig.vidWidth, glConfig.vidHeight );
			GLSL_SetUniformVec2( sp, UNIFORM_SCREEN_SIZE, screenSize );
		}

        GL_BindTexture( TB_DIFFUSEMAP, stageP->bundle[0].image );
        GLSL_SetUniformInt( sp, UNIFORM_DIFFUSE_MAP, 0 );

		// custom texture filtering
		if ( stageP->bundle[0].filter != -1 ) {
			nglBindSampler( TB_DIFFUSEMAP, rg.samplers[ stageP->bundle[0].filter ] );
		}

        //
        // draw
        //
		backend.pc.c_bufferBinds += 2;
		backend.pc.c_bufferIndices += nElems;
		backend.pc.c_bufferVertices += baseVertex;
		backend.pc.c_dynamicBufferDraws++;
		backend.pc.c_genericDraws++;
		if ( baseVertex == -1 ) {
			nglDrawElements( GL_TRIANGLES, nElems, type, offset );
		} else {
			nglDrawElementsBaseVertex( GL_TRIANGLES, nElems, type, offset, baseVertex );
		}
		backend.pc.c_drawCalls++;

		// reset to default filter
		if ( stageP->bundle[0].filter != -1 ) {
			nglBindSampler( TB_DIFFUSEMAP, 0 );
		}
	}

	if ( r_showTris->i ) {
		DrawTris();
	}
}

void RB_IterateShaderStages( shader_t *shader )
{
    uint32_t i;
	int deformGen;
	float deformParams[5];
	uint32_t numLights;

	numLights = backend.refdef.numDLights;
	if ( !( backend.refdef.flags & RSF_NOWORLDMODEL ) && rg.world ) {
		numLights += rg.world->numLights;
	}

	if ( backend.drawBatch.shader ) {
		backend.drawBatch.shaderTime = backend.refdef.floatTime - backend.drawBatch.shader->timeOffset;
		if ( backend.drawBatch.shader->clampTime && backend.drawBatch.shaderTime >= backend.drawBatch.shader->clampTime ) {
			backend.drawBatch.shaderTime = backend.drawBatch.shader->clampTime;
		}
	}

	ComputeDeformValues( &deformGen, deformParams );

    for ( i = 0; i < MAX_SHADER_STAGES; i++ ) {
        shaderStage_t *stageP = shader->stages[i];
		shaderProgram_t *sp;
		vec4_t texMatrix;
		vec4_t texOffTurb;

        if ( !stageP ) {
            break;
		}

		if ( backend.depthFill ) {
			if ( stageP->glslShaderGroup == rg.lightallShader ) {
				int index = 0;

				if ( stageP->stateBits & GLS_ATEST_BITS ) {
					index |= LIGHTDEF_USE_TCGEN_AND_TCMOD;
				}

				sp = &stageP->glslShaderGroup[ index ];
			} else {
				int shaderAttribs = 0;

				if ( backend.drawBatch.shader->numDeforms && !ShaderRequiresCPUDeforms( backend.drawBatch.shader ) ) {
					shaderAttribs |= GENERICDEF_USE_DEFORM_VERTEXES;
				}
				if ( stageP->stateBits ) {
					shaderAttribs |= GENERICDEF_USE_TCGEN_AND_TCMOD;
				}

				sp = &rg.genericShader[ shaderAttribs ];
			}
		}
		else if ( stageP->glslShaderGroup == rg.lightallShader ) {
			int index = stageP->glslShaderIndex;

			if ( r_sunlightMode->i && ( glState.viewData.flags & RSF_USE_SUNLIGHT ) && ( index & LIGHTDEF_LIGHTTYPE_MASK ) ) {
				index |= LIGHTDEF_USE_SHADOWMAP;
			}

			if ( r_lightmap->i && ( ( index & LIGHTDEF_LIGHTTYPE_MASK ) == LIGHTDEF_USE_LIGHTMAP ) ) {
				index = LIGHTDEF_USE_TCGEN_AND_TCMOD;
			}

			sp = &stageP->glslShaderGroup[ index ];
			backend.pc.c_lightallDraws++;
		}
		else {
			sp = GLSL_GetGenericShaderProgram( i );
			backend.pc.c_genericDraws++;
		}

        GLSL_UseProgram( sp );

		if ( rg.world && rg.world->numLights ) {
			GLSL_ShaderBufferData( sp, UNIFORM_LIGHTDATA, rg.lightData );
		}

		GLSL_SetUniformInt( sp, UNIFORM_NUM_LIGHTS, numLights );
        GLSL_SetUniformMatrix4( sp, UNIFORM_MODELVIEWPROJECTION, glState.viewData.camera.viewProjectionMatrix );
		GLSL_SetUniformVec3( sp, UNIFORM_LOCALVIEWORIGIN, vec3_origin );

		GLSL_SetUniformInt( sp, UNIFORM_DEFORMGEN, deformGen );

        GL_State( stageP->stateBits );
        if ( ( stageP->stateBits & GLS_ATEST_BITS ) == GLS_ATEST_GT_0 ) {
			GLSL_SetUniformInt( sp, UNIFORM_ALPHATEST, 1 );
		}
		else if ( ( stageP->stateBits & GLS_ATEST_BITS ) == GLS_ATEST_LT_80 ) {
			GLSL_SetUniformInt( sp, UNIFORM_ALPHATEST, 2 );
		}
		else if ( ( stageP->stateBits & GLS_ATEST_BITS ) == GLS_ATEST_GE_80 ) {
			GLSL_SetUniformInt( sp, UNIFORM_ALPHATEST, 3 );
	    }
		else {
			GLSL_SetUniformInt( sp, UNIFORM_ALPHATEST, 0 );
		}


		if ( r_lightmap->i ) {
			vec4_t v;
			VectorSet4( v, 1.0f, 0.0f, 0.0f, 1.0f );
			GLSL_SetUniformVec4( sp, UNIFORM_DIFFUSE_TEXMATRIX, v );
			VectorSet4( v, 0.0f, 0.0f, 0.0f, 0.0f );
			GLSL_SetUniformVec4( sp, UNIFORM_DIFFUSE_TEXOFFTURB, v );

			GLSL_SetUniformInt( sp, UNIFORM_TCGEN0, TCGEN_LIGHTMAP );
		}
		else {
			ComputeTexMods( stageP, TB_DIFFUSEMAP, texMatrix, texOffTurb );

			GLSL_SetUniformVec4( sp, UNIFORM_DIFFUSE_TEXMATRIX, texMatrix );
			GLSL_SetUniformVec4( sp, UNIFORM_DIFFUSE_TEXOFFTURB, texOffTurb );
			GLSL_SetUniformInt( sp, UNIFORM_TCGEN0, stageP->bundle[0].tcGen );

			if ( stageP->bundle[0].tcGen == TCGEN_VECTOR ) {
				vec3_t vec;
				VectorCopy( stageP->bundle[0].tcGenVectors[0], vec );
				GLSL_SetUniformVec3( sp, UNIFORM_TCGEN0VECTOR0, vec );
				VectorCopy( stageP->bundle[0].tcGenVectors[1], vec );
				GLSL_SetUniformVec3( sp, UNIFORM_TCGEN0VECTOR1, vec );
			}
		}

        {
			vec4_t baseColor;
			vec4_t vertColor;

			ComputeShaderColors( stageP, baseColor, vertColor, stageP->stateBits );

			GLSL_SetUniformVec4( sp, UNIFORM_BASECOLOR, baseColor );
			GLSL_SetUniformVec4( sp, UNIFORM_VERTCOLOR, vertColor );
		}

        GLSL_SetUniformInt( sp, UNIFORM_COLORGEN, stageP->rgbGen );
        GLSL_SetUniformInt( sp, UNIFORM_ALPHAGEN, stageP->alphaGen );
		GLSL_SetUniformVec4( sp, UNIFORM_NORMAL_SCALE, stageP->normalScale );
		GLSL_SetUniformVec4( sp, UNIFORM_SPECULAR_SCALE, stageP->specularScale );
		if ( rg.world && rg.worldMapLoaded ) {
			GLSL_SetUniformInt( sp, UNIFORM_GAMEPAUSED, ri.Cvar_VariableInteger( "g_paused" ) );
		}
		GLSL_SetUniformInt( sp, UNIFORM_HARDWAREGAMMA, !r_ignorehwgamma->i );
		GLSL_SetUniformFloat( sp, UNIFORM_GAMMA, r_gammaAmount->f );
		GLSL_SetUniformInt( sp, UNIFORM_ANTIALIASING, r_multisampleType->i );

		// custom texture filtering
		if ( stageP->bundle[0].filter != -1 ) {
			nglBindSampler( TB_DIFFUSEMAP, rg.samplers[stageP->bundle[0].filter] );
		}
		
		GLSL_SetUniformFloat( sp, UNIFORM_SHARPENING, r_imageSharpenAmount->f );

		{
			vec2_t screenSize;
			VectorSet2( screenSize, glConfig.vidWidth, glConfig.vidHeight );
			GLSL_SetUniformVec2( sp, UNIFORM_SCREEN_SIZE, screenSize );
		}

		if ( r_toneMapType->i == 1 ) {
			GLSL_SetUniformFloat( sp, UNIFORM_EXPOSURE, r_autoExposure->f );
		}

		{
			qboolean light = qtrue;
			qboolean fastLight = !( r_normalMapping->i || r_specularMapping->i );

			if ( light && !fastLight ) {
				if ( r_toneMap->i && r_toneMapType->i == 1 ) {
					GLSL_SetUniformFloat( sp, UNIFORM_EXPOSURE, r_autoExposure->f );
				}

				if ( stageP->bundle[TB_NORMALMAP].image ) {
					GL_BindTexture( 1, stageP->bundle[TB_NORMALMAP].image );
					GLSL_SetUniformInt( sp, UNIFORM_NORMAL_MAP, 1 );
				} else if ( r_normalMapping->i ) {
					GL_BindTexture( 1, rg.whiteImage );
					GLSL_SetUniformInt( sp, UNIFORM_NORMAL_MAP, 1 );
				}

				if ( stageP->bundle[TB_SPECULARMAP].image ) {
					GL_BindTexture( 2, stageP->bundle[TB_SPECULARMAP].image );
					GLSL_SetUniformInt( sp, UNIFORM_SPECULAR_MAP, 2 );
				} else if ( r_specularMapping->i ) {
					GL_BindTexture( 2, rg.whiteImage );
					GLSL_SetUniformInt( sp, UNIFORM_SPECULAR_MAP, 2 );
				}
			}
		}

		if ( !stageP->bundle[TB_DIFFUSEMAP].image ) {
			ri.Error( ERR_DROP, "RB_IterateShaderStages: shader has missing diffuseMap stage texture" );
		}
        GL_BindTexture( 0, stageP->bundle[TB_DIFFUSEMAP].image );
        GLSL_SetUniformInt( sp, UNIFORM_DIFFUSE_MAP, 0 );

		if ( r_vertexLight->i || r_dynamiclight->i ) {
			if ( rg.world && !( backend.refdef.flags & RSF_NOWORLDMODEL ) ) {
				GLSL_SetUniformVec3( sp, UNIFORM_AMBIENTLIGHT, rg.world->ambientLightColor );
			} else {
				vec3_t ambient;
				VectorSet( ambient, 0, 0, 0 );
				GLSL_SetUniformVec3( sp, UNIFORM_AMBIENTLIGHT, ambient );
			}
		}

        //
        // draw
        //
        R_DrawElements( backend.drawBatch.idxOffset, 0 );
    }

	if ( r_showTris->i ) {
		DrawTris();
	}
}

/*
==============
RB_InstantQuad

based on Tess_InstantQuad from xreal
==============
*/
void RB_InstantQuad2( vec4_t quadVerts[4], vec2_t texCoords[4] )
{
	int i;
	srfVert_t verts[4];
	void *data;

	ri.GLimp_LogComment( "--- RB_InstantQuad2 ---" );

#if 0
	nglOrtho( glState.viewData.viewportX, glState.viewData.viewportX + glState.viewData.viewportWidth,
		glState.viewData.viewportY + glState.viewData.viewportHeight, glState.viewData.viewportY,
		glState.viewData.zFar, glState.viewData.zNear );

	nglBegin( GL_TRIANGLE_FAN );
	
	nglVertex3f( quadVerts[0][0], quadVerts[0][1], quadVerts[0][2] );
	nglTexCoord2f( texCoords[0][0], texCoords[0][1] );

	nglVertex3f( quadVerts[1][0], quadVerts[1][1], quadVerts[1][2] );
	nglTexCoord2f( texCoords[1][0], texCoords[1][1] );

	nglVertex3f( quadVerts[2][0], quadVerts[2][1], quadVerts[2][2] );
	nglTexCoord2f( texCoords[2][0], texCoords[2][1] );

	nglVertex3f( quadVerts[3][0], quadVerts[3][1], quadVerts[3][2] );
	nglTexCoord2f( texCoords[3][0], texCoords[3][1] );

	nglEnd();
#else
    RB_SetBatchBuffer( backend.drawBuffer, backendData->verts, sizeof( srfVert_t ), backendData->indices, sizeof(glIndex_t) );

    for ( i = 0; i < 4; i++ ) {
        VectorCopy( verts[i].xyz, quadVerts[0] );
        VectorCopy2( verts[i].st, texCoords[0] );
    }

    backendData->indices[0] = 0;
    backendData->indices[1] = 1;
    backendData->indices[2] = 2;
    backendData->indices[3] = 3;
    backendData->indices[4] = 2;
    backendData->indices[5] = 0;

	backend.drawBatch.vtxOffset = 4;
	backend.drawBatch.idxOffset = 6;

	memcpy( backendData->verts, verts, sizeof( verts ) );

	VBO_Bind( backend.drawBuffer );

	// orphan the old index buffer so that we don't stall on it
	if ( r_drawMode->i == DRAWMODE_MAPPED ) {
		nglInvalidateBufferData( backend.drawBuffer->index.id );
		data = nglMapBufferRange( GL_ELEMENT_ARRAY_BUFFER_ARB, 0, backend.drawBatch.maxIndices, GL_MAP_WRITE_BIT
			| GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT );
		if ( data ) {
			memcpy( data, backend.drawBatch.indices, backend.drawBatch.idxOffset * backend.drawBatch.idxDataSize );
		}
		nglUnmapBuffer( GL_ELEMENT_ARRAY_BUFFER );
	} else if ( r_drawMode->i == DRAWMODE_GPU ) {
		nglBufferData( GL_ELEMENT_ARRAY_BUFFER, backend.drawBatch.maxIndices, NULL, backend.drawBatch.buffer->index.glUsage );
		nglBufferSubData( GL_ELEMENT_ARRAY_BUFFER, 0, backend.drawBatch.idxOffset * backend.drawBatch.idxDataSize, backend.drawBatch.indices );
	}

	// orphan the old vertex buffer so that we don't stall on it
	if ( r_drawMode->i == DRAWMODE_MAPPED ) {
		nglInvalidateBufferData( backend.drawBuffer->vertex.id );
		data = nglMapBufferRange( GL_ARRAY_BUFFER_ARB, 0, backend.drawBatch.maxVertices, GL_MAP_WRITE_BIT
			| GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT );
		if ( data ) {
			memcpy( data, backend.drawBatch.vertices, backend.drawBatch.vtxOffset * backend.drawBatch.vtxDataSize );
		}
		nglUnmapBuffer( GL_ARRAY_BUFFER );
	} else if ( r_drawMode->i == DRAWMODE_GPU ) {
		nglBufferData( GL_ARRAY_BUFFER, backend.drawBatch.maxVertices, NULL, backend.drawBatch.buffer->vertex.glUsage );
		nglBufferSubData( GL_ARRAY_BUFFER, 0, backend.drawBatch.vtxOffset * backend.drawBatch.vtxDataSize, backend.drawBatch.vertices );
	}

	R_DrawElements( 6, 0 );
#endif
}

void RB_InstantQuad( vec4_t quadVerts[4] )
{
	vec2_t texCoords[4];

	backend.refdef.x = 0;
	backend.refdef.y = 0;
	backend.refdef.width = glConfig.vidWidth;
	backend.refdef.height = glConfig.vidHeight;
	backend.refdef.time = backend.refdef.time;
	backend.refdef.flags = RSF_ORTHO_TYPE_SCREENSPACE | RSF_NOWORLDMODEL;
	
	glState.viewData.flags = backend.refdef.flags;
	backend.drawBatch.shader = rg.defaultShader;

	VectorSet2( texCoords[0], 0.0f, 0.0f );
	VectorSet2( texCoords[1], 1.0f, 0.0f );
	VectorSet2( texCoords[2], 1.0f, 1.0f );
	VectorSet2( texCoords[3], 0.0f, 1.0f );

	GLSL_UseProgram( &rg.textureColorShader );

    RB_MakeViewMatrix();
	GLSL_SetUniformMatrix4( &rg.textureColorShader, UNIFORM_MODELVIEWPROJECTION, glState.viewData.camera.viewProjectionMatrix );
	GLSL_SetUniformVec4( &rg.textureColorShader, UNIFORM_COLOR, colorWhite );

	RB_InstantQuad2(quadVerts, texCoords);
}
