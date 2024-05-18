#include "g_game.h"
#include "../rendercommon/imgui.h"
#include "g_threads.h"

/*
================
SCR_DrawNamedPic

Coordinates are 640*480 virtual values
=================
*/
void SCR_DrawNamedPic( float x, float y, float width, float height, const char *picname )
{
	nhandle_t	hShader;

	assert( width != 0 );

	hShader = re.RegisterShader( picname );
	SCR_AdjustFrom640( &x, &y, &width, &height );
	re.DrawImage( x, y, width, height, 0, 0, 1, 1, hShader );
}


/*
================
SCR_AdjustFrom640

Adjusted for resolution and screen aspect ratio
================
*/
void SCR_AdjustFrom640( float *x, float *y, float *w, float *h )
{
	float	xscale;
	float	yscale;

	// adjust for wide screens
	if ( gi.gpuConfig.vidWidth * 768.0f > gi.gpuConfig.vidHeight * 1024.0f ) {
		*x += 0.5 * ( gi.gpuConfig.vidWidth - ( gi.gpuConfig.vidHeight * 1024.0f / 768.0f ) );
	}

	// scale for screen sizes
	xscale = gi.gpuConfig.vidWidth / 1024.0f;
	yscale = gi.gpuConfig.vidHeight / 768.0f;
	if ( x ) {
		*x *= xscale;
	}
	if ( y ) {
		*y *= yscale;
	}
	if ( w ) {
		*w *= xscale;
	}
	if ( h ) {
		*h *= yscale;
	}
}

/*
================
SCR_FillRect

Coordinates are 640*480 virtual values
=================
*/
void SCR_FillRect( float x, float y, float width, float height, const float *color ) {
	re.SetColor( color );

	SCR_AdjustFrom640( &x, &y, &width, &height );
	re.DrawImage( x, y, width, height, 0, 0, 0, 0, gi.whiteShader );

	re.SetColor( NULL );
}


/*
================
SCR_DrawPic

Coordinates are 640*480 virtual values
=================
*/
void SCR_DrawPic( float x, float y, float width, float height, nhandle_t hShader ) {
	SCR_AdjustFrom640( &x, &y, &width, &height );
	re.DrawImage( x, y, width, height, 0, 0, 1, 1, hShader );
}


/*
** SCR_DrawChar
** chars are drawn at 640*480 virtual screen size
*/
static void SCR_DrawChar( uint32_t x, uint32_t y, float size, int ch ) {
	uint32_t row, col;
	float frow, fcol;
	float	ax, ay, aw, ah;

	ch &= 255;

	if ( ch == ' ' ) {
		return;
	}

	if ( y < -size ) {
		return;
	}

	ax = x;
	ay = y;
	aw = size;
	ah = size;
	SCR_AdjustFrom640( &ax, &ay, &aw, &ah );

	row = ch>>4;
	col = ch&15;

	frow = row*0.0625;
	fcol = col*0.0625;
	size = 0.0625;

	re.DrawImage( ax, ay, aw, ah,
					   fcol, frow, 
					   fcol + size, frow + size, 
					   gi.charSetShader );
}


/*
** SCR_DrawSmallChar
** small chars are drawn at native screen resolution
*/
void SCR_DrawSmallChar( uint32_t x, uint32_t y, int ch ) {
	uint32_t row, col;
	float frow, fcol;
	float size;

	ch &= 255;

	if ( ch == ' ' ) {
		return;
	}

	if ( y < -smallchar_height ) {
		return;
	}

	row = ch>>4;
	col = ch&15;

	frow = row*0.0625;
	fcol = col*0.0625;
	size = 0.0625;

//	re.DrawFromSpriteSheet( gi.charSet, col, row, x, y, smallchar_width, smallchar_height );

	re.DrawImage( x, y, smallchar_width, smallchar_height,
					   fcol, frow, 
					   fcol + size, frow + size, 
					   gi.charSetShader );
}


/*
** SCR_DrawSmallString
** small string are drawn at native screen resolution
*/
void SCR_DrawSmallString( uint32_t x, uint32_t y, const char *s, uint64_t len )
{
    uint32_t row, col, ch, i;
	float frow, fcol;
	float size;

	if ( y < -smallchar_height ) {
		return;
	}

	size = 0.0625;

	for ( i = 0; i < len; i++ ) {
		ch = *s++ & 255;
		row = ch>>4;
		col = ch&15;

		frow = row*0.0625;
		fcol = col*0.0625;

		re.DrawImage( x, y, smallchar_width, smallchar_height,
					   fcol, frow, 
					   fcol + size, frow + size,
					   gi.charSetShader );
//		re.DrawFromSpriteSheet( gi.charSet, col, row, x, y, smallchar_width, smallchar_height );
		x += smallchar_width;
	}
}


