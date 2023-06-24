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

void VectorNormalize(const glm::vec3& in, glm::vec3& out)
{
    float ilength = Q_rsqrt(glm::dot(in, in));

    out.x = in.x * ilength;
    out.y = in.y * ilength;
    out.z = in.z * ilength;
}

void WorldToScreen(const glm::vec3& in, glm::vec3& out)
{
    const glm::ivec2 endpos = glm::ivec2(
        Game::Get()->cameraPos.x + 32,
        Game::Get()->cameraPos.y + 12
    );
    const glm::ivec2 startpos = glm::ivec2(
        Game::Get()->cameraPos.x - 32,
        Game::Get()->cameraPos.y - 12
    );

//    if ((in.x <= startpos.x || in.y <= startpos.y) || (in.x >= endpos.x || in.y >= endpos.y))
//        return; // not within sight

    glm::mat4 projection = glm::perspective((float)(M_PI/1.5f), (float)1024/(float)720, -1.0f, 1.0f);
    glm::mat4 view = glm::lookAt(
        glm::vec3(renderer->camera.GetPos().x, renderer->camera.GetPos().y, 2.0f),
        glm::vec3(renderer->camera.GetPos().x, renderer->camera.GetPos().y, 1.0f),
        glm::vec3(0, 1, 0)
    );
    glm::mat4 viewproj = view * projection;
    glm::mat4 model =
        glm::rotate(glm::mat4(), renderer->camera.GetRotation(), glm::vec3(0.0f, 0.0f, 1.0f)) *
        glm::translate(glm::mat4(), glm::vec3(in.x, in.y, 1.0f));
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
    glm::vec4 positions[] = {
        glm::vec4( 0.5f,  0.5f, 0.0f, 1.0f),
        glm::vec4( 0.5f, -0.5f, 0.0f, 1.0f),
        glm::vec4(-0.5f, -0.5f, 0.0f, 1.0f),
        glm::vec4(-0.5f,  0.5f, 0.0f, 1.0f),
    };
    glm::vec2 texcoords[] = {
        glm::vec2(0.0f, 0.0f),
        glm::vec2(0.0f, 1.0f),
        glm::vec2(1.0f, 1.0f),
        glm::vec2(1.0f, 0.0f),
    };

    vertex_t vertices[] = {
        vertex_t(glm::vec3( 0.5f,  0.5f, 0.0f), glm::vec2(0.0f, 0.0f),/*glm::vec2(1.0f, 1.0f)*/ glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)),   // top right
        vertex_t(glm::vec3( 0.5f, -0.5f, 0.0f), glm::vec2(0.0f, 1.0f),/*glm::vec2(1.0f, 0.0f)*/ glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)),   // bottom right
        vertex_t(glm::vec3(-0.5f, -0.5f, 0.0f), glm::vec2(1.0f, 1.0f),/*glm::vec2(0.0f, 0.0f)*/ glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)),   // bottom left
        vertex_t(glm::vec3(-0.5f,  0.5f, 0.0f), glm::vec2(1.0f, 0.0f),/*glm::vec2(0.0f, 1.0f)*/ glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)),   // top left
    };
    uint32_t indices[] = {
        0, 1, 2,
        2, 3, 0
    };

    vertex_t screenvertices[] = {
        vertex_t(glm::vec3( 1.0f,  1.0f, 0.0f), glm::vec2(0.0f, 0.0f), glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)), // top right
        vertex_t(glm::vec3( 1.0f, -1.0f, 0.0f), glm::vec2(0.0f, 1.0f), glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)), // bottom right
        vertex_t(glm::vec3(-1.0f, -1.0f, 0.0f), glm::vec2(1.0f, 1.0f), glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)), // bottom left
        vertex_t(glm::vec3(-1.0f,  1.0f, 0.0f), glm::vec2(1.0f, 0.0f), glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)), // top left
    };

    SDL_Event event;
    memset(&event, 0, sizeof(event));

    Game::Get()->cameraPos = glm::ivec3(120, 120, 0);

    vertexCache_t* screenCache = RGL_InitCache(screenvertices, arraylen(screenvertices), indices, arraylen(indices), GL_UNSIGNED_INT);
    vertexCache_t* cache = RGL_InitCache(vertices, arraylen(vertices), indices, arraylen(indices), GL_UNSIGNED_INT);
    
    shader_t* shader = R_CreateShader("gamedata/shader.glsl", "shader0");
    shader_t* screenShader = R_CreateShader("gamedata/framebuffer.glsl", "screenShader");

    texture_t* texture = R_GetTexture("NMTEX_SAND");
    texture_t* screenTexture = R_GetTexture("NMTEX_BKGD");
    SpriteSheet sheet("NMTEXSHEET", glm::vec2(16.0f, 16.0f), glm::vec2(256, 256), 2);


    renderer->camera = Camera(-3.0f, 3.0f, -3.0f, 3.0f);
    std::vector<glm::vec3> translations = {
        glm::vec3(0, 0, 0),
        glm::vec3(1, 0, 0),
        glm::vec3(0, .5, 0),
    };
    glm::vec3 pos = glm::vec3(0.0f, 0.0f, 0.0f);

    LoadLevel(BFF_FetchLevel("NMLVL0"));

    glClearColor(0.1f, 0.1f, 0.1f, 0.0f);
    
    Hunk_Print();
    Z_Print(true);
    Snd_PlayTrack("NMMUS01");

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
            pos.y += 1;
        if (evState.kbstate[KEY_A])
            pos.x -= 1;
        if (evState.kbstate[KEY_S])
            pos.y -= 1;
        if (evState.kbstate[KEY_D])
            pos.x += 1;
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

        renderer->camera.CalculateViewMatrix();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glViewport(0, 0, r_screenwidth.i, r_screenheight.i);

//        R_BeginFramebuffer();
        Con_RenderConsole();

        next = 1000 / r_ticrate.i;

        R_BindShader(screenShader);
        R_SetInt(screenShader, "u_ScreenTexture", 0);
        R_BindTexture(screenTexture, 0);

        for (uint32_t i = 0; i < 4; i++)
            screenvertices[i].texcoords = texcoords[i];

        RGL_SwapVertexData(screenvertices, arraylen(screenvertices), screenCache);
        RGL_SwapIndexData(indices, arraylen(indices), screenCache);
        RGL_DrawCache(screenCache);

        R_UnbindTexture();
        R_UnbindShader();

        R_BindShader(shader);
        R_SetMat4(shader, "u_ViewProjection", renderer->camera.GetVPM());
//        RE_DrawPints(&sheet, cache);

//        glm::mat4 model = glm::translate(glm::mat4(1.0f), translations[0]);
//        glm::mat4 mvp = renderer->camera.GetProjection() * renderer->camera.GetViewMatrix() * model;
//        for (uint32_t i = 0; i < 4; i++) {
//            vertices[i].pos = mvp * positions[i];
//            vertices[i].texcoords = texcoords[i];
//        }
        
        R_BindTexture(sheet.GetTexture());
        RGL_SwapIndexData(indices, arraylen(indices), cache);
        
        sheet.DrawSprite(SPR_ROCK, cache, glm::vec2(pos.x, pos.y), glm::vec2(1, 1));
        RGL_DrawCache(cache);

        R_UnbindShader();
        R_UnbindTexture();
        
        Con_EndFrame();
//        R_EndFramebuffer();

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