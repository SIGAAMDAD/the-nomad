#include "rgl_local.h"

// rgl_shader.cpp -- this file deals with the parsing and defintion of shaders

static char *r_shaderText;
static const char *r_extensionOffset;
static uint64_t r_extendedShader;

static CShader shader;
static shaderStage_t stages[MAX_SHADER_STAGES];

static CShader *hashTable[MAX_RENDER_SHADERS];

#define MAX_SHADERTEXT_HASH 2048
static const char **shaderTextHashTable[MAX_SHADERTEXT_HASH];

/*
static void ParseSort(const char **text)
{
	const char *tok;

	tok = COM_ParseExt(text, qfalse);
	if (tok[0] == 0) {
		ri.Printf(PRINT_WARNING, "missing sort in shader '%s'\n", shader.name);
		return;
	}

	shader.sort = SS_BAD;

	if (!N_stricmp(tok, "opaque")) {
		shader.sort = SS_OPAQUE;
	} else if (!N_stricmp(tok, "decal")) {
		shader.sort = SS_DECAL;
	} else if (!N_stricmp(tok, "blend")) {
		shader.sort = SS_BLEND;
	} else {
		ri.Printf(PRINT_WARNING, "invalid shaderSort name '%s' in shader '%s'\n", tok, shader.name);
	}
}
*/

/*
===============
NameToAFunc
===============
*/
GDR_EXPORT uint32_t NameToAFunc( const char *funcname )
{	
	if ( !N_stricmp( funcname, "GT0" ) )
	{
		return GLS_ATEST_GT_0;
	}
	else if ( !N_stricmp( funcname, "LT128" ) )
	{
		return GLS_ATEST_LT_80;
	}
	else if ( !N_stricmp( funcname, "GE128" ) )
	{
		return GLS_ATEST_GE_80;
	}

	ri.Printf( PRINT_WARNING, "invalid alphaFunc name '%s' in shader '%s'\n", funcname, shader.name );
	return 0;
}

/*
===============
NameToSrcBlendMode
===============
*/
GDR_EXPORT int NameToSrcBlendMode( const char *name )
{
	if ( !N_stricmp( name, "GL_ONE" ) ) {
		return GLS_SRCBLEND_ONE;
	}
	else if ( !N_stricmp( name, "GL_ZERO" ) ) {
		return GLS_SRCBLEND_ZERO;
	}
	else if ( !N_stricmp( name, "GL_DST_COLOR" ) ) {
		return GLS_SRCBLEND_DST_COLOR;
	}
	else if ( !N_stricmp( name, "GL_ONE_MINUS_DST_COLOR" ) ) {
		return GLS_SRCBLEND_ONE_MINUS_DST_COLOR;
	}
	else if ( !N_stricmp( name, "GL_SRC_ALPHA" ) ) {
		return GLS_SRCBLEND_SRC_ALPHA;
	}
	else if ( !N_stricmp( name, "GL_ONE_MINUS_SRC_ALPHA" ) ) {
		return GLS_SRCBLEND_ONE_MINUS_SRC_ALPHA;
	}
	else if ( !N_stricmp( name, "GL_DST_ALPHA" ) ) {
		if (r_ignoreDstAlpha->i)
			return GLS_SRCBLEND_ONE;

		return GLS_SRCBLEND_DST_ALPHA;
	}
	else if ( !N_stricmp( name, "GL_ONE_MINUS_DST_ALPHA" ) ) {
		if (r_ignoreDstAlpha->i)
			return GLS_SRCBLEND_ZERO;

		return GLS_SRCBLEND_ONE_MINUS_DST_ALPHA;
    }
	else if ( !N_stricmp( name, "GL_SRC_ALPHA_SATURATE" ) ) {
		return GLS_SRCBLEND_ALPHA_SATURATE;
	}

	ri.Printf( PRINT_WARNING, "unknown blend mode '%s' in shader '%s', substituting GL_ONE\n", name, shader.name );
	return GLS_SRCBLEND_ONE;
}

/*
===============
NameToDstBlendMode
===============
*/
GDR_EXPORT int NameToDstBlendMode( const char *name )
{
	if ( !N_stricmp( name, "GL_ONE" ) ) {
		return GLS_DSTBLEND_ONE;
	}
	else if ( !N_stricmp( name, "GL_ZERO" ) ) {
		return GLS_DSTBLEND_ZERO;
	}
	else if ( !N_stricmp( name, "GL_SRC_ALPHA" ) ) {
		return GLS_DSTBLEND_SRC_ALPHA;
	}
	else if ( !N_stricmp( name, "GL_ONE_MINUS_SRC_ALPHA" ) ) {
		return GLS_DSTBLEND_ONE_MINUS_SRC_ALPHA;
	}
	else if ( !N_stricmp( name, "GL_DST_ALPHA" ) ) {
		if (r_ignoreDstAlpha->i)
			return GLS_DSTBLEND_ONE;

		return GLS_DSTBLEND_DST_ALPHA;
	}
	else if ( !N_stricmp( name, "GL_ONE_MINUS_DST_ALPHA" ) ) {
		if (r_ignoreDstAlpha->i)
			return GLS_DSTBLEND_ZERO;

		return GLS_DSTBLEND_ONE_MINUS_DST_ALPHA;
	}
	else if ( !N_stricmp( name, "GL_SRC_COLOR" ) ) {
		return GLS_DSTBLEND_SRC_COLOR;
	}
	else if ( !N_stricmp( name, "GL_ONE_MINUS_SRC_COLOR" ) ) {
		return GLS_DSTBLEND_ONE_MINUS_SRC_COLOR;
	}

	ri.Printf( PRINT_WARNING, "unknown blend mode '%s' in shader '%s', substituting GL_ONE\n", name, shader.name );
	return GLS_DSTBLEND_ONE;
}

