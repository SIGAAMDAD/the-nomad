#include "n_shared.h"
#include "g_game.h"

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

#if 0
void G_LoadBZip2()
{
    constexpr const char* libname = "Files/deps/libbz2.so.1.0.4";
    HMODULE handle = G_LoadLibrary(libname);
    
    LOAD(bzip2_bufcompress, "BZ2_bzBuffToBuffCompress");
    LOAD(bzip2_bufdecompress, "BZ2_bzBuffToBuffDecompress");
}
void G_LoadSndFile()
{
    constexpr const char* libname = "Files/deps/libsndfile.so.1.0.31";
    HMODULE handle = G_LoadLibrary(libname);

    LOAD(sndfile_open, "sf_open");
    LOAD(sndfile_close, "sf_close");
    LOAD(sndfile_readshort, "sf_read_short");
}
#endif

int I_GetParm(const char* name)
{
    if (myargc < 1)
        return -1;
    for (int i = 1; i < myargc; ++i) {
        if (strncmp(name, myargv[i], strlen(name))) {
            return i;
        }
    }
    return -1;
}

void __attribute__((__noreturn__)) N_Error(const char *err, ...)
{
    char msg[1024];
    memset(msg, 0, sizeof(msg));
    va_list argptr;
    va_start(argptr, err);
    stbsp_vsnprintf(msg, sizeof(msg), err, argptr);
    va_end(argptr);
    if (sdl_on) {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Engine Error (Fatal)", msg, Game::Get()->window);
    }
    con.ConError("Error: {}", msg);
    con.ConFlush();
    Game::Get()->~Game();
    Log::GetLogger()->flush();
    exit(EXIT_FAILURE);
}


void ImGui_Init()
{
    con.ConPrintf("ImGui_Init: initializing and allocating an ImGui_SDLRenderer context");
    IMGUI_CHECKVERSION();

//    Game::Get()->context = ImGui::CreateContext();
//    ImGui::SetCurrentContext(Game::Get()->context);
//    ImGui_ImplSDL2_InitForSDLRenderer(renderer->SDL_window, R_GetRenderer());
//    ImGui_ImplSDLRenderer_Init(R_GetRenderer());
    imgui_on = true;
}


void done()
{
    Game::Get()->~Game();
    exit(EXIT_SUCCESS);
}

void mainLoop()
{
    R_InitScene();
    Vertex vertices[] = {
        Vertex(glm::vec3(-1.5f, -0.5f, 0.0f), glm::vec4(1.0f, 1.0f, 0.0f, 6.0f)), // 0
        Vertex(glm::vec3(-0.5f, -0.5f, 0.0f), glm::vec4(1.0f, 0.0f, 5.0f, 0.0f)), // 1
        Vertex(glm::vec3(-0.5f,  0.5f, 0.0f), glm::vec4(1.0f, 2.0f, 0.0f, 0.0f)), // 2
        Vertex(glm::vec3(-1.5f,  0.5f, 0.0f), glm::vec4(1.0f, 1.0f, 0.0f, 6.0f)), // 3

        Vertex(glm::vec3( 0.5f, -0.5f, 0.0f), glm::vec4(1.0f, 1.0f, 0.0f, 0.0f)), // 4
        Vertex(glm::vec3( 1.5f, -0.5f, 0.0f), glm::vec4(1.0f, 0.0f, 1.0f, 0.0f)), // 5
        Vertex(glm::vec3( 1.5f,  0.5f, 0.0f), glm::vec4(1.0f, 2.0f, 1.0f, 0.0f)), // 6
        Vertex(glm::vec3( 0.5f,  0.5f, 0.0f), glm::vec4(1.0f, 0.0f, 1.0f, 0.0f)), // 7

        Vertex(glm::vec3( 0.5f, -0.5f, 0.0f), glm::vec4(0.0f, 1.0f, 0.0f, 0.0f)), // 8
        Vertex(glm::vec3( 1.5f, -0.5f, 0.0f), glm::vec4(0.0f, 0.0f, 1.0f, 0.0f)), // 9
        Vertex(glm::vec3( 1.5f,  0.5f, 0.0f), glm::vec4(0.0f, 2.0f, 1.0f, 0.0f)), // 10
        Vertex(glm::vec3( 0.5f,  0.5f, 0.0f), glm::vec4(0.0f, 0.0f, 1.0f, 0.0f)), // 11
    };
    uint32_t indices[] = {
        4, 5, 6,  4,  6,  7,
        8, 9, 10, 8, 10, 11,
    };
    std::shared_ptr<VertexArray> vao = std::make_shared<VertexArray>(vertices, arraylen(vertices), indices, arraylen(indices));
    std::shared_ptr<VertexBuffer>& vbo = vao->GetVBO().front();
    std::shared_ptr<IndexBuffer>& ibo = vao->GetIBO();
    vao->Unbind();
    vbo->Unbind();
    ibo->Unbind();

    SDL_Event event;
    memset(&event, 0, sizeof(event));

    std::shared_ptr<Shader> shader = std::make_shared<Shader>("shader.glsl");

    uint32_t pindices[] = {
        0, 1, 2, 0, 2, 3
    };

    Camera camera(-3.0f, 3.0f, -3.0f, 3.0f);

    std::vector<glm::vec3> translations = {
        glm::vec3(1, 1, 0),
        glm::vec3(2, 3, 0)
    };
    std::vector<glm::mat4> mvps(translations.size());

    glClearColor(0.1f, 0.1f, 0.1f, 0.0f);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    GLenum drawmode = GL_TRIANGLES;
#ifdef _NOMAD_DEBUG
    if (myargc > 1) {
        if (strstr(myargv[1], "0"))
            drawmode = GL_TRIANGLES;
        else if (strstr(myargv[1], "1"))
            drawmode = GL_LINES;
        else if (strstr(myargv[1], "2"))
            drawmode = GL_LINE_LOOP;
        else if (strstr(myargv[1], "3"))
            drawmode = GL_LINE_STRIP;
        else if (strstr(myargv[1], "4"))
            drawmode = GL_TRIANGLE_FAN;
        else if (strstr(myargv[1], "5"))
            drawmode = GL_PATCHES;
        else if (strstr(myargv[1], "6"))
            drawmode = GL_TRIANGLE_STRIP;
        else if (strstr(myargv[1], "7"))
            drawmode = GL_TRIANGLES_ADJACENCY;
        else if (strstr(myargv[1], "8"))
            drawmode = GL_LINE_STRIP_ADJACENCY;
    }
#endif
    vao->Bind();
    vbo->Bind();
    ibo->Bind();
    shader->Bind();
    glm::vec3 pos(0, 0, 0);
    glm::ivec2 mousepos;
    glm::vec3 mousepos_f;
    while (1) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                done();
            }
