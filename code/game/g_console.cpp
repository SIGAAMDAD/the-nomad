#include "../engine/n_shared.h"
#include "g_game.h"
#define STB_RECT_PACK_IMPLEMENTATION
#include "../rendercommon/imstb_rectpack.h"
#define STB_TRUETYPE_IMPLEMENTATION
#include "../rendercommon/imstb_truetype.h"
#include "../rendercommon/imgui.h"
#include "../rendercommon/r_public.h"


#define  DEFAULT_CONSOLE_WIDTH 78
#define  MAX_CONSOLE_WIDTH 120

#define  NUM_CON_TIMES  4

#define  CON_TEXTSIZE   65536

uint32_t bigchar_width;
uint32_t bigchar_height;
uint32_t smallchar_width;
uint32_t smallchar_height;

typedef struct {
	qboolean	initialized;

	int16_t text[CON_TEXTSIZE];
	uint32_t current;		// line where next message will be printed
	uint32_t x;				// offset in current line for next print
	uint32_t display;		// bottom of console displays this line

	uint32_t 	linewidth;		// characters across screen
	uint32_t 	totallines;		// total lines in console scrollback

	float	xadjust;		// for wide aspect screens

	float	displayFrac;	// aproaches finalFrac at scr_conspeed
	float	finalFrac;		// 0.0 to 1.0 lines of console to display

	uint32_t vislines;		// in scanlines

	uint32_t times[NUM_CON_TIMES];	// gi.realtime time the line was generated
								// for transparent notify lines
	vec4_t	color;

	uint32_t viswidth;
	uint32_t vispage;		

	qboolean newline;
} console_t;

console_t con;

file_t logfile = FS_INVALID_HANDLE;
field_t g_consoleField;
cvar_t *con_conspeed;
cvar_t *con_autoclear;
cvar_t *con_notifytime;
cvar_t *con_scale;
cvar_t *con_color;
cvar_t *con_noprint;
cvar_t *g_conXOffset;

uint32_t g_console_field_width;

/*
================
Con_ToggleConsole_f
================
*/
void Con_ToggleConsole_f( void ) {
	// Can't toggle the console when it's the only thing available
    if ( Key_GetCatcher() == KEYCATCH_CONSOLE ) {
		return;
	}

	if ( con_autoclear->i ) {
		Field_Clear( &g_consoleField );
	}

	g_consoleField.widthInChars = g_console_field_width;

	Con_ClearNotify();

	if (Key_GetCatcher() & KEYCATCH_CONSOLE) {
		Key_SetCatcher(Key_GetCatcher() & ~KEYCATCH_CONSOLE);
		Con_DPrintf("Toggle Console OFF\n");
	}
	else {
		Key_SetCatcher(Key_GetCatcher() | KEYCATCH_CONSOLE);
		Con_DPrintf("Toggle Console ON\n");
	}
}


