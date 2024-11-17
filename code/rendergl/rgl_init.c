#include "rgl_local.h"
#define _GNU_SOURCE
#include <pthread.h>

typedef uintptr_t ImDrawData;
#include "../rendercommon/imgui_impl_opengl3.h"

#define NGL( ret, name, ... ) PFN ## name n ## name = NULL;
NGL_Core_Procs
NGL_Debug_Procs
NGL_Buffer_Procs
NGL_FBO_Procs
NGL_Shader_Procs
NGL_GLSL_SPIRV_Procs
NGL_Texture_Procs
NGL_VertexArray_Procs
NGL_BufferARB_Procs
NGL_VertexArrayARB_Procs
NGL_VertexShaderARB_Procs

NGL_ARB_buffer_storage
NGL_ARB_map_buffer_range
NGL_ARB_sync
NGL_ARB_bindless_texture
NGL_ARB_transform_feedback
NGL_ARB_direct_state_access
#undef NGL

// because they're edgy...
void R_GLDebug_Callback_AMD(GLuint id, GLenum category, GLenum severity, GLsizei length, const GLchar *message, GLvoid *userParam);
// the normal stuff
void R_GLDebug_Callback_ARB(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message, const GLvoid *userParam);

char gl_extensions[32768];

cvar_t *r_measureOverdraw;

cvar_t *r_fastsky;
cvar_t *r_drawSun;
cvar_t *r_dynamiclight;
cvar_t *r_dlightBacks;

cvar_t *r_norefresh;
cvar_t *r_drawentities;
cvar_t *r_drawworld;
cvar_t *r_speeds;
cvar_t *r_detailTextures;

cvar_t *r_cameraExposure;

cvar_t *r_gammaAmount;

cvar_t *r_singleShader;
cvar_t *r_roundImagesDown;
cvar_t *r_picmip;
cvar_t *r_finish;
cvar_t *r_textureMode;

cvar_t *r_fullbright;
cvar_t *r_lightmap;
cvar_t *r_vertexLight;

cvar_t *r_showTris;

cvar_t *r_showSky;
cvar_t *r_clear;

cvar_t *r_shadows;
cvar_t *r_flares;

cvar_t *r_intensity;

cvar_t *r_skipBackEnd;

cvar_t *r_externalGLSL;

cvar_t *r_hdr;
cvar_t *r_floatLightmap;
cvar_t *r_postProcess;
cvar_t *r_lightmap;

cvar_t *r_toneMap;
cvar_t *r_forceToneMap;
cvar_t *r_forceToneMapMin;
cvar_t *r_forceToneMapAvg;
cvar_t *r_forceToneMapMax;

cvar_t  *r_autoExposure;
cvar_t  *r_forceAutoExposure;
cvar_t  *r_forceAutoExposureMin;
cvar_t  *r_forceAutoExposureMax;

cvar_t *r_depthPrepass;
cvar_t *r_ssao;
cvar_t *r_bloom;

cvar_t *r_paused;

cvar_t *r_clearColor;

cvar_t *r_normalMapping;
cvar_t *r_specularMapping;
cvar_t *r_deluxeMapping;
cvar_t *r_parallaxMapping;
cvar_t *r_parallaxMapOffset;
cvar_t *r_parallaxMapShadows;
cvar_t *r_cubeMapping;
cvar_t *r_cubemapSize;
cvar_t *r_deluxeSpecular;
cvar_t *r_pbr;
cvar_t *r_baseNormalX;
cvar_t *r_baseNormalY;
cvar_t *r_baseParallax;
cvar_t *r_baseSpecular;
cvar_t *r_baseGloss;
cvar_t *r_glossType;
cvar_t *r_dlightMode;
cvar_t *r_pshadowDist;
cvar_t *r_mergeLightmaps;
cvar_t *r_imageUpsample;
cvar_t *r_imageUpsampleMaxSize;
cvar_t *r_imageUpsampleType;
cvar_t *r_genNormalMaps;
cvar_t *r_forceSun;
cvar_t *r_forceSunLightScale;
cvar_t *r_forceSunAmbientScale;
cvar_t *r_sunlightMode;
cvar_t *r_drawSunRays;
cvar_t *r_sunShadows;
cvar_t *r_shadowFilter;
cvar_t *r_shadowBlur;
cvar_t *r_shadowMapSize;
cvar_t *r_shadowCascadeZNear;
cvar_t *r_shadowCascadeZFar;
cvar_t *r_shadowCascadeZBias;
cvar_t *r_ignoreDstAlpha;

cvar_t *r_greyscale;

cvar_t *r_ignoreGLErrors;

cvar_t *r_overBrightBits;
cvar_t *r_mapOverBrightBits;

cvar_t *r_imageSharpenAmount;

cvar_t *r_smaaEdgesType;
cvar_t *r_showImages;

cvar_t *r_printShaders;

cvar_t *r_useExtensions;
cvar_t *r_allowLegacy;
cvar_t *r_allowShaders;
cvar_t *r_multisampleType;
cvar_t *r_ignorehwgamma;
cvar_t *r_drawMode;
cvar_t *r_glDebug;
cvar_t *r_textureBits;
cvar_t *r_stencilBits;
cvar_t *r_textureDetail;
cvar_t *r_drawBuffer;
cvar_t *r_customWidth;
cvar_t *r_customHeight;
cvar_t *r_glDiagnostics;
cvar_t *r_colorMipLevels;

cvar_t *r_fixedRendering;
cvar_t *r_fixedResolutionScale;

cvar_t *r_maxPolys;
cvar_t *r_maxEntities;
cvar_t *r_maxDLights;

cvar_t *r_imageUpsampleType;
cvar_t *r_imageUpsample;
cvar_t *r_imageUpsampleMaxSize;

cvar_t *r_useShaderCache;

cvar_t *r_lightingQuality;
cvar_t *r_antialiasQuality;

cvar_t *r_arb_compute_shader;

cvar_t *r_loadTexturesOnDemand;

cvar_t *sys_forceSingleThreading;

// OpenGL extensions
cvar_t *r_arb_texture_compression;
cvar_t *r_arb_framebuffer_object;
cvar_t *r_arb_vertex_array_object;
cvar_t *r_arb_vertex_buffer_object;
cvar_t *r_arb_texture_filter_anisotropic;
cvar_t *r_arb_texture_max_anisotropy;
cvar_t *r_arb_texture_float;
cvar_t *r_arb_sync;
cvar_t *r_arb_shader_storage_buffer_object;
cvar_t *r_arb_map_buffer_range;
cvar_t *r_arb_pixel_buffer_object;
cvar_t *r_arb_direct_state_access;

cvar_t *r_screenshotJpegQuality;

static cvar_t *r_debugCamera;

renderGlobals_t rg;
glstate_t glState;
gpuConfig_t glConfig;
glContext_t glContext;
renderBackend_t backend;

refimport_t ri;

qboolean R_HasExtension(const char *ext)
{
	const char *ptr = N_stristr( gl_extensions, ext );
	if (ptr == NULL)
		return qfalse;
	ptr += strlen(ext);
	return ((*ptr == ' ') || (*ptr == '\0'));  // verify its complete string.
}

/*
================
R_PrintLongString

Workaround for ri.Printf's 8192 characters buffer limit.
================
*/
static void R_PrintLongString(const char *string)
{
	char buffer[8192];
	const char *p;
	size_t size = strlen(string);

	p = string;
	while(size > 0) {
		N_strncpyz(buffer, p, sizeof (buffer) );
		ri.Printf( PRINT_DEVELOPER, "%s", buffer );
		p += 1023;
		size -= 1023;
	}
}

static void GpuMemInfo_f(void)
{
	switch (glContext.memInfo) {
	case MI_NONE:
		ri.Printf(PRINT_INFO, "No extension found for GPU memory info.\n");
		break;
	case MI_NVX:
		{
			int value;
			int currentVidMem;
			int totalVidMem;

			nglGetIntegerv(GL_GPU_MEMORY_INFO_DEDICATED_VIDMEM_NVX, &value);
			ri.Printf(PRINT_DEVELOPER, "GPU_MEMORY_INFO_DEDICATED_VIDMEM_NVX: %i KiB\n", value);

			nglGetIntegerv(GL_GPU_MEMORY_INFO_TOTAL_AVAILABLE_MEMORY_NVX, &value);
			ri.Printf(PRINT_DEVELOPER, "GPU_MEMORY_INFO_TOTAL_AVAILABLE_MEMORY_NVX: %i KiB\n", value);

			nglGetIntegerv(GL_GPU_MEMORY_INFO_CURRENT_AVAILABLE_VIDMEM_NVX, &value);
			ri.Printf(PRINT_DEVELOPER, "GPU_MEMORY_INFO_CURRENT_AVAILABLE_VIDMEM_NVX: %i KiB\n", value);

			nglGetIntegerv(GL_GPU_MEMORY_INFO_EVICTION_COUNT_NVX, &value);
			ri.Printf(PRINT_DEVELOPER, "GPU_MEMORY_INFO_EVICTION_COUNT_NVX: %i\n", value);

			nglGetIntegerv(GL_GPU_MEMORY_INFO_EVICTED_MEMORY_NVX, &value);
			ri.Printf(PRINT_DEVELOPER, "GPU_MEMORY_INFO_EVICTED_MEMORY_NVX: %i KiB\n", value);

			ri.Printf(PRINT_DEVELOPER, "------------------------------------------------\n");

			// for those who don't want to read the eyeball barf
			nglGetIntegerv(GL_GPU_MEMORY_INFO_DEDICATED_VIDMEM_NVX, &totalVidMem);
			ri.Printf(PRINT_INFO, "Total Dedicated Video Memory: %i KiB\n", totalVidMem);

			nglGetIntegerv(GL_GPU_MEMORY_INFO_TOTAL_AVAILABLE_MEMORY_NVX, &value);
			ri.Printf(PRINT_INFO, "Total Available GPU Memory: %i KiB\n", value);

			nglGetIntegerv(GL_GPU_MEMORY_INFO_CURRENT_AVAILABLE_VIDMEM_NVX, &currentVidMem);
			ri.Printf(PRINT_INFO, "Total Current Dedicated Video Memory: %i KiB\n", currentVidMem);

			ri.Printf(PRINT_INFO, "Total Used Dedicated Video Memory: %i KiB\n", totalVidMem - currentVidMem);

			nglGetIntegerv(GL_GPU_MEMORY_INFO_EVICTION_COUNT_NVX, &value);
			ri.Printf(PRINT_INFO, "Total Evictions: %i\n", value);

			nglGetIntegerv(GL_GPU_MEMORY_INFO_EVICTED_MEMORY_NVX, &value);
			ri.Printf(PRINT_INFO, "Total Evicted Memory: %i KiB\n", value);
		}
		break;
	case MI_ATI:
		{
			int value[4];

			nglGetIntegerv(GL_VBO_FREE_MEMORY_ATI, &value[0]);
			ri.Printf(PRINT_INFO, "VBO_FREE_MEMORY_ATI: %i KiB total %i KiB largest aux: %i KiB total %i KiB largest\n", value[0], value[1], value[2], value[3]);

			nglGetIntegerv(GL_TEXTURE_FREE_MEMORY_ATI, &value[0]);
			ri.Printf(PRINT_INFO, "TEXTURE_FREE_MEMORY_ATI: %i KiB total %i KiB largest aux: %i KiB total %i KiB largest\n", value[0], value[1], value[2], value[3]);

			nglGetIntegerv(GL_RENDERBUFFER_FREE_MEMORY_ATI, &value[0]);
			ri.Printf(PRINT_INFO, "RENDERBUFFER_FREE_MEMORY_ATI: %i KiB total %i KiB largest aux: %i KiB total %i KiB largest\n", value[0], value[1], value[2], value[3]);
		}
		break;
	};
}


