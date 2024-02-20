#ifndef __UI_LIB__
#define __UI_LIB__

#pragma once

#include "../engine/n_shared.h"
#include "../game/g_game.h"
#include "ui_menu.h"
#include "ui_string_manager.h"
#include "ui_window.h"
#include "ui_font.h"
#include <new>
#include <fstream>

typedef enum : uint64_t
{
    STATE_MAIN,
        STATE_SINGLEPLAYER,
            STATE_NAMEISSUE,
            STATE_NEWGAME,
            STATE_LOADGAME,
            STATE_PLAYMISSION,

    STATE_LEGAL,

    STATE_SETTINGS,
        STATE_GRAPHICS,
        STATE_CONTROLS,
        STATE_AUDIO,
        STATE_GAMEPLAY,
    
    STATE_CREDITS,

    STATE_PAUSE,
        STATE_HELP,
            STATE_HELP_SHOW,
    
    STATE_NONE,
    STATE_ERROR
} menustate_t;

#define MAX_MENU_DEPTH 8

#define RCOLUMN_OFFSET			( BIGCHAR_WIDTH )
#define LCOLUMN_OFFSET			(-BIGCHAR_WIDTH )

#define SLIDER_RANGE			10

#define MTYPE_NULL				0
#define MTYPE_SLIDER			1	
#define MTYPE_ACTION			2
#define MTYPE_SPINCONTROL		3
#define MTYPE_FIELD				4
#define MTYPE_RADIOBUTTON		5
#define MTYPE_BITMAP			6	
#define MTYPE_TEXT				7
#define MTYPE_SCROLLLIST		8
#define MTYPE_PTEXT				9
#define MTYPE_BTEXT				10

#define QMF_BLINK				0x00000001
#define QMF_SMALLFONT			0x00000002
#define QMF_LEFT_JUSTIFY		0x00000004
#define QMF_CENTER_JUSTIFY		0x00000008
#define QMF_RIGHT_JUSTIFY		0x00000010
#define QMF_NUMBERSONLY			0x00000020	// edit field is only numbers
#define QMF_HIGHLIGHT			0x00000040
#define QMF_HIGHLIGHT_IF_FOCUS	0x00000080	// steady focus
#define QMF_PULSEIFFOCUS		0x00000100	// pulse if focus
#define QMF_HASMOUSEFOCUS		0x00000200
#define QMF_NOONOFFTEXT			0x00000400
#define QMF_MOUSEONLY			0x00000800	// only mouse input allowed
#define QMF_HIDDEN				0x00001000	// skips drawing
#define QMF_GRAYED				0x00002000	// grays and disables
#define QMF_INACTIVE			0x00004000	// disables any input
#define QMF_NODEFAULTINIT		0x00008000	// skip default initialization
#define QMF_OWNERDRAW			0x00010000
#define QMF_PULSE				0x00020000
#define QMF_LOWERCASE			0x00040000	// edit field is all lower case
#define QMF_UPPERCASE			0x00080000	// edit field is all upper case
#define QMF_SILENT				0x00100000

// callback notifications
#define EVENT_GOTFOCUS				1
#define EVENT_LOSTFOCUS			    2
#define EVENT_ACTIVATED			    3

#include "ui_menu.h"

class CUILib
{
public:
    CUILib( void ) = default;
    ~CUILib() = default;

    void Init( void );
    void Shutdown( void );
    
    void PushMenu( CUIMenu *menu );
    void PopMenu( void );
    void ForceMenuOff( void );