/*
================
Con_Clear_f
================
*/
static void Con_Clear_f( void ) {
	uint32_t i;

	for ( i = 0 ; i < con.linewidth ; i++ ) {
		con.text[i] = ( ColorIndex( S_COLOR_WHITE ) << 8 ) | ' ';
	}

	con.x = 0;
	con.current = 0;
	con.newline = qtrue;

	Con_Bottom();		// go to end
}

						
/*
================
Con_Dump_f

Save the console contents out to a file
================
*/
static void Con_Dump_f( void )
{
	uint32_t l, x, i, n;
	int16_t	*line;
	file_t	f;
	uint32_t bufferlen;
	char	*buffer;
	char	filename[ MAX_OSPATH ];
	const char *ext;

	if ( Cmd_Argc() != 2 )
	{
		Con_Printf( "usage: condump <filename>\n" );
		return;
	}

	N_strncpyz( filename, Cmd_Argv( 1 ), sizeof( filename ) );
	COM_DefaultExtension( filename, sizeof( filename ), ".txt" );

	if ( !FS_AllowedExtension( filename, qfalse, &ext ) ) {
		Con_Printf( "%s: Invalid filename extension '%s'.\n", __func__, ext );
		return;
	}

	f = FS_FOpenWrite( filename );
	if ( f == FS_INVALID_HANDLE )
	{
		Con_Printf( "ERROR: couldn't open %s.\n", filename );
		return;
	}

	Con_Printf( "Dumped console text to %s.\n", filename );

	if ( con.current >= con.totallines ) {
		n = con.totallines;
		l = con.current + 1;
	} else {
		n = con.current + 1;
		l = 0;
	}

	bufferlen = con.linewidth + arraylen( GDR_NEWLINE ) * sizeof( char );
	buffer = (char *)Hunk_AllocateTempMemory( bufferlen );

	// write the remaining lines
	buffer[ bufferlen - 1 ] = '\0';

	for ( i = 0; i < n ; i++, l++ ) 
	{
		line = con.text + (l % con.totallines) * con.linewidth;
		// store line
		for( x = 0; x < con.linewidth; x++ )
			buffer[ x ] = line[ x ] & 0xff;
		buffer[ con.linewidth ] = '\0';
		// terminate on ending space characters
		for ( x = con.linewidth - 1 ; x >= 0 ; x-- ) {
			if ( buffer[ x ] == ' ' )
				buffer[ x ] = '\0';
			else
				break;
		}
		N_strcat( buffer, bufferlen, GDR_NEWLINE );
		FS_Write( buffer, strlen( buffer ), f );
	}

	Hunk_FreeTempMemory( buffer );
	FS_FClose( f );
}

						
/*
================
Con_ClearNotify
================
*/
void Con_ClearNotify( void ) {
	uint32_t i;
	
	for ( i = 0 ; i < NUM_CON_TIMES ; i++ ) {
		con.times[i] = 0;
	}
}


/*
================
Con_CheckResize

If the line width has changed, reformat the buffer.
================
*/
void Con_CheckResize( void )
{
	uint32_t i, j, width, oldwidth, oldtotallines, oldcurrent, numlines, numchars;
	int16_t	tbuf[CON_TEXTSIZE], *src, *dst;
	static uint32_t old_width, old_vispage;
	uint32_t vispage;
	float	scale;

	if ( con.viswidth == gi.gpuConfig.vidWidth && !con_scale->modified ) {
		return;
	}

	scale = con_scale->f;

	con.viswidth = gi.gpuConfig.vidWidth;

	smallchar_width = SMALLCHAR_WIDTH * scale * gi.con_factor;
	smallchar_height = SMALLCHAR_HEIGHT * scale * gi.con_factor;
	bigchar_width = BIGCHAR_WIDTH * scale * gi.con_factor;
	bigchar_height = BIGCHAR_HEIGHT * scale * gi.con_factor;

	if (!smallchar_width) {
		smallchar_width = SMALLCHAR_WIDTH;
	}
	if (!smallchar_height) {
		smallchar_height = SMALLCHAR_HEIGHT;
	}

	if ( gi.gpuConfig.vidWidth == 0 ) // video hasn't been initialized yet
	{
		g_console_field_width = DEFAULT_CONSOLE_WIDTH;
		width = DEFAULT_CONSOLE_WIDTH * scale;
		con.linewidth = width;
		con.totallines = CON_TEXTSIZE / con.linewidth;
		con.vispage = 4;

		Con_Clear_f();
	}
	else
	{
		width = ((gi.gpuConfig.vidWidth / smallchar_width) - 2);

		g_console_field_width = width;
		g_consoleField.widthInChars = g_console_field_width;

		if ( width > MAX_CONSOLE_WIDTH )
			width = MAX_CONSOLE_WIDTH;

		vispage = gi.gpuConfig.vidHeight / ( smallchar_height * 2 ) - 1;

		if ( old_vispage == vispage && old_width == width )
			return;

		oldwidth = con.linewidth;
		oldtotallines = con.totallines;
		oldcurrent = con.current;

		con.linewidth = width;
		con.totallines = CON_TEXTSIZE / con.linewidth;
		con.vispage = vispage;

		old_vispage = vispage;
		old_width = width;

		numchars = oldwidth;
		if ( numchars > con.linewidth )
			numchars = con.linewidth;

		if ( oldcurrent > oldtotallines )
			numlines = oldtotallines;	
		else
			numlines = oldcurrent + 1;	

		if ( numlines > con.totallines )
			numlines = con.totallines;

		memcpy( tbuf, con.text, CON_TEXTSIZE * sizeof( short ) );

		for ( i = 0; i < CON_TEXTSIZE; i++ ) 
			con.text[i] = (ColorIndex(S_COLOR_WHITE)<<8) | ' ';

		for ( i = 0; i < numlines; i++ )
		{
			src = &tbuf[ ((oldcurrent - i + oldtotallines) % oldtotallines) * oldwidth ];
			dst = &con.text[ (numlines - 1 - i) * con.linewidth ];
			for ( j = 0; j < numchars; j++ )
				*dst++ = *src++;
		}

		Con_ClearNotify();

		con.current = numlines - 1;
	}

	con.display = con.current;

	con_scale->modified = qfalse;
}


