#include "n_shared.h"
#include "g_game.h"
#include "../common/n_vm.h"

bool sdl_on = false;
static bool exited = false;
int myargc;
char** myargv;

#ifndef _WIN32
#define LoadLibraryA(x) dlopen((x), RTLD_NOW | RTLD_LOCAL)
#define GetProcAddress(a,b) dlsym((a),(b))
#define FreeLibrary(x) dlclose((x))
using HMODULE = void*;
#endif

HMODULE G_LoadLibrary(const char *lib)
{
    HMODULE handle;
#ifdef _WIN32
    if ((handle = GetModuleHandleA(lib)) != NULL)
        return (void *)NULL;
#elif defined(__unix__)
    if (*lib == '\0')
        return (void *)NULL;
#endif
    handle = LoadLibraryA(lib);
    return handle;
}

void *G_LoadSym(HMODULE handle, const char *name)
{
    return (void *)GetProcAddress((HMODULE)handle, name);
}

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
        if (!strcasecmp(myargv[i], parm))
            return i;
    }
    return -1;
}

void __attribute__((noreturn)) N_Error(const char *err, ...)
{
    char msg[2048];
    memset(msg, 0, sizeof(msg));
    va_list argptr;
    va_start(argptr, err);
    stbsp_vsnprintf(msg, sizeof(msg) - 1, err, argptr);
    va_end(argptr);
    if (sdl_on) {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Engine Error (Fatal)", msg, Game::Get()->window);
    }
    Con_Error("Error: %s", msg);
    Con_Flush();
    Game::Get()->~Game();
    Log::GetLogger()->flush();
    exit(EXIT_FAILURE);
}


void ImGui_Init()
{
    Con_Printf("ImGui_Init: initializing and allocating an ImGui_SDLRenderer context");
    IMGUI_CHECKVERSION();
    imgui_on = true;
}

void VectorNormalize(glm::vec3& v)
{
    float ilength = Q_rsqrt((v.x*v.x+v.y*v.y+v.z*v.z));

    v.x *= ilength;
    v.y *= ilength;
    v.z *= ilength;
}

void done()
{
    Game::Get()->~Game();
    exit(EXIT_SUCCESS);
}

static void R_VertexTransforms(const glm::vec3& pos,
    Vertex* vertices, int numvertices, int offset, const glm::vec4* positions, float scale)
{
    glm::mat4 transform = glm::translate(glm::mat4(1.0f), pos) * glm::scale(glm::mat4(1.0f), glm::vec3(scale));
    for (int i = 0; i < numvertices; i++)
        vertices[i + offset].pos = transform * positions[i];
}

void mainLoop()
{
    SDL_GL_SetAttribute(SDL_GL_FRAMEBUFFER_SRGB_CAPABLE, 1);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 16);
    glEnable(GL_MULTISAMPLE);
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

    SDL_Event event;
    memset(&event, 0, sizeof(event));
    VertexArray* vao = VertexArray::Create("vao");
    vao->Bind();
    VertexBuffer* vbo = VertexBuffer::Create(vertices, sizeof(vertices), "vbo");
    vbo->Bind();
    vbo->PushVertexAttrib(vao, 0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void *)offsetof(Vertex, pos));
    vbo->PushVertexAttrib(vao, 1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void *)offsetof(Vertex, color));
    vbo->PushVertexAttrib(vao, 2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void *)offsetof(Vertex, texcoords));

    IndexBuffer* ibo = IndexBuffer::Create(indices, 6, GL_UNSIGNED_INT, "ibo");
    vbo->Unbind();
    vao->Unbind();
    Shader* shader = Shader::Create("shader.glsl", "shader0");

    Framebuffer* fbo = Framebuffer::Create("fbo");
    Texture2D* texture = Texture2D::Create(DEFAULT_TEXTURE_SETUP, "sand.jpg", "texture0");
    Texture2D* screenTexture = Texture2D::Create(DEFAULT_TEXTURE_SETUP, "desertbkgd.jpg", "screenTexture");
    fbo->SetScreenTexture(screenTexture);


    renderer->camera = CONSTRUCT(Camera, "camera", -3.0f, 3.0f, -3.0f, 3.0f);
    std::vector<glm::vec3> translations = {
        glm::vec3(0, -1, 0),
        glm::vec3(1, 0, 0),
        glm::vec3(0, .5, 0),
    };

    glClearColor(0.1f, 0.1f, 0.1f, 0.0f);
    
    Z_Print(true);

    float light_intensity = 1.0f;

    std::chrono::high_resolution_clock::time_point next = std::chrono::high_resolution_clock::now();
    while (1) {
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
                };
            }
        }
        renderer->camera->CalculateViewMatrix();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glViewport(0, 0, scf::renderer::width, scf::renderer::height);

        fbo->SetBuffer();
