#include "../game/g_game.h"
#include "ui_public.hpp"
#include "ui_menu.h"
#include "ui_lib.h"

CUIMenu::CUIMenu( void )
{
    cursor = 0;
    cursor_prev = 0;
    wrapAround = qfalse;
    fullscreen = qfalse;
    Key = NULL;
    Draw = NULL;

    nitems = 0;
    memset(name, 0, sizeof(name));
    memset(items, 0, sizeof(items));
}

CUIMenu::~CUIMenu() {
}

CUIMenuWidget::CUIMenuWidget( void )
{
	memset(name, 0, sizeof(name));

	EventCallback = NULL;
	StatusBar = NULL;
	OwnerDraw = NULL;

	type = 0;
	id = 0;
	x = 0;
	y = 0;
	left = 0;
	top = 0;
	right = 0;
	bottom = 0;
	parent = NULL;
	menuPosition = 0;
	flags = 0;
}

CUIMenuWidget::~CUIMenuWidget() {
}

CUIMenuWidget **CUIMenu::Items( void ) {
    return (CUIMenuWidget **)items;
}

const CUIMenuWidget **CUIMenu::Items( void ) const {
    return (const CUIMenuWidget **)items;
}



sfxHandle_t menu_in_sound;
sfxHandle_t menu_move_sound;
sfxHandle_t menu_out_sound;
sfxHandle_t menu_buzz_sound;
sfxHandle_t menu_null_sound;
sfxHandle_t weaponChangeSound;

static nhandle_t	sliderBar;
static nhandle_t	sliderButton_0;
static nhandle_t	sliderButton_1;

vec4_t menu_text_color	    = {1.0f, 1.0f, 1.0f, 1.0f};
vec4_t menu_dim_color       = {0.0f, 0.0f, 0.0f, 0.75f};
vec4_t color_black	    = {0.00f, 0.00f, 0.00f, 1.00f};
vec4_t color_white	    = {1.00f, 1.00f, 1.00f, 1.00f};
vec4_t color_yellow	    = {1.00f, 1.00f, 0.00f, 1.00f};
vec4_t color_blue	    = {0.00f, 0.00f, 1.00f, 1.00f};
vec4_t color_lightOrange    = {1.00f, 0.68f, 0.00f, 1.00f };
vec4_t color_orange	    = {1.00f, 0.43f, 0.00f, 1.00f};
vec4_t color_red	    = {1.00f, 0.00f, 0.00f, 1.00f};
vec4_t color_dim	    = {0.00f, 0.00f, 0.00f, 0.25f};

// current color scheme
vec4_t pulse_color          = {1.00f, 1.00f, 1.00f, 1.00f};
vec4_t text_color_disabled  = {0.50f, 0.50f, 0.50f, 1.00f};	// light gray
vec4_t text_color_normal    = {1.00f, 0.43f, 0.00f, 1.00f};	// light orange
vec4_t text_color_highlight = {1.00f, 1.00f, 0.00f, 1.00f};	// bright yellow
vec4_t listbar_color        = {1.00f, 0.43f, 0.00f, 0.30f};	// transluscent orange
vec4_t text_color_status    = {1.00f, 1.00f, 1.00f, 1.00f};	// bright white	

// action widget
static void	Action_Init( maction_t *a );
static void	Action_Draw( maction_t *a );

// radio button widget
static void	RadioButton_Init( mradiobutton_t *rb );
static void	RadioButton_Draw( mradiobutton_t *rb );
static sfxHandle_t RadioButton_Key( mradiobutton_t *rb, uint32_t key );

// slider widget
static void Slider_Init( mslider_t *s );
static sfxHandle_t Slider_Key( mslider_t *s, uint32_t key );
static void	Slider_Draw( mslider_t *s );

// spin control widget
static void	SpinControl_Init( mlist_t *s );
static void	SpinControl_Draw( mlist_t *s );
static sfxHandle_t SpinControl_Key( mlist_t *l, uint32_t key );

// text widget
static void Text_Init( mtext_t *b );
static void Text_Draw( mtext_t *b );

// scrolllist widget
static void	ScrollList_Init( mlist_t *l );
sfxHandle_t ScrollList_Key( mlist_t *l, uint32_t key );

// proportional text widget
static void PText_Init( mtext_t *b );
static void PText_Draw( mtext_t *b );

// proportional banner text widget
static void BText_Init( mtext_t *b );
static void BText_Draw( mtext_t *b );

/*
=================
Text_Init
=================
*/
static void Text_Init( mtext_t *t )
{
	t->generic.flags |= QMF_INACTIVE;
}

/*
=================
Text_Draw
=================
*/
static void Text_Draw( mtext_t *t )
{
	int		x;
	int		y;
	char	buff[512];	
	float*	color;

	x = t->generic.x;
	y = t->generic.y;

	buff[0] = '\0';

	// possible label
	if (t->generic.name)
		strcpy(buff,t->generic.name);

	// possible value
	if (t->string)
		strcat(buff,t->string);
		
	if (t->generic.flags & QMF_GRAYED)
		color = text_color_disabled;
	else
		color = t->color;

	ui->DrawString( x, y, buff, t->style, color );
}

/*
=================
BText_Init
=================
*/
static void BText_Init( mtext_t *t )
{
	t->generic.flags |= QMF_INACTIVE;
}

/*
=================
BText_Draw
=================
*/
static void BText_Draw( mtext_t *t )
{
	int		x;
	int		y;
	float*	color;

	x = t->generic.x;
	y = t->generic.y;

	if (t->generic.flags & QMF_GRAYED)
		color = text_color_disabled;
	else
		color = t->color;

    ui->DrawBannerString( x, y, t->string, t->style, color );
}

