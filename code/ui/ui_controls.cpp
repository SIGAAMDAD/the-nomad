#include "ui_lib.h"

#define ID_VIDEO        0
#define ID_PERFORMANCE  1
#define ID_AUDIO        2
#define ID_CONTROLS     3
#define ID_GAMEPLAY     4
#define ID_TABLE        5
#define ID_SETDEFAULTS  6
#define ID_SAVECONFIG   7

#define ID_MOUSE_SENSITIVITY   0
#define ID_MOUSE_ACCELERATION  1

typedef struct {
    const char *command;
    const char *label;
    int32_t id;
    int32_t defaultBind1;
    int32_t defaultBind2;
    int32_t bind1;
    int32_t bind2;
    
    menutext_t binding;
    menutext_t name;
} bind_t;

typedef struct {
    bind_t *keybinds;
    bind_t *rebindKey;
    int numBinds;
    
    qboolean waitingforkey;
    qboolean rebinding;

    menuframework_t menu;

    menutext_t video;
    menutext_t performance;
    menutext_t audio;
    menutext_t controls;
    menutext_t gameplay;

    menutab_t tabs;
    menubutton_t save;
    menubutton_t setDefaults;

    menutext_t mouseSensitivity;
    menutext_t mouseAcceleration;

    menutext_t bindText;

    menuarrow_t mouseSensitivityLeft;
    menuarrow_t mouseSensitivityRight;
    menuslider_t mouseSensitivitySlider;
    menuswitch_t mouseAccelerationButton;

    menucustom_t empty;

    menutable_t mouseTable;
    menutable_t keybindTable;
} controlsOptionsInfo_t;

static controlsOptionsInfo_t *s_controlsOptionsInfo;

const char *Hunk_CopyString( const char *str, ha_pref pref ) {
    char *out;
    uint64_t len;

    len = strlen( str ) + 1;
    out = (char *)Hunk_Alloc( len, pref );
    N_strncpyz( out, str, len );

    return out;
}

void UI_SettingsWriteBinds_f( void )
{
    fileHandle_t f;
    uint32_t i;

    f = FS_FOpenWrite( "bindings.cfg" );
    if ( f == FS_INVALID_HANDLE ) {
        N_Error( ERR_FATAL, "UI_SettingsWriteBinds_f: failed to write bindings" );
    }

    for ( i = 0; i < s_controlsOptionsInfo->numBinds; i++ ) {
        FS_Printf( f, "\"%s\" \"%s\" %i \"%s\" \"%s\" \"%s\" \"%s\"" GDR_NEWLINE,
            s_controlsOptionsInfo->keybinds[i].command,
            s_controlsOptionsInfo->keybinds[i].label,
            s_controlsOptionsInfo->keybinds[i].id,
            Key_KeynumToString( s_controlsOptionsInfo->keybinds[i].defaultBind1 ),
            Key_KeynumToString( s_controlsOptionsInfo->keybinds[i].defaultBind2 ),
            Key_KeynumToString( s_controlsOptionsInfo->keybinds[i].bind1 ),
            Key_KeynumToString( s_controlsOptionsInfo->keybinds[i].bind2 )
        );
    }

    FS_FClose( f );
}