/*
==================
Cmd_CompleteTxtName
==================
*/
static void Cmd_CompleteTxtName(const char *args, uint32_t argNum ) {
	if ( argNum == 2 ) {
		Field_CompleteFilename( "", "txt", qfalse, FS_MATCH_EXTERN );
	}
}


/*
================
Con_Init
================
*/
void Con_Init( void ) 
{
	con_notifytime = Cvar_Get( "con_notifytime", "3", 0 );
	Cvar_SetDescription( con_notifytime, "Defines how long messages (from players or the system) are on the screen (in seconds)." );
	con_conspeed = Cvar_Get( "scr_conspeed", "3", 0 );
	Cvar_SetDescription( con_conspeed, "Console opening/closing scroll speed." );
	con_autoclear = Cvar_Get("con_autoclear", "1", CVAR_ARCHIVE_ND);
	Cvar_SetDescription( con_autoclear, "Enable/disable clearing console input text when console is closed." );
	con_scale = Cvar_Get( "con_scale", "1", CVAR_ARCHIVE_ND );
	Cvar_CheckRange( con_scale, "0.5", "8", CVT_FLOAT );
	Cvar_SetDescription( con_scale, "Console font size scale." );
	con_noprint = Cvar_Get( "con_noprint", "0", CVAR_LATCH );
	Cvar_CheckRange( con_noprint, "0", "1", CVT_INT );
	Cvar_SetDescription( con_noprint, "Toggles logging to ingame console." );

	g_conXOffset = Cvar_Get ("g_conXOffset", "0", 0);
	Cvar_SetDescription( g_conXOffset, "Console notifications X-offset." );
	con_color = Cvar_Get( "con_color", "", 0 );
	Cvar_SetDescription( con_color, "Console background color, set as R G B A values from 0-255, use with \\sets to save in config." );

	Field_Clear( &g_consoleField );
	g_consoleField.widthInChars = g_console_field_width;

	Cmd_AddCommand( "clear", Con_Clear_f );
	Cmd_AddCommand( "condump", Con_Dump_f );
	Cmd_SetCommandCompletionFunc( "condump", Cmd_CompleteTxtName );
	Cmd_AddCommand( "toggleconsole", Con_ToggleConsole_f );
}


/*
================
Con_Shutdown
================
*/
void Con_Shutdown( void )
{
	Cmd_RemoveCommand( "clear" );
	Cmd_RemoveCommand( "condump" );
	Cmd_RemoveCommand( "toggleconsole" );
}


/*
===============
Con_Fixup
===============
*/
static void Con_Fixup( void ) 
{
	uint32_t filled;

	if ( con.current >= con.totallines ) {
		filled = con.totallines;
	} else {
		filled = con.current + 1;
	}

	if ( filled <= con.vispage ) {
		con.display = con.current;
	} else if ( con.current - con.display > filled - con.vispage ) {
		con.display = con.current - filled + con.vispage;
	} else if ( con.display > con.current ) {
		con.display = con.current;
	}
}


/*
===============
Con_Linefeed

Move to newline only when we _really_ need this
===============
*/
static void Con_NewLine( void )
{
	int16_t *s;
	uint32_t i;

	// follow last line
	if ( con.display == con.current )
		con.display++;
	con.current++;

	s = &con.text[ ( con.current % con.totallines ) * con.linewidth ];
	for ( i = 0; i < con.linewidth ; i++ ) 
		*s++ = (ColorIndex(S_COLOR_WHITE)<<8) | ' ';

	con.x = 0;
}


