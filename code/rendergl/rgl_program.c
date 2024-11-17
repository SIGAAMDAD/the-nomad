#include "rgl_local.h"
#define STB_INCLUDE_LINE_GLSL
#define STB_INCLUDE_IMPLEMENTATION
#include "stb_include.h"

extern const char *fallbackShader_generic_vp;
extern const char *fallbackShader_generic_fp;
extern const char *fallbackShader_imgui_vp;
extern const char *fallbackShader_imgui_fp;
extern const char *fallbackShader_texturecolor_vp;
extern const char *fallbackShader_texturecolor_fp;
extern const char *fallbackShader_tile_vp;
extern const char *fallbackShader_tile_fp;
extern const char *fallbackShader_blur_vp;
extern const char *fallbackShader_blur_fp;
extern const char *fallbackShader_bloom_vp;
extern const char *fallbackShader_bloom_fp;
extern const char *fallbackShader_gaussian_vp;
extern const char *fallbackShader_gaussian_fp;

#define GLSL_VERSION_ATLEAST(major,minor) (glContext.glslVersionMajor > (major) || (glContext.versionMajor == (major) && glContext.glslVersionMinor >= minor))

typedef struct {
	const char *name;
	uint64_t type;
} uniformInfo_t;

// these must in the same order as in uniform_t in rgl_local.h
static uniformInfo_t uniformsInfo[UNIFORM_COUNT] = {
	{ "u_DiffuseMap",           GLSL_TEXTURE },
	{ "u_LightMap",             GLSL_TEXTURE },
	{ "u_NormalMap",            GLSL_TEXTURE },
	{ "u_DeluxeMap",            GLSL_TEXTURE },
	{ "u_SpecularMap",          GLSL_TEXTURE },
	{ "u_BrightMap",			GLSL_TEXTURE },

	{ "u_TextureMap",           GLSL_TEXTURE },
	{ "u_LevelsMap",            GLSL_TEXTURE },

	{ "u_ScreenImageMap",       GLSL_TEXTURE },
	{ "u_ScreenDepthMap",       GLSL_TEXTURE },

	{ "u_ShadowMap",            GLSL_TEXTURE },

	{ "u_AreaTexture",          GLSL_TEXTURE },
	{ "u_SearchTexture",        GLSL_TEXTURE },
	{ "u_EdgesTexture",         GLSL_TEXTURE },
	{ "u_BlendTexture",         GLSL_TEXTURE },

	{ "u_ShadowMvp",            GLSL_MAT16 },

	{ "u_DiffuseTexMatrix",     GLSL_VEC4 },
	{ "u_DiffuseTexOffTurb",    GLSL_VEC4 },

	{ "u_TCGen0",               GLSL_INT },
	{ "u_TCGen0Vector0",        GLSL_VEC3 },
	{ "u_TCGen0Vector1",        GLSL_VEC3 },

	{ "u_DeformGen",            GLSL_INT },
	{ "u_DeformParams",         GLSL_VEC5 },

	{ "u_ColorGen",             GLSL_INT },
	{ "u_AlphaGen",             GLSL_INT },
	{ "u_Color",                GLSL_VEC4 },
	{ "u_BaseColor",            GLSL_VEC4 },
	{ "u_VertColor",            GLSL_VEC4 },
	{ "u_AmbientColor",         GLSL_VEC3 },

	{ "u_ModelViewProjection",  GLSL_MAT16 },

	{ "u_Time",                 GLSL_FLOAT },
	{ "u_NormalScale",          GLSL_VEC4 },
	{ "u_SpecularScale",        GLSL_VEC4 },
	{ "u_ViewOrigin",           GLSL_VEC3 },

	{ "u_AlphaTest",            GLSL_INT },
	{ "u_NumLights",            GLSL_INT },
	{ "u_BlurHorizontal",       GLSL_INT },

	{ "u_LightBuffer",          GLSL_BUFFER },

	{ "u_GammaAmount",          GLSL_FLOAT },
	{ "u_CameraExposure",		GLSL_FLOAT },
	{ "u_ScreenSize",           GLSL_VEC2 },
	{ "u_SharpenAmount",        GLSL_FLOAT },
	{ "u_GamePaused",           GLSL_INT },
	{ "u_AntiAliasing",         GLSL_INT },
	{ "u_HardwareGamma",        GLSL_INT },
	{ "u_HDR",                  GLSL_INT },
	{ "u_PBR",                  GLSL_INT },
	{ "u_ToneMapType",          GLSL_INT },
	{ "u_Bloom",                GLSL_INT },
	{ "u_LightingQuality",		GLSL_INT },
	{ "u_PostProcess",			GLSL_INT },
	{ "u_AntiAliasingQuality",	GLSL_INT },

	{ "u_DispatchComputeSize",	GLSL_UVEC2 },
	{ "u_FinalPass",			GLSL_INT },
};

#define SHADER_CACHE_HEADER (('L'<<24)+('S'<<16)+('L'<<8)+'G')
#define SHADER_CACHE_FILE_NAME CACHE_DIR "/glshadercache.dat"

typedef struct {
	char name[MAX_NPATH];
	void *data;
	GLint size;
	GLenum fmt;
} shaderCacheEntry_t;

static shaderCacheEntry_t *cacheHashTable;
static uint32_t cacheNumEntries;

static void R_LoadShaderCache( void )
{
	uint32_t i;
	char name[MAX_NPATH];
	shaderCacheEntry_t *entry;
	fileHandle_t fh;
	GLint *programFormats;
	GLint numProgramFormats, f;
	qboolean valid;
	uint32_t ident;
	fileStats_t tmp;

	cacheNumEntries = 0;
	cacheHashTable = NULL;

	if ( !glContext.ARB_gl_spirv || !r_useShaderCache->i || !ri.FS_FileExists( SHADER_CACHE_FILE_NAME ) ) {
		return;
	}

	nglGetIntegerv( GL_NUM_PROGRAM_BINARY_FORMATS, &numProgramFormats );
	programFormats = (GLint *)alloca( sizeof( *programFormats ) * numProgramFormats );
	nglGetIntegerv( GL_PROGRAM_BINARY_FORMATS, programFormats );

	fh = ri.FS_FOpenRead( SHADER_CACHE_FILE_NAME );
	if ( fh == FS_INVALID_HANDLE ) {
		ri.Printf( PRINT_INFO, "Failed to load glshadercache.dat\n" );
		return;
	}

	if ( !ri.FS_Read( &ident, sizeof( ident ), fh ) ) {
		ri.Printf( PRINT_ERROR, "Error reading glshadercache identifier.\n" );
		goto error;
	}
	if ( ident != SHADER_CACHE_HEADER ) {
		ri.Printf( PRINT_ERROR, "glshadercache.dat has incorrec identifier.\n" );
		goto error;
	}

	if ( !ri.FS_Read( &cacheNumEntries, sizeof( cacheNumEntries ), fh ) ) {
		ri.Printf( PRINT_ERROR, "Error reading glshadercache header.\n" );
		goto error;
	}
	if ( !cacheNumEntries ) {
		ri.Error( ERR_DROP, "R_LoadShaderCache: numEntries is a funny number" );
	}

	ri.Printf( PRINT_INFO, "Got %u cached SPIR-V objects.\n", cacheNumEntries );
	cacheHashTable = ri.Hunk_Alloc( sizeof( *cacheHashTable ) * cacheNumEntries, h_low );

	for  ( i = 0; i < cacheNumEntries; i++ ) {
		entry = &cacheHashTable[i];

		if ( !ri.FS_Read( entry->name, sizeof( entry->name ), fh ) ) {
			ri.Printf( PRINT_ERROR, "Error reading shader cache entry name at %u\n", i );
			goto error;
		}

		if ( !ri.FS_Read( &entry->fmt, sizeof( entry->fmt ), fh ) ) {
			ri.Printf( PRINT_ERROR, "Error reading shader cache entry format at %u\n", i );
			goto error;
		}

		valid = qfalse;
		for ( f = 0; f < numProgramFormats; f++ ) {
			if ( entry->fmt == programFormats[f] ) {
				valid = qtrue;
				break;
			}
		}
		if ( !valid ) {
			ri.Printf( PRINT_ERROR, "Cached SPIR-V '%s' object has incorrect binary format (0x%04x)\n",
				entry->name, entry->fmt );
			goto error;
		}

		if ( !ri.FS_Read( &entry->size, sizeof( entry->size ), fh ) ) {
			ri.Printf( PRINT_ERROR, "Error reading shader cache entry size at %u\n", i );
			goto error;
		}

		// the buffer shouldn't be that big
		if ( !entry->size || entry->size >= MAX_UINT ) {
			ri.Printf( PRINT_ERROR, "Cached SPIR-V object has funny size %i\n", entry->size );
			goto error;
		}

		entry->data = ri.Hunk_Alloc( entry->size, h_low );
		if ( !ri.FS_Read( entry->data, entry->size, fh ) ) {
			ri.Printf( PRINT_ERROR, "Error reading shader cache buffer at %u\n", i );
			goto error;
		}
	}

	ri.FS_FClose( fh );
	ri.Printf( PRINT_INFO, "Loaded %u cached SPIR-V shader objects.\n", cacheNumEntries );
	return;

error:
	ri.FS_FClose( fh );

	cacheNumEntries = 0;
	cacheHashTable = NULL;
	ri.Printf( PRINT_ERROR, "Error loading glshadercache.dat, clearing.\n" );
	ri.FS_HomeRemove( SHADER_CACHE_FILE_NAME );
	ri.FS_Remove( SHADER_CACHE_FILE_NAME );
}

