#include "../engine/n_shared.h"
#include "../game/g_game.h"
#include "ui_public.hpp"
#include "ui_menu.h"
#include "ui_lib.h"

qboolean m_entersound;

void CUILib::Shutdown( void ) {
	curmenu = NULL;
	memset( stack, 0, sizeof(stack) );
	menusp = 0;
}

bool CUILib::Menu_Option( const char *label )
{
	bool retn;

    ImGui::TableNextColumn();
    ImGui::TextUnformatted( label );
    ImGui::TableNextColumn();
    ImGui::SameLine();
    retn = ImGui::ArrowButton( label, ImGuiDir_Right );

	if ( retn ) {
		Snd_PlaySfx( sfx_select );
	}

	return retn;
}

bool CUILib::Menu_Title( const char *label, float fontScale )
{
	refdef_t refdef;

	memset( &refdef, 0, sizeof( refdef ) );
	refdef.x = 0;
	refdef.y = 0;
	refdef.width = 1024;
	refdef.height = 768;
	refdef.flags = RSF_NOWORLDMODEL | RSF_ORTHO_TYPE_SCREENSPACE;

	//
	// draw the background
	//
	re.ClearScene();
	ui->DrawHandlePic( 0, 0, refdef.width, refdef.height, menu_background );
	re.RenderScene( &refdef );

    ImGui::SetWindowFontScale( 1.5f * scale );
	if ( state != STATE_MAIN ) {
	    if ( ImGui::ArrowButton( va( "##BACK%s", label ), ImGuiDir_Left ) ) {
			Snd_PlaySfx( sfx_back );
	        return true;
	    }
	    ImGui::SameLine();
	    ImGui::TextUnformatted( "BACK" );
	}

    ImGui::SetWindowFontScale( fontScale * scale );
    ImGui::TextUnformatted( label );
    ImGui::SetWindowFontScale( 1.0f * scale );

    return false;
}

void CUILib::EscapeMenuToggle( menustate_t newstate )
{
    if (Key_IsDown( KEY_ESCAPE )) {
        if (escapeToggle) {
            escapeToggle = qfalse;
            state = newstate;

			Snd_PlaySfx( sfx_back );
        }
    }
    else {
        escapeToggle = qtrue;
    }
}

/*
=================
UI_Init
=================
*/
void CUILib::Init( void )
{
	// cache redundant calulations
	re.GetConfig( &gpuConfig );

	// for 1024x768 virtualized screen
	scale = gpuConfig.vidHeight * (1.0/768.0);
	if ( gpuConfig.vidWidth * 768 > gpuConfig.vidHeight * 1024 ) {
		// wide screen
		bias = 0.5 * ( gpuConfig.vidWidth - ( gpuConfig.vidHeight * ( 1024.0 / 768.0 ) ) );
//		bias = r_customWidth->i / r_customHeight->i;
	}
	else {
		// no wide screen
		bias = 0;
	}

	curmenu = NULL;
	menusp = 0;
	
	escapeToggle = qfalse;
	sfx_scroll_toggle = qfalse;
	state = STATE_MAIN;

	sfx_select = Snd_RegisterSfx( "sfx/menu1.wav" );
    sfx_scroll = Snd_RegisterSfx( "sfx/menu2.wav" );
    sfx_back = Snd_RegisterSfx( "sfx/menu3.wav" );
    sfx_null = Snd_RegisterSfx( "sfx/menu4.wav" );
}

void CUILib::PushMenu( CUIMenu *menu )
{
	uint32_t i;

    // avoid stacking meuns invoked by hotkeys
    for (i = 0; i < menusp; i++) {
        if (stack[i] == menu) {
            menusp = i;
            break;
        }
    }

    if (i == menusp) {
        if (menusp >= MAX_MENU_DEPTH) {
            N_Error(ERR_DROP, "UI_PushMenu: menu stack overflow");
        }
        stack[menusp++] = menu;
    }

    curmenu = menu;

    // default cursor position
    menu->cursor = 0;
    menu->cursor_prev = 0;

    Key_SetCatcher(KEYCATCH_UI);

    // force first available item to have focus
    for (i = 0; i < menu->nitems; i++) {
        CUIMenuWidget *item = menu->Items()[i];
        if (!(item->flags & (QMF_GRAYED | QMF_MOUSEONLY | QMF_INACTIVE))) {
            menu->cursor_prev = -1;
            Menu_SetCursor(menu, i);
            break;
        }
    }

    firstdraw = qtrue;
}

