#include "ui_lib.h"

#define ID_CONFIRM_NO  0
#define ID_CONFIRM_YES 1

typedef struct {
	menuframework_t menu;
	
	menutext_t yes;
	menutext_t no;
	
	const char *question;
	void (*draw)( void );
	void (*action)( qboolean result );
	
	const char **lines;
} confirmMenu_t;

static confirmMenu_t s_confirm;

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
	
	if ( s_confirm.action ) {
		s_confirm.action( result );
	}
}

static void MessageMenu_Draw( void )
{
	int flags;
	int i;
	
	for ( i = 0; s_confirm.lines[i]; i++ ) {
		ImGui::TextUnformatted( s_confirm.lines[i] );
	}
	
	Menu_Draw( &s_confirm.menu );
	
	if ( s_confirm.draw ) {
		s_confirm.draw();
	}
}

static void ConfirmMenu_Draw( void )
{
	Menu_Draw( &s_confirm.menu );

    if ( Key_IsDown( KEY_ENTER ) || Key_IsDown( KEY_Y ) ) {
        ConfirmMenu_Event( &s_confirm.yes, EVENT_ACTIVATED );
    }
    if ( Key_IsDown( KEY_ESCAPE ) || Key_IsDown( KEY_N ) ) {
        ConfirmMenu_Event( &s_confirm.no, EVENT_ACTIVATED );
    }
	
	if ( s_confirm.draw ) {
		s_confirm.draw();
	}
}

void UI_ConfirmMenu( const char *message, void (*draw)( void ), void (*action)( qboolean result ) )
{
	memset( &s_confirm, 0, sizeof( s_confirm ) );
	
	s_confirm.menu.name = "##ConfirmMenu";
	s_confirm.menu.fullscreen = qtrue;
	s_confirm.menu.draw = ConfirmMenu_Draw;
	
	s_confirm.yes.generic.type = MTYPE_TEXT;
	s_confirm.yes.generic.flags = QMF_HIGHLIGHT_IF_FOCUS | QMF_SAMELINE_NEXT;
	s_confirm.yes.generic.eventcallback = ConfirmMenu_Event;
	s_confirm.yes.generic.id = ID_CONFIRM_YES;
	s_confirm.yes.text = "YES";
	s_confirm.yes.color = color_red;
	
	s_confirm.no.generic.type = MTYPE_TEXT;
	s_confirm.no.generic.flags = QMF_HIGHLIGHT_IF_FOCUS;
	s_confirm.no.generic.eventcallback = ConfirmMenu_Event;
	s_confirm.no.generic.id = ID_CONFIRM_YES;
	s_confirm.no.text = "NO";
	s_confirm.no.color = color_red;
	
	Menu_AddItem( &s_confirm.menu, &s_confirm.yes );
	Menu_AddItem( &s_confirm.menu, &s_confirm.no );
	
	UI_PushMenu( &s_confirm.menu );
}
