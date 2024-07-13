#include "rgl_local.h"

// rgl_shader.c -- this file deals with the parsing and defintion of shaders

static char *r_shaderText;
static const char *r_extensionOffset;
static uint64_t r_extendedShader;

static shader_t shader;
static shaderStage_t stages[MAX_SHADER_STAGES];
static	texModInfo_t texMods[MAX_SHADER_STAGES][TR_MAX_TEXMODS];

static shader_t *hashTable[MAX_RENDER_SHADERS];

#define MAX_SHADERTEXT_HASH 2048
static const char **shaderTextHashTable[MAX_SHADERTEXT_HASH];

/*
=================
ParseSort
=================
*/
static void ParseSort( const char **text ) {
	const char	*token;

	token = COM_ParseExt( text, qfalse );
	if ( token[0] == 0 ) {
		ri.Printf( PRINT_WARNING, "WARNING: missing sort parameter in shader '%s'\n", shader.name );
		return;
	}

	if ( !N_stricmp( token, "portal" ) ) {
//		shader.sort = SS_PORTAL;
	} else if ( !N_stricmp( token, "sky" ) ) {
//		shader.sort = SS_ENVIRONMENT;
	} else if ( !N_stricmp( token, "opaque" ) ) {
		shader.sort = SS_OPAQUE;
	} else if ( !N_stricmp( token, "decal" ) ) {
		shader.sort = SS_DECAL;
	} else if ( !N_stricmp( token, "seeThrough" ) ) {
		shader.sort = SS_SEE_THROUGH;
	} else if ( !N_stricmp( token, "banner" ) ) {
//		shader.sort = SS_BANNER;
	} else if ( !N_stricmp( token, "additive" ) ) {
//		shader.sort = SS_BLEND1;
	} else if ( !N_stricmp( token, "nearest" ) ) {
//		shader.sort = SS_NEAREST;
	} else if ( !N_stricmp( token, "underwater" ) ) {
//		shader.sort = SS_UNDERWATER;
	} else {
		shader.sort = N_atof( token );
	}
}


// this table is also present in q3map

typedef struct {
	const char *name;
	int clearSolid, surfaceFlags, contents;
} infoParm_t;

static const infoParm_t infoParms[] = {
	// server relevant contents
	{ "water",		1,	0,	CONTENTS_WATER },
	{ "slime",		1,	0,	CONTENTS_SLIME },			// mildly damaging
	{ "lava",		1,	0,	CONTENTS_LAVA },			// very damaging
	{ "playerclip",	1,	0,	CONTENTS_PLAYERCLIP },
	{ "monsterclip", 1,	0,	CONTENTS_MONSTERCLIP },
	{ "nodrop",		1,	0,	CONTENTS_NODROP },			// don't drop items or leave bodies (death fog, lava, etc)
	{ "nonsolid",	1,	SURFACEPARM_NONSOLID, 0 },		// clears the solid flag


	// utility relevant attributes
	{ "trans",		0,	0,	CONTENTS_TRANSLUCENT },		// don't eat contained surfaces

	{ "lightfilter",	0,	SURFACEPARM_LIGHTFILTER, 0 },	// filter light going through it
	{ "alphashadow",	0,	SURFACEPARM_ALPHASHADOW, 0 },	// test light on a per-pixel basis

	// server attributes
	{ "slick",		0,	SURFACEPARM_SLICK,		0 },
	{ "noimpact",	0,	SURFACEPARM_NOMISSILE,	0 },	// don't make impact explosions or marks
	{ "nomarks",	0,	SURFACEPARM_NOMARKS,	0 },	// don't make impact marks, but still explode
	{ "ladder",		0,	SURFACEPARM_LADDER,		0 },
	{ "nodamage",	0,	SURFACEPARM_NODAMAGE,	0 },
	{ "metalsteps",	0,	SURFACEPARM_METAL,		0 },
	{ "flesh",		0,	SURFACEPARM_FLESH,		0 },
	{ "nosteps",	0,	SURFACEPARM_NOSTEPS,	0 },

	// drawsurf attributes
	{ "nodraw",		0,	SURFACEPARM_NODRAW,		0 },	// don't generate a drawsurface (or a lightmap)
	{ "pointlight",	0,	SURFACEPARM_POINTLIGHT, 0 },	// sample lighting at vertexes
	{ "nolightmap",	0,	SURFACEPARM_NOLIGHTMAP,	0 },	// don't generate a lightmap
	{ "nodlight",	0,	SURFACEPARM_NODLIGHT,	0 },	// don't ever add dynamic lights
	{ "dust",		0,	SURFACEPARM_DUST, 		0 }		// leave a dust trail when walking on this surface
};


/*
===============
ParseSurfaceParm

surfaceparm <name>
===============
*/
static void ParseSurfaceParm( const char **text ) {
	const char	*token;
	int		numInfoParms = arraylen( infoParms );
	int		i;

	token = COM_ParseExt( text, qfalse );
	for ( i = 0 ; i < numInfoParms ; i++ ) {
		if ( !N_stricmp( token, infoParms[i].name ) ) {
			shader.surfaceFlags |= infoParms[i].surfaceFlags;
			shader.contentFlags |= infoParms[i].contents;
#if 0
			if ( infoParms[i].clearSolid ) {
				si->contents &= ~CONTENTS_SOLID;
			}
#endif
			break;
		}
	}
}


typedef enum {
	res_invalid = -1,
	res_false = 0,
	res_true = 1
} resultType;

typedef enum {
	brIF,
	brELIF,
	brELSE
} branchType;

typedef enum {
	maskOR,
	maskAND
} resultMask;


static void derefVariable( const char *name, char *buf, int size )
{
	if ( !N_stricmp( name, "vid_width" ) ) {
		Com_snprintf( buf, size, "%i", glConfig.vidWidth );
		return;
	}
	if ( !N_stricmp( name, "vid_height" ) ) {
		Com_snprintf( buf, size, "%i", glConfig.vidHeight );
		return;
	}
	ri.Cvar_VariableStringBuffer( name, buf, size );
}


