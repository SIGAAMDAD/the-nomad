#ifndef _G_GAME_
#define _G_GAME_

#pragma once

#include "../engine/n_shared.h"
#include "../engine/n_common.h"
#include "../sgame/sg_public.h"
#include "../ui/ui_public.h"
#include "../rendercommon/r_public.h"

#ifndef O_BINARY
#define O_BINARY 0
#endif
typedef enum
{
    GS_MENU,
    GS_LEVEL,
    GS_PAUSE,
    GS_SETTINGS,

    NUM_GAME_STATES
} gamestate_t;

enum
{
    OCC_NORMY,
    OCC_EASTER,
    OCC_CHRISTMAS,
    OCC_HALLOWEEN,
    OCC_THANKSGIVING
};

#ifndef Q3_VM

#include "../engine/vm_local.h"

typedef struct {
    char currentmap[MAX_GDR_PATH];

    qboolean uiStarted;
    qboolean sgameStarted;
    qboolean soundStarted;
    qboolean rendererStarted;
    qboolean mapLoaded;

    gamestate_t state;
    uint64_t frametime;
    uint64_t framecount;
    uint64_t realtime;

    uiMenu_t menuIndex;
    nhandle_t consoleShader;
    nhandle_t whiteShader;
    nhandle_t charSetShader;
    uint32_t captureWidth;
    uint32_t captureHeight;
    float con_factor;
    float biasX;
    float biasY;
    float scale;
    
    gpuConfig_t gpuConfig;
} gameInfo_t;

extern field_t g_consoleField;
extern gameInfo_t gi;

// logging parameters
#define PRINT_INFO 0
#define PRINT_DEVELOPER 1

extern uint32_t g_console_field_width;
extern uint32_t bigchar_width;
extern uint32_t bigchar_height;
extern uint32_t smallchar_width;
extern uint32_t smallchar_height;

// non api specific renderer cvars
extern cvar_t *vid_xpos;
extern cvar_t *vid_ypos;
extern cvar_t *r_allowSoftwareGL;
extern cvar_t *g_renderer; // current rendering api
extern cvar_t *r_displayRefresh;
extern cvar_t *r_fullscreen;
extern cvar_t *r_customWidth;
extern cvar_t *r_customHeight;
extern cvar_t *r_aspectRatio;
extern cvar_t *r_driver;
extern cvar_t *r_noborder;
extern cvar_t *r_drawFPS;
extern cvar_t *r_swapInterval;
extern cvar_t *r_mode;
extern cvar_t *r_customPixelAspect;
extern cvar_t *r_colorBits;
extern cvar_t *r_multisample;
extern cvar_t *g_stencilBits;
extern cvar_t *g_depthBits;
extern cvar_t *r_stereoEnabled;
extern cvar_t *g_drawBuffer;

extern cvar_t *con_conspeed;
extern cvar_t *con_autoclear;
extern cvar_t *con_notifytime;
extern cvar_t *con_scale;
extern cvar_t *con_color;
extern cvar_t *con_noprint;
extern cvar_t *g_conXOffset;

//
// g_game.cpp
//
void G_Init(void);
void G_StartHunkUsers(void);
void G_Shutdown(qboolean quit);
void G_ClearMem(void);
void G_Restart(void);
void G_ShutdownRenderer(refShutdownCode_t code);
qboolean G_GameCommand(void);
void G_ClearState(void);
void G_FlushMemory(void);
void G_StartHunkUsers(void);
void G_ShutdownAll(void);
void G_ShutdownVMs(void);
void G_Frame(uint64_t msec, uint64_t realMsec);
void G_InitDisplay(gpuConfig_t *config);

//
// g_screen.cpp
//
uint32_t SCR_GetBigStringWidth( const char *str ); // returns in virtual 640x480 coordinates
void SCR_AdjustFrom640( float *x, float *y, float *w, float *h );
void SCR_FillRect( float x, float y, float width, float height,  const float *color );
void SCR_DrawPic( float x, float y, float width, float height, nhandle_t hShader );
void SCR_DrawNamedPic( float x, float y, float width, float height, const char *picname );

void SCR_DrawBigString( uint32_t x, uint32_t y, const char *s, float alpha, qboolean noColorEscape );			// draws a string with embedded color control characters with fade
void SCR_DrawStringExt( uint32_t x, uint32_t y, float size, const char *string, const float *setColor,
    qboolean forceColor, qboolean noColorEscape );
void	SCR_DrawSmallStringExt( uint32_t x, uint32_t y, const char *string, const float *setColor,
    qboolean forceColor, qboolean noColorEscape );
void SCR_DrawSmallChar( uint32_t x, uint32_t y, int ch );
void SCR_DrawSmallString( uint32_t x, uint32_t y, const char *s, uint64_t len );
void SCR_UpdateScreen( void );

//
// g_console.cpp
//
void Con_DrawConsole( void );
void Con_RunConsole( void );
void Con_PageUp( uint32_t lines );
void Con_PageDown( uint32_t lines );
void Con_Top( void );
void Con_Bottom( void );
void Con_Close( void );
void G_ConsolePrint( const char *txt );
void Con_CheckResize( void );
void Con_ClearNotify( void );
void Con_ToggleConsole_f( void );

extern qboolean key_overstrikeMode;

//
// g_event.cpp
//
void G_KeyEvent(uint32_t keynum, qboolean down, uint32_t time);
void Field_Draw( field_t *edit, uint32_t x, uint32_t y, uint32_t width, qboolean showCursor, qboolean noColorEscape );
void Field_BigDraw( field_t *edit, uint32_t x, uint32_t y, uint32_t width, qboolean showCursor, qboolean noColorEscape );
qboolean Key_AnyDown(void);

//
// g_ui.cpp
//
void G_ShutdownUI(void);
void G_InitUI(void);

//
// g_sgame.cpp
//
void G_ShutdownSGame(void);
void G_InitSGame(void);
void G_SGameRender(stereoFrame_t stereo);

extern vm_t *sgvm;
extern vm_t *uivm;
extern renderExport_t re;

#endif

#endif
