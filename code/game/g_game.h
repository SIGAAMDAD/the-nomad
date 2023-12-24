#ifndef _G_GAME_
#define _G_GAME_

#pragma once

#include "../engine/n_shared.h"
#include "../engine/n_common.h"
#include "../sgame/sg_public.h"
#include "../rendercommon/r_public.h"
#include "../engine/gln_files.h"
#if defined(__cplusplus)
#include "../rendercommon/imgui.h"
#include "g_vmimgui.h"
#include "../engine/n_cvar.h"
#include "../system/sys_timer.h"
#include <SDL2/SDL.h>
#endif

#ifndef O_BINARY
#define O_BINARY 0
#endif
typedef enum
{
    GS_INACTIVE,
    GS_READY,
    GS_MENU,
    GS_LEVEL,
    GS_PAUSE,
    GS_SETTINGS,

    NUM_GAME_STATES
} gamestate_t;

//
// mapinfo_t
// This is the only way that the engine and vm may communicate about
// the current map loaded or to be loaded
//
typedef struct {
    mapspawn_t spawns[MAX_MAP_SPAWNS];
    mapcheckpoint_t checkpoints[MAX_MAP_CHECKPOINTS];

    char name[MAX_GDR_PATH];
    int32_t width;
    int32_t height;
    int32_t numSpawns;
    int32_t numCheckpoints;
} mapinfo_t;

enum
{
    OCC_NORMY,
    OCC_EASTER,
    OCC_CHRISTMAS,
    OCC_HALLOWEEN,
    OCC_THANKSGIVING
};

#if !defined(Q3_VM) || (defined(UI_HARD_LINKED) || defined(SGAME_HARD_LINKED))
typedef struct vmRefImport_s vmRefImport_t;
struct vmRefImport_s
{
    void (*trap_Print)( const char *str );
    void (*trap_Error)( const char *str );

    uint32_t (*trap_Argc)( void );
    void (*trap_Argv)( uint32_t n, char *buf, uint32_t bufferLength );
    void (*trap_Args)( char *buf, uint32_t bufferLength );
    void (*trap_SendConsoleCommand)( const char *text );
    void (*trap_AddCommand)( const char *cmdName );
    void (*trap_RemoveCommand)( const char *cmdName );

    uint32_t (*trap_MemoryRemaining)( void );

    void (*Sys_SnapVector)( float *v );

    int (*G_LoadMap)( int32_t levelIndex, mapinfo_t *info, int32_t *soundBits, linkEntity_t *activeEnts );
    void (*G_CastRay)( ray_t *ray );
    void (*G_SoundRecursive)( int32_t width, int32_t height, float volume, const vec3_t origin );
    qboolean (*trap_CheckWallHit)( const vec3_t origin, dirtype_t dir );

    void (*G_SetCameraData)( const vec2_t origin, float zoom, float rotation );

    int (*trap_Milliseconds)( void );

    void (*trap_Key_SetCatcher)( int32_t catcher );
    uint32_t (*trap_Key_GetCatcher)( void );
    uint32_t (*trap_Key_GetKey)( const char *key );
    void (*trap_Key_ClearStates)( void );

    sfxHandle_t (*trap_Snd_RegisterSfx)( const char *npath );
    sfxHandle_t (*trap_Snd_RegisterTrack)( const char *npath );
    void (*trap_Snd_QueueTrack)( sfxHandle_t track );
    void (*trap_Snd_PlaySfx)( sfxHandle_t sfx );
    void (*trap_Snd_StopSfx)( sfxHandle_t sfx );
    void (*trap_Snd_SetLoopingTrack)( sfxHandle_t track );
    void (*trap_Snd_ClearLoopingTrack)( void );

    nhandle_t (*RE_RegisterShader)( const char *npath );
    nhandle_t (*RE_RegisterSpriteSheet)( const char *npath, uint32_t sheetWidth, uint32_t sheetHeight, uint32_t spriteWidth, uint32_t spriteHeight );
    nhandle_t (*RE_RegisterSprite)( nhandle_t hSpriteSheet, uint32_t index );
    void (*RE_LoadWorldMap)( const char *npath );
    void (*RE_ClearScene)( void );
    void (*RE_RenderScene)( const renderSceneRef_t *fd );
    void (*RE_AddSpriteToScene)( const vec3_t origin, nhandle_t hSpriteSheet, nhandle_t hSprite );
    void (*RE_AddPolyToScene)( nhandle_t hShader, const polyVert_t *verts, uint32_t numVerts );

    void (*Sys_GetGPUConfig)( gpuConfig_t *config );