/*
==================
SCR_DrawBigString[Color]

Draws a multi-colored string with a drop shadow, optionally forcing
to a fixed color.

Coordinates are at 640 by 480 virtual resolution
==================
*/
void SCR_DrawStringExt( uint32_t x, uint32_t y, float size, const char *string, const float *setColor, qboolean forceColor,
	qboolean noColorEscape )
{
	vec4_t		color;
	const char	*s;
	uint32_t	xx;

	// draw the drop shadow
	color[0] = color[1] = color[2] = 0.0;
	color[3] = setColor[3];
	re.SetColor( color );
	s = string;
	xx = x;
	while ( *s ) {
		if ( !noColorEscape && Q_IsColorString( s ) ) {
			s += 2;
			continue;
		}
		SCR_DrawChar( xx+2, y+2, size, *s );
		xx += size;
		s++;
	}


	// draw the colored text
	s = string;
	xx = x;
	re.SetColor( setColor );
	while ( *s ) {
		if ( Q_IsColorString( s ) ) {
			if ( !forceColor ) {
				memcpy( color, g_color_table[ ColorIndexFromChar( *(s+1) ) ], sizeof( color ) );
				color[3] = setColor[3];
				re.SetColor( color );
			}
			if ( !noColorEscape ) {
				s += 2;
				continue;
			}
		}
		SCR_DrawChar( xx, y, size, *s );
		xx += size;
		s++;
	}
	re.SetColor( NULL );
}


/*
==================
SCR_DrawBigString
==================
*/
void SCR_DrawBigString( uint32_t x, uint32_t y, const char *s, float alpha, qboolean noColorEscape )
{
	float	color[4];

	color[0] = color[1] = color[2] = 1.0;
	color[3] = alpha;
	SCR_DrawStringExt( x, y, BIGCHAR_WIDTH, s, color, qfalse, noColorEscape );
}


/*
==================
SCR_DrawSmallString[Color]

Draws a multi-colored string with a drop shadow, optionally forcing
to a fixed color.
==================
*/
void SCR_DrawSmallStringExt( uint32_t x, uint32_t y, const char *string, const float *setColor, qboolean forceColor,
		qboolean noColorEscape )
{
	vec4_t		color;
	const char	*s;
	uint32_t	xx;

	// draw the colored text
	s = string;
	xx = x;
	re.SetColor( setColor );
	while ( *s ) {
		if ( Q_IsColorString( s ) ) {
			if ( !forceColor ) {
				memcpy( color, g_color_table[ ColorIndexFromChar( *(s+1) ) ], sizeof( color ) );
				color[3] = setColor[3];
				re.SetColor( color );
			}
			if ( !noColorEscape ) {
				s += 2;
				continue;
			}
		}
		SCR_DrawSmallChar( xx, y, *s );
		xx += smallchar_width;
		s++;
	}
	re.SetColor( NULL );
}


/*
** SCR_Strlen -- skips color escape codes
*/
static uint32_t SCR_Strlen( const char *str ) {
	const char *s = str;
	uint32_t count = 0;

	while ( *s ) {
		if ( Q_IsColorString( s ) ) {
			s += 2;
		} else {
			count++;
			s++;
		}
	}

	return count;
}

void SCR_Init( void )
{
}


/*
** SCR_GetBigStringWidth
*/ 
uint32_t SCR_GetBigStringWidth( const char *str ) {
	return SCR_Strlen( str ) * BIGCHAR_WIDTH;
}


uint64_t time_frontend, time_backend;

/*
==================
SCR_UpdateScreen

This is called every frame, and can also be called explicitly to flush
text to the screen.
==================
*/
void SCR_UpdateScreen( void )
{
    static uint32_t recursive;
    static uint64_t framecount;
    static int64_t next_frametime;
	extern cvar_t *ui_debugOverlay;

//	Assert( !g_pRenderThread->IsAlive() );
//	g_pRenderThread->Start();
	re.BeginFrame( STEREO_CENTER );

    if ( framecount == gi.framecount ) {
        int64_t ms = Sys_Milliseconds();

        if ( next_frametime && ms - next_frametime < 0 ) {
            re.ThrottleBackend();
        }
        else {
            next_frametime = ms + 16; // limit to 60 FPS
        }
    }
    else {
        next_frametime = 0;
        framecount = gi.framecount;
    }

    if ( ++recursive > 2 ) {
        N_Error( ERR_FATAL, "G_UpdateScreen: recursively called" );
    }
    recursive = 1;

	UI_Refresh( gi.realtime );
	// if there is no VM, there are also no rendering comamnds. Stop the renderer in
    // that case
	// we're in a level
	// if the user is ending a level through the pause menu,
	// we let the ui handle the sgame call
	if ( gi.mapLoaded && ( gi.state == GS_LEVEL || gi.state == GS_STATS_MENU ) && !( Key_GetCatcher() & KEYCATCH_CONSOLE ) ) {
		switch ( g_pModuleLib->ModuleCall( sgvm, ModuleOnRunTic, 1, gi.realFrameTime ) ) {
		case 0:
		default:
			break;
		case 1:
			g_pModuleLib->ModuleCall( sgvm, ModuleOnLevelEnd, 0 );
			g_pModuleLib->RunModules( ModuleOnLevelEnd, 0 );
			break;
		case 2:
			gi.state = GS_STATS_MENU;
			break; // its showing the stats window
		case 3:
			UI_ShowDemoMenu();

			Key_SetCatcher( KEYCATCH_UI );
			Cvar_Set( "g_paused", "1" );

			// we're only doing this for the demo
			Cbuf_ExecuteText( EXEC_APPEND, "setmap\n" );
			gi.state = GS_MENU;
			break;
		};
	}

    // console draws next
    Con_DrawConsole();

	if ( ui_debugOverlay->i ) {
		re.EndFrame( &time_frontend, &time_backend );
	} else {
		re.EndFrame( NULL, NULL );
	}

	// draw it all
//	if ( !g_pRenderThread->Join() ) {
//		N_Error( ERR_FATAL, "SCR_UpdateScreen: render thread join timed out, aborting process" );
//	}

    recursive = 0;
}
