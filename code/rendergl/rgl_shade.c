#include "rgl_local.h"

/*
THIS ENTIRE FILE IS BACK END
*/

shaderDrawCommands_t drawBuf;

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
        shaderStage_t *stageP = input->shader->stages[stage];

        if (!stageP) {
            break;
        }

        GLSL_UseProgram(sp);

        GL_State(stageP->stateBits);

        GLSL_SetUniformInt(sp, UNIFORM_DIFFUSE_MAP, 0);

        nglBindTexture(GL_TEXTURE_2D, stageP->image->id);

        GLSL_SetUniformMatrix4(sp, UNIFORM_MODELVIEWPROJECTION, glState.modelviewProjection);

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