/*
===============
Con_Linefeed
===============
*/
static void Con_Linefeed( qboolean skipnotify )
{
	// mark time for transparent overlay
	if ( con.current >= 0 )	{
		if ( skipnotify )
			con.times[ con.current % NUM_CON_TIMES ] = 0;
		else
			con.times[ con.current % NUM_CON_TIMES ] = gi.realtime;
	}

	if ( con.newline ) {
		Con_NewLine();
	} else {
		con.newline = qtrue;
		con.x = 0;
	}

	Con_Fixup();
}


/*
================
G_ConsolePrint

Handles cursor positioning, line wrapping, etc
All console printing must go through this in order to be logged to disk
If no console is visible, the text will appear at the top of the game window
================
*/
void G_ConsolePrint( const char *txt ) {
	uint32_t y;
	uint32_t c, l;
	uint32_t colorIndex;
	qboolean skipnotify = qfalse;		// NERVE - SMF
	uint32_t prev;							// NERVE - SMF

	// TTimo - prefix for text that shows up in console but not in notify
	// backported from RTCW
	if ( !N_strncmp( txt, "[skipnotify]", 12 ) ) {
		skipnotify = qtrue;
		txt += 12;
	}

	// for some demos we don't want to ever show anything on the console
	if ( con_noprint && con_noprint->i ) {
		return;
	}
	
	if ( !con.initialized ) {
		static cvar_t null_cvar = { 0 };
		con.color[0] =
		con.color[1] =
		con.color[2] =
		con.color[3] = 1.0f;
		con.viswidth = -9999;
		gi.con_factor = 1.0f;
		con_scale = &null_cvar;
		con_scale->f = 1.0f;
		con_scale->modified = qtrue;
		Con_CheckResize();
		con.initialized = qtrue;
	}

	colorIndex = ColorIndex( S_COLOR_WHITE );

	while ( (c = *txt) != 0 ) {
		if ( Q_IsColorString( txt ) && *(txt+1) != '\n' ) {
			colorIndex = ColorIndexFromChar( *(txt+1) );
			txt += 2;
			continue;
		}

		// count word length
		for ( l = 0 ; l < con.linewidth ; l++ ) {
			if ( txt[l] <= ' ' ) {
				break;
			}
		}

		// word wrap
		if ( l != con.linewidth && ( con.x + l >= con.linewidth ) ) {
			Con_Linefeed( skipnotify );
		}

		txt++;

		switch( c )
		{
		case '\n':
			Con_Linefeed( skipnotify );
			break;
		case '\r':
			con.x = 0;
			break;
		default:
			if ( con.newline ) {
				Con_NewLine();
				Con_Fixup();
				con.newline = qfalse;
			}
			// display character and advance
			y = con.current % con.totallines;
			con.text[y * con.linewidth + con.x ] = (colorIndex << 8) | (c & 255);
			con.x++;
			if ( con.x >= con.linewidth ) {
				Con_Linefeed( skipnotify );
			}
			break;
		}
	}

	// mark time for transparent overlay
	if ( con.current >= 0 ) {
		if ( skipnotify ) {
			prev = con.current % NUM_CON_TIMES - 1;
			if ( prev < 0 )
				prev = NUM_CON_TIMES - 1;
			con.times[ prev ] = 0;
		} else {
			con.times[ con.current % NUM_CON_TIMES ] = gi.realtime;
		}
	}
}


/*
==============================================================================

DRAWING

==============================================================================
*/


/*
================
Con_DrawInput

Draw the editline after a ] prompt
================
*/
static void Con_DrawInput( void ) {
	uint32_t y;
	char str[2];

	if ( !(Key_GetCatcher( ) & KEYCATCH_CONSOLE ) ) {
		return;
	}

	y = con.vislines - ( smallchar_height * 2 );
	str[0] = ']';
	str[1] = 0;

	ImGui::PushStyleColor( ImGuiCol_Text, ImVec4( con.color ) );
	ImGui::TextUnformatted( str, str + 1 );
	ImGui::PopStyleColor();
	ImGui::SameLine();

	Field_Draw( &g_consoleField, con.xadjust + 2 * smallchar_width, y,
		SCREEN_WIDTH - 3 * smallchar_width, qtrue, qtrue );
}


