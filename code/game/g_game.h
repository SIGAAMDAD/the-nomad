/*
===========================================================================
Copyright (C) 2023-2024 GDR Games

This file is part of The Nomad source code.

The Nomad source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

The Nomad source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Foobar; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/

#ifndef __G_GAME__
#define __G_GAME__

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
    GS_STATS_MENU,
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
    mapsecret_t *secrets;

    uint32_t width;
    uint32_t height;
    uint32_t numTiles;
    uint32_t numSpawns;
    uint32_t numSecrets;
    uint32_t numCheckpoints;
    uint32_t numLights;
    uint32_t numLevels;
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
    mapinfo_t info;
    uint64_t numMapFiles;

    int32_t currentMapLoaded;
} mapCache_t;

#define	MAX_CONFIGSTRINGS	1024
#define CMD_BACKUP 64
#define CMD_MASK ( CMD_BACKUP - 1 )

typedef struct {
	int angles[3];
	signed char forwardmove;
	signed char sidemove;
	signed char upmove;
} usercmd_t;

typedef struct {
    char mapname[MAX_NPATH];

    int32_t framecount;
    int32_t frametime;      // msec since last frame

    int32_t realtime;       // ingores pause
    int32_t realFrameTime;

    qboolean uiStarted;
    qboolean sgameStarted;
    qboolean soundStarted;
    qboolean soundRegistered;
    qboolean rendererStarted;
    qboolean mapLoaded;

    qboolean togglePhotomode;
    
    uint64_t lastVidRestart;
    
    int mouseDx[2], mouseDy[2]; // added to by mouse events
    int mouseIndex;
    int joystickAxis[MAX_JOYSTICK_AXIS];

    // cmds[cmdNumber] is the predicted command, [cmdNumber-1] is the last
	// properly generated command
	usercmd_t	cmds[CMD_BACKUP];	// each message will send several old cmds
	int			cmdNumber;			// incremented each frame, because multiple
									// frames may need to be packed into a single packet
    int         oldCmdNumber;


    gameState_t oldState;
    gameState_t state;

    nhandle_t consoleShader;
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
    
    float cameraZoom;

    // demo information
    char demoName[MAX_OSPATH];
    char recordName[MAX_OSPATH]; // without extension
    qboolean explicitRecordName;
    char recordNameShort[TRUNCATE_LENGTH]; // for recording message
    qboolean spDemoRecording;
    qboolean demorecording;
    qboolean demoplaying;
    qboolean demowaiting;
    qboolean firstDemoFrameSkipped;

    fileHandle_t demofile;
    fileHandle_t recordfile;

    int timeDemoFrames;   // counter of rendered frames
    int timeDemoStart;    // gi.realtime before first frame
    int timeDemoBaseTime; // each frame will be at this time + frameNum * 50

    // simultaneous demo playback and recording
    int eventMask;
    int demoCommandSequence;
    int demoDeltaNum;
    int demoMessageSequence;

    char gameconfigStrings[BIG_INFO_STRING];

    backendCounters_t pc;

#ifdef __cplusplus
    glm::vec3 cameraPos;
    glm::vec3 cameraWorldPos;
    glm::mat4 viewMatrix;
    glm::mat4 viewProjectionMatrix;
    glm::mat4 projectionMatrix;
#endif
} gameInfo_t;

#define USE_GAMEKEY
// file full of random crap that gets used to create g_guid
#define GAMEKEY_FILE "gamekey"
#define GAMEKEY_SIZE 2048

#define EM_GAMESTATE 1
#define EM_SNAPSHOT  2
#define EM_COMMAND   4

extern field_t g_consoleField;
extern gameInfo_t gi;

// logging parameters
#define PRINT_INFO 0
#define PRINT_DEVELOPER 1
#define PRINT_WARNING 2
#define PRINT_ERROR 3

extern uint32_t g_console_field_width;
extern uint32_t bigchar_width;
extern uint32_t bigchar_height;
extern uint32_t smallchar_width;
extern uint32_t smallchar_height;

extern qboolean gw_minimized;
extern qboolean gw_active;

extern cvar_t *in_mode;

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
extern cvar_t *r_allowLegacy;
extern cvar_t *r_glDebug;
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
extern cvar_t *g_maxSaveSlots;

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

// NOTE: these resolutions are taken from AC4 Black Flag
typedef enum {
    // minimum resolution needed to run
    VIDMODE_1024x768,
    VIDMODE_1280x720,
    VIDMODE_1280x800,
    VIDMODE_1280x1024,
    VIDMODE_1440x900,
    VIDMODE_1440x960,
    VIDMODE_1600x900,
    VIDMODE_1600x1200,
    VIDMODE_1600x1050,
    VIDMODE_1920x800,
    VIDMODE_1920x1080,
    VIDMODE_1920x1200,
    VIDMODE_1920x1280,
    VIDMODE_2560x1080,
    VIDMODE_2560x1440,
    VIDMODE_2560x1600,
    VIDMODE_2880x1620,
    VIDMODE_3200x1800,
    VIDMODE_3840x1600,
    VIDMODE_3840x2160,

    NUMVIDMODES
} vidmodeNum_t;

extern const vidmode_t r_vidModes[NUMVIDMODES];

//
// g_game.cpp
//
qboolean G_GetModeInfo( int *width, int *height, float *windowAspect, int mode, const char *modeFS, int dw, int dh, qboolean fullscreen );
void G_Init( void );
void G_StartHunkUsers(void);
void G_Shutdown(qboolean quit);
void G_ClearMem(void);
void G_Restart(void);
void G_ShutdownRenderer(refShutdownCode_t code);
qboolean G_GameCommand(void);
void G_ClearState( void );
void G_FlushMemory( void );
void G_StartHunkUsers( void );
void G_ShutdownAll( void );
void G_ShutdownVMs( qboolean quit );
void G_Frame( int msec, int realMsec );
void G_InitDisplay( gpuConfig_t *config );
void GLimp_Minimize( void );
qboolean G_CheckPaused( void );
qboolean G_CheckWallHit( const vec3_t origin, dirtype_t dir );
void G_SetCameraData( const vec2_t origin, float zoom, float rotation );

//
// g_world.cpp
//
void G_InitMapCache( void );
void G_SetActiveMap( nhandle_t hMap, uint32_t *nCheckpoints, uint32_t *nSpawns, uint32_t *nTiles, int32_t *pWidth, int32_t *pHeight );
nhandle_t G_LoadMap( const char *name );

//
// g_screen.cpp
//
extern uint64_t time_frontend, time_backend;;
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
#ifdef __cplusplus
void Con_DrawConsole( qboolean open = qfalse );
#else
void Con_DrawConsole( qboolean open );
#endif
void Con_RunConsole( void );
void Con_Close( void );
void G_ConsolePrint( const char *txt );
void Con_CheckResize( void );
void Con_ClearNotify( void );
void Con_ToggleConsole_f( void );

void HandleEvents( void );

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

// Vulkan
#ifdef USE_VULKAN_API
#undef USE_VULKAN_API
#endif
#ifdef USE_VULKAN_API
void VKimp_Init( gpuConfig_t *config );
void VKimp_Shutdown( qboolean unloadDLL );
void *VK_GetInstanceProcAddr( VkInstance instance, const char *name );
qboolean VK_CreateSurface( VkInstance instance, VkSurfaceKHR *pSurface );
#endif

extern qboolean key_overstrikeMode;

//
// g_event.cpp
//
void G_MouseEvent( int dx, int dy );
void G_KeyEvent( uint32_t keynum, qboolean down, uint32_t time );
void G_JoystickEvent( int axis, int value, int time );
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

#ifdef __cplusplus
extern CModuleInfo *sgvm;
#endif
extern renderExport_t re;

#endif