/*
=================
PText_Init
=================
*/
static void PText_Init( mtext_t *t )
{
	int	x;
	int	y;
	int	w;
	int	h;
	float	sizeScale;

	sizeScale = ui->ProportionalSizeScale( t->style );

	x = t->generic.x;
	y = t->generic.y;
	w = ui->ProportionalStringWidth( t->string ) * sizeScale;
	h =	PROP_HEIGHT * sizeScale;

	if( t->generic.flags & QMF_RIGHT_JUSTIFY ) {
		x -= w;
	}
	else if( t->generic.flags & QMF_CENTER_JUSTIFY ) {
		x -= w / 2;
	}

	t->generic.left   = x - PROP_GAP_WIDTH * sizeScale;
	t->generic.right  = x + w + PROP_GAP_WIDTH * sizeScale;
	t->generic.top    = y;
	t->generic.bottom = y + h;
}

/*
=================
PText_Draw
=================
*/
static void PText_Draw( mtext_t *t )
{
	int		x;
	int		y;
	float *	color;
	int		style;

	x = t->generic.x;
	y = t->generic.y;

	if (t->generic.flags & QMF_GRAYED)
		color = text_color_disabled;
	else
		color = t->color;

	style = t->style;
	if( t->generic.flags & QMF_PULSEIFFOCUS ) {
		if( Menu_ItemAtCursor( t->generic.parent ) == t ) {
			style |= UI_PULSE;
		}
		else {
			style |= UI_INVERSE;
		}
	}

	ui->DrawProportionalString( x, y, t->string, style, color );
}

/*
=================
Bitmap_Init
=================
*/
void Bitmap_Init( mbitmap_t *b )
{
	int	x;
	int	y;
	int	w;
	int	h;

	x = b->generic.x;
	y = b->generic.y;
	w = b->width;
	h =	b->height;
	if( w < 0 ) {
		w = -w;
	}
	if( h < 0 ) {
		h = -h;
	}

	if (b->generic.flags & QMF_RIGHT_JUSTIFY)
	{
		x = x - w;
	}
	else if (b->generic.flags & QMF_CENTER_JUSTIFY)
	{
		x = x - w/2;
	}

	b->generic.left   = x;
	b->generic.right  = x + w;
	b->generic.top    = y;
	b->generic.bottom = y + h;

	b->shader      = 0;
	b->focusshader = 0;
}

/*
=================
Bitmap_Draw
=================
*/
void Bitmap_Draw( mbitmap_t *b )
{
	float	x;
	float	y;
	float	w;
	float	h;
	vec4_t	tempcolor;
	float*	color;

	x = b->generic.x;
	y = b->generic.y;
	w = b->width;
	h =	b->height;

	if (b->generic.flags & QMF_RIGHT_JUSTIFY)
	{
		x = x - w;
	}
	else if (b->generic.flags & QMF_CENTER_JUSTIFY)
	{
		x = x - w/2;
	}

	// used to refresh shader
	if (b->generic.name && !b->shader)
	{
		b->shader = re.RegisterShader( b->generic.name );
		if (!b->shader && b->errorpic)
			b->shader = re.RegisterShader( b->errorpic );
	}

	if (b->focuspic && !b->focusshader)
		b->focusshader = re.RegisterShader( b->focuspic );

	if (b->generic.flags & QMF_GRAYED)
	{
		if (b->shader)
		{
			ui->SetColor( colorMdGrey );
			ui->DrawHandlePic( x, y, w, h, b->shader );
			ui->SetColor( NULL );
		}
	}
	else
	{
		if (b->shader)
			ui->DrawHandlePic( x, y, w, h, b->shader );

		// bk001204 - parentheses
		if (  ( (b->generic.flags & QMF_PULSE) 
			|| (b->generic.flags & QMF_PULSEIFFOCUS) )
		      && (Menu_ItemAtCursor( b->generic.parent ) == b))
		{	
			if (b->focuscolor)			
			{
				tempcolor[0] = b->focuscolor[0];
				tempcolor[1] = b->focuscolor[1];
				tempcolor[2] = b->focuscolor[2];
				color        = tempcolor;	
			}
			else
				color = pulse_color;
			color[3] = 0.5+0.5*sin(ui->GetRealTime()/PULSE_DIVISOR);

			ImGui::PushStyleColor( ImGuiCol_Text, ImVec4( color ) );
			ImGui::Image( re.ImGui_TextureData( b->focusshader ), ImVec2( w, h ) );
			ImGui::PopStyleColor();

			ui->SetColor( color );
			ui->DrawHandlePic( x, y, w, h, b->focusshader );
			ui->SetColor( NULL );
		}
		else if ((b->generic.flags & QMF_HIGHLIGHT) || ((b->generic.flags & QMF_HIGHLIGHT_IF_FOCUS) && (Menu_ItemAtCursor( b->generic.parent ) == b)))
		{	
			if (b->focuscolor)
			{
				ui->SetColor( b->focuscolor );
				ui->DrawHandlePic( x, y, w, h, b->focusshader );
				ui->SetColor( NULL );
			}
			else
				ui->DrawHandlePic( x, y, w, h, b->focusshader );
		}
	}
}