/*
===============
ParseVector
===============
*/
GDR_EXPORT qboolean ParseVector( const char **text, uint32_t count, float *v ) {
	const char	*token;
	uint32_t i;

	// FIXME: spaces are currently required after parens, should change parseext...
	token = COM_ParseExt( text, qfalse );
	if ( strcmp( token, "(" ) ) {
		ri.Printf( PRINT_WARNING, "missing parenthesis in shader '%s'\n", shader.name );
		return qfalse;
	}

	for ( i = 0 ; i < count ; i++ ) {
		token = COM_ParseExt( text, qfalse );
		if ( !token[0] ) {
			ri.Printf( PRINT_WARNING, "missing vector element in shader '%s'\n", shader.name );
			return qfalse;
		}
		v[i] = N_atof( token );
	}

	token = COM_ParseExt( text, qfalse );
	if ( strcmp( token, ")" ) ) {
		ri.Printf( PRINT_WARNING, "missing parenthesis in shader '%s'\n", shader.name );
		return qfalse;
	}

	return qtrue;
}

GDR_EXPORT qboolean ParseStage(shaderStage_t *stage, const char **text)
{
    const char *tok;
    uint32_t depthMaskBits = GLS_DEPTHMASK_TRUE, blendSrcBits = 0, blendDstBits = 0, atestBits = 0, depthFuncBits = 0;
	qboolean depthMaskExplicit = qfalse;

	stage->active = qtrue;

    while (1) {
        tok = COM_ParseExt(text, qtrue);
        if (!tok[0]) {
            ri.Printf(PRINT_WARNING, "no matching '}' found\n");
            return qfalse;
        }

        if (tok[0] == '}') {
            break;
        }
        //
        // map <name>
        //
        else if (!N_stricmp(tok, "map")) {
            tok = COM_ParseExt(text, qfalse);
			if (!tok[0]) {
				ri.Printf(PRINT_WARNING, "missing parameter for 'map' keyword in shader '%s'\n", shader.name);
				return qfalse;
			}

			if (!N_stricmp(tok, "$whiteimage")) {
//				stage->image = rg.whiteImage;
				continue;
			}
			else if (!N_stricmp(tok, "$lightmap")) {
				stage->isLightmap = qtrue;
//				stage->image = rg.whiteImage;
			}
			else {
				imgType_t type = IMGTYPE_COLORALPHA;
				imgFlags_t flags = IMGFLAG_NONE;
                
                stage->image = R_FindImageFile( tok, type, flags );

				if ( !stage->image ) {
					ri.Printf( PRINT_WARNING, "R_FindImageFile could not find '%s' in shader '%s'\n", tok, shader.name );
					return qfalse;
				}
			}
        }
        //
		// alphafunc <func>
		//
		else if ( !N_stricmp( tok, "alphaFunc" ) ) {
			tok = COM_ParseExt( text, qfalse );
			if ( !tok[0] ) {
				ri.Printf( PRINT_WARNING, "missing parameter for 'alphaFunc' keyword in shader '%s'\n", shader.name );
				return qfalse;
			}

			atestBits = NameToAFunc( tok );
		}
		//
		// depthFunc <func>
		//
		else if ( !N_stricmp( tok, "depthfunc" ) ) {
			tok = COM_ParseExt( text, qfalse );

			if ( !tok[0] ) {
				ri.Printf( PRINT_WARNING, "missing parameter for 'depthfunc' keyword in shader '%s'\n", shader.name );
				return qfalse;
			}

			if ( !N_stricmp( tok, "lequal" ) ) {
				depthFuncBits = 0;
			}
			else if ( !N_stricmp( tok, "equal" ) ) {
				depthFuncBits = GLS_DEPTHFUNC_EQUAL;
			}
			else {
				ri.Printf( PRINT_WARNING, "unknown depthfunc '%s' in shader '%s'\n", tok, shader.name );
				continue;
			}
		}
		//
		// blendfunc <srcFactor> <dstFactor>
		// or blendfunc <add|filter|blend>
		//
		else if ( !N_stricmp( tok, "blendfunc" ) ) {
			tok = COM_ParseExt( text, qfalse );
			if ( tok[0] == 0 ) {
				ri.Printf( PRINT_WARNING, "missing parm for blendFunc in shader '%s'\n", shader.name );
				continue;
			}
			// check for "simple" blends first
			if ( !N_stricmp( tok, "add" ) ) {
				blendSrcBits = GLS_SRCBLEND_ONE;
				blendDstBits = GLS_DSTBLEND_ONE;
			} else if ( !N_stricmp( tok, "filter" ) ) {
				blendSrcBits = GLS_SRCBLEND_DST_COLOR;
				blendDstBits = GLS_DSTBLEND_ZERO;
			} else if ( !N_stricmp( tok, "blend" ) ) {
				blendSrcBits = GLS_SRCBLEND_SRC_ALPHA;
				blendDstBits = GLS_DSTBLEND_ONE_MINUS_SRC_ALPHA;
			} else {
				// complex double blends
				blendSrcBits = NameToSrcBlendMode( tok );

				tok = COM_ParseExt( text, qfalse );
				if ( tok[0] == 0 ) {
					ri.Printf( PRINT_WARNING, "missing parm for blendFunc in shader '%s'\n", shader.name );
					continue;
				}
				blendDstBits = NameToDstBlendMode( tok );
			}

			// clear depth mask for blended surfaces
			if ( !depthMaskExplicit ) {
				depthMaskBits = 0;
			}
		}
		//
		// rgbGen
		//
		else if ( !N_stricmp( tok, "rgbGen" ) )
		{
			tok = COM_ParseExt( text, qfalse );
			if ( tok[0] == 0 )
			{
				ri.Printf( PRINT_WARNING, "missing parameters for rgbGen in shader '%s'\n", shader.name );
				continue;
			}
			if ( !N_stricmp( tok, "const" ) )
			{
				vec3_t	color;

				VectorClear( color );

				ParseVector( text, 3, color );
				stage->constantColor[0] = 255 * color[0];
				stage->constantColor[1] = 255 * color[1];
				stage->constantColor[2] = 255 * color[2];

				stage->rgbGen = CGEN_CONST;
			}
			else if ( !N_stricmp( tok, "identity" ) )
			{
				stage->rgbGen = CGEN_IDENTITY;
			}
			else if ( !N_stricmp( tok, "identityLighting" ) )
			{
				stage->rgbGen = CGEN_IDENTITY_LIGHTING;
			}
			else if ( !N_stricmp( tok, "entity" ) )
			{
				stage->rgbGen = CGEN_ENTITY;
			}
			else if ( !N_stricmp( tok, "oneMinusEntity" ) )
			{
				stage->rgbGen = CGEN_ONE_MINUS_ENTITY;
			}
			else if ( !N_stricmp( tok, "vertex" ) )
			{
				stage->rgbGen = CGEN_VERTEX;
				if ( stage->alphaGen == 0 ) {
					stage->alphaGen = AGEN_VERTEX;
				}
			}
			else if ( !N_stricmp( tok, "exactVertex" ) )
			{
				stage->rgbGen = CGEN_EXACT_VERTEX;
			}
			else if ( !N_stricmp( tok, "vertexLit" ) )
			{
				stage->rgbGen = CGEN_VERTEX_LIT;
				if ( stage->alphaGen == 0 ) {
					stage->alphaGen = AGEN_VERTEX;
				}
			}
			else if ( !N_stricmp( tok, "exactVertexLit" ) )
			{
				stage->rgbGen = CGEN_EXACT_VERTEX_LIT;
			}
			else if ( !N_stricmp( tok, "lightingDiffuse" ) )
			{
				stage->rgbGen = CGEN_LIGHTING_DIFFUSE;
			}
			else if ( !N_stricmp( tok, "oneMinusVertex" ) )
			{
				stage->rgbGen = CGEN_ONE_MINUS_VERTEX;
			}
			else
			{
				ri.Printf( PRINT_WARNING, "unknown rgbGen parameter '%s' in shader '%s'\n", tok, shader.name );
				continue;
			}
		}
        //
		// alphaGen
		//
		else if ( !N_stricmp( tok, "alphaGen" ) )
		{
			tok = COM_ParseExt( text, qfalse );
			if ( tok[0] == 0 )
			{
				ri.Printf( PRINT_WARNING, "missing parameters for alphaGen in shader '%s'\n", shader.name );
				continue;
			}
			if ( !N_stricmp( tok, "const" ) )
			{
				tok = COM_ParseExt( text, qfalse );
				stage->constantColor[3] = 255 * N_atof( tok );
				stage->alphaGen = AGEN_CONST;
			}
			else if ( !N_stricmp( tok, "identity" ) )
			{
				stage->alphaGen = AGEN_IDENTITY;
			}
			else if ( !N_stricmp( tok, "entity" ) )
			{
				stage->alphaGen = AGEN_ENTITY;
			}
			else if ( !N_stricmp( tok, "oneMinusEntity" ) )
			{
				stage->alphaGen = AGEN_ONE_MINUS_ENTITY;
			}
			else if ( !N_stricmp( tok, "vertex" ) )
			{
				stage->alphaGen = AGEN_VERTEX;
			}
			else if ( !N_stricmp( tok, "lightingSpecular" ) )
			{
				stage->alphaGen = AGEN_LIGHTING_SPECULAR;
			}
			else if ( !N_stricmp( tok, "oneMinusVertex" ) )
			{
				stage->alphaGen = AGEN_ONE_MINUS_VERTEX;
			}
			else
			{
				ri.Printf( PRINT_WARNING, "unknown alphaGen parameter '%s' in shader '%s'\n", tok, shader.name );
				continue;
			}
		}
        else {
            ri.Printf( PRINT_WARNING, "unrecognized parameter in shader '%s' stage %i: '%s'\n", shader.name, (int)(stage - stages), tok);
        }
    }

    //
	// if cgen isn't explicitly specified, use either identity or identitylighting
	//
	if ( stage->rgbGen == CGEN_BAD ) {
		if ( blendSrcBits == 0 ||
			blendSrcBits == GLS_SRCBLEND_ONE ||
			blendSrcBits == GLS_SRCBLEND_SRC_ALPHA ) {
			stage->rgbGen = CGEN_IDENTITY_LIGHTING;
		} else {
			stage->rgbGen = CGEN_IDENTITY;
		}
	}


	//
	// implicitly assume that a GL_ONE GL_ZERO blend mask disables blending
	//
	if ( ( blendSrcBits == GLS_SRCBLEND_ONE ) &&
		 ( blendDstBits == GLS_DSTBLEND_ZERO ) )
	{
		blendDstBits = blendSrcBits = 0;
		depthMaskBits = GLS_DEPTHMASK_TRUE;
	}

	// decide which agens we can skip
	if ( stage->alphaGen == AGEN_IDENTITY ) {
		if ( stage->rgbGen == CGEN_IDENTITY
			|| stage->rgbGen == CGEN_LIGHTING_DIFFUSE ) {
			stage->alphaGen = AGEN_SKIP;
		}
	}

	//
	// compute state bits
	//
	stage->stateBits = depthMaskBits |
		                blendSrcBits | blendDstBits |
		                atestBits |
		                depthFuncBits;

    return qtrue;
}