//        {
//            screenTexture->Bind();
//            glm::mat4 model = glm::translate(glm::mat4(1.0f), translations[1]);
//            glm::mat4 mvp = renderer->camera->GetProjection() * renderer->camera->GetViewMatrix() * model;
//            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, NULL);
//            screenTexture->Unbind();
//        }
        next += std::chrono::milliseconds(1000 / scf::renderer::ticrate);
        
        fbo->SetDefault();

        shader->Bind();
        vao->Bind();
        ibo->Bind();
        
        shader->SetMat4("u_ViewProjection", renderer->camera->GetVPM());

        {
            texture->Bind(0);
            glm::mat4 model = glm::translate(glm::mat4(1.0f), translations[0]);
            glm::mat4 mvp = renderer->camera->GetProjection() * renderer->camera->GetViewMatrix() * model;
            shader->SetMat4("u_MVP", mvp);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, NULL);
            texture->Unbind();
        }

        ibo->Unbind();
        vao->Unbind();
        shader->Unbind();

        SDL_GL_SwapWindow(renderer->window);

        std::this_thread::sleep_until(next);
    }
}

void __attribute__((constructor)) Init_Memory()
{
    xalloc_init();
}

void I_NomadInit(int argc, char** argv)
{
    myargc = argc;
    myargv = argv;

//    G_LoadBZip2();

    Con_Printf("setting up logger");
    Log::Init();

    int i =  I_GetParm("-bff");
    if (i != -1) {
        bff_mode = true;

        bool operations[4];
        memset(operations, false, sizeof(operations));
        if (strstr(myargv[i], "help"))
            operations[0] = true;
        else if (strstr(myargv[i], "write"))
            operations[1] = true;
        else if (strstr(myargv[i], "read"))
            operations[2] = true;
        else if (strstr(myargv[i], "extract"))
            operations[3] = true;

        if (operations[0] || (!operations[1] && !operations[2] && !operations[3])) {
            Con_Printf(
                "%s -bff [operation] <arguments...>\n"
                "operations:\n"
                "  write [output] [dirpath]    write a bff file given an output file and a directory path\n"
                "  extract [input]             extract a written bff file to the game's file tree to use as a mod\n",
            myargv[0]);
            exit(EXIT_SUCCESS);
        }
        
        if (operations[1]) {
            if (myargc < (i + 1) || myargc < (i + 2)) {
                N_Error("output and/or dirpath not provided to bff write operations, aborting.");
            }
            G_WriteBFF(myargv[i + 1], myargv[i + 2]);
        }
        else if (operations[3]) {
            if (myargc < (i + 1)) {
                N_Error("input file must be provided to bff extract operations, aborting.");
            }
            G_ExtractBFF(myargv[i + 1]);
        }
    }
    Con_Printf("G_LoadBFF: loading bff file");
    G_LoadBFF("nomadmain.bff");

    fprintf(stdout,
        "+===========================================================+\n"
         "\"The Nomad\" is free software distributed under the terms\n"
         "of both the GNU General Public License v2.0 and Apache License\n"
         "v2.0\n"
         "+==========================================================+\n"
    );

    Con_Printf("G_LoadSCF: parsing scf file");
    G_LoadSCF();

    VM_Init();

//    LOG_INFO("setting up imgui");
//    ImGui_Init();

    Con_Flush();


    LOG_INFO("running main gameplay loop");
    mainLoop();
}