#include "../engine/n_shared.h"
#include "../game/g_game.h"
#include "ui_public.hpp"
#include "ui_menu.h"
#include "ui_lib.h"

qboolean m_entersound;

qboolean UI_MenuOption( const char *label )
{
	qboolean retn;

    ImGui::TableNextColumn();
    ImGui::TextUnformatted( label );
    ImGui::TableNextColumn();
    ImGui::SameLine();

    retn = ImGui::ArrowButton( label, ImGuiDir_Right );
	if ( retn ) {
		Snd_PlaySfx( ui->sfx_select );
	}

	return retn;
}

static qboolean GamepadUsed( void )
{
	if ( keys[ KEY_PAD0_A ].down ) { return qtrue; }
	if ( keys[ KEY_PAD0_B ].down ) { return qtrue; }
	if ( keys[ KEY_PAD0_X ].down ) { return qtrue; }
	if ( keys[ KEY_PAD0_Y ].down ) { return qtrue; }
	if ( keys[ KEY_PAD0_BACK ].down ) { return qtrue; }
	if ( keys[ KEY_PAD0_GUIDE ].down ) { return qtrue; }
	if ( keys[ KEY_PAD0_START ].down ) { return qtrue; }
	if ( keys[ KEY_PAD0_LEFTSTICK_CLICK ].down ) { return qtrue; }
	if ( keys[ KEY_PAD0_RIGHTSTICK_CLICK ].down ) { return qtrue; }
	if ( keys[ KEY_PAD0_LEFTBUTTON ].down ) { return qtrue; }
	if ( keys[ KEY_PAD0_RIGHTBUTTON ].down ) { return qtrue; }
	if ( keys[ KEY_PAD0_DPAD_UP ].down ) { return qtrue; }
	if ( keys[ KEY_PAD0_DPAD_DOWN ].down ) { return qtrue; }
	if ( keys[ KEY_PAD0_DPAD_LEFT ].down ) { return qtrue; }
	if ( keys[ KEY_PAD0_DPAD_RIGHT ].down ) { return qtrue; }
	if ( keys[ KEY_PAD0_LEFTSTICK_LEFT ].down ) { return qtrue; }
	if ( keys[ KEY_PAD0_LEFTSTICK_RIGHT ].down ) { return qtrue; }
	if ( keys[ KEY_PAD0_LEFTSTICK_UP ].down ) { return qtrue; }
	if ( keys[ KEY_PAD0_LEFTSTICK_DOWN ].down ) { return qtrue; }
	if ( keys[ KEY_PAD0_RIGHTSTICK_LEFT ].down ) { return qtrue; }
	if ( keys[ KEY_PAD0_RIGHTSTICK_RIGHT ].down ) { return qtrue; }
	if ( keys[ KEY_PAD0_RIGHTSTICK_UP ].down ) { return qtrue; }
	if ( keys[ KEY_PAD0_RIGHTSTICK_DOWN ].down ) { return qtrue; }
	if ( keys[ KEY_PAD0_LEFTTRIGGER ].down ) { return qtrue; }
	if ( keys[ KEY_PAD0_RIGHTTRIGGER ].down ) { return qtrue; }
	if ( keys[ KEY_PAD0_MISC1 ].down ) { return qtrue; }
	if ( keys[ KEY_PAD0_PADDLE1 ].down ) { return qtrue; }
	if ( keys[ KEY_PAD0_PADDLE2 ].down ) { return qtrue; }
	if ( keys[ KEY_PAD0_PADDLE3 ].down ) { return qtrue; }
	if ( keys[ KEY_PAD0_PADDLE4 ].down ) { return qtrue; }
	if ( keys[ KEY_PAD0_TOUCHPAD ].down ) { return qtrue; }
	return qfalse;
}