static void SettingsMenu_LoadBindings( void )
{
    union {
        void *v;
        char *b;
    } f;
    const char **text, *text_p, *tok;
    bind_t *bind;
    uint64_t i;

    FS_LoadFile( "bindings.cfg", &f.v );
    if ( !f.v ) {
        N_Error( ERR_DROP, "SettingsMenu_Cache: no bindings file" );
    }

    text_p = f.b;
    text = &text_p;

    COM_BeginParseSession( "bindings.cfg" );

    s_controlsOptionsInfo->numBinds = 0;
    while ( 1 ) {
        tok = COM_ParseExt( text, qtrue );
        if ( !tok || !tok[0] ) {
            break;
        }

        tok = COM_ParseExt( text, qfalse );
        if ( !tok[0] ) {
            COM_ParseError( "missing parameter for keybind 'label'" );
            break;
        }

        tok = COM_ParseExt( text, qfalse );
        if ( !tok[0] ) {
            COM_ParseError( "missing paramter for keybind 'id'" );
            break;
        }

        tok = COM_ParseExt( text, qfalse );
        if ( !tok[0] ) {
            COM_ParseError( "missing paramter for keybind 'defaultBinding1'" );
            break;
        }

        tok = COM_ParseExt( text, qfalse );
        if ( !tok[0] ) {
            COM_ParseError( "missing paramter for keybind 'defaultBinding2'" );
            break;
        }

        tok = COM_ParseExt( text, qfalse );
        if ( !tok[0] ) {
            COM_ParseError( "missing paramter for keybind 'bind1'" );
            break;
        }

        tok = COM_ParseExt( text, qfalse );
        if ( !tok[0] ) {
            COM_ParseError( "missing paramter for keybind 'bind2'" );
            break;
        }

        s_controlsOptionsInfo->numBinds++;
    }

    s_controlsOptionsInfo->keybinds = (bind_t *)Hunk_Alloc( sizeof( *s_controlsOptionsInfo->keybinds ) * s_controlsOptionsInfo->numBinds, h_high );
    text_p = f.b;
    text = &text_p;

    bind = s_controlsOptionsInfo->keybinds;
    for ( i = 0; i < s_controlsOptionsInfo->numBinds; i++ ) {
        tok = COM_ParseExt( text, qtrue );
        if ( !tok || !tok[0] ) {
            break;
        }
        bind->command = Hunk_CopyString( tok, h_high );

        tok = COM_ParseExt( text, qfalse );
        bind->label = Hunk_CopyString( tok, h_high );

        tok = COM_ParseExt( text, qfalse );
        bind->id = atoi( tok );

        tok = COM_ParseExt( text, qfalse );
        if ( tok[0] != '-' && tok[1] != '1' ) {
            bind->defaultBind1 = Key_StringToKeynum( tok );
        } else {
            bind->defaultBind1 = -1;
        }

        tok = COM_ParseExt( text, qfalse );
        if ( tok[0] != '-' && tok[1] != '1' ) {
            bind->defaultBind2 = Key_StringToKeynum( tok );
        } else {
            bind->defaultBind2 = -1;
        }

        tok = COM_ParseExt( text, qfalse );
        if ( tok[0] != '-' && tok[1] != '1' ) {
            bind->bind1 = Key_StringToKeynum( tok );
        } else {
            bind->bind1 = -1;
        }

        tok = COM_ParseExt( text, qfalse );
        if ( tok[0] != '-' && tok[1] != '1' ) {
            bind->bind2 = Key_StringToKeynum( tok );
        } else {
            bind->bind2 = -1;
        }

        if ( bind->defaultBind1 != -1 ) {
            bind->bind1 = bind->defaultBind1;
        }
        if ( bind->defaultBind2 != -1 ) {
            bind->bind2 = bind->defaultBind2;
        }

        Con_Printf( "Added keybind \"%s\": \"%s\"\n", bind->command, bind->label );

        bind++;
    }

    FS_FreeFile( f.v );
}

static void ControlsMenu_DrawKey( void *self )
{
	char bind[MAX_STRING_CHARS];
	char bind2[MAX_STRING_CHARS];
	bind_t *keybind;
	
	keybind = &s_controlsOptionsInfo->keybinds[ ( (menutext_t *)self )->generic.id ];
	
	if ( keybind->bind1 == -1 ) {
		N_strncpyz( bind, "???", sizeof( bind ) );
	}
	else {
		N_strncpyz( bind, Key_KeynumToString( keybind->bind1 ), sizeof( bind ) );
		N_strupr( bind );
		
		if ( keybind->bind2 != -1 ) {
			N_strncpyz( bind2, Key_KeynumToString( keybind->bind2 ), sizeof( bind2 ) );
			N_strupr( bind2 );
			
			N_strcat( bind, sizeof( bind ) - 1, " or " );
			N_strcat( bind, sizeof( bind ) - 1, bind2 );
		}
	}
	ImGui::TextUnformatted( bind );
}

static void ControlsMenu_ConfirmOverwrite( qboolean result )
{
	if ( !result ) {
		s_controlsOptionsInfo->rebinding = qfalse;
		Snd_PlaySfx( ui->sfx_select );
	} else {
		
	}
}

