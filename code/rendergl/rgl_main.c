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

/*
RB_MakeViewMatrix:
*/
void RB_MakeViewMatrix( qboolean useOrthoUI )
{
    float aspect, zoom;

    zoom = rg.viewData.camera.zoom = 1.0f;
    aspect = rg.viewData.camera.aspect = glConfig.vidWidth / glConfig.vidHeight;

    VectorClear(rg.viewData.camera.origin);
    Mat4Ortho(-aspect * zoom, aspect * zoom, -aspect, aspect, -1.0f, 1.0f, rg.viewData.camera.projectionMatrix);
    Mat4Identity(rg.viewData.camera.modelMatrix);
    Mat4Translation(rg.viewData.camera.origin, rg.viewData.camera.modelMatrix);
    Mat4Multiply(rg.viewData.camera.projectionMatrix, rg.viewData.camera.modelMatrix, rg.viewData.camera.transformMatrix);
}

void R_InitSamplers(void)
{
    nglGenSamplers(MAX_TEXTURE_UNITS, rg.samplers);

    for (int i = 0; i < MAX_TEXTURE_UNITS; i++) {
        nglBindSampler(i, rg.samplers[i]);
        nglSamplerParameteri(rg.samplers[i], GL_TEXTURE_MIN_FILTER, gl_filter_min);
        nglSamplerParameteri(rg.samplers[i], GL_TEXTURE_MAG_FILTER, gl_filter_max);
    }
}

void R_ShutdownSamplers(void)
{
    nglDeleteSamplers(MAX_TEXTURE_UNITS, rg.samplers);
}

/*
===============
R_Radix
===============
*/
static GDR_INLINE void R_Radix( int byte, uint32_t size, const drawSurf_t *source, drawSurf_t *dest )
{
    uint32_t       count[ 256 ] = { 0 };
    uint32_t       index[ 256 ];
    uint32_t       i;
    unsigned char *sortKey;
    unsigned char *end;

    sortKey = ( (unsigned char *)&source[ 0 ].sort ) + byte;
    end = sortKey + ( size * sizeof( drawSurf_t ) );
    for ( ; sortKey < end; sortKey += sizeof( drawSurf_t ) )
        ++count[ *sortKey ];

    index[ 0 ] = 0;

    for ( i = 1; i < 256; ++i )
      index[ i ] = index[ i - 1 ] + count[ i - 1 ];

    sortKey = ( (unsigned char *)&source[ 0 ].sort ) + byte;
    for ( i = 0; i < size; ++i, sortKey += sizeof( drawSurf_t ) )
        dest[ index[ *sortKey ]++ ] = source[ i ];
}