static uint32_t R_GetShaderFromCache( shaderProgram_t *program )
{
	uint32_t i;

#ifdef _NOMAD_DEBUG
	return cacheNumEntries; // for speed development
#endif
	if ( !glContext.ARB_gl_spirv || !cacheHashTable || !r_useShaderCache->i ) {
		return cacheNumEntries;
	}

	ri.Printf( PRINT_INFO, "Searching for program '%s' in GLSL SPIR-V cache...\n", program->name );
	for ( i = 0; i < cacheNumEntries; i++ ) {
		if ( !N_stricmp( program->name, cacheHashTable[i].name ) ) {
			ri.Printf( PRINT_INFO, "...found.\n" );
			return i;
		}
	}

	return cacheNumEntries;
}

static void R_SaveShaderCache( void )
{
	shaderCacheEntry_t entry;
	GLint length;
	GLenum fmt;
	uint32_t i;
	fileHandle_t cacheFile;
	const shaderProgram_t *program;
	uint32_t ident;

	if ( !glContext.ARB_gl_spirv || !r_useShaderCache->i ) {
		return;
	}

	cacheFile = ri.FS_FOpenWrite( SHADER_CACHE_FILE_NAME );
	if ( cacheFile == FS_INVALID_HANDLE ) {
		ri.Printf( PRINT_ERROR, "Couldn't create file '%s' in write-only mode\n", SHADER_CACHE_FILE_NAME );
		return;
	}
	ident = SHADER_CACHE_HEADER;
	ri.FS_Write( &ident, sizeof( ident ), cacheFile );
	ri.FS_Write( &rg.numPrograms, sizeof( rg.numPrograms ), cacheFile );

	for ( i = 0; i < rg.numPrograms; i++ ) {
		memset( &entry, 0, sizeof( entry ) );
		program = rg.programs[i];

		nglGetProgramiv( program->programId, GL_PROGRAM_BINARY_LENGTH, &entry.size );
		entry.data = ri.Hunk_AllocateTempMemory( entry.size );

		nglGetProgramBinary( program->programId, entry.size, NULL, &entry.fmt, (char *)entry.data );

		ri.FS_Write( program->name, sizeof( program->name ), cacheFile );
		ri.FS_Write( &entry.fmt, sizeof( entry.fmt ), cacheFile );
		ri.FS_Write( &entry.size, sizeof( entry.size ), cacheFile );
		ri.FS_Write( entry.data, entry.size, cacheFile );
		
		GL_LogComment( "Wrote cached GLSL SPIR-V object binary \"%s\" to \"%s\", %u bytes, format is 0x%04x",
			program->name, SHADER_CACHE_FILE_NAME, entry.size, entry.fmt );

		ri.Hunk_FreeTempMemory( entry.data );
	}

	ri.FS_FClose( cacheFile );
}

typedef enum {
	GLSL_PRINTLOG_PROGRAM_INFO,
	GLSL_PRINTLOG_SHADER_INFO,
	GLSL_PRINTLOG_SHADER_SOURCE
} glslPrintLog_t;

static void GLSL_PrintLog(GLuint programOrShader, glslPrintLog_t type, qboolean developerOnly)
{
	static char     *msg;
	static char     msgPart[8192];
	GLsizei         maxLength = 0;
	uint32_t        i;
	const int       printLevel = developerOnly ? PRINT_DEVELOPER : PRINT_INFO;

	switch (type) {
	case GLSL_PRINTLOG_PROGRAM_INFO:
		ri.Printf(printLevel, "Program info log:\n");
		nglGetProgramiv(programOrShader, GL_INFO_LOG_LENGTH, &maxLength);
		break;
	case GLSL_PRINTLOG_SHADER_INFO:
		ri.Printf(printLevel, "Shader info log:\n");
		nglGetShaderiv(programOrShader, GL_INFO_LOG_LENGTH, &maxLength);
		break;
	case GLSL_PRINTLOG_SHADER_SOURCE:
		ri.Printf(printLevel, "Shader source:\n");
		nglGetShaderiv(programOrShader, GL_SHADER_SOURCE_LENGTH, &maxLength);
		break;
	};

	if (maxLength <= 0) {
		ri.Printf(printLevel, "None.\n");
		return;
	}

	if ( maxLength < 1023 ) {
		msg = msgPart;
	} else {
		msg = alloca( maxLength );
	}

	switch (type) {
	case GLSL_PRINTLOG_PROGRAM_INFO:
		nglGetProgramInfoLog(programOrShader, maxLength, &maxLength, msg);
		break;
	case GLSL_PRINTLOG_SHADER_INFO:
		nglGetShaderInfoLog(programOrShader, maxLength, &maxLength, msg);
		break;
	case GLSL_PRINTLOG_SHADER_SOURCE:
		nglGetShaderSource(programOrShader, maxLength, &maxLength, msg);
		break;
	};

	ri.Printf(printLevel, "%s", msg);

	if (maxLength < 1023) {
		msgPart[maxLength + 1] = '\0';

		ri.Printf(printLevel, "%s\n", msgPart);
	}
	else {
		for(i = 0; i < maxLength; i += 1023) {
			N_strncpyz(msgPart, msg + i, sizeof(msgPart));

			ri.Printf(printLevel, "%s", msgPart);
		}

		ri.Printf(printLevel, "\n");
	}
}

static int GLSL_CompileGPUShader( GLuint program, GLuint *prevShader, const GLchar *buffer, uint64_t size, GLenum shaderType,
	const char *programName )
{
	GLuint compiled;
	GLuint shader;

	// create shader
	shader = nglCreateShader( shaderType );

	switch ( shaderType ) {
	case GL_VERTEX_SHADER:
		GL_SetObjectDebugName( GL_SHADER, shader, programName, "_vertexShader" );
		break;
	case GL_FRAGMENT_SHADER:
		GL_SetObjectDebugName( GL_SHADER, shader, programName, "_fragmentShader" );
		break;
	case GL_GEOMETRY_SHADER:
		GL_SetObjectDebugName( GL_SHADER, shader, programName, "_geometryShader" );
		break;
	case GL_COMPUTE_SHADER:
		GL_SetObjectDebugName( GL_SHADER, shader, programName, "_computeShader" );
		break;
	case GL_TESS_CONTROL_SHADER:
		GL_SetObjectDebugName( GL_SHADER, shader, programName, "_tessControlShader" );
		break;
	case GL_TESS_EVALUATION_SHADER:
		GL_SetObjectDebugName( GL_SHADER, shader, programName, "_tessEvalShader" );
		break;
	default:
		ri.Error( ERR_FATAL, "GLSL_CompileGPUShader: invalid shader type %u", shaderType );
	};

	// give it the source
	nglShaderSource( shader, 1, (const GLchar **)&buffer, (const GLint *)&size );

	// compile
	nglCompileShader( shader );

	// check if shader compiled
	nglGetShaderiv( shader, GL_COMPILE_STATUS, &compiled );
	if ( !compiled ) {
		GLSL_PrintLog( shader, GLSL_PRINTLOG_SHADER_INFO, qfalse );
		ri.Error( ERR_DROP, "Failed to compile shader" );
		return qfalse;
	}

	// attach shader to program
	nglAttachShader( program, shader );

	*prevShader = shader;

	return qtrue;
}

