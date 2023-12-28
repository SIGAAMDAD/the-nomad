#include "rgl_local.h"

void R_DrawElements( uint32_t numElements, uintptr_t nOffset ) {
    nglDrawElements( GL_TRIANGLES, numElements, GLN_INDEX_TYPE, BUFFER_OFFSET(nOffset) );
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
//			RB_CalcScrollTexMatrix( backEnd.currentEntity->e.shaderTexCoord, matrix );
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

//	qboolean is2DDraw = backEnd.currentEntity == &backEnd.entity2D;

//	float overbright = ( isBlend ) ? 1.0f : (float)( 1 << rg.overbrightBits );
    const float overbright = 1.0f;

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
	case CGEN_CONST:
		baseColor[0] = pStage->constantColor[0] / 255.0f;
		baseColor[1] = pStage->constantColor[1] / 255.0f;
		baseColor[2] = pStage->constantColor[2] / 255.0f;
		baseColor[3] = pStage->constantColor[3] / 255.0f;
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
	case CGEN_ENTITY:
/*		if (backEnd.currentEntity)
		{
			baseColor[0] = backend.currentEntity->e.shader.rgba[0] / 255.0f;
			baseColor[1] = backend.currentEntity->e.shader.rgba[1] / 255.0f;
			baseColor[2] = backend.currentEntity->e.shader.rgba[2] / 255.0f;
			baseColor[3] = backend.currentEntity->e.shader.rgba[3] / 255.0f;
		} */
		break;
	case CGEN_ONE_MINUS_ENTITY:
/*		if (backEnd.currentEntity)
		{
			baseColor[0] = 1.0f - backend.currentEntity->e.shader.rgba[0] / 255.0f;
			baseColor[1] = 1.0f - backend.currentEntity->e.shader.rgba[1] / 255.0f;
			baseColor[2] = 1.0f - backend.currentEntity->e.shader.rgba[2] / 255.0f;
			baseColor[3] = 1.0f - backend.currentEntity->e.shader.rgba[3] / 255.0f;
		} */
		break;
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
/*	case AGEN_ENTITY:
		if (backEnd.currentEntity)
		{
			baseColor[3] = backEnd.currentEntity->e.shader.rgba[3] / 255.0f;
		}
		vertColor[3] = 0.0f;
		break;
	case AGEN_ONE_MINUS_ENTITY:
		if (backEnd.currentEntity)
		{
			baseColor[3] = 1.0f - backEnd.currentEntity->e.shader.rgba[3] / 255.0f;
		}
		vertColor[3] = 0.0f;
		break;*/
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

	for ( i = 0; i < MAX_SHADER_STAGES; i++ ) {
		stageP = shader->stages[i];

		vec4_t texMatrix;
		vec4_t texOffTurb;

        if ( !stageP ) {
            break;
        }

        GLSL_UseProgram( sp );

        GLSL_SetUniformMatrix4( sp, UNIFORM_MODELVIEWPROJECTION, glState.viewData.camera.viewProjectionMatrix );

        GL_State( stageP->stateBits );
        if ( ( stageP->stateBits & GLS_ATEST_BITS ) == GLS_ATEST_GT_0 ) {
			GLSL_SetUniformInt( sp, UNIFORM_ALPHA_TEST, 1 );
		}
		else if ( ( stageP->stateBits & GLS_ATEST_BITS ) == GLS_ATEST_LT_80 ) {
			GLSL_SetUniformInt( sp, UNIFORM_ALPHA_TEST, 2 );
		}
		else if ( ( stageP->stateBits & GLS_ATEST_BITS ) == GLS_ATEST_GE_80 ) {
			GLSL_SetUniformInt( sp, UNIFORM_ALPHA_TEST, 3 );
	    }
		else {
			GLSL_SetUniformInt( sp, UNIFORM_ALPHA_TEST, 0 );
		}

		if ( r_lightmap->i ) {
			vec4_t v;
			VectorSet4( v, 1.0f, 0.0f, 0.0f, 1.0f );
			GLSL_SetUniformVec4( sp, UNIFORM_DIFFUSETEXMATRIX, v );
			VectorSet4( v, 0.0f, 0.0f, 0.0f, 0.0f );
			GLSL_SetUniformVec4( sp, UNIFORM_DIFFUSETEXOFFTURB, v );

			GLSL_SetUniformInt( sp, UNIFORM_TCGEN0, TCGEN_LIGHTMAP );
		}
		else {
			ComputeTexMods( stageP, TB_DIFFUSEMAP, texMatrix, texOffTurb );
			GLSL_SetUniformVec4( sp, UNIFORM_DIFFUSETEXMATRIX, texMatrix );
			GLSL_SetUniformVec4( sp, UNIFORM_DIFFUSETEXOFFTURB, texOffTurb );

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

        GL_BindTexture( TB_DIFFUSEMAP, stageP->bundle[0].image );
        GLSL_SetUniformInt( sp, UNIFORM_DIFFUSE_MAP, 0 );

		// custom texture filtering
		if ( stageP->bundle[0].filter != -1 ) {
			nglBindSampler( TB_DIFFUSEMAP, rg.samplers[stageP->bundle[0].filter] );
		}

        //
        // draw
        //
		if ( baseVertex == -1 ) {
			nglDrawElements( GL_TRIANGLES, nElems, type, offset );
		} else {
			nglDrawElementsBaseVertex( GL_TRIANGLES, nElems, type, offset, baseVertex );
		}

		// reset to default filter
		if ( stageP->bundle[0].filter != -1 ) {
			nglBindSampler( TB_DIFFUSEMAP, 0 );
		}
	}
}

void RB_IterateShaderStages( shader_t *shader )
{
    uint32_t i;
    shaderStage_t *stageP;
    shaderProgram_t *sp;

    sp = &rg.basicShader;

    for ( i = 0; i < MAX_SHADER_STAGES; i++ ) {
        stageP = shader->stages[i];

		vec4_t texMatrix;
		vec4_t texOffTurb;

        if ( !stageP ) {
            break;
        }

        GLSL_UseProgram( sp );

        GLSL_SetUniformMatrix4( sp, UNIFORM_MODELVIEWPROJECTION, glState.viewData.camera.viewProjectionMatrix );

        GL_State( stageP->stateBits );
        if ( ( stageP->stateBits & GLS_ATEST_BITS ) == GLS_ATEST_GT_0 ) {
			GLSL_SetUniformInt( sp, UNIFORM_ALPHA_TEST, 1 );
		}
		else if ( ( stageP->stateBits & GLS_ATEST_BITS ) == GLS_ATEST_LT_80 ) {
			GLSL_SetUniformInt( sp, UNIFORM_ALPHA_TEST, 2 );
		}
		else if ( ( stageP->stateBits & GLS_ATEST_BITS ) == GLS_ATEST_GE_80 ) {
			GLSL_SetUniformInt( sp, UNIFORM_ALPHA_TEST, 3 );
	    }
		else {
			GLSL_SetUniformInt( sp, UNIFORM_ALPHA_TEST, 0 );
		}

		if ( r_lightmap->i ) {
			vec4_t v;
			VectorSet4( v, 1.0f, 0.0f, 0.0f, 1.0f );
			GLSL_SetUniformVec4( sp, UNIFORM_DIFFUSETEXMATRIX, v );
			VectorSet4( v, 0.0f, 0.0f, 0.0f, 0.0f );
			GLSL_SetUniformVec4( sp, UNIFORM_DIFFUSETEXOFFTURB, v );

			GLSL_SetUniformInt( sp, UNIFORM_TCGEN0, TCGEN_LIGHTMAP );
		}
		else {
			ComputeTexMods( stageP, TB_DIFFUSEMAP, texMatrix, texOffTurb );
			GLSL_SetUniformVec4( sp, UNIFORM_DIFFUSETEXMATRIX, texMatrix );
			GLSL_SetUniformVec4( sp, UNIFORM_DIFFUSETEXOFFTURB, texOffTurb );

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

        GL_BindTexture( TB_DIFFUSEMAP, stageP->bundle[0].image );
        GLSL_SetUniformInt( sp, UNIFORM_DIFFUSE_MAP, 0 );

        //
        // draw
        //
        R_DrawElements( backend.drawBatch.idxOffset, 0 );
    }
}

/*
==============
RB_InstantQuad

based on Tess_InstantQuad from xreal
==============
*/
void RB_InstantQuad2(vec4_t quadVerts[4], vec2_t texCoords[4])
{
	GL_LogComment("--- RB_InstantQuad2 ---");

    polyVert_t verts[4];

    RB_SetBatchBuffer( backend.drawBuffer, backendData->polyVerts, sizeof(polyVert_t), backendData->indices, sizeof(glIndex_t) );

    for (uint32_t i = 0; i < 4; i++) {
        VectorCopy( verts[i].xyz, quadVerts[0] );
        VectorCopy2( verts[i].uv, texCoords[0] );
    }

    backendData->indices[0] = 0;
    backendData->indices[1] = 1;
    backendData->indices[2] = 2;
    backendData->indices[3] = 3;
    backendData->indices[4] = 2;
    backendData->indices[5] = 0;

    RB_CommitDrawData( verts, 4, backendData->indices, 6 );

    RB_FlushBatchBuffer();
}

void RB_InstantQuad(vec4_t quadVerts[4])
{
	vec2_t texCoords[4];

	VectorSet2(texCoords[0], 0.0f, 0.0f);
	VectorSet2(texCoords[1], 1.0f, 0.0f);
	VectorSet2(texCoords[2], 1.0f, 1.0f);
	VectorSet2(texCoords[3], 0.0f, 1.0f);

	GLSL_UseProgram(&rg.basicShader);
	
    RB_MakeViewMatrix();
	GLSL_SetUniformMatrix4(&rg.basicShader, UNIFORM_MODELVIEWPROJECTION, glState.viewData.camera.viewProjectionMatrix);
	GLSL_SetUniformVec4(&rg.basicShader, UNIFORM_COLOR, colorWhite );

	RB_InstantQuad2(quadVerts, texCoords);
}