/*
=================
Action_Init
=================
*/
static void Action_Init( maction_t *a )
{
	int	len;

	// calculate bounds
	if (a->generic.name)
		len = strlen(a->generic.name);
	else
		len = 0;

	// left justify text
	a->generic.left   = a->generic.x; 
	a->generic.right  = a->generic.x + len*BIGCHAR_WIDTH;
	a->generic.top    = a->generic.y;
	a->generic.bottom = a->generic.y + BIGCHAR_HEIGHT;
}

/*
=================
Action_Draw
=================
*/
static void Action_Draw( maction_t *a )
{
	int		x, y;
	int		style;
	float*	color;

	style = 0;
	color = menu_text_color;
	if ( a->generic.flags & QMF_GRAYED )
	{
		color = text_color_disabled;
	}
	else if (( a->generic.flags & QMF_PULSEIFFOCUS ) && ( a->generic.parent->cursor == a->generic.menuPosition ))
	{
		color = text_color_highlight;
		style = UI_PULSE;
	}
	else if (( a->generic.flags & QMF_HIGHLIGHT_IF_FOCUS ) && ( a->generic.parent->cursor == a->generic.menuPosition ))
	{
		color = text_color_highlight;
	}
	else if ( a->generic.flags & QMF_BLINK )
	{
		style = UI_BLINK;
		color = text_color_highlight;
	}

	x = a->generic.x;
	y = a->generic.y;

	ui->DrawString( x, y, a->generic.name, UI_LEFT|style, color );

	if ( a->generic.parent->cursor == a->generic.menuPosition )
	{
		// draw cursor
		ui->DrawChar( x - BIGCHAR_WIDTH, y, 13, UI_LEFT|UI_BLINK, color);
	}
}

/*
=================
RadioButton_Init
=================
*/
static void RadioButton_Init( mradiobutton_t *rb )
{
	int	len;

	// calculate bounds
	if (rb->generic.name) {
		len = strlen(rb->generic.name);
    }
	else {
		len = 0;
    }

	rb->generic.left   = rb->generic.x - (len+1)*SMALLCHAR_WIDTH;
	rb->generic.right  = rb->generic.x + 6*SMALLCHAR_WIDTH;
	rb->generic.top    = rb->generic.y;
	rb->generic.bottom = rb->generic.y + SMALLCHAR_HEIGHT;
}

/*
=================
RadioButton_Key
=================
*/
static sfxHandle_t RadioButton_Key( mradiobutton_t *rb, uint32_t key )
{
	switch (key) {
	case KEY_MOUSE_LEFT:
		if (!(rb->generic.flags & QMF_HASMOUSEFOCUS)) {
			break;
        }
	case KEY_ENTER:
	case KEY_KP_ENTER:
	case KEY_LEFT:
	case KEY_RIGHT:
		rb->curvalue = !rb->curvalue;
		if ( rb->generic.EventCallback ) {
			rb->generic.EventCallback( rb, EVENT_ACTIVATED );
        }

		return (menu_move_sound);
	};

	// key not handled
	return 0;
}

/*
=================
RadioButton_Draw
=================
*/
static void RadioButton_Draw( mradiobutton_t *rb )
{
	int	x;
	int y;
	float *color;
	int	style;
	qboolean focus;

	x = rb->generic.x;
	y = rb->generic.y;

	focus = (rb->generic.parent->cursor == rb->generic.menuPosition);

	if ( rb->generic.flags & QMF_GRAYED )
	{
		color = text_color_disabled;
		style = UI_LEFT|UI_SMALLFONT;
	}
	else if ( focus )
	{
		color = text_color_highlight;
		style = UI_LEFT|UI_PULSE|UI_SMALLFONT;
	}
	else
	{
		color = text_color_normal;
		style = UI_LEFT|UI_SMALLFONT;
	}

	if ( focus )
	{
		// draw cursor
		ui->FillRect( rb->generic.left, rb->generic.top, rb->generic.right-rb->generic.left+1, rb->generic.bottom-rb->generic.top+1, listbar_color ); 
		ui->DrawChar( x, y, 13, UI_CENTER|UI_BLINK|UI_SMALLFONT, color);
	}

	if ( rb->generic.name ) {
		ui->DrawString( x - SMALLCHAR_WIDTH, y, rb->generic.name, UI_RIGHT|UI_SMALLFONT, color );
    }

	if ( !rb->curvalue )
	{
		ui->DrawHandlePic( x + SMALLCHAR_WIDTH, y + 2, 16, 16, ui->rb_off);
		ui->DrawString( x + SMALLCHAR_WIDTH + 16, y, "off", style, color );
	}
	else
	{
		ui->DrawHandlePic( x + SMALLCHAR_WIDTH, y + 2, 16, 16, ui->rb_on );
		ui->DrawString( x + SMALLCHAR_WIDTH + 16, y, "on", style, color );
	}
}

/*
=================
Slider_Init
=================
*/
static void Slider_Init( mslider_t *s )
{
	int len;

	// calculate bounds
	if (s->generic.name) {
		len = strlen(s->generic.name);
    }
	else {
		len = 0;
    }

	s->generic.left   = s->generic.x - (len+1)*SMALLCHAR_WIDTH; 
	s->generic.right  = s->generic.x + (SLIDER_RANGE+2+1)*SMALLCHAR_WIDTH;
	s->generic.top    = s->generic.y;
	s->generic.bottom = s->generic.y + SMALLCHAR_HEIGHT;
}