static void GLSL_LinkProgram( GLuint program, int fromCache )
{
	GLint linked;

	nglLinkProgram( program );
	nglGetProgramiv( program, GL_LINK_STATUS, &linked );
	if( linked != GL_TRUE ) {
		GLSL_PrintLog( program, GLSL_PRINTLOG_PROGRAM_INFO, qfalse );
		ri.Error( ERR_DROP, "shaders failed to link" );
	}
}

static int GLSL_LoadGPUShaderText( const char *name, const char *fallback, GLenum shaderType, char *dest, uint64_t destSize )
{
	char filename[MAX_NPATH];
	GLchar *buffer = NULL;
	const GLchar *shaderText = NULL;
	uint64_t size;

	if ( shaderType == GL_VERTEX_SHADER ) {
		Com_snprintf( filename, sizeof( filename ) - 1, "shaders/%s_vp.glsl", name );
	}
	else if ( shaderType == GL_GEOMETRY_SHADER ) {
		Com_snprintf( filename, sizeof( filename ) - 1, "shaders/%s_geom.glsl", name );
	}
	else if ( shaderType == GL_COMPUTE_SHADER ) {
		Com_snprintf( filename, sizeof( filename ) - 1, "shaders/%s_cp.glsl", name );
	}
	else {
		Com_snprintf( filename, sizeof( filename ) - 1, "shaders/%s_fp.glsl", name );
	}

	if ( r_externalGLSL->i ) {
		size = ri.FS_LoadFile( filename, (void **)&buffer );
	}
	else {
		size = 0;
		buffer = NULL;
	}

	if ( !buffer ) {
		if ( fallback ) {
			ri.Printf( PRINT_DEVELOPER, "...loading built-in '%s'\n", filename );
			shaderText = fallback;
			size = strlen( shaderText );
		}
		else {
			ri.Printf( PRINT_DEVELOPER, "couldn't load '%s'\n", filename );
			return qfalse;
		}
	}
	else {
		ri.Printf( PRINT_DEVELOPER, "...loaded '%s'\n", filename );
		shaderText = buffer;
	}

	if ( size > destSize ) {
		return qfalse;
	}

	N_strncpyz( dest, shaderText, size + 1 );
	if ( buffer ) {
		ri.FS_FreeFile( buffer );
	}
	
	{
		char err[256];
		char *out = stb_include_string( dest, NULL, "gamedata/shaders/", filename, err );
		
		if ( !out ) {
			ri.Error( ERR_DROP, "Error loading shader code: %s", err );
		}
		if ( strlen( out ) > destSize ) {
			ri.Error( ERR_DROP, "Error loading shader code, output include string is too long" );
		}
		N_strncpyz( dest, out, destSize );

		free( out );
	}

	return qtrue;
}

static void GLSL_ShowProgramUniforms(GLuint program)
{
	uint32_t        i, j, count, size;
	GLenum			type;
	char            uniformName[1000];

	// query the number of active uniforms
	nglGetProgramiv( program, GL_ACTIVE_UNIFORMS, &count );

	// Loop over each of the active uniforms, and set their value
	for ( i = 0; i < count; i++ ) {
		nglGetActiveUniform( program, i, sizeof( uniformName ), NULL, &size, &type, uniformName );
		
		if ( N_stristr( uniformName, "u_LightData" ) || N_stristr( uniformName, "u_DLightData" ) ) {
			continue;
		}

		ri.Printf( PRINT_DEVELOPER, "active uniform: '%s'\n", uniformName );
	}
}


static void GLSL_PrepareHeader(GLenum shaderType, const GLchar *extra, char *dest, uint64_t size)
{
	float fbufWidthScale, fbufHeightScale;

	dest[0] = '\0';

	// OpenGL version from 3.3 and up have corresponding glsl versions
	if ( NGL_VERSION_ATLEAST( 3, 30 ) ) {
		N_strcat( dest, size, "#version 450 core\n" );
	}
	// otherwise, do the Quake3e method
	else if (GLSL_VERSION_ATLEAST(1, 30)) {
		if (GLSL_VERSION_ATLEAST(1, 50)) {
			N_strcat( dest, size, "#version 150\n" );
		}
		else if (GLSL_VERSION_ATLEAST(1, 30)) {
			N_strcat( dest, size, "#version 130\n" );
		}
	}
	else {
		N_strcat( dest, size, "#version 120\n" );
		N_strcat( dest, size, "#define GLSL_LEGACY\n" );
	}

	//
	// add any extensions
	//
	if ( r_loadTexturesOnDemand->i && glContext.bindlessTextures ) {
		N_strcat( dest, size, "#extension GL_ARB_bindless_texture : enable\n" );
		N_strcat( dest, size, "#define USE_BINDLESS_TEXTURE\n" );
		N_strcat( dest, size, "#define TEXTURE2D layout( bindless_sampler ) uniform sampler2D\n" );
	} else {
		N_strcat( dest, size, "#define TEXTURE2D uniform sampler2D\n" );
	}

	if ( !( NGL_VERSION_ATLEAST( 1, 30 ) ) ) {
		if ( shaderType == GL_VERTEX_SHADER ) {
			N_strcat( dest, size, "#define in attribute\n" );
			N_strcat( dest, size, "#define out varying\n" );
		}
		else {
			N_strcat( dest, size, "#define a_Color gl_FragColor\n" );
			N_strcat( dest, size, "#define in varying\n" );
			N_strcat( dest, size, "#define texture texture2D\n" ); // texture2D is deprecated in modern GLSL
		}
	}

	N_strcat(dest, size, "#ifndef M_PI\n#define M_PI 3.14159265358979323846\n#endif\n");

	N_strcat(dest, size,
					 va("#ifndef deformGen_t\n"
						"#define deformGen_t\n"
						"#define DGEN_WAVE_SIN %i\n"
						"#define DGEN_WAVE_SQUARE %i\n"
						"#define DGEN_WAVE_TRIANGLE %i\n"
						"#define DGEN_WAVE_SAWTOOTH %i\n"
						"#define DGEN_WAVE_INVERSE_SAWTOOTH %i\n"
						"#define DGEN_BULGE %i\n"
						"#define DGEN_MOVE %i\n"
						"#endif\n",
						DGEN_WAVE_SIN,
						DGEN_WAVE_SQUARE,
						DGEN_WAVE_TRIANGLE,
						DGEN_WAVE_SAWTOOTH,
						DGEN_WAVE_INVERSE_SAWTOOTH,
						DGEN_BULGE,
						DGEN_MOVE));

	N_strcat(dest, size,
					 va("#ifndef tcGen_t\n"
						"#define tcGen_t\n"
						"#define TCGEN_LIGHTMAP %i\n"
						"#define TCGEN_TEXTURE %i\n"
						"#define TCGEN_ENVIRONMENT_MAPPED %i\n"
						"#define TCGEN_FOG %i\n"
						"#define TCGEN_VECTOR %i\n"
						"#endif\n",
						TCGEN_LIGHTMAP,
						TCGEN_TEXTURE,
						TCGEN_ENVIRONMENT_MAPPED,
						TCGEN_FOG,
						TCGEN_VECTOR));

	N_strcat(dest, size,
					 va("#ifndef colorGen_t\n"
						"#define colorGen_t\n"
						"#define CGEN_LIGHTING_DIFFUSE %i\n"
						"#define CGEN_VERTEX %i\n"
						"#endif\n",
						CGEN_LIGHTING_DIFFUSE, CGEN_VERTEX ) );

	N_strcat(dest, size,
							 va("#ifndef alphaGen_t\n"
								"#define alphaGen_t\n"
								"#define AGEN_LIGHTING_SPECULAR %i\n"
								"#define AGEN_PORTAL %i\n"
								"#endif\n",
								AGEN_LIGHTING_SPECULAR,
								AGEN_PORTAL));
	
	N_strcat( dest, size,
							va( "#ifndef AntiAliasingType_t\n"
								"#define AntiAlias_FXAA %i\n"
								"#define AntiAlias_SMAA %i\n"
								"#define AntiAlias_SSAA %i\n"
								"#endif\n"
							, AntiAlias_FXAA, AntiAlias_SMAA, AntiAlias_SSAA ) );
	
	N_strcat( dest, size,
							va( "#ifndef ToneMapType_t\n"
								"#define ToneMap_Disabled 0\n"
								"#define ToneMap_Reinhard 1\n"
								"#define ToneMap_Exposure 2\n"
								"#endif\n" ) );
	

	// for #included files
	if ( shaderType == GL_VERTEX_SHADER ) {
		N_strcat( dest, size, "#define VERTEX_SHADER\n" );
	} else if ( shaderType == GL_FRAGMENT_SHADER ) {
		N_strcat( dest, size, "#define FRAGMENT_SHADER\n" );
	} else if ( shaderType == GL_COMPUTE_SHADER ) {
		N_strcat( dest, size, "#define COMPUTE_SHADER\n" );
	}

	N_strcat( dest, size, "#define QUALITY_LOW 0\n" );
	N_strcat( dest, size, "#define QUALITY_NORMAL 1\n" );
	N_strcat( dest, size, "#define QUALITY_HIGH 2\n" );

	N_strcat( dest, size, va( "#define MAX_TEXTURE_UNITS %i\n", glContext.maxTextureUnits ) );

	fbufWidthScale = 1.0f / ((float)glConfig.vidWidth);
	fbufHeightScale = 1.0f / ((float)glConfig.vidHeight);
	N_strcat(dest, size,
			 va("#ifndef r_FBufScale\n#define r_FBufScale vec2(%f, %f)\n#endif\n", fbufWidthScale, fbufHeightScale));

	if ( r_pbr->i ) {
		N_strcat( dest, size, "#define USE_PBR\n" );
	}

	if ( extra ) {
		N_strcat( dest, size, extra );
	}

	// OK we added a lot of stuff but if we do something bad in the GLSL shaders then we want the proper line
	// so we have to reset the line counting
	N_strcat(dest, size, "#line 0\n");
}

