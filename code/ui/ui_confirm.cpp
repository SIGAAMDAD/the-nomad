#include "ui_lib.h"

#define ID_CONFIRM_NO  0
#define ID_CONFIRM_YES 1

typedef struct {
	menuframework_t menu;
	
	menutext_t yes;
	menutext_t no;
	menutext_t question;
	
	void (*draw)( void );
	void (*action)( qboolean result );
	
	const char **lines;
} confirmMenu_t;

static confirmMenu_t *s_confirm;

extern void Menu_DrawItemList( void **items, int numitems );

static void ConfirmMenu_Event( void *ptr, int event )
{
	qboolean result;
	
	if ( event != EVENT_ACTIVATED ) {
		return;
	}
	
	UI_PopMenu();
	
	if ( ( (menucommon_t *)ptr )->id == ID_CONFIRM_NO ) {
		result = qfalse;
	} else {
		result = qtrue;
	}
	
	if ( s_confirm->action ) {
		s_confirm->action( result );
	}
}

static void MessageMenu_Draw( void )
{
	int flags;
	int i;
	
	ImGui::Begin( "##ConfirmMenuMainMenu", NULL, s_confirm->menu.flags );
	ImGui::SetWindowFocus();
	ImGui::SetWindowPos( ImVec2( s_confirm->menu.x, s_confirm->menu.y ) );
	ImGui::SetWindowSize( ImVec2( s_confirm->menu.width, s_confirm->menu.height ) );

	UI_EscapeMenuToggle();
	if ( UI_MenuTitle( s_confirm->menu.name, s_confirm->menu.titleFontScale ) ) {
		UI_PopMenu();
		Snd_PlaySfx( ui->sfx_back );

		ImGui::End();
		return;
	}

	for ( i = 0; s_confirm->lines[i]; i++ ) {
		ImGui::TextUnformatted( s_confirm->lines[i] );
	}

	if ( s_confirm->draw ) {
		s_confirm->draw();
	}

	ImGui::End();
}

static void ConfirmMenu_Draw( void )
{
	menuframework_t *menu;

	menu = &s_confirm->menu;

	ImGui::Begin( va( "%s##%sMainMenu", menu->name, menu->name ), NULL, menu->flags );
	if ( !( Key_GetCatcher() & KEYCATCH_CONSOLE ) ) {
		ImGui::SetWindowFocus();
	}
	ImGui::SetWindowPos( ImVec2( menu->x, menu->y ) );
	ImGui::SetWindowSize( ImVec2( menu->width, menu->height ) );

	UI_EscapeMenuToggle();
	if ( UI_MenuTitle( menu->name, menu->titleFontScale ) ) {
		UI_PopMenu();
		Snd_PlaySfx( ui->sfx_back );

		ImGui::End();
		return;
	}

	if ( s_confirm->draw ) {
		s_confirm->draw();
	}

	{
		menutext_t *text = &s_confirm->question;

		qboolean colorChanged;

		ImGui::PushStyleColor( ImGuiCol_Button, ImVec4( 0.0f, 0.0f, 0.0f, 0.0f ) );
		ImGui::PushStyleColor( ImGuiCol_ButtonActive, ImVec4( 0.0f, 0.0f, 0.0f, 0.0f ) );
		ImGui::PushStyleColor( ImGuiCol_ButtonHovered, ImVec4( 0.0f, 0.0f, 0.0f, 0.0f ) );

		if ( text->generic.font ) {
			FontCache()->SetActiveFont( text->generic.font );
		}

		colorChanged = qfalse;
		if ( text->generic.flags & QMF_GRAYED ) {
			ImGui::PushStyleColor( ImGuiCol_Text, colorMdGrey );
			colorChanged = qtrue;
		}
		else if ( !( ( text->generic.flags & QMF_HIGHLIGHT_IF_FOCUS && text->generic.focused )
			|| text->generic.flags & QMF_HIGHLIGHT ) )
		{
			ImGui::PushStyleColor( ImGuiCol_Text, text->color );
			colorChanged = qtrue;
		}
		if ( text->generic.flags & QMF_OWNERDRAW && text->generic.ownerdraw ) {
			text->generic.ownerdraw( text );
		} else {
			ImGui::TextWrapped( text->text ); // for joysticks
		}

		if ( ImGui::IsItemClicked() || ImGui::IsItemActivated() ) {
			if ( !( text->generic.flags & QMF_SILENT ) ) {
				Snd_PlaySfx( ui->sfx_select );
			}
			if ( text->generic.eventcallback ) {
				text->generic.eventcallback( text, EVENT_ACTIVATED );
			}
		}

		if ( colorChanged ) {
			ImGui::PopStyleColor();
		}

		ImGui::PopStyleColor( 3 );
	}
	Menu_DrawItemList( menu->items, menu->nitems );

    if ( Key_IsDown( KEY_ENTER ) || Key_IsDown( KEY_Y ) ) {
        ConfirmMenu_Event( &s_confirm->yes, EVENT_ACTIVATED );
    }
    if ( Key_IsDown( KEY_ESCAPE ) || Key_IsDown( KEY_N ) ) {
        ConfirmMenu_Event( &s_confirm->no, EVENT_ACTIVATED );
    }

	ImGui::End();
}