/*
=================
Slider_Key
=================
*/
static sfxHandle_t Slider_Key( mslider_t *s, uint32_t key )
{
	sfxHandle_t	sound;
	int			x;
	int			oldvalue;

	switch (key) {
	case KEY_MOUSE_LEFT:
		x           = ui->GetCursorX() - s->generic.x - 2*SMALLCHAR_WIDTH;
		oldvalue    = s->curvalue;
		s->curvalue = (x/(float)(SLIDER_RANGE*SMALLCHAR_WIDTH)) * (s->maxvalue-s->minvalue) + s->minvalue;
		if (s->curvalue < s->minvalue) {
			s->curvalue = s->minvalue;
        }
		else if (s->curvalue > s->maxvalue) {
			s->curvalue = s->maxvalue;
        }
		if (s->curvalue != oldvalue) {
			sound = menu_move_sound;
        }
		else {
			sound = 0;
        }
		break;
	case KEY_LEFT:
		if (s->curvalue > s->minvalue)
		{
			s->curvalue--;
			sound = menu_move_sound;
		}
		else {
			sound = menu_buzz_sound;
        }
		break;
	case KEY_RIGHT:
		if (s->curvalue < s->maxvalue)
		{
			s->curvalue++;
			sound = menu_move_sound;
		}
		else {
			sound = menu_buzz_sound;
        }
		break;			
	default:
		// key not handled
		sound = 0;
		break;
	};

	if ( sound && s->generic.EventCallback ) {
		s->generic.EventCallback( s, EVENT_ACTIVATED );
    }

	return (sound);
}

/*
=================
Slider_Draw
=================
*/
static void Slider_Draw( mslider_t *s ) {
	int			x;
	int			y;
	int			style;
	float		*color;
	int			button;
	qboolean	focus;
	
	x =	s->generic.x;
	y = s->generic.y;
	focus = (s->generic.parent->cursor == s->generic.menuPosition);

	if( s->generic.flags & QMF_GRAYED ) {
		color = text_color_disabled;
		style = UI_SMALLFONT;
	}
	else if( focus ) {
		color  = text_color_highlight;
		style = UI_SMALLFONT | UI_PULSE;
	}
	else {
		color = text_color_normal;
		style = UI_SMALLFONT;
	}

	// draw label
	ui->DrawString( x - SMALLCHAR_WIDTH, y, s->generic.name, UI_RIGHT|style, color );

	// draw slider
	ui->SetColor( color );
	ui->DrawHandlePic( x + SMALLCHAR_WIDTH, y, 96, 16, sliderBar );
	ui->SetColor( NULL );

	// clamp thumb
	if( s->maxvalue > s->minvalue )	{
		s->range = ( s->curvalue - s->minvalue ) / ( float ) ( s->maxvalue - s->minvalue );
		if( s->range < 0 ) {
			s->range = 0;
		}
		else if( s->range > 1) {
			s->range = 1;
		}
	}
	else {
		s->range = 0;
	}

	// draw thumb
	if( style & UI_PULSE) {
		button = sliderButton_1;
	}
	else {
		button = sliderButton_0;
	}

	ui->DrawHandlePic( (int)( x + 2*SMALLCHAR_WIDTH + (SLIDER_RANGE-1)*SMALLCHAR_WIDTH* s->range ) - 2, y - 2, 12, 20, button );
}

/*
=================
SpinControl_Init
=================
*/
static void SpinControl_Init( mlist_t *s ) {
	int	len;
	int	l;
	const char* str;

	if (s->generic.name) {
		len = strlen(s->generic.name) * SMALLCHAR_WIDTH;
    }
	else {
		len = 0;
    }

	s->generic.left	= s->generic.x - SMALLCHAR_WIDTH - len;

	len = s->numitems = 0;
	while ( (str = s->itemnames[s->numitems]) != 0 ) {
		l = strlen(str);
		if (l > len) {
			len = l;
        }

		s->numitems++;
	}		

	s->generic.top	  =	s->generic.y;
	s->generic.right  =	s->generic.x + (len+1)*SMALLCHAR_WIDTH;
	s->generic.bottom =	s->generic.y + SMALLCHAR_HEIGHT;
}

/*
=================
SpinControl_Key
=================
*/
static sfxHandle_t SpinControl_Key( mlist_t *s, uint32_t key )
{
	sfxHandle_t	sound;

	sound = 0;
	switch (key) {
	case KEY_MOUSE_LEFT:
		s->curvalue++;
		if (s->curvalue >= s->numitems) {
			s->curvalue = 0;
        }
		sound = menu_move_sound;
		break;
	case KEY_LEFT:
		if (s->curvalue > 0)
		{
			s->curvalue--;
			sound = menu_move_sound;
		}
		else {
		    sound = menu_buzz_sound;
        }
		break;
	case KEY_RIGHT:
		if (s->curvalue < s->numitems-1)
		{
			s->curvalue++;
			sound = menu_move_sound;
		}
		else {
			sound = menu_buzz_sound;
        }
		break;
	};

	if ( sound && s->generic.EventCallback ) {
		s->generic.EventCallback( s, EVENT_ACTIVATED );
    }

	return (sound);
}