static void GLSL_CheckAttribLocation(GLuint id, const char *name, const char *attribName, int index)
{
	GLint location;

	location = nglGetAttribLocation(id, name);
	if (location != index) {
		ri.Error(ERR_FATAL, "GetAttribLocation(%s):%i != %s (%i)", name, location, attribName, index);
	}
}

static int GLSL_InitGPUShader2( shaderProgram_t *program, const char *name, uint32_t attribs, const char *vsCode, const char *fsCode, uint32_t fromCache )
{
	ri.Printf( PRINT_DEVELOPER, "---------- GPU Shader ----------\n" );

	if ( strlen( name ) >= MAX_NPATH ) {
		ri.Error(ERR_DROP, "GLSL_InitGPUShader2: \"%s\" is too long", name);
	}

	N_strncpyz( program->name, name, sizeof(program->name) );

	program->programId = nglCreateProgram();
	program->attribBits = attribs;

	GL_SetObjectDebugName( GL_PROGRAM, program->programId, name, "_program" );

	if ( fromCache == cacheNumEntries ) {
		if ( vsCode ) {
			if ( !( GLSL_CompileGPUShader( program->programId, &program->vertexId, vsCode, strlen(vsCode),
				GL_VERTEX_SHADER, name ) ) )
			{
				ri.Printf(PRINT_INFO, "GLSL_InitGPUShader2: Unable to load \"%s\" as GL_VERTEX_SHADER\n", name);
				nglDeleteProgram( program->programId );
				return qfalse;
			}
		}

		if ( fsCode ) {
			if (!(GLSL_CompileGPUShader( program->programId, &program->fragmentId, fsCode, strlen(fsCode),
				GL_FRAGMENT_SHADER, name ) ) )
			{
				ri.Printf(PRINT_INFO, "GLSL_InitGPUShader2: Unable to load \"%s\" as GL_FRAGMENT_SHADER\n", name);
				nglDeleteProgram( program->programId );
				return qfalse;
			}
		}

		GLSL_LinkProgram( program->programId, fromCache );
	} else {
		nglProgramBinary( program->programId, cacheHashTable[ fromCache ].fmt, cacheHashTable[ fromCache ].data, cacheHashTable[ fromCache ].size );
	}
	if ( attribs & ATTRIB_POSITION ) {
		nglBindAttribLocation( program->programId, ATTRIB_INDEX_POSITION, "a_Position" );
	}
	if ( attribs & ATTRIB_TEXCOORD ) {
		nglBindAttribLocation( program->programId, ATTRIB_INDEX_TEXCOORD, "a_TexCoords" );
	}
	if ( attribs & ATTRIB_COLOR ) {
		nglBindAttribLocation( program->programId, ATTRIB_INDEX_COLOR, "a_Color" );
	}
	if ( attribs & ATTRIB_WORLDPOS ) {
		nglBindAttribLocation( program->programId, ATTRIB_INDEX_WORLDPOS, "a_WorldPos" );
	}

	return qtrue;
}

static int GLSL_InitComputeShader2( shaderProgram_t *program, const char *name, const char *csCode )
{
	ri.Printf( PRINT_DEVELOPER, "---------- GPU Shader ----------\n" );

	if ( strlen( name ) >= MAX_NPATH ) {
		ri.Error( ERR_DROP, "GLSL_InitGPUShader2: \"%s\" is too long", name );
	}

	N_strncpyz( program->name, name, sizeof( program->name ) );

	program->programId = nglCreateProgram();

	GL_SetObjectDebugName( GL_PROGRAM, program->programId, name, "_program" );

	if ( csCode ) {
		if ( !( GLSL_CompileGPUShader( program->programId, &program->vertexId, csCode, strlen( csCode ),
			GL_COMPUTE_SHADER, name ) ) )
		{
			ri.Printf( PRINT_INFO, "GLSL_InitGPUShader2: Unable to load \"%s\" as GL_COMPUTE_SHADER\n", name );
			nglDeleteProgram( program->programId );
			return qfalse;
		}
	}

	GLSL_LinkProgram( program->programId, -1 );

	return qtrue;
}

static int GLSL_InitComputeShader( shaderProgram_t *program, const char *name, const GLchar *extra, qboolean addHeader, const char *fallback_cp )
{
	char csCode[32000];
	char *postHeader;
	uint64_t size;
	uint32_t fromCache = cacheNumEntries;

	rg.programs[rg.numPrograms] = program;
	rg.numPrograms++;

	N_strncpyz( program->name, name, sizeof( program->name ) - 1 );
	fromCache = R_GetShaderFromCache( program );
	if ( fromCache != cacheNumEntries ) {
		ri.Printf( PRINT_INFO, "GLSL Program '%s' loaded from shader cache.\n", name );
	}

	if ( fromCache == cacheNumEntries ) {
		size = sizeof( csCode );

		if ( addHeader ) {
			GLSL_PrepareHeader( GL_COMPUTE_SHADER, extra, csCode, size );
			postHeader = &csCode[strlen( csCode )];
			size -= strlen( csCode );
		}
		else {
			postHeader = &csCode[0];
		}

		if ( !GLSL_LoadGPUShaderText( name, fallback_cp, GL_COMPUTE_SHADER, postHeader, size ) ) {
			return qfalse;
		}
	}

	return GLSL_InitComputeShader2( program, name, csCode );
}

