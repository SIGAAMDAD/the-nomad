
#ifdef USE_LOCAL_HEADERS
#	include "SDL2/SDL.h"
#	include "SDL2/SDL_opengl.h"
#ifdef USE_VULKAN_API
#	include "SDL2/SDL_vulkan.h"
#endif
#else
#	include <SDL2/SDL.h>
#	include <SDL2/SDL_opengl.h>
#ifdef USE_VULKAN_API
#	include <SDL2/SDL_vulkan.h>
#endif
#endif
#define NOMAD_ICON_INCLUDE
#ifdef NOMAD_ICON_INCLUDE
	#include "sdl_icon.h"
#endif

#include "../game/g_game.h"
#include "../rendercommon/r_public.h"
#include "sdl_glw.h"

typedef enum {
	RSERR_OK,
	RSERR_INVALID_FULLSCREEN,
	RSERR_INVALID_MODE,
	RSERR_FATAL_ERROR,
	RSERR_UNKNOWN
} rserr_t;

glwstate_t glw_state;

SDL_Window *SDL_window = NULL;
SDL_GLContext SDL_glContext = NULL;
#ifdef USE_VULKAN_API
PFN_vkGetInstanceProcAddr qvkGetInstanceProcAddr;
#endif

cvar_t *r_stereoEnabled;
cvar_t *r_logfile;
cvar_t *in_nograb;

static int FindNearestDisplay( int *x, int *y, int width, int height )
{
	const int cx = *x + width / 2;
	const int cy = *y + height / 2;
	int i, index, numDisplays;
	SDL_Rect *list, *m;

	index = -1; // selected display index

	numDisplays = SDL_GetNumVideoDisplays();
	if ( numDisplays <= 0 ) {
		return -1;
	}

	glw_state.monitorCount = numDisplays;

	list = (SDL_Rect *)alloca( numDisplays * sizeof( *list ) );

	for ( i = 0; i < numDisplays; i++ ) {
		SDL_GetDisplayBounds( i, list + i );
	}

	// select display by window center intersection
	for ( i = 0; i < numDisplays; i++ ) {
		m = list + i;
		if ( cx >= m->x && cx < ( m->x + m->w ) && cy >= m->y && cy < ( m->y + m->h ) ) {
			index = i;
			break;
		}
	}

	// select display by nearest distance between window center and display center
	if ( index == -1 ) {
		unsigned long nearest, dist;
		int dx, dy;
		nearest = ~0UL;

		for ( i = 0; i < numDisplays; i++ ) {
			m = list + i;
			dx = ( m->x + m->w / 2 ) - cx;
			dy = ( m->y + m->h / 2 ) - cy;
			dist = ( dx * dx ) + ( dy * dy );
			if ( dist < nearest ) {
				nearest = dist;
				index = i;
			}
		}
	}

	// adjust x and y coordinates if needed
	if ( index >= 0 ) {
		m = list + index;
		if ( *x < m->x ) {
			*x = m->x;
		}
		if ( *y < m->y ) {
			*y = m->y;
		}
	}

	return index;
}

static void GLimp_QuerySystemContext( void )
{
	typedef const GLubyte *(*GetString_t)( GLenum );

	SDL_GLContext queryContext;
	GetString_t GetString;
	int hardwareGL;
	int versionMinor, versionMajor;
	const GLubyte *renderer;

	queryContext = SDL_GL_CreateContext( SDL_window );
	if ( !queryContext ) {
		N_Error( ERR_FATAL, "Error creating OpenGL System Query Context" );
	}
	SDL_GL_MakeCurrent( SDL_window, queryContext );
	
	SDL_GL_GetAttribute( SDL_GL_ACCELERATED_VISUAL, &hardwareGL );
	SDL_GL_GetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, &versionMinor );
	SDL_GL_GetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, &versionMajor );

	if ( versionMajor < 4 && versionMinor < 0 ) {
		N_Error( ERR_FATAL, "ERROR: cannot create an OpenGL context with version < 4.0" );
	}

	GetString = (GetString_t)SDL_GL_GetProcAddress( "glGetString" );
	if ( !GetString ) {
		N_Error( ERR_FATAL, "ERROR: couldn't get glGetString proc" );
	}

	renderer = GetString( GL_RENDERER );
	if ( N_stristr( "Intel", (const char *)renderer ) ) {
		hardwareGL = false;
	}

	SDL_GL_MakeCurrent( SDL_window, NULL );
	SDL_GL_DeleteContext( queryContext );

	Cvar_SetIntegerValue( "r_allowSoftwareGL", !hardwareGL );
}