qboolean UI_MenuTitle( const char *label, float fontScale )
{
	ImVec2 cursorPos;
	renderSceneRef_t refdef;
	extern cvar_t *in_joystick;
	nhandle_t hShader;

	memset( &refdef, 0, sizeof( refdef ) );
	refdef.x = 0;
	refdef.y = 0;
	refdef.width = ui->gpuConfig.vidWidth;
	refdef.height = ui->gpuConfig.vidHeight;
	refdef.time = 0;
	refdef.flags = RSF_NOWORLDMODEL | RSF_ORTHO_TYPE_SCREENSPACE;

	FontCache()->SetActiveFont( AlegreyaSC );

	ImGui::PushStyleColor( ImGuiCol_Text, colorRed );
    ImGui::SetWindowFontScale( ( ImGui::GetFont()->Scale * fontScale ) * ui->scale );
    ImGui::TextUnformatted( label );
    ImGui::SetWindowFontScale( 1.0f );
	ImGui::PopStyleColor();

	cursorPos = ImGui::GetCursorScreenPos();

//	ImGui::SetWindowFontScale( 1.5f * scale );
	ImGui::Begin( "##BackButtonArrowMainMenu", NULL, MENU_DEFAULT_FLAGS | ImGuiWindowFlags_AlwaysAutoResize );
	ImGui::SetWindowPos( ImVec2( 16 * ui->scale, 680 * ui->scale ) );
	if ( ui->menusp >= 2 ) {
		if ( in_mode->i == 1 ) {
			hShader = ui->controller_back;
		} else {
			hShader = ui->backHovered ? ui->back_1 : ui->back_0;
		}
		ImGui::Image( (ImTextureID)(uintptr_t)hShader, ImVec2( 256 * ui->scale, 72 * ui->scale ) );
		if ( !ui->backHovered && ImGui::IsItemHovered( ImGuiHoveredFlags_AllowWhenDisabled | ImGuiHoveredFlags_DelayNone ) ) {
//			Snd_PlaySfx( ui->sfx_move );
		}
		ui->backHovered = ImGui::IsItemHovered( ImGuiHoveredFlags_AllowWhenDisabled | ImGuiHoveredFlags_DelayNone );
		if ( ImGui::IsItemClicked() ) {
			Snd_PlaySfx( ui->sfx_back );
			return true;
		}
	}
	ImGui::SetCursorScreenPos( cursorPos );
	ImGui::End();

	ImGui::SetWindowFontScale( 1.0f );

    return false;
}

void UI_PushMenu( menuframework_t *menu )
{
	int i;

    // avoid stacking menus invoked by hotkeys
    for ( i = 0; i < ui->menusp; i++ ) {
        if ( ui->stack[i] == menu ) {
            ui->menusp = i;
            break;
        }
    }

    if ( i == ui->menusp ) {
        if ( ui->menusp >= MAX_MENU_DEPTH ) {
            N_Error( ERR_DROP, "UI_PushMenu: menu stack overflow" );
        }
        ui->stack[ui->menusp++] = menu;
    }

    ui->activemenu = menu;

    // default cursor position
//    menu->cursor = 0;
//    menu->cursor_prev = 0;

    Key_SetCatcher( KEYCATCH_UI );
}

/*
* UI_DrawText: renders text, only handles colors and Quake3 engine style formatting
*/
extern "C" void UI_DrawText( const char *txt )
{
	uint64_t i, len;
	const char *text;
	int colorIndex, currentColorIndex;
	qboolean usedColor = qfalse;
	char s[2];

	len = strlen( txt );
	currentColorIndex = ColorIndex( S_COLOR_WHITE );

	for ( i = 0, text = txt; i < len; i++, text++ ) {
		// track color changes
		while ( Q_IsColorString( text ) && *( text + 1 ) != '\n' ) {
			colorIndex = ColorIndexFromChar( *( text + 1 ) );
			if ( currentColorIndex != colorIndex ) {
				currentColorIndex = colorIndex;
				if ( usedColor ) {
					ImGui::PopStyleColor();
				}
				ImGui::PushStyleColor( ImGuiCol_Text, ImVec4( g_color_table[ colorIndex ] ) );
				usedColor = qtrue;
			}
			text += 2;
		}

		switch ( *text ) {
		case '\n':
			if ( usedColor ) {
				ImGui::PopStyleColor();
				currentColorIndex = ColorIndex( S_COLOR_WHITE );
				ImGui::PushStyleColor( ImGuiCol_Text, ImVec4( g_color_table[ currentColorIndex ] ) );
				usedColor = qfalse;
			}
			ImGui::NewLine();
			break;
		case '\r':
			ImGui::SameLine();
			break;
		default:
			s[0] = *text;
			s[1] = 0;

			ImGui::TextUnformatted( s );
			ImGui::SameLine();
			break;
		};
	}
}