/*
=================
SpinControl_Draw
=================
*/
static void SpinControl_Draw( mlist_t *s )
{
	float *color;
	int	x,y;
	int	style;
	qboolean focus;

	x = s->generic.x;
	y =	s->generic.y;

	style = UI_SMALLFONT;
	focus = (s->generic.parent->cursor == s->generic.menuPosition);

	if ( s->generic.flags & QMF_GRAYED ) {
		color = text_color_disabled;
    }
	else if ( focus )
	{
		color = text_color_highlight;
		style |= UI_PULSE;
	}
	else if ( s->generic.flags & QMF_BLINK )
	{
		color = text_color_highlight;
		style |= UI_BLINK;
	}
	else {
		color = text_color_normal;
    }

	if ( focus )
	{
		// draw cursor
		ui->FillRect( s->generic.left, s->generic.top, s->generic.right-s->generic.left+1, s->generic.bottom-s->generic.top+1, listbar_color ); 
		ui->DrawChar( x, y, 13, UI_CENTER|UI_BLINK|UI_SMALLFONT, color);
	}

	ui->DrawString( x - SMALLCHAR_WIDTH, y, s->generic.name, style|UI_RIGHT, color );
	ui->DrawString( x + SMALLCHAR_WIDTH, y, s->itemnames[s->curvalue], style|UI_LEFT, color );
}

/*
=================
ScrollList_Init
=================
*/
static void ScrollList_Init( mlist_t *l )
{
	int		w;

	l->oldvalue = 0;
	l->curvalue = 0;
	l->top      = 0;

	if( !l->columns ) {
		l->columns = 1;
		l->separation = 0;
	}
	else if( !l->separation ) {
		l->separation = 3;
	}

	w = ( (l->width + l->separation) * l->columns - l->separation) * SMALLCHAR_WIDTH;

	l->generic.left   =	l->generic.x;
	l->generic.top    = l->generic.y;	
	l->generic.right  =	l->generic.x + w;
	l->generic.bottom =	l->generic.y + l->height * SMALLCHAR_HEIGHT;

	if( l->generic.flags & QMF_CENTER_JUSTIFY ) {
		l->generic.left -= w / 2;
		l->generic.right -= w / 2;
	}
}

/*
=================
ScrollList_Key
=================
*/
sfxHandle_t ScrollList_Key( mlist_t *l, uint32_t key )
{
	int	x;
	int	y;
	int	w;
	int	i;
	int	j;	
	int	c;
	int	cursorx;
	int	cursory;
	int	column;
	int	index;

	switch (key) {
	case KEY_MOUSE_LEFT:
		if (l->generic.flags & QMF_HASMOUSEFOCUS)
		{
			// check scroll region
			x = l->generic.x;
			y = l->generic.y;
			w = ( (l->width + l->separation) * l->columns - l->separation) * SMALLCHAR_WIDTH;
			if( l->generic.flags & QMF_CENTER_JUSTIFY ) {
				x -= w / 2;
			}
			if (ui->CursorInRect( x, y, w, l->height*SMALLCHAR_HEIGHT ))
			{
				cursorx = (ui->GetCursorX() - x)/SMALLCHAR_WIDTH;
				column = cursorx / (l->width + l->separation);
				cursory = (ui->GetCursorY() - y)/SMALLCHAR_HEIGHT;
				index = column * l->height + cursory;
				if (l->top + index < l->numitems)
				{
					l->oldvalue = l->curvalue;
					l->curvalue = l->top + index;

					if (l->oldvalue != l->curvalue && l->generic.EventCallback)
					{
						l->generic.EventCallback( l, EVENT_GOTFOCUS );
						return (menu_move_sound);
					}
				}
			}
			
			// absorbed, silent sound effect
			return (menu_null_sound);
		}
		break;
	case KEY_HOME:
		l->oldvalue = l->curvalue;
		l->curvalue = 0;
		l->top      = 0;

		if (l->oldvalue != l->curvalue && l->generic.EventCallback)
		{
			l->generic.EventCallback( l, EVENT_GOTFOCUS );
			return (menu_move_sound);
		}
		return (menu_buzz_sound);
	case KEY_END:
		l->oldvalue = l->curvalue;
		l->curvalue = l->numitems-1;
		if( l->columns > 1 ) {
			c = (l->curvalue / l->height + 1) * l->height;
			l->top = c - (l->columns * l->height);
		}
		else {
			l->top = l->curvalue - (l->height - 1);
		}
		if (l->top < 0) {
			l->top = 0;
        }

		if (l->oldvalue != l->curvalue && l->generic.EventCallback)
		{
			l->generic.EventCallback( l, EVENT_GOTFOCUS );
			return (menu_move_sound);
		}
		return (menu_buzz_sound);
	case KEY_PAGEUP:
		if( l->columns > 1 ) {
			return menu_null_sound;
		}

		if (l->curvalue > 0)
		{
			l->oldvalue = l->curvalue;
			l->curvalue -= l->height-1;
			if (l->curvalue < 0) {
				l->curvalue = 0;
            }
            l->top = l->curvalue;
			if (l->top < 0) {
				l->top = 0;
            }
			if (l->generic.EventCallback) {
				l->generic.EventCallback( l, EVENT_GOTFOCUS );
            }
            
			return (menu_move_sound);
		}
		return (menu_buzz_sound);
    case KEY_PAGEDOWN:
		if( l->columns > 1 ) {
			return menu_null_sound;
		}

		if (l->curvalue < l->numitems-1) {
				l->oldvalue = l->curvalue;
				l->curvalue += l->height-1;
				if (l->curvalue > l->numitems-1) {
					l->curvalue = l->numitems-1;
                }

				l->top = l->curvalue - (l->height-1);
				
                if (l->top < 0) {
					l->top = 0;
                }

				if (l->generic.EventCallback) {
					l->generic.EventCallback( l, EVENT_GOTFOCUS );
                }
            return (menu_move_sound);
		}
		return (menu_buzz_sound);
	case KEY_UP:
		if( l->curvalue == 0 ) {
			return menu_buzz_sound;
		}

		l->oldvalue = l->curvalue;
		l->curvalue--;

		if( l->curvalue < l->top ) {
			if( l->columns == 1 ) {
				l->top--;
			}
			else {
				l->top -= l->height;
			}
		}

		if( l->generic.EventCallback ) {
			l->generic.EventCallback( l, EVENT_GOTFOCUS );
		}

		return (menu_move_sound);
	case KEY_DOWN:
		if( l->curvalue == l->numitems - 1 ) {
			return menu_buzz_sound;
		}

		l->oldvalue = l->curvalue;
		l->curvalue++;

		if( l->curvalue >= l->top + l->columns * l->height ) {
			if( l->columns == 1 ) {
				l->top++;
			}
			else {
				l->top += l->height;
			}
		}

		if( l->generic.EventCallback ) {
			l->generic.EventCallback( l, EVENT_GOTFOCUS );
		}

		return menu_move_sound;
	case KEY_LEFT:
		if( l->columns == 1 ) {
			return menu_null_sound;
		}

		if( l->curvalue < l->height ) {
			return menu_buzz_sound;
		}

		l->oldvalue = l->curvalue;
		l->curvalue -= l->height;

		if( l->curvalue < l->top ) {
			l->top -= l->height;
		}

		if( l->generic.EventCallback ) {
			l->generic.EventCallback( l, EVENT_GOTFOCUS );
		}

		return menu_move_sound;
	case KEY_RIGHT:
		if( l->columns == 1 ) {
			return menu_null_sound;
		}

		c = l->curvalue + l->height;

		if( c >= l->numitems ) {
			return menu_buzz_sound;
		}

		l->oldvalue = l->curvalue;
		l->curvalue = c;

		if( l->curvalue > l->top + l->columns * l->height - 1 ) {
			l->top += l->height;
		}

		if( l->generic.EventCallback ) {
			l->generic.EventCallback( l, EVENT_GOTFOCUS );
		}

		return menu_move_sound;
	};

	// cycle look for ascii key inside list items
	if ( !N_isprint( key ) )
		return (0);

	// force to lower for case insensitive compare
	if ( N_isupper( key ) )
	{
		key -= 'A' - 'a';
	}

	// iterate list items
	for (i=1; i<=l->numitems; i++)
	{
		j = (l->curvalue + i) % l->numitems;
		c = l->itemnames[j][0];
		if ( N_isupper( c ) )
		{
			c -= 'A' - 'a';
		}

		if (c == key)
		{
			// set current item, mimic windows listbox scroll behavior
			if (j < l->top)
			{
				// behind top most item, set this as new top
				l->top = j;
			}
			else if (j > l->top+l->height-1)
			{
				// past end of list box, do page down
				l->top = (j+1) - l->height;
			}
			
			if (l->curvalue != j)
			{
				l->oldvalue = l->curvalue;
				l->curvalue = j;
				if (l->generic.EventCallback)
					l->generic.EventCallback( l, EVENT_GOTFOCUS );
				return ( menu_move_sound );			
			}

			return (menu_buzz_sound);
		}
	}

	return (menu_buzz_sound);
}