int GLSL_InitGPUShader( shaderProgram_t *program, const char *name, uint32_t attribs, qboolean fragmentShader,
	const GLchar *extra, qboolean addHeader, const char *fallback_vs, const char *fallback_fs )
{
	char vsCode[32000];
	char fsCode[32000];
	char *postHeader;
	uint64_t size;
	uint32_t fromCache = cacheNumEntries;

	rg.programs[rg.numPrograms] = program;
	rg.numPrograms++;

	N_strncpyz( program->name, name, sizeof( program->name ) - 1 );
	fromCache = R_GetShaderFromCache( program );
	if ( fromCache != cacheNumEntries ) {
		ri.Printf( PRINT_INFO, "GLSL Program '%s' loaded from shader cache.\n", name );
	}

	if ( fromCache == cacheNumEntries ) {
		size = sizeof( vsCode );

		if ( addHeader ) {
			GLSL_PrepareHeader(GL_VERTEX_SHADER, extra, vsCode, size);
			postHeader = &vsCode[strlen(vsCode)];
			size -= strlen(vsCode);
		}
		else {
			postHeader = &vsCode[0];
		}

		if (!GLSL_LoadGPUShaderText(name, fallback_vs, GL_VERTEX_SHADER, postHeader, size)) {
			return qfalse;
		}

		if ( fragmentShader ) {
			size = sizeof(fsCode);

			if (addHeader) {
				GLSL_PrepareHeader(GL_FRAGMENT_SHADER, extra, fsCode, size);
				postHeader = &fsCode[strlen(fsCode)];
				size -= strlen(fsCode);
			}
			else {
				postHeader = &fsCode[0];
			}

			if ( !GLSL_LoadGPUShaderText( name, fallback_fs, GL_FRAGMENT_SHADER, postHeader, size ) ) {
				return qfalse;
			}
		}
	}

	return GLSL_InitGPUShader2( program, name, attribs, vsCode, fsCode, fromCache );
}

void GLSL_InitUniforms( shaderProgram_t *program )
{
	uint32_t i, uniformBufferSize;

	GLint *uniforms = program->uniforms;
	uniformBufferSize = 0;

	for ( i = 0; i < UNIFORM_COUNT; i++ ) {
		uniforms[i] = nglGetUniformLocation( program->programId, uniformsInfo[i].name );

		if ( uniforms[i] == -1 ) {
			continue;
		}

		program->uniformBufferOffsets[i] = uniformBufferSize;
 
		switch ( uniformsInfo[i].type ) {
		case GLSL_TEXTURE:
			uniformBufferSize += sizeof( uintptr_t );
			break;
		case GLSL_INT:
			uniformBufferSize += sizeof( GLint );
			break;
		case GLSL_FLOAT:
			uniformBufferSize += sizeof( GLfloat );
			break;
		case GLSL_IVEC2:
		case GLSL_UVEC2:
		case GLSL_VEC2:
			uniformBufferSize += sizeof( vec2_t );
			break;
		case GLSL_IVEC3:
		case GLSL_UVEC3:
		case GLSL_VEC3:
			uniformBufferSize += sizeof( vec3_t );
			break;
		case GLSL_IVEC4:
		case GLSL_UVEC4:
		case GLSL_VEC4:
			uniformBufferSize += sizeof( vec4_t );
			break;
		case GLSL_VEC5:
			uniformBufferSize += sizeof( vec_t ) * 5;
			break;
		case GLSL_MAT16:
			uniformBufferSize += sizeof( mat4_t );
			break;
		case GLSL_BUFFER:
			// we store the block index instead of the data
			uniformBufferSize += sizeof( GLuint );
			break;
		default:
			break;
		};
	}

	program->uniformBuffer = (char *)ri.Hunk_Alloc( uniformBufferSize, h_low );

	for ( i = 0; i < UNIFORM_COUNT; i++ ) {
		if ( uniformsInfo[i].type == GLSL_BUFFER ) {
			*( (GLuint *)( program->uniformBuffer + program->uniformBufferOffsets[ i ] ) ) =
				nglGetUniformBlockIndex( program->programId, uniformsInfo[i].name );
		}
	}
}

void GLSL_FinishGPUShader( shaderProgram_t *program )
{
	GLSL_ShowProgramUniforms( program->programId );
	GL_CheckErrors();
}

void GLSL_DeleteGPUShader( shaderProgram_t *program )
{
	if ( program->programId ) {
		if ( program->vertexId ) {
		//	nglDetachShader( program->programId, program->vertexId );
			nglDeleteShader( program->vertexId );
		}
		if ( program->fragmentId ) {
		//	nglDetachShader( program->programId, program->fragmentId );
			nglDeleteShader( program->fragmentId );
		}

		nglDeleteProgram( program->programId );
	}
}

void GLSL_SetUniformTexture( shaderProgram_t *program, uint32_t uniformNum, texture_t *value )
{
	GLint *uniforms = program->uniforms;
	uintptr_t *compare = (uintptr_t *)( program->uniformBuffer + program->uniformBufferOffsets[uniformNum] );

	if ( uniforms[uniformNum] == -1 ) {
		return;
	}
	
	if ( uniformsInfo[uniformNum].type != GLSL_TEXTURE ) {
		ri.Printf(PRINT_INFO, COLOR_YELLOW "WARNING: GLSL_SetUniformTexture: wrong type for uniform %i in program %s\n", uniformNum, program->name);
		return;
	}
	if ( (uintptr_t)(void *)value == *compare ) {
		return;
	}
	
	*compare = (uintptr_t)(void *)value;
	if ( r_loadTexturesOnDemand->i ) {
		nglUniformHandleui64ARB( uniforms[ uniformNum ], value->handle );
	} else {
		nglUniform1i( uniforms[ uniformNum ], uniformNum );
	}
}

void GLSL_SetUniformInt( shaderProgram_t *program, uint32_t uniformNum, GLint value )
{
	GLint *uniforms = program->uniforms;
	GLint *compare = (GLint *)(program->uniformBuffer + program->uniformBufferOffsets[uniformNum]);

	if (uniforms[uniformNum] == -1)
		return;
	
	if (uniformsInfo[uniformNum].type != GLSL_INT) {
		ri.Printf(PRINT_INFO, COLOR_YELLOW "WARNING: GLSL_SetUniformInt: wrong type for uniform %i in program %s\n", uniformNum, program->name);
		return;
	}
	if (value == *compare)
		return;
	
	*compare = value;
	nglUniform1i(uniforms[uniformNum], value);
}

void GLSL_SetUniformFloat( shaderProgram_t *program, uint32_t uniformNum, GLfloat value )
{
	GLint *uniforms = program->uniforms;
	GLfloat *compare = (GLfloat *)( program->uniformBuffer + program->uniformBufferOffsets[uniformNum] );

	if ( uniforms[uniformNum] == -1 ) {
		return;
	}
	
	if ( uniformsInfo[uniformNum].type != GLSL_FLOAT ) {
		ri.Printf(PRINT_INFO, COLOR_YELLOW "WARNING: GLSL_SetUniformFloat: wrong type for uniform %i in program %s\n", uniformNum, program->name);
		return;
	}
	if ( value == *compare ) {
		return;
	}
	
	*compare = value;
	nglUniform1f(uniforms[uniformNum], value);
}

void GLSL_SetUniformUVec2(shaderProgram_t *program, uint32_t uniformNum, const uvec2_t v)
{
	GLint *uniforms = program->uniforms;
	uvec_t *compare = (uvec_t *)(program->uniformBuffer + program->uniformBufferOffsets[uniformNum]);

	if (uniforms[uniformNum] == -1)
		return;
	
	if (uniformsInfo[uniformNum].type != GLSL_UVEC2) {
		ri.Printf(PRINT_INFO, COLOR_YELLOW "WARNING: GLSL_SetUniformUVec2: wrong type for uniform %i in program %s\n", uniformNum, program->name);
		return;
	}
	if (v[0] == compare[0] && v[1] == compare[1])
		return;
	
	compare[0] = v[0];
	compare[1] = v[1];
	nglUniform2ui(uniforms[uniformNum], v[0], v[1]);
}