void CUILib::PopMenu( void )
{
    menusp--;

    if (menusp < 0) {
        N_Error(ERR_DROP, "UI_PopMenu: menu stack underflow");
    }

    if (menusp) {
        curmenu = stack[menusp - 1];
        firstdraw = qtrue;
    }
    else {
        ForceMenuOff();
    }
}

void CUILib::ForceMenuOff( void )
{
    menusp = 0;
    curmenu = NULL;

    Key_SetCatcher( Key_GetCatcher() & ~KEYCATCH_UI );
    Key_ClearStates();
}

/*
=================
UI_LerpColor
=================
*/
void UI_LerpColor(vec4_t a, vec4_t b, vec4_t c, float t)
{
	int i;

	// lerp and clamp each component
	for (i=0; i<4; i++)
	{
		c[i] = a[i] + t*(b[i]-a[i]);
		if (c[i] < 0)
			c[i] = 0;
		else if (c[i] > 1.0)
			c[i] = 1.0;
	}
}

/*
=================
UI_DrawProportionalString2
=================
*/
static int	propMap[128][3] = {
{0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1},
{0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1},

{0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1},
{0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1},

{0, 0, PROP_SPACE_WIDTH},		// SPACE
{11, 122, 7},	// !
{154, 181, 14},	// "
{55, 122, 17},	// #
{79, 122, 18},	// $
{101, 122, 23},	// %
{153, 122, 18},	// &
{9, 93, 7},		// '
{207, 122, 8},	// (
{230, 122, 9},	// )
{177, 122, 18},	// *
{30, 152, 18},	// +
{85, 181, 7},	// ,
{34, 93, 11},	// -
{110, 181, 6},	// .
{130, 152, 14},	// /

{22, 64, 17},	// 0
{41, 64, 12},	// 1
{58, 64, 17},	// 2
{78, 64, 18},	// 3
{98, 64, 19},	// 4
{120, 64, 18},	// 5
{141, 64, 18},	// 6
{204, 64, 16},	// 7
{162, 64, 17},	// 8
{182, 64, 18},	// 9
{59, 181, 7},	// :
{35,181, 7},	// ;
{203, 152, 14},	// <
{56, 93, 14},	// =
{228, 152, 14},	// >
{177, 181, 18},	// ?

{28, 122, 22},	// @
{5, 4, 18},		// A
{27, 4, 18},	// B
{48, 4, 18},	// C
{69, 4, 17},	// D
{90, 4, 13},	// E
{106, 4, 13},	// F
{121, 4, 18},	// G
{143, 4, 17},	// H
{164, 4, 8},	// I
{175, 4, 16},	// J
{195, 4, 18},	// K
{216, 4, 12},	// L
{230, 4, 23},	// M
{6, 34, 18},	// N
{27, 34, 18},	// O

{48, 34, 18},	// P
{68, 34, 18},	// Q
{90, 34, 17},	// R
{110, 34, 18},	// S
{130, 34, 14},	// T
{146, 34, 18},	// U
{166, 34, 19},	// V
{185, 34, 29},	// W
{215, 34, 18},	// X
{234, 34, 18},	// Y
{5, 64, 14},	// Z
{60, 152, 7},	// [
{106, 151, 13},	// '\'
{83, 152, 7},	// ]
{128, 122, 17},	// ^
{4, 152, 21},	// _

{134, 181, 5},	// '
{5, 4, 18},		// A
{27, 4, 18},	// B
{48, 4, 18},	// C
{69, 4, 17},	// D
{90, 4, 13},	// E
{106, 4, 13},	// F
{121, 4, 18},	// G
{143, 4, 17},	// H
{164, 4, 8},	// I
{175, 4, 16},	// J
{195, 4, 18},	// K
{216, 4, 12},	// L
{230, 4, 23},	// M
{6, 34, 18},	// N
{27, 34, 18},	// O

{48, 34, 18},	// P
{68, 34, 18},	// Q
{90, 34, 17},	// R
{110, 34, 18},	// S
{130, 34, 14},	// T
{146, 34, 18},	// U
{166, 34, 19},	// V
{185, 34, 29},	// W
{215, 34, 18},	// X
{234, 34, 18},	// Y
{5, 64, 14},	// Z
{153, 152, 13},	// {
{11, 181, 5},	// |
{180, 152, 13},	// }
{79, 93, 17},	// ~
{0, 0, -1}		// DEL
};

