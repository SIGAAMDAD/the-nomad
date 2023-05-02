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
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    Vertex vertices[] = {
        Vertex(glm::vec3(-0.5f, -0.5f, 0.0f), glm::vec4(1.0f, 1.0f, 0.0f, 0.0f), glm::vec2(-0.5f, -0.5f)), // 0
        Vertex(glm::vec3( 0.5f, -0.5f, 0.0f), glm::vec4(1.0f, 0.0f, 1.0f, 0.0f), glm::vec2( 0.5f, -0.5f)), // 1
        Vertex(glm::vec3( 0.5f,  0.5f, 0.0f), glm::vec4(1.0f, 2.0f, 1.0f, 0.0f), glm::vec2( 0.5f,  0.5f)), // 2
        Vertex(glm::vec3(-0.5f,  0.5f, 0.0f), glm::vec4(1.0f, 0.0f, 1.0f, 0.0f), glm::vec2(-0.5f,  0.5f)), // 3

//        Vertex(glm::vec3(-1.5f, -1.5f, 0.0f), glm::vec4(1.0f, 1.0f, 0.0f, 0.0f)), // 4
//        Vertex(glm::vec3( 1.5f, -1.5f, 0.0f), glm::vec4(1.0f, 0.0f, 1.0f, 0.0f)), // 5
//        Vertex(glm::vec3( 1.5f,  1.5f, 0.0f), glm::vec4(1.0f, 2.0f, 1.0f, 0.0f)), // 6
//        Vertex(glm::vec3(-1.5f,  1.5f, 0.0f), glm::vec4(1.0f, 0.0f, 1.0f, 0.0f)), // 7
    };
    uint32_t indices[] = {
        0, 1, 2,
        0, 2, 3,
    };
    IMG_Init(IMG_INIT_JPG | IMG_INIT_PNG);

    //SDL_Renderer* render = SDL_CreateRenderer(renderer->window, -1, SDL_RENDERER_TARGETTEXTURE);
    //assert(render);
    //SDL_Surface* surface = IMG_Load("Files/gamedata/RES/texmap.bmp");
    //SDL_Texture* texture = SDL_CreateTextureFromSurface(render, surface);

    SDL_Event event;
    memset(&event, 0, sizeof(event));
    VertexArray vao;
    VertexBuffer vbo(vertices, sizeof(vertices));
    vbo.PushVertexAttrib(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void *)offsetof(Vertex, pos));
    vbo.PushVertexAttrib(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void *)offsetof(Vertex, color));
    vbo.PushVertexAttrib(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void *)offsetof(Vertex, texcoords));

//    Texture2D texture("Files/gamedata/RES/texmap.bmp");

    SDL_Surface* surf = SDL_LoadBMP("Files/gamedata/RES/texmap.bmp");


    GLuint texture;
    GLenum data_fmt;
    Uint8 test = SDL_MapRGB(surf->format, 0xAA, 0xBB, 0xCC) & 0xFF;
    if (test == 0xAA) data_fmt = GL_RGB;
    else if (test == 0xCC) data_fmt = GL_BGR;
    else data_fmt = GL_RGBA;
    //Generate an array of textures.  We only want one texture (one element array), so trick
    //it by treating "texture" as array of length one.
    glGenTextures(1,&texture);
    //Select (bind) the texture we just generated as the current 2D texture OpenGL is using/modifying.
    //All subsequent changes to OpenGL's texturing state for 2D textures will affect this texture.
    glBindTexture(GL_TEXTURE_2D,texture);
    //Specify the texture's data.  This function is a bit tricky, and it's hard to find helpful documentation.  A summary:
    //   GL_TEXTURE_2D:    The currently bound 2D texture (i.e. the one we just made)
    //               0:    The mipmap level.  0, since we want to update the base level mipmap image (i.e., the image itself,
    //                         not cached smaller copies)
    //         GL_RGBA:    The internal format of the texture.  This is how OpenGL will store the texture internally (kinda)--
    //                         it's essentially the texture's type.
    //         surf->w:    The width of the texture
    //         surf->h:    The height of the texture
    //               0:    The border.  Don't worry about this if you're just starting.
    //        data_fmt:    The format that the *data* is in--NOT the texture!  Our test image doesn't have an alpha channel,
    //                         so this must be RGB.
    //GL_UNSIGNED_BYTE:    The type the data is in.  In SDL, the data is stored as an array of bytes, with each channel
    //                         getting one byte.  This is fairly typical--it means that the image can store, for each channel,
    //                         any value that fits in one byte (so 0 through 255).  These values are to be interpreted as
    //                         *unsigned* values (since 0x00 should be dark and 0xFF should be bright).
    // surface->pixels:    The actual data.  As above, SDL's array of bytes.
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, surf->w,surf->h, 0, data_fmt,GL_UNSIGNED_BYTE,surf->pixels);
    //Set the minification and magnification filters.  In this case, when the texture is minified (i.e., the texture's pixels (texels) are
    //*smaller* than the screen pixels you're seeing them on, linearly filter them (i.e. blend them together).  This blends four texels for
    //each sample--which is not very much.  Mipmapping can give better results.  Find a texturing tutorial that discusses these issues
    //further.  Conversely, when the texture is magnified (i.e., the texture's texels are *larger* than the screen pixels you're seeing
    //them on), linearly filter them.  Qualitatively, this causes "blown up" (overmagnified) textures to look blurry instead of blocky.
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    SDL_FreeSurface(surf);

    IndexBuffer ibo(indices, 6);
    ibo.Bind();

    Shader shader("shader.glsl");
    shader.Bind();
    
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

    while (1) {
        glViewport(-3.0f, 3.0f, scf::renderer::width, scf::renderer::height);
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                glDeleteTextures(1, &texture);
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
        shader.SetMat4("u_ViewProjection", renderer->camera->GetVPM());
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        vao.Bind();
        ibo.Bind();
//        SDL_GL_BindTexture(texture, NULL, NULL);
        
        for (const auto& i : translations) {
            model = glm::translate(glm::mat4(1.0f), i);
            mvp = renderer->camera->GetProjection() * renderer->camera->GetViewMatrix() * model;
            shader.SetMat4("u_MVP", mvp);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, NULL);
        }
        SDL_GL_SwapWindow(renderer->window);
        vao.Unbind();
        ibo.Unbind();
//        SDL_GL_UnbindTexture(texture);
//        texture.Unbind();
    }
}

void I_NomadInit(int argc, char** argv)
{
    myargc = argc;
    myargv = argv;

    Mem_Init();
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