void GLSL_SetUniformUVec3(shaderProgram_t *program, uint32_t uniformNum, const uvec3_t v)
{
	GLint *uniforms = program->uniforms;
	uvec_t *compare = (uvec_t *)(program->uniformBuffer + program->uniformBufferOffsets[uniformNum]);

	if (uniforms[uniformNum] == -1)
		return;
	
	if (uniformsInfo[uniformNum].type != GLSL_UVEC3) {
		ri.Printf(PRINT_INFO, COLOR_YELLOW "WARNING: GLSL_SetUniformUVec3: wrong type for uniform %i in program %s\n", uniformNum, program->name);
		return;
	}
	if ( v[0] == compare[0] && v[1] == compare[1] && v[2] == compare[2] ) {
		return;
	}
	
	VectorCopy(compare, v);
	nglUniform3ui(uniforms[uniformNum], v[0], v[1], v[2]);
}

void GLSL_SetUniformUVec4(shaderProgram_t *program, uint32_t uniformNum, const uvec4_t v)
{
	GLint *uniforms = program->uniforms;
	uvec_t *compare = (uvec_t *)(program->uniformBuffer + program->uniformBufferOffsets[uniformNum]);

	if (uniforms[uniformNum] == -1)
		return;
	
	if (uniformsInfo[uniformNum].type != GLSL_UVEC4) {
		ri.Printf(PRINT_INFO, COLOR_YELLOW "WARNING: GLSL_SetUniformUVec4: wrong type for uniform %i in program %s\n", uniformNum, program->name);
		return;
	}
	if ( v[0] == compare[0] && v[1] == compare[1] && v[2] == compare[2] && v[3] == compare[3] ) {
		return;
	}
	
	VectorCopy4( compare, v );
	nglUniform4ui(uniforms[uniformNum], v[0], v[1], v[2], v[3]);
}

void GLSL_SetUniformIVec2(shaderProgram_t *program, uint32_t uniformNum, const ivec2_t v)
{
	GLint *uniforms = program->uniforms;
	ivec_t *compare = (ivec_t *)(program->uniformBuffer + program->uniformBufferOffsets[uniformNum]);

	if (uniforms[uniformNum] == -1)
		return;
	
	if (uniformsInfo[uniformNum].type != GLSL_IVEC2) {
		ri.Printf(PRINT_INFO, COLOR_YELLOW "WARNING: GLSL_SetUniformIVec2: wrong type for uniform %i in program %s\n", uniformNum, program->name);
		return;
	}
	if (v[0] == compare[0] && v[1] == compare[1])
		return;
	
	compare[0] = v[0];
	compare[1] = v[1];
	nglUniform2i(uniforms[uniformNum], v[0], v[1]);
}

void GLSL_SetUniformIVec3(shaderProgram_t *program, uint32_t uniformNum, const ivec3_t v)
{
	GLint *uniforms = program->uniforms;
	ivec_t *compare = (ivec_t *)(program->uniformBuffer + program->uniformBufferOffsets[uniformNum]);

	if (uniforms[uniformNum] == -1)
		return;
	
	if (uniformsInfo[uniformNum].type != GLSL_IVEC3) {
		ri.Printf(PRINT_INFO, COLOR_YELLOW "WARNING: GLSL_SetUniformIVec3: wrong type for uniform %i in program %s\n", uniformNum, program->name);
		return;
	}
	if ( v[0] == compare[0] && v[1] == compare[1] && v[2] == compare[2] ) {
		return;
	}
	
	VectorCopy(compare, v);
	nglUniform3i(uniforms[uniformNum], v[0], v[1], v[2]);
}

void GLSL_SetUniformIVec4(shaderProgram_t *program, uint32_t uniformNum, const ivec4_t v)
{
	GLint *uniforms = program->uniforms;
	ivec_t *compare = (ivec_t *)(program->uniformBuffer + program->uniformBufferOffsets[uniformNum]);

	if (uniforms[uniformNum] == -1)
		return;
	
	if (uniformsInfo[uniformNum].type != GLSL_IVEC4) {
		ri.Printf(PRINT_INFO, COLOR_YELLOW "WARNING: GLSL_SetUniformIVec4: wrong type for uniform %i in program %s\n", uniformNum, program->name);
		return;
	}
	if ( v[0] == compare[0] && v[1] == compare[1] && v[2] == compare[2] && v[3] == compare[3] ) {
		return;
	}
	
	VectorCopy4( compare, v );
	nglUniform4i(uniforms[uniformNum], v[0], v[1], v[2], v[3]);
}

void GLSL_SetUniformVec2(shaderProgram_t *program, uint32_t uniformNum, const vec2_t v)
{
	GLint *uniforms = program->uniforms;
	vec_t *compare = (vec_t *)(program->uniformBuffer + program->uniformBufferOffsets[uniformNum]);

	if (uniforms[uniformNum] == -1)
		return;
	
	if (uniformsInfo[uniformNum].type != GLSL_VEC2) {
		ri.Printf(PRINT_INFO, COLOR_YELLOW "WARNING: GLSL_SetUniformVec2: wrong type for uniform %i in program %s\n", uniformNum, program->name);
		return;
	}
	if (v[0] == compare[0] && v[1] == compare[1])
		return;
	
	compare[0] = v[0];
	compare[1] = v[1];
	nglUniform2f(uniforms[uniformNum], v[0], v[1]);
}

void GLSL_SetUniformVec3(shaderProgram_t *program, uint32_t uniformNum, const vec3_t v)
{
	GLint *uniforms = program->uniforms;
	vec_t *compare = (vec_t *)(program->uniformBuffer + program->uniformBufferOffsets[uniformNum]);

	if (uniforms[uniformNum] == -1)
		return;
	
	if (uniformsInfo[uniformNum].type != GLSL_VEC3) {
		ri.Printf(PRINT_INFO, COLOR_YELLOW "WARNING: GLSL_SetUniformVec3: wrong type for uniform %i in program %s\n", uniformNum, program->name);
		return;
	}
	if (VectorCompare(v, compare))
		return;
	
	VectorCopy(compare, v);
	nglUniform3f(uniforms[uniformNum], v[0], v[1], v[2]);
}

void GLSL_SetUniformVec4(shaderProgram_t *program, uint32_t uniformNum, const vec4_t v)
{
	GLint *uniforms = program->uniforms;
	vec_t *compare = (vec_t *)(program->uniformBuffer + program->uniformBufferOffsets[uniformNum]);

	if (uniforms[uniformNum] == -1)
		return;
	
	if (uniformsInfo[uniformNum].type != GLSL_VEC4) {
		ri.Printf(PRINT_INFO, COLOR_YELLOW "WARNING: GLSL_SetUniformVec4: wrong type for uniform %i in program %s\n", uniformNum, program->name);
		return;
	}
	if ( VectorCompare4( v, compare ) )
		return;
	
	VectorCopy4( compare, v );
	nglUniform4f(uniforms[uniformNum], v[0], v[1], v[2], v[3]);
}

void GLSL_SetUniformVec5( shaderProgram_t *program, uint32_t uniformNum, const float *v )
{
	GLint *uniforms = program->uniforms;
	vec_t *compare = (vec_t *)(program->uniformBuffer + program->uniformBufferOffsets[uniformNum]);

	if (uniforms[uniformNum] == -1)
		return;
	
	if ( uniformsInfo[uniformNum].type != GLSL_VEC5 ) {
		ri.Printf(PRINT_INFO, COLOR_YELLOW "WARNING: GLSL_SetUniformVec5: wrong type for uniform %i in program %s\n", uniformNum, program->name);
		return;
	}
	if ( !memcmp( compare, v, sizeof(vec_t) * 5 ) )
		return;
	
	memcpy( compare, v, sizeof(vec_t) * 5 );
	nglUniform1fv(uniforms[uniformNum], 5, v);
}

