#include "rgl_local.h"

void R_DrawElements( uint32_t numElements, uintptr_t nOffset ) {
    nglDrawElements( GL_TRIANGLES, numElements, GLN_INDEX_TYPE, BUFFER_OFFSET(nOffset) );
}

void RB_IterateShaderStages( shader_t *shader )
{
    uint32_t i;
    const shaderStage_t *stageP;

    for (i = 0; i < MAX_SHADER_STAGES; i++) {
        stageP = shader->stages[i];

        if (!stageP) {
            break;
        }

        GLSL_UseProgram( &rg.basicShader );
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
