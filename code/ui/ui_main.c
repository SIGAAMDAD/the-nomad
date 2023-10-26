#include "ui_local.h"

uiGlobals_t ui;

/*
vmMain

this is the only way control passes into the module.
this must be the very first function compiled into the .qvm file
*/
int vmMain(int command, int arg0, int arg1, int arg2, int arg3, int arg4,
            int arg5, int arg6, int arg7, int arg8, int arg10, int arg11)
{
    switch (command) {
    case UI_GETAPIVERSION:
        return NOMAD_VERSION;
    case UI_GET_MENU:
        return (int)ui.menuIndex;
    case UI_IS_FULLSCREEN:
        return (int)UI_IsFullscreen();
    case UI_FINISH_FRAME:
        return UI_Refresh(arg0);
    case UI_INIT:
        return UI_Init();
    case UI_SHUTDOWN:
        return UI_Shutdown();
    case UI_KEY_EVENT:
        return UI_KeyEvent(arg0, arg1);
    case UI_MOUSE_EVENT:
        return UI_MouseEvent(arg0, arg1);
    default:
        G_Error("vmMain(uivm): bad command: %i", command);
        break;
    };

    return -1;
}


qboolean UI_IsFullscreen( void ) {
	if ( ui.curMenu && ( trap_Key_GetCatcher() & KEYCATCH_UI ) ) {
		return ui.curMenu->fullscreen;
	}

	return qfalse;
}

int UI_SetActiveMenu(uiMenu_t menu)
{
    // this should be the ONLY way the menu system is brought up
    // ensure minimum menu data is cached
    Menu_Cache();

    switch (menu) {
    case UI_MENU_NONE:
        G_Printf("Setting menu to None\n");
        UI_ForceMenuOff();
        return 1;
    case UI_MENU_TITLE:
        G_Printf("Setting menu to Title Screen\n");
        UI_TitleMenu();
        return 1;
    case UI_MENU_MAIN:
        G_Printf("Setting menu to Main Menu\n");
        UI_MainMenu();
        return 1;
    default:
        G_Printf("Bad menu index: %i\n", menu);
        break;
    };
    return -1;
}

int UI_KeyEvent(unsigned int key, int down)
{
    sfxHandle_t s;

    if (!ui.curMenu) {
        return 0;
    }

    if (!down) {
        return 1;
    }

    if (ui.curMenu->key)
        s = ui.curMenu->key(key);
    else
        s = Menu_DefaultKey(ui.curMenu, key);
    
    if ((s > 0) && (s != ui.null_sound))
        trap_Snd_PlaySfx(s);
    
    return 1;
}

int UI_MouseEvent( int dx, int dy )
{
	int				i;
	menucommon_t*	m;

	if (!ui.curMenu)
		return 0;

	// update mouse screen position
	ui.cursorx += dx;
	if (ui.cursorx < 0)
		ui.cursorx = 0;
	else if (ui.cursorx > SCREEN_WIDTH)
		ui.cursorx = SCREEN_WIDTH;

	ui.cursory += dy;
	if (ui.cursory < 0)
		ui.cursory = 0;
	else if (ui.cursory > SCREEN_HEIGHT)
		ui.cursory = SCREEN_HEIGHT;

	// region test the active menu items
	for (i = 0; i < ui.curMenu->nitems; i++) {
 		m = (menucommon_t *)ui.curMenu->items[i];

		if (m->flags & (QMF_GRAYED|QMF_INACTIVE))
			continue;

		if ((ui.cursorx < m->left) ||
			(ui.cursorx > m->right) ||
			(ui.cursory < m->top) ||
			(ui.cursory > m->bottom))
		{
			// cursor out of item bounds
			continue;
		}

		// set focus to item at cursor
		if (ui.curMenu->cursor != i) {
			Menu_SetCursor( ui.curMenu, i );
			((menucommon_t *)(ui.curMenu->items[ui.curMenu->cursor_prev]))->flags &= ~QMF_HASMOUSEFOCUS;

			if ( !(((menucommon_t *)(ui.curMenu->items[ui.curMenu->cursor]))->flags & QMF_SILENT ) ) {
				trap_Snd_PlaySfx( ui.move_sound );
			}
		}

		((menucommon_t *)(ui.curMenu->items[ui.curMenu->cursor]))->flags |= QMF_HASMOUSEFOCUS;
		return 1;
	}

	if (ui.curMenu->nitems > 0) {
		// out of any region
		((menucommon_t *)(ui.curMenu->items[ui.curMenu->cursor]))->flags &= ~QMF_HASMOUSEFOCUS;
	}

    return 1;
}

