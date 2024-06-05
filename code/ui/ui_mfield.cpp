/*
===========================================================================
Copyright (C) 2023-2024 GDR Games

This file is part of The Nomad source code.

The Nomad source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

The Nomad source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Foobar; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/

#include "../game/g_game.h"
#include "ui_public.hpp"
#include "ui_menu.h"
#include "ui_lib.h"

/*
===================
MField_Draw

Handles horizontal scrolling and cursor blinking
x, y, are in pixels
===================
*/
void MField_Draw( mfield_t *edit, int x, int y, int style, vec4_t color )
{
	int		len;
	int		charw;
	int		drawLen;
	int		prestep;
	int		cursorChar;
	char	str[MAX_STRING_CHARS];

	drawLen = edit->widthInChars;
	len     = strlen( edit->buffer ) + 1;

	// guarantee that cursor will be visible
	if ( len <= drawLen ) {
		prestep = 0;
	} else {
		if ( edit->scroll + drawLen > len ) {
			edit->scroll = len - drawLen;
			if ( edit->scroll < 0 ) {
				edit->scroll = 0;
			}
		}
		prestep = edit->scroll;
	}

	if ( prestep + drawLen > len ) {
		drawLen = len - prestep;
	}

	// extract <drawLen> characters from the field at <prestep>
	if ( drawLen >= MAX_STRING_CHARS ) {
		N_Error( ERR_DROP, "drawLen >= MAX_STRING_CHARS" );
	}
	memcpy( str, edit->buffer + prestep, drawLen );
	str[ drawLen ] = 0;

	ui->DrawString( x, y, str, style, color );

	// draw the cursor
	if (!(style & UI_PULSE)) {
		return;
	}

	if ( Key_GetOverstrikeMode() ) {
		cursorChar = 11;
	} else {
		cursorChar = 10;
	}

	style &= ~UI_PULSE;
	style |= UI_BLINK;

	if (style & UI_SMALLFONT)
	{
		charw =	SMALLCHAR_WIDTH;
	}
	else if (style & UI_GIANTFONT)
	{
		charw =	GIANTCHAR_WIDTH;
	}
	else
	{
		charw =	BIGCHAR_WIDTH;
	}

	if (style & UI_CENTER)
	{
		len = strlen(str);
		x = x - len*charw/2;
	}
	else if (style & UI_RIGHT)
	{
		len = strlen(str);
		x = x - len*charw;
	}
	
    ui->DrawChar( x + ( edit->cursor - prestep ) * charw, y, cursorChar, style & ~(UI_CENTER|UI_RIGHT), color );
}

/*
================
MField_Paste
================
*/
void MField_Paste( mfield_t *edit ) {
	char	pasteBuffer[64];
	int		pasteLen, i;

    memcpy(pasteBuffer, Sys_GetClipboardData(), sizeof(pasteBuffer));

	// send as if typed, so insert / overstrike works properly
	pasteLen = strlen( pasteBuffer );
	for ( i = 0 ; i < pasteLen ; i++ ) {
		MField_CharEvent( edit, pasteBuffer[i] );
	}
}

/*
=================
MField_KeyDownEvent

Performs the basic line editing functions for the console,
in-game talk, and menu fields

Key events are used for non-printable characters, others are gotten from char events.
=================
*/
void MField_KeyDownEvent( mfield_t *edit, uint32_t key )
{
	int		len;

	// shift-insert is paste
	if ( ( ( key == KEY_INSERT ) /*|| ( key == K_KP_INS )*/ ) && Key_IsDown( KEY_SHIFT ) ) {
		MField_Paste( edit );
		return;
	}

	len = strlen( edit->buffer );

	if ( key == KEY_DELETE /*|| key == K_KP_DEL*/ ) {
		if ( edit->cursor < len ) {
			memmove( edit->buffer + edit->cursor, 
				edit->buffer + edit->cursor + 1, len - edit->cursor );
		}
		return;
	}

	if ( key == KEY_RIGHT ) {
		if ( edit->cursor < len ) {
			edit->cursor++;
		}
		if ( edit->cursor >= edit->scroll + edit->widthInChars && edit->cursor <= len ) {
			edit->scroll++;
		}
		return;
	}

	if ( key == KEY_LEFT ) {
		if ( edit->cursor > 0 ) {
			edit->cursor--;
		}
		if ( edit->cursor < edit->scroll ) {
			edit->scroll--;
		}
		return;
	}

	if ( key == KEY_HOME || ( tolower(key) == 'a' && Key_IsDown( KEY_CTRL ) ) ) {
		edit->cursor = 0;
		edit->scroll = 0;
		return;
	}

	if ( key == KEY_END || ( tolower(key) == 'e' && Key_IsDown( KEY_CTRL ) ) ) {
		edit->cursor = len;
		edit->scroll = len - edit->widthInChars + 1;
		if (edit->scroll < 0) {
			edit->scroll = 0;
        }
		return;
	}

	if ( key == KEY_INSERT ) {
		Key_SetOverstrikeMode( !Key_GetOverstrikeMode() );
		return;
	}
}

