#include "n_shared.h"
#include "g_bff.h"
#include "g_game.h"
#include "g_sound.h"
#include "../rendergl/rgl_public.h"
#include "../sgame/sg_public.h"
#include "../common/n_vm.h"

bool sdl_on = false;
static bool exited = false;
int myargc;
char** myargv;

static void CameraGame_f(void)
{
    Con_Printf(
        "CameraPos.x: %i\n"
        "CameraPos.y: %i", Game::Get()->cameraPos.x, Game::Get()->cameraPos.y);
}

void mainLoop()
{
    Game::Get()->cameraPos = glm::ivec3(0, 0, 0);
    
    Z_Print(true);
    Snd_PlayTrack("NMMUS01");

    float light_intensity = 1.0f;

    uint64_t next = clock();
    Cmd_AddCommand("cameraGame", CameraGame_f);
    while (1) {
        Com_UpdateEvents();
        qboolean *kbstate = Com_GetKeyboard();
        vm_command = SGAME_RUNTIC;
        if (kbstate[KEY_W])
            Game::Get()->cameraPos.y -= 3;
        if (kbstate[KEY_A])
            Game::Get()->cameraPos.x -= 3;
        if (kbstate[KEY_S])
            Game::Get()->cameraPos.y += 3;
        if (kbstate[KEY_D])
            Game::Get()->cameraPos.x += 3;
        
        VM_Run(SGAME_VM);
        
        RE_BeginFrame();

        next = 1000 / r_ticrate.i;

        Snd_Submit();
        VM_Stop(SGAME_VM);

        RE_EndFrame();
        sleepfor(next);
    }
}

void I_NomadInit(void)
{
    Com_Init();

    Con_Printf("running main gameplay loop");
    mainLoop();
}