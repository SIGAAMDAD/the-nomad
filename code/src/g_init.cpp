#include "n_shared.h"
#include "g_bff.h"
#include "g_game.h"
#include "g_sound.h"
#include "m_renderer.h"
#include "../sgame/sg_public.h"
#include "../common/n_vm.h"

bool sdl_on = false;
static bool exited = false;
int myargc;
char** myargv;


#define LOAD(ptr,name) \
{ \
    *((void **)&ptr) = G_LoadSym(handle,name); \
    if (!ptr) N_Error("failed to load library symbol %s", name); \
}

int I_GetParm(const char *parm)
{
    if (!parm)
        N_Error("I_GetParm: parm is NULL");

    for (int i = 1; i < myargc; i++) {
        if (N_strcasecmp(myargv[i], parm))
            return i;
    }
    return -1;
}

void __attribute__((noreturn)) N_Error(const char *err, ...)
{
    char msg[1024];
    memset(msg, 0, sizeof(msg));
    va_list argptr;
    va_start(argptr, err);
    stbsp_vsnprintf(msg, sizeof(msg) - 1, err, argptr);
    va_end(argptr);
    Con_Error("%s", msg);

    Game::Get()->~Game();
    exit(EXIT_FAILURE);
}

static void done()
{
    Game::Get()->~Game();
    exit(EXIT_SUCCESS);
}

void LoadLevel(bfflevel_t *lvl)
{
    for (uint32_t y = 0; y < MAP_MAX_Y; ++y) {
        for (uint32_t x = 0; x < MAP_MAX_X; ++x) {
            switch (lvl->tilemap[y][x]) {
            case '#':
            case '.':
                Game::Get()->c_map[y][x] = SPR_ROCK;
                break;
            };
        }
    }
}

void mainLoop()
{
    SDL_Event event;
    memset(&event, 0, sizeof(event));

    Game::Get()->cameraPos = glm::ivec3(120, 120, 0);

    std::vector<glm::vec3> translations = {
        glm::vec3(0, 0, 0),
        glm::vec3(1, 0, 0),
        glm::vec3(0, .5, 0),
    };
    glm::vec3 pos = glm::vec3(0.0f, 0.0f, 0.0f);

    LoadLevel(BFF_FetchLevel("NMLVL0"));

    glClearColor(0.1f, 0.1f, 0.1f, 0.0f);

    texture_t *screenTexture = R_GetTexture("NMTEX_BKGD");
    
    Hunk_Print();
    Z_Print(true);
    Snd_PlayTrack("NMMUS01");
    RE_InitFrameData();

    float light_intensity = 1.0f;

    uint64_t next = clock();
    while (1) {
        Com_UpdateEvents();
        vm_command = SGAME_RUNTIC;
        if (evState.kbstate[KEY_O])
            light_intensity += 0.1f;
        if (evState.kbstate[KEY_P])
            light_intensity -= 0.1f;
        if (evState.kbstate[KEY_N])
            renderer->camera.ZoomIn();
        if (evState.kbstate[KEY_M])
            renderer->camera.ZoomOut();
        if (evState.kbstate[KEY_W])
            pos.y += 3;
        if (evState.kbstate[KEY_A])
            pos.x -= 3;
        if (evState.kbstate[KEY_S])
            pos.y -= 3;
        if (evState.kbstate[KEY_D])
            pos.x += 3;
        if (evState.kbstate[KEY_Q])
            renderer->camera.RotateLeft();
        if (evState.kbstate[KEY_E])
            renderer->camera.RotateRight();
        if (evState.kbstate[KEY_UP])
            renderer->camera.MoveUp();
        if (evState.kbstate[KEY_DOWN])
            renderer->camera.MoveDown();
        if (evState.kbstate[KEY_LEFT])
            renderer->camera.MoveLeft();
        if (evState.kbstate[KEY_RIGHT])
            renderer->camera.MoveRight();

        VM_Run(SGAME_VM);

        RE_BeginFrame();
        RE_BeginFramebuffer();
        renderer->camera.CalculateViewMatrix();
//        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//        glViewport(0, 0, r_screenwidth.i, r_screenheight.i);

        next = 1000 / r_ticrate.i;
        
        RE_CmdDrawSprite(SPR_PLAYR, glm::vec2(pos.x, pos.y), glm::vec2(.5));
        RE_EndFrame();
        RE_EndFramebuffer();

        Con_RenderConsole();
        Con_EndFrame();

        Snd_Submit();
        VM_Stop(SGAME_VM);
        SDL_GL_SwapWindow(renderer->window);
        
        sleepfor(next);
//        std::this_thread::sleep_until(next);
    }
}

void I_NomadInit(int argc, char** argv)
{
    myargc = argc;
    myargv = argv;

    Com_Init();

    Con_Printf("running main gameplay loop");
    mainLoop();
}