/* 
============================================================================== 
 
						SCREEN SHOTS 

NOTE TTimo
some thoughts about the screenshots system:
screenshots get written in fs_homepath + fs_gamedir
vanilla q3 .. baseq3/screenshots/ *.tga
team arena .. missionpack/screenshots/ *.tga

two commands: "screenshot" and "screenshotJPEG"
we use statics to store a count and start writing the first screenshot/screenshot????.tga (.jpg) available
(with FS_FileExists / FS_FOpenFileWrite calls)
FIXME: the statics don't get a reinit between fs_game changes

============================================================================== 
*/ 

/* 
================== 
RB_ReadPixels

Reads an image but takes care of alignment issues for reading RGB images.

Reads a minimum offset for where the RGB data starts in the image from
integer stored at pointer offset. When the function has returned the actual
offset was written back to address offset. This address will always have an
alignment of packAlign to ensure efficient copying.

Stores the length of padding after a line of pixels to address padlen

Return value must be freed with ri.Hunk_FreeTempMemory()
================== 
*/  

byte *RB_ReadPixels( int x, int y, int width, int height, size_t *offset, int *padlen )
{
	byte *buffer, *bufstart;
	int padwidth, linelen, bytesPerPixel;
	int yin, xin, xout;
	GLint packAlign, format;

	// OpenGL ES is only required to support reading GL_RGBA
	if ( 0 ) {
		format = GL_RGBA;
		bytesPerPixel = 4;
	} else {
		format = GL_RGB;
		bytesPerPixel = 3;
	}

	nglGetIntegerv(GL_PACK_ALIGNMENT, &packAlign);

	linelen = width * bytesPerPixel;
	padwidth = PAD(linelen, packAlign);

	// Allocate a few more bytes so that we can choose an alignment we like
	buffer = ri.Hunk_AllocateTempMemory(padwidth * height + *offset + packAlign - 1);

	bufstart = PADP((intptr_t) buffer + *offset, packAlign);
	nglReadPixels(x, y, width, height, format, GL_UNSIGNED_BYTE, bufstart);

	linelen = width * 3;

	// Convert RGBA to RGB, in place, line by line
	if (format == GL_RGBA) {
		for (yin = 0; yin < height; yin++) {
			for (xin = 0, xout = 0; xout < linelen; xin += 4, xout += 3) {
				bufstart[yin*padwidth + xout + 0] = bufstart[yin*padwidth + xin + 0];
				bufstart[yin*padwidth + xout + 1] = bufstart[yin*padwidth + xin + 1];
				bufstart[yin*padwidth + xout + 2] = bufstart[yin*padwidth + xin + 2];
			}
		}
	}

	*offset = bufstart - buffer;
	*padlen = padwidth - linelen;
	
	return buffer;
}

/* 
================== 
RB_TakeScreenshot
================== 
*/
void RB_TakeScreenshot( int x, int y, int width, int height, const char *fileName )
{
	byte *allbuf, *buffer;
	byte *srcptr, *destptr;
	byte *endline, *endmem;
	byte temp;
	
	int linelen, padlen;
	size_t offset = 18, memcount;
		
	allbuf = RB_ReadPixels( x, y, width, height, &offset, &padlen );
	buffer = allbuf + offset - 18;
	
	memset( buffer, 0, 18 );
	buffer[2] = 2;		// uncompressed type
	buffer[12] = width & 255;
	buffer[13] = width >> 8;
	buffer[14] = height & 255;
	buffer[15] = height >> 8;
	buffer[16] = 24;	// pixel size

	// swap rgb to bgr and remove padding from line endings
	linelen = width * 3;
	
	srcptr = destptr = allbuf + offset;
	endmem = srcptr + ( linelen + padlen ) * height;
	
	while ( srcptr < endmem ) {
		endline = srcptr + linelen;

		while ( srcptr < endline ) {
			temp = srcptr[0];
			*destptr++ = srcptr[2];
			*destptr++ = srcptr[1];
			*destptr++ = temp;
			
			srcptr += 3;
		}
		
		// Skip the pad
		srcptr += padlen;
	}

	memcount = linelen * height;

	// gamma correction
	if ( glConfig.deviceSupportsGamma ) {
		R_GammaCorrect( allbuf + offset, memcount );
	}

	ri.FS_WriteFile( fileName, buffer, memcount + 18 );

	ri.Hunk_FreeTempMemory( allbuf );
}

/* 
================== 
RB_TakeScreenshotJPEG
================== 
*/

static void RB_TakeScreenshotJPEG( int x, int y, int width, int height, const char *fileName )
{
	byte *buffer;
	size_t offset = 0, memcount;
	int padlen;

	buffer = RB_ReadPixels( x, y, width, height, &offset, &padlen );
	memcount = ( width * 3 + padlen ) * height;

	// gamma correction
	if ( glConfig.deviceSupportsGamma ) {
		R_GammaCorrect( buffer + offset, memcount );
	}

	ri.G_SaveJPG( fileName, r_screenshotJpegQuality->i, width, height, buffer + offset, padlen );
	ri.Hunk_FreeTempMemory( buffer );
}

/*
==================
RB_TakeScreenshotCmd
==================
*/
void RB_TakeScreenshotCmd( void ) {
	const screenshotCommand_t *cmd;
	
	cmd = (const screenshotCommand_t *)&backendData[ rg.smpFrame ]->screenshotBuf;

	if ( cmd->jpeg ) {
		RB_TakeScreenshotJPEG( cmd->x, cmd->y, cmd->width, cmd->height, cmd->fileName);
	} else {
		RB_TakeScreenshot( cmd->x, cmd->y, cmd->width, cmd->height, cmd->fileName);
	}
}

/*
==================
R_TakeScreenshot
==================
*/
static void R_TakeScreenshot( int x, int y, int width, int height, char *name, qboolean jpeg ) {
	static char	fileName[MAX_OSPATH]; // bad things if two screenshots per frame?
	screenshotCommand_t	*cmd;

	cmd = R_GetCommandBuffer( sizeof( *cmd ) );
	if ( !cmd ) {
		return;
	}
	cmd->commandId = RC_SCREENSHOT;

	cmd->x = x;
	cmd->y = y;
	cmd->width = width;
	cmd->height = height;
	N_strncpyz( fileName, name, sizeof(fileName) );
	cmd->fileName = fileName;
	cmd->jpeg = jpeg;
}

/* 
================== 
R_ScreenshotFilename
================== 
*/  
static void R_ScreenshotFilename( int lastNumber, char *fileName ) {
	int		a,b,c,d;

	if ( lastNumber < 0 || lastNumber > 9999 ) {
		Com_snprintf( fileName, MAX_OSPATH, "screenshots/shot9999.tga" );
		return;
	}

	a = lastNumber / 1000;
	lastNumber -= a*1000;
	b = lastNumber / 100;
	lastNumber -= b*100;
	c = lastNumber / 10;
	lastNumber -= c*10;
	d = lastNumber;

	Com_snprintf( fileName, MAX_OSPATH, "screenshots/shot%i%i%i%i.tga"
		, a, b, c, d );
}

/* 
================== 
R_ScreenshotFilename
================== 
*/  
static void R_ScreenshotFilenameJPEG( int lastNumber, char *fileName ) {
	int		a,b,c,d;

	if ( lastNumber < 0 || lastNumber > 9999 ) {
		Com_snprintf( fileName, MAX_OSPATH, "screenshots/shot9999.jpg" );
		return;
	}

	a = lastNumber / 1000;
	lastNumber -= a*1000;
	b = lastNumber / 100;
	lastNumber -= b*100;
	c = lastNumber / 10;
	lastNumber -= c*10;
	d = lastNumber;

	Com_snprintf( fileName, MAX_OSPATH, "screenshots/shot%i%i%i%i.jpg"
		, a, b, c, d );
}

/*
====================
R_LevelShot

levelshots are specialized 128*128 thumbnails for
the menu system, sampled down from full screen distorted images
====================
*/
#define LEVELSHOT_WIDTH 128
#define LEVELSHOT_HEIGHT 128

static void R_LevelShot( void ) {
	char		checkname[MAX_OSPATH];
	byte		*buffer;
	byte		*source, *allsource;
	byte		*src, *dst;
	size_t			offset = 0;
	int			padlen;
	int			x, y;
	int			r, g, b;
	float		xScale, yScale;
	int			xx, yy;

	Com_snprintf( checkname, sizeof(checkname), "levelshots/%s.tga", rg.world->baseName );

	allsource = RB_ReadPixels( 0, 0, glConfig.vidWidth, glConfig.vidHeight, &offset, &padlen );
	source = allsource + offset;

	buffer = ri.Hunk_AllocateTempMemory( LEVELSHOT_WIDTH* LEVELSHOT_HEIGHT*3 + 18 );
	memset( buffer, 0, 18 );
	buffer[2] = 2;		// uncompressed type
	*(uint16_t *)( buffer + 12 ) = LEVELSHOT_WIDTH;
	*(uint16_t *)( buffer + 14 ) = LEVELSHOT_HEIGHT;
	buffer[16] = 24;	// pixel size

	// resample from source
	xScale = glConfig.vidWidth / 512.0f;
	yScale = glConfig.vidHeight / 384.0f;

	for ( y = 0 ; y < LEVELSHOT_WIDTH ; y++ ) {
		for ( x = 0 ; x < LEVELSHOT_HEIGHT ; x++ ) {
			r = g = b = 0;
			for ( yy = 0 ; yy < 3 ; yy++ ) {
				for ( xx = 0 ; xx < 4 ; xx++ ) {
					src = source + (3 * glConfig.vidWidth + padlen) * (int)((y*3 + yy) * yScale) +
						3 * (int) ((x*4 + xx) * xScale);
					r += src[0];
					g += src[1];
					b += src[2];
				}
			}
			dst = buffer + 18 + 3 * ( y * 128 + x );
			dst[0] = b / 12;
			dst[1] = g / 12;
			dst[2] = r / 12;
		}
	}
	// gamma correction
	if ( glConfig.deviceSupportsGamma ) {
		R_GammaCorrect( buffer + 18, LEVELSHOT_WIDTH * LEVELSHOT_HEIGHT * 3 );
	}

	ri.FS_WriteFile( checkname, buffer, LEVELSHOT_WIDTH * LEVELSHOT_HEIGHT*3 + 18 );

	ri.Hunk_FreeTempMemory( buffer );
	ri.Hunk_FreeTempMemory( allsource );

	ri.Printf( PRINT_INFO, "Wrote %s\n", checkname );
}

/* 
================== 
R_ScreenShot_f

screenshot
screenshot [silent]
screenshot [levelshot]
screenshot [filename]

Doesn't print the pacifier message if there is a second arg
================== 
*/  
static void R_ScreenShot_f( void ) {
	char	checkname[MAX_OSPATH];
	static	int	lastNumber = -1;
	qboolean	silent;

	if ( !strcmp( ri.Cmd_Argv( 1 ), "levelshot" ) ) {
		R_LevelShot();
		return;
	}

	if ( !strcmp( ri.Cmd_Argv( 1 ), "silent" ) ) {
		silent = qtrue;
	} else {
		silent = qfalse;
	}

	if ( ri.Cmd_Argc() == 2 && !silent ) {
		// explicit filename
		Com_snprintf( checkname, MAX_OSPATH, "screenshots/%s.tga", ri.Cmd_Argv( 1 ) );
	} else {
		// scan for a free filename

		// if we have saved a previous screenshot, don't scan
		// again, because recording demo avis can involve
		// thousands of shots
		if ( lastNumber == -1 ) {
			lastNumber = 0;
		}
		// scan for a free number
		for ( ; lastNumber <= 9999 ; lastNumber++ ) {
			R_ScreenshotFilename( lastNumber, checkname );

			if ( !ri.FS_FileExists( checkname ) ) {
				break; // file doesn't exist
			}
		}

		if ( lastNumber >= 9999 ) {
			ri.Printf( PRINT_INFO, "ScreenShot: Couldn't create a file\n" );
			return;
 		}

		lastNumber++;
	}

	R_TakeScreenshot( 0, 0, glConfig.vidWidth, glConfig.vidHeight, checkname, qfalse );

	if ( !silent ) {
		ri.Printf( PRINT_INFO, "Wrote %s\n", checkname);
	}
} 

