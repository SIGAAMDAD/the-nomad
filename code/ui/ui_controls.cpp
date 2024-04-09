#include "ui_lib.h"

#define ID_VIDEO        0
#define ID_PERFORMANCE  1
#define ID_AUDIO        2
#define ID_CONTROLS     3
#define ID_GAMEPLAY     4
#define ID_TABLE        5

typedef struct {
    const char *command;
    const char *label;
    int32_t id;
    int32_t defaultBind1;
    int32_t defaultBind2;
    int32_t bind1;
    int32_t bind2;
} bind_t;

typedef struct {
    const char *name;

} configcvar_t;

typedef struct {
    bind_t *keybinds;
    int numBinds;

    menuframework_t menu;

    menutext_t video;
    menutext_t performance;
    menutext_t audio;
    menutext_t controls;
    menutext_t gameplay;

    menutab_t tabs;
    menutable_t table;
} controlsOptionsInfo_t;

static controlsOptionsInfo_t s_controlsOptionsInfo;

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

    for ( i = 0; i < s_controlsOptionsInfo.numBinds; i++ ) {
        FS_Printf( f, "\"%s\" \"%s\" %i \"%s\" \"%s\" \"%s\" \"%s\"" GDR_NEWLINE,
            s_controlsOptionsInfo.keybinds[i].command,
            s_controlsOptionsInfo.keybinds[i].label,
            s_controlsOptionsInfo.keybinds[i].id,
            Key_KeynumToString( s_controlsOptionsInfo.keybinds[i].defaultBind1 ),
            Key_KeynumToString( s_controlsOptionsInfo.keybinds[i].defaultBind2 ),
            Key_KeynumToString( s_controlsOptionsInfo.keybinds[i].bind1 ),
            Key_KeynumToString( s_controlsOptionsInfo.keybinds[i].bind2 )
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

    s_controlsOptionsInfo.numBinds = 0;
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

        s_controlsOptionsInfo.numBinds++;
    }

    s_controlsOptionsInfo.keybinds = (bind_t *)Hunk_Alloc( sizeof( *s_controlsOptionsInfo.keybinds ) * s_controlsOptionsInfo.numBinds, h_high );
    text_p = f.b;
    text = &text_p;

    bind = s_controlsOptionsInfo.keybinds;
    for ( i = 0; i < s_controlsOptionsInfo.numBinds; i++ ) {
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

void ControlsSettingsMenu_Cache( void )
{
    memset( &s_controlsOptionsInfo, 0, sizeof( s_controlsOptionsInfo ) );

    s_controlsOptionsInfo.menu.fullscreen = qtrue;
    s_controlsOptionsInfo.menu.width = ui->gpuConfig.vidWidth;
    s_controlsOptionsInfo.menu.height = ui->gpuConfig.vidHeight;
    s_controlsOptionsInfo.menu.x = 0;
    s_controlsOptionsInfo.menu.y = 0;
    s_controlsOptionsInfo.menu.name = "Controls##SettingsMenu";

    s_controlsOptionsInfo.tabs.generic.type = MTYPE_TAB;
    s_controlsOptionsInfo.tabs.generic.name = "##SettingsMenuTabs";
    s_controlsOptionsInfo.tabs.generic.eventcallback = ControlsSettingsMenu_EventCallback;
    s_controlsOptionsInfo.tabs.numitems = ID_TABLE;
    s_controlsOptionsInfo.tabs.items[0] = (menucommon_t *)&s_controlsOptionsInfo.video;
    s_controlsOptionsInfo.tabs.items[1] = (menucommon_t *)&s_controlsOptionsInfo.performance;
    s_controlsOptionsInfo.tabs.items[2] = (menucommon_t *)&s_controlsOptionsInfo.audio;
    s_controlsOptionsInfo.tabs.items[3] = (menucommon_t *)&s_controlsOptionsInfo.controls;
    s_controlsOptionsInfo.tabs.items[4] = (menucommon_t *)&s_controlsOptionsInfo.gameplay;
}

void UI_ControlsSettingsMenu( void )
{
    UI_PushMenu( &s_controlsOptionsInfo.menu );
}