static void ControlsMenu_Rebind( void )
{
	int i;
    const char *binding;
	
	ImGui::Begin( "##RebindPopup", NULL, MENU_DEFAULT_FLAGS & ~( ImGuiWindowFlags_NoBackground ) );
	ImGui::SetWindowFocus();
	ImGui::SetWindowPos( ImVec2( ui->gpuConfig.vidWidth * 0.5f, ui->gpuConfig.vidHeight * 0.5f ) );
	ImGui::TextUnformatted( "Press Any Key..." );
	
	for ( i = 0; i < NUMKEYS; i++ ) {
        if ( Key_IsDown( i ) ) {
            s_controlsOptionsInfo->rebinding = qfalse;
            binding = Key_GetBinding( i );

            if ( binding != NULL ) {
                if ( s_controlsOptionsInfo->keybinds[ Key_GetKey( binding ) ].bind1 != -1 ) {
                	// we're overwriting a binding, warn them
                	UI_ConfirmMenu( "WARNING: You are overwriting another binding, are you sure you want to do this?\n",
                		NULL, ControlsMenu_ConfirmOverwrite );
                	ImGui::End();
                	return;
                }
            }

            if ( s_controlsOptionsInfo->rebindKey->bind1 != -1 ) {
                Con_Printf( "setting double-binding for key \"%s\".\n",
                    Key_GetBinding( s_controlsOptionsInfo->rebindKey->bind1 ) );
                
                s_controlsOptionsInfo->rebindKey->bind2 = i;
            } else {
            	s_controlsOptionsInfo->rebindKey->bind1 = i;
            }
            Cbuf_ExecuteText( EXEC_APPEND, va( "bind %s \"%s\"\n",
                Key_KeynumToString( i ),
                s_controlsOptionsInfo->rebindKey->command ) );
            
            s_controlsOptionsInfo->rebinding = qfalse;
        }
	}
	
	ImGui::End();
}

static void ControlsMenu_Draw( void )
{
	Menu_Draw( &s_controlsOptionsInfo->menu );
	
	if ( s_controlsOptionsInfo->rebinding ) {
	}
}

static void ControlsMenu_RebindEvent( void *ptr, int event )	
{
	uint64_t i;
	bind_t *keybind;
	
	if ( event != EVENT_ACTIVATED ) {
		return;
	}
	
	s_controlsOptionsInfo->rebinding = qtrue;
	s_controlsOptionsInfo->rebindKey = (bind_t *)ptr;
}

static void Controls_MenuEvent( void *ptr, int event )
{
	if ( event != EVENT_ACTIVATED ) {
		return;
	}
	
	switch ( ( (menucommon_t *)ptr )->id ) {
	case ID_SETDEFAULTS:
		
		break;
	case ID_SAVECONFIG:
        Cvar_Set( "g_mouseAcceleration", "" );
		UI_SettingsWriteBinds_f();
		break;
	default:
		break;
	};
}

static void ControlsSettingsMenu_EventCallback( void *ptr, int event )
{
    if ( event != EVENT_ACTIVATED ) {
        return;
    }

    switch ( ( (menucommon_t *)ptr )->id ) {
    case ID_VIDEO:
        UI_PopMenu();
        UI_VideoSettingsMenu();
        break;
    case ID_PERFORMANCE:
        UI_PopMenu();
        UI_PerformanceSettingsMenu();
        break;
    case ID_AUDIO:
        UI_PopMenu();
        UI_AudioSettingsMenu();
        break;
    case ID_CONTROLS:
        break;
    case ID_GAMEPLAY:
        UI_PopMenu();
        UI_GameplaySettingsMenu();
        break;
    default:
        break;
    };
}

void ControlsSettingsMenu_Save( void )
{
}

void ControlsSettingsMenu_SetDefaults( void )
{
}