static int GLimp_CreateBaseWindow( gpuConfig_t *config )
{
	PROFILE_FUNCTION();
	
	uint32_t windowFlags;
	uint32_t contextFlags;
	int32_t depthBits, stencilBits, colorBits;
	int32_t perChannelColorBits;
	int32_t x, y;
	int32_t i;

	// set window flags
	windowFlags = SDL_WINDOW_OPENGL;
	if ( r_fullscreen->i ) {
		// custom fullscreen or native?
		if ( r_mode->i == -2 ) {
			windowFlags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
		}
		else {
			windowFlags |= SDL_WINDOW_FULLSCREEN;
		}
	}
	if ( r_noborder->i ) {
		windowFlags |= SDL_WINDOW_BORDERLESS;
	}

	// destroy existing context if it exists
	if  ( SDL_glContext ) {
		SDL_GL_DeleteContext( SDL_glContext );
		SDL_glContext = NULL;
	}
	if ( SDL_window ) {
		SDL_GetWindowPosition( SDL_window, &x, &y );
		Con_DPrintf( "Existing window at %ix%i before destruction\n", x, y );
		SDL_DestroyWindow( SDL_window );
		SDL_window = NULL;
	}

	colorBits = r_colorBits->i;
	if ( colorBits == 0 || colorBits > 32 ) {
		colorBits = 32;
	}
	if ( g_depthBits->i == 0 ) {
		// implicitly assume Z-buffer depth == desktop color depth
		if ( colorBits > 16 ) {
			depthBits = 24;
		} else {
			depthBits = 16;
		}
	}
	else {
		depthBits = g_depthBits->i;
	}

	stencilBits = g_stencilBits->i;

	// do not allow stencil if Z-buffer depth likely won't contain it
	if ( depthBits < 24 ) {
		stencilBits = 0;
	}
	
	for ( i = 0; i < 16; i++ ) {
		int testColorBits, testDepthBits, testStencilBits;
		int realColorBits[3];

		// 0 - default
		// 1 - minus colorBits
		// 2 - minus depthBits
		// 3 - minus stencil
		if ( ( i % 4 ) == 0 && i ) {
			// one pass, reduce
			switch ( i / 4 ) {
			case 2:
				if ( colorBits == 24 ) {
					colorBits = 16;
				}
				break;
			case 1:
				if ( depthBits == 24 ) {
					depthBits = 16;
				} else if ( depthBits == 16 ) {
					depthBits = 8;
				}
			case 3:
				if ( stencilBits == 24 ) {
					stencilBits = 16;
				} else if ( stencilBits == 16 ) {
					stencilBits = 8;
				}
			};
		}

		testColorBits = colorBits;
		testDepthBits = depthBits;
		testStencilBits = stencilBits;

		if ( ( i % 4 ) == 3 ) { // reduce colorBits
			if ( testColorBits == 24 ) {
				testColorBits = 16;
			}
		}

		if ( ( i % 4 ) == 2 ) { // reduce depthBits
			if ( testDepthBits == 24 ) {
				testDepthBits = 16;
			} else if ( testDepthBits == 16 ) {
				testDepthBits = 8;
			}
		}

		if ( ( i % 4 ) == 1 ) { // reduce stencilBits
			if ( testStencilBits == 24 ) {
				testStencilBits = 16;
			} else if ( testStencilBits == 16 ) {
				testStencilBits = 8;
			} else {
				testStencilBits = 0;
			}
		}

		if ( testColorBits == 24 ) {
			perChannelColorBits = 8;
		} else {
			perChannelColorBits = 4;
		}
		
		SDL_GL_SetAttribute( SDL_GL_RED_SIZE, perChannelColorBits );
		SDL_GL_SetAttribute( SDL_GL_GREEN_SIZE, perChannelColorBits );
		SDL_GL_SetAttribute( SDL_GL_BLUE_SIZE, perChannelColorBits );

		SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE, testDepthBits );
		SDL_GL_SetAttribute( SDL_GL_STENCIL_SIZE, testStencilBits );

		SDL_GL_SetAttribute( SDL_GL_MULTISAMPLEBUFFERS, 0 );
		SDL_GL_SetAttribute( SDL_GL_MULTISAMPLESAMPLES, 0 );

		// [the-nomad] make sure we only create ONE window
		if ( !SDL_window ) {
			if ( ( SDL_window = SDL_CreateWindow( cl_title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
				config->vidWidth, config->vidHeight, windowFlags ) ) == NULL )
			{
				Con_DPrintf( "SDL_CreateWindow(%s, %i, %i, %x) failed: %s",
					cl_title, config->vidWidth, config->vidHeight, windowFlags, SDL_GetError() );
				return -1;
			}
		}
		if ( r_fullscreen->i ) {
			SDL_DisplayMode mode;

			switch ( testColorBits ) {
			case 16: mode.format = SDL_PIXELFORMAT_RGB565; break;
			case 24: mode.format = SDL_PIXELFORMAT_RGB24;  break;
			case 32: mode.format = SDL_PIXELFORMAT_RGBA32; break;
			default: Con_DPrintf( "testColorBits is %d, can't fullscreen\n", testColorBits ); continue;
			};

			mode.w = config->vidWidth;
			mode.h = config->vidHeight;
			mode.refresh_rate = Cvar_VariableInteger( "r_displayRefresh" );
			mode.driverdata = NULL;

			if ( SDL_SetWindowDisplayMode( SDL_window, &mode ) < 0 ) {
				Con_DPrintf( "SDL_SetWindowDisplayMode failed: %s\n", SDL_GetError( ) );
				continue;
			}

			if ( SDL_GetWindowDisplayMode( SDL_window, &mode ) >= 0 ) {
				config->displayFrequency = mode.refresh_rate;
				config->vidWidth = mode.w;
				config->vidHeight = mode.h;
			}
		}

		contextFlags = 0;
		if ( r_glDebug->i ) {
			contextFlags |= SDL_GL_CONTEXT_DEBUG_FLAG;
		}
		if ( !r_allowLegacy->i ) {
			contextFlags |= SDL_GL_CONTEXT_PROFILE_CORE;
		}

		GLimp_QuerySystemContext();

		// set the recommended version, this is not mandatory,
		// however if your driver isn't >= 3.3, that'll be
		// deprecated stuff
		SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, 4 );
		SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, 5 );
		SDL_GL_SetAttribute( SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY );

		if ( contextFlags ) {
			SDL_GL_SetAttribute( SDL_GL_CONTEXT_FLAGS, contextFlags );
		}

		SDL_GL_SetAttribute( SDL_GL_STEREO, r_stereoEnabled->i );
		SDL_GL_SetAttribute( SDL_GL_ACCELERATED_VISUAL, !r_allowSoftwareGL->i );
		SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );

		if ( !SDL_glContext ) {
			if ( ( SDL_glContext = SDL_GL_CreateContext( SDL_window ) ) == NULL ) {
				Con_DPrintf( "SDL_GL_CreateContext failed: %s\n", SDL_GetError( ) );
				SDL_DestroyWindow( SDL_window );
				SDL_window = NULL;
				continue;
			}
		}
		SDL_GL_MakeCurrent( SDL_window, SDL_glContext );
		if ( SDL_GL_SetSwapInterval( r_swapInterval->i ) == -1 ) {
			// NOTE: if you get negative swap isn't supported, that just means dynamic
			// vsync isn't available
			Con_DPrintf( "SDL_GL_SetSwapInterval failed: %s\n", SDL_GetError( ) );
		}
		SDL_GL_GetAttribute( SDL_GL_RED_SIZE, &realColorBits[0] );
		SDL_GL_GetAttribute( SDL_GL_GREEN_SIZE, &realColorBits[1] );
		SDL_GL_GetAttribute( SDL_GL_BLUE_SIZE, &realColorBits[2] );
		SDL_GL_GetAttribute( SDL_GL_DEPTH_SIZE, &config->depthBits );
		SDL_GL_GetAttribute( SDL_GL_STENCIL_SIZE, &config->stencilBits );

		config->colorBits = realColorBits[0] + realColorBits[1] + realColorBits[2];
	}
	Con_Printf( "Using %d color bits, %d depth, %d stencil display.\n",
		config->colorBits, config->depthBits, config->stencilBits );

	if ( SDL_window ) {
#ifdef NOMAD_ICON_INCLUDE
		SDL_Surface *icon = SDL_CreateRGBSurfaceFrom(
			(void *)GAME_WINDOW_ICON.pixel_data,
			GAME_WINDOW_ICON.width,
			GAME_WINDOW_ICON.height,
			GAME_WINDOW_ICON.bytes_per_pixel * 8,
			GAME_WINDOW_ICON.bytes_per_pixel * GAME_WINDOW_ICON.width,
#ifdef GDR_LITTLE_ENDIAN
			0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000
#else
			0xFF000000, 0x00FF0000, 0x0000FF00, 0x000000FF
#endif
		);
		if ( icon ) {
			SDL_SetWindowIcon( SDL_window, icon );
			SDL_FreeSurface (icon );
		}
		else {
			Con_DPrintf( "SDL_CreateRGBSurfaceFrom(WINDOW_ICON) == NULL\n" ); // just to let us know
		}
#endif
	}
	else {
		Con_Printf( "Failed video initialization\n" );
		return -1;
	}
	SDL_GL_MakeCurrent( SDL_window, SDL_glContext );
	SDL_GL_GetDrawableSize( SDL_window, &config->vidWidth, &config->vidHeight );

	return 1;
}

