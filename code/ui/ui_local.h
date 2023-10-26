#ifndef _UI_LOCAL_
#define _UI_LOCAL_

#include "ui_public.h"
#include "../rendercommon/r_types.h"
#include "ui_defs.h"

#define RCOLUMN_OFFSET			( BIGCHAR_WIDTH )
#define LCOLUMN_OFFSET			(-BIGCHAR_WIDTH )

#define SLIDER_RANGE			10

#define MAX_MENUDEPTH			8
#define MAX_MENUITEMS			96

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

#define EVENT_ACTIVATED 0
#define EVENT_DEACTIVATED 1
#define EVENT_GOTFOCUS 2
#define EVENT_LOSTFOCUS 3

typedef struct
{
	int	cursor;
	int cursor_prev;

	int	nitems;
	void *items[MAX_MENUITEMS];

	void (*draw) (void);
	sfxHandle_t (*key) (unsigned int key);

	qboolean	wrapAround;
	qboolean	fullscreen;
} menuframework_t;

typedef struct
{
	int type;
	const char *name;
	int	id;
	int x, y;
	int left;
	int	top;
	int	right;
	int	bottom;
	menuframework_t *parent;
	int menuPosition;
	unsigned flags;

	void (*callback)( void *self, unsigned int event );
	void (*statusbar)( void *self );
	void (*ownerdraw)( void *self );
} menucommon_t;

typedef struct {
	int		cursor;
	int		scroll;
	int		widthInChars;
	char	buffer[MAX_EDIT_LINE];
	int		maxchars;
} mfield_t;

typedef struct
{
	menucommon_t	generic;
	mfield_t		field;
} menufield_t;

typedef struct 
{
	menucommon_t generic;

	float minvalue;
	float maxvalue;
	float curvalue;

	float range;
} menuslider_t;

typedef struct
{
	menucommon_t generic;

	int	oldvalue;
	int curvalue;
	int	numitems;
	int	top;
		
	const char **itemnames;

	int width;
	int height;
	int	columns;
	int	seperation;
} menulist_t;

typedef struct
{
	menucommon_t generic;
} menuaction_t;

typedef struct
{
	menucommon_t generic;
	int curvalue;
} menuradiobutton_t;

typedef struct
{
	menucommon_t	generic;
	char*			focuspic;	
	char*			errorpic;
	nhandle_t		shader;
	nhandle_t		focusshader;
	int				width;
	int				height;
	float*			focuscolor;
} menubitmap_t;

typedef struct
{
	menucommon_t	generic;
	char*			string;
	int				style;
	float*			color;
} menutext_t;

typedef struct {
    nhandle_t arrow_left;
    nhandle_t arrow_right;
    nhandle_t charset;
    nhandle_t cursor;
    nhandle_t whiteShader;
    nhandle_t menuBackShader;
	nhandle_t charsetProp;
	nhandle_t charsetPropB;
	nhandle_t charsetPropGlow;
	nhandle_t rb_on;
	nhandle_t rb_off;
	sfxHandle_t buzz_sound;
    sfxHandle_t null_sound;
    sfxHandle_t move_sound;
    sfxHandle_t in_sound;
	sfxHandle_t out_sound;
    qboolean entersound;

    int cursorx;
    int cursory;
    int frameTime;
    int realTime;
    int stackp;
    float scale;
    float bias;
	uiMenu_t menuIndex;

    qboolean firstdraw;

    gpuConfig_t gpuConfig;

    menuframework_t *curMenu;
    menuframework_t *stack[MAX_MENUDEPTH];
} uiGlobals_t;

extern uiGlobals_t ui;

extern vec4_t pulse_color;
extern vec4_t text_color_disabled; // light gray
extern vec4_t text_color_normal; // light orange
extern vec4_t text_color_highlight; // bright yellow
extern vec4_t listbar_color; // transluscent orange
extern vec4_t text_color_status; // bright white

extern vec4_t menu_text_color;
extern vec4_t menu_dim_color;
extern vec4_t color_black;
extern vec4_t color_white;
extern vec4_t color_yellow;
extern vec4_t color_blue;
extern vec4_t color_lightOrange;
extern vec4_t color_orange;
extern vec4_t color_red;
extern vec4_t color_dim;

