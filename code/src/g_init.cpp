#include "../engine/n_shared.h"
#include "../engine/n_sound.h"
#include "g_bff.h"
#include "g_game.h"
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
    extern cvar_t *com_frameTime;
    Game::Get()->cameraPos = glm::ivec3(0, 0, 0);
    
    Z_Print(true);
    Hunk_Print();

    float light_intensity = 1.0f;

    int32_t r_ticrate;
    uint64_t next = clock();
    Com_TouchMemory();
    while (1) {
        Hunk_Check();
        Com_EventLoop();
        vm_command = SGAME_RUNTIC;
        VM_Run(SGAME_VM);
        RE_BeginFrame();

        r_ticrate = Cvar_VariableInteger("r_ticrate");
        next = 1000 / r_ticrate;

        Snd_Submit();
        VM_Stop(SGAME_VM);

        Con_GetInput();
        RE_EndFrame();
        sleepfor(next);
        
        com_frameTime->i++;
    }
}

void I_NomadInit(void)
{
    Com_Init();

    Con_Printf("running main gameplay loop");
    mainLoop();
}