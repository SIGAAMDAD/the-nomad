#ifndef _G_GAME_
#define _G_GAME_

#pragma once

#include "g_bff.h"
#include "../engine/n_map.h"

#ifndef O_BINARY
#define O_BINARY 0
#endif

typedef enum
{
    GS_MENU,
    GS_LEVEL,
    GS_PAUSE,
    GS_SETTINGS,

    NUMGAMESTATES
} gamestate_t;

enum
{
    OCC_NORMY,
    OCC_EASTER,
    OCC_CHRISTMAS,
    OCC_HALLOWEEN,
    OCC_THANKSGIVING
};


#ifdef __cplusplus
class Game
{
private:
    static Game* gptr;
public:
    nmap_t *c_map;
    
    gamestate_t gamestate;
    gamedif_t difficulty;
    glm::ivec3 cameraPos;
public:
    Game(void) = default;
    ~Game();

    Game(Game &&) = delete;
    Game(const Game &) = delete;

    static void Alloc(void);
    static inline Game* Get(void) { return gptr; }
};

void G_Init(void);
void G_Shutdown(void);
void G_ClearMem(void);
void G_Restart(void);
void G_SGameRender(void);
qboolean G_GameCommand(void);
void G_InitSGame(void);

extern vm_t *sgvm;

void I_NomadInit(int argc, char** argv);
void N_MainLoop();
#endif

#endif