GDR_EXPORT qboolean ParseShader( const char **text )
{
    const char *tok;
	int s;
    unsigned depthMaskBits = GLS_DEPTHMASK_TRUE, blendSrcBits = 0, blendDstBits = 0, atestBits = 0, depthFuncBits = 0;
    qboolean depthMaskExplicit;
	uintptr_t p1, p2;

	// gcc doesn't like it when we do *text + r_extensionOffset
	p1 = (uintptr_t)(*text);
	p2 = (uintptr_t)(r_extensionOffset);

	s = 0;
	r_extendedShader = p1 + p2;

	tok = COM_ParseExt(text, qtrue);
	if (tok[0] != '{') {
		ri.Printf(PRINT_WARNING, "expecting '{', found '%s' instead in shader '%s'\n", tok, shader.name);
		return qfalse;
	}

    while (1) {
        tok = COM_ParseComplex(text, qtrue);

        if (!tok[0]) {
            ri.Printf(PRINT_WARNING, "no concluding '}' in shader %s\n", shader.name);
            return qfalse;
        }

		// end of shader definition
        if (tok[0] == '}') {
            break;
        }
        // stage definition
        if (tok[0] == '{') {
            if (s >= MAX_SHADER_STAGES) {
                ri.Printf(PRINT_WARNING, "too many stages in shader %s (max is %i)\n", shader.name, MAX_SHADER_STAGES);
                return qfalse;
            }
            if (!ParseStage(&stages[s], text)) {
                return qfalse;
            }
            stages[s].active = qtrue;
            s++;
            
            continue;
        }
		// disable picmipping
		else if ( !N_stricmp( tok, "nopicmip" )) {
			continue;
		}
		// disable mipmapping
		else if ( !N_stricmp( tok, "nomipmaps")) {
			continue;
		}
		/*
        //
        // shaderSort <sort>
        //
        else if ( !N_stricmp( tok, "shaderSort") )
        {
            tok = COM_ParseExt( text, qfalse );
            if ( tok[0] == 0 ) {
                ri.Printf( PRINT_WARNING, "missing parameter for shaderSort in shader '%s'\n", shader.name);
                continue;
            }
            ParseSort( text );
        } */
		// polygonOffset
		else if (!N_stricmp(tok, "polygonOffset")) {
			shader.polygonOffset = qtrue;
			continue;
		}
        else {
            ri.Printf(PRINT_WARNING, "unrecognized parameter in shader '%s': '%s'\n", shader.name, tok);
            continue;
        }
    }

    return qtrue;
}