//
// ui_lib.c
//
void *Menu_ItemAtCursor(menuframework_t *m);
sfxHandle_t Menu_DefaultKey(menuframework_t *m, unsigned int key);
void UI_DrawChar( int x, int y, int ch, int style, vec4_t color );
void UI_DrawBannerString( int x, int y, const char* str, int style, vec4_t color );
int UI_ProportionalStringWidth( const char* str );
float UI_ProportionalSizeScale( int style );
void UI_DrawString(int x, int y, const char *str, int style, vec4_t color);
void UI_DrawProportionalString( int x, int y, const char* str, int style, vec4_t color );
void UI_DrawProportionalString_AutoWrapped( int x, int y, int xmax, int ystep, const char* str, int style, vec4_t color );
void Menu_SetCursor(menuframework_t *m, int cursor);
void Menu_CursorMoved(menuframework_t *m);
void Menu_SetCursorToItem(menuframework_t *m, void *ptr);
void Menu_AdjustCursor(menuframework_t *m, int dir);
int UI_Shutdown( void );
int UI_SetActiveMenu(uiMenu_t menu);
int UI_KeyEvent( unsigned int key, int down );
int UI_MouseEvent( int dx, int dy );
int UI_Refresh( int realtime );
qboolean UI_ConsoleCommand( int realTime );
void UI_PopMenu(void);
void UI_PushMenu(menuframework_t *menu);
void UI_ForceMenuOff(void);
void UI_AdjustFrom640(float *x, float *y, float *w, float *h);
void UI_DrawTextBox(int x, int y, int width, int lines);
void UI_DrawNamedPic( float x, float y, float width, float height, const char *picname );
void UI_DrawHandlePic( float x, float y, float w, float h, nhandle_t hShader ); 
void UI_FillRect( float x, float y, float width, float height, const float *color );
void UI_DrawRect( float x, float y, float width, float height, const float *color );

//
// ui_mfield.c
//
void MField_Clear( mfield_t *edit );
void MField_KeyDownEvent( mfield_t *edit, unsigned int key );
void MField_CharEvent( mfield_t *edit, int ch );
void MField_Draw( mfield_t *edit, int x, int y, int style, vec4_t color );
void MenuField_Init( menufield_t* m );
void MenuField_Draw( menufield_t *f );
sfxHandle_t MenuField_Key( menufield_t* m, unsigned int* key );

//
// ui_main.c
//
int UI_Init( void );
int UI_Shutdown(void);
void GDR_DECL G_Printf(const char *fmt, ...);
void GDR_DECL G_Error(const char *fmt, ...);
qboolean UI_IsFullscreen(void);
void UI_UpdateCvars(void);

//
// ui_title.c
//
void UI_TitleMenu(void);
void TitleMenu_Cache(void);

//
// ui_menu.c
//
void UI_MainMenu(void);
void MainMenu_Cache(void);

void Menu_Cache(void);
void Menu_AddItem(menuframework_t *menu, void *item);
void Menu_Draw(menuframework_t *menu);

void *G_AllocMem(unsigned int size);
void G_ClearMem(void);

//==================================================================
// system calls (traps)
//

void trap_Cmd_ExecuteText(cbufExec_t exec, const char *text);
void trap_UpdateScreen(void);

void trap_RE_ClearScene(void);

void trap_GetClipboardData( char *buf, int bufsize );

// keyboard stuff
int trap_Key_GetCatcher(void);
void trap_Key_SetCatcher(int catcher);
int trap_Key_GetKey(const char *binding);
qboolean trap_Key_IsDown(unsigned int keynum);
void trap_Key_ClearStates(void);
qboolean trap_Key_AnyDown(void);

// print a formatted string to the console
void trap_Print(const char *str);

// throw an error and reset the vm
void trap_Error(const char *str);

void trap_RE_SetColor(const float *rgba);

void trap_RE_AddPolyToScene( nhandle_t hShader, const polyVert_t *verts, uint32_t numVerts );
void trap_RE_AddPolyListToScene( const poly_t *polys, uint32_t numPolys );
void trap_RE_DrawImage( float x, float y, float w, float h, float u1, float v1, float u2, float v2, nhandle_t hShader );

nhandle_t trap_RE_RegisterShader(const char *name);

void trap_GetGPUConfig(gpuConfig_t *config);

// register a sound effect
sfxHandle_t trap_Snd_RegisterSfx(const char *npath);

// queue a sound effect
void trap_Snd_PlaySfx(sfxHandle_t sfx);

// force stop an sfx
void trap_Snd_StopSfx(sfxHandle_t sfx);

// console variable interaction
void trap_Cvar_Register( vmCvar_t *vmCvar, const char *varName, const char *defaultValue, unsigned int flags );
void trap_Cvar_Update( vmCvar_t *vmCvar );
void trap_Cvar_Set( const char *var_name, const char *value );
void trap_Cvar_VariableStringBuffer( const char *var_name, char *buffer, unsigned int bufsize );

// ConsoleCommand parameter access
int trap_Argc( void );
void trap_Argv( unsigned int n, char *buffer, unsigned int bufferLength );
void trap_Args( char *buffer, unsigned int bufferLength );

#endif
