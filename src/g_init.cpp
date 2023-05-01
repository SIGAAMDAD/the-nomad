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
        if (strstr(name, myargv[i])) {
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

#if 1
static GLuint R_CompileShader(const std::string& src, GLenum type)
{
    GLuint id = glCreateShader(type);
    const char *buffer = src.c_str();
    glShaderSource(id, 1, &buffer, NULL);
    glCompileShader(id);
    int success;
    char infolog[512];
    glGetShaderiv(id, GL_COMPILE_STATUS, &success);
    if (success == GL_FALSE) {
        glGetShaderInfoLog(id, sizeof(infolog), NULL, infolog);
        fprintf(stderr, "Error: failed to compile shader: %s\n", infolog);
        glDeleteShader(id);
        return 0;
    }
    return id;
}

static GLuint R_AllocShader(const char *shaderfile)
{
    std::ifstream file(shaderfile, std::ios::in);
    if (!file) {
        fprintf(stderr, "Error: failed to open shader file %s\n", shaderfile);
        return 0;
    }
    std::string line;
    int index;
    std::stringstream stream[2];
    while (std::getline(file, line)) {
        if (line == "#type vertex")
            index = 0;
        else if (line == "#type fragment")
            index = 1;
        else
            stream[index] << line << '\n';
    }
    const std::string vertsrc = stream[0].str();
    const std::string fragsrc = stream[1].str();
    file.close();
    GLuint vertid = R_CompileShader(vertsrc, GL_VERTEX_SHADER);
    GLuint fragid = R_CompileShader(fragsrc, GL_FRAGMENT_SHADER);

    GLuint shader = glCreateProgram();
    glAttachShader(shader, vertid);
    glAttachShader(shader, fragid);
    glLinkProgram(shader);
    glValidateProgram(shader);
    glDeleteShader(vertid);
    glDeleteShader(fragid);
    return shader;
}

#endif

void mainLoop()
{
#if 1
    R_InitScene();
    Vertex vertices[] = {
        Vertex(glm::vec3( 0.5f,  0.5f, 0.0f), glm::vec4(1.0f, 1.0f, 0.0f, 0.0f)), // 0
        Vertex(glm::vec3(-0.5f,  0.5f, 0.0f), glm::vec4(1.0f, 0.0f, 1.0f, 0.0f)), // 1
        Vertex(glm::vec3(-0.5f, -0.5f, 0.0f), glm::vec4(1.0f, 2.0f, 1.0f, 0.0f)), // 2
        Vertex(glm::vec3( 0.5f, -0.5f, 0.0f), glm::vec4(1.0f, 0.0f, 1.0f, 0.0f)), // 3
        Vertex(glm::vec3( 0.5f,  0.5f, 0.0f), glm::vec4(1.0f, 1.0f, 0.0f, 0.0f)), // 0
        Vertex(glm::vec3(-0.5f, -0.5f, 0.0f), glm::vec4(1.0f, 2.0f, 1.0f, 0.0f)), // 2
        Vertex(glm::vec3( 0.5f, -0.5f, 0.0f), glm::vec4(1.0f, 0.0f, 1.0f, 0.0f)),
//        100.0f, 100.0f,
//        200.0f, 200.0f,
//        100.0f, 200.0f // 3
    };
    uint32_t indices[] = {
        0, 1, 2,
        0, 2, 3
    };

    nomadvector<Vertex> verts;

    SDL_Event event;
    memset(&event, 0, sizeof(event));
//    GLuint vao;
//    glGenVertexArrays(1, &vao);
//    glBindVertexArray(vao);
//
//    GLuint buffer;
//    glGenBuffers(1, &buffer);
//    glBindBuffer(GL_ARRAY_BUFFER, buffer);
//    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
//
//    glEnableVertexArrayAttrib(buffer, 0);
//    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 7, 0);
//
//    glEnableVertexArrayAttrib(buffer, 2);
//    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(float) * 7, (const void *)12);
//
//    GLuint ibo;
//    glGenBuffers(1, &ibo);
//    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
//    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
//    VertexArray vao;
//    VertexBuffer vbo(sizeof(vertices));
//    vbo.PushVertexAttrib(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void *)offsetof(Vertex, pos));
//    vbo.PushVertexAttrib(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void *)offsetof(Vertex, color));

//    IndexBuffer ibo(indices, 6);
//    ibo.Bind();
//
//    Shader shader("shader.glsl");
//    GLuint shader = R_AllocShader("shader.glsl");
//    glUseProgram(shader);
//    shader.Bind();

//    glm::mat4 proj = glm::ortho(0.0f, 960.0f, 0.0f, 540.0f, -1.0f, 1.0f);
//    glm::mat4 view = glm::translate(glm::mat4(1.0f), glm::vec3(-100, 0, 0));
//    glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(200, 200, 0));
//    glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, 0));
//    glm::mat4 mvp = renderer->camera->GetProjection() * renderer->camera->GetViewMatrix() * model;
//
    std::vector<glm::vec3> translations = {
        glm::vec3(1, 1.5, 0),
        glm::vec3(1, 2.5, 0),
        glm::vec3(3, 2.5, 0)
    };
//
//    shader.SetMat4("u_MVP", mvp);
//    GLint location = glGetUniformLocation(shader, "u_MVP");
//    glUniformMatrix4fv(location, 1, GL_FALSE, &mvp[0][0]);
//    glClearColor(0.1f, 0.1f, 0.1f, 0.0f);

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
            };
        }
        Renderer::BeginScene();
        Renderer::DrawQuad(glm::vec2(translations[0].x, translations[0].y), glm::vec2(10, 10), glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
        Renderer::EndScene();
//        renderer->camera->CalculateViewMatrix();
//        shader.SetMat4("u_ViewProjection", renderer->camera->GetVPM());
//        glClear(GL_COLOR_BUFFER_BIT);
//
//        for (auto &i : vertices) {
//            i.color.r += .1f;
//            i.color.g += .1f;
//        }
//
//        vbo.SetData(vertices, sizeof(vertices));
//        
//        for (const auto& i : translations) {
//            model = glm::translate(glm::mat4(1.0f), i);
//            mvp = renderer->camera->GetProjection() * renderer->camera->GetViewMatrix() * model;
//            shader.SetMat4("u_MVP", mvp);
//            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, NULL);
//        }
//        SDL_GL_SwapWindow(renderer->window);
//        verts.clear();
    }
