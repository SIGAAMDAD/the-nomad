#include "rgl_local.h"

// rgl_shader.c -- this file deals with the parsing and defintion of shaders

static char *r_shaderText;
static const char *r_extensionOffset;
static uint64_t r_extendedShader;

static shader_t shader;
static shader_t *hashTable[MAX_RENDER_SHADERS];
static texModInfo_t texMods[MAX_SHADER_STAGES][TR_MAX_TEXMODS];;

static shaderStage_t stages[MAX_SHADER_STAGES];

#define MAX_SHADERTEXT_HASH 2048
static const char **shaderTextHashTable[MAX_SHADERTEXT_HASH];

static void ParseSort(const char **text)
{
	const char *tok;

	tok = COM_ParseExt(text, qfalse);
	if (tok[0] == 0) {
		ri.Printf(PRINT_INFO, COLOR_YELLOW "WARNING: missing sort in shader '%s'\n", shader.name);
		return;
	}

	if (!N_stricmp(tok, "opaque")) {
		shader.sort = SS_OPAQUE;
	} else if (!N_stricmp(tok, "decal")) {
		shader.sort = SS_DECAL;
	}
}

/*
===============
NameToAFunc
===============
*/
static unsigned NameToAFunc( const char *funcname )
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

	ri.Printf( PRINT_INFO, COLOR_YELLOW "WARNING: invalid alphaFunc name '%s' in shader '%s'\n", funcname, shader.name );
	return 0;
}

/*
===============
NameToSrcBlendMode
===============
*/
static int NameToSrcBlendMode( const char *name )
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

	ri.Printf( PRINT_INFO, COLOR_YELLOW "WARNING: unknown blend mode '%s' in shader '%s', substituting GL_ONE\n", name, shader.name );
	return GLS_SRCBLEND_ONE;
}

/*
===============
NameToDstBlendMode
===============
*/
static int NameToDstBlendMode( const char *name )
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

	ri.Printf( PRINT_INFO, COLOR_YELLOW "WARNING: unknown blend mode '%s' in shader '%s', substituting GL_ONE\n", name, shader.name );
	return GLS_DSTBLEND_ONE;
}

/*
===============
ParseVector
===============
*/
static qboolean ParseVector( const char **text, int count, float *v ) {
	const char	*token;
	int		i;

	// FIXME: spaces are currently required after parens, should change parseext...
	token = COM_ParseExt( text, qfalse );
	if ( strcmp( token, "(" ) ) {
		ri.Printf( PRINT_INFO, COLOR_YELLOW "WARNING: missing parenthesis in shader '%s'\n", shader.name );
		return qfalse;
	}

	for ( i = 0 ; i < count ; i++ ) {
		token = COM_ParseExt( text, qfalse );
		if ( !token[0] ) {
			ri.Printf( PRINT_INFO, COLOR_YELLOW "WARNING: missing vector element in shader '%s'\n", shader.name );
			return qfalse;
		}
		v[i] = N_atof( token );
	}

	token = COM_ParseExt( text, qfalse );
	if ( strcmp( token, ")" ) ) {
		ri.Printf( PRINT_INFO, COLOR_YELLOW "WARNING: missing parenthesis in shader '%s'\n", shader.name );
		return qfalse;
	}

	return qtrue;
}

/*
===================
ParseTexMod
===================
*/
static void ParseTexMod( const char *_text, shaderStage_t *stage )
{
	const char *token;
	const char **text = &_text;
	texModInfo_t *tmi;

	if ( stage->bundle[0].numTexMods == TR_MAX_TEXMODS ) {
		ri.Error( ERR_DROP, "ERROR: too many tcMod stages in shader '%s'", shader.name );
		return;
	}

	tmi = &stage->bundle[0].texMods[stage->bundle[0].numTexMods];
	stage->bundle[0].numTexMods++;

	token = COM_ParseExt( text, qfalse );

	//
	// turb
	//
	#if 0
	if ( !N_stricmp( token, "turb" ) )
	{
		token = COM_ParseExt( text, qfalse );
		if ( token[0] == 0 )
		{
			ri.Printf( PRINT_INFO, COLOR_YELLOW "WARNING: missing tcMod turb parms in shader '%s'\n", shader.name );
			return;
		}
		tmi->wave.base = N_atof( token );
		token = COM_ParseExt( text, qfalse );
		if ( token[0] == 0 )
		{
			ri.Printf( PRINT_INFO, COLOR_YELLOW "WARNING: missing tcMod turb in shader '%s'\n", shader.name );
			return;
		}
		tmi->wave.amplitude = N_atof( token );
		token = COM_ParseExt( text, qfalse );
		if ( token[0] == 0 )
		{
			ri.Printf( PRINT_INFO, COLOR_YELLOW "WARNING: missing tcMod turb in shader '%s'\n", shader.name );
			return;
		}
		tmi->wave.phase = N_atof( token );
		token = COM_ParseExt( text, qfalse );
		if ( token[0] == 0 )
		{
			ri.Printf( PRINT_INFO, COLOR_YELLOW "WARNING: missing tcMod turb in shader '%s'\n", shader.name );
			return;
		}
		tmi->wave.frequency = N_atof( token );

		tmi->type = TMOD_TURBULENT;
	}
	#endif
	//
	// scale
	//
	if ( !N_stricmp( token, "scale" ) )
	{
		token = COM_ParseExt( text, qfalse );
		if ( token[0] == 0 )
		{
			ri.Printf( PRINT_INFO, COLOR_YELLOW "WARNING: missing scale parms in shader '%s'\n", shader.name );
			return;
		}
		tmi->scale[0] = N_atof( token );

		token = COM_ParseExt( text, qfalse );
		if ( token[0] == 0 )
		{
			ri.Printf( PRINT_INFO, COLOR_YELLOW "WARNING: missing scale parms in shader '%s'\n", shader.name );
			return;
		}
		tmi->scale[1] = N_atof( token );
		tmi->type = TMOD_SCALE;
	}
	//
	// scroll
	//
	else if ( !N_stricmp( token, "scroll" ) )
	{
		token = COM_ParseExt( text, qfalse );
		if ( token[0] == 0 )
		{
			ri.Printf( PRINT_INFO, COLOR_YELLOW "WARNING: missing scale scroll parms in shader '%s'\n", shader.name );
			return;
		}
		tmi->scroll[0] = N_atof( token );
		token = COM_ParseExt( text, qfalse );
		if ( token[0] == 0 )
		{
			ri.Printf( PRINT_INFO, COLOR_YELLOW "WARNING: missing scale scroll parms in shader '%s'\n", shader.name );
			return;
		}
		tmi->scroll[1] = N_atof( token );
		tmi->type = TMOD_SCROLL;
	}
	#if 0
	//
	// stretch
	//
	else if ( !N_stricmp( token, "stretch" ) )
	{
		token = COM_ParseExt( text, qfalse );
		if ( token[0] == 0 )
		{
			ri.Printf( PRINT_INFO, COLOR_YELLOW "WARNING: missing stretch parms in shader '%s'\n", shader.name );
			return;
		}
		tmi->wave.func = NameToGenFunc( token );

		token = COM_ParseExt( text, qfalse );
		if ( token[0] == 0 )
		{
			ri.Printf( PRINT_INFO, COLOR_YELLOW "WARNING: missing stretch parms in shader '%s'\n", shader.name );
			return;
		}
		tmi->wave.base = atof( token );

		token = COM_ParseExt( text, qfalse );
		if ( token[0] == 0 )
		{
			ri.Printf( PRINT_INFO, COLOR_YELLOW "WARNING: missing stretch parms in shader '%s'\n", shader.name );
			return;
		}
		tmi->wave.amplitude = N_atof( token );

		token = COM_ParseExt( text, qfalse );
		if ( token[0] == 0 )
		{
			ri.Printf( PRINT_INFO, COLOR_YELLOW "WARNING: missing stretch parms in shader '%s'\n", shader.name );
			return;
		}
		tmi->wave.phase = N_atof( token );

		token = COM_ParseExt( text, qfalse );
		if ( token[0] == 0 )
		{
			ri.Printf( PRINT_INFO, COLOR_YELLOW "WARNING: missing stretch parms in shader '%s'\n", shader.name );
			return;
		}
		tmi->wave.frequency = N_atof( token );
		
		tmi->type = TMOD_STRETCH;
	}
	#endif
	//
	// transform
	//
	else if ( !N_stricmp( token, "transform" ) )
	{
		token = COM_ParseExt( text, qfalse );
		if ( token[0] == 0 )
		{
			ri.Printf( PRINT_INFO, COLOR_YELLOW "WARNING: missing transform parms in shader '%s'\n", shader.name );
			return;
		}
		tmi->matrix[0][0] = N_atof( token );

		token = COM_ParseExt( text, qfalse );
		if ( token[0] == 0 )
		{
			ri.Printf( PRINT_INFO, COLOR_YELLOW "WARNING: missing transform parms in shader '%s'\n", shader.name );
			return;
		}
		tmi->matrix[0][1] = N_atof( token );

		token = COM_ParseExt( text, qfalse );
		if ( token[0] == 0 )
		{
			ri.Printf( PRINT_INFO, COLOR_YELLOW "WARNING: missing transform parms in shader '%s'\n", shader.name );
			return;
		}
		tmi->matrix[1][0] = N_atof( token );

		token = COM_ParseExt( text, qfalse );
		if ( token[0] == 0 )
		{
			ri.Printf( PRINT_INFO, COLOR_YELLOW "WARNING: missing transform parms in shader '%s'\n", shader.name );
			return;
		}
		tmi->matrix[1][1] = N_atof( token );

		token = COM_ParseExt( text, qfalse );
		if ( token[0] == 0 )
		{
			ri.Printf( PRINT_INFO, COLOR_YELLOW "WARNING: missing transform parms in shader '%s'\n", shader.name );
			return;
		}
		tmi->translate[0] = N_atof( token );

		token = COM_ParseExt( text, qfalse );
		if ( token[0] == 0 )
		{
			ri.Printf( PRINT_INFO, COLOR_YELLOW "WARNING: missing transform parms in shader '%s'\n", shader.name );
			return;
		}
		tmi->translate[1] = N_atof( token );

		tmi->type = TMOD_TRANSFORM;
	}
	//
	// rotate
	//
	else if ( !N_stricmp( token, "rotate" ) )
	{
		token = COM_ParseExt( text, qfalse );
		if ( token[0] == 0 )
		{
			ri.Printf( PRINT_INFO, COLOR_YELLOW "WARNING: missing tcMod rotate parms in shader '%s'\n", shader.name );
			return;
		}
		tmi->rotateSpeed = N_atof( token );
		tmi->type = TMOD_ROTATE;
	}
	//
	// entityTranslate
	//
	else if ( !N_stricmp( token, "entityTranslate" ) )
	{
		tmi->type = TMOD_ENTITY_TRANSLATE;
	}
	else
	{
		ri.Printf( PRINT_INFO, COLOR_YELLOW "WARNING: unknown tcMod '%s' in shader '%s'\n", token, shader.name );
	}
}