void GLimp_LogComment( const char *msg )
{
	if ( glw_state.logFile == FS_INVALID_HANDLE ) {
		return;
	}

	FS_Write( msg, strlen( msg ), glw_state.logFile );
}

void GLimp_Minimize( void )
{
	SDL_MinimizeWindow( SDL_window );
}

static rserr_t GLimp_StartDriverAndSetMode( int mode, const char *modeFS, qboolean fullscreen )
{
	rserr_t err;

	if ( !SDL_WasInit( SDL_INIT_VIDEO ) ) {
		const char *driverName;

		if ( SDL_Init( SDL_INIT_VIDEO ) != 0 ) {
			Con_Printf( "SDL_Init( SDL_INIT_VIDEO ) FAILED (%s)\n", SDL_GetError() );
			return RSERR_FATAL_ERROR;
		}

		driverName = SDL_GetCurrentVideoDriver();
		Con_Printf( "SDL2 using video driver \"%s\"\n", driverName );
	}
}

/*
* GLimp_Init: will initialize a new OpenGL
* window and context handle, will also handle all
* the cvar stuff
*/
void GLimp_Init( gpuConfig_t *config )
{
	uint32_t width, height;
	uint32_t windowFlags;
	float windowAspect;
	SDL_DisplayMode dm;

	Con_Printf( "GLimp_Init()\n" );

	in_nograb = Cvar_Get( "in_nograb", "0", 0 );
	Cvar_SetDescription( in_nograb, "Do not capture mouse in game, may be useful during online streaming." );

	r_allowSoftwareGL = Cvar_Get( "r_allowSoftwareGL", "0", CVAR_LATCH );
	Cvar_SetDescription( r_allowSoftwareGL,
		"Disables hardware acceleration to allow for a software (IGPU) graphics driver.\n"
		"Set this to \"0\" if you have a dedicated/discrete GPU."
	);

	r_logfile = Cvar_Get( "r_logfile", "1", CVAR_LATCH | CVAR_SAVE );
	Cvar_SetDescription( r_logfile, "Writes a dedicated OpenGL log." );

	r_swapInterval = Cvar_Get( "r_swapInterval", "1", CVAR_SAVE );

	r_logfile = Cvar_Get( "r_logfile", "0", CVAR_SAVE );
	Cvar_SetDescription( r_logfile, "Enables dedicated Renderer Driver logfile, use only for debugging purposes." );

	r_stereoEnabled = Cvar_Get( "r_stereoEnabled", "0", CVAR_SAVE | CVAR_LATCH );
	Cvar_SetDescription( r_stereoEnabled, "Enable stereo rendering for techniques like shutter glasses." );

	if ( !SDL_WasInit( SDL_INIT_VIDEO ) ) {
		if ( SDL_Init( SDL_INIT_VIDEO ) != 0 ) {
			N_Error( ERR_FATAL, "SDL_Init(SDL_INIT_VIDEO) Failed: %s", SDL_GetError() );
			return;
		}
	}

	if ( r_logfile->i ) {
		Con_Printf( "Creating dedicated OpenGL logfile...\n" );
		glw_state.logFile = FS_FOpenWrite( "Log/graphics.log" );
		if ( glw_state.logFile ) {
			Con_Printf( COLOR_RED "Error opening file \"Log/graphics.log\"!\n" );
		}
	}

	if ( SDL_GetDesktopDisplayMode( 0, &dm ) != 0 ) {
		N_Error( ERR_FATAL, "SDL_GetDesktopDisplayMode failed: %s", SDL_GetError() );
	}

	gi.desktopWidth = dm.w;
	gi.desktopHeight = dm.h;

	in_nograb = Cvar_Get( "in_nograb", "0", 0 );
	Cvar_SetDescription( in_nograb, "Do not capture mouse in game, may be useful during online streaming." );

	const char *driverName;

	if ( !GLimp_CreateBaseWindow( config ) ) {
		N_Error( ERR_FATAL, "Failed to init OpenGL\n" );
	}

	driverName = SDL_GetCurrentVideoDriver();

	Con_Printf( "SDL using driver \"%s\"\n", driverName );

	// These values force the UI to disable driver selection
	config->driverType = GLDRV_ICD;
	config->hardwareType = GLHW_GENERIC;
}