void UI_PopMenu( void )
{
    ui->menusp--;

    if ( ui->menusp < 0 ) {
        N_Error( ERR_DROP, "UI_PopMenu: menu stack underflow" );
    }

    if ( ui->menusp ) {
        ui->activemenu = ui->stack[ui->menusp - 1];
    }
    else {
        UI_ForceMenuOff();
    }
}

void UI_ForceMenuOff( void )
{
    ui->menusp = 0;
    ui->activemenu = NULL;

    Key_SetCatcher( Key_GetCatcher() & ~KEYCATCH_UI );
    Key_ClearStates();
}

/*
=================
UI_LerpColor
=================
*/
void UI_LerpColor( vec4_t a, vec4_t b, vec4_t c, float t )
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

qboolean UI_IsFullscreen( void ) {
	if ( ui->activemenu && ( Key_GetCatcher() & KEYCATCH_UI ) ) {
		return ui->activemenu->fullscreen;
	}

	return qfalse;
}

void UI_SetActiveMenu( uiMenu_t menu )
{
	// this should be the ONLY way the menu system is brought up
	// enusure minumum menu data is cached
	Menu_Cache();

	ui->menustate = menu;

	switch ( menu ) {
	case UI_MENU_NONE:
		FontCache()->SetActiveFont( RobotoMono );
		UI_ForceMenuOff();
		Key_SetCatcher( Key_GetCatcher() | KEYCATCH_SGAME );
		Cvar_Set( "g_paused", "0" );
		break;
	case UI_MENU_PAUSE:
//		Cvar_Set( "g_paused", "1" );
		UI_PauseMenu();
		break;
	case UI_MENU_MAIN:
		UI_MainMenu();
		break;
	case UI_MENU_DEMO:
		UI_DemoMenu();
		break;
	default:
#ifdef _NOMAD_DEBUG
	    Con_Printf("UI_SetActiveMenu: bad enum %lu\n", menu );
#endif
        break;
	};
}

/*
================
UI_AdjustFrom1024

Adjusted for resolution and screen aspect ratio
================
*/
void UI_AdjustFrom1024( float *x, float *y, float *w, float *h )
{
	// expect valid pointers
	*x *= ui->scale + ui->bias;
	*y *= ui->scale;
	*w *= ui->scale;
	*h *= ui->scale;
}

void UI_DrawNamedPic( float x, float y, float width, float height, const char *picname )
{
	nhandle_t hShader;

	hShader = re.RegisterShader( picname );
	UI_AdjustFrom1024( &x, &y, &width, &height );
	re.DrawImage( x, y, width, height, 0, 0, 1, 1, hShader );
}

void UI_DrawHandlePic( float x, float y, float w, float h, nhandle_t hShader )
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
		s0 = 0;
		s1 = 1;
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
	
	UI_AdjustFrom1024( &x, &y, &w, &h );
	re.DrawImage( x, y, w, h, s0, t0, s1, t1, hShader );
}

