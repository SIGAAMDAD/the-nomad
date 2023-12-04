#include "rgl_local.h"

// this is here to that functions in n_shared.c can link

void GDR_DECL N_Error(errorCode_t code, const char *err, ...)
{
    va_list argptr;
    char msg[MAXPRINTMSG];

    va_start(argptr, err);
    N_vsnprintf(msg, sizeof(msg), err, argptr);
    va_end(argptr);

    ri.Error(code, "%s", msg);
}

void GDR_DECL Con_Printf(const char *fmt, ...)
{
    va_list argptr;
    char msg[MAXPRINTMSG];

    va_start(argptr, fmt);
    N_vsnprintf(msg, sizeof(msg), fmt, argptr);
    va_end(argptr);

    ri.Printf(PRINT_INFO, "%s", msg);
}

void RB_MakeViewMatrix( void )
{
    float aspect;

    aspect = glConfig.vidWidth / glConfig.vidHeight;

    VectorSet( glState.viewData.camera.origin, 0.0f, 0.0f, 0.0f );

    glState.viewData.zFar = -1.0f;
    glState.viewData.zNear = 1.0f;

    ri.GLM_MakeVPM( aspect, &glState.viewData.camera.zoom, glState.viewData.camera.origin, glState.viewData.camera.viewProjectionMatrix,
        glState.viewData.camera.projectionMatrix, glState.viewData.camera.viewMatrix );

#if 0
    // setup ortho projection matrix
    glm_ortho( -aspect * zoom, aspect * zoom, -aspect, aspect, glState.viewData.zNear, glState.viewData.zFar, glState.viewData.camera.projectionMatrix );

    // create the view matrix
    glm_mat4_identity( transpose );
    glm_translate( transpose, glState.viewData.camera.origin );
    glm_mat4_inv( transpose, transpose );
    glm_mat4_copy( glState.viewData.camera.viewMatrix, transpose );

    // make the final thinga-majig
    glm_mat4_mul( glState.viewData.camera.projectionMatrix, glState.viewData.camera.viewMatrix, glState.viewData.camera.viewProjectionMatrix );
#endif
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
    for ( ; sortKey < end; sortKey += sizeof( srfPoly_t ) )
        ++count[ *sortKey ];

    index[ 0 ] = 0;

    for ( i = 1; i < 256; ++i )
      index[ i ] = index[ i - 1 ] + count[ i - 1 ];

    sortKey = ( (uint8_t *)&source[ 0 ].hShader ) + byte;
    for ( i = 0; i < size; ++i, sortKey += sizeof( srfPoly_t ) )
        dest[ index[ *sortKey ]++ ] = source[ i ];
}


/*
===============
R_RadixSort

Radix sort with 4 byte size buckets
===============
*/
static void R_RadixSort( srfPoly_t *source, uint32_t size )
{
    srfPoly_t scratch[MAX_BATCH_QUADS];
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
#endif //Q3_LITTLE_ENDIAN
}


extern uint64_t r_numPolys, r_numPolyVerts;

static int SortPoly( const void *a, const void *b ) {
    return (int)(((srfPoly_t *)a)->hShader - ((srfPoly_t *)b)->hShader);
}

void R_WorldToGL( drawVert_t *verts, vec3_t pos )
{
    vec3_t xyz[4];

    ri.GLM_TransformToGL( pos, xyz, glState.viewData.camera.viewProjectionMatrix );

    for (uint32_t i = 0; i < 4; ++i)
        VectorCopy( verts[i].xyz, xyz[i] );
}