    void (*Cvar_Register)( vmCvar_t *vmCvar, const char *varName, const char *defaultValue, uint32_t flags );
    void (*Cvar_Update)( vmCvar_t *vmCvar );
    void (*Cvar_Set)( const char *varName, const char *value );
    void (*Cvar_VariableStringBuffer)( const char *var_name, char *buffer, uint32_t bufsize );

    file_t (*FS_FOpenRead)( const char *npath, handleOwner_t owner );
    file_t (*FS_FOpenWrite)( const char *npath, handleOwner_t owner );
    file_t (*FS_FOpenAppend)( const char *npath, handleOwner_t owner );
    file_t (*FS_FOpenRW)( const char *npath, handleOwner_t owner );
    fileOffset_t (*FS_FileSeek)( file_t file, fileOffset_t offset, uint32_t whence, handleOwner_t owner );
    fileOffset_t (*FS_FileTell)( file_t file, handleOwner_t owner );
    uint32_t (*FS_FOpenFile)( const char *npath, file_t *file, fileMode_t mode, handleOwner_t owner );
    file_t (*FS_FOpenFileWrite)( const char *npath, file_t *file, handleOwner_t owner );
    uint32_t (*FS_FOpenFileRead)( const char *npath, file_t *file, handleOwner_t owner );
    void (*FS_FClose)( file_t file, handleOwner_t owner );
    uint32_t (*FS_WriteFile)( const void *buffer, uint32_t len, file_t file, handleOwner_t owner );
    uint32_t (*FS_Write)( const void *buffer, uint32_t len, file_t file, handleOwner_t owner );
    uint32_t (*FS_Read)( void *buffer, uint32_t len, file_t file, handleOwner_t owner );
    uint32_t (*FS_FileLength)( file_t file, handleOwner_t owner );
    uint32_t (*FS_GetFileList)( const char *path, const char *extension, char *listbuf, uint32_t bufsize );

    int (*ImGui_BeginWindow)( const char *pLabel, byte *pOpen, ImGuiWindowFlags windowFlags );
    int (*ImGui_BeginPopupModal)( const char *pName, ImGuiWindowFlags flags );
    int (*ImGui_IsWindowCollapsed)( void );
    int (*ImGui_BeginMenu)( const char *pLabel );
    int (*ImGui_MenuItem)( const char *pLabel, const char *pShortcut, byte bUsed );
    int (*ImGui_BeginTable)( const char *pLabel, uint32_t nColumns );
    int (*ImGui_InputText)( const char *pLabel, char *pBuffer, size_t nBufSize, ImGuiInputTextFlags flags );
    int (*ImGui_InputTextMultiline)( const char *pLabel, char *pBuffer, size_t nBufSize, ImGuiInputTextFlags flags );
    int (*ImGui_InputTextWithHint)( const char *pLabel, const char *pHint, char *pBuffer, size_t nBufSize, ImGuiInputTextFlags flags );
    int (*ImGui_InputFloat)( const char *pLabel, float *pData );
    int (*ImGui_InputFloat2)( const char *pLabel, vec2_t pData );
    int (*ImGui_InputFloat3)( const char *pLabel, vec3_t pData );
    int (*ImGui_InputFloat4)( const char *pLabel, vec4_t pData );
    int (*ImGui_InputInt)( const char *pLabel, int32_t *pData );
    int (*ImGui_InputInt2)( const char *pLabel, ivec2_t pData );
    int (*ImGui_InputInt3)( const char *pLabel, ivec3_t pData );
    int (*ImGui_InputInt4)( const char *pLabel, ivec4_t pData );
    int (*ImGui_SliderFloat)( const char *pLabel, float *pData, float nMax, float nMin );
    int (*ImGui_SliderFloat2)( const char *pLabel, vec2_t pData, float nMax, float nMin );
    int (*ImGui_SliderFloat3)( const char *pLabel, vec3_t pData, float nMax, float nMin );
    int (*ImGui_SliderFloat4)( const char *pLabel, vec4_t pData, float nMax, float nMin );
    int (*ImGui_SliderInt)( const char *pLabel, int32_t *pData, int32_t nMax, int32_t nMin );
    int (*ImGui_SliderInt2)( const char *pLabel, ivec2_t pData, int32_t nMax, int32_t nMin );
    int (*ImGui_SliderInt3)( const char *pLabel, ivec3_t pData, int32_t nMax, int32_t nMin );
    int (*ImGui_SliderInt4)( const char *pLabel, ivec4_t pData, int32_t nMax, int32_t nMin );
    int (*ImGui_ColorEdit3)( const char *pLabel, vec3_t pColor, ImGuiColorEditFlags flags );
    int (*ImGui_ColorEdit4)( const char *pLabel, vec4_t pColor, ImGuiColorEditFlags flags );
    int (*ImGui_ArrowButton)( const char *pLabel, ImGuiDir dir );
    int (*ImGui_Checkbox)( const char *pLabel, byte *bPressed );
    int (*ImGui_Button)( const char *pLabel );
    float (*ImGui_GetFontScale)( void );
    void (*ImGui_EndWindow)( void );
    void (*ImGui_SetWindowCollapsed)( int bCollapsed );
    void (*ImGui_SetWindowPos)( float x, float y );
    void (*ImGui_SetWindowSize)( float w, float h );
    void (*ImGui_SetWindowFontScale)( float scale );
    void (*ImGui_EndMenu)( void );
    void (*ImGui_SetItemTooltipUnformatted)( const char *pTooltip );
    void (*ImGui_TableNextRow)( void );
    void (*ImGui_TableNextColumn)( void );
    void (*ImGui_EndTable)( void );
    void (*ImGui_SetCursorPos)( float x, float y );
    void (*ImGui_GetCursorPos)( float *x, float *y );
    void (*ImGui_SetCursorScreenPos)( float x, float y );
    void (*ImGui_GetCursorScreenPos)( float *x, float *y );
    void (*ImGui_PushColor)( ImGuiCol index, const vec4_t color );
    void (*ImGui_PopColor)( void );
    void (*ImGui_SameLine)( float offsetFromStartX );
    void (*ImGui_NewLine)( void );
    void (*ImGui_TextUnformatted)( const char *pText );
    void (*ImGui_ColoredTextUnformatted)( const vec4_t pColor, const char *pText );
    void (*ImGui_SeparatorText)( const char *pText );
    void (*ImGui_Separator)( void );
    void (*ImGui_ProgressBar)( float fraction );
    void (*ImGui_OpenPopup)( const char *pName );
    void (*ImGui_CloseCurrentPopup)( void );
    void (*ImGui_EndPopup)( void );
};
#endif