//====================================================


GDR_EXPORT void ComputeVertexAttribs(void)
{
    uint32_t i, stage;

    // dlights always need ATTRIB_NORMAL
    shader.vertexAttribs = ATTRIB_POSITION | ATTRIB_NORMAL;

    if (shader.defaultShader) {
        shader.vertexAttribs |= ATTRIB_TEXCOORD;
    }

    
}

/*
static void SortNewShader( void )
{
    int32_t i;
    uint32_t sort;
    CShader *newShader;

    newShader = rg.shaders[rg.numShaders - 1];
    sort = newShader->sort;

    for (i = rg.numShaders - 2; i >= 0; i--) {
        if (rg.sortedShaders[i]->sort <= sort) {
            break;
        }
        rg.sortedShaders[i + 1] = rg.sortedShaders[i];
        rg.sortedShaders[i + 1]->sortedIndex++;
    }

    newShader->sortedIndex = i + 1;
    rg.sortedShaders[i + 1] = newShader;
}
*/

GDR_EXPORT CShader *GeneratePermanentShader( void )
{
    CShader *newShader;
    uint64_t size, hash;

    newShader = (CShader *)ri.Hunk_Alloc(sizeof(CShader), h_low);

    *newShader = shader;

    rg.shaders[rg.numShaders] = newShader;
    newShader->index = rg.numShaders;

//s    rg.sortedShaders[rg.numShaders] = newShader;
//s    newShader->sortedIndex = rg.numShaders;

    rg.numShaders++;

	for (uint32_t i = 0; i < MAX_SHADER_STAGES; i++) {
		if (!stages[i].active) {
			break;
		}

		newShader->stages[i] = (shaderStage_t *)ri.Hunk_Alloc(sizeof(stages[i]), h_low);
		*newShader->stages[i] = stages[i];
	}

//    SortNewShader();

    hash = Com_GenerateHashValue(newShader->name, MAX_RENDER_SHADERS);
    newShader->next = hashTable[hash];
    hashTable[hash] = newShader;
    
    return newShader;
}



