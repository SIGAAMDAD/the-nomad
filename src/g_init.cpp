#include "n_shared.h"
#include "g_game.h"
#include <allegro5/allegro5.h>
#include <allegro5/allegro_opengl.h>
#include <allegro5/bitmap_io.h>
#include <allegro5/allegro_image.h>

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

static nomadvector<eastl::string> arglist;

static void I_InitArgs(int argc, char** argv)
{
    myargc = argc;
    myargv = argv;
    arglist.reserve(argc);
    for (int i = 0; i < argc; i++)
        arglist.emplace_back(argv[i]);
}

int I_GetParm(const char* name)
{
    if (!name)
        N_Error("I_GetParm: name is NULL");
    
    int i;
    for (i = 1; i < myargc; i++) {
        if (!arglist[i].compare(name))
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

void mainLoop()
{
    glEnable(GL_MULTISAMPLE);
    Vertex vertices[] = {
        Vertex(glm::vec3( 0.5f,  0.5f, 0.0f), glm::vec4(0.0f, 0.0f, 0.0f, 1.0f), glm::vec2(1.0f, 1.0f)),   // top right
        Vertex(glm::vec3( 0.5f, -0.5f, 0.0f), glm::vec4(0.0f, 0.0f, 0.0f, 1.0f), glm::vec2(1.0f, 0.0f)),   // bottom right
        Vertex(glm::vec3(-0.5f, -0.5f, 0.0f), glm::vec4(0.0f, 0.0f, 0.0f, 1.0f), glm::vec2(0.0f, 0.0f)),   // bottom left
        Vertex(glm::vec3(-0.5f,  0.5f, 0.0f), glm::vec4(0.0f, 0.0f, 0.0f, 1.0f), glm::vec2(0.0f, 1.0f))    // top left
    };
    uint32_t indices[] = {
        0, 1, 2,
        0, 2, 3,
    };

    SDL_Event event;
    memset(&event, 0, sizeof(event));
    VertexArray* vao = VertexArray::Create("vao");
    vao->Bind();
    VertexBuffer* vbo = VertexBuffer::Create(vertices, sizeof(vertices), "vbo");
    vbo->Bind();
    vbo->PushVertexAttrib(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void *)offsetof(Vertex, pos));
    vbo->PushVertexAttrib(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void *)offsetof(Vertex, color));
    vbo->PushVertexAttrib(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void *)offsetof(Vertex, texcoords));

    IndexBuffer* ibo = IndexBuffer::Create(indices, 6, GL_UNSIGNED_INT, "ibo");
    Shader* shader = Shader::Create("shader.glsl", "shader0");
    Framebuffer fbo;
    Texture2D* texture = Texture2D::Create("chernologo.png", "texture");
    fbo.Bind();
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, texture->GetID(), 0);

    vao->Unbind();
    vbo->Unbind();

    renderer->camera = (Camera *)Z_Malloc(sizeof(Camera), TAG_STATIC, &renderer->camera, "camera");
    new (renderer->camera) Camera(-3.0f, 3.0f, -3.0f, 3.0f);
    glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, 0));
    glm::mat4 mvp = renderer->camera->GetProjection() * renderer->camera->GetViewMatrix() * model;

    std::vector<glm::vec3> translations = {
        glm::vec3(1, 1.5, 0),
        glm::vec3(1, 2.5, 0),
        glm::vec3(3, 2.5, 0)
    };

    glClearColor(0.1f, 0.1f, 0.1f, 0.0f);
    
    Z_Print(true);

    std::chrono::high_resolution_clock::time_point next = std::chrono::high_resolution_clock::now();
    while (1) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                done();
            }
            switch (event.key.keysym.sym) {
            case SDLK_w:
                translations[0].y += .25;
                break;
            case SDLK_a:
                translations[0].x -= .25;
                break;
            case SDLK_s:
                translations[0].y -= .25;
                break;
            case SDLK_d:
                translations[0].x += .25;
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
        renderer->camera->CalculateViewMatrix();
        
        glViewport(0, 0, scf::renderer::width * 2, scf::renderer::height * 2);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        shader->Bind();
        vao->Bind();
        ibo->Bind();

        shader->SetMat4("u_ViewProjection", renderer->camera->GetVPM());

        next += std::chrono::milliseconds(1000 / scf::renderer::ticrate);

        fbo.Bind();
//        glFramebufferTexture(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, texture->GetID(), 0);
        glActiveTexture(GL_TEXTURE0);
        texture->Bind();
        shader->SetMat4("u_MVP", renderer->camera->GetProjection() * renderer->camera->GetViewMatrix() * glm::translate(glm::mat4(1.0f), translations[0]));
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, NULL);
        texture->Unbind();
        fbo.Unbind();

        texture->Bind();
        shader->SetMat4("u_MVP", renderer->camera->GetProjection() * renderer->camera->GetViewMatrix() * glm::translate(glm::mat4(1.0f), translations[1]));
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, NULL);
        texture->Unbind();

        shader->Unbind();
        ibo->Unbind();
        vao->Unbind();
        //for (size_t i = 1; i < translations.size(); i++) {
        //    model = glm::translate(glm::mat4(1.0f), translations[i]);
        //    mvp = renderer->camera->GetProjection() * renderer->camera->GetViewMatrix() * model;
        //    shader->SetMat4("u_MVP", mvp);
        //    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, NULL);
        //}
//
        //vao->Unbind();
        //ibo->Unbind();
        //shader->Unbind();
        //fbo.Unbind();
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
    I_InitArgs(argc, argv);

//    G_LoadBZip2();

    con.ConPrintf("setting up logger");
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
            fprintf(stdout,
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