void GLSL_SetUniformMatrix4(shaderProgram_t *program, uint32_t uniformNum, const mat4_t m)
{
	GLint *uniforms = program->uniforms;
	mat4_t *compare = (mat4_t *)(program->uniformBuffer + program->uniformBufferOffsets[uniformNum]);

	if (uniforms[uniformNum] == -1)
		return;
	
	if (uniformsInfo[uniformNum].type != GLSL_MAT16) {
		ri.Printf(PRINT_INFO, COLOR_YELLOW "WARNING: GLSL_SetUniformMatrix4: wrong type for uniform %i in program %s\n", uniformNum, program->name);
		return;
	}
	if (Mat4Compare(m, *compare))
		return;
	
	memcpy( *compare, m, sizeof(mat4_t) );
	nglUniformMatrix4fv(uniforms[uniformNum], 1, GL_FALSE, &m[0][0]);
}


void GLSL_ShaderBufferData( shaderProgram_t *shader, uint32_t uniformNum, uniformBuffer_t *buffer, uint64_t nSize, uint64_t nOffset, qboolean dynamicStorage )
{
	GLint *uniforms;
	GLuint bufferObject;
	GLenum target;

	GLSL_UseProgram( shader );

	uniforms = shader->uniforms;
	bufferObject = buffer ? buffer->id : 0;

	if ( buffer->binding == -1 ) {
		ri.Printf( PRINT_WARNING, "Bad binding for uniform buffer '%s'\n", buffer->name );
		return;
	}

	if ( uniformsInfo[ uniformNum ].type != GLSL_BUFFER ) {
		ri.Printf( PRINT_INFO, COLOR_YELLOW "WARNING: GLSL_BindShaderBuffer: wrong type for uniform %i in program %s\n", uniformNum, shader->name );
		return;
	}

	nglBindBuffer( GL_SHADER_STORAGE_BUFFER, buffer->id );
	nglBufferSubData( GL_SHADER_STORAGE_BUFFER, nOffset, nSize, buffer->data );

//	void *data = nglMapBufferRange( GL_SHADER_STORAGE_BUFFER, nOffset, nSize, GL_MAP_WRITE_BIT | GL_MAP_UNSYNCHRONIZED_BIT );
//	if ( data ) {
//		memcpy( data, buffer->data, nSize );
//	}
//	nglUnmapBuffer( GL_SHADER_STORAGE_BUFFER );
	GL_CheckErrors();

	nglBindBuffer( GL_SHADER_STORAGE_BUFFER, 0 );
}

void GLSL_LinkUniformToShader( shaderProgram_t *program, uint32_t uniformNum, uniformBuffer_t *buffer, qboolean dynamicStorage, uint32_t binding )
{
	GLint *uniforms = program->uniforms;
	GLuint *compare = (GLuint *)( program->uniformBuffer + program->uniformBufferOffsets[ uniformNum ] );
	GLenum target;

	GLSL_UseProgram( program );

//	buffer->binding = nglGetUniformBlockIndex( program->programId, uniformsInfo[ uniformNum ].name );
//	if ( buffer->binding == -1 ) {
//		ri.Printf( PRINT_WARNING, "GLSL_LinkUniformToShader: uniformBuffer %s not in program '%s'\n", buffer->name, program->name );
//		return;
//	}
//	nglUniformBlockBinding( program->programId, buffer->binding, program->numBuffers );

	target = GL_SHADER_STORAGE_BUFFER;

	nglBindBuffer( GL_SHADER_STORAGE_BUFFER, buffer->id );
	nglBindBufferRange( GL_SHADER_STORAGE_BUFFER, 0, buffer->id, 0, buffer->size );
	nglBindBufferBase( GL_SHADER_STORAGE_BUFFER, 0, buffer->id );
	nglBindBuffer( GL_SHADER_STORAGE_BUFFER, 0 );

	GL_CheckErrors();
	
	GLSL_UseProgram( NULL );

	program->numBuffers++;

	ri.Printf( PRINT_DEVELOPER, "GLSL_LinkUniformToShader: linked uniform buffer object '%s' to GLSL program '%s'\n",
		buffer->name, program->name );
}

uniformBuffer_t *GLSL_InitUniformBuffer( const char *name, byte *buffer, uint64_t bufSize, qboolean dynamicStorage )
{
	uint64_t size;
	uint64_t nameLen;
	GLenum target;
	uniformBuffer_t *buf;

	nameLen = strlen( name );
	if ( nameLen >= MAX_NPATH ) {
		ri.Error( ERR_FATAL, "GLSL_InitUniformBuffer: name '%s' too long", name );
	}

	// this should never happen
	if ( rg.numUniformBuffers == MAX_UNIFORM_BUFFERS ) {
		ri.Error( ERR_FATAL, "GLSL_InitUniformBuffer: MAX_UNIFORM_BUFFERS hit" );
	}

	size = 0;
	size += PAD( nameLen + 1, sizeof( uintptr_t ) );
	size += PAD( sizeof( *buf ), sizeof( uintptr_t ) );

	buf = rg.uniformBuffers[ rg.numUniformBuffers ] = (uniformBuffer_t *)ri.Hunk_Alloc( size, h_low );
	memset( buf, 0, size );

	buf->name = (char *)( buf + 1 );
	buf->size = bufSize;
	strcpy( buf->name, name );

	target = GL_SHADER_STORAGE_BUFFER;

	// generate buffer
	nglGenBuffers( 1, &buf->id );
	nglBindBuffer( GL_SHADER_STORAGE_BUFFER, buf->id );

	// good 'ol fashioned buffers
	buf->data = ri.Hunk_Alloc( bufSize, h_low );
	nglBufferData( GL_SHADER_STORAGE_BUFFER, bufSize, buffer, GL_STREAM_DRAW );
	nglBindBuffer( GL_SHADER_STORAGE_BUFFER, 0 );

	rg.numUniformBuffers++;

	return buf;
}

