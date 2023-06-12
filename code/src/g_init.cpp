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

void mainLoop()
{
    glm::vec4 positions[] = {
        glm::vec4( 0.5f,  0.5f, 0.0f, 1.0f),
        glm::vec4( 0.5f, -0.5f, 0.0f, 1.0f),
        glm::vec4(-0.5f, -0.5f, 0.0f, 1.0f),
        glm::vec4(-0.5f,  0.5f, 0.0f, 1.0f),
    };
    Vertex vertices[] = {
        Vertex(glm::vec3( 0.5f,  0.5f, 0.0f), glm::vec4(0.0f, 0.0f, 0.0f, 1.0f), /*glm::vec2(1.0f, 0.5f)*/ glm::vec2(1.0f, 1.0f)),   // top right
        Vertex(glm::vec3( 0.5f, -0.5f, 0.0f), glm::vec4(0.0f, 0.0f, 0.0f, 1.0f), /*glm::vec2(1.0f, 0.0f)*/ glm::vec2(1.0f, 0.0f)),   // bottom right
        Vertex(glm::vec3(-0.5f, -0.5f, 0.0f), glm::vec4(0.0f, 0.0f, 0.0f, 1.0f), /*glm::vec2(0.0f, 0.0f)*/ glm::vec2(0.0f, 0.0f)),   // bottom left
        Vertex(glm::vec3(-0.5f,  0.5f, 0.0f), glm::vec4(0.0f, 0.0f, 0.0f, 1.0f), /*glm::vec2(0.0f, 0.5f)*/ glm::vec2(0.0f, 1.0f)),   // top left
    };
    uint32_t indices[] = {
        0, 1, 2,
        0, 2, 3
    };

    Vertex screenvertices[] = {
        Vertex(glm::vec3( 1.0f,  1.0f, 0.0f), glm::vec2(0.0f, 0.0f)), // top right
        Vertex(glm::vec3( 1.0f, -1.0f, 0.0f), glm::vec2(0.0f, 1.0f)), // bottom right
        Vertex(glm::vec3(-1.0f, -1.0f, 0.0f), glm::vec2(1.0f, 1.0f)), // bottom left
        Vertex(glm::vec3(-1.0f,  1.0f, 0.0f), glm::vec2(1.0f, 0.0f)), // top left
    };

    SDL_Event event;
    memset(&event, 0, sizeof(event));

    vertexCache_t* screenCache = R_CreateCache(screenvertices, sizeof(screenvertices), indices, sizeof(indices), "scache");
    R_PushVertexAttrib(screenCache, 0, GL_FLOAT, 3, sizeof(Vertex), (const void *)offsetof(Vertex, pos));
    R_PushVertexAttrib(screenCache, 1, GL_FLOAT, 2, sizeof(Vertex), (const void *)offsetof(Vertex, texcoords));

    vertexCache_t* cache = R_CreateCache(vertices, sizeof(vertices), indices, sizeof(indices), "vcache");
    R_PushVertexAttrib(cache, 0, GL_FLOAT, 3, sizeof(Vertex), (const void *)offsetof(Vertex, pos));
    R_PushVertexAttrib(cache, 1, GL_FLOAT, 4, sizeof(Vertex), (const void *)offsetof(Vertex, color));
    R_PushVertexAttrib(cache, 2, GL_FLOAT, 2, sizeof(Vertex), (const void *)offsetof(Vertex, texcoords));
    
    shader_t* shader = R_CreateShader("gamedata/shader.glsl", "shader0");
    shader_t* screenShader = R_CreateShader("gamedata/framebuffer.glsl", "screenShader");

    texture_t* texture = R_GetTexture("NMTEX_SAND");
    texture_t* screenTexture = R_GetTexture("NMTEX_BKGD");


    renderer->camera = CONSTRUCT(Camera, "camera", -3.0f, 3.0f, -3.0f, 3.0f);
    std::vector<glm::vec3> translations = {
        glm::vec3(0, -1, 0),
        glm::vec3(1, 0, 0),
        glm::vec3(0, .5, 0),
    };

    glClearColor(0.1f, 0.1f, 0.1f, 0.0f);
    
    Hunk_Print();
    Z_Print(true);
    Snd_PlayTrack("NMMUS01");

    float light_intensity = 1.0f;

    uint32_t ticrate = atoi(r_ticrate.value);
    uint64_t next = clock();
    while (1) {
        std::thread sndthread(G_RunSound);
        Com_UpdateEvents();
        vm_command = SGAME_RUNTIC;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                done();
            }
            if (event.type == SDL_KEYDOWN) {
                switch (event.key.keysym.sym) {
                case SDLK_o:
                    light_intensity += 0.1f;
                    break;
                case SDLK_p:
                    light_intensity -= 0.1f;
                    break;
                case SDLK_n:
                    renderer->camera->ZoomIn();
                    break;
                case SDLK_m:
                    renderer->camera->ZoomOut();
                    break;
                case SDLK_w:
                    translations[0].y += 0.25f;
                    break;
                case SDLK_a:
                    translations[0].x -= 0.25f;
                    break;
                case SDLK_s:
                    translations[0].y -= 0.25f;
                    break;
                case SDLK_d:
                    translations[0].x += 0.25;
                    break;
                case SDLK_q:
                    renderer->camera->RotateLeft();
                    break;
                case SDLK_e:
                    renderer->camera->RotateRight();
                    break;
                case SDLK_UP:
                    renderer->camera->MoveUp();
                    break;
                case SDLK_DOWN:
                    renderer->camera->MoveDown();
                    break;
                case SDLK_LEFT:
                    renderer->camera->MoveLeft();
                    break;
                case SDLK_RIGHT:
                    renderer->camera->MoveRight();
                    break;
                case SDLK_BACKQUOTE:
                    if (console_open)
                        console_open = qfalse;
                    else
                        console_open = qtrue;
                    break;
                };
            }
        }
        VM_Run(SGAME_VM);
        Con_RenderConsole();

        renderer->camera->CalculateViewMatrix();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glViewport(0, 0, N_atoi(r_screenwidth.value), N_atoi(r_screenheight.value));

        next = 1000 / ticrate;

        R_BindShader(screenShader);
        R_SetInt(screenShader, "u_ScreenTexture", 0);
        R_BindTexture(screenTexture, 0);

        R_BindVertexArray(screenCache);
        R_BindVertexBuffer(screenCache);
        R_BindIndexBuffer(screenCache);
        
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, NULL);

        R_UnbindVertexArray();
        R_UnbindVertexBuffer();
        R_UnbindIndexBuffer();
        R_UnbindTexture();
        R_UnbindShader();


        R_BindCache(cache);
        R_BindShader(shader);
        R_BindTexture(texture);
        R_SetMat4(shader, "u_ViewProjection", renderer->camera->GetVPM());

        {
            glm::mat4 model = glm::translate(glm::mat4(1.0f), translations[0]);
            glm::mat4 mvp = renderer->camera->GetProjection() * renderer->camera->GetViewMatrix() * model;
            R_SetMat4(shader, "u_MVP", mvp);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, NULL);
        }

        R_UnbindTexture();
        R_UnbindShader();
        R_UnbindVertexArray();
        R_UnbindVertexBuffer();
        R_UnbindIndexBuffer();
        
        VM_Stop(SGAME_VM);
        Con_EndFrame();
        sndthread.join();
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

    Con_Printf("G_LoadBFF: loading bff file");
    G_LoadBFF("nomadmain.bff");

    Con_Printf(
        "+===========================================================+\n"
         "\"The Nomad\" is free software distributed under the terms\n"
         "of both the GNU General Public License v2.0 and Apache License\n"
         "v2.0\n"
         "+==========================================================+\n"
    );

    Con_Printf("G_LoadSCF: parsing scf file");
    G_LoadSCF();

    Con_Printf("running main gameplay loop");
    mainLoop();
}