static void R_ScreenShotJPEG_f( void ) {
	char		checkname[MAX_OSPATH];
	static	int	lastNumber = -1;
	qboolean	silent;

	if ( !strcmp( ri.Cmd_Argv( 1 ), "levelshot" ) ) {
		R_LevelShot();
		return;
	}

	if ( !strcmp( ri.Cmd_Argv( 1 ), "silent" ) ) {
		silent = qtrue;
	} else {
		silent = qfalse;
	}

	if ( ri.Cmd_Argc() == 2 && !silent ) {
		// explicit filename
		Com_snprintf( checkname, MAX_OSPATH, "screenshots/%s.jpg", ri.Cmd_Argv( 1 ) );
	} else {
		// scan for a free filename

		// if we have saved a previous screenshot, don't scan
		// again, because recording demo avis can involve
		// thousands of shots
		if ( lastNumber == -1 ) {
			lastNumber = 0;
		}
		// scan for a free number
		for ( ; lastNumber <= 9999 ; lastNumber++ ) {
			R_ScreenshotFilenameJPEG( lastNumber, checkname );

			if ( !ri.FS_FileExists( checkname ) ) {
				break; // file doesn't exist
			}
		}

		if ( lastNumber == 10000 ) {
			ri.Printf( PRINT_INFO, "ScreenShot: Couldn't create a file\n" ); 
			return;
 		}

		lastNumber++;
	}

	R_TakeScreenshot( 0, 0, glConfig.vidWidth, glConfig.vidHeight, checkname, qtrue );

	if ( !silent ) {
		ri.Printf( PRINT_INFO, "Wrote %s\n", checkname );
	}
} 

//============================================================================

static void GpuInfo_f( void ) 
{
	const char *enablestrings[] = {
		"disabled",
		"enabled"
	};
	const char *fsstrings[] = {
		"windowed",
		"fullscreen"
	};

	ri.Printf( PRINT_INFO, "\nGL_VENDOR: %s\n", glContext.vendor );
	ri.Printf( PRINT_INFO, "GL_RENDERER: %s\n", glContext.renderer );
	ri.Printf( PRINT_INFO, "GL_VERSION: %s\n", glContext.version_str );
	ri.Printf( PRINT_INFO, "GL_EXTENSIONS: " );
	if ( nglGetStringi ) {
		GLint numExtensions;
		int i;

		nglGetIntegerv( GL_NUM_EXTENSIONS, &numExtensions );
		for ( i = 0; i < numExtensions; i++ ) {
			if ( i > 64 ) {
				break;
			}
			ri.Printf( PRINT_INFO, "%s ", nglGetStringi( GL_EXTENSIONS, i ) );
		}
	}
	else {
		R_PrintLongString( gl_extensions );
	}
	ri.Printf( PRINT_INFO, "\n" );
	ri.Printf( PRINT_INFO, "GL_MAX_TEXTURE_SIZE: %d\n", glContext.maxTextureSize );
	ri.Printf( PRINT_INFO, "GL_MAX_TEXTURE_IMAGE_UNITS: %d\n", glContext.maxTextureUnits );
	ri.Printf( PRINT_INFO, "GL_MAX_TEXTURE_MAX_ANISOTROPY: %f\n", glContext.maxAnisotropy );
	ri.Printf( PRINT_INFO, "\nPIXELFORMAT: color(%d-bits) Z(%d-bit) stencil(%d-bits)\n", glConfig.colorBits, glConfig.depthBits, glConfig.stencilBits );
	ri.Printf( PRINT_INFO, "MODE: %li, %d x %d %s hz:", ri.Cvar_VariableInteger( "r_mode" ), glConfig.vidWidth, glConfig.vidHeight, fsstrings[ glConfig.isFullscreen != 0 ] );
	if ( glConfig.displayFrequency ) {
		ri.Printf( PRINT_INFO, "%d\n", glConfig.displayFrequency );
	}
	else {
		ri.Printf( PRINT_INFO, "N/A\n" );
	}
	if ( glConfig.deviceSupportsGamma ) {
		ri.Printf( PRINT_INFO, "GAMMA: hardware w/ %d overbright bits\n", rg.overbrightBits );
	}
	else {
		ri.Printf( PRINT_INFO, "GAMMA: software w/ %d overbright bits\n", rg.overbrightBits );
	}

	ri.Printf( PRINT_INFO, "texture filtering: %s\n", r_textureMode->s );
	ri.Printf( PRINT_INFO, "Extensions:\n" );
	ri.Printf( PRINT_INFO, "GL_ARB_framebuffer_blit: %s\n", enablestrings[glContext.ARB_framebuffer_blit]);
	ri.Printf( PRINT_INFO, "GL_ARB_framebuffer_object: %s\n", enablestrings[glContext.ARB_framebuffer_object]);
	ri.Printf( PRINT_INFO, "GL_ARB_framebuffer_multisample: %s\n", enablestrings[glContext.ARB_framebuffer_multisample]);
	ri.Printf( PRINT_INFO, "GL_ARB_framebuffer_sRGB: %s\n", enablestrings[glContext.ARB_framebuffer_sRGB]);
	ri.Printf( PRINT_INFO, "GL_ARB_gl_spirv: %s\n", enablestrings[glContext.ARB_gl_spirv]);
	ri.Printf( PRINT_INFO, "GL_ARB_texture_compression: %s\n", enablestrings[glContext.ARB_texture_compression]);
	ri.Printf( PRINT_INFO, "GL_ARB_texture_filter_anisotropic: %s\n", enablestrings[glContext.ARB_texture_filter_anisotropic]);
	ri.Printf( PRINT_INFO, "GL_ARB_texture_float: %s\n", enablestrings[glContext.ARB_texture_float]);
	ri.Printf( PRINT_INFO, "GL_ARB_vertex_array_object: %s\n", enablestrings[glContext.ARB_vertex_array_object]);
	ri.Printf( PRINT_INFO, "GL_ARB_vertex_buffer_object: %s\n", enablestrings[glContext.ARB_vertex_buffer_object]);
	ri.Printf( PRINT_INFO, "GL_ARB_vertex_shader: %s\n", enablestrings[glContext.ARB_vertex_shader]);
	ri.Printf( PRINT_INFO, "GL_ARG_sync: %s\n", enablestrings[ glContext.ARB_sync ] );
	ri.Printf( PRINT_INFO, "GL_ARB_map_buffer_range: %s\n", enablestrings[ glContext.ARB_map_buffer_range ] );
	ri.Printf( PRINT_INFO, "GL_ARB_buffer_storage: %s\n", enablestrings[ glContext.ARB_buffer_storage ] );
	ri.Printf( PRINT_INFO, "GL_ARB_shader_storage_buffer_object: %s\n", enablestrings[ glContext.ARB_shader_storage_buffer_object ] );

	if ( r_finish->i ) {
		ri.Printf( PRINT_INFO, "Forcing glFinish\n" );
	}
}