void GLimp_Shutdown( qboolean unloadDLL )
{
	IN_Shutdown();

	SDL_DestroyWindow( SDL_window );
	SDL_window = NULL;

	if ( glw_state.logFile ) {
		FS_FClose( glw_state.logFile );
	}

	if ( glw_state.isFullscreen ) {
		SDL_WarpMouseGlobal( glw_state.desktop_width / 2, glw_state.desktop_height / 2 );
	}
	if ( unloadDLL ) {
		SDL_QuitSubSystem( SDL_INIT_VIDEO );
	}
}

void GLimp_EndFrame( void )
{
	// don't flip if drawing to front buffer
	if ( N_stricmp( g_drawBuffer->s, "GL_FRONT" ) !=  0) {
		SDL_GL_SwapWindow( SDL_window );
	}
	if ( r_fullscreen->modified ) {
		int fullscreen;
		qboolean needToToggle;
		qboolean sdlToggled = qfalse;

		// find out the current state
		fullscreen = !!( SDL_GetWindowFlags( SDL_window ) & SDL_WINDOW_FULLSCREEN );

		// is this any different from our current state?
		needToToggle = !!r_fullscreen->i != fullscreen;

		if ( needToToggle ) {
			sdlToggled = SDL_SetWindowFullscreen( SDL_window, r_fullscreen->i );

			// SDL_WM_ToggleFullScreen didn't work, so do it the slow way
			if ( !sdlToggled ) {
				Cbuf_ExecuteText( EXEC_APPEND, "vid_restart\n" );
			}

//			IN_Restart();
		}
		
		r_fullscreen->modified = qfalse;
	}
}