void ControlsSettingsMenu_Cache( void )
{
	uint64_t i;
	int maxBinds;
	
    if ( !ui->uiAllocated ) {
        s_controlsOptionsInfo = (controlsOptionsInfo_t *)Hunk_Alloc( sizeof( *s_controlsOptionsInfo ), h_high );
    }
    memset( s_controlsOptionsInfo, 0, sizeof( *s_controlsOptionsInfo ) );

    s_controlsOptionsInfo->video.generic.type = MTYPE_TEXT;
    s_controlsOptionsInfo->video.generic.id = ID_VIDEO;
    s_controlsOptionsInfo->video.generic.eventcallback = ControlsSettingsMenu_EventCallback;
    s_controlsOptionsInfo->video.generic.font = AlegreyaSC;
    s_controlsOptionsInfo->video.text = "Video##VideoSettingsMenuTabBar";
    s_controlsOptionsInfo->video.color = color_white;
    
    s_controlsOptionsInfo->performance.generic.type = MTYPE_TEXT;
    s_controlsOptionsInfo->performance.generic.id = ID_PERFORMANCE;
    s_controlsOptionsInfo->performance.generic.eventcallback = ControlsSettingsMenu_EventCallback;
    s_controlsOptionsInfo->performance.generic.font = AlegreyaSC;
    s_controlsOptionsInfo->performance.text = "Performance##PerformanceSettingsMenuTabBar";
    s_controlsOptionsInfo->performance.color = color_white;
    
    s_controlsOptionsInfo->audio.generic.type = MTYPE_TEXT;
    s_controlsOptionsInfo->audio.generic.id = ID_AUDIO;
    s_controlsOptionsInfo->audio.generic.eventcallback = ControlsSettingsMenu_EventCallback;
    s_controlsOptionsInfo->audio.generic.font = AlegreyaSC;
    s_controlsOptionsInfo->audio.text = "Audio##AudioSettingsMenuTabBar";
    s_controlsOptionsInfo->audio.color = color_white;
    
    s_controlsOptionsInfo->controls.generic.type = MTYPE_TEXT;
    s_controlsOptionsInfo->controls.generic.id = ID_CONTROLS;
    s_controlsOptionsInfo->controls.generic.eventcallback = ControlsSettingsMenu_EventCallback;
    s_controlsOptionsInfo->controls.generic.font = AlegreyaSC;
    s_controlsOptionsInfo->controls.text = "Controls##ControlsSettingsMenuTabBar";
    s_controlsOptionsInfo->controls.color = color_white;
    
    s_controlsOptionsInfo->gameplay.generic.type = MTYPE_TEXT;
    s_controlsOptionsInfo->gameplay.generic.id = ID_GAMEPLAY;
    s_controlsOptionsInfo->gameplay.generic.eventcallback = ControlsSettingsMenu_EventCallback;
    s_controlsOptionsInfo->gameplay.generic.font = AlegreyaSC;
    s_controlsOptionsInfo->gameplay.text = "Gameplay##GameplaySettingsMenuTabBar";
    s_controlsOptionsInfo->gameplay.color = color_white;

    s_controlsOptionsInfo->menu.fullscreen = qtrue;
    s_controlsOptionsInfo->menu.width = ui->gpuConfig.vidWidth;
    s_controlsOptionsInfo->menu.height = ui->gpuConfig.vidHeight;
    s_controlsOptionsInfo->menu.x = 0;
    s_controlsOptionsInfo->menu.y = 0;
    s_controlsOptionsInfo->menu.name = "Controls";
    s_controlsOptionsInfo->menu.flags = MENU_DEFAULT_FLAGS;
    s_controlsOptionsInfo->menu.titleFontScale = 3.5f;
    s_controlsOptionsInfo->menu.textFontScale = 1.5f;

    s_controlsOptionsInfo->tabs.generic.type = MTYPE_TAB;
	s_controlsOptionsInfo->tabs.generic.font = FontCache()->AddFontToCache( "AlegreyaSC", "Bold" );
	s_controlsOptionsInfo->tabs.numitems = ID_TABLE;
	s_controlsOptionsInfo->tabs.items[0] = (menucommon_t *)&s_controlsOptionsInfo->video;
	s_controlsOptionsInfo->tabs.items[1] = (menucommon_t *)&s_controlsOptionsInfo->performance;
	s_controlsOptionsInfo->tabs.items[2] = (menucommon_t *)&s_controlsOptionsInfo->audio;
	s_controlsOptionsInfo->tabs.items[3] = (menucommon_t *)&s_controlsOptionsInfo->controls;
	s_controlsOptionsInfo->tabs.items[4] = (menucommon_t *)&s_controlsOptionsInfo->gameplay;
    
    Con_Printf( "Loading control bindings...\n" );
    SettingsMenu_LoadBindings();
    
    if ( s_controlsOptionsInfo->numBinds >= ( MAX_TABLE_ITEMS / 2 ) ) {
    	Con_Printf( COLOR_YELLOW "WARNING: too many keybinds, maximum is %i\n", (int)( MAX_TABLE_ITEMS / 2 ) );
    }
    
    // init menu widgets
    for ( i = 0; i < s_controlsOptionsInfo->numBinds; i++ ) {
    	s_controlsOptionsInfo->keybinds[i].binding.generic.type = MTYPE_TEXT;
    	s_controlsOptionsInfo->keybinds[i].binding.generic.id = i;
    	s_controlsOptionsInfo->keybinds[i].binding.generic.flags = QMF_HIGHLIGHT_IF_FOCUS | QMF_OWNERDRAW;
    	s_controlsOptionsInfo->keybinds[i].binding.generic.ownerdraw = ControlsMenu_DrawKey;
    	s_controlsOptionsInfo->keybinds[i].binding.generic.eventcallback = ControlsMenu_RebindEvent;
    	s_controlsOptionsInfo->keybinds[i].binding.color = color_white;
    	
    	s_controlsOptionsInfo->keybinds[i].name.generic.type = MTYPE_TEXT;
    	s_controlsOptionsInfo->keybinds[i].name.generic.id = i;
    	s_controlsOptionsInfo->keybinds[i].name.generic.flags = QMF_HIGHLIGHT_IF_FOCUS;
    	s_controlsOptionsInfo->keybinds[i].name.color = color_white;
    }

    s_controlsOptionsInfo->mouseSensitivity.generic.type = MTYPE_TEXT;
    s_controlsOptionsInfo->mouseSensitivity.generic.id = ID_MOUSE_SENSITIVITY;
    s_controlsOptionsInfo->mouseSensitivity.generic.eventcallback = ControlsSettingsMenu_EventCallback;
    s_controlsOptionsInfo->mouseSensitivity.generic.flags = QMF_HIGHLIGHT_IF_FOCUS;
    s_controlsOptionsInfo->mouseSensitivity.generic.font = FontCache()->AddFontToCache( "AlegreyaSC", "Bold" );
    s_controlsOptionsInfo->mouseSensitivity.text = "Mouse Sensitivity";
    s_controlsOptionsInfo->mouseSensitivity.color = color_white;

    s_controlsOptionsInfo->mouseSensitivityLeft.generic.type = MTYPE_ARROW;
    s_controlsOptionsInfo->mouseSensitivityLeft.generic.id = ID_MOUSE_SENSITIVITY;
    s_controlsOptionsInfo->mouseSensitivityLeft.generic.eventcallback = MenuEvent_ArrowLeft;
    s_controlsOptionsInfo->mouseSensitivityLeft.generic.name = "##MouseSensitivitySettingsConfigLeft";
    s_controlsOptionsInfo->mouseSensitivityLeft.generic.flags = QMF_HIGHLIGHT_IF_FOCUS;
    s_controlsOptionsInfo->mouseSensitivityLeft.generic.font = FontCache()->AddFontToCache( "AlegreyaSC", "Bold" );
    s_controlsOptionsInfo->mouseSensitivityLeft.direction = ImGuiDir_Left;

    s_controlsOptionsInfo->mouseSensitivityRight.generic.type = MTYPE_ARROW;
    s_controlsOptionsInfo->mouseSensitivityRight.generic.id = ID_MOUSE_SENSITIVITY;
    s_controlsOptionsInfo->mouseSensitivityRight.generic.eventcallback = MenuEvent_ArrowRight;
    s_controlsOptionsInfo->mouseSensitivityRight.generic.name = "##MouseSensitivitySettingsConfigRight";
    s_controlsOptionsInfo->mouseSensitivityRight.generic.flags = QMF_HIGHLIGHT_IF_FOCUS;
    s_controlsOptionsInfo->mouseSensitivityRight.generic.font = FontCache()->AddFontToCache( "AlegreyaSC", "Bold" );
    s_controlsOptionsInfo->mouseSensitivityRight.direction = ImGuiDir_Right;

    s_controlsOptionsInfo->mouseSensitivitySlider.generic.type = MTYPE_SLIDER;
    s_controlsOptionsInfo->mouseSensitivitySlider.generic.id = ID_MOUSE_SENSITIVITY;
    s_controlsOptionsInfo->mouseSensitivitySlider.generic.eventcallback = ControlsSettingsMenu_EventCallback;
    s_controlsOptionsInfo->mouseSensitivitySlider.generic.name = "##MouseSensitivitySettingsConfigSlider";
    s_controlsOptionsInfo->mouseSensitivitySlider.generic.type = MTYPE_SLIDER;
    s_controlsOptionsInfo->mouseSensitivitySlider.isIntegral = qfalse;
    s_controlsOptionsInfo->mouseSensitivitySlider.minvalue = 1.0f;
    s_controlsOptionsInfo->mouseSensitivitySlider.maxvalue = 100.0f;
    s_controlsOptionsInfo->mouseSensitivitySlider.curvalue = Cvar_VariableFloat( "g_mouseSensitivity" );

    s_controlsOptionsInfo->mouseAcceleration.generic.type = MTYPE_TEXT;
    s_controlsOptionsInfo->mouseAcceleration.generic.id = ID_MOUSE_ACCELERATION;
    s_controlsOptionsInfo->mouseAcceleration.generic.eventcallback = ControlsSettingsMenu_EventCallback;
    s_controlsOptionsInfo->mouseAcceleration.generic.flags = QMF_HIGHLIGHT_IF_FOCUS;
    s_controlsOptionsInfo->mouseAcceleration.generic.font = FontCache()->AddFontToCache( "AlegreyaSC", "Bold" );
    s_controlsOptionsInfo->mouseAcceleration.text = "Mouse Acceleration";
    s_controlsOptionsInfo->mouseAcceleration.color = color_white;

    s_controlsOptionsInfo->mouseAccelerationButton.generic.type = MTYPE_RADIOBUTTON;
    s_controlsOptionsInfo->mouseAccelerationButton.generic.id = ID_MOUSE_ACCELERATION;
    s_controlsOptionsInfo->mouseAccelerationButton.generic.eventcallback = ControlsSettingsMenu_EventCallback;
    s_controlsOptionsInfo->mouseAccelerationButton.generic.name = "##MouseAccelerationSettingsConfigButton";
    s_controlsOptionsInfo->mouseAccelerationButton.generic.font = FontCache()->AddFontToCache( "AlegreyaSC", "Bold" );
    s_controlsOptionsInfo->mouseAccelerationButton.curvalue = Cvar_VariableInteger( "g_mouseAccelerate" );
    s_controlsOptionsInfo->mouseAccelerationButton.color = color_white;

    Menu_AddItem( &s_controlsOptionsInfo->menu, &s_controlsOptionsInfo->tabs );
    
    Menu_AddItem( &s_controlsOptionsInfo->menu, &s_controlsOptionsInfo->mouseTable );

    Table_AddRow( &s_controlsOptionsInfo->mouseTable );
    Table_AddItem( &s_controlsOptionsInfo->mouseTable, &s_controlsOptionsInfo->mouseSensitivity );
    Table_AddItem( &s_controlsOptionsInfo->mouseTable, &s_controlsOptionsInfo->mouseSensitivityLeft );
    Table_AddItem( &s_controlsOptionsInfo->mouseTable, &s_controlsOptionsInfo->mouseSensitivitySlider );
    Table_AddItem( &s_controlsOptionsInfo->mouseTable, &s_controlsOptionsInfo->mouseSensitivityRight );

    Table_AddRow( &s_controlsOptionsInfo->mouseTable );
    Table_AddItem( &s_controlsOptionsInfo->mouseTable, &s_controlsOptionsInfo->mouseAcceleration );
    Table_AddItem( &s_controlsOptionsInfo->mouseTable, &s_controlsOptionsInfo->empty );
    Table_AddItem( &s_controlsOptionsInfo->mouseTable, &s_controlsOptionsInfo->mouseAccelerationButton );
    Table_AddItem( &s_controlsOptionsInfo->mouseTable, &s_controlsOptionsInfo->empty );

    Menu_AddItem( &s_controlsOptionsInfo->menu, &s_controlsOptionsInfo->keybindTable );
	
	maxBinds = MAX_TABLE_ITEMS / 2;
	for ( i = 0; i < maxBinds; i++ ) {
		Table_AddRow( &s_controlsOptionsInfo->keybindTable );
		Table_AddItem( &s_controlsOptionsInfo->keybindTable, &s_controlsOptionsInfo->keybinds[i].name );
		Table_AddItem( &s_controlsOptionsInfo->keybindTable, &s_controlsOptionsInfo->keybinds[i].binding );
	}
	
	Menu_AddItem( &s_controlsOptionsInfo->menu, &s_controlsOptionsInfo->setDefaults );
	Menu_AddItem( &s_controlsOptionsInfo->menu, &s_controlsOptionsInfo->save );
}

void UI_ControlsSettingsMenu( void )
{
    UI_PushMenu( &s_controlsOptionsInfo->menu );
}