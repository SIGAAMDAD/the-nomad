#ifndef _G_GAME_
#define _G_GAME_

#pragma once

#include "../engine/n_shared.h"
#include "../engine/n_common.h"
#include "../sgame/sg_public.h"
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

#if !defined(Q3_VM) || (defined(UI_HARD_LINKED) || defined(SGAME_HARD_LINKED))
typedef struct vmRefImport_s vmRefImport_t;
struct vmRefImport_s
{
    void (*trap_Cmd_ExecuteText)( cbufExec_t exec, const char *text );
    void (*trap_UpdateScreen)( void );
    void (*trap_GetClipboardData)( char *buf, uint32_t bufsize );
    void (*trap_GetGPUConfig)( gpuConfig_t *config );

    uint32_t (*trap_Key_GetCatcher)( void );
    void (*trap_Key_SetCatcher)( uint32_t catcher );
    uint32_t (*trap_Key_GetKey)( const char *binding );
    qboolean (*trap_Key_IsDown)( uint32_t keynum );
    void (*trap_Key_ClearStates)( void );
    qboolean (*trap_Key_AnyDown)( void );

    void (*trap_Print)( const char *str );
    void (*trap_Error)( const char *str );

    file_t (*trap_FS_FOpenWrite)( const char *path, file_t *f, handleOwner_t owner );
    file_t (*trap_FS_FOpenRead)( const char *path, file_t *f, handleOwner_t owner );
    void (*trap_FS_FClose)( file_t f );
    uint32_t (*trap_FS_Read)( void *buffer, uint32_t len, file_t f, handleOwner_t owner );
    uint32_t (*trap_FS_Write)( const void *buffer, uint32_t len, file_t f, handleOwner_t owner );
    void (*trap_FS_WriteFile)( const void *buffer, uint32_t len, file_t f, handleOwner_t owner );
    void (*trap_FS_CreateTmp)( char *name, const char *ext, file_t *f, handleOwner_t owner );
    uint64_t (*trap_FS_FOpenFileRead)( const char *path, file_t *f, handleOwner_t owner );
    fileOffset_t (*trap_FS_FileSeek)( file_t f, fileOffset_t offset, uint32_t whence, handleOwner_t owner );
    uint64_t (*trap_FS_FOpenFileWrite)( const char *path, file_t *f, handleOwner_t owner );
    fileOffset_t (*trap_FS_FileTell)( file_t f );
    uint64_t (*trap_FS_FileLength)( file_t f );

    void (*trap_RE_ClearScene)( void );
    void (*trap_RE_SetColor)( const float *rgba );
    void (*trap_RE_AddPolyToScene)( nhandle_t hShader, const polyVert_t *verts, uint32_t numVerts );
    void (*trap_RE_AddPolyListToScene)( const poly_t *polys, uint32_t numPolys );
    void (*trap_RE_AddEntityToScene)( const renderEntityRef_t *ent );
    void (*trap_RE_DrawImage)( float x, float y, float w, float h, float u1, float v1, float u2, float v2, nhandle_t hShader );
    nhandle_t (*trap_RE_RegisterShader)( const char *name );
    void (*trap_RE_RenderScene)( const renderSceneRef_t *fd );

    sfxHandle_t (*trap_Snd_RegisterSfx)( const char *npath );
    void (*trap_Snd_PlaySfx)( sfxHandle_t sfx );
    void (*trap_Snd_StopSfx)( sfxHandle_t sfx );

    void (*trap_Cvar_Register)( vmCvar_t *vmCvar, const char *varName, const char *defaultValue, uint32_t flags );
    void (*trap_Cvar_Update)( vmCvar_t *vmCvar );
    void (*trap_Cvar_Set)( const char *var_name, const char *value );
    void (*trap_Cvar_VariableStringBuffer)( const char *var_name, char *buffer, uint32_t bufsize );
    uint32_t (*trap_Argc)( void );
    void (*trap_Argv)( uint32_t n, char *buffer, uint32_t bufferLength );
    void (*trap_Args)( char *buffer, uint32_t bufferLength );
};
#endif

#if 0
class CGame
{
public:
    CGame(void);
    ~CGame();

    void Init(void);
};
#endif

#if !defined(UI_HARD_LINKED) && !defined(SGAME_HARD_LINKED) && !defined(Q3_VM)

#include "../ui/ui_public.h"
#include "../engine/vm_local.h"

typedef struct {
    char currentmap[MAX_GDR_PATH];

    qboolean uiStarted;
    qboolean sgameStarted;
    qboolean soundStarted;
    qboolean rendererStarted;
    qboolean mapLoaded;

    gamestate_t state;
    int frametime;
    int framecount;
    int realtime;

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
    
    gpuConfig_t gpuConfig;
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
qboolean G_WindowMinimized( void );
void G_ClearState(void);
void G_FlushMemory(void);
void G_StartHunkUsers(void);
void G_ShutdownAll(void);
void G_ShutdownVMs(void);
void G_Frame(int msec, int realMsec);
void G_InitDisplay(gpuConfig_t *config);
SDL_Window *G_GetSDLWindow(void);
void GLimp_Minimize( void );

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