void *GL_GetProcAddress( const char *name )
{
	return SDL_GL_GetProcAddress( name );
}

void GLimp_HideFullscreenWindow( void )
{
	if ( SDL_window && glw_state.isFullscreen ) {
		SDL_HideWindow( SDL_window );
	}
}

static Uint16 r[256];
static Uint16 g[256];
static Uint16 b[256];

void GLimp_InitGamma( gpuConfig_t *config )
{
	config->deviceSupportsGamma = qfalse;

	if ( SDL_GetWindowGammaRamp( SDL_window, r, g, b ) == 0 ) {
		config->deviceSupportsGamma = SDL_SetWindowBrightness( SDL_window, 1.0f ) >= 0 ? qtrue : qfalse;
	}
}


/*
=================
GLimp_SetGamma
=================
*/
void GLimp_SetGamma( unsigned char red[256], unsigned char green[256], unsigned char blue[256] )
{
	Uint16 table[3][256];
	uint32_t i, j;

	for ( i = 0; i < 256; i++ ) {
		table[0][i] = ( ( ( Uint16 ) red[i] ) << 8 ) | red[i];
		table[1][i] = ( ( ( Uint16 ) green[i] ) << 8 ) | green[i];
		table[2][i] = ( ( ( Uint16 ) blue[i] ) << 8 ) | blue[i];
	}

#ifdef _WIN32
#include <windows.h>

	// Win2K and newer put this odd restriction on gamma ramps...
	{
		//OSVERSIONINFO	vinfo;
		//vinfo.dwOSVersionInfoSize = sizeof( vinfo );
		//GetVersionEx( &vinfo );
		//if( vinfo.dwMajorVersion >= 5 && vinfo.dwPlatformId == VER_PLATFORM_WIN32_NT )
		{
			qboolean clamped = qfalse;
			for ( j = 0 ; j < 3 ; j++ ) {
				for ( i = 0 ; i < 128 ; i++ ) {
					if ( table[ j ] [ i] > ( ( 128 + i ) << 8 ) ) {
						table[ j ][ i ] = ( 128 + i ) << 8;
						clamped = qtrue;
					}
				}

				if ( table[ j ] [127 ] > 254 << 8 ) {
					table[ j ][ 127 ] = 254 << 8;
					clamped = qtrue;
				}
			}
			if ( clamped ) {
				Con_DPrintf( "performing gamma clamp.\n" );
			}
		}
	}
#endif

	// enforce constantly increasing
	for ( j = 0; j < 3; j++ ) {
		for ( i = 1; i < 256; i++) {
			if ( table[j][i] < table[j][i-1] ) {
				table[j][i] = table[j][i-1];
			}
		}
	}

	if ( SDL_SetWindowGammaRamp( SDL_window, table[0], table[1], table[2] ) < 0 ) {
		Con_DPrintf( "SDL_SetWindowGammaRamp() failed: %s\n", SDL_GetError() );
	}
}