    void AdjustFrom1024( float *x, float *y, float *w, float *h ) const;
    void DrawNamedPic( float x, float y, float width, float height, const char *picname ) const;
    void DrawHandlePic( float x, float y, float w, float h, nhandle_t hShader ) const;
    void FillRect( float x, float y, float width, float height, const float *color ) const;
    void DrawRect( float x, float y, float width, float height, const float *color ) const;
    void SetColor( const float *rgba ) const;
    void Refresh( uint64_t realtime );
    qboolean CursorInRect( int32_t x, int32_t y, int32_t width, int32_t height ) const;
    void DrawTextBox( int32_t x, int32_t y, int32_t width, int32_t lines ) const;
    void DrawString( const char *str ) const;
    void DrawStringBlink( const char *str, int32_t ticker, int32_t mult ) const;
    void DrawString( int32_t x, int32_t y, const char *str, int32_t style, vec4_t color ) const;
    void DrawMenu( void ) const;
    void DrawChar( int32_t x, int32_t y, int32_t ch, int32_t style, vec4_t color ) const;
    void DrawProportionalString_AutoWrapped( int32_t x, int32_t y, int32_t xmax, int32_t ystep, const char* str, int32_t style, vec4_t color ) const;
    void DrawProportionalString( int32_t x, int32_t y, const char* str, int32_t style, vec4_t color ) const;
    qboolean IsFullscreen( void ) const;
    int32_t ProportionalStringWidth( const char* str ) const;
    float ProportionalSizeScale( int32_t style ) const;
    void DrawBannerString( int32_t x, int32_t y, const char* str, int32_t style, vec4_t color ) const;
    void SetActiveMenu( uiMenu_t menu );

    bool Menu_Option( const char *label );
    bool Menu_Title( const char *label );

    GDR_INLINE menustate_t GetState( void ) const { return state; }
    GDR_INLINE void SetState( menustate_t _state) { state = _state; }
    void EscapeMenuToggle( menustate_t newstate );
    GDR_INLINE void SetFontSize( float size ) const { ImGui::SetWindowFontScale( ImGui::GetFont()->Scale * size * scale ); }

    void PlaySelected( void ) const { Snd_PlaySfx( sfx_select ); }

    void MouseEvent( uint32_t dx, uint32_t dy );
    void KeyEvent( uint32_t key, qboolean down );

    CUIMenu *GetCurrentMenu( void ) {
        return curmenu;
    }
    const CUIMenu *GetCurrentMenu( void ) const {
        return curmenu;
    }

    int32_t GetFrameTime( void ) const {
        return frametime;
    }
    int32_t GetRealTime( void ) const {
        return realtime;
    }

    void SetFrameTime( uint64_t n ) {
        frametime = n;
    }
    void SetRealTime (uint64_t n ) {
        realtime = n;
    }

    qboolean GetFirstDraw( void ) const {
        return firstdraw;
    }
    void SetFirstDraw( qboolean yas ) {
        firstdraw = yas;
    }

    int32_t GetCursorX( void ) const {
        return cursorx;
    }
    int32_t GetCursorY( void ) const {
        return cursory;
    }

    int32_t GetDebug( void ) const {
        return debug;
    }
    int32_t IsDebug( void ) const {
        return debug;
    }
    void SetDebug( int32_t i ) {
        debug = i;
    }

    const gpuConfig_t& GetConfig( void ) const {
        return gpuConfig;
    }

    void ItemHandleSfx( void ) const;

public:
    nhandle_t whiteShader;
    nhandle_t menubackShader;
    nhandle_t charset;
    nhandle_t rb_on;
    nhandle_t rb_off;
    sfxHandle_t sfx_null;

    float scale;
    float bias;
private:
    void DrawString2( int32_t x, int32_t y, const char* str, vec4_t color, int32_t charw, int32_t charh ) const;
    void DrawBannerString2( int32_t x, int32_t y, const char* str, vec4_t color ) const;
    void DrawProportionalString2( int32_t x, int32_t y, const char* str, vec4_t color, float sizeScale, nhandle_t charset ) const;

    CUIMenu *stack[MAX_MENU_DEPTH];
    CUIMenu *curmenu;

    int32_t menusp;
    qboolean firstdraw;
    int32_t debug;

    int32_t cursorx;
    int32_t cursory;

    int32_t frametime;
    int32_t realtime;

    sfxHandle_t sfx_scroll;
    sfxHandle_t sfx_back;
    sfxHandle_t sfx_select;
    qboolean sfx_scroll_toggle;

    gpuConfig_t gpuConfig;
    menustate_t state;
    qboolean escapeToggle;
};

