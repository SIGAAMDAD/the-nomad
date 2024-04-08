#ifndef __UI_MENU__
#define __UI_MENU__

#pragma once

#include "ui_string_manager.h"

#define MAX_TABLE_ITEMS 528
#define MAX_MENU_ITEMS 1024

#define MTYPE_NULL          0
#define MTYPE_TEXT          1
#define MTYPE_SLIDER        2
#define MTYPE_BUTTON        3
#define MTYPE_TABLE         4
#define MTYPE_RADIOBUTTON   5
#define MTYPE_FIELD         6
#define MTYPE_ARROW         7
#define MTYPE_TREE          8
#define MTYPE_NESTED        9
#define MTYPE_LIST          10

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

#define MENU_DEFAULT_FLAGS ( ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoTitleBar \
                        | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoCollapse )

typedef struct {
    void (*draw)( void );
    sfxHandle_t (*key)( unsigned );

    const char *name;

    float titleFontScale;
    float textFontScale;

    int cursor;
    int cursor_prev;

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
    int x, y;
    int left;
    int top;
    int right;
    int bottom;
    unsigned flags;
    qboolean hovered;
    ImFont *font;

    void (*draw)( void *self );
    void (*eventcallback)( void *self, int id );
} menucommon_t;

typedef struct {
    menucommon_t generic;

    char buffer[MAX_EDIT_LINE];

    vec4_t color;
    int widthInChars;
    int cursor;
    int scroll;
    int maxchars;
} menufield_t;

typedef struct {
    menucommon_t generic;
    vec4_t color;
    int direction;
} menuarrow_t;

typedef struct {
    menucommon_t generic;
    vec4_t color;
} menubutton_t;

typedef struct {
    menucommon_t generic;

    vec4_t color;

    float minvalue;
    float maxvalue;
    float curvalue;

    float range;

    qboolean isIntegral;
} menuslider_t;

typedef struct {
    menucommon_t generic;

    vec4_t color;
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
    
    vec4_t color;
    const char *onstring;
    const char *offstring;
    qboolean curvalue;
} menuswitch_t;

typedef struct {
    menucommon_t generic;

    qboolean useTable;
    int curitem;
    int numitems;
    const char *itemnames[64];
} menulist_t;

typedef struct {
    menucommon_t generic;
    menuframework_t data;
} menu_nested_t;

extern void Table_AddRow( menutable_t *table );
extern void Table_AddItem( menutable_t *table, void *item );
extern void Menu_AddItem( menuframework_t *menu, void *item );
extern void Menu_Draw( menuframework_t *menu );
extern void Menu_Cache( void );
extern const char *StringDup( const stringHash_t *str, const char *id );

#endif