#else
    glDisable(GL_CULL_FACE);
    glDisable(GL_)
    SDL_Event event;
    memset(&event, 0, sizeof(event));

    Vertex vertices[] = {
        Vertex(glm::vec3(-0.5f, -0.5f, 0.0f), glm::vec4(1.0f, 0.0f, 0.0f, 1.0f)),
        Vertex(glm::vec3( 0.5f, -0.5f, 0.0f), glm::vec4(1.0f, 0.0f, 0.0f, 1.0f)),
        Vertex(glm::vec3( 0.5f,  0.5f, 0.0f), glm::vec4(1.0f, 0.0f, 0.0f, 1.0f)),
        Vertex(glm::vec3(-0.5f, -0.5f, 0.0f), glm::vec4(1.0f, 0.0f, 0.0f, 1.0f)),
        Vertex(glm::vec3( 0.5f,  0.5f, 0.0f), glm::vec4(1.0f, 0.0f, 0.0f, 1.0f)),
        Vertex(glm::vec3(-0.5f,  0.5f, 0.0f), glm::vec4(1.0f, 0.0f, 0.0f, 1.0f)),
    };

    std::shared_ptr<VertexArray> vao = VertexArray::Create();
    std::shared_ptr<VertexBuffer> vbo = VertexBuffer::Create(vertices, sizeof(vertices));

    glEnableVertexArrayAttrib(vbo->id, 0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void *)offsetof(Vertex, pos));

    glEnableVertexArrayAttrib(vbo->id, 1);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void *)offsetof(Vertex, color));

    std::shared_ptr<Shader> shader = Shader::Create("shader.glsl");

    renderer->camera = (Camera *)Z_Malloc(sizeof(Camera), TAG_STATIC, &renderer->camera, "camera");
    new (renderer->camera) Camera(-2.0f, 2.0f, -2.0f, 2.0f);

    glm::vec3 pos(0);
    glm::mat4 model = glm::translate(glm::mat4(1.0f), pos);
    glm::mat4 mvp = renderer->camera->GetProjection() * renderer->camera->GetViewMatrix() * model;

//    shader->Unbind();
//    vao->Unbind();
//    vbo->Unbind();

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
    while (1) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                done();
            }
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
                    renderer->camera->GetRotation() -= renderer->camera->GetRotationSpeed();
                    if (renderer->camera->GetRotation() > 180.0f)
                        renderer->camera->GetRotation() -= 360.0f;
                    else if (renderer->camera->GetRotation() <= -180.0f)
                        renderer->camera->GetRotation() += 360.0f;
                    break;
                case SDLK_e:
                    renderer->camera->GetRotation() += renderer->camera->GetRotationSpeed();
                    if (renderer->camera->GetRotation() > 180.0f)
                        renderer->camera->GetRotation() -= 360.0f;
                    else if (renderer->camera->GetRotation() <= -180.0f)
                        renderer->camera->GetRotation() += 360.0f;
                    break;
                case SDLK_UP:
                    renderer->camera->GetPos().x += -sin(glm::radians(renderer->camera->GetRotation())) * renderer->camera->GetSpeed();
                    renderer->camera->GetPos().y += cos(glm::radians(renderer->camera->GetRotation())) * renderer->camera->GetSpeed();
                    break;
                case SDLK_DOWN:
                    renderer->camera->GetPos().x -= -sin(glm::radians(renderer->camera->GetRotation())) * renderer->camera->GetSpeed();
                    renderer->camera->GetPos().y -= cos(glm::radians(renderer->camera->GetRotation())) * renderer->camera->GetSpeed();
                    break;
                case SDLK_LEFT:
                    renderer->camera->GetPos().x -= cos(glm::radians(renderer->camera->GetRotation())) * renderer->camera->GetSpeed();
                    renderer->camera->GetPos().y -= sin(glm::radians(renderer->camera->GetRotation())) * renderer->camera->GetSpeed();
                    break;
                case SDLK_RIGHT:
                    renderer->camera->GetPos().x += cos(glm::radians(renderer->camera->GetRotation())) * renderer->camera->GetSpeed();
                    renderer->camera->GetPos().y += sin(glm::radians(renderer->camera->GetRotation())) * renderer->camera->GetSpeed();
                    break;
                };
                renderer->camera->
                glClear(GL_COLOR_BUFFER_BIT);

                shader->Bind();
                vao->Bind();
                vbo->Bind();

                model = glm::translate(glm::mat4(1.0f), pos);
                mvp = renderer->camera->GetProjection() * renderer->camera->GetViewMatrix() * model;

//                shader->SetMat4("u_MVP", mvp);
                glDrawArrays(GL_TRIANGLES, 0, 6);

                shader->Unbind();
                vao->Unbind();
                vbo->Unbind();

                SDL_GL_SwapWindow(renderer->window);
            }
        }
    }
#endif
}

void I_NomadInit(int argc, char** argv)
{
    myargc = argc;
    myargv = argv;

    xalloc_init();

//    G_LoadBZip2();

    con.ConPrintf("setting up logger");
    Log::Init();

    int i = I_GetParm("-bff");
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