/*
* UI_VirtualKeypoard: creates a somewhat similar experience to the Xbox One/PS4 on-screen gamepad keyboard
*/
int UI_VirtualKeyboard( const char *pName, char *pBuffer, size_t nBufSize )
{
	int ret;
	ImVec2 csize, bsize;
	int n, i;
	char *endPtr;
	char key;
	ImGuiID id;

	ret = 0;
	if ( !ui->virtKeyboard.open ) {
		return 0;
	}
	if ( !pName || !pBuffer ) {
		return ret;
	}

	const int windowFlags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar
		| ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoCollapse;

    csize = ImGui::GetContentRegionAvail();
    n = ( csize.y / 5 ); // height / 5 button rows

    ImGuiStyle& style = ImGui::GetStyle();

	FontCache()->SetActiveFont( RobotoMono );

	if ( ui->virtKeyboard.bufTextLen < 0 ) {
		ui->virtKeyboard.bufTextLen = 0;
	}

    if ( ImGui::Begin( pName, NULL, windowFlags ) ) {
		//ImGui::SetWindowSize( ImVec2( ( n * 4 ) + style.WindowPadding.x, n * 5 ) );
		ImGui::SetWindowSize( ImVec2( 800 * ui->scale, ( ( SCREEN_HEIGHT / 2 ) - 32 ) * ui->scale ) );
		ImGui::SetWindowPos( ImVec2( 256 * ui->scale, 380 * ui->scale ) );
    	csize = ImGui::GetContentRegionAvail(); // now inside this child
    	n = ( csize.y / 5 ) - style.ItemSpacing.y; // button size
    	bsize = ImVec2( n, n ); // buttons are square

		ImGui::SetWindowFontScale( ( 1.5f * ImGui::GetFont()->Scale ) );

		if ( Key_IsDown( KEY_PAD0_LEFTTRIGGER ) ) {
			if ( !ui->virtKeyboard.modeToggle ) {
				ui->virtKeyboard.modeToggle = qtrue;
				ui->virtKeyboard.mode = !ui->virtKeyboard.mode;
				ret = 1;
			}
		} else {
			ui->virtKeyboard.modeToggle = qfalse;
		}
		if ( Key_IsDown( KEY_PAD0_LEFTSTICK_CLICK ) ) {
			if ( !ui->virtKeyboard.capsToggle ) {
				ui->virtKeyboard.capsToggle = qtrue;
				ui->virtKeyboard.caps = !ui->virtKeyboard.caps;
				ret = 1;
			}
		} else {
			ui->virtKeyboard.capsToggle = qfalse;
		}
		if ( Key_IsDown( KEY_PAD0_START ) ) {
			if ( !ui->virtKeyboard.doneToggle ) {
				ui->virtKeyboard.doneToggle = qtrue;
				ui->virtKeyboard.open = qfalse;
				ret = 1;
			}
		} else {
			ui->virtKeyboard.doneToggle = qfalse;
		}
		if ( Key_IsDown( KEY_PAD0_X ) ) {
			if ( !ui->virtKeyboard.backspaceToggle ) {
				ui->virtKeyboard.backspaceToggle = qtrue;
				ui->virtKeyboard.pBuffer[ui->virtKeyboard.bufTextLen--] = '\0';
				ret = 1;
			}
		} else {
			ui->virtKeyboard.backspaceToggle = qfalse;
		}
		if ( Key_IsDown( KEY_PAD0_Y ) ) {
			if ( !ui->virtKeyboard.spaceToggle ) {
				ui->virtKeyboard.spaceToggle = qtrue;
				if ( ui->virtKeyboard.bufTextLen < ui->virtKeyboard.bufMaxLen ) {
					ui->virtKeyboard.pBuffer[ui->virtKeyboard.bufTextLen++] = ' ';
					ui->virtKeyboard.pBuffer[ui->virtKeyboard.bufTextLen + 1] = '\0';
				}
				ret = 1;
			}
		} else {
			ui->virtKeyboard.spaceToggle = qfalse;
		}

		bsize = ImVec2( 64, 64 );

//        ImGui::PushStyleVar( ImGuiStyleVar_FrameRounding, 6 );

		switch ( ui->virtKeyboard.mode ) {
		case VIRT_KEYBOARD_ASCII: {
			
			static const char keyboardNumbersAlt[11] = {
				'!', '@', '#', '$', '%', '^', '&', '*', '(', ')'
			};
			static const char keyboardData[4][10] = {
				{ '1', '2', '3', '4', '5', '6', '7', '8', '9', '0' },
				{ 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p' },
				{ 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', '\'' },
				{ 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '?' }
			};
			
			ImGui::Button( "", ImVec2( 128 * ui->scale, bsize.y ) );
			ImGui::SameLine();

			for ( i = 0; i < arraylen( *keyboardData ); i++ ) {
				const char data[2] = { ui->virtKeyboard.caps ? keyboardNumbersAlt[i] : keyboardData[0][i], '\0' };
				if ( ImGui::Button( data, bsize ) ) {
					if ( ui->virtKeyboard.bufTextLen < ui->virtKeyboard.bufMaxLen ) {
						ui->virtKeyboard.pBuffer[ui->virtKeyboard.bufTextLen++] = *data;
						ui->virtKeyboard.pBuffer[ui->virtKeyboard.bufTextLen + 1] = '\0';
					}
					ret = 1;
				}
				ImGui::SameLine();
			}
			ImGui::Button( "", ImVec2( 136 * ui->scale, bsize.y ) );

			ImGui::NewLine();

			if ( ImGui::Button( "Caps (L-Stick Down)", ImVec2( 172 * ui->scale, bsize.y ) ) ) {
				ui->virtKeyboard.caps = !ui->virtKeyboard.caps;
			}
			ImGui::SameLine();

			for ( i = 0; i < arraylen( *keyboardData ); i++ ) {
				const char data[2] = { ui->virtKeyboard.caps ? toupper( keyboardData[1][i] ) : keyboardData[1][i], '\0' };
				if ( ImGui::Button( data, bsize ) ) {
					if ( ui->virtKeyboard.bufTextLen < ui->virtKeyboard.bufMaxLen ) {
						ui->virtKeyboard.pBuffer[ui->virtKeyboard.bufTextLen++] = *data;
						ui->virtKeyboard.pBuffer[ui->virtKeyboard.bufTextLen + 1] = '\0';
					}
					ret = 1;
				}
				ImGui::SameLine();
			}

			ImGui::NewLine();

			if ( ImGui::Button( "Symbols (LT)", ImVec2( 128 * ui->scale, bsize.y ) ) ) {
				ui->virtKeyboard.mode = VIRT_KEYBOARD_SYMBOLS;
				ret = 1;
			}
			ImGui::SameLine();

			for ( i = 0; i < arraylen( *keyboardData ); i++ ) {
				char data[2];
				if ( N_isalpha( keyboardData[2][i] ) && ui->virtKeyboard.caps ) {
					data[0] = toupper( keyboardData[2][i] );
				} else {
					data[0] = keyboardData[2][i];
				}
				data[1] = '\0';
				if ( ImGui::Button( data, bsize ) ) {
					if ( ui->virtKeyboard.bufTextLen < ui->virtKeyboard.bufMaxLen ) {
						ui->virtKeyboard.pBuffer[ui->virtKeyboard.bufTextLen++] = *data;
						ui->virtKeyboard.pBuffer[ui->virtKeyboard.bufTextLen + 1] = '\0';
					}
					ret = 1;
				}
				ImGui::SameLine();
			}

			ImGui::NewLine();

			for ( i = 0; i < arraylen( *keyboardData ); i++ ) {
				char data[2];
				if ( N_isalpha( keyboardData[3][i] ) && ui->virtKeyboard.caps ) {
					data[0] = toupper( keyboardData[3][i] );
				} else {
					data[0] = keyboardData[3][i];
				}
				data[1] = '\0';
				if ( ImGui::Button( data, bsize ) ) {
					if ( ui->virtKeyboard.bufTextLen < ui->virtKeyboard.bufMaxLen ) {
						ui->virtKeyboard.pBuffer[ui->virtKeyboard.bufTextLen++] = *data;
						ui->virtKeyboard.pBuffer[ui->virtKeyboard.bufTextLen + 1] = '\0';
					}
					ret = 1;
				}
				ImGui::SameLine();
			}

			if ( ImGui::Button( "Enter (START)", ImVec2( 128 * ui->scale, bsize.y ) ) ) {
				ret = 1;
				ui->virtKeyboard.open = qfalse;
			}

			ImGui::NewLine();

			const ImVec2 windowSize = ImGui::GetWindowSize();
			if ( ImGui::Button( "Space (Y)", ImVec2( ( windowSize.x / 2 ) - 4, bsize.y ) ) ) {
				if ( ui->virtKeyboard.bufTextLen < ui->virtKeyboard.bufMaxLen ) {
						ui->virtKeyboard.pBuffer[ui->virtKeyboard.bufTextLen++] = ' ';
						ui->virtKeyboard.pBuffer[ui->virtKeyboard.bufTextLen + 1] = '\0';
					}
				ret = 1;
			}
			ImGui::SameLine();
			if ( ImGui::Button( "<- Backspace (X)", ImVec2( ( windowSize.x / 2 ) - 4, bsize.y ) ) ) {
				if ( ui->virtKeyboard.bufTextLen < ui->virtKeyboard.bufMaxLen ) {
						ui->virtKeyboard.pBuffer[ui->virtKeyboard.bufTextLen--] = '\0';
					}
				ret = 1;
			}

			break; }
		case VIRT_KEYBOARD_SYMBOLS: {

			static const char symbolsData[3][8] = {
				{ '!', '@', '#', '$', '%', '^', '&', '*' },
				{ '(', ')', '-', '_', '{', '}', '[', ']' },
				{ '\\', ';', ':', '\"', '/', '=', '+', '|' }
			};

			if ( ImGui::Button( "Tab", bsize ) ) {
				ui->virtKeyboard.pBuffer[ui->virtKeyboard.bufTextLen++] = '\t';
				ui->virtKeyboard.pBuffer[ui->virtKeyboard.bufTextLen + 1] = '\0';
				ret = 1;
			}
			ImGui::SameLine();

			for ( i = 0; i < arraylen( *symbolsData ); i++ ) {
				const char data[2] = { symbolsData[0][i], '\0' };
				if ( ImGui::Button( data, bsize ) ) {
					if ( ui->virtKeyboard.bufTextLen < ui->virtKeyboard.bufMaxLen ) {
						ui->virtKeyboard.pBuffer[ui->virtKeyboard.bufTextLen++] = *data;
						ui->virtKeyboard.pBuffer[ui->virtKeyboard.bufTextLen + 1] = '\0';
					}
					ret = 1;
				}
				ImGui::SameLine();
			}

			if ( ImGui::Button( "Enter (START)", ImVec2( 128 * ui->scale, bsize.y ) ) ) {
				ret = 1;
				ui->virtKeyboard.open = qfalse;
			}

			ImGui::NewLine();

			ImGui::Button( "", ImVec2( 128 * ui->scale, bsize.y ) );
			ImGui::SameLine();

			for ( i = 0; i < arraylen( *symbolsData ); i++ ) {
				const char data[2] = { symbolsData[1][i], '\0' };
				if ( ImGui::Button( data, bsize ) ) {
					if ( ui->virtKeyboard.bufTextLen < ui->virtKeyboard.bufMaxLen ) {
						ui->virtKeyboard.pBuffer[ui->virtKeyboard.bufTextLen++] = *data;
						ui->virtKeyboard.pBuffer[ui->virtKeyboard.bufTextLen + 1] = '\0';
					}
					ret = 1;
				}
				ImGui::SameLine();
			}

			ImGui::Button( "", ImVec2( 136 * ui->scale, bsize.y ) );

			ImGui::NewLine();
			if ( ImGui::Button( "Alpha (LT)", ImVec2( 128 * ui->scale, bsize.y ) ) ) {
				ui->virtKeyboard.mode = VIRT_KEYBOARD_ASCII;
				ret = 1;
			}
			ImGui::SameLine();

			for ( i = 0; i < arraylen( *symbolsData ); i++ ) {
				const char data[2] = { symbolsData[2][i], '\0' };
				if ( ImGui::Button( data, bsize ) ) {
					if ( ui->virtKeyboard.bufTextLen < ui->virtKeyboard.bufMaxLen ) {
						ui->virtKeyboard.pBuffer[ui->virtKeyboard.bufTextLen++] = *data;
						ui->virtKeyboard.pBuffer[ui->virtKeyboard.bufTextLen + 1] = '\0';
					}
					ret = 1;
				}
				ImGui::SameLine();
			}

			ImGui::NewLine();

			const ImVec2 windowSize = ImGui::GetWindowSize();
			if ( ImGui::Button( "Space (Y)", ImVec2( ( windowSize.x / 2 ) - 4, bsize.y ) ) ) {
				if ( ui->virtKeyboard.bufTextLen < ui->virtKeyboard.bufMaxLen ) {
					ui->virtKeyboard.pBuffer[ui->virtKeyboard.bufTextLen++] = ' ';
					ui->virtKeyboard.pBuffer[ui->virtKeyboard.bufTextLen + 1] = '\0';
				}
				ret = 1;
			}
			ImGui::SameLine();
			if ( ImGui::Button( "<- Backspace (X)", ImVec2( ( windowSize.x / 2 ) - 4, bsize.y ) ) ) {
				if ( ui->virtKeyboard.bufTextLen < ui->virtKeyboard.bufMaxLen ) {
					ui->virtKeyboard.pBuffer[ui->virtKeyboard.bufTextLen--] = '\0';
				}
				ret = 1;
			}

			break; }
		};
		ImGui::End();
	}

    return ret;
}

/*
================
UI_FillRect

Coordinates are 1024*768 virtual values
=================
*/
void UI_FillRect( float x, float y, float width, float height, const float *color )
{
	re.SetColor( color );

	UI_AdjustFrom1024( &x, &y, &width, &height );
	re.DrawImage( x, y, width, height, 0, 0, 0, 0, ui->whiteShader );

	re.SetColor( NULL );
}

/*
================
UI_DrawRect

Coordinates are 1024*768 virtual values
=================
*/
void UI_DrawRect( float x, float y, float width, float height, const float *color )
{
	re.SetColor( color );

	UI_AdjustFrom1024( &x, &y, &width, &height );

	re.DrawImage( x, y, width, 1, 0, 0, 0, 0, ui->whiteShader );
	re.DrawImage( x, y, 1, height, 0, 0, 0, 0, ui->whiteShader );
	re.DrawImage( x, y + height - 1, width, 1, 0, 0, 0, 0, ui->whiteShader );
	re.DrawImage( x + width - 1, y, 1, height, 0, 0, 0, 0, ui->whiteShader );

	re.SetColor( NULL );
}

void UI_SetColor( const float *rgba ) {
	re.SetColor( rgba );
}

void UI_DrawTextBox( int x, int y, int width, int lines )
{
//	FillRect( x + BIGCHAR_WIDTH/2, y + BIGCHAR_HEIGHT/2, ( width + 1 ) * BIGCHAR_WIDTH, ( lines + 1 ) * BIGCHAR_HEIGHT, colorBlack );
//	DrawRect( x + BIGCHAR_WIDTH/2, y + BIGCHAR_HEIGHT/2, ( width + 1 ) * BIGCHAR_WIDTH, ( lines + 1 ) * BIGCHAR_HEIGHT, colorWhite );
}