static int propMapB[26][3] = {
{11, 12, 33},
{49, 12, 31},
{85, 12, 31},
{120, 12, 30},
{156, 12, 21},
{183, 12, 21},
{207, 12, 32},

{13, 55, 30},
{49, 55, 13},
{66, 55, 29},
{101, 55, 31},
{135, 55, 21},
{158, 55, 40},
{204, 55, 32},

{12, 97, 31},
{48, 97, 31},
{82, 97, 30},
{118, 97, 30},
{153, 97, 30},
{185, 97, 25},
{213, 97, 30},

{11, 139, 32},
{42, 139, 51},
{93, 139, 32},
{126, 139, 31},
{158, 139, 25},
};

#define PROPB_GAP_WIDTH		4
#define PROPB_SPACE_WIDTH	12
#define PROPB_HEIGHT		36

// bk001205 - code below duplicated in cgame/cg_drawtools.c
// bk001205 - FIXME: does this belong in ui_shared.c?
/*
=================
UI_DrawBannerString
=================
*/
void CUILib::DrawBannerString2( int x, int y, const char* str, vec4_t color ) const
{
	const char* s;
	unsigned char	ch; // bk001204 - unsigned
	float	ax;
	float	ay;
	float	aw;
	float	ah;
	float	frow;
	float	fcol;
	float	fwidth;
	float	fheight;

	// draw the colored text
	re.SetColor( color );
	
	ax = x * scale + bias;
	ay = y * scale;

	s = str;
	while ( *s )
	{
		ch = *s & 127;
		if ( ch == ' ' ) {
			ax += ((float)PROPB_SPACE_WIDTH + (float)PROPB_GAP_WIDTH)* scale;
		}
		else if ( ch >= 'A' && ch <= 'Z' ) {
			ch -= 'A';
			fcol = (float)propMapB[ch][0] / 256.0f;
			frow = (float)propMapB[ch][1] / 256.0f;
			fwidth = (float)propMapB[ch][2] / 256.0f;
			fheight = (float)PROPB_HEIGHT / 256.0f;
			aw = (float)propMapB[ch][2] * scale;
			ah = (float)PROPB_HEIGHT * scale;
//			re.DrawImage( ax, ay, aw, ah, fcol, frow, fcol+fwidth, frow+fheight, charsetPropB );
			ax += (aw + (float)PROPB_GAP_WIDTH * scale);
		}
		s++;
	}

	re.SetColor( NULL );
}

void CUILib::DrawBannerString( int x, int y, const char* str, int style, vec4_t color ) const
{
	const char *	s;
	int				ch;
	int				width;
	vec4_t			drawcolor;

	// find the width of the drawn text
	s = str;
	width = 0;
	while ( *s ) {
		ch = *s;
		if ( ch == ' ' ) {
			width += PROPB_SPACE_WIDTH;
		}
		else if ( ch >= 'A' && ch <= 'Z' ) {
			width += propMapB[ch - 'A'][2] + PROPB_GAP_WIDTH;
		}
		s++;
	}
	width -= PROPB_GAP_WIDTH;

	switch( style & UI_FORMATMASK ) {
	case UI_CENTER:
		x -= width / 2;
		break;
	case UI_RIGHT:
		x -= width;
		break;
	case UI_LEFT:
	default:
		break;
	};

	if ( style & UI_DROPSHADOW ) {
		drawcolor[0] = drawcolor[1] = drawcolor[2] = 0;
		drawcolor[3] = color[3];
		DrawBannerString2( x+2, y+2, str, drawcolor );
	}

	DrawBannerString2( x, y, str, color );
}

int CUILib::ProportionalStringWidth( const char* str ) const
{
	const char *	s;
	int				ch;
	int				charWidth;
	int				width;

	s = str;
	width = 0;
	while ( *s ) {
		ch = *s & 127;
		charWidth = propMap[ch][2];
		if ( charWidth != -1 ) {
			width += charWidth;
			width += PROP_GAP_WIDTH;
		}
		s++;
	}

	width -= PROP_GAP_WIDTH;
	return width;
}

