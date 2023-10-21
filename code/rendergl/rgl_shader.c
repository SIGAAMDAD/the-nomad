#include "rgl_local.h"

// rgl_shader.c -- this file deals with the parsing and defintion of shaders

static char *r_shaderText;
static const char *r_extensionOffset;
static uint64_t r_extendedShader;

static shader_t shader;
static shader_t *hashTable[MAX_RENDER_SHADERS];

#define MAX_SHADERTEXT_HASH 2048
static const char **shaderTextHashTable[MAX_SHADERTEXT_HASH];

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

static qboolean ParseShader(const char **text)
{
    const char *tok;
    int blendSrcBits, blendDstBits, depthMaskBits;

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
        // blendfunc <srcFactor> <dstFactor>
        // or blendfunc <add|filter|blend>
        //
        else if (!N_stricmp(tok, "blendFunc")) {
            tok = COM_ParseExt(text, qfalse);
            if (!tok[0]) {
                ri.Printf(PRINT_INFO, COLOR_YELLOW "WARNING: missing parm for blendFunc in shader '%s'\n", shader.name);
                return qfalse;
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

    shader.stateBits = depthMaskBits |
        blendSrcBits | blendDstBits;
    
    return qtrue;
}

//========================================================================================

/*
FinishShader: allocates a new shader onto the hunk ready for use
*/
static shader_t *FinishShader(void)
{
    shader_t *sh;
    uint64_t hash;
	
	if (shader.defaultShader) {
		sh = rg.defaultShader;
		return sh;
	}

    sh = rg.shaders[rg.numShaders] = ri.Hunk_Alloc(sizeof(*sh), h_low);
    sh->index = rg.numShaders;
    rg.numShaders++;

    // update hash table
    hash = Com_GenerateHashValue(shader.name, MAX_RENDER_SHADERS);
    sh->next = hashTable[hash];
    hashTable[hash] = sh;

	// allocate the texture
    sh->texture = R_FindImageFile(shader.name, 0, IMGFLAG_NO_COMPRESSION);
	if (!sh->texture) {
		ri.Error(ERR_DROP, "Failed to load shader texture for '%s'", shader.name);
	}

    sh->sortedIndex = Com_GenerateHashValue(shader.name, MAX_RENDER_SHADERS);

	return sh;
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
	const char *token, *p;
	uint64_t i, hash;

	hash = Com_GenerateHashValue(shadername, MAX_SHADERTEXT_HASH);

	if (shaderTextHashTable[hash]) {
		for (i = 0; shaderTextHashTable[hash][i]; i++) {
			p = shaderTextHashTable[hash][i];
			token = COM_ParseExt(&p, qtrue);
			if (!N_stricmp(token, shadername))
				return p;
		}
	}

	return NULL;
}

/*
InitShader
*/
static void InitShader(const char *name)
{
	memset(&shader, 0, sizeof(shader));
	N_strncpyz(shader.name, name, sizeof(shader.name));
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

shader_t *R_FindShader(const char *name)
{
    char strippedName[MAX_GDR_PATH];
    uint64_t hash;
    const char *shaderText;
    texture_t *texture;
    shader_t *sh;

    if (name[0] == '\0') {
        return rg.defaultShader;
    }

    COM_StripExtension(name, strippedName, sizeof(strippedName));

    hash = Com_GenerateHashValue(strippedName, MAX_RENDER_SHADERS);

    //
    // see if the shader is already loaded
    //
    for (sh = hashTable[hash]; sh; sh = sh->next) {
        if (sh->defaultShader || !N_stricmp(sh->name, name)) {
            // match found
            return sh;
        }
    }

    //
    // attempt to define shader from an explicit parameter file
    //
    shaderText = FindShaderInShaderText(strippedName);
    if (shaderText) {
        // enables this when building a bff file to get a global list
        // of all explicit shaders
        if (!ParseShader(&shaderText)) {
            // had errors, so use default shader
            shader.defaultShader = qtrue;
        }
		sh = FinishShader();
        return sh;
    }
    return NULL;
}

nhandle_t RE_RegisterShaderFromTexture(const char *name, texture_t *texture)
{
    uint64_t hash;
    shader_t *sh;

    hash = Com_GenerateHashValue(name, MAX_RENDER_SHADERS);

    //
    // see if the shader is already loaded
    //
    for (sh = hashTable[hash]; sh; sh = sh->next) {
        if (sh->defaultShader || !N_stricmp(sh->name, name)) {
            // match found
            return sh->index;
        }
    }
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
	const char *p, *token;
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

		if ( !buffers[i] )
			ri.Error( ERR_DROP, "Couldn't load %s", filename );

		p = buffers[i];
		COM_BeginParseSession( filename );
		
		shaderStart = NULL;
		denyErrors = qfalse;

		while ( 1 ) {
			token = COM_ParseExt( &p, qtrue );
			
			if ( !*token )
				break;

			N_strncpyz( shaderName, token, sizeof( shaderName ) );
			shaderLine = COM_GetCurrentParseLine();

			token = COM_ParseExt( &p, qtrue );
			if ( token[0] != '{' || token[1] != '\0' ) {
				ri.Printf( PRINT_DEVELOPER, "File %s: shader \"%s\" " \
					"on line %lu missing opening brace", filename, shaderName, shaderLine );
				if ( token[0] )
					ri.Printf( PRINT_DEVELOPER, " (found \"%s\" on line %lu)\n", token, COM_GetCurrentParseLine() );
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
	uint64_t i;
	const char *token, *hashMem;
	char *textEnd;
	const char *p, *oldp;
	uint64_t shaderTextHashTableSizes[MAX_SHADERTEXT_HASH], hash, size;

    uint64_t sum = 0;

	// scan for legacy shader files
	shaderFiles = ri.FS_ListFiles( "scripts", ".shader", &numShaderFiles );
//	mtrFiles = ri.FS_ListFiles( "scripts", ".mtrx", &numMtrFiles );

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
	r_shaderText[ 0 ] = r_shaderText[ sum + numShaderFiles*2 ] = '\0';

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
		token = COM_ParseExt( &p, qtrue );
		if ( token[0] == 0 ) {
			break;
		}
		hash = Com_GenerateHashValue(token, MAX_SHADERTEXT_HASH);
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
		token = COM_ParseExt( &p, qtrue );
		if ( token[0] == 0 ) {
			break;
		}

		hash = Com_GenerateHashValue(token, MAX_SHADERTEXT_HASH);
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
	InitShader( "<default>" );
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