void GLSL_InitGPUShaders_f( void )
{
	uint64_t start, end;
	uint64_t i;
	uint32_t attribs;
	uint32_t numEtcShaders = 0, numGenShaders = 0, numLightShaders = 0;
	char extradefines[MAX_STRING_CHARS];

	rg.numPrograms = 0;

	ri.Printf( PRINT_INFO, "---- GLSL_InitGPUShaders ----\n" );

	R_IssuePendingRenderCommands();

	R_LoadShaderCache();

	start = ri.Milliseconds();

	for ( i = 0; i < GENERICDEF_COUNT; i++ ) {
		qboolean fastLight = !( r_normalMapping->i || r_specularMapping->i || r_bloom->i );

		attribs = ATTRIB_POSITION | ATTRIB_TEXCOORD | ATTRIB_COLOR;

		extradefines[0] = '\0';

		N_strcat( extradefines, sizeof( extradefines ) - 1, va( "#define MAX_TEXTURES %i\n", MAX_TEXTURES ) );
		N_strcat( extradefines, sizeof( extradefines ) - 1, va( "#define MAX_MAP_LIGHTS %i\n", MAX_MAP_LIGHTS ) );
		N_strcat( extradefines, sizeof( extradefines ) - 1, va( "#define POINT_LIGHT %i\n", LIGHT_POINT ) );
		N_strcat( extradefines, sizeof( extradefines ) - 1, va( "#define DIRECTION_LIGHT %i\n", LIGHT_DIRECTIONAL ) );

		N_strcat( extradefines, sizeof( extradefines ) - 1, "#define USE_LIGHT\n" );
		if ( fastLight ) {
			N_strcat( extradefines, sizeof( extradefines ) - 1, "#define USE_FAST_LIGHT\n" );
		}

		if ( i & GENERICDEF_USE_DEFORM_VERTEXES ) {
			N_strcat( extradefines, sizeof( extradefines ) - 1, "#define USE_DEFORM_VERTEXES\n" );
		}
		if ( i & GENERICDEF_USE_TCGEN_AND_TCMOD ) {
			N_strcat( extradefines, sizeof( extradefines ) - 1, "#define USE_TCMOD\n" );
			N_strcat( extradefines, sizeof( extradefines ) - 1, "#define USE_TCGEN\n" );
		}
		N_strcat( extradefines, sizeof( extradefines ) - 1, "#define USE_RGBAGEN\n" );

		if ( !GLSL_InitGPUShader( &rg.genericShader[i], "generic", attribs, qtrue, extradefines, qtrue, fallbackShader_generic_vp,
			fallbackShader_generic_fp ) )
		{
			ri.Error( ERR_FATAL, "Could not load generic shader!" );
		}

		GLSL_InitUniforms( &rg.genericShader[i] );
		GLSL_FinishGPUShader( &rg.genericShader[i] );

		numGenShaders++;
	}

	extradefines[ 0 ] = '\0';
	attribs = ATTRIB_POSITION | ATTRIB_TEXCOORD;
	if ( !GLSL_InitGPUShader( &rg.textureColorShader, "texturecolor", attribs, qtrue, extradefines, qtrue, fallbackShader_texturecolor_vp,
		fallbackShader_texturecolor_fp ) )
	{
		ri.Error( ERR_FATAL, "Could not load texturecolor shader!" );
	}
	GLSL_InitUniforms( &rg.textureColorShader );
	GLSL_FinishGPUShader( &rg.textureColorShader );

	numGenShaders++;

	attribs = ATTRIB_POSITION | ATTRIB_TEXCOORD | ATTRIB_COLOR;
	extradefines[0] = '\0';
	N_strcat( extradefines, sizeof( extradefines ) - 1, "#define USE_TCGEN\n" );
	N_strcat( extradefines, sizeof( extradefines ) - 1, "#define USE_TCMOD\n" );
	if ( !GLSL_InitGPUShader( &rg.imguiShader, "imgui", attribs, qtrue, extradefines, qtrue, fallbackShader_imgui_vp, fallbackShader_imgui_fp ) ) {
		ri.Error( ERR_FATAL, "Could not load imgui shader!" );
	}
	GLSL_InitUniforms( &rg.imguiShader );
	GLSL_FinishGPUShader( &rg.imguiShader );

	numGenShaders++;

	extradefines[ 0 ] = '\0';
	attribs = ATTRIB_POSITION | ATTRIB_TEXCOORD;
	if ( !GLSL_InitGPUShader( &rg.blurShader, "blur", attribs, qtrue, extradefines, qtrue, fallbackShader_blur_vp, fallbackShader_blur_fp ) ) {
		ri.Error( ERR_FATAL, "Could not load blur shader!" );
	}
	GLSL_InitUniforms( &rg.blurShader );
	GLSL_FinishGPUShader( &rg.blurShader );

	numGenShaders++;

	extradefines[ 0 ] = '\0';
	attribs = ATTRIB_POSITION | ATTRIB_TEXCOORD;
	if ( !GLSL_InitGPUShader( &rg.gaussianShader, "gaussian", attribs, qtrue, extradefines, qtrue, fallbackShader_gaussian_vp, fallbackShader_gaussian_fp ) ) {
		ri.Error( ERR_FATAL, "Could not load gaussian shader!" );
	}
	GLSL_InitUniforms( &rg.gaussianShader );
	GLSL_FinishGPUShader( &rg.gaussianShader );

	numGenShaders++;

	extradefines[ 0 ] = '\0';
	attribs = ATTRIB_POSITION | ATTRIB_TEXCOORD;
	if ( !GLSL_InitGPUShader( &rg.bloomResolveShader, "bloom", attribs, qtrue, extradefines, qtrue, fallbackShader_bloom_vp, fallbackShader_bloom_fp ) ) {
		ri.Error( ERR_FATAL, "Could not load bloom shader!" );
	}
	GLSL_InitUniforms( &rg.bloomResolveShader );
	GLSL_FinishGPUShader( &rg.bloomResolveShader );

	numGenShaders++;

	R_SaveShaderCache();

	end = ri.Milliseconds();

	ri.Printf( PRINT_INFO, "...loaded %u GLSL shaders (%u gen %u etc %u light) in %5.2f seconds\n",
		rg.numPrograms, numGenShaders, numEtcShaders, numLightShaders, ( end - start ) / 1000.0f );
}

void GLSL_InitGPUShaders( void )
{
	ri.Cmd_AddCommand( "gpushaders_init", GLSL_InitGPUShaders_f );

	GLSL_InitGPUShaders_f();
}

void GLSL_ShutdownGPUShaders( void )
{
	uint32_t i;

	ri.Printf( PRINT_INFO, "---------- GLSL_ShutdownGPUShaders -----------\n" );
	ri.Cmd_RemoveCommand( "gpushaders_init" );

	for ( i = 0; i < ATTRIB_INDEX_COUNT; i++ ) {
		nglDisableVertexAttribArray( i );
	}

	R_SaveShaderCache();
	
	GL_BindNullProgram();

	GLSL_DeleteGPUShader( &rg.imguiShader );
	GLSL_DeleteGPUShader( &rg.textureColorShader );
	GLSL_DeleteGPUShader( &rg.bloomResolveShader );
	GLSL_DeleteGPUShader( &rg.blurShader );
	
	for ( i = 0; i < rg.numUniformBuffers; i++ ) {
		nglBindBuffer( GL_SHADER_STORAGE_BUFFER, rg.uniformBuffers[i]->id );
		if ( NGL_VERSION_ATLEAST( 4, 4 ) || ( glContext.ARB_buffer_storage && glContext.ARB_map_buffer_range ) ) {
			nglUnmapBuffer( GL_SHADER_STORAGE_BUFFER );
		}
		nglBindBuffer( GL_SHADER_STORAGE_BUFFER, 0 );
		nglDeleteBuffers( 1, &rg.uniformBuffers[i]->id );
	}
	if ( rg.world ) {
		GLSL_DeleteGPUShader( &rg.tileShader );
	}

	for ( i = 0; i < GENERICDEF_COUNT; i++ ) {
		GLSL_DeleteGPUShader( &rg.genericShader[i] );
	}

	cacheHashTable = NULL;
	cacheNumEntries = 0;
}

void GLSL_UseProgram( shaderProgram_t *program )
{
	const GLuint programObject = program ? program->programId : 0;

	if ( GL_UseProgram( programObject ) ) {
		backend.pc.c_glslShaderBinds++;
		glState.currentShader = program;
	}
}

shaderProgram_t *GLSL_GetGenericShaderProgram( int stage )
{
	const shaderStage_t *pStage = backend.drawBatch.shader->stages[stage];
	int shaderAttribs = 0;

	switch ( pStage->rgbGen ) {
	case CGEN_LIGHTING_DIFFUSE:
		shaderAttribs |= GENERICDEF_USE_RGBAGEN;
		break;
	default:
		break;
	};

	switch ( pStage->alphaGen ) {
	case AGEN_LIGHTING_SPECULAR:
	case AGEN_PORTAL:
		shaderAttribs |= GENERICDEF_USE_RGBAGEN;
		break;
	default:
		break;
	};

	if ( backend.drawBatch.shader->numDeforms == 0 && pStage->bundle[0].numTexMods == 0 ) {
		return &rg.genericShader[shaderAttribs];
	}

	if ( pStage->bundle[0].tcGen != TCGEN_TEXTURE ) {
		shaderAttribs |= GENERICDEF_USE_TCGEN_AND_TCMOD;
	}

	if ( backend.drawBatch.shader->numDeforms && !ShaderRequiresCPUDeforms( backend.drawBatch.shader ) ) {
		shaderAttribs |= GENERICDEF_USE_DEFORM_VERTEXES;
	}

	if ( pStage->bundle[0].numTexMods ) {
		shaderAttribs |= GENERICDEF_USE_TCGEN_AND_TCMOD;
	}

	return &rg.genericShader[ shaderAttribs ];
}