static qboolean ParseStage(shaderStage_t *stage, const char **text)
{
	const char *tok;
	int depthMaskBits = GLS_DEPTHMASK_TRUE, blendSrcBits = 0, blendDstBits = 0, atestBits = 0, depthFuncBits = 0;
	qboolean depthMaskExplicit = qfalse;

	stage->active = qtrue;

	while (1) {
		tok = COM_ParseExt(text, qtrue);
		if (!tok[0]) {
			ri.Printf(PRINT_INFO, COLOR_YELLOW "WARNING: no matching '}' found\n");
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
				ri.Printf(PRINT_INFO, COLOR_YELLOW "WARNING: missing parameter for 'map' keyword in shader '%s'\n", shader.name);
				return qfalse;
			}

			if (!N_stricmp(tok, "$whiteimage")) {
				stage->bundle[0].image[0] = rg.whiteImage;
				continue;
			}
			else if (!N_stricmp(tok, "$lightmap")) {
				stage->bundle[0].isLightmap = qtrue;
				stage->bundle[0].image[0] = rg.whiteImage;
			}
			else {
				imgType_t type = IMGTYPE_COLORALPHA;
				imgFlags_t flags = IMGFLAG_NONE;

				if (!shader.noMipMaps)
					flags |= IMGFLAG_MIPMAP;

				if (!shader.noPicMips)
					flags |= IMGFLAG_PICMIP;

				if (stage->type == ST_NORMALMAP || stage->type == ST_NORMALPARALLAXMAP) {
					type = IMGTYPE_NORMAL;
					flags |= IMGFLAG_NOLIGHTSCALE;

					if (stage->type == ST_NORMALPARALLAXMAP)
						type = IMGTYPE_NORMALHEIGHT;
				}
//				else {
//					if (r_genNormalMaps->i)
//						flags |= IMGFLAG_GENNORMALMAP;
//				}

				stage->bundle[0].image[0] = R_FindImageFile( tok, type, flags );

				if ( !stage->bundle[0].image[0] ) {
					ri.Printf( PRINT_INFO, COLOR_YELLOW "WARNING: R_FindImageFile could not find '%s' in shader '%s'\n", tok, shader.name );
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
				ri.Printf( PRINT_INFO, COLOR_YELLOW "WARNING: missing parameter for 'alphaFunc' keyword in shader '%s'\n", shader.name );
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
				ri.Printf( PRINT_INFO, COLOR_YELLOW "WARNING: missing parameter for 'depthfunc' keyword in shader '%s'\n", shader.name );
				return qfalse;
			}

			if ( !N_stricmp( tok, "lequal" ) ) {
				depthFuncBits = 0;
			}
			else if ( !N_stricmp( tok, "equal" ) ) {
				depthFuncBits = GLS_DEPTHFUNC_EQUAL;
			}
			else {
				ri.Printf( PRINT_INFO, COLOR_YELLOW "WARNING: unknown depthfunc '%s' in shader '%s'\n", tok, shader.name );
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
				ri.Printf( PRINT_INFO, COLOR_YELLOW "WARNING: missing parm for blendFunc in shader '%s'\n", shader.name );
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
					ri.Printf( PRINT_INFO, COLOR_YELLOW "WARNING: missing parm for blendFunc in shader '%s'\n", shader.name );
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
				ri.Printf( PRINT_INFO, COLOR_YELLOW "WARNING: missing parameters for rgbGen in shader '%s'\n", shader.name );
				continue;
			}

			/*
			if ( !N_stricmp( tok, "wave" ) )
			{
				ParseWaveForm( text, &stage->rgbWave );
				stage->rgbGen = CGEN_WAVEFORM;
			} */
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
				ri.Printf( PRINT_INFO, COLOR_YELLOW "WARNING: unknown rgbGen parameter '%s' in shader '%s'\n", tok, shader.name );
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
				ri.Printf( PRINT_INFO, COLOR_YELLOW "WARNING: missing parameters for alphaGen in shader '%s'\n", shader.name );
				continue;
			}
			/*
			if ( !N_stricmp( tok, "wave" ) )
			{
				ParseWaveForm( text, &stage->alphaWave );
				stage->alphaGen = AGEN_WAVEFORM;
			} */
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
				ri.Printf( PRINT_INFO, COLOR_YELLOW "WARNING: unknown alphaGen parameter '%s' in shader '%s'\n", tok, shader.name );
				continue;
			}
		}
		//
		// tcGen <function>
		//
		else if ( !N_stricmp(tok, "texgen") || !N_stricmp( tok, "tcGen" ) )
		{
			tok = COM_ParseExt( text, qfalse );
			if ( tok[0] == 0 )
			{
				ri.Printf( PRINT_INFO, COLOR_YELLOW "WARNING: missing texgen parm in shader '%s'\n", shader.name );
				continue;
			}

			if ( !N_stricmp( tok, "environment" ) )
			{
				const char *t = *text;
				stage->bundle[0].tcGen = TCGEN_ENVIRONMENT_MAPPED;
				tok = COM_ParseExt( text, qfalse );
				if ( N_stricmp( tok, "firstPerson" ) == 0 )
				{
					//stage->bundle[0].tcGen = TCGEN_ENVIRONMENT_MAPPED_FP;
				}
				else
				{
					*text = t; // rewind
				}
			}
			else if ( !N_stricmp( tok, "lightmap" ) )
			{
				stage->bundle[0].tcGen = TCGEN_LIGHTMAP;
			}
			else if ( !N_stricmp( tok, "texture" ) || !N_stricmp( tok, "base" ) )
			{
				stage->bundle[0].tcGen = TCGEN_TEXTURE;
			}
			else if ( !N_stricmp( tok, "vector" ) )
			{
				ParseVector( text, 3, stage->bundle[0].tcGenVectors[0] );
				ParseVector( text, 3, stage->bundle[0].tcGenVectors[1] );

				stage->bundle[0].tcGen = TCGEN_VECTOR;
			}
			else 
			{
				ri.Printf( PRINT_INFO, COLOR_YELLOW "WARNING: unknown texgen parm in shader '%s'\n", shader.name );
			}
		}
		//
		// tcMod <type> <...>
		//
		else if ( !N_stricmp( tok, "tcMod" ) )
		{
			char buffer[1024] = "";

			while ( 1 )
			{
				tok = COM_ParseExt( text, qfalse );
				if ( tok[0] == 0 )
					break;
				N_strcat( buffer, sizeof (buffer), tok );
				N_strcat( buffer, sizeof (buffer), " " );
			}

			ParseTexMod( buffer, stage );

			continue;
		}
		//
		// depthmask
		//
		else if ( !N_stricmp( tok, "depthwrite" ) )
		{
			depthMaskBits = GLS_DEPTHMASK_TRUE;
			depthMaskExplicit = qtrue;

			continue;
		}
		else if ( !N_stricmp( tok, "depthFragment" ) )
		{
			continue;
		}
		else
		{
			ri.Printf( PRINT_INFO, COLOR_YELLOW "WARNING: unknown parameter '%s' in shader '%s'\n", tok, shader.name );
			return qfalse;
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

static qboolean ParseShader(const char **text)
{
    const char *tok;
	int s;
	uintptr_t p1, p2;

	// gcc doesn't like it when we do *text + r_extensionOffset
	p1 = (uintptr_t)(*text);
	p2 = (uintptr_t)(r_extensionOffset);

	s = 0;
	r_extendedShader = p1 + p2;

	tok = COM_ParseExt(text, qtrue);
	if (tok[0] != '{') {
		ri.Printf(PRINT_INFO, COLOR_YELLOW "WARNING: expecting '{', found '%s' instead in shader '%s'\n", tok, shader.name);
		return qfalse;
	}

    while (1) {
        tok = COM_ParseComplex(text, qtrue);

        if (!tok[0]) {
            ri.Printf(PRINT_INFO, COLOR_YELLOW "WARNING: no concluding '}' in shader %s\n", shader.name);
            return qfalse;
        }

		// end of shader definition
        if (tok[0] == '}') {
            break;
        }
		// stage definition
		if (tok[0] == '{') {
			if (s >= MAX_SHADER_STAGES) {
				ri.Printf(PRINT_INFO, COLOR_YELLOW "WARNING: too many stages in shader %s (max is %i)\n", shader.name, MAX_SHADER_STAGES);
				return qfalse;
			}

			if (!ParseStage(&stages[s], text)) {
				return qfalse;
			}

			stages[s].active = qtrue;
			s++;

			continue;
		}
		// no mip maps
		else if (!N_stricmp(tok, "nomipmaps")) {
			shader.noMipMaps = qtrue;
			shader.noPicMips = qtrue;
			continue;
		}
		// no picmip adjustment
		else if (!N_stricmp(tok, "nopicmip")) {
			shader.noPicMips = qtrue;
			continue;
		}
		// polygonOffset
		else if (!N_stricmp(tok, "polygonOffset")) {
			shader.polygonOffset = qtrue;
			continue;
		}
    }

    return qtrue;
}

//========================================================================================

/*
===================
ComputeVertexAttribs

Check which vertex attributes we only need, so we
don't need to submit/copy all of them.
===================
*/
static void ComputeVertexAttribs(void)
{
	uint32_t i, stage;

	// dlights always need ATTRIB_NORMAL
	shader.vertexAttribs = ATTRIB_POSITION | ATTRIB_NORMAL;

	// portals always need normals, for SurfIsOffscreen()
	#if 0
	if (shader.isPortal)
	{
		shader.vertexAttribs |= ATTRIB_NORMAL;
	}
	#endif

	if (shader.defaultShader) 	{
		shader.vertexAttribs |= ATTRIB_TEXCOORD;
		return;
	}

	#if 0
	if(shader.numDeforms)
	{
		for ( i = 0; i < shader.numDeforms; i++)
		{
			deformStage_t  *ds = &shader.deforms[i];

			switch (ds->deformation)
			{
				case DEFORM_BULGE:
					shader.vertexAttribs |= ATTRIB_NORMAL | ATTRIB_TEXCOORD;
					break;

				case DEFORM_AUTOSPRITE:
					shader.vertexAttribs |= ATTRIB_NORMAL | ATTRIB_COLOR;
					break;

				case DEFORM_WAVE:
				case DEFORM_NORMALS:
				case DEFORM_TEXT0:
				case DEFORM_TEXT1:
				case DEFORM_TEXT2:
				case DEFORM_TEXT3:
				case DEFORM_TEXT4:
				case DEFORM_TEXT5:
				case DEFORM_TEXT6:
				case DEFORM_TEXT7:
					shader.vertexAttribs |= ATTRIB_NORMAL;
					break;

				default:
				case DEFORM_NONE:
				case DEFORM_MOVE:
				case DEFORM_PROJECTION_SHADOW:
				case DEFORM_AUTOSPRITE2:
					break;
			}
		}
	}
	#endif

	for ( stage = 0; stage < MAX_SHADER_STAGES; stage++ ) {
		shaderStage_t *pStage = &stages[stage];

		if ( !pStage->active ) {
			break;
		}

	#if 0
		if (pStage->glslShaderGroup == rg.lightallShader)
		{
			shader.vertexAttribs |= ATTRIB_NORMAL;

			if ((pStage->glslShaderIndex & LIGHTDEF_LIGHTTYPE_MASK) && !(r_normalMapping->i == 0 && r_specularMapping->i == 0))
			{
				shader.vertexAttribs |= ATTRIB_TANGENT;
			}

			switch (pStage->glslShaderIndex & LIGHTDEF_LIGHTTYPE_MASK)
			{
				case LIGHTDEF_USE_LIGHTMAP:
				case LIGHTDEF_USE_LIGHT_VERTEX:
					shader.vertexAttribs |= ATTRIB_LIGHTDIRECTION;
					break;
				default:
					break;
			}
		}
	#endif

		for (i = 0; i < NUM_TEXTURE_BUNDLES; i++) {
			if ( pStage->bundle[i].image[0] == 0 ) {
				continue;
			}

			switch(pStage->bundle[i].tcGen) {
			case TCGEN_TEXTURE:
				shader.vertexAttribs |= ATTRIB_TEXCOORD;
				break;
//			case TCGEN_LIGHTMAP:
//				shader.vertexAttribs |= ATTRIB_LIGHTCOORD;
//				break;
			case TCGEN_ENVIRONMENT_MAPPED:
				shader.vertexAttribs |= ATTRIB_NORMAL;
				break;
			default:
				break;
			};
		}

		switch(pStage->rgbGen) {
		case CGEN_EXACT_VERTEX:
		case CGEN_VERTEX:
		case CGEN_EXACT_VERTEX_LIT:
		case CGEN_VERTEX_LIT:
		case CGEN_ONE_MINUS_VERTEX:
			shader.vertexAttribs |= ATTRIB_COLOR;
			break;
		case CGEN_LIGHTING_DIFFUSE:
			shader.vertexAttribs |= ATTRIB_NORMAL;
			break;
		default:
			break;
		};

		switch(pStage->alphaGen) {
		case AGEN_LIGHTING_SPECULAR:
			shader.vertexAttribs |= ATTRIB_NORMAL;
			break;
		case AGEN_VERTEX:
		case AGEN_ONE_MINUS_VERTEX:
			shader.vertexAttribs |= ATTRIB_COLOR;
			break;
		default:
			break;
		};
	}
}


static void CollapseStagesToLightall(shaderStage_t *diffuse, 
	shaderStage_t *normal, shaderStage_t *specular, shaderStage_t *lightmap, 
	qboolean useLightVector, qboolean useLightVertex, qboolean parallax, qboolean tcgen)
{
	int defs = 0;

	//ri.Printf(PRINT_ALL, "shader %s has diffuse %s", shader.name, diffuse->bundle[0].image[0]->imgName);

	// reuse diffuse, mark others inactive
	diffuse->type = ST_GLSL;

#if 0
	if (lightmap)
	{
		//ri.Printf(PRINT_ALL, ", lightmap");
		diffuse->bundle[TB_LIGHTMAP] = lightmap->bundle[0];
		defs |= LIGHTDEF_USE_LIGHTMAP;
	}
	else if (useLightVector)
	{
		defs |= LIGHTDEF_USE_LIGHT_VECTOR;
	}
	else if (useLightVertex)
	{
		defs |= LIGHTDEF_USE_LIGHT_VERTEX;
	}
#endif

	// [glnomad] until I understand what deluxemapping is, I'm not using it
	#if 0
	if (r_deluxeMapping->i && rg.worldDeluxeMapping && lightmap && shader.lightmapIndex >= 0)
	{
		//ri.Printf(PRINT_ALL, ", deluxemap");
		diffuse->bundle[TB_DELUXEMAP] = lightmap->bundle[0];
		diffuse->bundle[TB_DELUXEMAP].image[0] = rg.deluxemaps[shader.lightmapIndex];
	}
	#endif

	if (r_normalMapping->i)
	{
		texture_t *diffuseImg;
		if (normal)
		{
			//ri.Printf(PRINT_ALL, ", normalmap %s", normal->bundle[0].image[0]->imgName);
			diffuse->bundle[TB_NORMALMAP] = normal->bundle[0];
//			if (parallax && r_parallaxMapping->i)
//				defs |= LIGHTDEF_USE_PARALLAXMAP;

			VectorCopy4(normal->normalScale, diffuse->normalScale);
		}
		else if ((lightmap || useLightVector || useLightVertex) && (diffuseImg = diffuse->bundle[TB_DIFFUSEMAP].image[0]) != NULL)
		{
			char normalName[MAX_GDR_PATH];
			texture_t *normalImg;
			imgFlags_t normalFlags = (diffuseImg->flags & ~IMGFLAG_GENNORMALMAP) | IMGFLAG_NOLIGHTSCALE;

			// try a normalheight image first
			COM_StripExtension(diffuseImg->imgName, normalName, MAX_GDR_PATH);
			N_strcat(normalName, MAX_GDR_PATH, "_nh");

			normalImg = R_FindImageFile(normalName, IMGTYPE_NORMALHEIGHT, normalFlags);

			if (normalImg)
			{
				parallax = qtrue;
			}
			else
			{
				// try a normal image ("_n" suffix)
				normalName[strlen(normalName) - 1] = '\0';
				normalImg = R_FindImageFile(normalName, IMGTYPE_NORMAL, normalFlags);
			}

			if (normalImg)
			{
				diffuse->bundle[TB_NORMALMAP] = diffuse->bundle[0];
				diffuse->bundle[TB_NORMALMAP].numImageAnimations = 0;
				diffuse->bundle[TB_NORMALMAP].image[0] = normalImg;

//				if (parallax && r_parallaxMapping->i)
//					defs |= LIGHTDEF_USE_PARALLAXMAP;

				VectorSet4(diffuse->normalScale, r_baseNormalX->f, r_baseNormalY->f, 1.0f, r_baseParallax->f);
			}
		}
	}

	if (r_specularMapping->i)
	{
		texture_t *diffuseImg;
		if (specular)
		{
			//ri.Printf(PRINT_ALL, ", specularmap %s", specular->bundle[0].image[0]->imgName);
			diffuse->bundle[TB_SPECULARMAP] = specular->bundle[0];
			VectorCopy4(specular->specularScale, diffuse->specularScale);
		}
		else if ((lightmap || useLightVector || useLightVertex) && (diffuseImg = diffuse->bundle[TB_DIFFUSEMAP].image[0]) != NULL)
		{
			char specularName[MAX_GDR_PATH];
			texture_t *specularImg;
			imgFlags_t specularFlags = (diffuseImg->flags & ~IMGFLAG_GENNORMALMAP) | IMGFLAG_NOLIGHTSCALE;

			COM_StripExtension(diffuseImg->imgName, specularName, MAX_GDR_PATH);
			N_strcat(specularName, MAX_GDR_PATH, "_s");

			specularImg = R_FindImageFile(specularName, IMGTYPE_COLORALPHA, specularFlags);

			if (specularImg)
			{
				diffuse->bundle[TB_SPECULARMAP] = diffuse->bundle[0];
				diffuse->bundle[TB_SPECULARMAP].numImageAnimations = 0;
				diffuse->bundle[TB_SPECULARMAP].image[0] = specularImg;

				VectorSet4(diffuse->specularScale, 1.0f, 1.0f, 1.0f, 1.0f);
			}
		}
	}

	if (tcgen || diffuse->bundle[0].numTexMods)
	{
//		defs |= LIGHTDEF_USE_TCGEN_AND_TCMOD;
	}

	//ri.Printf(PRINT_ALL, ".\n");

//	diffuse->glslShaderGroup = rg.lightallShader;
//	diffuse->glslShaderIndex = defs;
}



static int CollapseStagesToGLSL(void)
{
	int i, j, numStages;
	qboolean skip = qfalse;

	// skip shaders with deforms
//	if (shader.numDeforms != 0)
//	{
//		skip = qtrue;
//	}

	if (!skip)
	{
		// if 2+ stages and first stage is lightmap, switch them
		// this makes it easier for the later bits to process
		if (stages[0].active && stages[0].bundle[0].tcGen == TCGEN_LIGHTMAP && stages[1].active)
		{
			int blendBits = stages[1].stateBits & ( GLS_DSTBLEND_BITS | GLS_SRCBLEND_BITS );

			if (blendBits == (GLS_DSTBLEND_SRC_COLOR | GLS_SRCBLEND_ZERO)
				|| blendBits == (GLS_DSTBLEND_ZERO | GLS_SRCBLEND_DST_COLOR))
			{
				int stateBits0 = stages[0].stateBits;
				int stateBits1 = stages[1].stateBits;
				shaderStage_t swapStage;

				swapStage = stages[0];
				stages[0] = stages[1];
				stages[1] = swapStage;

				stages[0].stateBits = stateBits0;
				stages[1].stateBits = stateBits1;
			}
		}
	}

	if (!skip)
	{
		// scan for shaders that aren't supported
		for (i = 0; i < MAX_SHADER_STAGES; i++)
		{
			shaderStage_t *pStage = &stages[i];

			if (!pStage->active)
				continue;

			//if (pStage->adjustColorsForFog)
			//{
			//	skip = qtrue;
			//	break;
			//}

			if (pStage->bundle[0].tcGen == TCGEN_LIGHTMAP)
			{
				int blendBits = pStage->stateBits & ( GLS_DSTBLEND_BITS | GLS_SRCBLEND_BITS );
				
				if (blendBits != (GLS_DSTBLEND_SRC_COLOR | GLS_SRCBLEND_ZERO)
					&& blendBits != (GLS_DSTBLEND_ZERO | GLS_SRCBLEND_DST_COLOR))
				{
					skip = qtrue;
					break;
				}
			}

			switch(pStage->bundle[0].tcGen)
			{
				case TCGEN_TEXTURE:
				case TCGEN_LIGHTMAP:
				case TCGEN_ENVIRONMENT_MAPPED:
				case TCGEN_VECTOR:
					break;
				default:
					skip = qtrue;
					break;
			}

			switch(pStage->alphaGen)
			{
				case AGEN_LIGHTING_SPECULAR:
				case AGEN_PORTAL:
					skip = qtrue;
					break;
				default:
					break;
			}
		}
	}

	if (!skip)
	{
		qboolean usedLightmap = qfalse;

		for (i = 0; i < MAX_SHADER_STAGES; i++)
		{
			shaderStage_t *pStage = &stages[i];
			shaderStage_t *diffuse, *normal, *specular, *lightmap;
			qboolean parallax, tcgen, diffuselit, vertexlit;

			if (!pStage->active)
				continue;

			// skip normal and specular maps
			if (pStage->type != ST_COLORMAP)
				continue;

			// skip lightmaps
			if (pStage->bundle[0].tcGen == TCGEN_LIGHTMAP)
				continue;

			diffuse  = pStage;
			normal   = NULL;
			parallax = qfalse;
			specular = NULL;
			lightmap = NULL;

			// we have a diffuse map, find matching normal, specular, and lightmap
			for (j = i + 1; j < MAX_SHADER_STAGES; j++)
			{
				shaderStage_t *pStage2 = &stages[j];

				if (!pStage2->active)
					continue;

				switch(pStage2->type)
				{
					case ST_NORMALMAP:
						if (!normal)
						{
							normal = pStage2;
						}
						break;

					case ST_NORMALPARALLAXMAP:
						if (!normal)
						{
							normal = pStage2;
							parallax = qtrue;
						}
						break;

					case ST_SPECULARMAP:
						if (!specular)
						{
							specular = pStage2;
						}
						break;

					case ST_COLORMAP:
						if (pStage2->bundle[0].tcGen == TCGEN_LIGHTMAP)
						{
							int blendBits = pStage->stateBits & ( GLS_DSTBLEND_BITS | GLS_SRCBLEND_BITS );

							// Only add lightmap to blendfunc filter stage if it's the first time lightmap is used
							// otherwise it will cause the shader to be darkened by the lightmap multiple times.
							if (!usedLightmap || (blendBits != (GLS_DSTBLEND_SRC_COLOR | GLS_SRCBLEND_ZERO)
								&& blendBits != (GLS_DSTBLEND_ZERO | GLS_SRCBLEND_DST_COLOR)))
							{
								lightmap = pStage2;
								usedLightmap = qtrue;
							}
						}
						break;

					default:
						break;
				}
			}

			tcgen = qfalse;
			if (diffuse->bundle[0].tcGen == TCGEN_ENVIRONMENT_MAPPED
			    || diffuse->bundle[0].tcGen == TCGEN_LIGHTMAP
			    || diffuse->bundle[0].tcGen == TCGEN_VECTOR)
			{
				tcgen = qtrue;
			}

			diffuselit = qfalse;
			if (diffuse->rgbGen == CGEN_LIGHTING_DIFFUSE)
			{
				diffuselit = qtrue;
			}

			vertexlit = qfalse;
			if (diffuse->rgbGen == CGEN_VERTEX_LIT || diffuse->rgbGen == CGEN_EXACT_VERTEX_LIT)
			{
				vertexlit = qtrue;
			}

			CollapseStagesToLightall(diffuse, normal, specular, lightmap, diffuselit, vertexlit, parallax, tcgen);
		}

		// deactivate lightmap stages
		for (i = 0; i < MAX_SHADER_STAGES; i++)
		{
			shaderStage_t *pStage = &stages[i];

			if (!pStage->active)
				continue;

			if (pStage->bundle[0].tcGen == TCGEN_LIGHTMAP)
			{
				pStage->active = qfalse;
			}
		}
	}

	// deactivate normal and specular stages
	for (i = 0; i < MAX_SHADER_STAGES; i++)
	{
		shaderStage_t *pStage = &stages[i];

		if (!pStage->active)
			continue;

		if (pStage->type == ST_NORMALMAP)
		{
			pStage->active = qfalse;
		}

		if (pStage->type == ST_NORMALPARALLAXMAP)
		{
			pStage->active = qfalse;
		}

		if (pStage->type == ST_SPECULARMAP)
		{
			pStage->active = qfalse;
		}			
	}

	// remove inactive stages
	numStages = 0;
	for (i = 0; i < MAX_SHADER_STAGES; i++)
	{
		if (!stages[i].active)
			continue;

		if (i == numStages)
		{
			numStages++;
			continue;
		}

		stages[numStages] = stages[i];
		stages[i].active = qfalse;
		numStages++;
	}

#if 0
	// convert any remaining lightmap stages to a lighting pass with a white texture
	// only do this with r_sunlightMode non-zero, as it's only for correct shadows.
	if (r_sunlightMode->i) {
		for (i = 0; i < MAX_SHADER_STAGES; i++)
		{
			shaderStage_t *pStage = &stages[i];

			if (!pStage->active)
				continue;

			if (pStage->adjustColorsForFog)
				continue;

			if (pStage->bundle[TB_DIFFUSEMAP].tcGen == TCGEN_LIGHTMAP)
			{
				pStage->glslShaderGroup = rg.lightallShader;
				pStage->glslShaderIndex = LIGHTDEF_USE_LIGHTMAP;
				pStage->bundle[TB_LIGHTMAP] = pStage->bundle[TB_DIFFUSEMAP];
				pStage->bundle[TB_DIFFUSEMAP].image[0] = rg.whiteImage;
				pStage->bundle[TB_DIFFUSEMAP].isLightmap = qfalse;
				pStage->bundle[TB_DIFFUSEMAP].tcGen = TCGEN_TEXTURE;
			}
		}
	}
#endif

	// convert any remaining lightingdiffuse stages to a lighting pass
	#if 0
	if (shader.numDeforms == 0)
	{
		for (i = 0; i < MAX_SHADER_STAGES; i++)
		{
			shaderStage_t *pStage = &stages[i];

			if (!pStage->active)
				continue;

			if (pStage->adjustColorsForFog)
				continue;

			if (pStage->rgbGen == CGEN_LIGHTING_DIFFUSE)
			{
				pStage->glslShaderGroup = rg.lightallShader;
				pStage->glslShaderIndex = LIGHTDEF_USE_LIGHT_VECTOR;

				if (pStage->bundle[0].tcGen != TCGEN_TEXTURE || pStage->bundle[0].numTexMods != 0)
					pStage->glslShaderIndex |= LIGHTDEF_USE_TCGEN_AND_TCMOD;
			}
		}
	}
	#endif

	return numStages;
}
/*
=============

FixRenderCommandList
https://zerowing.idsoftware.com/bugzilla/show_bug.cgi?id=493
Arnout: this is a nasty issue. Shaders can be registered after drawsurfaces are generated
but before the frame is rendered. This will, for the duration of one frame, cause drawsurfaces
to be rendered with bad shaders. To fix this, need to go through all render commands and fix
sortedIndex.
==============
*/
static void FixRenderCommandList( int newShader ) {
	renderCommandList_t	*cmdList = &backendData->commandList;

	if( cmdList ) {
		const void *curCmd = cmdList->buffer;

		while ( 1 ) {
			curCmd = PADP(curCmd, sizeof(void *));

			switch ( *(const int *)curCmd ) {
			case RC_SET_COLOR:
				{
				const setColorCmd_t *sc_cmd = (const setColorCmd_t *)curCmd;
				curCmd = (const void *)(sc_cmd + 1);
				break;
				}
			case RC_DRAW_IMAGE:
				{
				const drawImageCmd_t *sp_cmd = (const drawImageCmd_t *)curCmd;
				curCmd = (const void *)(sp_cmd + 1);
				break;
				}
			case RC_DRAW_SURFS:
				{
				int i;
				drawSurf_t	*drawSurf;
				shader_t	*sh;
				int			sortedIndex;
				const drawSurfCmd_t *ds_cmd =  (const drawSurfCmd_t *)curCmd;

				R_SortDrawSurfs(ds_cmd->drawSurfs, ds_cmd->numDrawSurfs);
				for( i = 0, drawSurf = ds_cmd->drawSurfs; i < ds_cmd->numDrawSurfs; i++, drawSurf++ ) {
					sortedIndex = ((drawSurf->sort << 2) & (MAX_RENDER_SHADERS-1));
					if( sortedIndex >= newShader ) {
						sortedIndex = sh->sortedIndex;
						drawSurf->sort = R_GenDrawSurfSort(sh);
					}
				}
				curCmd = (const void *)(ds_cmd + 1);
				break;
				}
			case RC_DRAW_BUFFER:
				{
				const drawBufferCmd_t *db_cmd = (const drawBufferCmd_t *)curCmd;
				curCmd = (const void *)(db_cmd + 1);
				break;
				}
			case RC_SWAP_BUFFERS:
				{
				const swapBuffersCmd_t *sb_cmd = (const swapBuffersCmd_t *)curCmd;
				curCmd = (const void *)(sb_cmd + 1);
				break;
				}
			case RC_END_LIST:
			default:
				return;
			}
		}
	}
}

/*
==============
SortNewShader

Positions the most recently created shader in the rg.sortedShaders[]
array so that the shader->sort key is sorted relative to the other
shaders.

Sets shader->sortedIndex
==============
*/
static void SortNewShader( void ) {
	uint32_t sort;
	int32_t i;
	shader_t	*newShader;

	newShader = rg.shaders[ rg.numShaders - 1 ];
	sort = newShader->sort;

	for ( i = rg.numShaders - 2 ; i >= 0 ; i-- ) {
		if ( rg.sortedShaders[ i ]->sort <= sort ) {
			break;
		}
		rg.sortedShaders[i+1] = rg.sortedShaders[i];
		rg.sortedShaders[i+1]->sortedIndex++;
	}

	// Arnout: fix rendercommandlist
	// https://zerowing.idsoftware.com/bugzilla/show_bug.cgi?id=493
	FixRenderCommandList( i+1 );

	newShader->sortedIndex = i+1;
	rg.sortedShaders[i+1] = newShader;
}

static shader_t *GeneratePermanentShader(void)
{
	shader_t *newShader;
	uint32_t i, b;
	uint32_t hash, size;

	if (rg.numShaders >= MAX_RENDER_SHADERS) {
		ri.Printf(PRINT_INFO, COLOR_YELLOW "WARNING: GeneratePermanentShader - MAX_RENDER_SHADERS hit\n");
		return rg.defaultShader;
	}

	newShader = ri.Hunk_Alloc(sizeof(shader_t), h_low);

	*newShader = shader;

	rg.shaders[rg.numShaders] = newShader;
	newShader->index = rg.numShaders;

	rg.sortedShaders[rg.numShaders] = newShader;
	newShader->sortedIndex = rg.numShaders;

	rg.numShaders++;

	for (i = 0; i < MAX_SHADER_STAGES; i++) {
		if (!stages[i].active) {
			break;
		}

		newShader->stages[i] = ri.Hunk_Alloc(sizeof(stages[i]), h_low);
		*newShader->stages[i] = stages[i];

		for (b = 0; b < NUM_TEXTURE_BUNDLES; b++) {
			size = newShader->stages[i]->bundle[b].numTexMods * sizeof(texModInfo_t);
			if (size) {
				newShader->stages[i]->bundle[b].texMods = ri.Hunk_Alloc(size, h_low);
				memcpy(newShader->stages[i]->bundle[b].texMods, stages[i].bundle[b].texMods, size);
			}
		}
	}

	SortNewShader();

	hash = Com_GenerateHashValue(newShader->name, MAX_RENDER_SHADERS);
	newShader->next = hashTable[hash];
	hashTable[hash] = newShader;

	return newShader;
}

/*
FinishShader: allocates a new shader onto the hunk ready for use
*/
static shader_t *FinishShader(void)
{
	int stage;
	qboolean hasLightmapStage;

	hasLightmapStage = qfalse;

	if (shader.polygonOffset && shader.sort == SS_BAD) {
		shader.sort = SS_DECAL;
	}

	for (stage = 0; stage < MAX_SHADER_STAGES; ) {
		shaderStage_t *stageP = &stages[stage];

		if (!stageP->active) {
			break;
		}

		// check for a missing texture
		if (!stageP->bundle[0].image[0]) {
			ri.Printf(PRINT_INFO, COLOR_YELLOW "Shader %s has a stage with no image\n", shader.name);
			stageP->active = qfalse;
			stage++;
			continue;
		}

		//
		// default texture coordinate generation
		//
		if ( stageP->bundle[0].isLightmap ) {
			if ( stageP->bundle[0].tcGen == TCGEN_BAD ) {
				stageP->bundle[0].tcGen = TCGEN_LIGHTMAP;
			}
			hasLightmapStage = qtrue;
		} else {
			if ( stageP->bundle[0].tcGen == TCGEN_BAD ) {
				stageP->bundle[0].tcGen = TCGEN_TEXTURE;
			}
		}

		{
			// don't screw with sort order if this is a portal or environment
			if ( shader.sort == SS_BAD ) {
				// see through item, like a grill or grate
				if ( stageP->stateBits & GLS_DEPTHMASK_TRUE ) {
					shader.sort = SS_SEE_THROUGH;
				} else {
					shader.sort = SS_BLEND0;
				}
			}
		}
		
		stage++;
	}

	// there are times when you will need to manually apply a sort to
	// opaque alpha tested shaders that have later blend passes
	if ( shader.sort == SS_BAD ) {
		shader.sort = SS_OPAQUE;
	}

	stage = CollapseStagesToGLSL();

	// determine which vertex attributes this shader needs
	ComputeVertexAttribs();

	return GeneratePermanentShader();
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
static const char *FindShaderInShaderText( const char *shadername )
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
static void InitShader(const char *name, int32_t lightmapIndex)
{
	memset(&shader, 0, sizeof(shader));
	memset(&stages, 0, sizeof(stages));
	N_strncpyz(shader.name, name, sizeof(shader.name));
	shader.lightmapIndex = lightmapIndex;

	for (int i = 0; i < MAX_SHADER_STAGES; i++) {
		stages[i].bundle[0].texMods = texMods[i];

		// default normal/specular
		VectorSet4(stages[i].normalScale, 0.0f, 0.0f, 0.0f, 0.0f);
		{
			stages[i].specularScale[0] =
			stages[i].specularScale[1] =
			stages[i].specularScale[2] = r_baseSpecular->f;
			stages[i].specularScale[3] = r_baseGloss->f;
		}
	}
}


/*
==================
R_FindShaderByName

Will always return a valid shader, but it might be the
default shader if the real one can't be found.
==================
*/
shader_t *R_FindShaderByName( const char *name )
{
	char		strippedName[MAX_GDR_PATH];
	uint64_t	hash;
	shader_t	*sh;

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
shader_t *R_FindShader( const char *name, int32_t lightmapIndex, qboolean mipRawImage ) {
	char		strippedName[MAX_GDR_PATH];
	uint64_t	hash;
	const char	*shaderText;
	texture_t	*image;
	shader_t	*sh;

	if ( name[0] == '\0' ) {
		return rg.defaultShader;
	}

	// use (fullbright) vertex lighting if the bsp file doesn't have
	// lightmaps
	if ( lightmapIndex >= 0 && lightmapIndex >= (int32_t)rg.numLightmaps ) {
		lightmapIndex = LIGHTMAP_BY_VERTEX;
	} else if ( lightmapIndex < LIGHTMAP_2D ) {
		// negative lightmap indexes cause stray pointers (think rg.lightmaps[lightmapIndex])
		ri.Printf( PRINT_INFO, COLOR_YELLOW "WARNING: shader '%s' has invalid lightmap index of %d\n", name, lightmapIndex  );
		lightmapIndex = LIGHTMAP_BY_VERTEX;
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
		if ( (sh->lightmapSearchIndex == lightmapIndex || sh->defaultShader) &&	!N_stricmp(sh->name, strippedName)) {
			// match found
			return sh;
		}
	}

	InitShader( strippedName, lightmapIndex );

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

		if (mipRawImage)
		{
			flags |= IMGFLAG_MIPMAP | IMGFLAG_PICMIP;

			if (r_genNormalMaps->i)
				flags |= IMGFLAG_GENNORMALMAP;
		}
		else
		{
			flags |= IMGFLAG_CLAMPTOEDGE;
		}

		image = R_FindImageFile( name, IMGTYPE_COLORALPHA, flags );
		if ( !image ) {
			ri.Printf( PRINT_DEVELOPER, "Couldn't find image file for shader %s\n", name );
			shader.defaultShader = qtrue;
			return FinishShader();
		}
	}

	//
	// create the default shading commands
	//
	if ( shader.lightmapIndex == LIGHTMAP_NONE ) {
		// dynamic colors at vertexes
		stages[0].bundle[0].image[0] = image;
		stages[0].active = qtrue;
		stages[0].rgbGen = CGEN_LIGHTING_DIFFUSE;
		stages[0].stateBits = GLS_DEFAULT;
	} else if ( shader.lightmapIndex == LIGHTMAP_BY_VERTEX ) {
		// explicit colors at vertexes
		stages[0].bundle[0].image[0] = image;
		stages[0].active = qtrue;
		stages[0].rgbGen = CGEN_EXACT_VERTEX;
		stages[0].alphaGen = AGEN_SKIP;
		stages[0].stateBits = GLS_DEFAULT;
	} else if ( shader.lightmapIndex == LIGHTMAP_2D ) {
		// GUI elements
		stages[0].bundle[0].image[0] = image;
		stages[0].active = qtrue;
		stages[0].rgbGen = CGEN_VERTEX;
		stages[0].alphaGen = AGEN_VERTEX;
		stages[0].stateBits = GLS_DEPTHTEST_DISABLE |
			  GLS_SRCBLEND_SRC_ALPHA |
			  GLS_DSTBLEND_ONE_MINUS_SRC_ALPHA;
	} else if ( shader.lightmapIndex == LIGHTMAP_WHITEIMAGE ) {
		// fullbright level
		stages[0].bundle[0].image[0] = rg.whiteImage;
		stages[0].active = qtrue;
		stages[0].rgbGen = CGEN_IDENTITY_LIGHTING;
		stages[0].stateBits = GLS_DEFAULT;

		stages[1].bundle[0].image[0] = image;
		stages[1].active = qtrue;
		stages[1].rgbGen = CGEN_IDENTITY;
		stages[1].stateBits |= GLS_SRCBLEND_DST_COLOR | GLS_DSTBLEND_ZERO;
	} else {
		// two pass lightmap
		stages[0].bundle[0].image[0] = rg.lightmaps[shader.lightmapIndex];
		stages[0].bundle[0].isLightmap = qtrue;
		stages[0].active = qtrue;
		stages[0].rgbGen = CGEN_IDENTITY;	// lightmaps are scaled on creation
													// for identitylight
		stages[0].stateBits = GLS_DEFAULT;

		stages[1].bundle[0].image[0] = image;
		stages[1].active = qtrue;
		stages[1].rgbGen = CGEN_IDENTITY;
		stages[1].stateBits |= GLS_SRCBLEND_DST_COLOR | GLS_DSTBLEND_ZERO;
	}

	return FinishShader();
}

nhandle_t RE_RegisterShaderFromTexture(const char *name, texture_t *image, int32_t lightmapIndex, qboolean mipRawImage)
{
    uint64_t hash;
    shader_t *sh;

    hash = Com_GenerateHashValue(name, MAX_RENDER_SHADERS);

    // probably not necessary since this function
	// only gets called from tr_font.c with lightmapIndex == LIGHTMAP_2D
	// but better safe than sorry.
	if ( lightmapIndex >= (int32_t)rg.numLightmaps ) {
		lightmapIndex = LIGHTMAP_WHITEIMAGE;
	}

	//
	// see if the shader is already loaded
	//
	for (sh=hashTable[hash]; sh; sh=sh->next) {
		// NOTE: if there was no shader or image available with the name strippedName
		// then a default shader is created with lightmapIndex == LIGHTMAP_NONE, so we
		// have to check all default shaders otherwise for every call to R_FindShader
		// with that same strippedName a new default shader is created.
		if ( (sh->lightmapSearchIndex == lightmapIndex || sh->defaultShader) && !N_stricmp(sh->name, name)) {
			// match found
			return sh->index;
		}
	}

	InitShader( name, lightmapIndex );

	//
	// create the default shading commands
	//
	if ( shader.lightmapIndex == LIGHTMAP_NONE ) {
		// dynamic colors at vertexes
		stages[0].bundle[0].image[0] = image;
		stages[0].active = qtrue;
		stages[0].rgbGen = CGEN_LIGHTING_DIFFUSE;
		stages[0].stateBits = GLS_DEFAULT;
	} else if ( shader.lightmapIndex == LIGHTMAP_BY_VERTEX ) {
		// explicit colors at vertexes
		stages[0].bundle[0].image[0] = image;
		stages[0].active = qtrue;
		stages[0].rgbGen = CGEN_EXACT_VERTEX;
		stages[0].alphaGen = AGEN_SKIP;
		stages[0].stateBits = GLS_DEFAULT;
	} else if ( shader.lightmapIndex == LIGHTMAP_2D ) {
		// GUI elements
		stages[0].bundle[0].image[0] = image;
		stages[0].active = qtrue;
		stages[0].rgbGen = CGEN_VERTEX;
		stages[0].alphaGen = AGEN_VERTEX;
		stages[0].stateBits = GLS_DEPTHTEST_DISABLE |
			  GLS_SRCBLEND_SRC_ALPHA |
			  GLS_DSTBLEND_ONE_MINUS_SRC_ALPHA;
	} else if ( shader.lightmapIndex == LIGHTMAP_WHITEIMAGE ) {
		// fullbright level
		stages[0].bundle[0].image[0] = rg.whiteImage;
		stages[0].active = qtrue;
		stages[0].rgbGen = CGEN_IDENTITY_LIGHTING;
		stages[0].stateBits = GLS_DEFAULT;

		stages[1].bundle[0].image[0] = image;
		stages[1].active = qtrue;
		stages[1].rgbGen = CGEN_IDENTITY;
		stages[1].stateBits |= GLS_SRCBLEND_DST_COLOR | GLS_DSTBLEND_ZERO;
	} else {
		// two pass lightmap
		stages[0].bundle[0].image[0] = rg.lightmaps[shader.lightmapIndex];
		stages[0].bundle[0].isLightmap = qtrue;
		stages[0].active = qtrue;
		stages[0].rgbGen = CGEN_IDENTITY;	// lightmaps are scaled on creation
													// for identitylight
		stages[0].stateBits = GLS_DEFAULT;

		stages[1].bundle[0].image[0] = image;
		stages[1].active = qtrue;
		stages[1].rgbGen = CGEN_IDENTITY;
		stages[1].stateBits |= GLS_SRCBLEND_DST_COLOR | GLS_DSTBLEND_ZERO;
	}

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
nhandle_t RE_RegisterShaderLightMap( const char *name, int32_t lightmapIndex ) {
	shader_t	*sh;

	if ( strlen( name ) >= MAX_GDR_PATH ) {
		ri.Printf( PRINT_INFO, "Shader name exceeds MAX_GDR_PATH\n" );
		return 0;
	}

	sh = R_FindShader( name, lightmapIndex, qtrue );

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
nhandle_t RE_RegisterShader( const char *name ) {
	shader_t	*sh;

	if ( !name ) {
		ri.Printf( PRINT_INFO, "NULL shader\n" );
		return 0;
	}

	if ( strlen( name ) >= MAX_GDR_PATH ) {
		ri.Printf( PRINT_INFO, "Shader name exceeds MAX_GDR_PATH\n" );
		return 0;
	}

	sh = R_FindShader( name, LIGHTMAP_2D, qfalse );

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
it and returns a valid (possibly default) shader_t to be used internally.
====================
*/
shader_t *R_GetShaderByHandle( nhandle_t hShader ) {
	if ( hShader < 0 ) {
	    ri.Printf( PRINT_INFO, COLOR_YELLOW "WARNING: R_GetShaderByHandle: out of range hShader '%d'\n", hShader );
		return rg.defaultShader;
	}
	if ( hShader >= rg.numShaders ) {
		ri.Printf( PRINT_INFO, COLOR_YELLOW "WARNING: R_GetShaderByHandle: out of range hShader '%d'\n", hShader );
		return rg.defaultShader;
	}
	return rg.shaders[hShader];
}


#define	MAX_SHADER_FILES 16384

static int loadShaderBuffers( char **shaderFiles, const uint64_t numShaderFiles, char **buffers )
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
				ri.Printf(PRINT_INFO, COLOR_YELLOW "WARNING: Ignoring shader file %s. Shader \"%s\" " \
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
static void ScanAndLoadShaderFiles( void )
{
	char **shaderFiles;
	char *buffers[MAX_SHADER_FILES];
	uint64_t numShaderFiles;
	int64_t i;
	const char *tok, *hashMem;
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
		ri.Printf( PRINT_INFO, COLOR_YELLOW "WARNING: no shader files found\n" );
		return;
	}

	if ( numShaderFiles > MAX_SHADER_FILES ) {
		numShaderFiles = MAX_SHADER_FILES;
	}

	sum = 0;
	sum += loadShaderBuffers( shaderFiles, numShaderFiles, buffers );

	// build single large buffer
	r_shaderText = ri.Hunk_Alloc( sum + numShaderFiles*2 + 1, h_low );
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
	if ( shaderFiles )
		ri.FS_FreeFileList( shaderFiles );

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

	hashMem = ri.Hunk_Alloc( size * sizeof(char *), h_low );

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
static void CreateInternalShaders( void )
{
	rg.numShaders = 0;

	// init the default shader
	InitShader( "<default>", LIGHTMAP_NONE );
	stages[0].bundle[0].image[0] = rg.defaultImage;
	stages[0].active = qtrue;
	stages[0].stateBits = GLS_DEFAULT;
	rg.defaultShader = FinishShader();
}

/*
==================
R_InitShaders
==================
*/
void R_InitShaders( void )
{
	ri.Printf( PRINT_INFO, "Initializing Shaders\n" );

    memset(hashTable, 0, sizeof(hashTable));

	CreateInternalShaders();

	ScanAndLoadShaderFiles();
}