/*
===============
ParseCondition

if ( $cvar|<integer value> [<condition> $cvar|<integer value> [ [ || .. ] && .. ] ] )
{ shader stage }
[ else
{ shader stage } ]
===============
*/
static qboolean ParseCondition( const char **text, resultType *res )
{
	char lval_str[ MAX_CVAR_VALUE ];
	char rval_str[ MAX_CVAR_VALUE ];
	tokenType_t lval_type;
	tokenType_t rval_type;
	const char *token;
	tokenType_t op;
	resultMask	rm;
	qboolean	str;
	int r, r0;

	r = 0;			// resulting value
	rm = maskOR;	// default mask

	for ( ;; ) {
		rval_str[0] = '\0';
		rval_type = TK_GENEGIC;

		// expect l-value at least
		token = COM_ParseComplex( text, qfalse );
		if ( token[0] == '\0' ) {
			ri.Printf( PRINT_WARNING, "WARNING: expecting lvalue for condition in shader %s\n", shader.name );
			return qfalse;
		}
	
		N_strncpyz( lval_str, token, sizeof( lval_str ) );
		lval_type = com_tokentype;

		// get operator
		token = COM_ParseComplex( text, qfalse );
		if ( com_tokentype >= TK_EQ && com_tokentype <= TK_LTE ) {
			op = com_tokentype;

			// expect r-value
			token = COM_ParseComplex( text, qfalse );
			if ( token[0] == '\0' ) {
				ri.Printf( PRINT_WARNING, "WARNING: expecting rvalue for condition in shader %s\n", shader.name );
				return qfalse;
			}

			N_strncpyz( rval_str, token, sizeof( rval_str ) );
			rval_type = com_tokentype;

			// read next token, expect '||', '&&' or ')', allow newlines
			/*token =*/ COM_ParseComplex( text, qtrue );
		} 
		else if ( com_tokentype == TK_SCOPE_CLOSE || com_tokentype == TK_OR || com_tokentype == TK_AND ) {
			// no r-value, assume 'not zero' comparison
			op = TK_NEQ;
		}
		else {
			ri.Printf( PRINT_WARNING, "WARNING: unexpected operator '%s' for comparison in shader %s\n", token, shader.name );
			return qfalse;
		}
		
		str = qfalse;

		if ( lval_type == TK_QUOTED ) {
			str = qtrue;
		} else {
			// dereference l-value
			if ( lval_str[0] == '$' ) {
				derefVariable( lval_str + 1, lval_str, sizeof( lval_str ) );
			}
		}

		if ( rval_type == TK_QUOTED ) {
			str = qtrue;
		} else {
			// dereference r-value
			if ( rval_str[0] == '$' ) {
				derefVariable( rval_str + 1, rval_str, sizeof( rval_str ) );
			}
		}

		// evaluate expression
		if ( str ) {
			// string comparison
			switch ( op ) {
			case TK_EQ:  r0 = strcmp( lval_str, rval_str ) == 0; break;
			case TK_NEQ: r0 = strcmp( lval_str, rval_str ) != 0; break;
			case TK_GT:  r0 = strcmp( lval_str, rval_str ) >  0; break;
			case TK_GTE: r0 = strcmp( lval_str, rval_str ) >= 0; break;
			case TK_LT:  r0 = strcmp( lval_str, rval_str ) <  0; break;
			case TK_LTE: r0 = strcmp( lval_str, rval_str ) <= 0; break;
			default:     r0 = 0; break;
			};
		} else {
			// integer comparison
			int lval = atoi( lval_str );
			int rval = atoi( rval_str );
			switch ( op ) {
			case TK_EQ:  r0 = ( lval == rval ); break;
			case TK_NEQ: r0 = ( lval != rval ); break;
			case TK_GT:  r0 = ( lval >  rval ); break;
			case TK_GTE: r0 = ( lval >= rval ); break;
			case TK_LT:  r0 = ( lval <  rval ); break;
			case TK_LTE: r0 = ( lval <= rval ); break;
			default:     r0 = 0; break;
			};
		}

		if ( rm == maskOR ) {
			r |= r0;
		} else {
			r &= r0;
		}
			
		if ( com_tokentype == TK_OR ) {
			rm = maskOR;
			continue;
		}

		if ( com_tokentype == TK_AND ) {
			rm = maskAND;
			continue;
		}

		if ( com_tokentype != TK_SCOPE_CLOSE ) {
			ri.Printf( PRINT_WARNING, "WARNING: expecting ')' in shader %s\n", shader.name );
			return qfalse;
		}

		break;
	}

	if ( res ) {
		*res = r ? res_true : res_false;
	}
	
	return qtrue;
}