void ConfirmMenu_Cache( void )
{
	if ( !ui->uiAllocated ) {
		static confirmMenu_t menu;
		s_confirm = &menu;
	}
	memset( s_confirm, 0, sizeof( *s_confirm ) );
}

void UI_ConfirmMenu( const char *question, void (*draw)( void ), void (*action)( qboolean result ) )
{
	int n1, n2, n3;
	int l1, l2, l3;

	ConfirmMenu_Cache();

	n1 = ImGui::CalcTextSize( "YES/NO" ).x;
	n2 = ImGui::CalcTextSize( "YES" ).x + 3;
	n3 = ImGui::CalcTextSize( "/" ).x + 3;
	l1 = 380 - ( n1 / 2 );
	l2 = l1 + n2;
	l3 = l2 + n3;
	
	s_confirm->draw = draw;
	s_confirm->action = action;

	s_confirm->menu.draw = ConfirmMenu_Draw;
	s_confirm->menu.x = 540 - ( n1 * ui->scale );
	s_confirm->menu.y = 280 * ui->scale;
	s_confirm->menu.width = l1 * ui->scale + ui->bias;
	s_confirm->menu.height = 372 * ui->scale;
	s_confirm->menu.textFontScale = 1.0f;
	s_confirm->menu.name = "ConfirmMenu";
	s_confirm->menu.flags = ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar
		| ImGuiWindowFlags_NoCollapse;

	if ( gi.state == GS_LEVEL ) {
		s_confirm->menu.fullscreen = qfalse;
	} else {
		s_confirm->menu.fullscreen = qtrue;
	}

	s_confirm->question.generic.type = MTYPE_TEXT;
	s_confirm->question.text = question;
	s_confirm->question.color = color_white;

	s_confirm->yes.generic.type = MTYPE_TEXT;
	s_confirm->yes.generic.flags = QMF_HIGHLIGHT_IF_FOCUS;
	s_confirm->yes.generic.eventcallback = ConfirmMenu_Event;
	s_confirm->yes.generic.id = ID_CONFIRM_YES;
	s_confirm->yes.text = "YES";
	s_confirm->yes.color = color_red;

	s_confirm->no.generic.type = MTYPE_TEXT;
	s_confirm->no.generic.flags = QMF_HIGHLIGHT_IF_FOCUS;
	s_confirm->no.generic.eventcallback = ConfirmMenu_Event;
	s_confirm->no.generic.id = ID_CONFIRM_NO;
	s_confirm->no.text = "NO";
	s_confirm->no.color = color_red;
	
	Menu_AddItem( &s_confirm->menu, &s_confirm->yes );
	Menu_AddItem( &s_confirm->menu, &s_confirm->no );

	UI_PushMenu( &s_confirm->menu );
}

/*
=================
UI_Message
hacked over from Confirm stuff
=================
*/
void UI_Message( const char **lines )
{
	int length;

	length = 528 - ( ImGui::CalcTextSize( "OK" ).x / 2 );

	ConfirmMenu_Cache();

	s_confirm->lines = lines;

	s_confirm->menu.name = "";
	s_confirm->menu.fullscreen = qtrue;
	s_confirm->menu.width = gi.gpuConfig.vidWidth;
	s_confirm->menu.height = gi.gpuConfig.vidHeight;
	s_confirm->menu.x = length * ui->scale;
	s_confirm->menu.y = 268 * ui->scale;
	s_confirm->menu.draw = MessageMenu_Draw;

	if ( gi.state == GS_LEVEL ) {
		s_confirm->menu.fullscreen = qfalse;
	} else {
		s_confirm->menu.fullscreen = qtrue;
	}
	
	s_confirm->yes.generic.type = MTYPE_TEXT;
	s_confirm->yes.generic.flags = QMF_HIGHLIGHT_IF_FOCUS | QMF_SAMELINE_NEXT;
	s_confirm->yes.generic.eventcallback = ConfirmMenu_Event;
	s_confirm->yes.generic.id = ID_CONFIRM_YES;
	s_confirm->yes.text = "OK";
	s_confirm->yes.color = color_red;
	
	Menu_AddItem( &s_confirm->menu, &s_confirm->yes );
	Menu_AddItem( &s_confirm->menu, &s_confirm->no );
	
	UI_PushMenu( &s_confirm->menu );
}