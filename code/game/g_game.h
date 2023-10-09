#ifndef _G_GAME_
#define _G_GAME_

#pragma once

#include "../sgame/sg_public.h"
#include "../ui/ui_public.h"
#include "../rendergl/rgl_public.h"

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

typedef struct {
    char currentmap[MAX_GDR_PATH];

    qboolean uiStarted;
    qboolean sgameStarted;
    qboolean soundStarted;
    qboolean rendererStarted;
    qboolean mapLoaded;

    gamestate_t state;
    uint64_t frametime;
    uint64_t realtime;
} gameInfo_t;

extern gameInfo_t gi;

// logging parameters
#define PRINT_INFO 0
#define PRINT_DEVELOPER 1

// non api specific renderer cvars
extern cvar_t *cl_title;
extern cvar_t *vid_xpos;
extern cvar_t *vid_ypos;
extern cvar_t *r_allowSoftwareGL;
extern cvar_t *g_renderer; // current rendering api
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

//
// g_game
//
void G_Init(void);
void G_StartHunkUsers(void);
void G_Shutdown(qboolean quit);
void G_ClearMem(void);
void G_Restart(void);
qboolean G_GameCommand(void);
void G_ClearState(void);
void G_FlushMemory(void);
void G_StartHunkUsers(void);
void G_ShutdownAll(void);
void G_ShutdownVMs(void);
void G_Frame(uint64_t msec, uint64_t realMsec);

//
// g_ui
//
void G_ShutdownUI(void);
void G_InitUI(void);

//
// g_sgame
//
void G_ShutdownSGame(void);
void G_InitSGame(void);

extern vm_t *sgvm;
extern vm_t *uivm;
extern renderExport_t re;

#endif
