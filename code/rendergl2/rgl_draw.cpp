#include "rgl_local.h"

GDR_EXPORT void R_DrawElements( uint32_t numElements, uintptr_t nOffset )
{
    switch (r_drawMode->i) {
    case DRAW_GPU_BUFFERED:
        if (!glState.currentVao) {
            ri.Error( ERR_DROP, "R_DrawElements: no vao bound" );
        }
        nglDrawElements( GL_TRIANGLES, numElements, GLN_INDEX_TYPE, BUFFER_OFFSET(nOffset) );
        break;
    case DRAW_IMMEDIATE:
        nglBegin(GL_TRIANGLES);
        for (uint32_t i = nOffset; i < numElements; i++) {
            nglVertex2f( 0, 0 );
            nglTexCoord2f( 0, 0);
        }
        nglEnd();
        break;
    };
}

uint64_t r_numEntities;
uint64_t r_firstSceneEntity;

uint64_t r_numDLights;
uint64_t r_firstSceneDLight;

uint64_t r_numPolys;
uint64_t r_firstScenePoly;

uint64_t r_numPolyVerts;

uint64_t r_firstSceneDrawSurf;

GDR_EXPORT void R_InitNextFrame(void)
{
    backendData->commandList.usedBytes = 0;

    r_firstSceneDrawSurf = 0;

    r_firstSceneDLight = 0;
    r_numDLights = 0;

    r_firstSceneEntity = 0;
    r_numEntities = 0;

    r_firstScenePoly = 0;
    r_numPolys = 0;

    r_numPolyVerts = 0;
}

GDR_EXPORT void RE_AddPolyToScene(nhandle_t hShader, const polyVert_t *verts, uint32_t numVerts)
{
    uint32_t i;
    srfPoly_t *poly;
    polyVert_t *vtx;

    if (!rg.registered) {
        return;
    }

    if (r_numPolyVerts + numVerts >= r_maxPolyVerts->i) {
        ri.Printf(PRINT_DEVELOPER, "RE_AddPolyToScene: r_maxPolyVerts hit, dropping %i vertices\n", numVerts);
        return;
    }

    vtx = &backendData->polyVerts[r_numPolyVerts];
    memcpy( vtx, verts, numVerts * sizeof(polyVert_t) );

    poly = &backendData->polys[r_numPolys];
    poly->hShader = hShader;
    poly->verts = vtx;
    poly->numVerts = numVerts;

    r_numPolyVerts += numVerts;
    r_numPolys++;
}

GDR_EXPORT void RE_AddPolyListToScene(const poly_t *polys, uint32_t numPolys)
{
    uint32_t i;

    if (!rg.registered) {
        return;
    }

    if (r_numPolys + numPolys >= r_maxPolys->i) {
        ri.Printf(PRINT_DEVELOPER, "RE_AddPolyListToScene: r_maxPolys hit, dropping %i polys\n", numPolys);
        return;
    }

    for (i = 0; i < numPolys; i++) {
        RE_AddPolyToScene(polys[i].hShader, polys[i].verts, polys[i].numVerts);
    }
}

GDR_EXPORT void R_DrawWorld( void )
{
    uint32_t y, x;
    drawVert_t *vtx;
    uint32_t numVerts;

    if (!rg.world) {
        ri.Error( ERR_FATAL, "R_DrawWorld: no world map loaded" );
    }

    RB_SetBatchBuffer( rg.world->GetDrawBuffer(), rg.world->GetVertices(), sizeof(drawVert_t), rg.world->GetIndices(), sizeof(glIndex_t) );

    numVerts = 0;
    vtx = rg.world->GetVertices();

    // give all indices over, they won't need a change
    RB_CommitDrawData( NULL, 0, rg.world->GetIndices(), rg.world->GetWidth() * rg.world->GetHeight() * 6 );

    for (y = 0; y < rg.world->GetHeight(); y++) {
        for (x = 0; x < rg.world->GetWidth(); x++) {
            R_WorldToGL( vtx, { x - (rg.world->GetWidth() * 0.5f), rg.world->GetHeight() - y, 0.0f }, 0.5f, 0.5f );

            RB_CommitDrawData( vtx, 4, NULL, 0 );

            vtx += 4;
        }
    }

    RB_FlushBatchBuffer();
}

GDR_EXPORT void RE_BeginScene( const renderSceneRef_t *fd ) {
}

GDR_EXPORT void RE_EndScene( void ) {
}

GDR_EXPORT void RE_RenderScene( const renderSceneRef_t *fd )
{
    RE_BeginScene( fd );

    // draw everything
    R_RenderView();

    RB_FlushBatchBuffer();

    RE_EndScene();
}

/*
==============
RB_InstantQuad

based on Tess_InstantQuad from xreal
==============
*/
GDR_EXPORT void RB_InstantQuad2(glm::vec4 quadVerts[4], glm::vec2 texCoords[4])
{
//	GLimp_LogComment("--- RB_InstantQuad2 ---\n");

    polyVert_t verts[4];

    RB_SetBatchBuffer( backend.drawBuffer, backendData->polyVerts, sizeof(polyVert_t), backendData->indices, sizeof(glIndex_t) );

    for (uint32_t i = 0; i < 4; i++) {
        VectorCopy( verts[i].xyz, quadVerts[0] );
        VectorCopy2( verts[i].uv, texCoords[0] );
    }
    RB_CommitDrawData( verts, 4, NULL, 0 );

    RB_FlushBatchBuffer();
}

GDR_EXPORT void RB_InstantQuad(glm::vec4 quadVerts[4])
{
	glm::vec2 texCoords[4];

	VectorSet2(texCoords[0], 0.0f, 0.0f);
	VectorSet2(texCoords[1], 1.0f, 0.0f);
	VectorSet2(texCoords[2], 1.0f, 1.0f);
	VectorSet2(texCoords[3], 0.0f, 1.0f);

	GLSL_UseProgram(&rg.basicShader);
	
	rg.basicShader.SetUniformMatrix4(UNIFORM_MODELVIEWPROJECTION, glState.viewData.modelViewProjection);
	rg.basicShader.SetUniformVec4(UNIFORM_COLOR, { colorWhite[0], colorWhite[1], colorWhite[2], colorWhite[3] });

	RB_InstantQuad2(quadVerts, texCoords);
}