#if 0 // works but too fast and sensitive
            else if (event.type == SDL_MOUSEMOTION) {
                SDL_GetMouseState(&mousepos.x, &mousepos.y);
                mousepos_f.y = (float)mousepos.y;
                mousepos_f.x = (float)mousepos.x;

                // this is where that high school trig comes in handy
                float adjacent = disBetweenOBJ(translations[0], mousepos_f);
                float opposite = tan(90) * adjacent;

                camera.GetRotation() = -opposite / M_PI;
            }
#endif
            else if (event.type == SDL_KEYDOWN) {
                switch (event.key.keysym.sym) {
                case SDLK_w:
                    pos.y += 0.25f;
                    break;
                case SDLK_a:
                    pos.x -= 0.25f;
                    break;
                case SDLK_s:
                    pos.y -= 0.25f;
                    break;
                case SDLK_d:
                    pos.x += 0.25f;
                    break;
                case SDLK_q:
                    camera.GetRotation() -= camera.GetRotationSpeed();
                    if (camera.GetRotation() > 180.0f)
                        camera.GetRotation() -= 360.0f;
                    else if (camera.GetRotation() <= -180.0f)
                        camera.GetRotation() += 360.0f;
                    break;
                case SDLK_e:
                    camera.GetRotation() += camera.GetRotationSpeed();
                    if (camera.GetRotation() > 180.0f)
                        camera.GetRotation() -= 360.0f;
                    else if (camera.GetRotation() <= -180.0f)
                        camera.GetRotation() += 360.0f;
                    break;
                case SDLK_UP:
                    camera.GetPos().x += -sin(glm::radians(camera.GetRotation())) * camera.GetSpeed();
                    camera.GetPos().y += cos(glm::radians(camera.GetRotation())) * camera.GetSpeed();
                    break;
                case SDLK_DOWN:
                    camera.GetPos().x -= -sin(glm::radians(camera.GetRotation())) * camera.GetSpeed();
                    camera.GetPos().y -= cos(glm::radians(camera.GetRotation())) * camera.GetSpeed();
                    break;
                case SDLK_LEFT:
                    camera.GetPos().x -= cos(glm::radians(camera.GetRotation())) * camera.GetSpeed();
                    camera.GetPos().y -= sin(glm::radians(camera.GetRotation())) * camera.GetSpeed();
                    break;
                case SDLK_RIGHT:
                    camera.GetPos().x += cos(glm::radians(camera.GetRotation())) * camera.GetSpeed();
                    camera.GetPos().y += sin(glm::radians(camera.GetRotation())) * camera.GetSpeed();
                    break;
                };
            }
        }
        glClear(GL_COLOR_BUFFER_BIT);
        camera.CalculateViewMatrix();
        {
            ibo->SwapBuffer(indices, 6);
            vbo->SwapBuffer(vertices, 4);
            shader->UniformMat4("u_ViewProjection", camera.GetVPM());
            shader->UniformMat4("u_Transform", glm::translate(glm::mat4(1.0f), pos));
            ibo->Draw(GL_TRIANGLES, 6);
        }
        {
            shader->UniformMat4("u_ViewProjection", camera.GetVPM());
            shader->UniformMat4("u_Transform", glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f)));
            ibo->SwapBuffer(pindices, 6);
            glBufferSubData(GL_ARRAY_BUFFER, 4, 8, vertices);
            ibo->Draw(GL_TRIANGLES, ibo->numindices());
        }
        SDL_GL_SwapWindow(renderer->window);
    }
}

void I_NomadInit(int argc, char** argv)
{
    myargc = argc;
    myargv = argv;

//    G_LoadBZip2();

    con.ConPrintf("setting up logger");
    Log::Init();

    int i = I_GetParm("-bff=");
    if (i != -1 && argc > 2) {
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
            fprintf(stdout,
                "%s -bff=[operation] <arguments...>\n"
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

    LOG_INFO("initiazing renderer");
    R_Init();

    con.ConPrintf("G_LoadBFF: loading bff file");
    G_LoadBFF("nomadmain.bff");

    fprintf(stdout,
        "+===========================================================+\n"
         "\"The Nomad\" is free software distributed under the terms\n"
         "of both the GNU General Public License v2.0 and Apache License\n"
         "v2.0\n"
         "+==========================================================+\n"
    );

    con.ConPrintf("G_LoadSCF: parsing scf file");
    G_LoadSCF();

//    LOG_INFO("setting up imgui");
//    ImGui_Init();

    con.ConFlush();


    LOG_INFO("running main gameplay loop");
    mainLoop();
}