static void R_Register( void )
{
	//
	// latched and archived variables
	//
	r_useExtensions = ri.Cvar_Get( "r_useExtensions", "1", CVAR_SAVE | CVAR_LATCH );
	ri.Cvar_SetDescription( r_useExtensions, "Use all of the OpenGL extensions your card is capable of." );

	r_arb_pixel_buffer_object = ri.Cvar_Get( "r_arb_pixel_buffer_object", "0", CVAR_SAVE | CVAR_LATCH );
	ri.Cvar_SetDescription( r_arb_pixel_buffer_object, "Enables pixel buffer objects." );

	r_arb_direct_state_access = ri.Cvar_Get( "r_arb_direct_state_access", "1", CVAR_SAVE | CVAR_LATCH );
	ri.Cvar_SetDescription( r_arb_direct_state_access, "Enables direct state access." );

	r_arb_texture_compression = ri.Cvar_Get( "r_arb_texture_compression", "3", CVAR_SAVE | CVAR_LATCH );
	ri.Cvar_SetDescription( r_arb_texture_compression, "Enables texture compression." );
	r_arb_framebuffer_object = ri.Cvar_Get( "r_arb_framebuffer_object", "1", CVAR_SAVE | CVAR_LATCH );
	ri.Cvar_SetDescription( r_arb_framebuffer_object, "Enables post-processing via multiple framebuffers." );
	r_arb_vertex_array_object = ri.Cvar_Get( "r_arb_vertex_array_object", "1", CVAR_SAVE | CVAR_LATCH );
	ri.Cvar_SetDescription( r_arb_vertex_array_object, "Enables use of vertex array object extensions.\nNOTE: only really matters if OpenGL version < 3.3" );
	r_arb_vertex_buffer_object = ri.Cvar_Get( "r_arb_vertex_buffer_object", "1", CVAR_SAVE | CVAR_LATCH );
	ri.Cvar_SetDescription( r_arb_vertex_buffer_object, "Enables use of hardware accelerated vertex and index rendering." );

	r_arb_sync = ri.Cvar_Get( "r_arb_sync", "0", CVAR_SAVE | CVAR_LATCH );
	ri.Cvar_CheckRange( r_arb_sync, "0", "1", CVT_INT );

	r_arb_texture_filter_anisotropic = ri.Cvar_Get( "r_arb_texture_filter_anisotropic", "1", CVAR_SAVE | CVAR_LATCH );
	ri.Cvar_SetDescription( r_arb_texture_filter_anisotropic, "Enabled anisotropic filtering." );
	r_arb_texture_max_anisotropy = ri.Cvar_Get( "r_arb_texture_max_anisotropy", "4", CVAR_SAVE | CVAR_LATCH );
	ri.Cvar_SetDescription( r_arb_texture_max_anisotropy, "Sets maximum anisotropic level for your graphics driver. Requires \\r_arb_texture_filter_anisotropic 1." );

	r_arb_map_buffer_range = ri.Cvar_Get( "r_arb_map_buffer_range", "0", CVAR_LATCH | CVAR_SAVE );

	r_arb_compute_shader = ri.Cvar_Get( "r_arb_compute_shader", "1", CVAR_SAVE );
	ri.Cvar_SetDescription( r_arb_compute_shader, "Enables the usage of compute shaders for parallalized GPU batch work." );

	r_arb_shader_storage_buffer_object = ri.Cvar_Get( "r_arb_shader_storage_buffer_object", "0", CVAR_LATCH | CVAR_SAVE );
	ri.Cvar_SetDescription( r_arb_shader_storage_buffer_object,
		"Enables usage of Shader Storage Buffer Objects (SSBO) instead of Uniform Buffer Objects (UBO).\n"
		"SSBOs can hold much more data and are dynamically sizeable, but are generally slower than UBOs." );

	r_arb_texture_float = ri.Cvar_Get( "r_arb_texture_float", "1", CVAR_SAVE | CVAR_LATCH );
	ri.Cvar_SetDescription( r_arb_texture_float, "Enables HDR framebuffer." );

	r_allowLegacy = ri.Cvar_Get( "r_allowLegacy", "0", CVAR_SAVE | CVAR_LATCH );
	ri.Cvar_SetDescription( r_allowLegacy, "Allow the use of old OpenGL API versions, requires \\r_drawMode 0 or 1 and \\r_allowShaders 0" );
	r_allowShaders = ri.Cvar_Get( "r_allowShaders", "1", CVAR_SAVE | CVAR_LATCH );
	ri.Cvar_SetDescription( r_allowShaders, "Allow the use of GLSL shaders, requires \\r_allowLegacy 0." );

	r_lightingQuality = ri.Cvar_Get( "r_lightingQuality", "1", CVAR_SAVE );
	ri.Cvar_CheckRange( r_lightingQuality, "0", "2", CVT_INT );
	ri.Cvar_SetDescription( r_lightingQuality, "Sets desired lighting quality" );

	r_picmip = ri.Cvar_Get( "r_picmip", "0", CVAR_SAVE | CVAR_LATCH );
	ri.Cvar_CheckRange( r_picmip, "0", "16", CVT_INT );
	ri.Cvar_SetDescription( r_picmip, "Set texture quality, lower is better." );
	r_roundImagesDown = ri.Cvar_Get( "r_roundImagesDown", "1", CVAR_SAVE | CVAR_LATCH );
	ri.Cvar_SetDescription( r_roundImagesDown, "When images are scaled, round images down instead of up." );

	r_detailTextures = ri.Cvar_Get( "r_detailtextures", "1", CVAR_SAVE | CVAR_LATCH );
	ri.Cvar_SetDescription( r_detailTextures, "Enables usage of shader stages flagged as detail." );
	r_textureBits = ri.Cvar_Get( "r_textureBits", "0", CVAR_SAVE | CVAR_LATCH );
	ri.Cvar_SetDescription( r_textureBits, "Number of texture bits per texture." );
	r_stencilBits = ri.Cvar_Get( "r_stencilBits", "8", CVAR_SAVE | CVAR_LATCH );
	ri.Cvar_SetDescription( r_stencilBits, "Stencil buffer size, value decreases Z-buffer depth." );

	r_hdr = ri.Cvar_Get( "r_hdr", "1", CVAR_SAVE );
	ri.Cvar_SetDescription( r_hdr, "Do scene rendering in a framebuffer with high dynamic range." );

	r_floatLightmap = ri.Cvar_Get( "r_floatLightmap", "0", CVAR_SAVE | CVAR_LATCH );

	r_toneMap = ri.Cvar_Get( "r_toneMap", "1", CVAR_SAVE );
	ri.Cvar_SetDescription( r_toneMap, "Enable tone mapping. Requires r_hdr and r_postProcess." );
	r_forceToneMap = ri.Cvar_Get( "r_forceToneMap", "0", CVAR_CHEAT );
	r_forceToneMapMin = ri.Cvar_Get( "r_forceToneMapMin", "-8.0", CVAR_CHEAT );
	r_forceToneMapAvg = ri.Cvar_Get( "r_forceToneMapAvg", "-2.0", CVAR_CHEAT );
	r_forceToneMapMax = ri.Cvar_Get( "r_forceToneMapMax", "0.0", CVAR_CHEAT );

	r_sunShadows = ri.Cvar_Get( "r_sunShadows", "1", CVAR_SAVE | CVAR_LATCH );
	r_shadowFilter = ri.Cvar_Get( "r_shadowFilter", "1", CVAR_SAVE | CVAR_LATCH );
	ri.Cvar_SetDescription( r_shadowFilter, "Enable filtering shadows for a smoother look (0 - No. 1 - Some. 2 - Much)." );
	r_shadowBlur = ri.Cvar_Get("r_shadowBlur", "0", CVAR_SAVE | CVAR_LATCH);
	r_shadowMapSize = ri.Cvar_Get("r_shadowMapSize", "1024", CVAR_SAVE | CVAR_LATCH);
	ri.Cvar_SetDescription( r_shadowMapSize, "Size of each cascaded shadow map." );
	r_shadowCascadeZNear = ri.Cvar_Get( "r_shadowCascadeZNear", "8", CVAR_SAVE | CVAR_LATCH );
	ri.Cvar_SetDescription( r_shadowCascadeZNear, "Near plane for shadow cascade frustums." );
	r_shadowCascadeZFar = ri.Cvar_Get( "r_shadowCascadeZFar", "1024", CVAR_SAVE | CVAR_LATCH );
	ri.Cvar_SetDescription( r_shadowCascadeZFar, "Far plane for shadow cascade frustums." );
	r_shadowCascadeZBias = ri.Cvar_Get( "r_shadowCascadeZBias", "0", CVAR_SAVE | CVAR_LATCH );
	ri.Cvar_SetDescription( r_shadowCascadeZBias, "Z-bias for shadow cascade frustums." );

	r_forceSun = ri.Cvar_Get( "r_forceSun", "0", CVAR_CHEAT );
	r_forceSunLightScale = ri.Cvar_Get( "r_forceSunLightScale", "1.0", CVAR_CHEAT );
	r_forceSunAmbientScale = ri.Cvar_Get( "r_forceSunAmbientScale", "0.5", CVAR_CHEAT );
	r_drawSunRays = ri.Cvar_Get( "r_drawSunRays", "0", CVAR_SAVE | CVAR_LATCH );
	r_sunlightMode = ri.Cvar_Get( "r_sunlightMode", "1", CVAR_SAVE | CVAR_LATCH );

	r_smaaEdgesType = ri.Cvar_Get( "r_smaaEdgesType", "luma", CVAR_SAVE | CVAR_LATCH );
	ri.Cvar_SetDescription( r_smaaEdgesType, "Sets SMAA anti-aliasing edges processing method. Can be luma, depth, or color." );

	r_useShaderCache = ri.Cvar_Get( "r_useShaderCache", "1", CVAR_LATCH | CVAR_SAVE );
	ri.Cvar_SetDescription( r_useShaderCache, "Caches GLSL shader objects for faster loading, requires GL_ARB_gl_spirv extension." );

	sys_forceSingleThreading = ri.Cvar_Get( "sys_forceSingleThreading", "0", CVAR_LATCH | CVAR_SAVE );

	r_drawMode = ri.Cvar_Get( "r_drawMode", "2", CVAR_SAVE | CVAR_LATCH );
	ri.Cvar_SetDescription( r_drawMode,
							"Sets the rendering mode (see OpenGL docs if you want more info):\n"
							" 0: immediate mode, deprecated in modern GPUs, this will not be supported\n"
							" 1: client buffered, cpu buffers, but uses glDrawElements\n"
							" 2: gpu buffered, gpu and cpu buffers, most supported\n"
							" 3: mapped to client space using glMapBuffer, the most recent in NVidia\n" );

	r_depthPrepass = ri.Cvar_Get( "r_depthPrepass", "1", CVAR_SAVE );
	ri.Cvar_SetDescription( r_depthPrepass, "Do a depth-only pass before rendering. Speeds up rendering in cases where advanced features are used. Required for r_sunShadows." );
	r_ssao = ri.Cvar_Get( "r_ssao", "0", CVAR_SAVE );
	ri.Cvar_SetDescription( r_ssao, "Enable screen-space ambient occlusion." );
	r_bloom = ri.Cvar_Get( "r_bloom", "1", CVAR_SAVE );
	ri.Cvar_SetDescription( r_bloom, "Enables framebuffer based bloom to make light sources stand out, requires \\r_hdr." );

	r_loadTexturesOnDemand = ri.Cvar_Get( "r_loadTexturesOnDemand", "1", CVAR_LATCH | CVAR_SAVE );
	ri.Cvar_SetDescription( r_loadTexturesOnDemand, "Enables loading textures on demand, requires GL_ARB_bindless_textures\n" );

	r_normalMapping = ri.Cvar_Get( "r_normalMapping", "1", CVAR_SAVE | CVAR_LATCH );
	ri.Cvar_SetDescription( r_normalMapping, "Enable normal maps for materials that support it." );
	r_specularMapping = ri.Cvar_Get( "r_specularMapping", "1", CVAR_SAVE | CVAR_LATCH );
	ri.Cvar_SetDescription( r_specularMapping, "Enable specular maps for materials that support it." );
	r_deluxeMapping = ri.Cvar_Get( "r_deluxeMapping", "1", CVAR_SAVE | CVAR_LATCH );
	ri.Cvar_SetDescription( r_deluxeMapping, "Enable deluxe mapping (map is compiled with light directions). Even if the map doesn't have deluxe mapping compiled in, an approximation based on the lightgrid will be used." );
	r_parallaxMapping = ri.Cvar_Get( "r_parallaxMapping", "0", CVAR_SAVE | CVAR_LATCH );
	ri.Cvar_SetDescription( r_parallaxMapping, "Enable parallax mapping for materials that support it.\n 0: No\n 1: Use parallax occlusion mapping\n 2: Use relief mapping" );
	r_parallaxMapOffset = ri.Cvar_Get( "r_parallaxMapOffset", "0", CVAR_SAVE | CVAR_LATCH );
	ri.Cvar_SetDescription( r_parallaxMapOffset, "Set the parallax height offset." );
	r_parallaxMapShadows = ri.Cvar_Get( "r_parallaxMapShadows", "0", CVAR_SAVE | CVAR_LATCH );
	ri.Cvar_SetDescription( r_parallaxMapShadows, "Enable self-shadowing on parallax map supported materials." );
	r_deluxeSpecular = ri.Cvar_Get("r_deluxeSpecular", "0.3", CVAR_SAVE | CVAR_LATCH);
	r_pbr = ri.Cvar_Get( "r_pbr", "0", CVAR_SAVE | CVAR_LATCH );
	ri.Cvar_SetDescription( r_pbr, "Enable physically based rendering." );
	r_baseNormalX = ri.Cvar_Get( "r_baseNormalX", "1.0", CVAR_SAVE | CVAR_LATCH );
	r_baseNormalY = ri.Cvar_Get( "r_baseNormalY", "1.0", CVAR_SAVE | CVAR_LATCH );
	r_baseParallax = ri.Cvar_Get( "r_baseParallax", "0.05", CVAR_SAVE | CVAR_LATCH );
	r_baseSpecular = ri.Cvar_Get( "r_baseSpecular", "0.04", CVAR_SAVE | CVAR_LATCH );
	r_baseGloss = ri.Cvar_Get( "r_baseGloss", "0.3", CVAR_SAVE | CVAR_LATCH );
	r_glossType = ri.Cvar_Get("r_glossType", "1", CVAR_SAVE | CVAR_LATCH);
	r_dlightMode = ri.Cvar_Get( "r_dlightMode", "0", CVAR_SAVE | CVAR_LATCH );
	ri.Cvar_SetDescription( r_dlightMode,
							"Dynamic light mode:\n"
							" 0: Software processed per-polygon dynamic lights\n"
							" 1: High quality per-pixel dynamic lighting done in a shader, slower than per-vertex lighting" );
	r_pshadowDist = ri.Cvar_Get( "r_pshadowDist", "128", CVAR_SAVE );
	r_mergeLightmaps = ri.Cvar_Get( "r_mergeLightmaps", "1", CVAR_SAVE | CVAR_LATCH );
	ri.Cvar_SetDescription( r_mergeLightmaps, "Merge small lightmaps into 2 or fewer giant lightmaps." );
	r_imageUpsample = ri.Cvar_Get( "r_imageUpsample", "0", CVAR_SAVE | CVAR_LATCH );
	r_imageUpsampleMaxSize = ri.Cvar_Get( "r_imageUpsampleMaxSize", "1024", CVAR_SAVE | CVAR_LATCH );
	r_imageUpsampleType = ri.Cvar_Get( "r_imageUpsampleType", "1", CVAR_SAVE | CVAR_LATCH );
	r_genNormalMaps = ri.Cvar_Get( "r_genNormalMaps", "0", CVAR_SAVE | CVAR_LATCH );

	r_greyscale = ri.Cvar_Get("r_greyscale", "0", CVAR_SAVE | CVAR_LATCH);
	ri.Cvar_CheckRange( r_greyscale, "0", "1", CVT_INT );
	ri.Cvar_SetDescription( r_greyscale, "Desaturates rendered frame." );

	r_externalGLSL = ri.Cvar_Get( "r_externalGLSL", "0", CVAR_LATCH );
	ri.Cvar_SetDescription( r_externalGLSL, "Enables loading glsl from external files instead of just built-in shaders." );

	r_ignoreDstAlpha = ri.Cvar_Get( "r_ignoreDstAlpha", "1", CVAR_SAVE | CVAR_LATCH );

	//
	// temporary latched variables that can only change over a restart
	//
	r_fullbright = ri.Cvar_Get( "r_fullbright", "0", CVAR_LATCH | CVAR_CHEAT );
	ri.Cvar_SetDescription( r_fullbright, "Debugging tool to render the entire level without lighting." );
	r_intensity = ri.Cvar_Get( "r_intensity", "1", CVAR_LATCH );
	ri.Cvar_SetDescription( r_intensity, "Global texture lighting scale." );
	r_singleShader = ri.Cvar_Get( "r_singleShader", "0", CVAR_CHEAT | CVAR_LATCH );
	ri.Cvar_SetDescription( r_singleShader, "Debugging tool that only uses the default shader for all rendering." );
	r_mapOverBrightBits = ri.Cvar_Get( "r_mapOverBrightBits", "2", CVAR_LATCH );
	ri.Cvar_SetDescription( r_mapOverBrightBits, "Sets the number of overbright bits baked into all lightmaps and map data." );


	//
	// temporary variables that can change at any time
	//
	r_lightmap = ri.Cvar_Get( "r_lightmap", "0", 0 );
	ri.Cvar_SetDescription( r_lightmap, "Show only lightmaps on all world surfaces." );

	r_debugCamera = ri.Cvar_Get( "r_debugCamera", "0", CVAR_PRIVATE | CVAR_TEMP );
	ri.Cvar_SetDescription( r_debugCamera, "Toggles free camera movement, used for photomode." );

	//
	// archived variables that can change any time
	//
	r_customWidth = ri.Cvar_Get( "r_customWidth", "1980", CVAR_SAVE );
	r_customHeight = ri.Cvar_Get( "r_customHeight", "1080", CVAR_SAVE );

	r_multisampleType = ri.Cvar_Get( "r_multisampleType", "1", CVAR_SAVE );
	ri.Cvar_CheckRange( r_multisampleType, va( "%i", AntiAlias_None ), va( "%i", AntiAlias_FXAA ), CVT_INT );
	ri.Cvar_SetDescription( r_multisampleType, "Sets the desired anti-aliasing technique. Requires \\r_postProcess" );
	r_antialiasQuality = ri.Cvar_Get( "r_antialiasQuality", "1", CVAR_SAVE );
	ri.Cvar_CheckRange( r_antialiasQuality, "0", "2", CVT_INT );
	ri.Cvar_SetDescription( r_antialiasQuality, "Sets antialiasing quality" );

	r_overBrightBits = ri.Cvar_Get( "r_overBrightBits", "1", CVAR_SAVE );
	ri.Cvar_SetDescription( r_overBrightBits, "Sets the intensity of overall brightness of texture pixels." );
	r_ignorehwgamma = ri.Cvar_Get( "r_ignorehwgamma", "1", CVAR_SAVE );
	ri.Cvar_SetDescription( r_ignorehwgamma, "Overrides hardware gamma capabilities." );

	r_postProcess = ri.Cvar_Get( "r_postProcess", "1", CVAR_SAVE );
	ri.Cvar_SetDescription( r_postProcess, "Enable post-processing." );

	r_imageSharpenAmount = ri.Cvar_Get( "r_imageSharpenAmount", "1.0", CVAR_SAVE );
	ri.Cvar_CheckRange( r_imageSharpenAmount, "0.5", "5.0", CVT_FLOAT );
	ri.Cvar_SetDescription( r_imageSharpenAmount, "Sets the amount of sharpening applied per-pixel to a rendered texture" );

#ifdef _NOMAD_DEBUG
	r_glDiagnostics = ri.Cvar_Get( "r_gpuDiagnostics", "1", CVAR_SAVE | CVAR_LATCH );
#else
	r_glDiagnostics = ri.Cvar_Get( "r_gpuDiagnostics", "0", CVAR_SAVE | CVAR_LATCH );
#endif
	ri.Cvar_CheckRange( r_glDiagnostics, "0", "1", CVT_INT );
	ri.Cvar_SetDescription( r_glDiagnostics, "Toggles display of gpu per-frame statistics." );

#ifdef _NOMAD_DEBUG
	r_glDebug = ri.Cvar_Get( "r_glDebug", "1", CVAR_SAVE | CVAR_LATCH );
#else
	r_glDebug = ri.Cvar_Get( "r_glDebug", "0", CVAR_SAVE | CVAR_LATCH );
#endif
	ri.Cvar_SetDescription( r_glDebug, "Toggles OpenGL driver debug logging." );

	r_textureBits = ri.Cvar_Get( "r_textureBits", "0", CVAR_SAVE | CVAR_LATCH );
	ri.Cvar_SetDescription( r_textureBits, "Number of texture bits per texture." );
	r_stencilBits = ri.Cvar_Get( "r_stencilBits", "8", CVAR_SAVE | CVAR_LATCH );
	ri.Cvar_SetDescription( r_stencilBits, "Stencil buffer size, value decreases Z-buffer depth." );

	r_finish = ri.Cvar_Get( "r_finish", "0", CVAR_SAVE | CVAR_LATCH );
	ri.Cvar_SetDescription( r_finish, "Force a glFinish call after rendering a scene." );

	r_picmip = ri.Cvar_Get( "r_picmip", "0", CVAR_SAVE | CVAR_LATCH );
	ri.Cvar_CheckRange( r_picmip, "0", "16", CVT_INT );
	ri.Cvar_SetDescription( r_picmip, "Set texture quality, lower is better." );
	r_colorMipLevels = ri.Cvar_Get( "r_colorMipLevels", "0", CVAR_LATCH );
	ri.Cvar_SetDescription( r_colorMipLevels, "Debugging tool to artificially color different mipmap levels so that they are more apparent." );
	r_roundImagesDown = ri.Cvar_Get( "r_roundImagesDown", "1", CVAR_SAVE | CVAR_LATCH );
	ri.Cvar_SetDescription( r_roundImagesDown, "When images are scaled, round images down instead of up." );

	r_fixedRendering = ri.Cvar_Get( "r_fixedRendering", "1", CVAR_SAVE | CVAR_LATCH );
	ri.Cvar_SetDescription( r_fixedRendering, "Forces the engine the render the screen at a lower virtual resolution then scale up if needed." );

	r_fixedResolutionScale = ri.Cvar_Get( "r_fixedResolutionScale", "0", CVAR_SAVE | CVAR_LATCH );
	ri.Cvar_CheckRange( r_fixedResolutionScale, "0", "1", CVT_FLOAT );
	ri.Cvar_SetDescription( r_fixedResolutionScale, "Sets the fixed rendering resolution, requires \\r_fixedRendering \"1\"" );

	r_vertexLight = ri.Cvar_Get( "r_vertexLight", "0", CVAR_SAVE | CVAR_DEV );
	ri.Cvar_SetDescription( r_vertexLight, "Set to 1 to use vertex light instead of lightmaps, collapse all multi-stage shaders into single-stage ones, might cause rendering artifacts." );
	

	r_ignoreGLErrors = ri.Cvar_Get( "r_ignoreGLErrors", "1", CVAR_SAVE );
	ri.Cvar_SetDescription( r_ignoreGLErrors, "Ignore OpenGL errors." );
//	r_fastsky = ri.Cvar_Get( "r_fastsky", "0", CVAR_SAVE );
//	ri.Cvar_SetDescription( r_fastsky, "Draw flat colored skies." );
	r_drawSun = ri.Cvar_Get( "r_drawSun", "0", CVAR_SAVE );
	ri.Cvar_SetDescription( r_drawSun, "Draw sun shader in skies." );
	r_dynamiclight = ri.Cvar_Get( "r_dynamiclight", "1", CVAR_SAVE );
	ri.Cvar_SetDescription( r_dynamiclight, "Enables dynamic lighting." );
	r_dlightBacks = ri.Cvar_Get( "r_dlightBacks", "1", CVAR_SAVE );
	ri.Cvar_SetDescription( r_dlightBacks, "Whether or not dynamic lights should light up back-face culled geometry." );
	r_finish = ri.Cvar_Get( "r_finish", "0", CVAR_SAVE );
	ri.Cvar_SetDescription( r_finish, "Force a glFinish call after rendering a scene." );
	r_textureMode = ri.Cvar_Get( "r_textureMode", "Nearest", CVAR_SAVE );
	ri.Cvar_SetDescription( r_textureMode,
							"Texture interpolation mode:\n"
							" Bilinear: Linear interpolation and will appear to blend in objects that are closer than the resolution that the textures are set as\n"
							" Nearest: Nearest neighbor interpolation and will cause the texture to look pixelated. Use for the most retro look and feel\n"
							" Linear Nearest: Linear magnification filter, Nearest minification filter\n"
							" Nearest Linear: Nearest magnification filter, Linear minification filter"
	);
	r_gammaAmount = ri.Cvar_Get( "r_gammaAmount", "1.0", CVAR_SAVE );
	ri.Cvar_CheckRange( r_gammaAmount, "0.5", "3", CVT_FLOAT );
	ri.Cvar_SetDescription( r_gammaAmount, "Gamma correction factor." );

	r_autoExposure = ri.Cvar_Get( "r_autoExposure", "1", CVAR_SAVE );
	ri.Cvar_SetDescription( r_autoExposure, "Do automatic exposure based on scene brightness. Hardcoded to -2 to 2 on maps that don't specify otherwise. Requires r_hdr, r_postProcess, and r_toneMap." );
	r_forceAutoExposure = ri.Cvar_Get( "r_forceAutoExposure", "0", CVAR_CHEAT );
	r_forceAutoExposureMin = ri.Cvar_Get( "r_forceAutoExposureMin", "-2.0", CVAR_CHEAT );
	r_forceAutoExposureMax = ri.Cvar_Get( "r_forceAutoExposureMax", "2.0", CVAR_CHEAT );

	r_cameraExposure = ri.Cvar_Get( "r_cameraExposure", "1", CVAR_CHEAT );

	r_printShaders = ri.Cvar_Get( "r_printShaders", "0", 0 );
	ri.Cvar_SetDescription( r_printShaders, "Debugging tool to print on console of the number of shaders used." );

	r_textureDetail = ri.Cvar_Get( "r_textureDetail", va( "%i", TexDetail_Normie ), CVAR_SAVE | CVAR_LATCH );
	ri.Cvar_CheckRange(r_textureDetail, va( "%i", TexDetail_MSDOS ), va( "%i", TexDetail_GPUvsGod ), CVT_INT );

	r_speeds = ri.Cvar_Get( "r_speeds", "0", CVAR_SAVE | CVAR_LATCH );
	ri.Cvar_SetDescription( r_speeds,
							"Prints out various debugging stats from renderer:\n"
							"0: Disabled\n"
							"1: Backend drawing\n"
							"2: Lighting\n"
							"3: OpenGL state changes" );

	//
	// temporary variables that can change at any time
	//
	r_clearColor = ri.Cvar_Get( "r_clearColor", "0.1 0.1 0.1 1.0", CVAR_TEMP );

	r_showTris = ri.Cvar_Get( "r_showTris", "0", CVAR_TEMP );
	ri.Cvar_SetDescription( r_showTris, "Draw outlines of polygons for bounding box debugging." );

	r_showImages = ri.Cvar_Get( "r_showImages", "0", CVAR_TEMP );
	ri.Cvar_SetDescription( r_showImages,
										"Draw all images currently loaded into memory:\n"
										" 0: Disabled\n"
										" 1: Show images set to uniform size\n"
										" 2: Show images with scaled relative to largest image" );

	r_skipBackEnd = ri.Cvar_Get( "r_skipBackEnd", "0", CVAR_CHEAT );
	ri.Cvar_SetDescription( r_skipBackEnd, "Skips loading rendering backend." );

	r_norefresh = ri.Cvar_Get( "r_norefresh", "0", CVAR_CHEAT );
	ri.Cvar_SetDescription( r_norefresh, "Bypasses refreshing of the rendered scene." );
	r_drawentities = ri.Cvar_Get( "r_drawentities", "1", CVAR_CHEAT );
	ri.Cvar_SetDescription( r_drawentities, "Draw all world entities." );

	r_drawworld = ri.Cvar_Get( "r_drawworld", "1", CVAR_CHEAT );
	ri.Cvar_SetDescription( r_drawworld, "Set to 0 to disable drawing the world. Set to 1 to enable." );
	r_lightmap = ri.Cvar_Get( "r_lightmap", "0", 0 );
	ri.Cvar_SetDescription( r_lightmap, "Show only lightmaps on all world surfaces." );

	r_measureOverdraw = ri.Cvar_Get( "r_measureOverdraw", "0", CVAR_CHEAT );
	r_ignoreGLErrors = ri.Cvar_Get( "r_ignoreGLErrors", "1", CVAR_LATCH );
	r_clear = ri.Cvar_Get( "r_clear", "0", CVAR_CHEAT );
	r_drawBuffer = ri.Cvar_Get( "r_drawBuffer", "GL_BACK", CVAR_CHEAT );
	ri.Cvar_SetDescription( r_drawBuffer, "Sets which frame buffer to draw into." );

	r_maxPolys = ri.Cvar_Get( "r_maxPolys", "528", CVAR_LATCH | CVAR_PROTECTED );
	ri.Cvar_CheckRange( r_maxPolys, "64", "8192", CVT_INT );
	ri.Cvar_SetDescription( r_maxPolys, "Sets the maximum amount of polygons that can be processed per scene.\n"
										"NOTE: there can be multiple scenes rendered in a single frame." );

	r_screenshotJpegQuality = ri.Cvar_Get( "r_screenshotJpegQuality", "90", CVAR_SAVE );
	ri.Cvar_SetDescription( r_screenshotJpegQuality, "Controls quality of Jpeg screenshots when using screenshotJpeg." );

	r_maxDLights = ri.Cvar_Get( "r_maxDLights", "128", CVAR_LATCH | CVAR_PROTECTED );
	ri.Cvar_SetDescription( r_maxDLights, "Sets the maximum amount of dynamic lights that can be processed per scene.\n"
											"NOTE: there can be multiple scenes rendered in a single frame." );
	r_maxEntities = ri.Cvar_Get( "r_maxEntities", "1024", CVAR_LATCH | CVAR_PROTECTED );
	ri.Cvar_SetDescription( r_maxEntities, "Sets the maximum amount of dynamic entities that can be processed per scene.\n"
											"NOTE: there can be multiple scenes rendered in a single frame." );

	if ( r_textureDetail->i >= 2 ) {
		ri.Cvar_Set( "r_normalMapping", "1" );
		ri.Cvar_Set( "r_specularMapping", "1" );
	}

	// make sure all commands added here are also
	// removed in R_Shutdown
	ri.Cmd_AddCommand( "texturelist", R_ImageList_f );
	ri.Cmd_AddCommand( "shaderlist", R_ShaderList_f );
	ri.Cmd_AddCommand( "screenshot", R_ScreenShot_f );
	ri.Cmd_AddCommand( "screenshotJPEG", R_ScreenShotJPEG_f );
	ri.Cmd_AddCommand( "gpuinfo", GpuInfo_f );
	ri.Cmd_AddCommand( "gpumeminfo", GpuMemInfo_f );
}