/*
================
Con_DrawNotify

Draws the last few lines of output transparently over the game top
================
*/
static void Con_DrawNotify( void )
{
	// FIXME: implement
#if 0
	uint32_t x, v;
	int16_t	*text;
	uint32_t i;
	uint32_t time;
	uint32_t skip;
	uint32_t currentColorIndex;
	uint32_t colorIndex;

	currentColorIndex = ColorIndex( S_COLOR_WHITE );

	ImGui::PushStyleColor(ImGuiCol_Text, ImVec4( g_color_table[ currentColorIndex ] ));

	v = 0;

	for (i = con.current - NUM_CON_TIMES + 1; i <= con.current; i++) {
		if (i < 0) {
			continue;
		}
		time = con.times[i % NUM_CON_TIMES];
		if (time == 0) {
			continue;
		}
		time = gi.realtime - time;
		if ( time >= con_notifytime->f*1000 ) {
			continue;
		}
		text = con.text + (i % con.totallines)*con.linewidth;

		if ( Key_GetCatcher( ) & (KEYCATCH_UI | KEYCATCH_SGAME) ) {
			continue;
		}

		for (x = 0 ; x < con.linewidth ; x++) {
			if ( ( text[x] & 0xff ) == ' ' ) {
				continue;
			}
			colorIndex = ( text[x] >> 8 ) & 63;
			if ( currentColorIndex != colorIndex ) {
				currentColorIndex = colorIndex;
				ImGui::PopStyleColor();
				ImGui::PushStyleColor( ImGuiCol_Text, ImVec4( g_color_table[ colorIndex ] ) );
			}
			SCR_DrawSmallChar( g_conXOffset->i + con.xadjust + (x+1)*smallchar_width, v, text[x] & 0xff );
		}

		v += smallchar_height;
	}

	re.SetColor( NULL );

	if ( Key_GetCatcher() & (KEYCATCH_UI | KEYCATCH_SGAME) ) {
		return;
	}
#endif
}