void CUILib::DrawProportionalString2( int x, int y, const char* str, vec4_t color, float sizeScale, nhandle_t charset ) const
{
	const char* s;
	unsigned char	ch; // bk001204 - unsigned
	float	ax;
	float	ay;
	float	aw = 0; // bk001204 - init
	float	ah;
	float	frow;
	float	fcol;
	float	fwidth;
	float	fheight;

	// draw the colored text
	re.SetColor( color );
	
	ax = x * scale + bias;
	ay = y * scale;

	s = str;
	while ( *s ) {
		ch = *s & 127;
		if ( ch == ' ' ) {
			aw = (float)PROP_SPACE_WIDTH * scale * sizeScale;
		}
		else if ( propMap[ch][2] != -1 ) {
			fcol = (float)propMap[ch][0] / 256.0f;
			frow = (float)propMap[ch][1] / 256.0f;
			fwidth = (float)propMap[ch][2] / 256.0f;
			fheight = (float)PROP_HEIGHT / 256.0f;
			aw = (float)propMap[ch][2] * scale * sizeScale;
			ah = (float)PROP_HEIGHT * scale * sizeScale;
			re.DrawImage( ax, ay, aw, ah, fcol, frow, fcol+fwidth, frow+fheight, charset );
		}

		ax += (aw + (float)PROP_GAP_WIDTH * scale * sizeScale);
		s++;
	}

	re.SetColor( NULL );
}

/*
=================
UI_ProportionalSizeScale
=================
*/
float CUILib::ProportionalSizeScale( int style ) const
{
	if(  style & UI_SMALLFONT ) {
		return PROP_SMALL_SIZE_SCALE;
	}

	return 1.00;
}


/*
=================
UI_DrawProportionalString
=================
*/
void CUILib::DrawProportionalString( int x, int y, const char* str, int style, vec4_t color ) const
{
	vec4_t	drawcolor;
	int		width;
	float	sizeScale;

	sizeScale = ProportionalSizeScale( style );

	switch( style & UI_FORMATMASK ) {
		case UI_CENTER:
			width = ProportionalStringWidth( str ) * sizeScale;
			x -= width / 2;
			break;

		case UI_RIGHT:
			width = ProportionalStringWidth( str ) * sizeScale;
			x -= width;
			break;

		case UI_LEFT:
		default:
			break;
	}

	if ( style & UI_DROPSHADOW ) {
		drawcolor[0] = drawcolor[1] = drawcolor[2] = 0;
		drawcolor[3] = color[3];
//		DrawProportionalString2( x+2, y+2, str, drawcolor, sizeScale, charsetProp );
	}

	if ( style & UI_INVERSE ) {
		drawcolor[0] = color[0] * 0.7;
		drawcolor[1] = color[1] * 0.7;
		drawcolor[2] = color[2] * 0.7;
		drawcolor[3] = color[3];
//		DrawProportionalString2( x, y, str, drawcolor, sizeScale, charsetProp );
		return;
	}

	if ( style & UI_PULSE ) {
		drawcolor[0] = color[0] * 0.7;
		drawcolor[1] = color[1] * 0.7;
		drawcolor[2] = color[2] * 0.7;
		drawcolor[3] = color[3];
//		DrawProportionalString2( x, y, str, color, sizeScale, charsetProp );

		drawcolor[0] = color[0];
		drawcolor[1] = color[1];
		drawcolor[2] = color[2];
		drawcolor[3] = 0.5 + 0.5 * sin( realtime / PULSE_DIVISOR );
//		DrawProportionalString2( x, y, str, drawcolor, sizeScale, charsetPropGlow );
		return;
	}

//	DrawProportionalString2( x, y, str, color, sizeScale, charsetProp );
}

