#ifndef _G_GAME_
#define _G_GAME_

#pragma once

#include "g_bff.h"
#include "../engine/n_map.h"
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

#define PRINT_INFO 0
#define PRINT_DEVELOPER 1

//
// g_game
//
void G_Init(void);
void G_StartHunkUsers(void);
void G_Shutdown(void);
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

void I_NomadInit(int argc, char** argv);
void N_MainLoop();

#endif