/*
================
Con_DrawSolidConsole

Draws the console with the solid background
================
*/
static void Con_DrawSolidConsole( float frac ) {
#if 0
	static float conColorValue[4] = { 0.0, 0.0, 0.0, 0.0 };
	// for cvar value change tracking
	static char  conColorString[ MAX_CVAR_VALUE ] = { '\0' };

	uint32_t		i, x, y;
	uint32_t		rows;
	int16_t			*text;
	uint32_t		row;
	uint32_t		lines;
	uint32_t		currentColorIndex;
	uint32_t		colorIndex;
	float			yf, wf;
	char			buf[ MAX_CVAR_VALUE ], *v[4];

	lines = gi.gpuConfig.vidHeight * frac;
	if ( lines <= 0 )
		return;

	if ( re.FinishBloom )
		re.FinishBloom();

	if ( lines > gi.gpuConfig.vidHeight )
		lines = gi.gpuConfig.vidHeight;

	wf = SCREEN_WIDTH;

	// draw the background
	yf = frac * SCREEN_HEIGHT;

	// on wide screens, we will center the text
	con.xadjust = 0;
	SCR_AdjustFrom640( &con.xadjust, &yf, &wf, NULL );

	if ( yf < 1.0 ) {
		yf = 0;
	} else {
		// custom console background color
		if ( con_color->s[0] ) {
			// track changes
			if ( strcmp( con_color->s, conColorString ) )  {
				N_strncpyz( conColorString, con_color->s, sizeof( conColorString ) );
				N_strncpyz( buf, con_color->s, sizeof( buf ) );
				Com_Split( buf, v, 4, ' ' );
				for ( i = 0; i < 4 ; i++ ) {
					conColorValue[ i ] = N_atof( v[ i ] ) / 255.0f;
					if ( conColorValue[ i ] > 1.0f ) {
						conColorValue[ i ] = 1.0f;
					} else if ( conColorValue[ i ] < 0.0f ) {
						conColorValue[ i ] = 0.0f;
					}
				}
			}
			ImGui::PopStyleColor();
			ImGui::PushStyleColor( ImGuiCol_Text, ImVec4( conColorValue ) );
			re.DrawImage( 0, 0, wf, yf, 0, 0, 1, 1, gi.whiteShader );
		} else {
			re.SetColor( g_color_table[ ColorIndex( S_COLOR_WHITE ) ] );
			re.DrawImage( 0, 0, wf, yf, 0, 0, 1, 1, gi.consoleShader );
		}

	}

	re.SetColor( g_color_table[ ColorIndex( S_COLOR_RED ) ] );
	re.DrawImage( 0, yf, wf, 2, 0, 0, 1, 1, gi.whiteShader );

	//y = yf;

	// draw the version number
	SCR_DrawSmallString( gi.gpuConfig.vidWidth - ( arraylen( GLN_VERSION ) ) * smallchar_width,
		lines - smallchar_height, GLN_VERSION, arraylen( GLN_VERSION ) - 1 );

	// draw the text
	con.vislines = lines;
	rows = lines / smallchar_width - 1;	// rows of text to draw

	y = lines - (smallchar_height * 3);

	row = con.display;

	// draw from the bottom up
	if ( con.display != con.current )
	{
		// draw arrows to show the buffer is backscrolled
		re.SetColor( g_color_table[ ColorIndex( S_COLOR_RED ) ] );
		for ( x = 0 ; x < con.linewidth ; x += 4 )
			SCR_DrawSmallChar( con.xadjust + (x+1)*smallchar_width, y, '^' );
		y -= smallchar_height;
		row--;
	}

	currentColorIndex = ColorIndex( S_COLOR_WHITE );
	re.SetColor( g_color_table[ currentColorIndex ] );

	for ( i = 0 ; i < rows ; i++, y -= smallchar_height, row-- )
	{
		if ( row < 0 )
			break;

		if ( con.current - row >= con.totallines ) {
			// past scrollback wrap point
			continue;
		}

		text = con.text + (row % con.totallines) * con.linewidth;

		for ( x = 0 ; x < con.linewidth ; x++ ) {
			// skip rendering whitespace
			if ( ( text[x] & 0xff ) == ' ' ) {
				continue;
			}
			// track color changes
			colorIndex = ( text[ x ] >> 8 ) & 63;
			if ( currentColorIndex != colorIndex ) {
				currentColorIndex = colorIndex;
				re.SetColor( g_color_table[ colorIndex ] );
			}
			SCR_DrawSmallChar( con.xadjust + (x + 1) * smallchar_width, y, text[x] & 0xff );
		}
	}
#endif
	static float conColorValue[4] = { 0.0, 0.0, 0.0, 0.0 };
	// for cvar value change tracking
	static char  conColorString[ MAX_CVAR_VALUE ] = { '\0' };
	int currentColorIndex, colorIndex;
	int i, x;
	int16_t *text;
	int lines, row;
	char str[2];
	char buf[ MAX_CVAR_VALUE ], *v[4];

	ImGui::Begin("Command Console", NULL, ImGuiWindowFlags_None);

	lines = gi.gpuConfig.vidHeight * frac;
	if ( lines <= 0 )
		return;

	if ( re.FinishBloom )
		re.FinishBloom();

	if ( lines > gi.gpuConfig.vidHeight )
		lines = gi.gpuConfig.vidHeight;
	
	// custom console background color
	if ( con_color->s[0] ) {
		// track changes
		if ( strcmp( con_color->s, conColorString ) )  {
			N_strncpyz( conColorString, con_color->s, sizeof( conColorString ) );
			N_strncpyz( buf, con_color->s, sizeof( buf ) );
			Com_Split( buf, v, 4, ' ' );
			for ( i = 0; i < 4 ; i++ ) {
				conColorValue[ i ] = N_atof( v[ i ] ) / 255.0f;
				if ( conColorValue[ i ] > 1.0f ) {
					conColorValue[ i ] = 1.0f;
				} else if ( conColorValue[ i ] < 0.0f ) {
					conColorValue[ i ] = 0.0f;
				}
			}
		}
		ImGui::PushStyleColor( ImGuiCol_WindowBg, ImVec4( conColorValue ) );
		ImGui::Image( re.GetTexDateFromShader(gi.whiteShader), ImGui::GetWindowSize() );
	} else {
		ImGui::PushStyleColor( ImGuiCol_WindowBg, ImVec4( g_color_table[ ColorIndex( S_COLOR_WHITE ) ] ) );
		ImGui::Image( re.GetTexDateFromShader(gi.consoleShader), ImGui::GetWindowSize() );
	}

	// draw the version number
	ImGui::TextColored(ImVec4( g_color_table[ ColorIndex(S_COLOR_RED) ] ), "%s", GLN_VERSION);

	// draw the text
	con.vislines = lines;
	row = con.display;

	// draw from the bottom up
	if (con.display != con.current) {
		// drwa arrows to show the buffer is backscrolled
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4( g_color_table[ ColorIndex(S_COLOR_RED) ] ));
		ImGui::Text("%80c", '^');
		ImGui::PopStyleColor();
		row--;
	}

	for (i = 0; i < lines; i++, row--) {
		if (row < 0) {
			break;
		}

		if (con.current - row >= con.totallines) {
			// past scrollback wrap point
			continue;
		}

		text = con.text + (row % con.totallines) * con.linewidth;
		for (x = 0; x < con.linewidth; x++) {
			// skip rendering whitespace
			if ((text[x] & 0xff) == ' ') {
				continue;
			}
			// track color changes
			colorIndex = (text[x] >> 8) & 63;
			if (currentColorIndex != colorIndex) {
				currentColorIndex = colorIndex;
				ImGui::PopStyleColor();
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4( g_color_table[ colorIndex ] ));
			}
			str[0] = text[x] & 0xff;
			str[1] = 0;
			ImGui::TextUnformatted(str, str + 1);
			ImGui::SameLine();
		}
	}

	// draw the input prompt, user text, and cursor if desired
	Con_DrawInput();

	ImGui::PopStyleColor();
	ImGui::End();