/*
=================
ScrollList_Draw
=================
*/
void ScrollList_Draw( mlist_t *l )
{
	int			x;
	int			u;
	int			y;
	int			i;
	int			base;
	int			column;
	float*		color;
	qboolean	hasfocus;
	int			style;

	hasfocus = (l->generic.parent->cursor == l->generic.menuPosition);

	x =	l->generic.x;
	for( column = 0; column < l->columns; column++ ) {
		y =	l->generic.y;
		base = l->top + column * l->height;
		for( i = base; i < base + l->height; i++) {
			if (i >= l->numitems) {
				break;
            }

			if (i == l->curvalue) {
				u = x - 2;
				if( l->generic.flags & QMF_CENTER_JUSTIFY ) {
					u -= (l->width * SMALLCHAR_WIDTH) / 2 + 1;
				}

				ui->FillRect(u,y,l->width*SMALLCHAR_WIDTH,SMALLCHAR_HEIGHT+2,listbar_color);
				color = text_color_highlight;

				if (hasfocus) {
					style = UI_PULSE|UI_LEFT|UI_SMALLFONT;
                }
				else {
					style = UI_LEFT|UI_SMALLFONT;
                }
			}
			else {
				color = text_color_normal;
				style = UI_LEFT|UI_SMALLFONT;
			}
			if( l->generic.flags & QMF_CENTER_JUSTIFY ) {
				style |= UI_CENTER;
			}

			ui->DrawString(
				x,
				y,
				l->itemnames[i],
				style,
				color);

			y += SMALLCHAR_HEIGHT;
		}
		x += (l->width + l->separation) * SMALLCHAR_WIDTH;
	}
}

