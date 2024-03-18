#include "rgl_local.h"

/*
THIS ENTIRE FILE IS BACK END
*/

shaderDrawCommands_t drawBuf;


static void ComputeTexMods( shaderStage_t *pStage, int bundleNum, float *outMatrix, float *outOffTurb)
{
	int tm;
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
		switch ( bundle->texMods[tm].type )
		{
			
		case TMOD_NONE:
			tm = TR_MAX_TEXMODS;		// break out of for loop
			break;

		case TMOD_TURBULENT:
			RB_CalcTurbulentFactors(&bundle->texMods[tm].wave, &outOffTurb[2], &outOffTurb[3]);
			break;

	//	case TMOD_ENTITY_TRANSLATE:
	//		RB_CalcScrollTexMatrix( backEnd.currentEntity->e.shaderTexCoord, matrix );
	//		break;

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
		}

		switch ( bundle->texMods[tm].type )
		{	
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
		}
	}
}


static void ComputeShaderColors( const shaderStage_t *pStage, vec4_t baseColor, vec4_t vertColor, int blend )
{
	qboolean isBlend = ((blend & GLS_SRCBLEND_BITS) == GLS_SRCBLEND_DST_COLOR)
		|| ((blend & GLS_SRCBLEND_BITS) == GLS_SRCBLEND_ONE_MINUS_DST_COLOR)
		|| ((blend & GLS_DSTBLEND_BITS) == GLS_DSTBLEND_SRC_COLOR)
		|| ((blend & GLS_DSTBLEND_BITS) == GLS_DSTBLEND_ONE_MINUS_SRC_COLOR);

//	qboolean is2DDraw = backEnd.currentEntity == &backEnd.entity2D;

	float overbright = (float)(1 << rg.overbrightBits);

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
	switch ( pStage->rgbGen )
	{
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
//		case CGEN_FOG:
//			fog = tr.world->fogs + tess.fogNum;
//
//			baseColor[0] = ((unsigned char *)(&fog->colorInt))[0] / 255.0f;
//			baseColor[1] = ((unsigned char *)(&fog->colorInt))[1] / 255.0f;
//			baseColor[2] = ((unsigned char *)(&fog->colorInt))[2] / 255.0f;
//			baseColor[3] = ((unsigned char *)(&fog->colorInt))[3] / 255.0f;
//			break;
		case CGEN_WAVEFORM:
			baseColor[0] = 
			baseColor[1] = 
			baseColor[2] = RB_CalcWaveColorSingle( &pStage->rgbWave );
			break;
/*		case CGEN_ENTITY:
			if (backEnd.currentEntity)
			{
				baseColor[0] = backEnd.currentEntity->e.shader.rgba[0] / 255.0f;
				baseColor[1] = backEnd.currentEntity->e.shader.rgba[1] / 255.0f;
				baseColor[2] = backEnd.currentEntity->e.shader.rgba[2] / 255.0f;
				baseColor[3] = backEnd.currentEntity->e.shader.rgba[3] / 255.0f;
			}
			break;
		case CGEN_ONE_MINUS_ENTITY:
			if (backEnd.currentEntity)
			{
				baseColor[0] = 1.0f - backEnd.currentEntity->e.shader.rgba[0] / 255.0f;
				baseColor[1] = 1.0f - backEnd.currentEntity->e.shader.rgba[1] / 255.0f;
				baseColor[2] = 1.0f - backEnd.currentEntity->e.shader.rgba[2] / 255.0f;
				baseColor[3] = 1.0f - backEnd.currentEntity->e.shader.rgba[3] / 255.0f;
			}
			break; */
		case CGEN_IDENTITY:
		case CGEN_LIGHTING_DIFFUSE:
			baseColor[0] =
			baseColor[1] =
			baseColor[2] = overbright;
			break;
		case CGEN_IDENTITY_LIGHTING:
		case CGEN_BAD:
			break;
	}

	//
	// alphaGen
	//
	switch ( pStage->alphaGen )
	{
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
/*		case AGEN_ENTITY:
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
			break; */
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
		case AGEN_PORTAL:
			// Done entirely in vertex program
			baseColor[3] = 1.0f;
			vertColor[3] = 0.0f;
			break;
	}

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


void R_DrawElements(uint32_t numIndices, glIndex_t firstIndex)
{
    nglDrawElements(GL_TRIANGLES, numIndices, GLN_INDEX_TYPE, BUFFER_OFFSET(firstIndex * sizeof(glIndex_t)));
}

void RB_BeginSurface(shader_t *shader)
{
    drawBuf.numIndices = 0;
    drawBuf.numVertices = 0;
    drawBuf.firstIndex = 0;
    drawBuf.shader = shader;
}

static void RB_IterateStages( const shaderDrawCommands_t *input )
{
    uint32_t stage;
    shaderProgram_t *sp;

    sp = &rg.basicShader;

    VBO_Bind(input->buf);

    for (stage = 0; stage < MAX_SHADER_STAGES; stage++) {
        shaderStage_t *pStage = input->shader->stages[stage];

        if (!pStage) {
            break;
        }

        GLSL_UseProgram(sp);

        GLSL_SetUniformInt(sp, UNIFORM_DIFFUSE_MAP, 0);

        nglBindTexture(GL_TEXTURE_2D, stageP->image->id);


        GL_State(pStage->stateBits);
        if ((pStage->stateBits & GLS_ATEST_BITS) == GLS_ATEST_GT_0)
		{
			GLSL_SetUniformInt(sp, UNIFORM_ALPHATEST, 1);
		}
		else if ((pStage->stateBits & GLS_ATEST_BITS) == GLS_ATEST_LT_80)
		{
			GLSL_SetUniformInt(sp, UNIFORM_ALPHATEST, 2);
		}
		else if ((pStage->stateBits & GLS_ATEST_BITS) == GLS_ATEST_GE_80)
		{
			GLSL_SetUniformInt(sp, UNIFORM_ALPHATEST, 3);
		}
		else
		{
			GLSL_SetUniformInt(sp, UNIFORM_ALPHATEST, 0);
		}

        GLSL_SetUniformMatrix4(sp, UNIFORM_MODELVIEWPROJECTION, glState.modelviewProjection);
        GLSL_SetUniformMatrix4( sp, UNIFORM );

        //
        // draw
        //
        R_DrawElements(input->numIndices, input->firstIndex);
    }

    VBO_BindNull();
}

static void RB_ShaderIterateStages( void )
{
    uint32_t i;
    const shaderDrawCommands_t *input;

    input = &drawBuf;

    if (drawBuf.useInternalVao) {
        RB_UpdateCache(glState.vertexAttribsEnabled ^ ATTRIB_BITS);
    }
    else {
        backend.pc.c_staticBufferDraws++;
    }

    // set polygon offset if necessary
    if (input->shader->polygonOffset) {
        nglEnable(GL_POLYGON_OFFSET_FILL);
    }

    //
	// render depth if in depthfill mode
	//
	if (backend.depthFill)
	{
		RB_IterateStages( input );

		//
		// reset polygon offset
		//
		if ( input->shader->polygonOffset )
		{
			nglDisable( GL_POLYGON_OFFSET_FILL );
		}

		return;
	}

    //
    // call shader function
    //
    RB_IterateStages( input );
}

void RB_EndSurface( void )
{
    const shaderDrawCommands_t *input;

    input = &drawBuf;

    if (!input->numIndices || !input->numVertices)
        return;
    
    if (input->indices[MAX_BATCH_INDICES - 1] != 0) {
        ri.Error(ERR_DROP, "RB_EndSurface() - MAX_BATCH_INDICES hit");
    }
    if (input->xyz[MAX_BATCH_VERTICES - 1][0] != 0) {
        ri.Error(ERR_DROP, "RB_EndSurface() - MAX_BATCH_VERTICES hit");
    }

    backend.pc.c_bufferIndices += input->numIndices;
    backend.pc.c_bufferVertices += input->numVertices;

    RB_ShaderIterateStages();

    // clear draw buffer so we can tell we don't have any unclosed surfaces
    drawBuf.numIndices = 0;
    drawBuf.numVertices = 0;
    drawBuf.firstIndex = 0;
}