char *UI_Argv( unsigned int arg )
{
	static char	buffer[MAX_STRING_CHARS];

	trap_Argv( arg, buffer, sizeof( buffer ) );

	return buffer;
}


char *UI_Cvar_VariableString( const char *var_name )
{
	static char	buffer[MAX_STRING_CHARS];

	trap_Cvar_VariableStringBuffer( var_name, buffer, sizeof( buffer ) );

	return buffer;
}

void GDR_DECL G_Printf(const char *fmt, ...)
{
    va_list argptr;
    char msg[4096];

    va_start(argptr, fmt);
    vsprintf(msg, fmt, argptr);
    va_end(argptr);

    trap_Print(msg);
}

void GDR_DECL G_Error(const char *fmt, ...)
{
    va_list argptr;
    char msg[4096];

    va_start(argptr, fmt);
    vsprintf(msg, fmt, argptr);
    va_end(argptr);

    trap_Error(msg);
}

void GDR_DECL GDR_ATTRIBUTE((format(printf, 2, 3))) N_Error(errorCode_t code, const char *err, ...)
{
    va_list argptr;
    char msg[4096];

    va_start(argptr, err);
    vsprintf(msg, err, argptr);
    va_end(argptr);

    G_Error("%s", msg);
}

#ifndef SGAME_HARD_LINKED
// this is only here so the functions in n_shared.c and bg_*.c can link

void GDR_DECL Con_Printf(const char *fmt, ...)
{
    va_list argptr;
    char msg[4096];

    va_start(argptr, fmt);
    vsprintf(msg, fmt, argptr);
    va_end(argptr);

    G_Printf("%s", msg);
}

#endif

static void UI_RegisterCvars(void)
{
    
}

void UI_UpdateCvars(void)
{
    
}

int UI_Shutdown(void) {
    return 1;
}

int UI_Init(void)
{
    G_ClearMem();

    memset(&ui, 0, sizeof(ui));

    // cache redundant calculations
    trap_GetGPUConfig(&ui.gpuConfig);

    // for 640x480 virtualized screen
    ui.scale = ui.gpuConfig.vidHeight * (1.0/480.0);
    if (ui.gpuConfig.vidWidth * 480 > ui.gpuConfig.vidHeight * 640) {
        // wide screen
        ui.bias = 0.5 * (ui.gpuConfig.vidWidth - (ui.gpuConfig.vidHeight * (640.0/480.0)));
    }
    else {
        // no wide screen
        ui.bias = 0;
    }

    // initialize the menu system
    Menu_Cache();

    UI_SetActiveMenu(UI_MENU_TITLE);

    ui.curMenu = NULL;
    ui.stackp = 0;

    return 1;
}


// 256 KiB for the ui vm to use
#define MEMPOOL_SIZE (256*1024)
static char mempool[MEMPOOL_SIZE];
static int allocPoint;

void G_ClearMem(void)
{
    memset(mempool, 0, sizeof(mempool));
    allocPoint = 0;
}

void *G_AllocMem(unsigned int size)
{
    char *buf;

    if (!size || size >= sizeof(mempool))
        G_Error( "G_AllocMem: bad size: %i", size );
    
    // round to 16-bit
    size = (size - 15) & ~15;

    buf = &mempool[allocPoint];
    allocPoint += size;

    return buf;
}
