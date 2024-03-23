#ifndef _G_GAME_
#define _G_GAME_

#pragma once

#include "../engine/n_shared.h"
#include "../rendercommon/r_public.h"
#include "../engine/gln_files.h"
#if defined(__cplusplus)
#include "../module_lib/module_public.h"
#include "../engine/n_common.h"
#include "../rendercommon/imgui.h"
#include "../engine/n_cvar.h"
#include "../system/sys_timer.h"
#include <SDL2/SDL.h>
#include "../ui/ui_public.hpp"
#endif

typedef enum
{
    GS_INACTIVE,
    GS_READY,
    GS_MENU,
    GS_LEVEL,
    GS_PAUSE,
    GS_SETTINGS,
    GS_MEMORY_VIEW,

    NUM_GAME_STATES
} gameState_t;

//
// mapinfo_t
// This is the only way that the engine and vm may communicate about
// the current map loaded or to be loaded
//
typedef struct {
    char name[MAX_NPATH];

    maptile_t *tiles;
    mapcheckpoint_t *checkpoints;
    mapspawn_t *spawns;
    maplight_t *lights;

    uint32_t width;
    uint32_t height;
    uint32_t numTiles;
    uint32_t numSpawns;
    uint32_t numCheckpoints;
    uint32_t numLights;
} mapinfo_t;

typedef struct linkEntity_s {
    bbox_t bounds;
    vec3_t origin;
    uint32_t type;
    uint32_t id;
    uint32_t entityNumber;

    struct linkEntity_s *next;
    struct linkEntity_s *prev;
} linkEntity_t;

typedef struct {
	vec3_t start;
	vec3_t end;
	vec3_t origin;
    uint32_t entityNumber;
//	float speed;
	float length;
	float angle;
    uint32_t flags; // unused for now
} ray_t;

typedef enum
{
    SG_LOADGAME,
    SG_SAVEGAME,
    SG_INACTIVE,
    SG_IN_LEVEL,
    SG_SHOW_LEVEL_STATS,
} sgameState_t;

typedef enum
{
    SGAME_INIT,
    SGAME_SHUTDOWN,
    SGAME_ON_LEVEL_START,
    SGAME_ON_LEVEL_END,
} sgameExport_t;

enum
{
    OCC_NORMY,
    OCC_EASTER,
    OCC_CHRISTMAS,
    OCC_HALLOWEEN,
    OCC_THANKSGIVING
};

typedef enum
{
	ET_ITEM,
	ET_WEAPON,
	ET_MOB,
	ET_BOT,
	ET_WALL, // a tile with pre-determined collision physics
	ET_PLAYR,
	
	NUMENTITYTYPES
} entitytype_t;

typedef struct {
    char **mapList;
    mapinfo_t *infoList;
    uint64_t numMapFiles;

    int32_t currentMapLoaded;
} mapCache_t;

typedef struct {
    char mapname[MAX_NPATH];

    int32_t framecount;
    int32_t frametime;      // msec since last frame

    int32_t realtime;       // ingores pause
    int32_t realFrameTime;

    qboolean uiStarted;
    qboolean sgameStarted;
    qboolean soundStarted;
    qboolean rendererStarted;
    qboolean mapLoaded;

    qboolean togglePhotomode;
    
    uint64_t lastVidRestart;

    int32_t mouseDx[2], mouseDy[2];	// added to by mouse events
	int32_t mouseIndex;
    vec3_t viewangles;
    
    gameState_t oldState;
    gameState_t state;

    nhandle_t consoleShader0;
    nhandle_t consoleShader1;
    nhandle_t whiteShader;
    nhandle_t charSetShader;
    uint32_t captureWidth;
    uint32_t captureHeight;
    float con_factor;
    float biasX;
    float biasY;
    float scale;

    int32_t desktopWidth;
    int32_t desktopHeight;
    
    gpuConfig_t gpuConfig;

    mapCache_t mapCache;
} gameInfo_t;

extern field_t g_consoleField;
extern gameInfo_t gi;