/*
=================
Menu_AddItem
=================
*/
void Menu_AddItem( CUIMenu *menu, void *item )
{
	CUIMenuWidget	*itemptr;

	if (menu->nitems >= MAX_MENU_ITEMS) {
		N_Error (ERR_DROP, "Menu_AddItem: excessive items");
    }

	menu->items[menu->nitems] = item;
	((CUIMenuWidget*)menu->items[menu->nitems])->parent        = menu;
	((CUIMenuWidget*)menu->items[menu->nitems])->menuPosition  = menu->nitems;
	((CUIMenuWidget*)menu->items[menu->nitems])->flags        &= ~QMF_HASMOUSEFOCUS;

	// perform any item specific initializations
	itemptr = (CUIMenuWidget*)item;
	if (!(itemptr->flags & QMF_NODEFAULTINIT)) {
		switch (itemptr->type) {
		case MTYPE_ACTION:
			Action_Init((maction_t*)item);
			break;
		case MTYPE_FIELD:
			MenuField_Init((mfield_t*)item);
			break;
		case MTYPE_SPINCONTROL:
			SpinControl_Init((mlist_t*)item);
			break;
		case MTYPE_RADIOBUTTON:
			RadioButton_Init((mradiobutton_t*)item);
			break;
		case MTYPE_SLIDER:
			Slider_Init((mslider_t*)item);
			break;
		case MTYPE_BITMAP:
			Bitmap_Init((mbitmap_t*)item);
			break;
		case MTYPE_TEXT:
			Text_Init((mtext_t*)item);
			break;
		case MTYPE_SCROLLLIST:
			ScrollList_Init((mlist_t*)item);
			break;
		case MTYPE_PTEXT:
			PText_Init((mtext_t*)item);
			break;
		case MTYPE_BTEXT:
			BText_Init((mtext_t*)item);
			break;
		default:
			N_Error( ERR_DROP, "Menu_Init: unknown type %d", itemptr->type );
		};
	}

	menu->nitems++;
}

/*
=================
Menu_CursorMoved
=================
*/
void Menu_CursorMoved( CUIMenu *m )
{
	void (*callback)( void *self, uint32_t notification );
	
	if (m->cursor_prev == m->cursor) {
		return;
    }

	if (m->cursor_prev >= 0 && m->cursor_prev < m->nitems) {
		callback = ((CUIMenuWidget*)(m->items[m->cursor_prev]))->EventCallback;
		if (callback) {
			callback(m->items[m->cursor_prev],EVENT_LOSTFOCUS);
        }
	}
	
	if (m->cursor >= 0 && m->cursor < m->nitems) {
		callback = ((CUIMenuWidget*)(m->items[m->cursor]))->EventCallback;
		if (callback) {
			callback(m->items[m->cursor],EVENT_GOTFOCUS);
        }
	}
}

/*
=================
Menu_SetCursor
=================
*/
void Menu_SetCursor( CUIMenu *m, int cursor )
{
	if (((CUIMenuWidget*)(m->items[cursor]))->flags & (QMF_GRAYED|QMF_INACTIVE)) {
		// cursor can't go there
		return;
	}

	m->cursor_prev = m->cursor;
	m->cursor      = cursor;

	Menu_CursorMoved( m );
}

/*
=================
Menu_SetCursorToItem
=================
*/
void Menu_SetCursorToItem( CUIMenu *m, void* ptr )
{
	int	i;

	for (i=0; i<m->nitems; i++) {
		if (m->items[i] == ptr) {
			Menu_SetCursor( m, i );
			return;
		}
	}
}

/*
** Menu_AdjustCursor
**
** This function takes the given menu, the direction, and attempts
** to adjust the menu's cursor so that it's at the next available
** slot.
*/
void Menu_AdjustCursor( CUIMenu *m, int dir ) {
	CUIMenuWidget	*item = NULL;
	qboolean		wrapped = qfalse;

wrap:
	while ( m->cursor >= 0 && m->cursor < m->nitems ) {
		item = ( CUIMenuWidget * ) m->items[m->cursor];
		if (( item->flags & (QMF_GRAYED|QMF_MOUSEONLY|QMF_INACTIVE) ) ) {
			m->cursor += dir;
		}
		else {
			break;
		}
	}

	if ( dir == 1 ) {
		if ( m->cursor >= m->nitems ) {
			if ( m->wrapAround ) {
				if ( wrapped ) {
					m->cursor = m->cursor_prev;
					return;
				}
				m->cursor = 0;
				wrapped = qtrue;
				goto wrap;
			}
			m->cursor = m->cursor_prev;
		}
	}
	else {
		if ( m->cursor < 0 ) {
			if ( m->wrapAround ) {
				if ( wrapped ) {
					m->cursor = m->cursor_prev;
					return;
				}
				m->cursor = m->nitems - 1;
				wrapped = qtrue;
				goto wrap;
			}
			m->cursor = m->cursor_prev;
		}
	}
}

/*
=================
Menu_Draw
=================
*/
void Menu_Draw( CUIMenu *menu )
{
	int				i;
	CUIMenuWidget	*itemptr;

	// draw menu
	for (i=0; i<menu->nitems; i++) {
		itemptr = (CUIMenuWidget*)menu->items[i];

		if (itemptr->flags & QMF_HIDDEN) {
			continue;
        }

		if (itemptr->OwnerDraw) {
			// total subclassing, owner draws everything
			itemptr->OwnerDraw( itemptr );
		}	
		else {
			switch (itemptr->type) {	
			case MTYPE_RADIOBUTTON:
				RadioButton_Draw( (mradiobutton_t*)itemptr );
				break;
			case MTYPE_FIELD:
				MenuField_Draw( (mfield_t*)itemptr );
				break;
	
			case MTYPE_SLIDER:
				Slider_Draw( (mslider_t*)itemptr );
				break;
			case MTYPE_SPINCONTROL:
				SpinControl_Draw( (mlist_t*)itemptr );
				break;
			case MTYPE_ACTION:
				Action_Draw( (maction_t*)itemptr );
				break;
			case MTYPE_BITMAP:
				Bitmap_Draw( (mbitmap_t*)itemptr );
				break;
			case MTYPE_TEXT:
				Text_Draw( (mtext_t*)itemptr );
				break;
			case MTYPE_SCROLLLIST:
				ScrollList_Draw( (mlist_t*)itemptr );
				break;
			case MTYPE_PTEXT:
				PText_Draw( (mtext_t*)itemptr );
				break;
			case MTYPE_BTEXT:
				BText_Draw( (mtext_t*)itemptr );
				break;
			default:
				N_Error( ERR_DROP, "Menu_Draw: unknown type %d", itemptr->type );
			};
		}
#ifdef _NOMAD_DEBUG
		if( ui->IsDebug() ) {
			int	x;
			int	y;
			int	w;
			int	h;

			if( !( itemptr->flags & QMF_INACTIVE ) ) {
				x = itemptr->left;
				y = itemptr->top;
				w = itemptr->right - itemptr->left + 1;
				h =	itemptr->bottom - itemptr->top + 1;

				if (itemptr->flags & QMF_HASMOUSEFOCUS) {
					ui->DrawRect(x, y, w, h, colorYellow );
				}
				else {
					ui->DrawRect(x, y, w, h, colorWhite );
				}
			}
		}
#endif
	}

	itemptr = (CUIMenuWidget *)Menu_ItemAtCursor( menu );
	if ( itemptr && itemptr->StatusBar) {
		itemptr->StatusBar( ( void * ) itemptr );
    }
}