extern CUILib *ui;
extern qboolean m_entersound;

// cvars
extern cvar_t *ui_language;
extern cvar_t *ui_printStrings;
extern cvar_t *ui_active;

extern const char *UI_LangToString( int32_t lang );

extern void			Menu_Cache( void );
extern void			Menu_Focus( CUIMenuWidget *m );
extern void			Menu_AddItem( CUIMenuWidget *menu, void *item );
extern void			Menu_AdjustCursor( CUIMenuWidget *menu, int32_t dir );
extern void			Menu_Draw( CUIMenu *menu );
extern void			*Menu_ItemAtCursor( CUIMenu *m );
extern sfxHandle_t	Menu_ActivateItem( CUIMenuWidget *s, CUIMenuWidget* item );
extern void			Menu_SetCursor( CUIMenu *m, int32_t cursor );
extern void			Menu_SetCursorToItem( CUIMenu  *m, void* ptr );
extern sfxHandle_t	Menu_DefaultKey( CUIMenu *s, uint32_t key );
extern void			Bitmap_Init( mbitmap_t *b );
extern void			Bitmap_Draw( mbitmap_t *b );
extern void			ScrollList_Draw( mlist_t *l );
extern sfxHandle_t	ScrollList_Key( mlist_t *l, uint32_t key );
extern sfxHandle_t	menu_in_sound;
extern sfxHandle_t	menu_move_sound;
extern sfxHandle_t	menu_out_sound;
extern sfxHandle_t	menu_buzz_sound;
extern sfxHandle_t	menu_null_sound;
extern sfxHandle_t	weaponChangeSound;
extern vec4_t		menu_text_color;
extern vec4_t		menu_grayed_color;
extern vec4_t		menu_dark_color;
extern vec4_t		menu_highlight_color;
extern vec4_t		menu_red_color;
extern vec4_t		menu_black_color;
extern vec4_t		menu_dim_color;
extern vec4_t		color_black;
extern vec4_t		color_white;
extern vec4_t		color_yellow;
extern vec4_t		color_blue;
extern vec4_t		color_orange;
extern vec4_t		color_red;
extern vec4_t		color_dim;
extern vec4_t		name_color;
extern vec4_t		list_color;
extern vec4_t		listbar_color;
extern vec4_t		text_color_disabled; 
extern vec4_t		text_color_normal;
extern vec4_t		text_color_highlight;

//
// ui_mfield.cpp
//
extern void			MField_Clear( mfield_t *edit );
extern void			MField_KeyDownEvent( mfield_t *edit, uint32_t key );
extern void			MField_CharEvent( mfield_t *edit, int32_t ch );
extern void			MField_Draw( mfield_t *edit, int32_t x, int32_t y, int32_t style, vec4_t color );
extern void			MenuField_Init( mfield_t *m );
extern void			MenuField_Draw( mfield_t *f );
extern sfxHandle_t	MenuField_Key( mfield_t* m, uint32_t* key );

//
// ui_pause.cpp
//
extern void         UI_PauseMenu( void );
extern void         PauseMenu_Cache( void );

//
// ui_title.cpp
//
extern void         UI_TitleMenu( void );
extern void         TitleMenu_Cache( void );

//
// ui_intro.cpp
//
extern void         UI_IntroMenu( void );
extern void         IntroMenu_Cache( void );

//
// ui_main.cpp
//
extern void         UI_MainMenu( void );
extern void         MainMenu_Cache( void );
extern void         MainMenu_Draw( void );

//
// ui_settings.cpp
//
extern void         UI_SettingsMenu( void );
extern void         SettingsMenu_Cache( void );
extern void         SettingsMenu_Draw( void );

//
// ui_legal.cpp
//
extern void         LegalMenu_Cache( void );
extern void         LegalMenu_Draw( void );

//
// ui_single_player.cpp
//
extern void         UI_SinglePlayerMenu( void );
extern void         SinglePlayerMenu_Cache( void );
extern void         NewGame_DrawNameIssue( void );
extern void         SinglePlayerMenu_Draw( void );

#include "ui_defs.h"

#endif