/*
* G_InitDisplay: called during renderer init
*/
void G_InitDisplay( gpuConfig_t *config )
{
	SDL_DisplayMode mode;

	if ( SDL_Init( SDL_INIT_VIDEO ) != 0 ) {
		N_Error( ERR_FATAL, "SDL_Init(SDL_INIT_VIDEO) Failed: %s", SDL_GetError() );
	}
	
	if ( SDL_GetDesktopDisplayMode( 0, &mode ) != 0 ) {
		Con_Printf( COLOR_YELLOW "SDL_GetDesktopDisplayMode() Failed: %s\n", SDL_GetError() );
		Con_Printf( COLOR_YELLOW "Setting mode to default of 1920x1080\n" );

		mode.refresh_rate = 60;
		mode.w = 1920;
		mode.h = 1080;
	}

	gi.desktopWidth = mode.w;
	gi.desktopHeight = mode.h;

	if ( !G_GetModeInfo( &config->vidWidth, &config->vidHeight, &config->windowAspect, r_mode->i,
		"", mode.w, mode.h, r_fullscreen->i ) )
	{
		Con_Printf( "Invalid r_mode, resetting...\n" );
		Cvar_ForceReset( "r_mode" );
		if ( !G_GetModeInfo( &config->vidWidth, &config->vidHeight, &config->windowAspect, r_mode->i,
			"", mode.w, mode.h, r_fullscreen->i ) )
		{
			Con_Printf( COLOR_YELLOW "Could not determine video mode, setting to default of 1920x1080\n" );

			config->vidWidth = 1920;
			config->vidHeight = 1080;
			config->windowAspect = 1;
		}
	}

	Con_Printf( "Setting up display\n" );
	Con_Printf( "...setting mode %i\n", r_mode->i );
	
	// init OpenGL
	GLimp_Init( config );

	// This depends on SDL_INIT_VIDEO, hence having it here
	IN_Init();

	HandleEvents();

	Key_ClearStates();
}