static void R_InitGLContext( void )
{
	if ( glConfig.vidWidth == 0 ) {
		GLint temp;

		if ( !ri.GLimp_Init ) {
			ri.Error( ERR_FATAL, "OpenGL interface is not initialized" );
		}

		ri.GLimp_Init( &glConfig );

		ri.G_SetScaling( 1.0, glConfig.vidWidth, glConfig.vidHeight );
		ri.GLimp_InitGamma( &glConfig );
	}

	// GL function loader, based on https://gist.github.com/rygorous/16796a0c876cf8a5f542caddb55bce8a
#ifdef _NOMAD_DEBUG
#define NGL( ret, name, ... ) n ## name = (PFN ## name) ri.GL_GetProcAddress(#name); \
							if (!n ## name) ri.Error(ERR_FATAL, "Failed to load OpenGL proc '" #name "'");
#elif defined(_NOMAD_EXPERIMENTAL)
#define NGL( ret, name, ... ) n ## name = (PFN ## name) ri.GL_GetProcAddress(#name); \
							if (!n ## name) ri.Printf(PRINT_INFO, COLOR_YELLOW "Failed to load OpenGL proc '" #name "'\n");
#else
#define NGL( ret, name, ... ) n ## name = (PFN ## name) ri.GL_GetProcAddress(#name);
#endif
	NGL_Core_Procs
	NGL_Shader_Procs
	NGL_Texture_Procs
#undef NGL

	if ( !nglGetString )
		ri.Error( ERR_FATAL, "glGetString is NULL" );

	//
	// get the OpenGL config vars
	//

	N_strncpyz( glContext.vendor, (const char *)nglGetString( GL_VENDOR ), sizeof(glContext.vendor) );
	N_strncpyz( glContext.renderer, (const char *)nglGetString( GL_RENDERER ), sizeof(glContext.renderer) );
	N_strncpyz( glContext.version_str, (const char *)nglGetString( GL_VERSION ), sizeof(glContext.version_str) );
	N_strncpyz( gl_extensions, (const char *)nglGetString( GL_EXTENSIONS ), sizeof(gl_extensions) );

	N_strncpyz( glConfig.renderer_string, glContext.renderer, sizeof(glConfig.renderer_string) );
	N_strncpyz( glConfig.version_string, glContext.version_str, sizeof(glConfig.version_string) );
	N_strncpyz( glConfig.vendor_string, glContext.vendor, sizeof(glConfig.vendor_string) );
	N_strncpyz( glConfig.shader_version_str, (const char *)nglGetString( GL_SHADING_LANGUAGE_VERSION ), sizeof(glConfig.shader_version_str) );
	N_strncpyz( glConfig.extensions_string, gl_extensions, sizeof(glConfig.extensions_string) );

	nglGetIntegerv( GL_NUM_EXTENSIONS, &glContext.numExtensions );
	nglGetIntegerv( GL_STEREO, (GLint *)&glContext.stereo );
	nglGetIntegerv( GL_MAX_IMAGE_UNITS, &glContext.maxTextureUnits );
	nglGetIntegerv( GL_MAX_TEXTURE_SIZE, &glContext.maxTextureSize );
	nglGetIntegerv( GL_MAX_SAMPLES, &glContext.maxSamples );

	nglGetIntegeri_v( GL_MAX_COMPUTE_WORK_GROUP_COUNT, 0, &glContext.workGroupCount[0] );
	nglGetIntegeri_v( GL_MAX_COMPUTE_WORK_GROUP_COUNT, 1, &glContext.workGroupCount[1] );
	nglGetIntegeri_v( GL_MAX_COMPUTE_WORK_GROUP_COUNT, 2, &glContext.workGroupCount[2] );

	nglGetIntegeri_v( GL_MAX_COMPUTE_WORK_GROUP_SIZE, 0, &glContext.workGroupSize[0] );
	nglGetIntegeri_v( GL_MAX_COMPUTE_WORK_GROUP_SIZE, 1, &glContext.workGroupSize[1] );
	nglGetIntegeri_v( GL_MAX_COMPUTE_WORK_GROUP_SIZE, 2, &glContext.workGroupSize[2] );

	nglGetIntegerv( GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS, &glContext.workGroupInvocations );

	sscanf( glContext.version_str, "%i.%i", &glContext.versionMajor, &glContext.versionMinor );

	ri.Printf( PRINT_INFO, "Getting OpenGL version...\n" );
	// check OpenGL version
	if ( !NGL_VERSION_ATLEAST( 3, 3 ) ) {
		if ( !r_allowLegacy->i ) {
			ri.Error( ERR_FATAL, "OpenGL version must be at least 3.3, please install more recent drivers" );
		}
		else {
			ri.Printf( PRINT_DEVELOPER, "r_allowLegacy enabled and we have a legacy api version\n" );
			ri.Printf( PRINT_INFO, "...using Legacy OpenGL API:" );
		}
		ri.Printf( PRINT_ERROR, "Update your fucking drivers, jesus...\n" );
	}
	else {
		ri.Printf( PRINT_INFO, "...using OpenGL API:" );
	}
	ri.Printf( PRINT_INFO, " v%s\n", glContext.version_str );

	//
	// check for broken driver
	//

	if ( !glContext.numExtensions ) {
		ri.Error( ERR_FATAL, "Broken OpenGL installation, GL_NUM_EXTENSIONS <= 0" );
	}
	if ( !glContext.maxTextureUnits ) {
		ri.Error( ERR_FATAL, "Broken OpenGL installation, GL_MAX_TEXTURE_UNITS <= 0" );
	}

	if ( NGL_VERSION_ATLEAST( 3, 0 ) ) { // force the loading of specific procs
#ifdef _NOMAD_DEBUG
#define NGL( ret, name, ... ) n ## name = (PFN ## name) ri.GL_GetProcAddress( #name ); \
							if (!n ## name) ri.Error( ERR_FATAL, "Failed to load OpenGL proc '" #name "'" );
#elif defined(_NOMAD_EXPERIMENTAL)
#define NGL( ret, name, ... ) n ## name = (PFN ## name) ri.GL_GetProcAddress( #name ); \
							if (!n ## name) ri.Printf( PRINT_INFO, COLOR_YELLOW "Failed to load OpenGL proc '" #name "'\n" );
#else
#define NGL( ret, name, ... ) n ## name = (PFN ## name) ri.GL_GetProcAddress(#name);
#endif
		NGL_VertexArray_Procs
		NGL_Buffer_Procs
		NGL_FBO_Procs
#undef NGL
	}

	// check if we need Intel graphics specific fixes
	glContext.intelGraphics = qfalse;
	if ( strstr( (const char *)nglGetString( GL_RENDERER ), "Intel" ) ) {
		glContext.intelGraphics = qtrue;
	}
	
	R_InitExtensions();
}

static void RE_GetTextureId( nhandle_t hShader, uint32_t stageNum, uint32_t *id )
{
	shader_t *shader;

	shader = R_GetShaderByHandle( hShader );

	if ( !shader ) {
		ri.Printf( PRINT_WARNING, "RE_GetTextureId: Invalid shader given" );
		*id = 0;
		return;
	}

	*id = shader->stages[stageNum]->bundle[0].image[0]->id;
}

static void R_InitImGui( void )
{
	imguiGL3Import_t import;

	import.glGetString = nglGetString;
	import.glGetStringi = nglGetStringi;
	import.glBindSampler = nglBindSampler;
	import.glScissor = nglScissor;
	import.glViewport = nglViewport;
	import.glUniform1i = nglUniform1i;
	import.glUniform1f = nglUniform1f;
	import.glGetAttribLocation = nglGetAttribLocation;
	import.glGetUniformLocation = nglGetUniformLocation;
	import.glUniformMatrix4fv = nglUniformMatrix4fv;
	import.glEnable = nglEnable;
	import.glDisable = nglDisable;
	import.glGetIntegerv = nglGetIntegerv;
	import.glBlendEquation = nglBlendEquation;
	import.glBlendFuncSeparate = nglBlendFuncSeparate;
	import.glBlendEquationSeparate = nglBlendEquationSeparate;
	import.glPolygonMode = nglPolygonMode;
	import.glPixelStorei = nglPixelStorei;
	import.glIsEnabled = nglIsEnabled;
	import.glIsDisabled = nglIsDisabled;
	import.glIsProgram = nglIsProgram;
	import.glIsShader = nglIsShader;
	import.glDrawElementsBaseVertex = nglDrawElementsBaseVertex;
	import.glDrawElements = nglDrawElements;
	import.glGetVertexAttribPointerv = nglGetVertexAttribPointerv;
	import.glVertexAttribPointer = nglVertexAttribPointer;
	import.glEnableVertexAttribArray = nglEnableVertexAttribArray;
	import.glDisableVertexAttribArray = nglDisableVertexAttribArray;
	import.glBufferData = nglBufferData;
	import.glBufferSubData = nglBufferSubData;
	import.glGetVertexAttribiv = nglGetVertexAttribiv;
	import.glBindTexture = nglBindTexture;
	import.glUseProgram = nglUseProgram;
	import.glBindBuffer = nglBindBuffer;
	import.glBindVertexArray = nglBindVertexArray;
	import.glGenBuffers = nglGenBuffers;
	import.glGenVertexArrays = nglGenVertexArrays;
	import.glDeleteBuffers = nglDeleteBuffers;
	import.glDeleteVertexArrays = nglDeleteVertexArrays;
	import.glGenTextures = nglGenTextures;
	import.glDeleteTextures = nglDeleteTextures;
	import.glTexParameteri = nglTexParameteri;
	import.glTexImage2D = nglTexImage2D;
	import.glActiveTexture = nglActiveTexture;
	import.glAlphaFunc = nglAlphaFunc;
	import.glClear = nglClear;
	import.glClearColor = nglClearColor;
	import.glInvalidateBufferData = nglInvalidateBufferData;
	import.glUnmapBuffer = nglUnmapBuffer;
	import.glMapBufferRange = nglMapBufferRange;
	import.glBufferStorage = nglBufferStorage;
	import.glGetTextureHandleARB = nglGetTextureHandleARB;
	import.glMakeTextureHandleResidentARB = nglMakeTextureHandleResidentARB;
	import.glFlushMappedBufferRangeARB = nglFlushMappedBufferRange;

	import.DrawShaderStages = RB_DrawShaderStages;
	import.GetTextureId = RE_GetTextureId;
	import.GetShaderByHandle = (void *(*)( nhandle_t ))R_GetShaderByHandle;
	import.SetAttribPointers = VBO_SetVertexPointers;

	ri.ImGui_Init( (void *)(uintptr_t)rg.imguiShader.programId, &import );
}

static void R_AllocBackend( void ) {
	uint64_t size;
	uint64_t polyVertBytes;
	uint64_t vertBytes;
	uint64_t polyBytes;
	uint64_t indexBytes;
	uint64_t dlightBytes;
	uint64_t entityBytes;
	int i;

	vertBytes = PAD( sizeof( srfVert_t ) * r_maxPolys->i * 4, sizeof(uintptr_t) );
	polyVertBytes = PAD( sizeof(polyVert_t) * r_maxPolys->i * 4, sizeof(uintptr_t) );
	polyBytes = PAD( sizeof(srfPoly_t) * r_maxPolys->i, sizeof(uintptr_t) );
	indexBytes = PAD( sizeof(glIndex_t) * r_maxPolys->i * 6, sizeof(uintptr_t) );
	entityBytes = PAD( sizeof(renderEntityDef_t) * r_maxEntities->i, sizeof(uintptr_t) );
	dlightBytes = PAD( sizeof(dlight_t) * r_maxDLights->i, sizeof(uintptr_t) );

	size = 0;
	size += PAD( sizeof( renderBackendData_t ), sizeof( uintptr_t ) );
	size += PAD( sizeof( srfVert_t ) * r_maxPolys->i * 4, sizeof( uintptr_t ) );
	size += PAD( sizeof( polyVert_t ) * r_maxPolys->i * 4, sizeof( uintptr_t ) );
	size += PAD( sizeof( srfPoly_t ) * r_maxPolys->i, sizeof( uintptr_t ) );
	size += PAD( sizeof( glIndex_t ) * r_maxPolys->i * 6, sizeof( uintptr_t ) );
	size += PAD( sizeof( renderEntityDef_t ) * r_maxEntities->i, sizeof( uintptr_t ) );
	size += PAD( sizeof( dlight_t ) * r_maxDLights->i, sizeof( uintptr_t ) );

	backendData[ 0 ] = (renderBackendData_t *)ri.Malloc( size );
	backendData[ 0 ]->verts = (srfVert_t *)( backendData[ 0 ] + 1 );
	backendData[ 0 ]->polyVerts = (polyVert_t *)( backendData[ 0 ]->verts + r_maxPolys->i * 4 );
	backendData[ 0 ]->polys = (srfPoly_t *)( backendData[ 0 ]->polyVerts + r_maxPolys->i * 4 );
	backendData[ 0 ]->indices = (glIndex_t *)( backendData[ 0 ]->polys + r_maxPolys->i );
	backendData[ 0 ]->entities = (renderEntityDef_t *)( backendData[ 0 ]->indices + r_maxPolys->i * 6 );
	backendData[ 0 ]->dlights = (dlight_t *)( backendData[ 0 ]->entities + r_maxEntities->i );

	if ( !sys_forceSingleThreading->i ) {
		backendData[ 1 ] = (renderBackendData_t *)ri.Malloc( size );
		backendData[ 1 ]->verts = (srfVert_t *)( backendData[ 1 ] + 1 );
		backendData[ 1 ]->polyVerts = (polyVert_t *)( backendData[ 1 ]->verts + r_maxPolys->i * 4 );
		backendData[ 1 ]->polys = (srfPoly_t *)( backendData[ 1 ]->polyVerts + r_maxPolys->i * 4 );
		backendData[ 1 ]->indices = (glIndex_t *)( backendData[ 1 ]->polys + r_maxPolys->i );
		backendData[ 1 ]->entities = (renderEntityDef_t *)( backendData[ 1 ]->indices + r_maxPolys->i * 6 );
		backendData[ 1 ]->dlights = (dlight_t *)( backendData[ 1 ]->entities + r_maxEntities->i );
	} else {
		backendData[ 1 ] = NULL;
	}

	ri.Printf( PRINT_DEVELOPER,
		COLOR_CYAN "---------- Renderer Backend Allocation Info ----------\n"
		COLOR_CYAN "%-10lu Bytes : %-8.04lf KiB : %-4.04lf MiB allocated to renderer backend\n"
		COLOR_CYAN "%-10lu Bytes : %-8.04lf KiB : %-4.04lf MiB allocated for vertices\n" 
		COLOR_CYAN "%-10lu Bytes : %-8.04lf KiB : %-4.04lf MiB allocated for polygon vertices\n"
		COLOR_CYAN "%-10lu Bytes : %-8.04lf KiB : %-4.04lf MiB allocated for polygons\n"
		COLOR_CYAN "%-10lu Bytes : %-8.04lf KiB : %-4.04lf MiB allocated for polygon indices\n"
		COLOR_CYAN "%-10lu Bytes : %-8.04lf KiB : %-4.04lf MiB allocated for renderer entities\n"
		COLOR_CYAN "%-10lu Bytes : %-8.04lf KiB : %-4.04lf MiB allocated for dynamic lights\n"
		COLOR_CYAN "--------------------\n"
	, size, ( (double)size / 1024 ), ( (double)size / 1024 / 1024 ),
	vertBytes, ( (double)vertBytes / 1024 ), ( (double)vertBytes / 1024 / 1024 ),
	polyVertBytes, ( (double)polyVertBytes / 1024 ), ( (double)polyVertBytes / 1024 / 1024 ),
	polyBytes, ( (double)polyBytes / 1024 ), ( (double)polyBytes / 1024 / 1024 ),
	indexBytes, ( (double)indexBytes / 1024 ), ( (double)indexBytes / 1024 / 1024 ),
	entityBytes, ( (double)entityBytes / 1024 ), ( (double)entityBytes / 1024 / 1024 ),
	dlightBytes, ( (double)dlightBytes / 1024 ), ( (double)dlightBytes / 1024 / 1024 ) );
}

static void R_CameraInfo_f( void ) {
	ri.Printf( PRINT_INFO, "\n---------- Camera Info ----------\n" );
	ri.Printf( PRINT_INFO, "** Matrix Dump: Projection Matrix **\n" );
	Mat4Dump( glState.viewData.camera.projectionMatrix );
	ri.Printf( PRINT_INFO, "** Matrix Dump: View Matrix **\n" );
	Mat4Dump( glState.viewData.camera.viewMatrix );
	ri.Printf( PRINT_INFO, "** Matrix Dump: View Projection Matrix **\n" );
	Mat4Dump( glState.viewData.camera.viewProjectionMatrix );

	ri.Printf( PRINT_INFO, "\n" );
	ri.Printf( PRINT_INFO, "Origin: %f, %f\n", glState.viewData.camera.origin[0], glState.viewData.camera.origin[1] );
	ri.Printf( PRINT_INFO, "Zoom: %f\n", glState.viewData.camera.zoom );
	ri.Printf( PRINT_INFO, "Aspect: %f\n", glState.viewData.camera.aspect );
}

static void R_InitSamplers( void )
{
	nglGenSamplers( MAX_TEXTURE_UNITS, rg.samplers );

	nglSamplerParameteri( rg.samplers[TexFilter_Bilinear], GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	nglSamplerParameteri( rg.samplers[TexFilter_Bilinear], GL_TEXTURE_MAG_FILTER, GL_LINEAR );

	nglSamplerParameteri( rg.samplers[TexFilter_Nearest], GL_TEXTURE_MIN_FILTER, GL_NEAREST );
	nglSamplerParameteri( rg.samplers[TexFilter_Nearest], GL_TEXTURE_MAG_FILTER, GL_NEAREST );

	nglSamplerParameteri( rg.samplers[TexFilter_LinearNearest], GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	nglSamplerParameteri( rg.samplers[TexFilter_LinearNearest], GL_TEXTURE_MAG_FILTER, GL_NEAREST );

	nglSamplerParameteri( rg.samplers[TexFilter_NearestLinear], GL_TEXTURE_MIN_FILTER, GL_NEAREST );
	nglSamplerParameteri( rg.samplers[TexFilter_NearestLinear], GL_TEXTURE_MAG_FILTER, GL_LINEAR );
}

void R_Init( void )
{
	GLenum error;
	uint32_t i;

	ri.Printf( PRINT_INFO, "---------- RE_Init ----------\n" );

	// clear all globals
	memset( &rg, 0, sizeof( rg ) );
	memset( &backend, 0, sizeof( backend ) );
	
	glState.viewData.camera.zoom = 1.0f;
	screenshotFrame = qfalse;

	ri.GLimp_AcquireContext();

	//
	// init function tables
	//
	for ( i = 0; i < FUNCTABLE_SIZE; i++ ) {
		rg.sinTable[i]		= sin( DEG2RAD( i * 360.0f / ( ( float ) ( FUNCTABLE_SIZE - 1 ) ) ) );
		rg.squareTable[i]	= ( i < FUNCTABLE_SIZE/2 ) ? 1.0f : -1.0f;
		rg.sawToothTable[i] = (float)i / FUNCTABLE_SIZE;
		rg.inverseSawToothTable[i] = 1.0f - rg.sawToothTable[i];

		if ( i < FUNCTABLE_SIZE / 2 ) {
			if ( i < FUNCTABLE_SIZE / 4 ) {
				rg.triangleTable[i] = ( float ) i / ( FUNCTABLE_SIZE / 4 );
			}
			else {
				rg.triangleTable[i] = 1.0f - rg.triangleTable[i-FUNCTABLE_SIZE / 4];
			}
		}
		else {
			rg.triangleTable[i] = -rg.triangleTable[i-FUNCTABLE_SIZE/2];
		}
	}

	R_NoiseInit();

	R_Register();

	R_AllocBackend();

	R_InitNextFrame();

	R_InitGLContext();

	R_InitTextures();

	nglGenQueries( 3, rg.queries );

	if ( glContext.ARB_framebuffer_object ) {
		FBO_Init();
	}
	
	GLSL_InitGPUShaders();

	R_InitGPUBuffers();

	R_InitShaders();

	// init samplers
	R_InitSamplers();

	error = nglGetError();
	if ( error != GL_NO_ERROR ) {
		ri.Printf( PRINT_INFO, COLOR_RED "glGetError() = 0x%x\n", error );
	}

	// print info
	GpuInfo_f();
	GpuMemInfo_f();
	ri.Printf( PRINT_INFO, "---------- finished RE_Init ----------\n" );
}

static pthread_t initThread;

static void *InitRenderer( void * )
{
	R_Init();

	return NULL;
}

void RE_BeginRegistration( gpuConfig_t *config )
{
	if ( !ri.Cvar_VariableInteger( "sys_forceSingleThreading" ) ) {
		int ret;

		if ( ( ret = pthread_create( &initThread, NULL, InitRenderer, NULL ) ) ) {
			ri.Printf( PRINT_ERROR, "Error creating InitRenderer thread, pthread_create(): %s\n", strerror( ret ) );
			R_Init();
		}
	} else {
		R_Init();
	}
	rg.registered = qtrue;
	*config = glConfig;
}

void RE_Shutdown( refShutdownCode_t code )
{
	ri.Printf( PRINT_INFO, "RE_Shutdown( %i )\n", code );

	ri.Cmd_RemoveCommand( "texturelist" );
	ri.Cmd_RemoveCommand( "shaderlist" );
	ri.Cmd_RemoveCommand( "screenshot" );
	ri.Cmd_RemoveCommand( "gpuinfo" );
	ri.Cmd_RemoveCommand( "gpumeminfo" );
	ri.Cmd_RemoveCommand( "camerainfo" );
	ri.Cmd_RemoveCommand( "unloadworld" );
	ri.Cmd_RemoveCommand( "fbo_restart" );
	ri.Cmd_RemoveCommand( "fbolist" );

	if ( rg.registered ) {
		R_IssuePendingRenderCommands();

		R_ShutdownCommandBuffers();

		nglDeleteQueries( 3, rg.queries );
		nglDeleteSamplers( MAX_TEXTURE_UNITS, rg.samplers );

		R_DeleteTextures();
		R_ShutdownGPUBuffers();
		GLSL_ShutdownGPUShaders();

		ri.ImGui_Shutdown();
	}

	// shutdown platform specific OpenGL thingies
	if ( code != REF_KEEP_CONTEXT ) {
		ri.GLimp_Shutdown( code == REF_UNLOAD_DLL ? qtrue : qfalse );

		memset( &glConfig, 0, sizeof( glConfig ) );
		memset( &glState, 0, sizeof( glState ) );
		memset( &glContext, 0, sizeof( glContext ) );
	}

	// free everything
	ri.FreeAll();
	rg.registered = qfalse;
}

/*
=============
RE_EndRegistration

Touch all images to make sure they are resident (probably obsolete on modern systems)
=============
*/
void RE_EndRegistration( void ) {
	R_IssuePendingRenderCommands();
//    RB_ShowImages(); // not doing it here
}

void RE_GetConfig( gpuConfig_t *config ) {
	*config = glConfig;
}

void RE_GetGPUFrameStats( uint32_t *time, uint32_t *samples, uint32_t *primitives ) {
	*time = rg.queryCounts[TIME_QUERY];
	*samples = rg.queryCounts[SAMPLES_QUERY];
	*primitives = rg.queryCounts[PRIMTIVES_QUERY];
}

void *RE_GetImGuiTextureData( nhandle_t hShader )
{
	shader_t *shader;

	shader = R_GetShaderByHandle( hShader );
	if ( !shader ) {
		ri.Printf( PRINT_WARNING, "RE_GetImGuiTextureData: invalid shader given\n" );
		return NULL;
	}

	return (void *)(intptr_t)shader->stages[0]->bundle[0].image[0]->id;
}

void R_VertexLighting( qboolean allowed )
{
	rg.vertexLightingAllowed = allowed;
}

void RE_GetGPUMemStats( gpuMemory_t *memstats )
{
	*memstats = glState.memstats;
}

void RE_WaitRegistered( void )
{
	if ( ri.Cvar_VariableInteger( "sys_forceSingleThreading" ) ) {
		// init imgui
		R_InitImGui();

		return;
	}

	pthread_join( initThread, NULL );

	rg.registered = qtrue;

	ri.GLimp_AcquireContext();

	// init imgui
	R_InitImGui();

	// only create the smp thread after we've fully initialized
	// otherwise we just hang in the init thread
	R_InitCommandBuffers();
}

GDR_EXPORT renderExport_t *GDR_DECL GetRenderAPI( uint32_t version, refimport_t *import )
{
	static renderExport_t re;

	ri = *import;
	memset( &re, 0, sizeof( re ) );

	if ( version != NOMAD_VERSION_FULL ) {
		ri.Error( ERR_FATAL, "GetRenderAPI: rendergl version (%i) != glnomad engine version (%i)", NOMAD_VERSION_FULL, version );
	}

	re.Shutdown = RE_Shutdown;
	re.BeginRegistration = RE_BeginRegistration;
	re.RegisterShader = RE_RegisterShader;
	re.GetConfig = RE_GetConfig;
	re.ImGui_TextureData = RE_GetImGuiTextureData;
	re.GetGPUFrameStats = RE_GetGPUFrameStats;
	re.GetGPUMemStats = RE_GetGPUMemStats;

	re.ClearScene = RE_ClearScene;
	re.BeginScene = RE_BeginScene;
	re.EndScene = RE_EndScene;
	re.RenderScene = RE_RenderScene;

	re.BeginFrame = RE_BeginFrame;
	re.EndFrame = RE_EndFrame;

	re.RegisterSprite = RE_RegisterSprite;
	re.RegisterSpriteSheet = RE_RegisterSpriteSheet;
	re.RegisterShader = RE_RegisterShader;

	re.LoadWorld = RE_LoadWorldMap;
	re.EndRegistration = RE_EndRegistration;
	
	re.AddDynamicLightToScene = RE_AddDynamicLightToScene;
	re.AddPolyListToScene = RE_AddPolyListToScene;
	re.AddSpriteToScene = RE_AddSpriteToScene;
	re.AddPolyToScene = RE_AddPolyToScene;
	re.AddEntityToScene = RE_AddEntityToScene;
	re.SetColor = RE_SetColor;
	re.DrawImage = RE_DrawImage;

	re.VertexLighting = R_VertexLighting;
	re.CanMinimize = NULL;
	re.ThrottleBackend = NULL;
	re.FinishBloom = NULL;
	re.WaitRegistered = RE_WaitRegistered;

	return &re;
}

void R_GLDebug_Callback_AMD(GLuint id, GLenum category, GLenum severity, GLsizei length, const GLchar *message, GLvoid *userParam)
{

}

#define MAX_CACHED_GL_MESSAGES 8192
static char *cachedGLMessages[ MAX_CACHED_GL_MESSAGES ];
static int numCachedGLMessages = 0;

void R_GLDebug_Callback_ARB( GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message, const GLvoid *userParam )
{
	const char *color;
	uint64_t len, i;
	char msg[1024];

	// save the messages so OpenGL can't spam us with useless shit
	for ( i = 0; i < numCachedGLMessages; i++ ) {
		if ( !N_stricmp( cachedGLMessages[i], message ) ) {
			return;
		}
	}

	len = strlen( message ) + 1;
	cachedGLMessages[ numCachedGLMessages ] = ri.Hunk_Alloc( len, h_low );
	cachedGLMessages[ numCachedGLMessages ][ len - 1 ] = '\0';
	strcpy( cachedGLMessages[ numCachedGLMessages ], message );
	numCachedGLMessages++;

	if ( severity == GL_DEBUG_SEVERITY_NOTIFICATION ) {
		ri.Printf( PRINT_INFO, COLOR_MAGENTA "[GLDebug Log]:" COLOR_WHITE " %s\n", message );
		return;
	}

	if ( !color ) {
		if ( type == GL_DEBUG_TYPE_ERROR_ARB || type == GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR_ARB ) {
			color = COLOR_RED;
		} else if ( type == GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR_ARB ) {
			color = COLOR_YELLOW;
		} else if ( type == GL_DEBUG_TYPE_PERFORMANCE_ARB || type == GL_DEBUG_TYPE_PORTABILITY_ARB ) {
			color = COLOR_CYAN;
		} else {
			color = COLOR_WHITE;
		}
	}

	msg[ 0 ] = '\0';
	N_strcat( msg, sizeof( msg ) - 1, va( "%s[GLDebug Log] " COLOR_WHITE " %s\n", color, message ) );

	switch ( source ) {
	case GL_DEBUG_SOURCE_API_ARB:
		N_strcat( msg, sizeof( msg ) - 1, "\tSource: GL_DEBUG_SOURCE_API_ARB\n" );
		break;
	case GL_DEBUG_SOURCE_APPLICATION_ARB:
		N_strcat( msg, sizeof( msg ) - 1, "\tSource: GL_DEBUG_SOURCE_APPLICATION_ARB\n" );
		break;
	case GL_DEBUG_SOURCE_OTHER_ARB:
		N_strcat( msg, sizeof( msg ) - 1, "\tSource: GL_DEBUG_SOURCE_OTHER_ARB\n" );
		break;
	case GL_DEBUG_SOURCE_WINDOW_SYSTEM_ARB:
		N_strcat( msg, sizeof( msg ) - 1, "\tSource: GL_DEBUG_SOURCE_WINDOW_SYSTEM_ARB\n" );
		break;
	case GL_DEBUG_SOURCE_THIRD_PARTY_ARB:
		N_strcat( msg, sizeof( msg ) - 1, "\tSource: GL_DEBUG_SOURCE_THIRD_PARTY_ARB\n" );
		break;
	case GL_DEBUG_SOURCE_SHADER_COMPILER_ARB:
		N_strcat( msg, sizeof( msg ) - 1, "\tSource: GL_DEBUG_SOURCE_SHADER_COMPILER_ARB\n" );
		break;
	};

	switch ( type ) {
	case GL_DEBUG_TYPE_ERROR_ARB:
		N_strcat( msg, sizeof( msg ) - 1, "\tType: GL_DEBUG_TYPE_ERROR_ARB\n" );
		break;
	case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR_ARB:
		N_strcat( msg, sizeof( msg ) - 1, "\tType: GL_DEBUG_TYPE_DEPRECATED_BEHAVIOUR_ARB\n" );
		break;
	case GL_DEBUG_TYPE_PERFORMANCE_ARB:
		N_strcat( msg, sizeof( msg ) - 1, "\tType: GL_DEBUG_TYPE_PERFORMANCE_ARB\n" );
		break;
	case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR_ARB:
		N_strcat( msg, sizeof( msg ) - 1, "\tType: GL_DEBUG_TYPE_UNDEFINED_BEHAVIOUR_ARB\n" );
		break;
	case GL_DEBUG_TYPE_PORTABILITY_ARB:
		N_strcat( msg, sizeof( msg ) - 1, "\tType: GL_DEBUG_TYPE_PORTABILITY_ARB\n" );
		break;
	case GL_DEBUG_TYPE_OTHER_ARB:
		N_strcat( msg, sizeof( msg ) - 1, "\tType: GL_DEBUG_TYPE_OTHER_ARB\n" );
		break;
	};

	switch ( severity ) {
	case GL_DEBUG_SEVERITY_HIGH_ARB:
		N_strcat( msg, sizeof( msg ) - 1, "\tSeverity: GL_DEBUG_SEVERITY_HIGH_ARB\n" );
		break;
	case GL_DEBUG_SEVERITY_MEDIUM_ARB:
		N_strcat( msg, sizeof( msg ) - 1, "\tSeverity: GL_DEBUG_SEVERITY_MEDIUM_ARB\n" );
		break;
	case GL_DEBUG_SEVERITY_LOW_ARB:
		N_strcat( msg, sizeof( msg ) - 1, "\tSeverity: GL_DEBUG_SEVERITY_LOW_ARB\n" );
		break;
	};
	N_strcat( msg, sizeof( msg ) - 1, "\n" );

	if ( r_glDebug->i ) {
		ri.Printf( PRINT_INFO, "%s", msg );
	}
	ri.GLimp_LogComment( msg );
}