/*
====================
FindShaderInShaderText

Scans the combined text description of all the shader files for
the given shader name.

return NULL if not found

If found, it will return a valid shader
=====================
*/
GDR_EXPORT const char *FindShaderInShaderText( const char *shadername )
{
	const char *tok, *p;
	uint64_t i, hash;

	hash = Com_GenerateHashValue(shadername, MAX_SHADERTEXT_HASH);

	if (shaderTextHashTable[hash]) {
		for (i = 0; shaderTextHashTable[hash][i]; i++) {
			p = shaderTextHashTable[hash][i];
			tok = COM_ParseExt(&p, qtrue);
			if (!N_stricmp(tok, shadername))
				return p;
		}
	}

	return NULL;
}

/*
InitShader
*/
GDR_EXPORT void InitShader(const char *name)
{
	memset(&shader, 0, sizeof(shader));
	N_strncpyz(shader.name, name, sizeof(shader.name));
}

GDR_EXPORT CShader *FinishShader(void)
{
	uint32_t stage;

    //
    // set polygon offset
    //
    /*
    if (shader.polygonOffset && shader.sort == SS_BAD) {
        shader.sort = SS_DECAL;
    }

    // there are times when you will need to manually apply a sort to
	// opaque alpha tested shaders that have later blend passes
	if ( shader.sort == SS_BAD ) {
		shader.sort = SS_OPAQUE;
	}
    */

	for (stage = 0; stage < MAX_SHADER_STAGES; ) {
		shaderStage_t *stageP = &stages[stage];

		if (!stageP->active) {
			break;
		}

		// check for a missing texture
		if (!stageP->image) {
			ri.Printf(PRINT_WARNING, "Shader %s has a stage with no image\n", shader.name);
			stageP->active = qfalse;
			stage++;
			continue;
		}

		//
		// default texture coordinate generation
		//
		if (stageP->isLightmap) {
		}


		//
		// [now its all NOPE] determine sort order and [nope...]fog color adjustment
		//
		if ( ( stageP->stateBits & ( GLS_SRCBLEND_BITS | GLS_DSTBLEND_BITS ) ) &&
			 ( stages[0].stateBits & ( GLS_SRCBLEND_BITS | GLS_DSTBLEND_BITS ) ) ) {
			int blendSrcBits = stageP->stateBits & GLS_SRCBLEND_BITS;
			int blendDstBits = stageP->stateBits & GLS_DSTBLEND_BITS;

			// fog color adjustment only works for blend modes that have a contribution
			// that aproaches 0 as the modulate values aproach 0 --
			// GL_ONE, GL_ONE
			// GL_ZERO, GL_ONE_MINUS_SRC_COLOR
			// GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA

#if 0
			// modulate, additive
			if ( ( ( blendSrcBits == GLS_SRCBLEND_ONE ) && ( blendDstBits == GLS_DSTBLEND_ONE ) ) ||
				( ( blendSrcBits == GLS_SRCBLEND_ZERO ) && ( blendDstBits == GLS_DSTBLEND_ONE_MINUS_SRC_COLOR ) ) ) {
				pStage->adjustColorsForFog = ACFF_MODULATE_RGB;
			}
			// strict blend
			else if ( ( blendSrcBits == GLS_SRCBLEND_SRC_ALPHA ) && ( blendDstBits == GLS_DSTBLEND_ONE_MINUS_SRC_ALPHA ) )
			{
				pStage->adjustColorsForFog = ACFF_MODULATE_ALPHA;
			}
			// premultiplied alpha
			else if ( ( blendSrcBits == GLS_SRCBLEND_ONE ) && ( blendDstBits == GLS_DSTBLEND_ONE_MINUS_SRC_ALPHA ) )
			{
				pStage->adjustColorsForFog = ACFF_MODULATE_RGBA;
			} else {
				// we can't adjust this one correctly, so it won't be exactly correct in fog
			}
#endif
			// don't screw with sort order if this is a portal or environment
			/* if ( shader.sort == SS_BAD ) {
				// see through item, like a grill or grate
				if ( stageP->stateBits & GLS_DEPTHMASK_TRUE ) {
					shader.sort = SS_SEE_THROUGH;
				} else {
					shader.sort = SS_BLEND;
				}
			} */
		}

		// don't screw with sort order if this is environment
	/*	if (shader.sort == SS_BAD) {
			// see through item, like a grill or grate
			if (stageP->stateBits & GLS_DEPTHMASK_TRUE) {
				shader.sort = SS_SEE_THROUGH;
			} else {
				shader.sort = SS_BLEND;
			}
		} */

		stage++;
	}

	// there are times when you will need to manually apply a sort to
	// opaque alpha tested shaders that have later blend passes
/*	if ( shader.sort == SS_BAD ) {
		shader.sort = SS_OPAQUE;
	} */

    return GeneratePermanentShader();
}

