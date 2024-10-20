#include "rgl_local.h"

void R_DrawElements( uint32_t numElements, uintptr_t nOffset ) {
	backend.pc.c_drawCalls++;

	switch ( r_drawMode->i ) {
	case DRAWMODE_GPU:
	case DRAWMODE_MAPPED: {
		if ( rg.world && rg.world->drawing ) {
			nglDrawElementsInstanced( GL_TRIANGLES, numElements, GLN_INDEX_TYPE, NULL, backend.drawBatch.instanceCount );
		} else if ( backend.drawBatch.instanced && backend.drawBatch.instanceCount > 1 ) {
			nglDrawElementsInstanced( GL_TRIANGLES, numElements, GLN_INDEX_TYPE, NULL, backend.drawBatch.instanceCount );
		} else {
			nglDrawElements( GL_TRIANGLES, numElements, GLN_INDEX_TYPE, BUFFER_OFFSET( nOffset ) );
		}
		break; }
	case DRAWMODE_CLIENT: {
		nglDrawElements( GL_TRIANGLES, numElements, GLN_INDEX_TYPE, (byte *)backend.drawBatch.indices + nOffset );
		break; }
	default:
		ri.Error( ERR_FATAL, "R_DrawElements: invalid draw mode" );
	};
}

/*
=================
R_BindAnimatedImageToTMU
=================
*/
static void R_BindAnimatedImageToTMU( const textureBundle_t *bundle, int tmu ) {
	int64_t index;
	double	v;

	/*
	if ( bundle->isVideoMap ) {
		ri.CIN_RunCinematic(bundle->videoMapHandle);
		ri.CIN_UploadCinematic(bundle->videoMapHandle);
		GL_BindToTMU(tr.scratchImage[bundle->videoMapHandle], tmu);
		return;
	}
	*/

	if ( bundle->numImageAnimations <= 1 ) {
		GL_BindTexture( tmu, bundle->image[0] );
		return;
	}

	// it is necessary to do this messy calc to make sure animations line up
	// exactly with waveforms of the same frequency
	index = (int64_t)( backend.drawBatch.shaderTime * bundle->imageAnimationSpeed ) * FUNCTABLE_SIZE; // fix for frameloss bug -EC-
	index >>= FUNCTABLE_SIZE2;

	if ( index < 0 ) {
		index = 0;	// may happen with shader time offsets
	}
	
	while ( index >= bundle->numImageAnimations ) {
		index -= bundle->numImageAnimations;
	}

	GL_BindTexture( tmu, bundle->image[ index ] );
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

		nglDrawElements( GL_LINE_STRIP, backend.drawBatch.idxOffset, GLN_INDEX_TYPE, NULL );
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
		GLSL_SetUniformInt( sp, UNIFORM_GAMEPAUSED, ri.Cvar_VariableInteger( "g_paused" ) );
		GLSL_SetUniformFloat( sp, UNIFORM_SHARPENING, r_imageSharpenAmount->f );
		GLSL_SetUniformInt( sp, UNIFORM_ANTIALIASING, r_multisampleType->i );
		GLSL_SetUniformInt( sp, UNIFORM_POSTPROCESS, r_postProcess->i );
		GLSL_SetUniformFloat( sp, UNIFORM_EXPOSURE, r_autoExposure->f );
		
		{
			vec2_t screenSize;
			VectorSet2( screenSize, glConfig.vidWidth, glConfig.vidHeight );
			GLSL_SetUniformVec2( sp, UNIFORM_SCREEN_SIZE, screenSize );
		}

		R_BindAnimatedImageToTMU( &stageP->bundle[TB_DIFFUSEMAP], TB_DIFFUSEMAP );
		GLSL_SetUniformTexture( sp, UNIFORM_DIFFUSE_MAP, stageP->bundle[ TB_DIFFUSEMAP ].image[ 0 ] );

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
	uint32_t i, j;
	int deformGen;
	float deformParams[5];
	uint32_t numLights;
	qboolean horizontalBlur = qtrue;

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

		if ( rg.world && rg.world->drawing ) {
			sp = &rg.tileShader;
		}
		else {
			if ( backend.depthFill ) {
				if ( /* stageP->glslShaderGroup == rg.lightallShader */ 0 ) {
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
			else if ( /* stageP->glslShaderGroup == rg.lightallShader */ 0 ) {
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
		}

		GLSL_UseProgram( sp );

		GLSL_SetUniformInt( sp, UNIFORM_NUM_LIGHTS, numLights );
		GLSL_SetUniformMatrix4( sp, UNIFORM_MODELVIEWPROJECTION, glState.viewData.camera.viewProjectionMatrix );

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

		GLSL_SetUniformVec3( sp, UNIFORM_VIEWORIGIN, glState.viewData.camera.origin );

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
		nglUniform1i( nglGetUniformLocation( sp->programId, "u_GamePaused" ), ri.Cvar_VariableInteger( "g_paused" ) && rg.world );
		GLSL_SetUniformInt( sp, UNIFORM_ANTIALIASING, r_multisampleType->i );
		GLSL_SetUniformInt( sp, UNIFORM_HARDWAREGAMMA, !r_ignorehwgamma->i );
		GLSL_SetUniformFloat( sp, UNIFORM_GAMMA, r_gammaAmount->f );
		GLSL_SetUniformInt( sp, UNIFORM_POSTPROCESS, r_postProcess->i );
		GLSL_SetUniformInt( sp, UNIFORM_LIGHTING_QUALITY, r_lightingQuality->i );

		// custom texture filtering
		if ( stageP->bundle[0].filter != -1 ) {
			nglBindSampler( TB_DIFFUSEMAP, rg.samplers[stageP->bundle[0].filter] );
		}

		{
			vec2_t screenSize;
			VectorSet2( screenSize, glConfig.vidWidth, glConfig.vidHeight );
			GLSL_SetUniformVec2( sp, UNIFORM_SCREEN_SIZE, screenSize );
		}

		GLSL_SetUniformFloat( sp, UNIFORM_EXPOSURE, r_autoExposure->f );

		{
			qboolean light = qtrue;
			qboolean fastLight = !( r_normalMapping->i || r_specularMapping->i );

			if ( light && !fastLight ) {
				if ( stageP->bundle[TB_NORMALMAP].image[0] ) {
					R_BindAnimatedImageToTMU( &stageP->bundle[TB_NORMALMAP], TB_NORMALMAP );
					GLSL_SetUniformTexture( sp, UNIFORM_NORMAL_MAP, stageP->bundle[ TB_NORMALMAP ].image[ 0 ] );
				} else if ( r_normalMapping->i ) {
					GL_BindTexture( 1, rg.whiteImage );
					GLSL_SetUniformTexture( sp, UNIFORM_NORMAL_MAP, rg.whiteImage );
				}

				if ( stageP->bundle[TB_SPECULARMAP].image[0] ) {
					R_BindAnimatedImageToTMU( &stageP->bundle[TB_SPECULARMAP], TB_SPECULARMAP );
					GLSL_SetUniformTexture( sp, UNIFORM_SPECULAR_MAP, stageP->bundle[ TB_SPECULARMAP ].image[ 0 ] );
				} else if ( r_specularMapping->i ) {
					GL_BindTexture( 2, rg.whiteImage );
					GLSL_SetUniformTexture( sp, UNIFORM_SPECULAR_MAP, rg.whiteImage );
				}

				if ( stageP->bundle[ TB_LEVELSMAP ].image[0] ) {
					R_BindAnimatedImageToTMU( &stageP->bundle[TB_LEVELSMAP], TB_LEVELSMAP );
					GLSL_SetUniformTexture( sp, UNIFORM_LEVELS_MAP, stageP->bundle[ TB_LEVELSMAP ].image[ 0 ] );
				} else {
					GL_BindTexture( 3, rg.whiteImage );
					GLSL_SetUniformTexture( sp, UNIFORM_LEVELS_MAP, rg.whiteImage );
				}
			}
		}

		if ( !stageP->bundle[TB_DIFFUSEMAP].image[0] ) {
			ri.Error( ERR_DROP, "RB_IterateShaderStages: shader has missing diffuseMap stage texture" );
		}

		R_BindAnimatedImageToTMU( &stageP->bundle[TB_DIFFUSEMAP], TB_DIFFUSEMAP );
		GLSL_SetUniformTexture( sp, UNIFORM_DIFFUSE_MAP, stageP->bundle[ TB_DIFFUSEMAP ].image[ 0 ] );
//		GLSL_SetUniformTexture( sp, TB_DIFFUSEMAP, 0 );
		nglUniform1i( nglGetUniformLocation( sp->programId, "u_InLevel" ), rg.world != NULL );

		if ( rg.world && !( backend.refdef.flags & RSF_NOWORLDMODEL ) ) {
			RE_ProcessDLights();
		} else {
			vec3_t ambient;
			VectorSet( ambient, 1, 1, 1 );
			GLSL_SetUniformVec3( sp, UNIFORM_AMBIENTLIGHT, ambient );
		}

		//
		// draw
		//
		R_DrawElements( backend.drawBatch.idxOffset, backend.drawBuffer->index.offset );
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

	assert( false );
}

/*
void RB_RenderPass( void )
{
	VBO_Bind( rg.renderPassVBO );
	VBO_SetVertexPointers( rg.renderPassVBO, ATTRIB_POSITION | ATTRIB_TEXCOORD );
	backend.drawBatch.shader = rg.defaultShader;
	nglDrawElements( GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, NULL );
}
*/

void RB_RenderPass( void )
{
	vertexBuffer_t *oldBuffer;

	oldBuffer = glState.currentVao;
	VBO_BindNull();

	VBO_Bind( rg.renderPassVBO );
	nglDrawArrays( GL_TRIANGLE_FAN, 0, 4 );
	VBO_BindNull();

	if ( oldBuffer ) {
		VBO_Bind( oldBuffer );
	}
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

/*
==============
RB_BeginSurface

We must set some things up before beginning any tesselation,
because a surface may be forced to perform a RB_End due
to overflow.
==============
*/
#if 0
void RB_BeginSurface( shader_t *shader ) {

//	shader_t *state = ( shader->remappedShader ) ? shader->remappedShader : shader;
	shader_t *state = shader;

	tess.numIndexes = 0;
	tess.firstIndex = 0;
	tess.numVertexes = 0;
	tess.shader = state;
//	tess.dlightBits = 0;		// will be OR'd in by surface functions
//	tess.pshadowBits = 0;       // will be OR'd in by surface functions
	tess.xstages = state->stages;
//	tess.numPasses = state->numUnfoggedPasses;
//	tess.currentStageIteratorFunc = state->optimalStageIteratorFunc;
//	tess.useInternalVao = qtrue;
//	tess.useCacheVao = qfalse;

	tess.shaderTime = backend.refdef.floatTime - tess.shader->timeOffset;
	if ( tess.shader->clampTime && tess.shaderTime >= tess.shader->clampTime ) {
		tess.shaderTime = tess.shader->clampTime;
	}

	RB_SetBatchBuffer( tess.vao, NULL, 0, NULL, 0 );

//	if ( backend.viewParms.flags & VPF_SHADOWMAP) {
//		tess.currentStageIteratorFunc = RB_StageIteratorGeneric;
//	}
}

void RB_StageIteratorGeneric( void )
{
	const shaderCommands_t *input;
	unsigned int vertexAttribs = 0;

	input = &tess;

	if ( !input->numVertexes || !input->numIndexes ) {
		return;
	}

	vertexAttribs = input->shader->vertexAttribs;

	backend.drawBuffer = input->vao;
	backend.drawBatch.shader = input->shader;

	RB_UpdateTessVao( vertexAttribs );

	// set polygon offset if necessary
	if ( input->shader->polygonOffset ) {
		nglEnable( GL_POLYGON_OFFSET_FILL );
	}

	//
	// render depth if in depthfill mode
	//
	if ( backend.depthFill ) {
		RB_IterateShaderStages( input->shader );

		//
		// reset polygon offset
		//
		if ( input->shader->polygonOffset ) {
			nglDisable( GL_POLYGON_OFFSET_FILL );
		}

		return;
	}

	//
	// call shader function
	//
	RB_IterateShaderStages( input->shader );

	//
	// pshadows!
	//
	if ( glContext.ARB_framebuffer_object && r_shadows->i == 4 /* && tess.pshadowBits */
		&& tess.shader->sort <= SS_OPAQUE && !( tess.shader->surfaceFlags & SURFACEPARM_NODLIGHT
		/* ( SURF_NODLIGHT | SURFACEPARM_SKY ) */ ) )
	{
//		ProjectPshadowVBOGLSL();
	}

	//
	// reset polygon offset
	//
	if ( input->shader->polygonOffset ) {
		nglDisable( GL_POLYGON_OFFSET_FILL );
	}
}


/*
==============
RB_CheckOverflow
==============
*/
void RB_CheckOverflow( int verts, int indexes ) {
	if ( tess.numVertexes + verts < SHADER_MAX_VERTEXES
		&& tess.numIndexes + indexes < SHADER_MAX_INDEXES )
	{
		return;
	}

	RB_EndSurface();

	if ( verts >= SHADER_MAX_VERTEXES ) {
		ri.Error( ERR_DROP, "RB_CheckOverflow: verts > MAX (%d > %d)", verts, SHADER_MAX_VERTEXES );
	}

	if ( indexes >= SHADER_MAX_INDEXES ) {
		ri.Error( ERR_DROP, "RB_CheckOverflow: indices > MAX (%d > %d)", indexes, SHADER_MAX_INDEXES );
	}

	RB_BeginSurface( tess.shader );
}

void RB_EndSurface( void )
{
	const shaderCommands_t *input;

	input = &tess;

	if ( input->numIndexes == 0 || input->numVertexes == 0 ) {
		return;
	}

	if ( input->numIndexes >= SHADER_MAX_INDEXES - 1 ) {
		ri.Error( ERR_DROP, "RB_EndSurface() - SHADER_MAX_INDEXES hit" );
	}
	if ( input->numVertexes >= SHADER_MAX_VERTEXES - 1 ) {
		ri.Error( ERR_DROP, "RB_EndSurface() - SHADER_MAX_VERTEXES hit" );
	}

	if ( tess.shader == rg.shadowShader ) {
		return;
	}

	//
	// update performance counters
	//
	backend.pc.c_surfaces++;
	backend.pc.c_bufferVertices += tess.numVertexes;
	backend.pc.c_bufferIndices += tess.numIndexes;
//	backend.pc.c_totalIndexes += tess.numIndexes * tess.numPasses;

	//
	// call off to shader specific tess end function
	//
//	tess.currentStageIteratorFunc();
	RB_StageIteratorGeneric();

	//
	// draw debugging stuff
	//
	if ( r_showTris->i ) {
		DrawTris();
	}
//	if ( r_showNormals->i ) {
//		DrawNormals( input );
//	}
	// clear shader so we can tell we don't have any unclosed surfaces
	tess.numIndexes = 0;
	tess.numVertexes = 0;
	tess.firstIndex = 0;
}
#endif