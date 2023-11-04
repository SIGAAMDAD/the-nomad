#ifndef __UI_MENU__
#define __UI_MENU__

#pragma once

#define MAX_MENU_ITEMS 1024

class CUIMenuWidget;

class CUIMenu
{
public:
    CUIMenu(void);
    ~CUIMenu();

    void Load( const char **text );
    void AddItem( void *item );

    CUIMenuWidget **Items( void );
    const CUIMenuWidget **Items( void ) const;
public:
    int cursor;
    int cursor_prev;

    qboolean wrapAround;
    qboolean fullscreen;

    sfxHandle_t (*Key)( uint32_t key );
    void (*Draw)( void );
//private:
    char name[MAX_STRING_CHARS];
    void *items[MAX_MENU_ITEMS];

    int nitems;
};

class CUIMenuWidget
{
public:
    CUIMenuWidget( void );
    ~CUIMenuWidget();


    char name[MAX_STRING_CHARS];

    void (*EventCallback)( void *self, unsigned event );
    void (*StatusBar)( void *self );
    void (*OwnerDraw)( void *self );

    int type;
    int id;
    int x, y;
    int left;
    int top;
    int right;
    int bottom;
    CUIMenu *parent;
    int menuPosition;
    unsigned flags;
};

typedef struct {
    CUIMenuWidget generic;

    char buffer[MAX_EDIT_LINE];
    int cursor;
    int scroll;
    int widthInChars;
    int maxchars;
} mfield_t;

typedef struct {
    CUIMenuWidget generic;

    float minvalue;
    float maxvalue;
    float curvalue;

    float range;
} mslider_t;

typedef struct {
    CUIMenuWidget generic;

    int curvalue;
} mradiobutton_t;

typedef struct {
    CUIMenuWidget generic;

    int oldvalue;
    int curvalue;
    int numitems;
    int top;
    
    char **itemnames;
    int width;
    int height;
    int columns;
    int separation;
} mlist_t;

typedef struct {
    CUIMenuWidget generic;
} maction_t;

typedef struct {
    CUIMenuWidget generic;

    char *string;
    int style;
    vec4_t color;
} mtext_t;

typedef struct
{
    CUIMenuWidget	generic;

	char*			focuspic;	
	char*			errorpic;
	nhandle_t		shader;
	nhandle_t		focusshader;
	int				width;
	int				height;
	float*			focuscolor;
} mbitmap_t;

#endif