/*
=================
UI_DrawProportionalString_Wrapped
=================
*/
void CUILib::DrawProportionalString_AutoWrapped( int x, int y, int xmax, int ystep, const char* str, int style, vec4_t color ) const
{
	int width;
	char *s1,*s2,*s3;
	char c_bcp;
	char buf[1024];
	float   sizeScale;

	if (!str || str[0]=='\0') {
		return;
    }
	
	sizeScale = ProportionalSizeScale( style );
	
	N_strncpyz(buf, str, sizeof(buf));
	s1 = s2 = s3 = buf;

	while (1) {
		do {
			s3++;
		} while (*s3!=' ' && *s3!='\0');

		c_bcp = *s3;
		*s3 = '\0';
		width = ProportionalStringWidth(s1) * sizeScale;
		*s3 = c_bcp;
		
        if (width > xmax) {
			if (s1==s2) {
				// fuck, don't have a clean cut, we'll overflow
				s2 = s3;
			}

			*s2 = '\0';
			DrawProportionalString(x, y, s1, style, color);
			y += ystep;
			if (c_bcp == '\0') {
                // that was the last word
                // we could start a new loop, but that wouldn't be much use
                // even if the word is too long, we would overflow it (see above)
                // so just print it now if needed
                s2++;
                
                if (*s2 != '\0') { // if we are printing an overflowing line we have s2 == s3
                    DrawProportionalString(x, y, s2, style, color);
                }

			    break; 
            }
			s2++;
			s1 = s2;
			s3 = s2;
		}
		else {
			s2 = s3;
			if (c_bcp == '\0') { // we reached the end
				DrawProportionalString(x, y, s1, style, color);
				break;
			}
		}
	}
}

/*
=================
UI_DrawString2
=================
*/
void CUILib::DrawString2( int x, int y, const char* str, vec4_t color, int charw, int charh ) const
{
	const char* s;
	char	ch;
	int forceColor = qfalse; //APSFIXME;
	vec4_t	tempcolor;
	float	ax;
	float	ay;
	float	aw;
	float	ah;
	float	frow;
	float	fcol;

	if (y < -charh)
		// offscreen
		return;

	// draw the colored text
	re.SetColor( color );
	
	ax = x * scale + bias;
	ay = y * scale;
	aw = charw * scale;
	ah = charh * scale;

	s = str;
	while ( *s ) {
		if ( Q_IsColorString( s ) ) {
			if ( !forceColor ) {
				memcpy( tempcolor, g_color_table[ColorIndex(s[1])], sizeof( tempcolor ) );
				tempcolor[3] = color[3];
				re.SetColor( tempcolor );
			}
			s += 2;
			continue;
		}

		ch = *s & 255;
		if (ch != ' ') {
			frow = (ch>>4)*0.0625;
			fcol = (ch&15)*0.0625;
			re.DrawImage( ax, ay, aw, ah, fcol, frow, fcol + 0.0625, frow + 0.0625, charset );
		}

		ax += aw;
		s++;
	}

	re.SetColor( NULL );
}

void CUILib::DrawStringBlink( const char *str, int ticker, int mult) const
{
	if ((ticker % mult) != 0) {
		return;
	}

	DrawString( str );
}

void CUILib::DrawString( const char *str ) const
{
	char s[2];
    int currentColorIndex, colorIndex;
    uint32_t i, length;
    qboolean useColor = qfalse;

    // if it's got multiple lines, we need to
    // print with a drop-down
    if (strstr(str, "\\n") != NULL) {
    }

    length = (uint32_t)strlen(str);
    s[1] = 0;
    currentColorIndex = ColorIndex(S_COLOR_WHITE);

    for (i = 0; i < length; i++) {
        if (Q_IsColorString(str) && *(str+1) != '\n') {
            colorIndex = ColorIndexFromChar( *(str+1) );
            if (currentColorIndex != colorIndex) {
                currentColorIndex = colorIndex;
                if (useColor) {
                    ImGui::PopStyleColor();
                    useColor = qfalse;
                }
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4( g_color_table[ colorIndex ] ));
                useColor = qtrue;
            }
            i += 2;
        }

        s[0] = str[i];
        ImGui::TextUnformatted(s);
    }
}