//	re.SetColor( NULL );
}


/*
==================
Con_DrawConsole
==================
*/
void Con_DrawConsole( void ) {

	// check for console width changes from a vid mode change
	Con_CheckResize();

	if ( con.displayFrac ) {
		Con_DrawSolidConsole( con.displayFrac );
	} else {
		// draw notify lines
		if ( gi.state == GS_LEVEL ) {
			Con_DrawNotify();
		}
	}
}

//================================================================

/*
==================
Con_RunConsole

Scroll it up or down
==================
*/
void Con_RunConsole( void ) 
{
	// decide on the destination height of the console
	if ( Key_GetCatcher( ) & KEYCATCH_CONSOLE )
		con.finalFrac = 0.5;	// half screen
	else
		con.finalFrac = 0.0;	// none visible
	
	// scroll towards the destination height
	if ( con.finalFrac < con.displayFrac )
	{
		con.displayFrac -= con_conspeed->f * gi.realtime * 0.001;
		if ( con.finalFrac > con.displayFrac )
			con.displayFrac = con.finalFrac;

	}
	else if ( con.finalFrac > con.displayFrac )
	{
		con.displayFrac += con_conspeed->f * gi.realtime * 0.001;
		if ( con.finalFrac < con.displayFrac )
			con.displayFrac = con.finalFrac;
	}
}


void Con_PageUp( uint32_t lines )
{
	if ( lines == 0 )
		lines = con.vispage - 2;

	con.display -= lines;
	
	Con_Fixup();
}


void Con_PageDown( uint32_t lines )
{
	if ( lines == 0 )
		lines = con.vispage - 2;

	con.display += lines;

	Con_Fixup();
}


void Con_Top( void )
{
	// this is generally incorrect but will be adjusted in Con_Fixup()
	con.display = con.current - con.totallines;

	Con_Fixup();
}


void Con_Bottom( void )
{
	con.display = con.current;

	Con_Fixup();
}


void Con_Close( void )
{
//	if ( !com_cl_running->i )
//		return;

	Field_Clear( &g_consoleField );
	Con_ClearNotify();
	Key_SetCatcher( Key_GetCatcher( ) & ~KEYCATCH_CONSOLE );
	con.finalFrac = 0.0;			// none visible
	con.displayFrac = 0.0;
}