void R_DrawPolys( void )
{
    uint64_t i;
    srfPoly_t *poly;
    nhandle_t oldShader;
    glIndex_t *idx;
    drawVert_t vtx[4];
    vec3_t pos;
    uint32_t numVerts;
    uint32_t numIndices;

    // no polygon submissions this frame
    if (!r_numPolys || !r_numPolyVerts) {
        return;
    }

    RB_SetBatchBuffer( backend.drawBuffer, backendData->polyVerts, sizeof(polyVert_t), backendData->indices, sizeof(glIndex_t) );

    // sort the polys to be more efficient with our shaders
//    R_RadixSort( backendData->polys, r_numPolys ); // segfaults, dunno why rn
    qsort( backendData->polys, backendData->numPolys, sizeof(srfPoly_t), SortPoly );

    // submit all the indices
    RB_CommitDrawData( NULL, 0, backendData->indices, backendData->numIndices );

    poly = backendData->polys;
    oldShader = poly->hShader;

    for (i = 0; i < r_numPolys; i++, poly++) {
        if (oldShader != poly->hShader) {
            // if we have a new shader, flush the current batch
            RB_FlushBatchBuffer();
            oldShader = poly->hShader;

            GL_BindTexture( 0,  R_GetShaderByHandle( oldShader )->stages[0]->image );
            GLSL_SetUniformInt( &rg.basicShader, UNIFORM_DIFFUSE_MAP, 0 );
        }

        // convert local world coordinates to opengl world coordinates
        for (uint32_t a = 0; a < 4; a++) {
            vtx[a].xyz[0] = poly->verts[a].xyz[0] - (rg.world->width * 0.5f);
            vtx[a].xyz[1] = rg.world->height - poly->verts[a].xyz[1];
            vtx[a].xyz[2] = 0.0f;

            pos[0] += vtx[a].xyz[0];
            pos[1] += vtx[a].xyz[1];
            pos[2] += vtx[a].xyz[2];
        }
        // average them out
        pos[0] /= 4;
        pos[1] /= 4;

        R_WorldToGL( vtx, pos );

        // submit to draw buffer
        RB_CommitDrawData( poly->verts, poly->numVerts, NULL, 0 );
    }

    RB_FlushBatchBuffer();
}

static void R_DrawWorld( void )
{
    uint32_t y, x;
    uint32_t i;
    vec3_t pos;
    drawVert_t *vtx;

    if ((backend.refdef.flags & RSF_NOWORLDMODEL)) {
        // nothing to draw
        return;
    }

    if (!rg.world) {
        ri.Error( ERR_FATAL, "R_DrawWorld: no world model loaded" );
    }

    // prepare the batch
    RB_SetBatchBuffer( rg.world->buffer, rg.world->vertices, sizeof(drawVert_t), rg.world->indices, sizeof(glIndex_t) );

    // submit the indices, we won't be ever changing those
    RB_CommitDrawData( NULL, 0, rg.world->indices, rg.world->numIndices );

    GL_BindTexture( 0, rg.world->shader->stages[0]->image );
    GLSL_SetUniformInt( &rg.basicShader, UNIFORM_DIFFUSE_MAP, 0 );

    vtx = rg.world->vertices;

    for (y = 0; y < rg.world->height; y++) {
        for (x = 0; x < rg.world->width; x++) {
            pos[0] = x - (rg.world->width * 0.5f);
            pos[1] = rg.world->height - y;
            pos[2] = 0.0f;

            // convert the local world coordinates to OpenGL screen coordinates
            R_WorldToGL( vtx, pos );

            for (i = 0; i < 4; i++) {
                VectorCopy2( vtx[i].uv, rg.world->tiles[y * rg.world->width + x].texcoords[i] );
            }

            // submit the processed vertices
            RB_CommitDrawData( vtx, 4, NULL, 0 );

            vtx += 4;
        }
    }

    // flush it we have anything left in there
    RB_FlushBatchBuffer();
}

void R_RenderView( const viewData_t *parms )
{
    rg.viewCount++;
    polyVert_t verts[4];

    memcpy( &glState.viewData, parms, sizeof(*parms) );

    GLSL_UseProgram( &rg.basicShader );

    // setup the correct matrices
    RB_MakeViewMatrix();

    GLSL_SetUniformMatrix4( &rg.basicShader, UNIFORM_MODELVIEWPROJECTION, glState.viewData.camera.viewProjectionMatrix );

    // draw the tilemap
    R_DrawWorld();

    // render all submitted sgame polygons
    R_DrawPolys();

    GLSL_UseProgram( NULL );
}


static void R_CalcSpriteTextureCoords(uint32_t x, uint32_t y, uint32_t spriteWidth, uint32_t spriteHeight,
    uint32_t sheetWidth, uint32_t sheetHeight, vec2_t *texCoords)
{
    const vec2_t min = { (((float)x + 1) * spriteWidth) / sheetWidth, (((float)y + 1) * spriteHeight) / sheetHeight };
    const vec2_t max = { ((float)x * spriteWidth) / sheetWidth, ((float)y * spriteHeight) / sheetHeight };

    texCoords[0][0] = min[0];
    texCoords[0][1] = max[1];

    texCoords[1][0] = min[0];
    texCoords[1][1] = min[1];

    texCoords[2][0] = max[0];
    texCoords[2][1] = min[1];
        
    texCoords[3][0] = max[0];
    texCoords[3][1] = max[1];
}

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
