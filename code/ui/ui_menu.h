#ifndef __UI_MENU__
#define __UI_MENU__

#pragma once

#include "ui_string_manager.h"

#define MAX_TABLE_ITEMS 528
#define MAX_MENU_ITEMS 528

#define MTYPE_NULL          0
#define MTYPE_TEXT          1
#define MTYPE_SLIDER        2
#define MTYPE_BUTTON        3
#define MTYPE_TABLE         4
#define MTYPE_RADIOBUTTON   5
#define MTYPE_FIELD         6
#define MTYPE_ARROW         7
#define MTYPE_LIST          8
#define MTYPE_CUSTOM        9
#define MTYPE_TREE          10
#define MTYPE_LISTEXT       11
#define MTYPE_TAB           12

#define QMF_BLINK               0x00000001
#define QMF_SMALLFONT           0x00000002
#define QMF_NUMBERSONLY         0x00000004 // edit field is only numbers
#define QMF_HIGHLIGHT           0x00000008
#define QMF_HIGHLIGHT_IF_FOCUS  0x00000010 // steady focus
#define QMF_PULSEIFFOCUS        0x00000020 // pulse if focus
#define QMF_HASMOUSEFOCUS       0x00000040
#define QMF_NOONOFFTEXT         0x00000080
#define QMF_MOUSEONLY           0x00000100 // only mouse input allowed
#define QMF_HIDDEN              0x00000200 // skips drawing
#define QMF_GRAYED              0x00000400 // grays and disables
#define QMF_INACTIVE            0x00000800 // disables any input
#define QMF_OWNERDRAW           0x00001000
#define QMF_PULSE               0x00002000
#define QMF_SILENT              0x00004000 // don't make any sounds
#define QMF_CUSTOMFONT          0x00008000 // use a custom font
#define QMF_SAMELINE_NEXT       0x00010000
#define QMF_SAMELINE_PREV       0x00020000

// callback notifications
#define EVENT_GOTFOCUS				1
#define EVENT_LOSTFOCUS			    2
#define EVENT_ACTIVATED			    3

#define MENU_DEFAULT_FLAGS ( ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoTitleBar \
                        | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoCollapse )

typedef struct {
    void (*draw)( void );
    sfxHandle_t (*key)( unsigned );

    const char *name;

    float titleFontScale;
    float textFontScale;

//    int cursor;
//    int cursor_prev;

    int flags;
    int x;
    int y;
    int width;
    int height;

    int nitems;
    void *items[MAX_MENU_ITEMS];

    qboolean fullscreen;
} menuframework_t;

typedef struct {
    const char *name;
    menuframework_t *parent;
	
    int type;
    int id;
//    int x, y;
//    int left;
//    int top;
//    int right;
//    int bottom;
    unsigned flags;
    qboolean focused;
    ImFont *font;
    
    void (*eventcallback)( void *self, int id );
    void (*ownerdraw)( void *self );
} menucommon_t;

typedef struct {
    menucommon_t generic;

    char buffer[MAX_EDIT_LINE];

    float *color;
    int widthInChars;
    int cursor;
    int scroll;
    int maxchars;
} menufield_t;

typedef struct {
    menucommon_t generic;
    
    float *color;
    int direction;
} menuarrow_t;

typedef struct {
    menucommon_t generic;
    
    float *color;
    nhandle_t shader;
    nhandle_t focusshader;
} menubutton_t;

typedef struct {
    menucommon_t generic;

    float *color;

    float minvalue;
    float maxvalue;
    float curvalue;

    float range;

    qboolean isIntegral;
} menuslider_t;

typedef struct {
    menucommon_t generic;
	
	float *color;
    const char *text;
} menutext_t;

typedef struct {
    menucommon_t generic;
    menucommon_t *items[MAX_TABLE_ITEMS];

    int columns;
    int rows;
    int numitems;
} menutable_t;

typedef struct {
    menucommon_t generic;
    
    float *color;
    qboolean curvalue;
} menuswitch_t;

typedef struct {
    menucommon_t generic;
    
    int curitem;
    int numitems;
    const char **itemnames;
} menulist_t;

typedef struct {
	menucommon_t generic;
	
	int curitem;
	int numitems;
	menucommon_t *items[128];
} menutree_t;

typedef struct {
	menucommon_t generic;
	
	int curitem;
	int numitems;
	menucommon_t *items[128];
} menulistex_t;

typedef struct {
	menucommon_t generic;
	
	float *tabColorActive;
	float *tabColorFocused;
	float *tabColor;
	int curitem;
	int numitems;
	menucommon_t *items[128];
} menutab_t;

typedef struct {
	menucommon_t generic;
} menucustom_t;

extern void MenuEvent_ArrowLeft( void *ptr, int event );
extern void MenuEvent_ArrowRight( void *ptr, int event );

extern void Table_AddRow( menutable_t *table );
extern void Table_AddItem( menutable_t *table, void *item );
extern void Menu_AddItem( menuframework_t *menu, void *item );
extern void Menu_Draw( menuframework_t *menu );
extern void Menu_Cache( void );
extern const char *StringDup( const stringHash_t *str, const char *id );

#endif