#define CMD_BACKUP 64
#define	CMD_MASK			(CMD_BACKUP - 1)

#if 0
class CGame
{
public:
    CGame(void);
    ~CGame();

    void Init(void);
};
#endif

typedef struct usercmd_s {
	int32_t			angles[3];
	uint32_t		buttons;
	byte			weapon;           // weapon 
	int8_t	        forwardmove, rightmove, upmove;
} usercmd_t;

#if !defined(UI_HARD_LINKED) && !defined(SGAME_HARD_LINKED) && !defined(Q3_VM)

typedef struct {
    mapinfo_t info;

    uint32_t numTiles;
    maptile_t *tiles;
} mapinfoReal_t;

#include "../ui/ui_public.h"
#include "../engine/vm_local.h"

typedef struct {
    char **mapList;
    mapinfoReal_t *infoList;
    uint64_t numMapFiles;

    int32_t currentMapLoaded;
} mapCache_t;

typedef struct {
    char currentmap[MAX_GDR_PATH];

    qboolean uiStarted;
    qboolean sgameStarted;
    qboolean soundStarted;
    qboolean rendererStarted;
    qboolean mapLoaded;

    qboolean togglePhotomode;

    gamestate_t state;
    int32_t frametime;
    int32_t oldframetime;
    int32_t framecount;
    int32_t realtime;
    int32_t realFrameTime;
    int32_t sendtime;
    
    uint64_t lastVidRestart;

    int32_t mouseDx[2], mouseDy[2];	// added to by mouse events
	int32_t mouseIndex;
    vec3_t viewangles;

    uint32_t cmdNumber;
    usercmd_t cmds[CMD_BACKUP];

//    uiMenu_t menuIndex;
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
extern cvar_t *r_multisample;
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
void G_Frame(int msec, int realMsec);
void G_InitDisplay(gpuConfig_t *config);
SDL_Window *G_GetSDLWindow(void);
SDL_GLContext G_GetGLContext( void );
void GLimp_Minimize( void );
int32_t G_LoadMap( int32_t index, mapinfo_t *info, int32_t *soundBits, linkEntity_t *activeEnts );
qboolean G_CheckPaused( void );
qboolean G_CheckWallHit( const vec3_t origin, dirtype_t dir );
void G_SetCameraData( const vec2_t origin, float zoom, float rotation );

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
uint32_t Con_ExternalWindowID( void );
void Con_ExternalWindowEvent( uint32_t value );

//
// g_input.cpp
//
void G_MouseEvent( int32_t dx, int32_t dy /*, int time*/ );
void G_InitInput( void );
void G_ShutdownInput( void );
usercmd_t G_CreateNewCommand( void );

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