/*
==================
R_FindShaderByName

Will always return a valid shader, but it might be the
default shader if the real one can't be found.
==================
*/
GDR_EXPORT CShader *R_FindShaderByName( const char *name )
{
	char		strippedName[MAX_GDR_PATH];
	uint64_t	hash;
	CShader	*sh;

	if ( (name == NULL) || (name[0] == 0) ) {
		return rg.defaultShader;
	}

	COM_StripExtension(name, strippedName, sizeof(strippedName));

	hash = Com_GenerateHashValue(strippedName, MAX_RENDER_SHADERS);

	//
	// see if the shader is already loaded
	//
	for (sh = hashTable[hash]; sh; sh = sh->next) {
		// NOTE: if there was no shader or image available with the name strippedName
		// then a default shader is created with lightmapIndex == LIGHTMAP_NONE, so we
		// have to check all default shaders otherwise for every call to R_FindShader
		// with that same strippedName a new default shader is created.
		if (N_stricmp(sh->name, strippedName) == 0) {
			// match found
			return sh;
		}
	}

	return rg.defaultShader;
}

/*
===============
R_FindShader

Will always return a valid shader, but it might be the
default shader if the real one can't be found.

In the interest of not requiring an explicit shader text entry to
be defined for every single image used in the game, three default
shader behaviors can be auto-created for any image:

If lightmapIndex == LIGHTMAP_NONE, then the image will have
dynamic diffuse lighting applied to it, as appropriate for most
entity skin surfaces.

If lightmapIndex == LIGHTMAP_2D, then the image will be used
for 2D rendering unless an explicit shader is found

If lightmapIndex == LIGHTMAP_BY_VERTEX, then the image will use
the vertex rgba modulate values, as appropriate for misc_model
pre-lit surfaces.

Other lightmapIndex values will have a lightmap stage created
and src*dest blending applied with the texture, as appropriate for
most world construction surfaces.

===============
*/
GDR_EXPORT CShader *R_FindShader( const char *name ) {
	char		strippedName[MAX_GDR_PATH];
	uint64_t	hash;
	const char	*shaderText;
	CTexture	*image;
	CShader	*sh;

	if ( name[0] == '\0' ) {
		return rg.defaultShader;
	}

	COM_StripExtension(name, strippedName, sizeof(strippedName));

	hash = Com_GenerateHashValue(strippedName, MAX_RENDER_SHADERS);

	//
	// see if the shader is already loaded
	//
	for (sh = hashTable[hash]; sh; sh = sh->next) {
		// NOTE: if there was no shader or image available with the name strippedName
		// then a default shader is created with lightmapIndex == LIGHTMAP_NONE, so we
		// have to check all default shaders otherwise for every call to R_FindShader
		// with that same strippedName a new default shader is created.
		if (!N_stricmp(sh->name, strippedName)) {
			// match found
			return sh;
		}
	}

	InitShader( strippedName );

	//
	// attempt to define shader from an explicit parameter file
	//
	shaderText = FindShaderInShaderText( strippedName );
	if ( shaderText ) {
		// enable this when building a pak file to get a global list
		// of all explicit shaders
		if ( r_printShaders->i ) {
			ri.Printf( PRINT_INFO, "*SHADER* %s\n", name );
		}

		if ( !ParseShader( &shaderText ) ) {
			// had errors, so use default shader
			shader.defaultShader = qtrue;
		}
		sh = FinishShader();
		return sh;
	}

	//
	// if not defined in the in-memory shader descriptions,
	// look for a single supported image file
	//
	{
		imgFlags_t flags;

		flags = IMGFLAG_NONE;
		flags |= IMGFLAG_CLAMPTOEDGE;

		image = R_FindImageFile( name, IMGTYPE_COLORALPHA, flags );
		if ( !image ) {
			ri.Printf( PRINT_DEVELOPER, "Couldn't find image file for shader %s\n", name );
			shader.defaultShader = qtrue;
			return sh;
		}
	}

	//
	// create the default shading commands
	//
	stages[0].image = image;
	stages[0].rgbGen = CGEN_VERTEX;
	stages[0].alphaGen = AGEN_VERTEX;
	stages[0].stateBits = GLS_DEPTHTEST_DISABLE |
		                GLS_SRCBLEND_SRC_ALPHA |
		                GLS_DSTBLEND_ONE_MINUS_SRC_ALPHA;

	return FinishShader();
}

GDR_EXPORT nhandle_t RE_RegisterShaderFromTexture(const char *name, CTexture *image)
{
    uint64_t hash;
    CShader *sh;

    hash = Com_GenerateHashValue(name, MAX_RENDER_SHADERS);

	//
	// see if the shader is already loaded
	//(sh->defaultShader) ||
	for (sh=hashTable[hash]; sh; sh=sh->next) {
		// NOTE: if there was no shader or image available with the name strippedName
		// then a default shader is created with lightmapIndex == LIGHTMAP_NONE, so we
		// have to check all default shaders otherwise for every call to R_FindShader
		// with that same strippedName a new default shader is created.
		if (!N_stricmp(sh->name, name)) {
			// match found
			return sh->index;
		}
	}

    stages[0].image = image;
	stages[0].rgbGen = CGEN_VERTEX;
	stages[0].alphaGen = AGEN_VERTEX;
	stages[0].stateBits = GLS_DEPTHTEST_DISABLE |
		                GLS_SRCBLEND_SRC_ALPHA |
		                GLS_DSTBLEND_ONE_MINUS_SRC_ALPHA;

	InitShader( name );

	sh = FinishShader();
	return sh->index; 
}