/*
===============
NameToAFunc
===============
*/
static unsigned NameToAFunc( const char *funcname )
{	
	if ( !N_stricmp( funcname, "GT0" ) ) {
		return GLS_ATEST_GT_0;
	} else if ( !N_stricmp( funcname, "LT128" ) ) {
		return GLS_ATEST_LT_80;
	} else if ( !N_stricmp( funcname, "GE128" ) ) {
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

	ri.Printf( PRINT_WARNING, "unknown blend mode '%s' in shader '%s', substituting GL_ONE\n", name, shader.name );
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

	ri.Printf( PRINT_WARNING, "unknown blend mode '%s' in shader '%s', substituting GL_ONE\n", name, shader.name );
	return GLS_DSTBLEND_ONE;
}

/*
===============
NameToGenFunc
===============
*/
static genFunc_t NameToGenFunc( const char *funcname )
{
	if ( !N_stricmp( funcname, "sin" ) ) {
		return GF_SIN;
	}
	else if ( !N_stricmp( funcname, "square" ) ) {
		return GF_SQUARE;
	}
	else if ( !N_stricmp( funcname, "triangle" ) ) {
		return GF_TRIANGLE;
	}
	else if ( !N_stricmp( funcname, "sawtooth" ) ) {
		return GF_SAWTOOTH;
	}
	else if ( !N_stricmp( funcname, "inversesawtooth" ) ) {
		return GF_INVERSE_SAWTOOTH;
	}
	else if ( !N_stricmp( funcname, "noise" ) ) {
		return GF_NOISE;
	}

	ri.Printf( PRINT_WARNING, "WARNING: invalid genfunc name '%s' in shader '%s'\n", funcname, shader.name );
	return GF_SIN;
}

/*
===============
ParseVector
===============
*/
static qboolean ParseVector( const char **text, int count, float *v ) {
	const char	*tok;
	int		i;

	// FIXME: spaces are currently required after parens, should change parseext...
	tok = COM_ParseExt( text, qfalse );
	if ( strcmp( tok, "(" ) ) {
		ri.Printf( PRINT_WARNING, "missing parenthesis in shader '%s'\n", shader.name );
		return qfalse;
	}

	for ( i = 0 ; i < count ; i++ ) {
		tok = COM_ParseExt( text, qfalse );
		if ( !tok[0] ) {
			ri.Printf( PRINT_WARNING, "missing vector element in shader '%s'\n", shader.name );
			return qfalse;
		}
		v[i] = N_atof( tok );
	}

	tok = COM_ParseExt( text, qfalse );
	if ( strcmp( tok, ")" ) ) {
		ri.Printf( PRINT_WARNING, "missing parenthesis in shader '%s'\n", shader.name );
		return qfalse;
	}

	return qtrue;
}


/*
===================
ParseWaveForm
===================
*/
static void ParseWaveForm( const char **text, waveForm_t *wave )
{
	const char *tok;

	tok = COM_ParseExt( text, qfalse );
	if ( tok[0] == 0 )
	{
		ri.Printf( PRINT_WARNING, "WARNING: missing waveform parm in shader '%s'\n", shader.name );
		return;
	}
	wave->func = NameToGenFunc( tok );

	// BASE, AMP, PHASE, FREQ
	tok = COM_ParseExt( text, qfalse );
	if ( tok[0] == 0 )
	{
		ri.Printf( PRINT_WARNING, "WARNING: missing waveform parm in shader '%s'\n", shader.name );
		return;
	}
	wave->base = N_atof( tok );

	tok = COM_ParseExt( text, qfalse );
	if ( tok[0] == 0 )
	{
		ri.Printf( PRINT_WARNING, "WARNING: missing waveform parm in shader '%s'\n", shader.name );
		return;
	}
	wave->amplitude = N_atof( tok );

	tok = COM_ParseExt( text, qfalse );
	if ( tok[0] == 0 )
	{
		ri.Printf( PRINT_WARNING, "WARNING: missing waveform parm in shader '%s'\n", shader.name );
		return;
	}
	wave->phase = N_atof( tok );

	tok = COM_ParseExt( text, qfalse );
	if ( tok[0] == 0 )
	{
		ri.Printf( PRINT_WARNING, "WARNING: missing waveform parm in shader '%s'\n", shader.name );
		return;
	}
	wave->frequency = N_atof( tok );
}


/*
===================
ParseTexMod
===================
*/
static void ParseTexMod( const char *_text, shaderStage_t *stage )
{
	const char *tok;
	const char **text = &_text;
	texModInfo_t *tmi;

	if ( stage->bundle[0].numTexMods == TR_MAX_TEXMODS ) {
		ri.Error( ERR_DROP, "ERROR: too many tcMod stages in shader '%s'", shader.name );
		return;
	}

	tmi = &stage->bundle[0].texMods[stage->bundle[0].numTexMods];
	stage->bundle[0].numTexMods++;

	tok = COM_ParseExt( text, qfalse );

	//
	// turb
	//
	if ( !N_stricmp( tok, "turb" ) ) {
		tok = COM_ParseExt( text, qfalse );
		if ( tok[0] == 0 ) {
			ri.Printf( PRINT_WARNING, "WARNING: missing tcMod turb parms in shader '%s'\n", shader.name );
			return;
		}
		tmi->wave.base = N_atof( tok );
		tok = COM_ParseExt( text, qfalse );
		if ( tok[0] == 0 ) {
			ri.Printf( PRINT_WARNING, "WARNING: missing tcMod turb in shader '%s'\n", shader.name );
			return;
		}
		tmi->wave.amplitude = N_atof( tok );
		tok = COM_ParseExt( text, qfalse );
		if ( tok[0] == 0 ) {
			ri.Printf( PRINT_WARNING, "WARNING: missing tcMod turb in shader '%s'\n", shader.name );
			return;
		}
		tmi->wave.phase = N_atof( tok );
		tok = COM_ParseExt( text, qfalse );
		if ( tok[0] == 0 ) {
			ri.Printf( PRINT_WARNING, "WARNING: missing tcMod turb in shader '%s'\n", shader.name );
			return;
		}
		tmi->wave.frequency = N_atof( tok );

		tmi->type = TMOD_TURBULENT;
	}
	//
	// scale
	//
	else if ( !N_stricmp( tok, "scale" ) ) {
		tok = COM_ParseExt( text, qfalse );
		if ( tok[0] == 0 ) {
			ri.Printf( PRINT_WARNING, "WARNING: missing scale parms in shader '%s'\n", shader.name );
			return;
		}
		tmi->scale[0] = N_atof( tok );

		tok = COM_ParseExt( text, qfalse );
		if ( tok[0] == 0 ) {
			ri.Printf( PRINT_WARNING, "WARNING: missing scale parms in shader '%s'\n", shader.name );
			return;
		}
		tmi->scale[1] = N_atof( tok );
		tmi->type = TMOD_SCALE;
	}
	//
	// scroll
	//
	else if ( !N_stricmp( tok, "scroll" ) ) {
		tok = COM_ParseExt( text, qfalse );
		if ( tok[0] == 0 ) {
			ri.Printf( PRINT_WARNING, "WARNING: missing scale scroll parms in shader '%s'\n", shader.name );
			return;
		}
		tmi->scroll[0] = N_atof( tok );
		tok = COM_ParseExt( text, qfalse );
		if ( tok[0] == 0 ) {
			ri.Printf( PRINT_WARNING, "WARNING: missing scale scroll parms in shader '%s'\n", shader.name );
			return;
		}
		tmi->scroll[1] = N_atof( tok );
		tmi->type = TMOD_SCROLL;
	}
	//
	// stretch
	//
	else if ( !N_stricmp( tok, "stretch" ) ) {
		tok = COM_ParseExt( text, qfalse );
		if ( tok[0] == 0 ) {
			ri.Printf( PRINT_WARNING, "WARNING: missing stretch parms in shader '%s'\n", shader.name );
			return;
		}
		tmi->wave.func = NameToGenFunc( tok );

		tok = COM_ParseExt( text, qfalse );
		if ( tok[0] == 0 ) {
			ri.Printf( PRINT_WARNING, "WARNING: missing stretch parms in shader '%s'\n", shader.name );
			return;
		}
		tmi->wave.base = N_atof( tok );

		tok = COM_ParseExt( text, qfalse );
		if ( tok[0] == 0 ) {
			ri.Printf( PRINT_WARNING, "WARNING: missing stretch parms in shader '%s'\n", shader.name );
			return;
		}
		tmi->wave.amplitude = N_atof( tok );

		tok = COM_ParseExt( text, qfalse );
		if ( tok[0] == 0 ) {
			ri.Printf( PRINT_WARNING, "WARNING: missing stretch parms in shader '%s'\n", shader.name );
			return;
		}
		tmi->wave.phase = N_atof( tok );

		tok = COM_ParseExt( text, qfalse );
		if ( tok[0] == 0 ) {
			ri.Printf( PRINT_WARNING, "WARNING: missing stretch parms in shader '%s'\n", shader.name );
			return;
		}
		tmi->wave.frequency = N_atof( tok );

		tmi->type = TMOD_STRETCH;
	}
	//
	// transform
	//
	else if ( !N_stricmp( tok, "transform" ) ) {
		tok = COM_ParseExt( text, qfalse );
		if ( tok[0] == 0 ) {
			ri.Printf( PRINT_WARNING, "WARNING: missing transform parms in shader '%s'\n", shader.name );
			return;
		}
		tmi->matrix[0][0] = N_atof( tok );

		tok = COM_ParseExt( text, qfalse );
		if ( tok[0] == 0 ) {
			ri.Printf( PRINT_WARNING, "WARNING: missing transform parms in shader '%s'\n", shader.name );
			return;
		}
		tmi->matrix[0][1] = N_atof( tok );

		tok = COM_ParseExt( text, qfalse );
		if ( tok[0] == 0 ) {
			ri.Printf( PRINT_WARNING, "WARNING: missing transform parms in shader '%s'\n", shader.name );
			return;
		}
		tmi->matrix[1][0] = N_atof( tok );

		tok = COM_ParseExt( text, qfalse );
		if ( tok[0] == 0 ) {
			ri.Printf( PRINT_WARNING, "WARNING: missing transform parms in shader '%s'\n", shader.name );
			return;
		}
		tmi->matrix[1][1] = N_atof( tok );

		tok = COM_ParseExt( text, qfalse );
		if ( tok[0] == 0 ) {
			ri.Printf( PRINT_WARNING, "WARNING: missing transform parms in shader '%s'\n", shader.name );
			return;
		}
		tmi->translate[0] = N_atof( tok );

		tok = COM_ParseExt( text, qfalse );
		if ( tok[0] == 0 ) {
			ri.Printf( PRINT_WARNING, "WARNING: missing transform parms in shader '%s'\n", shader.name );
			return;
		}
		tmi->translate[1] = N_atof( tok );

		tmi->type = TMOD_TRANSFORM;
	}
	//
	// rotate
	//
	else if ( !N_stricmp( tok, "rotate" ) ) {
		tok = COM_ParseExt( text, qfalse );
		if ( tok[0] == 0 ) {
			ri.Printf( PRINT_WARNING, "WARNING: missing tcMod rotate parms in shader '%s'\n", shader.name );
			return;
		}
		tmi->rotateSpeed = N_atof( tok );
		tmi->type = TMOD_ROTATE;
	}
	//
	// entityTranslate
	//
	else if ( !N_stricmp( tok, "entityTranslate" ) ) {
		tmi->type = TMOD_ENTITY_TRANSLATE;
	}
	else {
		ri.Printf( PRINT_WARNING, "WARNING: unknown tcMod '%s' in shader '%s'\n", tok, shader.name );
	}
}


static qboolean ParseStage(shaderStage_t *stage, const char **text)
{
    const char *tok;
    uint32_t depthMaskBits = GLS_DEPTHMASK_TRUE, blendSrcBits = 0, blendDstBits = 0, atestBits = 0, depthFuncBits = 0;
	qboolean depthMaskExplicit = qfalse;

	stage->active = qtrue;
	stage->bundle[0].filter = (textureFilter_t)-1;

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
        else if ( !N_stricmp( tok, "map" ) ) {
            tok = COM_ParseExt( text, qfalse );
			if (!tok[0]) {
				ri.Printf(PRINT_WARNING, "missing parameter for 'map' keyword in shader '%s'\n", shader.name);
				return qfalse;
			}

			if ( !N_stricmp( tok, "$whiteimage" ) ) {
				stage->bundle[0].image[0] = rg.whiteImage;
				continue;
			}
			else if ( !N_stricmp( tok, "$lightmap" ) ) {
				stage->bundle[0].isLightmap = qtrue;
				stage->bundle[0].image[0] = rg.whiteImage;
			}
			else {
				imgType_t type = IMGTYPE_COLORALPHA;
				imgFlags_t flags = IMGFLAG_NONE;

				if ( !shader.noMipMaps ) {
					flags |= IMGFLAG_MIPMAP;
				}
				if ( !shader.noPicMip ) {
					flags |= IMGFLAG_PICMIP;
				}
				if ( shader.noLightScale ) {
					flags |= IMGFLAG_NOLIGHTSCALE;
				}
                
                stage->bundle[0].image[0] = R_FindImageFile( tok, type, flags );

				if ( !stage->bundle[0].image[0] ) {
					ri.Printf( PRINT_WARNING, "R_FindImageFile could not find '%s' in shader '%s'\n", tok, shader.name );
					return qfalse;
				}
			}
        }
		//
		// clampmap <name>
		//
		else if ( !N_stricmp( tok, "clampmap" ) ) {
			imgType_t type = IMGTYPE_COLORALPHA;
			imgFlags_t flags = IMGFLAG_CLAMPTOEDGE;

			tok = COM_ParseExt( text, qfalse );
			if ( !tok[0] ) {
				ri.Printf( PRINT_WARNING, "missing parameter for 'clampmap' keyword in shader '%s'\n", shader.name );
				return qfalse;
			}

			if ( !shader.noMipMaps ) {
				flags |= IMGFLAG_MIPMAP;
			}

			if ( !shader.noPicMip ) {
				flags |= IMGFLAG_PICMIP;
			}

			if ( stage->type == ST_NORMALMAP || stage->type == ST_NORMALPARALLAXMAP ) {
				type = IMGTYPE_NORMAL;
				flags |= IMGFLAG_NOLIGHTSCALE;

				if ( stage->type == ST_NORMALPARALLAXMAP ) {
					type = IMGTYPE_NORMALHEIGHT;
				}
			}
			else {
				if ( r_genNormalMaps->i ) {
					flags |= IMGFLAG_GENNORMALMAP;
				}
			}


			stage->bundle[0].image[0] = R_FindImageFile( tok, type, flags );
			if ( !stage->bundle[0].image[0] ) {
				ri.Printf( PRINT_WARNING, "WARNING: R_FindImageFile could not find '%s' in shader '%s'\n", tok, shader.name );
				return qfalse;
			}
		}
		//
		// animMap <frequency> <image1> ... <imageN>
		//
		else if ( !N_stricmp( tok, "animMap" ) ) {
			int totalImages = 0;
			int maxAnimations = MAX_IMAGE_ANIMATIONS;

			tok = COM_ParseExt( text, qfalse );
			if ( !tok[0] ) {
				ri.Printf( PRINT_WARNING, "WARNING: missing paramter for 'animMap' keyword in shader '%s'\n", shader.name) ;
				return qfalse;
			}
			stage->bundle[0].imageAnimationSpeed = N_atof( tok );

			// parse up to MAX_IMAGE_ANIMATIONS animations
			while ( 1 ) {
				int		num;

				tok = COM_ParseExt( text, qfalse );
				if ( !tok[0] ) {
					break;
				}
				num = stage->bundle[0].numImageAnimations;
				if ( num < maxAnimations ) {
					imgFlags_t flags = IMGFLAG_NONE;

					if ( !shader.noMipMaps ) {
						flags |= IMGFLAG_MIPMAP;
					}

					if ( !shader.noPicMip ) {
						flags |= IMGFLAG_PICMIP;
					}

					stage->bundle[0].image[num] = R_FindImageFile( tok, IMGTYPE_COLORALPHA, flags );
					if ( !stage->bundle[0].image[num] ) {
						ri.Printf( PRINT_WARNING, "WARNING: R_FindImageFile could not find '%s' in shader '%s'\n", tok, shader.name );
						return qfalse;
					}
					stage->bundle[0].numImageAnimations++;
				}
				totalImages++;
			}

			if ( totalImages > maxAnimations ) {
				ri.Printf( PRINT_WARNING, "WARNING: ignoring excess images for 'animMap' (found %d, max is %d) in shader '%s'\n",
					totalImages, maxAnimations, shader.name );
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
		// specularScale <rgb> <gloss>
		// or specularScale <metallic> <smoothness> with r_pbr 1
		// or specularScale <r> <g> <b>
		// or specularScale <r> <g> <b> <gloss>
		//
		else if ( !N_stricmp( tok, "specularscale" ) ) {
			tok = COM_ParseExt( text, qfalse );
			if ( tok[0] == 0 ) {
				ri.Printf( PRINT_WARNING, "WARNING: missing parameter for specularScale in shader '%s'\n", shader.name );
				continue;
			}

			stage->specularScale[0] = atof( tok );

			tok = COM_ParseExt( text, qfalse );
			if ( tok[0] == 0 ) {
				ri.Printf( PRINT_WARNING, "WARNING: missing parameter for specularScale in shader '%s'\n", shader.name );
				continue;
			}

			stage->specularScale[1] = atof( tok );

			tok = COM_ParseExt( text, qfalse );
			if ( tok[0] == 0 ) {
				if ( r_pbr->i ) {
					// two values, metallic then smoothness
					float smoothness = stage->specularScale[1];
					stage->specularScale[1] = ( stage->specularScale[0] < 0.5f ) ? 0.0f : 1.0f;
					stage->specularScale[0] = smoothness;
				}
				else {
					// two values, rgb then gloss
					stage->specularScale[3] = stage->specularScale[1];
					stage->specularScale[1] =
					stage->specularScale[2] = stage->specularScale[0];
				}
				continue;
			}

			stage->specularScale[2] = atof( tok );

			tok = COM_ParseExt( text, qfalse );
			if ( tok[0] == 0 ) {
				// three values, rgb
				continue;
			}

			stage->specularScale[3] = atof( tok );

		}
		//
		// tcGen <function>
		//
		else if ( !N_stricmp( tok, "texgen" ) || !N_stricmp( tok, "tcGen" ) ) {
			tok = COM_ParseExt( text, qfalse );
			if ( tok[0] == 0 ) {
				ri.Printf( PRINT_WARNING, "WARNING: missing texgen parm in shader '%s'\n", shader.name );
				continue;
			}
		
			if ( !N_stricmp( tok, "environment" ) ) {
				const char *t = *text;
				stage->bundle[0].tcGen = TCGEN_ENVIRONMENT_MAPPED;
				tok = COM_ParseExt( text, qfalse );
				if ( N_stricmp( tok, "firstPerson" ) == 0 ) {
					//stage->bundle[0].tcGen = TCGEN_ENVIRONMENT_MAPPED_FP;
				}
				else
				{
					*text = t; // rewind
				}
			}
			else if ( !N_stricmp( tok, "lightmap" ) ) {
				stage->bundle[0].tcGen = TCGEN_LIGHTMAP;
			}
			else if ( !N_stricmp( tok, "texture" ) || !N_stricmp( tok, "base" ) ) {
				stage->bundle[0].tcGen = TCGEN_TEXTURE;
			}
			else if ( !N_stricmp( tok, "vector" ) ) {
				ParseVector( text, 3, stage->bundle[0].tcGenVectors[0] );
				ParseVector( text, 3, stage->bundle[0].tcGenVectors[1] );

				stage->bundle[0].tcGen = TCGEN_VECTOR;
			}
			else {
				ri.Printf( PRINT_WARNING, "WARNING: unknown texgen parm in shader '%s'\n", shader.name );
			}
		}
		//
		// tcMod <type> <...>
		//
		else if ( !N_stricmp( tok, "tcMod" ) ) {
			char buffer[1024] = "";

			while ( 1 ) {
				tok = COM_ParseExt( text, qfalse );
				if ( tok[0] == 0 ) {
					break;
				}
				N_strcat( buffer, sizeof (buffer), tok );
				N_strcat( buffer, sizeof (buffer), " " );
			}

			ParseTexMod( buffer, stage );

			continue;
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
		// texFilter <bilinear|nearest|linear_nearest|nearest_linear|default>
		//
		else if ( !N_stricmp( tok, "texFilter" ) ) {
			tok = COM_ParseExt( text, qfalse );
			if ( tok[0] == 0 ) {
				stage->bundle[0].filter = NumTexFilters;
				ri.Printf( PRINT_WARNING, "missing parameters for texFilter in shader '%s'\n", shader.name );
				continue;
			}
			if ( !N_stricmp( tok, "bilinear" ) ) {
				stage->bundle[0].filter = TexFilter_Bilinear;
			}
			else if ( !N_stricmp( tok, "nearest" ) ) {
				stage->bundle[0].filter = TexFilter_Nearest;
			}
			else if ( !N_stricmp( tok, "linear_nearest" ) ) {
				stage->bundle[0].filter = TexFilter_LinearNearest;
			}
			else if ( !N_stricmp( tok, "nearest_linear" ) ) {
				stage->bundle[0].filter = TexFilter_NearestLinear;
			}
			else if ( !N_stricmp( tok, "default" ) ) {
				continue;
			}
			else {
				stage->bundle[0].filter = NumTexFilters;
				ri.Printf( PRINT_WARNING, "unknown texFilter parameter '%s' in shader '%s'\n", tok, shader.name );
				continue;
			}
		}
		//
		// stage <type>
		//
		else if ( !N_stricmp( tok, "stage" ) ) {
			tok = COM_ParseExt( text, qfalse );
			if ( !tok[0] ) {
				ri.Printf( PRINT_WARNING, "missing parameters for stage in shader '%s'\n", shader.name );
				continue;
			}

			if ( !N_stricmp( tok, "diffuseMap" ) ) {
				stage->type = ST_DIFFUSEMAP;
			}
			else if ( !N_stricmp( tok, "normalMap" ) || !N_stricmp( tok, "bumpMap" ) || !N_stricmp( tok, "heightMap" ) ) {
				stage->type = ST_NORMALMAP;
				VectorSet4( stage->normalScale, r_baseNormalX->f, r_baseNormalY->f, 1.0f, r_baseParallax->f );
			}
			else if ( !N_stricmp( tok, "normalParallaxMap" ) || !N_stricmp( tok, "bumpParallaxMap" ) || !N_stricmp( tok, "heightParallaxMap" ) ) {
				if ( r_parallaxMapping->i ) {
					stage->type = ST_NORMALPARALLAXMAP;
				} else {
					stage->type = ST_NORMALMAP;
				}
				VectorSet4( stage->normalScale, r_baseNormalX->f, r_baseNormalY->f, 1.0f, r_baseParallax->f );
			}
			else if ( !N_stricmp( tok, "specularMap" ) ) {
				stage->type = ST_SPECULARMAP;
				VectorSet4( stage->specularScale, 1.0f, 1.0f, 1.0f, 1.0f );
			}
			else {
				ri.Printf( PRINT_WARNING, "unknown stage parameter '%s' in shader '%s'\n", tok, shader.name );
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
			if ( !N_stricmp( tok, "wave" ) )
			{
				ParseWaveForm( text, &stage->rgbWave );
				stage->rgbGen = CGEN_WAVEFORM;
			}
			else if ( !N_stricmp( tok, "const" ) )
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

static qboolean ParseShader( const char **text )
{
    const char *tok;
	int s;
	branchType branch;
	resultType res;
    unsigned depthMaskBits = GLS_DEPTHMASK_TRUE, blendSrcBits = 0, blendDstBits = 0, atestBits = 0, depthFuncBits = 0;
    qboolean depthMaskExplicit;
	uintptr_t p1, p2;

	// gcc doesn't like it when we do *text + r_extensionOffset
	p1 = (uintptr_t)(*text);
	p2 = (uintptr_t)(r_extensionOffset);

	s = 0;
	r_extendedShader = p1 + p2;

	res = res_invalid;

	tok = COM_ParseExt( text, qtrue );
	if ( tok[0] != '{' ) {
		ri.Printf( PRINT_WARNING, "expecting '{', found '%s' instead in shader '%s'\n", tok, shader.name );
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
			shader.noPicMip = qtrue;
			continue;
		}
		// disable mipmapping
		else if ( !N_stricmp( tok, "nomipmaps")) {
			shader.noMipMaps = qtrue;
			shader.noPicMip = qtrue;
			continue;
		}
		// sort
		else if ( !N_stricmp( tok, "sort" ) ) {
			ParseSort( text );
			continue;
		}
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
        }
		// polygonOffset
		else if ( !N_stricmp( tok, "polygonOffset" ) ) {
			shader.polygonOffset = qtrue;
			continue;
		}
		// tonemap parms
		else if  ( !N_stricmp( tok, "tonemap" ) ) {
			tok = COM_ParseExt( text, qfalse );
			rg.toneMinAvgMaxLevel[0] = N_atof( tok );
			tok = COM_ParseExt( text, qfalse );
			rg.toneMinAvgMaxLevel[1] = N_atof( tok );
			tok = COM_ParseExt( text, qfalse );
			rg.toneMinAvgMaxLevel[2] = N_atof( tok );

			tok = COM_ParseExt( text, qfalse );
			rg.autoExposureMinMax[0] = N_atof( tok );
			tok = COM_ParseExt( text, qfalse );
			rg.autoExposureMinMax[1] = N_atof( tok );

			SkipRestOfLine( text );
			continue;
		}
		// skip stuff that only q3map or the server needs
		else if ( !N_stricmp( tok, "surfaceParm" ) ) {
			ParseSurfaceParm( text );
			continue;
		}
		// conditional stage definition
		else if ( ( !N_stricmp( tok, "if" ) || !N_stricmp( tok, "else" ) || !N_stricmp( tok, "elif" ) ) /* && r_extendedShader */ ) {
			if ( N_stricmp( tok, "if" ) == 0 ) {
				branch = brIF;
			} else {
				if ( res == res_invalid  ) {
					// we don't have any previous 'if' statements
					ri.Printf( PRINT_WARNING, "WARNING: unexpected '%s' in '%s'\n", tok, shader.name );
					return qfalse;
				}
				if ( N_stricmp( tok, "else" ) == 0 ) {
					branch = brELSE;
				} else {
					branch = brELIF;
				}
			}

			if ( branch != brELSE ) { // we can set/update result
				tok = COM_ParseComplex( text, qfalse );
				if ( com_tokentype != TK_SCOPE_OPEN ) {
					ri.Printf( PRINT_WARNING, "WARNING: expecting '(' in '%s'\n", shader.name );
					return qfalse;
				}
				if ( !ParseCondition( text, ( branch == brIF || res == res_true ) ? &res : NULL ) ) {
					ri.Printf( PRINT_WARNING, "WARNING: error parsing condition in '%s'\n", shader.name );
					return qfalse;
				}
			}

			if ( res == res_false )	{
				// skip next stage or keyword until newline
				tok = COM_ParseExt( text, qtrue );
				if ( tok[0] == '{' ) {
					SkipBracedSection( text, 1 );
				} else {
					SkipRestOfLine( text );
				}
			} else {
				// parse next tokens as usual
			}

			if ( branch == brELSE ) {
				res = res_invalid; // finalize branch
			} else {
				res ^= 1; // or toggle for possible "elif" / "else" statements
			}

			continue;
		}
        else {
            ri.Printf( PRINT_WARNING, "unrecognized parameter in shader '%s': '%s'\n", shader.name, tok );
            continue;
        }
    }

    return qtrue;
}

//====================================================


/*
====================
FindLightingStages

Find proper stage for dlight pass
====================
*/
#define GLS_BLEND_BITS (GLS_SRCBLEND_BITS | GLS_DSTBLEND_BITS)
static void FindLightingStages( void )
{
	int i;
	shader.lightingStage = -1;

	if ( /* shader.isSky || */ ( shader.surfaceFlags & (SURFACEPARM_NODLIGHT /* | SURFACEPARM_SKY */) ) || shader.sort > SS_OPAQUE ) {
		return;
	}

	for ( i = 0; i < MAX_SHADER_STAGES; i++ ) {
		if ( !stages[i].bundle[0].isLightmap ) {
			if ( stages[i].bundle[0].tcGen != TCGEN_TEXTURE )
				continue;
			if ( (stages[i].stateBits & GLS_BLEND_BITS) == (GLS_SRCBLEND_ONE | GLS_DSTBLEND_ONE) )
				continue;
			if ( stages[i].rgbGen == CGEN_IDENTITY && (stages[i].stateBits & GLS_BLEND_BITS) == (GLS_SRCBLEND_DST_COLOR | GLS_DSTBLEND_ZERO) ) {
				if ( shader.lightingStage >= 0 ) {
					continue;
				}
			}
			shader.lightingStage = i;
		}
	}
}


/*
=================
VertexLightingCollapse

If vertex lighting is enabled, only render a single
pass, trying to guess which is the correct one to best approximate
what it is supposed to look like.
=================
*/
static void VertexLightingCollapse( void ) {
	int		stage;
	shaderStage_t	*bestStage;
	int		bestImageRank;
	int		rank;

	// if we aren't opaque, just use the first pass
	if ( shader.sort == SS_OPAQUE ) {

		// pick the best texture for the single pass
		bestStage = &stages[0];
		bestImageRank = -999999;

		for ( stage = 0; stage < MAX_SHADER_STAGES; stage++ ) {
			shaderStage_t *pStage = &stages[stage];

			if ( !pStage->active ) {
				break;
			}
			rank = 0;

			if ( pStage->bundle[0].isLightmap ) {
				rank -= 100;
			}
			if ( pStage->bundle[0].tcGen != TCGEN_TEXTURE ) {
				rank -= 5;
			}
			if ( pStage->bundle[0].numTexMods ) {
				rank -= 5;
			}
			if ( pStage->rgbGen != CGEN_IDENTITY && pStage->rgbGen != CGEN_IDENTITY_LIGHTING ) {
				rank -= 3;
			}

			if ( rank > bestImageRank  ) {
				bestImageRank = rank;
				bestStage = pStage;
			}
		}

		stages[0].bundle[0] = bestStage->bundle[0];
		stages[0].stateBits &= ~( GLS_DSTBLEND_BITS | GLS_SRCBLEND_BITS );
		stages[0].stateBits |= GLS_DEPTHMASK_TRUE;
		if ( shader.lightmapIndex == LIGHTMAP_NONE ) {
			stages[0].rgbGen = CGEN_LIGHTING_DIFFUSE;
		} else {
			stages[0].rgbGen = CGEN_EXACT_VERTEX;
		}
		stages[0].alphaGen = AGEN_SKIP;
	} else {
		// don't use a lightmap (tesla coils)
		if ( stages[0].bundle[0].isLightmap ) {
			stages[0] = stages[1];
		}

		// if we were in a cross-fade cgen, hack it to normal
		if ( stages[0].rgbGen == CGEN_ONE_MINUS_ENTITY || stages[1].rgbGen == CGEN_ONE_MINUS_ENTITY ) {
			stages[0].rgbGen = CGEN_IDENTITY_LIGHTING;
		}
		if ( ( stages[0].rgbGen == CGEN_WAVEFORM && stages[0].rgbWave.func == GF_SAWTOOTH )
			&& ( stages[1].rgbGen == CGEN_WAVEFORM && stages[1].rgbWave.func == GF_INVERSE_SAWTOOTH ) ) {
			stages[0].rgbGen = CGEN_IDENTITY_LIGHTING;
		}
		if ( ( stages[0].rgbGen == CGEN_WAVEFORM && stages[0].rgbWave.func == GF_INVERSE_SAWTOOTH )
			&& ( stages[1].rgbGen == CGEN_WAVEFORM && stages[1].rgbWave.func == GF_SAWTOOTH ) ) {
			stages[0].rgbGen = CGEN_IDENTITY_LIGHTING;
		}
	}

	for ( stage = 1; stage < MAX_SHADER_STAGES; stage++ ) {
		shaderStage_t *pStage = &stages[stage];

		if ( !pStage->active ) {
			break;
		}

		memset( pStage, 0, sizeof( *pStage ) );
	}
}

static void ComputeVertexAttribs(void)
{
    uint32_t i, stage;

    // dlights always need ATTRIB_NORMAL
    shader.vertexAttribs = ATTRIB_POSITION | ATTRIB_COLOR;

    if (shader.defaultShader) {
        shader.vertexAttribs |= ATTRIB_TEXCOORD;
    }
    
}

static void CollapseStagesToLightall( shaderStage_t *diffuse, shaderStage_t *normal, shaderStage_t *specular, shaderStage_t *lightmap,
	qboolean useLightVector, qboolean useLightVertex, qboolean parallax, qboolean tcgen )
{
	uint32_t defs = 0;

	if ( lightmap ) {
		diffuse->bundle[TB_LIGHTMAP] = lightmap->bundle[0];
	}

	if ( r_normalMapping->i ) {
		texture_t *diffuseTex;

		if ( normal ) {
			diffuse->bundle[TB_NORMALMAP] = normal->bundle[0];

			if ( parallax && 0 ) {

			}

			VectorCopy4( normal->normalScale, diffuse->normalScale );
		}
		else if ( ( lightmap || useLightVector || useLightVertex ) && ( diffuseTex = diffuse->bundle[TB_DIFFUSEMAP].image[0] ) != NULL ) {
			char normalName[MAX_NPATH];
			texture_t *normalTex;
			imgFlags_t normalFlags = ( diffuseTex->flags & ~IMGFLAG_GENNORMALMAP ) | IMGFLAG_NOLIGHTSCALE;
			
			// try a normalheight image first
			COM_StripExtension( diffuseTex->imgName, normalName, MAX_NPATH );
			N_strcat( normalName, MAX_NPATH, "_nh" );

			normalTex = R_FindImageFile( normalName, IMGTYPE_NORMALHEIGHT, normalFlags );

			if ( normalTex ) {
				parallax = qtrue;
			} else {
				// try a normal image ("_n" suffix)
				normalName[strlen(normalName) - 1] = '\0';
				normalTex = R_FindImageFile( normalName, IMGTYPE_NORMAL, normalFlags );
			}

			if ( normalTex ) {
				diffuse->bundle[TB_NORMALMAP] = diffuse->bundle[0];
				diffuse->bundle[TB_NORMALMAP].image[0] = normalTex;

				VectorSet4( diffuse->normalScale, r_baseNormalX->f, r_baseNormalY->f, 1.0f, r_baseParallax->f );
			}
		}
	}

	if ( r_specularMapping->i ) {
		texture_t *diffuseTex;
		if ( specular ) {
			//ri.Printf(PRINT_ALL, ", specularmap %s", specular->bundle[0].image->imgName);
			diffuse->bundle[TB_SPECULARMAP] = specular->bundle[0];
			VectorCopy4( specular->specularScale, diffuse->specularScale );
		}
		else if ( ( lightmap || useLightVector || useLightVertex ) && ( diffuseTex = diffuse->bundle[TB_DIFFUSEMAP].image[0] ) != NULL ) {
			char specularName[MAX_NPATH];
			texture_t *specularTex;
			imgFlags_t specularFlags = ( diffuseTex->flags & ~IMGFLAG_GENNORMALMAP ) | IMGFLAG_NOLIGHTSCALE;

			COM_StripExtension( diffuseTex->imgName, specularName, MAX_NPATH );
			N_strcat( specularName, MAX_NPATH, "_s" );

			specularTex = R_FindImageFile(specularName, IMGTYPE_COLORALPHA, specularFlags);

			if ( specularTex ) {
				diffuse->bundle[TB_SPECULARMAP] = diffuse->bundle[0];
				diffuse->bundle[TB_SPECULARMAP].image[0] = specularTex;

				VectorSet4( diffuse->specularScale, 1.0f, 1.0f, 1.0f, 1.0f );
			}
		}
	}

	if (tcgen || diffuse->bundle[0].numTexMods) {
		defs |= LIGHTDEF_USE_TCGEN_AND_TCMOD;
	}

	//ri.Printf(PRINT_ALL, ".\n");

//	diffuse->glslShaderGroup = tr.lightallShader;
//	diffuse->glslShaderIndex = defs;
}

static void SortNewShader( void )
{
    int32_t i;
    uint32_t sort;
    shader_t *newShader;

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

static shader_t *GeneratePermanentShader( void )
{
    shader_t *newShader;
    uint64_t size, hash;
	uint32_t i;

    newShader = ri.Hunk_Alloc( sizeof( *newShader ), h_low );

    *newShader = shader;

    rg.shaders[rg.numShaders] = newShader;
    newShader->index = rg.numShaders;

    rg.sortedShaders[rg.numShaders] = newShader;
    newShader->sortedIndex = rg.numShaders;

    rg.numShaders++;

	for ( i = 0; i < MAX_SHADER_STAGES; i++ ) {
		if (!stages[i].active) {
			break;
		}

		newShader->stages[i] = ri.Hunk_Alloc( sizeof(stages[i]), h_low );
		*newShader->stages[i] = stages[i];

		for ( uint32_t b = 0 ; b < NUM_TEXTURE_BUNDLES ; b++ ) {
			size = newShader->stages[i]->bundle[b].numTexMods * sizeof( texModInfo_t );
			if ( size ) {
				newShader->stages[i]->bundle[b].texMods = ri.Hunk_Alloc( size, h_low );
				memcpy( newShader->stages[i]->bundle[b].texMods, stages[i].bundle[b].texMods, size );
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
	memset( &shader, 0, sizeof( shader ) );
	N_strncpyz( shader.name, name, sizeof(shader.name) );

	shader.lightmapIndex = lightmapIndex;

	for ( uint32_t i = 0 ; i < MAX_SHADER_STAGES ; i++ ) {
		stages[i].bundle[0].texMods = texMods[i];
		stages[i].bundle[0].numTexMods = 0; // fixes two images one screen bug

		// default normal/specular
		VectorSet4( stages[i].normalScale, 0.0f, 0.0f, 0.0f, 0.0f );
		if ( r_pbr->i ) {
			stages[i].specularScale[0] = r_baseGloss->f;
		}
		else {
			stages[i].specularScale[0] =
			stages[i].specularScale[1] =
			stages[i].specularScale[2] = r_baseSpecular->f;
			stages[i].specularScale[3] = r_baseGloss->f;
		}
	}
}


static int CollapseStagesToGLSL(void)
{
	int i, j, numStages;
	qboolean skip = qfalse;

	// skip shaders with deforms
	if (shader.numDeforms != 0)
	{
		skip = qtrue;
	}

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

	// convert any remaining lightmap stages to a lighting pass with a white texture
	// only do this with r_sunlightMode non-zero, as it's only for correct shadows.
	if ( r_sunlightMode->i /* && shader.numDeforms == 0 */ ) {
		for  (i = 0; i < MAX_SHADER_STAGES; i++ ) {
			shaderStage_t *pStage = &stages[i];

			if ( !pStage->active ) {
				continue;
			}

			//if (pStage->adjustColorsForFog)
			//	continue;

			if ( pStage->bundle[TB_DIFFUSEMAP].tcGen == TCGEN_LIGHTMAP ) {
				pStage->glslShaderGroup = rg.lightallShader;
				pStage->glslShaderIndex = LIGHTDEF_USE_LIGHTMAP;
				pStage->bundle[TB_LIGHTMAP] = pStage->bundle[TB_DIFFUSEMAP];
				pStage->bundle[TB_DIFFUSEMAP].image[0] = rg.whiteImage;
				pStage->bundle[TB_DIFFUSEMAP].isLightmap = qfalse;
				pStage->bundle[TB_DIFFUSEMAP].tcGen = TCGEN_TEXTURE;
			}
		}
	}

	// convert any remaining lightingdiffuse stages to a lighting pass
	if ( shader.numDeforms == 0 ) {
		for ( i = 0; i < MAX_SHADER_STAGES; i++ ) {
			shaderStage_t *pStage = &stages[i];

			if ( !pStage->active ) {
				continue;
			}

			//if (pStage->adjustColorsForFog)
			//	continue;

			if (pStage->rgbGen == CGEN_LIGHTING_DIFFUSE)
			{
				pStage->glslShaderGroup = rg.lightallShader;
				pStage->glslShaderIndex = LIGHTDEF_USE_LIGHT_VECTOR;

				if (pStage->bundle[0].tcGen != TCGEN_TEXTURE || pStage->bundle[0].numTexMods != 0)
					pStage->glslShaderIndex |= LIGHTDEF_USE_TCGEN_AND_TCMOD;
			}
		}
	}

	return numStages;
}

static shader_t *FinishShader(void)
{
	uint32_t stage;
	qboolean hasLightmapStage;
	qboolean vertexLightmap;

	hasLightmapStage = qfalse;
	vertexLightmap = qfalse;

    //
    // set polygon offset
    //
    if (shader.polygonOffset && shader.sort == SS_BAD) {
        shader.sort = SS_DECAL;
    }

    // there are times when you will need to manually apply a sort to
	// opaque alpha tested shaders that have later blend passes
	if ( shader.sort == SS_BAD ) {
		shader.sort = SS_OPAQUE;
	}

	for (stage = 0; stage < MAX_SHADER_STAGES; ) {
		shaderStage_t *stageP = &stages[stage];

		if ( !stageP->active ) {
			break;
		}

		// check for a missing texture
		if ( !stageP->bundle[0].image[0] ) {
			ri.Printf( PRINT_WARNING, "Shader %s has a stage with no image\n", shader.name );
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


		//
		// determine sort order and [nope...]fog color adjustment
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

			// modulate, additive
			if ( ( ( blendSrcBits == GLS_SRCBLEND_ONE ) && ( blendDstBits == GLS_DSTBLEND_ONE ) ) ||
				( ( blendSrcBits == GLS_SRCBLEND_ZERO ) && ( blendDstBits == GLS_DSTBLEND_ONE_MINUS_SRC_COLOR ) ) ) {
//				pStage->adjustColorsForFog = ACFF_MODULATE_RGB;
			}
			// strict blend
			else if ( ( blendSrcBits == GLS_SRCBLEND_SRC_ALPHA ) && ( blendDstBits == GLS_DSTBLEND_ONE_MINUS_SRC_ALPHA ) )
			{
//				pStage->adjustColorsForFog = ACFF_MODULATE_ALPHA;
			}
			// premultiplied alpha
			else if ( ( blendSrcBits == GLS_SRCBLEND_ONE ) && ( blendDstBits == GLS_DSTBLEND_ONE_MINUS_SRC_ALPHA ) )
			{
//				pStage->adjustColorsForFog = ACFF_MODULATE_RGBA;
			} else {
				// we can't adjust this one correctly, so it won't be exactly correct in fog
			}
			// don't screw with sort order if this is a portal or environment
			if ( shader.sort == SS_BAD ) {
				// see through item, like a grill or grate
				if ( stageP->stateBits & GLS_DEPTHMASK_TRUE ) {
					shader.sort = SS_SEE_THROUGH;
				} else {
					shader.sort = SS_BLEND;
				}
			}
		}

		// don't screw with sort order if this is environment
		if (shader.sort == SS_BAD) {
			// see through item, like a grill or grate
			if (stageP->stateBits & GLS_DEPTHMASK_TRUE) {
				shader.sort = SS_SEE_THROUGH;
			} else {
				shader.sort = SS_BLEND;
			}
		}

		stage++;
	}

	// there are times when you will need to manually apply a sort to
	// opaque alpha tested shaders that have later blend passes
	if ( shader.sort == SS_BAD ) {
		shader.sort = SS_OPAQUE;
	}

	//
	// if we are in r_vertexLight mode, never use a lightmap texture
	//
	if ( stage > 1 && ( (r_vertexLight->i && rg.vertexLightingAllowed) || glConfig.hardwareType == GLHW_PERMEDIA2 ) ) {
		VertexLightingCollapse();
		hasLightmapStage = qfalse;
	}

	//
	// look for multitexture potential
	//
	stage = CollapseStagesToGLSL();

	if ( shader.lightmapIndex >= 0 && !hasLightmapStage ) {
		if (vertexLightmap) {
			ri.Printf( PRINT_DEVELOPER, "WARNING: shader '%s' has VERTEX forced lightmap!\n", shader.name );
		} else {
			ri.Printf( PRINT_DEVELOPER, "WARNING: shader '%s' has lightmap but no lightmap stage!\n", shader.name );
			shader.lightmapIndex = LIGHTMAP_NONE;
		}
	}

	FindLightingStages();

	// fogonly shaders don't have any normal passes
//	if (stage == 0 /* && !shader.isSky */)
//		shader.sort = SS_FOG;

	// determine which vertex attributes this shader needs
	ComputeVertexAttribs();

	if ( rg.world && rg.worldMapLoaded ) {
		rg.world->levelShaders++;
	}

    return GeneratePermanentShader();
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
	char		strippedName[MAX_NPATH];
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
shader_t *R_FindShader( const char *name ) {
	char		strippedName[MAX_NPATH];
	uint64_t	hash;
	const char	*shaderText;
	texture_t	*image;
	shader_t	*sh;

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

	InitShader( strippedName, LIGHTMAP_2D );

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

/*
===============
R_ShaderList_f

Dump information on all valid shaders to the console
A second parameter will cause it to print in sorted order
===============
*/
void R_ShaderList_f (void) {
	uint32_t		i;
	uint32_t		count;
	const shader_t *sh;

	ri.Printf( PRINT_INFO, "-----------------------\n" );

	count = 0;
	for ( i = 0 ; i < rg.numShaders ; i++ ) {
//		if ( ri.Cmd_Argc() > 1 ) {
//			sh = rg.sortedShaders[i];
//		} else {
//			sh = tr.shaders[i];
//		}
		sh = rg.shaders[i];

//		ri.Printf( PRINT_INFO, "%i ", sh->numUnfoggedPasses );

		if ( sh->lightmapIndex >= 0 ) {
			ri.Printf( PRINT_INFO, "L " );
		} else {
			ri.Printf( PRINT_INFO, "  " );
		}
		if ( sh->explicitlyDefined ) {
			ri.Printf( PRINT_INFO, "E " );
		} else {
			ri.Printf( PRINT_INFO, "  " );
		}

		/*
		if ( sh->optimalStageIteratorFunc == RB_StageIteratorGeneric ) {
			ri.Printf( PRINT_INFO, "gen " );
		} else if ( sh->optimalStageIteratorFunc == RB_StageIteratorSky ) {
			ri.Printf( PRINT_INFO, "sky " );
		} else {
			ri.Printf( PRINT_INFO, "    " );
		}
		*/

		if ( sh->defaultShader ) {
			ri.Printf( PRINT_INFO, ": %s (DEFAULTED)\n", sh->name );
		} else {
			ri.Printf( PRINT_INFO, ": %s\n", sh->name );
		}
		count++;
	}
	ri.Printf( PRINT_INFO, "%i total shaders\n", count );
	ri.Printf( PRINT_INFO, "------------------\n" );
}

nhandle_t RE_RegisterShaderFromTexture( const char *name, int32_t lightmapIndex, texture_t *image )
{
    uint64_t hash;
    shader_t *sh;

    hash = Com_GenerateHashValue(name, MAX_RENDER_SHADERS);

	// probably not necessary since this function
	// only gets called from tr_font.c with lightmapIndex == LIGHTMAP_2D
	// but better safe than sorry.
	if ( lightmapIndex >= rg.numLightmaps ) {
		lightmapIndex = LIGHTMAP_WHITEIMAGE;
	}

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
nhandle_t RE_RegisterShaderLightMap( const char *name ) {
	shader_t	*sh;

	if ( strlen( name ) >= MAX_NPATH ) {
		ri.Printf( PRINT_INFO, "Shader name exceeds MAX_NPATH\n" );
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
nhandle_t RE_RegisterShader( const char *name ) {
	shader_t	*sh;

	if ( !name ) {
		ri.Printf( PRINT_INFO, "NULL shader\n" );
		return 0;
	}

	if ( strlen( name ) >= MAX_NPATH ) {
		ri.Printf( PRINT_INFO, "Shader name exceeds MAX_NPATH\n" );
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
it and returns a valid (possibly default) shader_t to be used internally.
====================
*/
shader_t *R_GetShaderByHandle( nhandle_t hShader ) {
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

static int loadShaderBuffers( char **shaderFiles, const uint64_t numShaderFiles, char **buffers )
{
	char filename[MAX_NPATH+8];
	char shaderName[MAX_NPATH];
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

	memset( buffers, 0, sizeof( buffers ) );
	memset( shaderTextHashTableSizes, 0, sizeof( shaderTextHashTableSizes ) );

    uint64_t sum = 0;

	// scan for legacy shader files
	shaderFiles = ri.FS_ListFiles( "scripts", ".shader", &numShaderFiles );

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
	r_shaderText = ri.Hunk_Alloc( sum + numShaderFiles*2 + 1, h_low );
	memset( r_shaderText, 0, sum + numShaderFiles*2 );

	textEnd = r_shaderText;

	// free in reverse order, so the temp files are all dumped
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

		hash = Com_GenerateHashValue( tok, MAX_SHADERTEXT_HASH );
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

    memset( hashTable, 0, sizeof( hashTable ) );

	CreateInternalShaders();

	ScanAndLoadShaderFiles();
}

void R_UnloadLevelShaders( void )
{
	uint64_t i, j;

	ri.Printf( PRINT_INFO, "R_UnloadLevelShaders(): Releasing %u shaders...\n", rg.world->levelShaders );

	for ( i = 0; i < MAX_RENDER_SHADERS; i++ ) {
		if ( hashTable[i] && hashTable[i]->index >= rg.world->firstLevelShader ) {
			hashTable[i]->next = NULL;
			hashTable[i] = NULL;
		}
	}
	for ( i = 0; i < rg.world->levelShaders; i++ ) {
		rg.shaders[ i + rg.world->firstLevelShader ] = NULL;
	}
	rg.numShaders = rg.world->firstLevelShader;
}