/*
=================
UI_DrawString
=================
*/
void CUILib::DrawString( int x, int y, const char* str, int style, vec4_t color ) const
{
	int		len;
	int		charw;
	int		charh;
	vec4_t	newcolor;
	vec4_t	lowlight;
	float	*drawcolor;
	vec4_t	dropcolor;

	if( !str ) {
		return;
	}

	if ((style & UI_BLINK) && ((realtime/BLINK_DIVISOR) & 1)) {
		return;
    }

	if (style & UI_SMALLFONT) {
		charw =	SMALLCHAR_WIDTH;
		charh =	SMALLCHAR_HEIGHT;
	}
	else if (style & UI_GIANTFONT) {
		charw =	GIANTCHAR_WIDTH;
		charh =	GIANTCHAR_HEIGHT;
	}
	else {
		charw =	BIGCHAR_WIDTH;
		charh =	BIGCHAR_HEIGHT;
	}

	if (style & UI_PULSE) {
	    lowlight[0] = 0.8*color[0]; 
		lowlight[1] = 0.8*color[1];
		lowlight[2] = 0.8*color[2];
		lowlight[3] = 0.8*color[3];
		UI_LerpColor(color,lowlight,newcolor,0.5+0.5*sin(realtime/PULSE_DIVISOR));
		drawcolor = newcolor;
	}	
	else {
		drawcolor = color;
    }

	switch (style & UI_FORMATMASK) {
	case UI_CENTER:
		// center justify at x
		len = strlen(str);
		x   = x - len*charw/2;
		break;
	case UI_RIGHT:
		// right justify at x
		len = strlen(str);
		x   = x - len*charw;
		break;
    default:
    	// left justify at x
    	break;
	};

	if ( style & UI_DROPSHADOW ) {
		dropcolor[0] = dropcolor[1] = dropcolor[2] = 0;
		dropcolor[3] = drawcolor[3];
		DrawString2(x+2,y+2,str,dropcolor,charw,charh);
	}

	DrawString2(x,y,str,drawcolor,charw,charh);
}

/*
=================
UI_DrawChar
=================
*/
void CUILib::DrawChar( int x, int y, int ch, int style, vec4_t color ) const
{
	char	buff[2];

	buff[0] = ch;
	buff[1] = '\0';

	DrawString( x, y, buff, style, color );
}

qboolean CUILib::IsFullscreen( void ) const {
	if ( curmenu && ( Key_GetCatcher() & KEYCATCH_UI ) ) {
		return curmenu->fullscreen;
	}

	return qfalse;
}

void CUILib::SetActiveMenu( uiMenu_t menu )
{
	// this should be the ONLY way the menu system is brought up
	// enusure minumum menu data is cached
	Menu_Cache();

	switch ( menu ) {
	case UI_MENU_NONE:
		Key_SetCatcher( Key_GetCatcher() & ~KEYCATCH_UI );
		Key_ClearStates();
		Cvar_Set( "g_paused", "0" );
		ui->ForceMenuOff();
		break;
	case UI_MENU_PAUSE:
		Cvar_Set( "g_paused", "1" );
		Key_SetCatcher( KEYCATCH_UI );
		UI_PauseMenu();
		break;
	case UI_MENU_MAIN:
		UI_MainMenu();
		break;
	case UI_MENU_INTRO:
		UI_IntroMenu();
		break;
    case UI_MENU_TITLE:
        UI_TitleMenu();
		break;
	default:
#ifdef _NOMAD_DEBUG
	    Con_Printf("UI_SetActiveMenu: bad enum %lu\n", menu );
#endif
        break;
	};
}

/*
=================
UI_KeyEvent
=================
*/
void CUILib::KeyEvent( uint32_t key, qboolean down )
{
	sfxHandle_t		s;

	if (!curmenu) {
		return;
	}

	if (!down) {
		return;
	}

	if (curmenu->Key) {
		s = curmenu->Key( key );
	} else {
		s = Menu_DefaultKey( curmenu, key );
    }

	if ((s > 0) && (s != menu_null_sound)) {
		Snd_PlaySfx( s );
    }
}