/*
=================
Menu_ItemAtCursor
=================
*/
void *Menu_ItemAtCursor( CUIMenu *m )
{
	if ( m->cursor < 0 || m->cursor >= m->nitems ) {
		return 0;
    }

	return m->items[m->cursor];
}

/*
=================
Menu_ActivateItem
=================
*/
sfxHandle_t Menu_ActivateItem( CUIMenu *s, CUIMenuWidget* item ) {
	if ( item->EventCallback ) {
		item->EventCallback( item, EVENT_ACTIVATED );
		if( !( item->flags & QMF_SILENT ) ) {
			return menu_move_sound;
		}
	}

	return 0;
}

/*
=================
Menu_DefaultKey
=================
*/
sfxHandle_t Menu_DefaultKey( CUIMenu *m, uint32_t key )
{
	sfxHandle_t		sound = 0;
	CUIMenuWidget	*item;
	int				cursor_prev;

	// menu system keys
	switch ( key ) {
	case KEY_MOUSE_LEFT:
	case KEY_ESCAPE:
		ui->PopMenu();
		return menu_out_sound;
	};

	if (!m || !m->nitems) {
		return 0;
    }

	// route key stimulus to widget
	item = (CUIMenuWidget *)Menu_ItemAtCursor( m );
	if (item && !(item->flags & (QMF_GRAYED|QMF_INACTIVE))) {
		switch (item->type) {
		case MTYPE_SPINCONTROL:
			sound = SpinControl_Key( (mlist_t*)item, key );
			break;
		case MTYPE_RADIOBUTTON:
			sound = RadioButton_Key( (mradiobutton_t*)item, key );
			break;
		case MTYPE_SLIDER:
			sound = Slider_Key( (mslider_t*)item, key );
			break;
		case MTYPE_SCROLLLIST:
			sound = ScrollList_Key( (mlist_t*)item, key );
			break;
		case MTYPE_FIELD:
			sound = MenuField_Key( (mfield_t*)item, &key );
			break;
		};

		if (sound) {
			// key was handled
			return sound;		
		}
	}

	// default handling
	switch ( key ) {
	case KEY_F11:
		ui->SetDebug( ui->GetDebug() ^ 1 );
		break;
	case KEY_F12:
		Cbuf_ExecuteText(EXEC_APPEND, "screenshot\n");
		break;
	case KEY_UP:
		cursor_prev    = m->cursor;
		m->cursor_prev = m->cursor;
		m->cursor--;
		Menu_AdjustCursor( m, -1 );
		if ( cursor_prev != m->cursor ) {
			Menu_CursorMoved( m );
			sound = menu_move_sound;
		}
		break;
	case KEY_TAB:
	case KEY_DOWN:
		cursor_prev    = m->cursor;
		m->cursor_prev = m->cursor;
		m->cursor++;
		Menu_AdjustCursor( m, 1 );
		if ( cursor_prev != m->cursor ) {
			Menu_CursorMoved( m );
			sound = menu_move_sound;
		}
		break;
	case KEY_MOUSE_LEFT:
	case KEY_MOUSE_RIGHT:
		if (item) {
			if ((item->flags & QMF_HASMOUSEFOCUS) && !(item->flags & (QMF_GRAYED|QMF_INACTIVE))) {
				return (Menu_ActivateItem( m, item ));
            }
        }
		break;
    case KEY_KP_ENTER:
	case KEY_ENTER:
		if (item) {
			if (!(item->flags & (QMF_MOUSEONLY|QMF_GRAYED|QMF_INACTIVE))) {
				return (Menu_ActivateItem( m, item ));
            }
        }
		break;
	};

	return sound;
}

typedef struct {
	const char *m_pName;
	int32_t m_nColumns;
	int32_t m_nRows;
	ImGuiTableFlags m_Flags;
} mtable_t;

void Table_Draw( mtable_t *table )
{
	ImGui::BeginTable( table->m_pName, table->m_nColumns, table->m_Flags );

	for ( int32_t y = 0; y < table->m_nRows; y++ ) {
		if ( y != table->m_nRows - 1 ) {
			ImGui::TableNextRow();
		}
		for ( int32_t x = 0; x < table->m_nColumns; x++ ) {
			if ( x != table->m_nColumns - 1 ) {
				ImGui::TableNextColumn();
			}
			
		}
	}

	ImGui::EndTable();
}
