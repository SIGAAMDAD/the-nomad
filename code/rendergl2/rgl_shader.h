#ifndef __RGL_SHADER__
#define __RGL_SHADER__

#pragma once

typedef enum {
	AGEN_IDENTITY,
	AGEN_SKIP,
	AGEN_ENTITY,
	AGEN_ONE_MINUS_ENTITY,
	AGEN_VERTEX,
	AGEN_ONE_MINUS_VERTEX,
	AGEN_LIGHTING_SPECULAR,
	AGEN_WAVEFORM,
	AGEN_PORTAL,
	AGEN_CONST,
} alphaGen_t;

typedef enum {
	CGEN_BAD,
	CGEN_IDENTITY_LIGHTING,	// tr.identityLight
	CGEN_IDENTITY,			// always (1,1,1,1)
	CGEN_ENTITY,			// grabbed from entity's modulate field
	CGEN_ONE_MINUS_ENTITY,	// grabbed from 1 - entity.modulate
	CGEN_EXACT_VERTEX,		// tess.vertexColors
	CGEN_VERTEX,			// tess.vertexColors * tr.identityLight
	CGEN_EXACT_VERTEX_LIT,	// like CGEN_EXACT_VERTEX but takes a light direction from the lightgrid
	CGEN_VERTEX_LIT,		// like CGEN_VERTEX but takes a light direction from the lightgrid
	CGEN_ONE_MINUS_VERTEX,
	CGEN_WAVEFORM,			// programmatically generated
	CGEN_LIGHTING_DIFFUSE,
	CGEN_FOG,				// standard fog
	CGEN_CONST				// fixed color
} colorGen_t;

typedef struct {
    qboolean active;
    qboolean isLightmap;

    CTexture *image;

    colorGen_t rgbGen;
    alphaGen_t alphaGen;

    uint32_t stateBits;         // GLS_xxxx mask

    byte constantColor[4];      // for CGEN_CONST and AGEN_CONST

    vec4_t normalScale;
    vec4_t specularScale;
} shaderStage_t;

class CShader
{
public:
    CShader( void ) = default;
    ~CShader() = default;

    char name[MAX_GDR_PATH];

    shaderStage_t *stages[MAX_SHADER_STAGES];

    uint32_t sortedIndex;       // this shader == rg.sortedShaders[sortedIndex]
    uint32_t index;             // this shader == rg.shaders[index]
//    shaderSort_t sort;

    uint32_t surfaceFlags;      // if explicitly defined this will have SURFPARM_* flags

    uint32_t vertexAttribs;

    qboolean polygonOffset;     // set for decals and other items that must be offset

    qboolean defaultShader;		// we want to return index 0 if the shader failed to
								// load for some reason, but R_FindShader should
								// still keep a name allocated for it, so if
								// something calls RE_RegisterShader again with
								// the same name, we don't try looking for it again

    qboolean isLightmap;

    CShader *next;
};

#endif