/*
==================
MField_CharEvent
==================
*/
void MField_CharEvent( mfield_t *edit, int ch )
{
	int		len;

	if ( ch == 'v' - 'a' + 1 ) {	// ctrl-v is paste
		MField_Paste( edit );
		return;
	}

	if ( ch == 'c' - 'a' + 1 ) {	// ctrl-c clears the field
		MField_Clear( edit );
		return;
	}

	len = strlen( edit->buffer );

	if ( ch == 'h' - 'a' + 1 )	{	// ctrl-h is backspace
		if ( edit->cursor > 0 ) {
			memmove( edit->buffer + edit->cursor - 1, edit->buffer + edit->cursor, len + 1 - edit->cursor );
			edit->cursor--;
			if ( edit->cursor < edit->scroll ) {
				edit->scroll--;
			}
		}
		return;
	}

	if ( ch == 'a' - 'a' + 1 ) {	// ctrl-a is home
		edit->cursor = 0;
		edit->scroll = 0;
		return;
	}

	if ( ch == 'e' - 'a' + 1 ) {	// ctrl-e is end
		edit->cursor = len;
		edit->scroll = edit->cursor - edit->widthInChars + 1;
		if (edit->scroll < 0)
			edit->scroll = 0;
		return;
	}

	//
	// ignore any other non printable chars
	//
	if ( ch < 32 ) {
		return;
	}

	if ( !Key_GetOverstrikeMode() ) {	
		if ((edit->cursor == MAX_EDIT_LINE - 1) || (edit->maxchars && edit->cursor >= edit->maxchars)) {
			return;
        }
	} else {
		// insert mode
		if (( len == MAX_EDIT_LINE - 1 ) || (edit->maxchars && len >= edit->maxchars)) {
			return;
        }
		memmove( edit->buffer + edit->cursor + 1, edit->buffer + edit->cursor, len + 1 - edit->cursor );
	}

	edit->buffer[edit->cursor] = ch;
	if (!edit->maxchars || edit->cursor < edit->maxchars-1) {
		edit->cursor++;
    }

	if ( edit->cursor >= edit->widthInChars ) {
		edit->scroll++;
	}

	if ( edit->cursor == len + 1) {
		edit->buffer[edit->cursor] = 0;
	}
}

/*
==================
MField_Clear
==================
*/
void MField_Clear( mfield_t *edit ) {
	edit->buffer[0] = 0;
	edit->cursor = 0;
	edit->scroll = 0;
}

/*
==================
MenuField_Init
==================
*/
void MenuField_Init( mfield_t* m )
{
	int	l;
	int	w;
	int	h;

	MField_Clear( m );

	if (m->generic.flags & QMF_SMALLFONT) {
		w = SMALLCHAR_WIDTH;
		h = SMALLCHAR_HEIGHT;
	}
	else {
		w = BIGCHAR_WIDTH;
		h = BIGCHAR_HEIGHT;
	}	

	if (m->generic.name) {
		l = (strlen( m->generic.name )+1) * w;		
	}
	else {
		l = 0;
	}

	m->generic.left   = m->generic.x - l;
	m->generic.top    = m->generic.y;
	m->generic.right  = m->generic.x + w + m->widthInChars*w;
	m->generic.bottom = m->generic.y + h;
}

/*
==================
MenuField_Draw
==================
*/
void MenuField_Draw( mfield_t *f )
{
	int		x;
	int		y;
	int		w;
	int		h;
	int		style;
	qboolean focus;
	float	*color;

	x =	f->generic.x;
	y =	f->generic.y;

	if (f->generic.flags & QMF_SMALLFONT) {
		w = SMALLCHAR_WIDTH;
		h = SMALLCHAR_HEIGHT;
		style = UI_SMALLFONT;
	}
	else {
		w = BIGCHAR_WIDTH;
		h = BIGCHAR_HEIGHT;
		style = UI_BIGFONT;
	}	

	if (Menu_ItemAtCursor( f->generic.parent ) == f) {
		focus = qtrue;
		style |= UI_PULSE;
	}
	else {
		focus = qfalse;
	}

	if (f->generic.flags & QMF_GRAYED)
		color = text_color_disabled;
	else if (focus)
		color = text_color_highlight;
	else
		color = text_color_normal;

	if ( focus )
	{
		// draw cursor
		ui->FillRect( f->generic.left, f->generic.top, f->generic.right-f->generic.left+1, f->generic.bottom-f->generic.top+1, listbar_color ); 
		ui->DrawChar( x, y, 13, UI_CENTER|UI_BLINK|style, color);
	}

	if ( f->generic.name ) {
		ui->DrawString( x - w, y, f->generic.name, style|UI_RIGHT, color );
	}

	MField_Draw( f, x + w, y, style, color );
}

/*
==================
MenuField_Key
==================
*/
sfxHandle_t MenuField_Key( mfield_t* m, uint32_t* key )
{
	uint32_t keycode;

	keycode = *key;

	switch ( keycode ) {
	case KEY_KP_ENTER:
	case KEY_ENTER:
		// have enter go to next cursor point
		*key = KEY_TAB;
		break;
	case KEY_TAB:
	case KEY_DOWN:
	case KEY_UP:
		break;
	default:
		if ( keycode & K_CHAR_FLAG ) {
			keycode &= ~K_CHAR_FLAG;
			if ((m->generic.flags & QMF_UPPERCASE) && N_islower( keycode )) {
				keycode -= 'a' - 'A';
            } else if ((m->generic.flags & QMF_LOWERCASE) && N_isupper( keycode )) {
				keycode -= 'A' - 'a';
            } else if ((m->generic.flags & QMF_NUMBERSONLY) && N_isalpha( keycode )) {
				return (menu_buzz_sound);
            }
			MField_CharEvent( m, keycode);
		}
		else {
			MField_KeyDownEvent( m, keycode );
        }
		break;
	};

	return (0);
}