/* 
====================
RE_RegisterShaderLightMap

This is the exported shader entry point for the rest of the system
It will always return an index that will be valid.

This should really only be used for explicit shaders, because there is no
way to ask for different implicit lighting modes (vertex, lightmap, etc)
====================
*/
GDR_EXPORT nhandle_t RE_RegisterShaderLightMap( const char *name ) {
	CShader	*sh;

	if ( strlen( name ) >= MAX_GDR_PATH ) {
		ri.Printf( PRINT_INFO, "Shader name exceeds MAX_GDR_PATH\n" );
		return 0;
	}

	sh = R_FindShader( name );

	// we want to return 0 if the shader failed to
	// load for some reason, but R_FindShader should
	// still keep a name allocated for it, so if
	// something calls RE_RegisterShader again with
	// the same name, we don't try looking for it again
	if ( sh->defaultShader ) {
		return 0;
	}

	return sh->index;
}

/* 
====================
RE_RegisterShader

This is the exported shader entry point for the rest of the system
It will always return an index that will be valid.

This should really only be used for explicit shaders, because there is no
way to ask for different implicit lighting modes (vertex, lightmap, etc)
====================
*/
GDR_EXPORT nhandle_t RE_RegisterShader( const char *name ) {
	CShader	*sh;

	if ( !name ) {
		ri.Printf( PRINT_INFO, "NULL shader\n" );
		return 0;
	}

	if ( strlen( name ) >= MAX_GDR_PATH ) {
		ri.Printf( PRINT_INFO, "Shader name exceeds MAX_GDR_PATH\n" );
		return 0;
	}

	sh = R_FindShader( name );

    if (!sh) {
        ri.Printf(PRINT_INFO, "RE_RegisterShader: failed to register shader '%s'\n", name);
        return FS_INVALID_HANDLE;
    }

	// we want to return 0 if the shader failed to
	// load for some reason, but R_FindShader should
	// still keep a name allocated for it, so if
	// something calls RE_RegisterShader again with
	// the same name, we don't try looking for it again
	if ( sh->defaultShader ) {
		return 0;
	}

	return sh->index;
}

/*
====================
R_GetShaderByHandle

When a handle is passed in by another module, this range checks
it and returns a valid (possibly default) CShader to be used internally.
====================
*/
GDR_EXPORT CShader *R_GetShaderByHandle( nhandle_t hShader ) {
	if ( hShader < 0 ) {
	    ri.Printf( PRINT_WARNING, "R_GetShaderByHandle: out of range hShader '%d'\n", hShader );
		return rg.defaultShader;
	}
	if ( hShader >= rg.numShaders ) {
		ri.Printf( PRINT_WARNING, "R_GetShaderByHandle: out of range hShader '%d'\n", hShader );
		return rg.defaultShader;
	}
	return rg.shaders[hShader];
}


#define	MAX_SHADER_FILES 16384

GDR_EXPORT int loadShaderBuffers( char **shaderFiles, const uint64_t numShaderFiles, char **buffers )
{
	char filename[MAX_GDR_PATH+8];
	char shaderName[MAX_GDR_PATH];
	const char *p, *tok;
	uint64_t summand, sum = 0;
	uint64_t shaderLine;
	uint64_t i;
	const char *shaderStart;
	qboolean denyErrors;

	// load and parse shader files
	for ( i = 0; i < numShaderFiles; i++ ) {
		Com_snprintf( filename, sizeof( filename ), "scripts/%s", shaderFiles[i] );
		//ri.Printf( PRINT_DEVELOPER, "...loading '%s'\n", filename );
		summand = ri.FS_LoadFile( filename, (void **)&buffers[i] );

		if ( !buffers[i] || !summand )
			ri.Error( ERR_DROP, "Couldn't load %s", filename );

		p = buffers[i];
		COM_BeginParseSession( filename );
		
		shaderStart = NULL;
		denyErrors = qfalse;

		while ( 1 ) {
			tok = COM_ParseExt( &p, qtrue );
			
			if ( !*tok )
				break;

			N_strncpyz( shaderName, tok, sizeof( shaderName ) );
			shaderLine = COM_GetCurrentParseLine();

			tok = COM_ParseExt( &p, qtrue );
			if ( tok[0] != '{' || tok[1] != '\0' ) {
				ri.Printf( PRINT_DEVELOPER, "File %s: shader \"%s\" " \
					"on line %lu missing opening brace", filename, shaderName, shaderLine );
				if ( tok[0] )
					ri.Printf( PRINT_DEVELOPER, " (found \"%s\" on line %lu)\n", tok, COM_GetCurrentParseLine() );
				else
					ri.Printf( PRINT_DEVELOPER, "\n" );

				if ( denyErrors || !p )
				{
					ri.Printf( PRINT_INFO, COLOR_YELLOW "Ignoring entire file '%s' due to error.\n", filename );
					ri.FS_FreeFile( buffers[i] );
					buffers[i] = NULL;
					break;
				}

				SkipRestOfLine( &p );
				shaderStart = p;
				continue;
			}

			if ( !SkipBracedSection( &p, 1 ) ) {
				ri.Printf(PRINT_WARNING, "Ignoring shader file %s. Shader \"%s\" " \
					"on line %lu missing closing brace.\n", filename, shaderName, shaderLine );
				ri.FS_FreeFile( buffers[i] );
				buffers[i] = NULL;
				break;
			}

			denyErrors = qtrue;
		}

		if ( buffers[ i ] ) {
			if ( shaderStart ) {
				summand -= (shaderStart - buffers[i]);
				if ( summand >= 0 ) {
					memmove( buffers[i], shaderStart, summand + 1 );
				}
			}
			//sum += summand;
			sum += COM_Compress( buffers[ i ] );
		}
	}

	return sum;
}