/*
===============
R_RadixSort

Radix sort with 4 byte size buckets
===============
*/
static void R_RadixSort( drawSurf_t *source, uint32_t size )
{
    static drawSurf_t scratch[ MAX_DRAWSURFS ];
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

void R_SortDrawSurfs(drawSurf_t *drawSurfs, uint32_t numDrawSurfs)
{
    shader_t *shader;

    // sort by integers first
    R_RadixSort(drawSurfs, numDrawSurfs);

    for (uint32_t i = 0; i < numDrawSurfs; i++) {
        shader = rg.sortedShaders[drawSurfs[i].sort];

        // no shader should ever have this sort type
        if (shader->sort == SS_BAD) {
            ri.Error(ERR_DROP, "Shader '%s' with sort == SS_BAD", shader->name);
        }
    }

    R_AddDrawSurfCmd(drawSurfs, numDrawSurfs);
}

void R_AddDrawSurf(surfaceType_t *surface, shader_t *shader)
{
    uint32_t index;

    // instead of checking for overflow, we just mask the index
	// so it wraps around
	index = rg.refdef.numDrawSurfs & DRAWSURF_MASK;
	// the sort data is packed into a single 32 bit value so it can be
	// compared quickly during the qsorting process
    rg.refdef.drawSurfs[index].sort = shader->sortedIndex;
    rg.refdef.drawSurfs[index].surface = surface;

    rg.refdef.numDrawSurfs++;
}

static void R_AddWorldSurfaces(void)
{
    uint32_t y, x;
}


static surfaceType_t entitySurface = SF_POLY;
static void R_AddEntitySurface(uint32_t entityNum)
{
    renderEntityDef_t *ent;

    ent = &rg.refdef.entities[entityNum];

    R_AddDrawSurf(&entitySurface, R_GetShaderByHandle(ent->e.hShader));
}

static void R_AddEntitySurfaces(void)
{
    uint32_t i;

    for (i = 0; i < rg.refdef.numEntities; i++) {
        R_AddEntitySurface(i);
    }
}

void R_GenerateDrawSurfs(void)
{
    R_AddWorldSurfaces();
    
    R_AddEntitySurfaces();
}

void R_RenderView(const viewData_t *parms, qboolean useOrthoUI)
{
    uint64_t firstDrawSurf;
    uint64_t numDrawSurfs;

    if (!parms->viewportX || !parms->viewportY)
        return;
    
    rg.viewCount++;
    memcpy(&rg.viewData, parms, sizeof(*parms));
    rg.viewData.frameSceneNum = rg.frameSceneNum;
    rg.viewData.frameCount = rg.frameCount;

    firstDrawSurf = rg.refdef.numDrawSurfs;

    RB_MakeViewMatrix( useOrthoUI );

    R_GenerateDrawSurfs();

    // if we overflowed MAX_DRAWSURFS, the drawsurfs
	// wrapped around in the buffer and we will be missing
	// the first surfaces, not the last ones
	numDrawSurfs = rg.refdef.numDrawSurfs;
	if ( numDrawSurfs > MAX_DRAWSURFS ) {
		numDrawSurfs = MAX_DRAWSURFS;
	}

    R_SortDrawSurfs(rg.refdef.drawSurfs + firstDrawSurf, numDrawSurfs - firstDrawSurf);
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

nhandle_t RE_RegisterSpriteSheet(const char *shaderName, uint32_t numSprites, uint32_t spriteWidth, uint32_t spriteHeight,
    uint32_t sheetWidth, uint32_t sheetHeight)
{
    refSpriteSheet_t *sheet;
    shader_t *shader;
    uint64_t hash, size;

    shader = R_FindShaderByName(shaderName);
    if (!shader) {
        ri.Error(ERR_DROP, "RE_RegisterSpriteSheet: invalid shader file '%s'", shaderName);
    }

    size = 0;
    size += PAD(sizeof(*sheet), sizeof(uintptr_t));
    size += PAD(sizeof(*sheet->texCoords) * numSprites, sizeof(uintptr_t));
    sheet = rg.spriteSheets[rg.numSpriteSheets] = ri.Malloc(size);
    memset(sheet, 0, size);

    sheet->texCoords = (refSprite_t *)(sheet + 1);
    sheet->numSprites = numSprites;
    sheet->spriteHeight = spriteHeight;
    sheet->spriteWidth = spriteWidth;
    sheet->shader = shader;
    sheet->hShader = Com_GenerateHashValue(shaderName, MAX_RENDER_SHADERS);
    sheet->spriteCountX = sheetWidth / spriteWidth;
    sheet->spriteCountY = sheetHeight / spriteHeight;

    N_strncpyz(sheet->name, shaderName, sizeof(sheet->name));
    hash = Com_GenerateHashValue(shaderName, MAX_RENDER_SPRITESHEETS);

    for (uint32_t y = 0; y < sheet->spriteCountY; y++) {
        for (uint32_t x = 0; x < sheet->spriteCountX; x++) {
            R_CalcSpriteTextureCoords(x, y, spriteWidth, spriteHeight, sheetWidth, sheetHeight, sheet->texCoords[y * sheetWidth + x]);
        }
    }

    return (nhandle_t)hash;
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