/*
=================
UI_MouseEvent
=================
*/
void CUILib::MouseEvent( uint32_t dx, uint32_t dy )
{
	int				i;
	CUIMenuWidget*	m;

	if (!curmenu)
		return;

	// update mouse screen position
	cursorx += dx;
	if (cursorx < 0)
		cursorx = 0;
	else if (cursorx > SCREEN_WIDTH)
		cursorx = SCREEN_WIDTH;

	cursory += dy;
	if (cursory < 0)
		cursory = 0;
	else if (cursory > SCREEN_HEIGHT)
		cursory = SCREEN_HEIGHT;

	// region test the active menu items
	for (i=0; i<curmenu->nitems; i++)
	{
		m = (CUIMenuWidget*)curmenu->items[i];

		if (m->flags & (QMF_GRAYED|QMF_INACTIVE))
			continue;

		if ((cursorx < m->left) ||
			(cursorx > m->right) ||
			(cursory < m->top) ||
			(cursory > m->bottom))
		{
			// cursor out of item bounds
			continue;
		}

		// set focus to item at cursor
		if (curmenu->cursor != i)
		{
			Menu_SetCursor( curmenu, i );
			((CUIMenuWidget*)(curmenu->items[curmenu->cursor_prev]))->flags &= ~QMF_HASMOUSEFOCUS;

			if ( !(((CUIMenuWidget*)(curmenu->items[curmenu->cursor]))->flags & QMF_SILENT ) ) {
				Snd_PlaySfx( menu_move_sound );
			}
		}

		((CUIMenuWidget*)(curmenu->items[curmenu->cursor]))->flags |= QMF_HASMOUSEFOCUS;
		return;
	}  

	if (curmenu->nitems > 0) {
		// out of any region
		((CUIMenuWidget*)(curmenu->items[curmenu->cursor]))->flags &= ~QMF_HASMOUSEFOCUS;
	}
}

/*
================
UI_AdjustFrom1024

Adjusted for resolution and screen aspect ratio
================
*/
void CUILib::AdjustFrom1024( float *x, float *y, float *w, float *h ) const
{
	// expect valid pointers
	*x *= scale;
	*y *= scale;
	*w *= scale;
	*h *= scale;
}

void CUILib::DrawNamedPic( float x, float y, float width, float height, const char *picname ) const
{
	nhandle_t hShader;

	hShader = re.RegisterShader( picname );
	AdjustFrom1024( &x, &y, &width, &height );
	re.DrawImage( x, y, width, height, 0, 0, 1, 1, hShader );
}

void CUILib::DrawHandlePic( float x, float y, float w, float h, nhandle_t hShader ) const
{
	float	s0;
	float	s1;
	float	t0;
	float	t1;

	if( w < 0 ) {	// flip about vertical
		w  = -w;
		s0 = 1;
		s1 = 0;
	}
	else {
		s0 = 1;
		s1 = 0;
	}

	if( h < 0 ) {	// flip about horizontal
		h  = -h;
		t0 = 1;
		t1 = 0;
	}
	else {
		t0 = 0;
		t1 = 1;
	}
	
	AdjustFrom1024( &x, &y, &w, &h );
	re.DrawImage( x, y, w, h, s0, t0, s1, t1, hShader );
}

/*
================
UI_FillRect

Coordinates are 640*480 virtual values
=================
*/
void CUILib::FillRect( float x, float y, float width, float height, const float *color ) const
{
	re.SetColor( color );

	AdjustFrom1024( &x, &y, &width, &height );
	re.DrawImage( x, y, width, height, 0, 0, 0, 0, whiteShader );

	re.SetColor( NULL );
}

/*
================
UI_DrawRect

Coordinates are 640*480 virtual values
=================
*/
void CUILib::DrawRect( float x, float y, float width, float height, const float *color ) const
{
	re.SetColor( color );

	AdjustFrom1024( &x, &y, &width, &height );

	re.DrawImage( x, y, width, 1, 0, 0, 0, 0, whiteShader );
	re.DrawImage( x, y, 1, height, 0, 0, 0, 0, whiteShader );
	re.DrawImage( x, y + height - 1, width, 1, 0, 0, 0, 0, whiteShader );
	re.DrawImage( x + width - 1, y, 1, height, 0, 0, 0, 0, whiteShader );

	re.SetColor( NULL );
}

void CUILib::SetColor( const float *rgba ) const {
	re.SetColor( rgba );
}

void CUILib::DrawTextBox( int x, int y, int width, int lines ) const
{
	FillRect( x + BIGCHAR_WIDTH/2, y + BIGCHAR_HEIGHT/2, ( width + 1 ) * BIGCHAR_WIDTH, ( lines + 1 ) * BIGCHAR_HEIGHT, colorBlack );
	DrawRect( x + BIGCHAR_WIDTH/2, y + BIGCHAR_HEIGHT/2, ( width + 1 ) * BIGCHAR_WIDTH, ( lines + 1 ) * BIGCHAR_HEIGHT, colorWhite );
}

qboolean CUILib::CursorInRect( int x, int y, int width, int height ) const
{
	if (cursorx < x ||
		cursory < y ||
		cursorx > x+width ||
		cursory > y+height)
		return qfalse;

	return qtrue;
}