/*
====================
ScanAndLoadShaderFiles

Finds and loads all .shader files, combining them into
a single large text block that can be scanned for shader names
=====================
*/
GDR_EXPORT void ScanAndLoadShaderFiles( void )
{
	char **shaderFiles;
	char *buffers[MAX_SHADER_FILES];
	uint64_t numShaderFiles;
	int64_t i;
	const char *tok;
    char *hashMem;
	char *textEnd;
	const char *p, *oldp;
	uint64_t shaderTextHashTableSizes[MAX_SHADERTEXT_HASH], hash, size;

	memset(buffers, 0, sizeof(buffers));
	memset(shaderTextHashTableSizes, 0, sizeof(shaderTextHashTableSizes));

    uint64_t sum = 0;

	// scan for legacy shader files
	shaderFiles = ri.FS_ListFiles( "scripts", ".shader", &numShaderFiles );
//	mtrFiles = ri.FS_ListFiles( "scripts", ".mtrx", &numMtrFiles );

	ri.Printf(PRINT_DEVELOPER, "Found %lu shader files.\n", numShaderFiles);

	if (!shaderFiles || !numShaderFiles) {
		ri.Printf( PRINT_WARNING, "no shader files found\n" );
		return;
	}

	if ( numShaderFiles > MAX_SHADER_FILES ) {
		numShaderFiles = MAX_SHADER_FILES;
	}

	sum = 0;
	sum += loadShaderBuffers( shaderFiles, numShaderFiles, buffers );

	// build single large buffer
	r_shaderText = (char *)ri.Hunk_Alloc( sum + numShaderFiles*2 + 1, h_low );
	memset(r_shaderText, 0, sum + numShaderFiles*2);

	textEnd = r_shaderText;

	// free in reverse order, so the temp files are all dumped
	// legacy shaders
	for ( i = numShaderFiles - 1; i >= 0 ; i-- ) {
		if ( buffers[ i ] ) {
			textEnd = N_stradd( textEnd, buffers[ i ] );
			textEnd = N_stradd( textEnd, "\n" );
			ri.FS_FreeFile( buffers[ i ] );
		}
	}

	// if shader text >= r_extensionOffset then it is an extended shader
	// normal shaders will never encounter that
	r_extensionOffset = textEnd;

	// free up memory
	if ( shaderFiles ) {
		ri.FS_FreeFileList( shaderFiles );
    }

	COM_Compress( r_shaderText );
	memset( shaderTextHashTableSizes, 0, sizeof( shaderTextHashTableSizes ) );
	size = 0;

	p = r_shaderText;
	// look for shader names
	while ( 1 ) {
		tok = COM_ParseExt( &p, qtrue );
		if ( tok[0] == 0 ) {
			break;
		}
		hash = Com_GenerateHashValue(tok, MAX_SHADERTEXT_HASH);
		shaderTextHashTableSizes[hash]++;
		size++;
		SkipBracedSection(&p, 0);
	}

	size += MAX_SHADERTEXT_HASH;

	hashMem = (char *)ri.Hunk_Alloc( size * sizeof(char *), h_low );

	for (i = 0; i < MAX_SHADERTEXT_HASH; i++) {
		shaderTextHashTable[i] = (const char **) hashMem;
		hashMem = ((char *) hashMem) + ((shaderTextHashTableSizes[i] + 1) * sizeof(char *));
	}

	p = r_shaderText;
	// look for shader names
	while ( 1 ) {
		oldp = p;
		tok = COM_ParseExt( &p, qtrue );
		if ( tok[0] == 0 ) {
			break;
		}

		hash = Com_GenerateHashValue(tok, MAX_SHADERTEXT_HASH);
		shaderTextHashTable[hash][--shaderTextHashTableSizes[hash]] = (char*)oldp;

		SkipBracedSection(&p, 0);
	}
}


/*
====================
CreateInternalShaders
====================
*/
GDR_EXPORT void CreateInternalShaders( void )
{
	rg.numShaders = 0;

	// init the default shader
	InitShader( "<default>" );
    stages[0].stateBits = GLS_DEFAULT;
	rg.defaultShader = FinishShader();
}

/*
==================
R_InitShaders
==================
*/
GDR_EXPORT void R_InitShaders( void )
{
	ri.Printf( PRINT_INFO, "Initializing Shaders\n" );

    memset(hashTable, 0, sizeof(hashTable));

	CreateInternalShaders();

	ScanAndLoadShaderFiles();
}


