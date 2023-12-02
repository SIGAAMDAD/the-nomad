#include "rgl_local.h"

// this is here to that functions in n_shared.c can link

void GDR_ATTRIBUTE((format(printf, 2, 3))) GDR_DECL N_Error(errorCode_t code, const char *err, ...)
{
    va_list argptr;
    char msg[MAXPRINTMSG];

    va_start(argptr, err);
    N_vsnprintf(msg, sizeof(msg), err, argptr);
    va_end(argptr);

    ri.Error(code, "%s", msg);
}

void GDR_ATTRIBUTE((format(printf, 1, 2))) GDR_DECL Con_Printf(const char *fmt, ...)
{
    va_list argptr;
    char msg[MAXPRINTMSG];

    va_start(argptr, fmt);
    N_vsnprintf(msg, sizeof(msg), fmt, argptr);
    va_end(argptr);

    ri.Printf(PRINT_INFO, "%s", msg);
}

GDR_EXPORT void R_WorldToGL( drawVert_t *verts, const glm::vec3& position, float width, float height )
{
    uint32_t i;
    glm::mat4 model = glm::translate( glm::mat4( 1.0f ), position );
    glm::mat4 mvp = glState.viewData.modelViewProjection * model;
    glm::vec3 total; // will be used later for lighting

    const glm::vec4 positions[4] = {
        { width,  height, 0.0f, 1.0f },
        { width, -height, 0.0f, 1.0f },
        {-width, -height, 0.0f, 1.0f },
        {-width,  height, 0.0f, 1.0f },
    };

    for (i = 0; i < arraylen(positions); i++) {
        verts[i].xyz = mvp * positions[i];
    }
}

/*
R_MakeViewMatrix:
*/
GDR_EXPORT void R_MakeViewMatrix( void )
{
    float aspect, zoom;
    glm::mat4 transpose;

    glState.viewData.aspect = r_customWidth->i / r_customHeight->i;
    aspect = glState.viewData.aspect;
    zoom = glState.viewData.zoom;
    zoom = 1.0f;

    glState.viewData.projectionMatrix = glm::ortho( -aspect * zoom, aspect * zoom, -aspect, aspect, -1.0f, 1.0f );
    transpose = glm::translate( glm::mat4( 1.0f ), glState.viewData.origin )
                * glm::scale( glm::mat4( 1.0f ), glm::vec3( zoom ) );

    glState.viewData.viewMatrix = glm::inverse( transpose );
    glState.viewData.modelViewProjection = glState.viewData.projectionMatrix * glState.viewData.viewMatrix;    
}


/*
===============
R_Radix
===============
*/
GDR_EXPORT GDR_INLINE void R_Radix( int32_t byte, uint32_t size, const srfPoly_t *source, srfPoly_t *dest )
{
    uint32_t       count[ 256 ] = { 0 };
    uint32_t       index[ 256 ];
    uint32_t       i;
    uint8_t        *sortKey;
    uint8_t        *end;

    sortKey = ( (unsigned char *)&source[ 0 ].hShader ) + byte;
    end = sortKey + ( size * sizeof( srfPoly_t ) );
    for ( ; sortKey < end; sortKey += sizeof( srfPoly_t ) ) {
        ++count[ *sortKey ];
    }

    index[ 0 ] = 0;

    for ( i = 1; i < 256; ++i ) {
        index[ i ] = index[ i - 1 ] + count[ i - 1 ];
    }

    sortKey = ( (unsigned char *)&source[ 0 ].hShader ) + byte;
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
GDR_EXPORT void R_RadixSort( srfPoly_t *source, uint32_t size )
{
    static srfPoly_t *scratch = (srfPoly_t *)alloca( sizeof(*scratch) * r_maxPolys->i );

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

GDR_EXPORT void R_DrawPolys( void )
{
    uint64_t i;
    srfPoly_t *poly;
    nhandle_t oldShader;
    glIndex_t *idx;
    glIndex_t offset;
    uint32_t index;

    // no polygon submissions this frame
    if (!r_numPolys || !r_numPolyVerts) {
        return;
    }

    RB_SetBatchBuffer( backend.drawBuffer, backendData->polyVerts, sizeof(polyVert_t), backendData->indices, sizeof(glIndex_t) );

    // sort the polys to be more efficient with our shaders
    R_RadixSort( backendData->polys, r_numPolys );

    poly = backendData->polys;
    idx = backendData->indices;
    oldShader = poly->hShader;
    offset = 0;
    index = 0;

    for (i = 0; i < r_numPolys; i++, poly++, idx += 6) {
        if (oldShader != poly->hShader) {
            // if we have a new shader, flush the current batch
            RB_FlushBatchBuffer();
            oldShader = poly->hShader;

            // reset the index
            index = 0;
            offset = 0;
        }

        idx[index + 0] = offset + 0;
        idx[index + 1] = offset + 1;
        idx[index + 2] = offset + 2;
        
        idx[index + 3] = offset + 3;
        idx[index + 4] = offset + 2;
        idx[index + 5] = offset + 0;

        RB_CommitDrawData( poly->verts, poly->numVerts, idx, 6 );

        index += 6;
        offset += 4;
    }

    RB_FlushBatchBuffer();
}

GDR_EXPORT void R_RenderView( void )
{
    // create the basic matrix
    R_MakeViewMatrix();
    
    // bind the basic glsl program
    rg.basicShader.Bind();

    // render the tilemap
    R_DrawWorld();
    
    // draw all submitted sgame polys
    R_DrawPolys();

    rg.basicShader.Unbind();
}