// logging parameters
#define PRINT_INFO 0
#define PRINT_DEVELOPER 1
#define PRINT_WARNING 2

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
extern cvar_t *r_multisampleAmount;
extern cvar_t *r_multisampleType;
extern cvar_t *g_stencilBits;
extern cvar_t *g_depthBits;
extern cvar_t *r_stereoEnabled;
extern cvar_t *g_drawBuffer;

extern cvar_t *g_paused;

extern cvar_t *con_conspeed;
extern cvar_t *con_autoclear;
extern cvar_t *con_notifytime;
extern cvar_t *con_scale;
extern cvar_t *con_color;
extern cvar_t *con_noprint;
extern cvar_t *g_conXOffset;

typedef struct {
	const char	*description;
	uint32_t    width, height;
	float		pixelAspect;		// pixel width / height
} vidmode_t;

typedef enum
{
//    VIDMODE_640x480,
//    VIDMODE_800x600,

    // minimum resolution needed to run
    VIDMODE_1024x768,
    VIDMODE_1280x720,
    VIDMODE_1600x900,
    VIDMODE_1920x1080,
    VIDMODE_2048x1536,
    VIDMODE_3840x2160,

    NUMVIDMODES
} vidmodeNum_t;

extern const vidmode_t r_vidModes[NUMVIDMODES];

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
qboolean G_WindowMinimized( void );
void G_ClearState(void);
void G_FlushMemory(void);
void G_StartHunkUsers(void);
void G_ShutdownAll(void);
void G_ShutdownVMs(void);
void G_Frame( int32_t msec, int32_t realMsec );
void G_InitDisplay( gpuConfig_t *config );
SDL_Window *G_GetSDLWindow(void);
SDL_GLContext G_GetGLContext( void );
void GLimp_Minimize( void );
qboolean G_CheckPaused( void );
qboolean G_CheckWallHit( const vec3_t origin, dirtype_t dir );
void G_SetCameraData( const vec2_t origin, float zoom, float rotation );

//
// g_world.cpp
//
void G_InitMapCache( void );
void G_SetActiveMap( nhandle_t hMap, uint32_t *nCheckpoints, uint32_t *nSpawns,
    uint32_t *nTiles, float *soundBits, linkEntity_t *activeEnts );
nhandle_t G_LoadMap( const char *name );

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
// g_jpeg.cpp
//
size_t G_SaveJPGToBuffer( byte *buffer, size_t bufSize, int32_t quality, int32_t imageWidth, int32_t imageHeight, byte *imageBuffer, int32_t padding );
void G_SaveJPG( const char *filename, int32_t quality, int32_t imageWidth, int32_t imageHeight, byte *imageBuffer, int32_t padding );
void G_LoadJPG( const char *filename, byte **pic, int32_t *width, int32_t *height );

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
uint32_t Con_ExternalWindowID( void );
void Con_ExternalWindowEvent( uint32_t value );

// platform-specific
void GLimp_InitGamma( gpuConfig_t *config );
void GLimp_SetGamma( unsigned char red[256], unsigned char green[256], unsigned char blue[256] );

// OpenGL
#ifdef USE_OPENGL_API
void GLimp_Init( gpuConfig_t *config );
void GLimp_Shutdown( qboolean unloadDLL );
void GLimp_EndFrame( void );
void GLimp_HideFullscreenWindow( void );
void GLimp_LogComment( const char *msg );
void *GL_GetProcAddress( const char *name );
#endif

//
// g_input.cpp
//
void G_MouseEvent( int32_t dx, int32_t dy /*, int time*/ );
void G_InitInput( void );
void G_ShutdownInput( void );

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
void G_DrawUI(void);
qboolean UI_Command( void );

//
// g_sgame.cpp
//
void G_ShutdownSGame(void);
void G_InitSGame(void);
void G_SGameRender(stereoFrame_t stereo);
qboolean G_SGameCommand( void );

struct CModuleInfo;

extern CModuleInfo *sgvm;
extern renderExport_t re;

#endif
