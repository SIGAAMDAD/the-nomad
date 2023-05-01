#include "n_shared.h"
#include "g_game.h"

color_t red =   {255, 0, 0, 255};
color_t green = {0, 255, 0, 255};
color_t blue =  {0, 0, 255, 255};

static constexpr const char *TEXMAP_PATH = "Files/gamedata/RES/texmap.bmp";

Renderer* renderer;

constexpr int32_t vert_fov = 24 >> 1;
constexpr int32_t horz_fov = 88 >> 1;

static Uint32 R_GetWindowFlags(void)
{
    Uint32 flags = 0;

    switch (scf::renderer::api) {
    case scf::R_OPENGL:
        flags |= SDL_WINDOW_OPENGL;
    case scf::R_SDL2:
        break;
    case scf::R_VULKAN:
        N_Error("R_Init: Vulkan rendering isn't yet supported, will be though very soon");
        break;
    };
    if (scf::renderer::hidden)
        flags |= SDL_WINDOW_HIDDEN;
    
    if (scf::renderer::fullscreen && !scf::renderer::native_fullscreen)
        flags |= SDL_WINDOW_FULLSCREEN;
    else if (scf::renderer::native_fullscreen)
        flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
    
    return flags;
}

void R_InitScene();

void R_Init()
{
    renderer = (Renderer *)Z_Malloc(sizeof(Renderer), TAG_STATIC, &renderer, "renderer");
    assert(renderer);
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) < 0) {
        N_Error("R_Init: failed to initialize SDL2, error message: %s",
            SDL_GetError());
    }

    LOG_INFO("alllocating memory to the SDL_Window context");
    renderer->window = SDL_CreateWindow(
                            "The Nomad",
                            SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                            scf::renderer::width, scf::renderer::height,
                            SDL_WINDOW_OPENGL
                        );
    if (!renderer->window) {
        N_Error("R_Init: failed to initialize an SDL2 window, error message: %s",
            SDL_GetError());
    }
    SDL_GL_SetSwapInterval(1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY);
    SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
    renderer->context = SDL_GL_CreateContext(renderer->window);
    
    if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress))
        N_Error("glad failed to initialize");

    std::cout <<
        "OpenGL Info:\n"
        "  Vendor: " << glGetString(GL_VENDOR) << '\n' <<
        "  Version: " << glGetString(GL_VERSION) << '\n' <<
        "  Renderer: " << glGetString(GL_RENDERER) << std::endl;

    GLenum params[] = {
        GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS,
        GL_MAX_CUBE_MAP_TEXTURE_SIZE,
        GL_MAX_DRAW_BUFFERS,
        GL_MAX_FRAGMENT_UNIFORM_COMPONENTS,
        GL_MAX_TEXTURE_IMAGE_UNITS,
        GL_MAX_TEXTURE_SIZE,
        GL_MAX_VARYING_FLOATS,
        GL_MAX_VERTEX_ATTRIBS,
        GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS,
        GL_MAX_VERTEX_UNIFORM_COMPONENTS,
        GL_MAX_VIEWPORT_DIMS,
        GL_STEREO,
    };
    const char *names[] = {
        "GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS",
        "GL_MAX_CUBE_MAP_TEXTURE_SIZE",
        "GL_MAX_DRAW_BUFFERS",
        "GL_MAX_FRAGMENT_UNIFORM_COMPONENTS",
        "GL_MAX_TEXTURE_IMAGE_UNITS",
        "GL_MAX_TEXTURE_SIZE",
        "GL_MAX_VARYING_FLOATS",
        "GL_MAX_VERTEX_ATTRIBS",
        "GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS",
        "GL_MAX_VERTEX_UNIFORM_COMPONENTS",
        "GL_MAX_VIEWPORT_DIMS",
        "GL_STEREO",
    };

    printf("-----------------------------\n");
    printf("OpenGL Context Params:\n");
    // integers - only works if the order is 0-10 integer return types
    for (int i = 0; i < 10; i++) {
        int v = 0;
        glGetIntegerv(params[i], &v);
        con.ConPrintf("{}: {}" , names[i], v);
    }
    // others
    int v[2];
    memset(v, 0, sizeof(v));
    glGetIntegerv(params[10], v);
    con.ConPrintf("{}: {} {}", names[10], v[0], v[1]);
    unsigned char s = 0;
    glGetBooleanv(params[11], &s);
    con.ConPrintf("{}: {}", names[11], (unsigned int)s);
    printf("\n");

    printf("-----------------------------\n");
    printf("OpenGL Extensions:\n");
    int extension_count = 0;
    glGetIntegerv(GL_NUM_EXTENSIONS, &extension_count);
    for (int i = 0; i < extension_count; ++i) {
        const GLubyte* name = glGetStringi(GL_EXTENSIONS, i);
        printf("%s\n", name);
    }
    printf("\n");

#ifdef _NOMAD_DEBUG
    con.ConPrintf("turning on OpenGL debug callbacks");
    if (glDebugMessageControlARB != NULL) {
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallback((GLDEBUGPROCARB)DBG_GL_ErrorCallback, NULL);
        GLuint unusedIds = 0;
        glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, &unusedIds, GL_TRUE);
    }
#endif

    LOG_INFO("successful initialization of SDL2 context");

    glDisable(GL_DEPTH_TEST);
//    glEnable(GL_BLEND);
//    glEnable(GL_DEPTH_TEST);
}

void R_ShutDown()
{
    if (!sdl_on)
        return;

    con.ConPrintf("R_ShutDown: deallocating SDL2 contexts and window");

    if (renderer->context) {
        SDL_GL_DeleteContext(renderer->context);
        renderer->context = NULL;
    }
    if (renderer->window) {
        SDL_DestroyWindow(renderer->window);
        renderer->window = NULL;
    }
    SDL_Quit();
    sdl_on = false;
}


Camera::Camera(float left, float right, float bottom, float top)
    : m_ProjectionMatrix(glm::ortho(left, right, bottom, top, -1.0f, 1.0f)), m_ViewMatrix(1.0f)
{
    m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;
    m_CameraPos = glm::vec3(0.0f, 0.0f, 0.0f);
}

void Camera::CalculateViewMatrix()
{
    glm::mat4 transform = glm::translate(glm::mat4(1.0f), m_CameraPos) *
        glm::rotate(glm::mat4(1.0f), glm::radians(m_Rotation), glm::vec3(0, 0, 1));
    
    m_ViewMatrix = glm::inverse(transform);
    m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;
}

int R_DrawMenu(const char* fontfile, const nomadvector<std::string>& choices,
    const char* title)
{
    assert(fontfile);
    assert(title);
    assert(choices.size() > 0);

    std::string font_name = "Files/gamedata/RES/"+std::string(fontfile);
    int x, y;
    int NUMMENU = choices.size();
    std::vector<SDL_Texture*> menus(choices.size());
    std::vector<bool> selected(choices.size());
    std::vector<SDL_Rect> pos(choices.size());
    
    SDL_Color color[2] = {{255,255,255},{255,0,0}};
    int32_t text_size = DEFAULT_TEXT_SIZE;

    TTF_Font *font = TTF_OpenFont(font_name.c_str(), text_size);
    if (!font) {
        N_Error("R_DrawMenu: TTF_OpenFont returned NULL for font file %s", fontfile);
    }
#if 0
    for (int i = 0; i < NUMMENU; ++i) {
        menus[i] = R_GenTextureFromFont(font, choices[i].c_str(), color[0]);
    }
    // get the title
    SDL_Texture* title_texture = R_GenTextureFromFont(font, title, {0, 255, 255});
#endif

    int w, h;
//    SDL_Call(SDL_QueryTexture(title_texture, NULL, NULL, &w, &h));
    SDL_Rect title_rect;
    title_rect.x = 75;
    title_rect.y = 50;
    title_rect.w = w;
    title_rect.h = h;

    int round = 0;
    for (int i = 0; i < NUMMENU; ++i) {
//        SDL_Call(SDL_QueryTexture(menus[i], NULL, NULL, &w, &h));
        pos[i].x = 100;
        pos[i].y = 150 + round;
        pos[i].w = w;
        pos[i].h = h;
        round += 70;
    }

    glClear(GL_COLOR_BUFFER_BIT);

    SDL_Event event;
    while (1) {
        N_DebugWindowClear();
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                for (auto* i : menus) {
                    SDL_DestroyTexture(i);
                }
                Game::Get()->~Game();
                exit(EXIT_SUCCESS);
            }
            switch (event.type) {
            case SDL_MOUSEMOTION: {
                y = event.motion.y;
                x = event.motion.x;
                for (int i = 0; i < NUMMENU; i++) {
                    if (x >= pos[i].x && x <= pos[i].x + pos[i].w
                     && y >= pos[i].y && y <= pos[i].y + pos[i].h) {
                        if (!selected[i]) {
                            selected[i] = true;
                            SDL_DestroyTexture(menus[i]);
//                            menus[i] = R_GenTextureFromFont(font, choices[i].c_str(), color[1]);
                        }
                    }
                    else {
                        if (selected[i]) {
                            selected[i] = false;
                            SDL_DestroyTexture(menus[i]);
//                            menus[i] = R_GenTextureFromFont(font, choices[i].c_str(), color[0]);
                        }
                    }
                }
                break; }
            case SDL_MOUSEBUTTONDOWN: {
                x = event.button.x;
                y = event.button.y;
                for(int i = 0; i < NUMMENU; i++) {
                    if(x >= pos[i].x && x <= pos[i].x + pos[i].w
                    && y >= pos[i].y && y <= pos[i].y + pos[i].h) {
                        for (auto* i : menus) {
                            SDL_DestroyTexture(i);
                        }
                        N_DebugWindowDraw();
                        return i;
                    }
                }
                break; }
            case SDL_KEYDOWN: {
                if (event.key.keysym.sym == SDLK_ESCAPE) {
                    goto done;
                }
                break; }
            };
        }
//        R_DrawTexture(title_texture, NULL, &title_rect);
        for (int i = 0; i < NUMMENU; ++i)
//            R_DrawTexture(menus[i], NULL, &pos[i]);
        N_DebugWindowDraw();
    }
done:
    N_DebugWindowDraw();
    return -1;
}

#ifdef _NOMAD_DEBUG

const char *DBG_GL_SourceToStr(GLenum source)
{
    switch (source) {
    case GL_DEBUG_SOURCE_API: return "API";
    case GL_DEBUG_SOURCE_WINDOW_SYSTEM: return "Window System";
    case GL_DEBUG_SOURCE_SHADER_COMPILER: return "Shader Compiler";
    case GL_DEBUG_SOURCE_THIRD_PARTY: return "Third Party";
    case GL_DEBUG_SOURCE_APPLICATION: return "Application User";
    case GL_DEBUG_SOURCE_OTHER: return "Other";
    };
    return "Unknown Source";
}

const char *DBG_GL_TypeToStr(GLenum type)
{
    switch (type) {
    case GL_DEBUG_TYPE_ERROR: return "Error";
    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: return "Deprecated Behaviour";
    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR: return "Undefined Behaviour";
    case GL_DEBUG_TYPE_PORTABILITY: return "Portability";
    case GL_DEBUG_TYPE_PERFORMANCE: return "Performance";
    case GL_DEBUG_TYPE_MARKER: return "Marker";
    case GL_DEBUG_TYPE_PUSH_GROUP: return "Debug Push group";
    case GL_DEBUG_TYPE_POP_GROUP: return "Debug Pop Group";
    case GL_DEBUG_TYPE_OTHER: return "Other";
    };
    return "Unknown Type";
}

const char *DBG_GL_SeverityToStr(GLenum severity)
{
    switch (severity) {
    case GL_DEBUG_SEVERITY_HIGH: return "High";
    case GL_DEBUG_SEVERITY_MEDIUM: return "Medium";
    case GL_DEBUG_SEVERITY_LOW: return "Low";
    case GL_DEBUG_SEVERITY_NOTIFICATION: return "Notification";
    };
    return "Unknown Severity";
}

void DBG_GL_ErrorCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message, const GLvoid *userParam)
{
    if (type == GL_DEBUG_TYPE_ERROR) {
        LOG_ERROR(
            "<---- OpenGL Debug Log (ERROR) ---->\n"
            "    source: {}\n"
            "    message: {}\n"
            "    type: {}\n"
            "    id: {}\n"
            "    severity: {}\n",
        DBG_GL_SourceToStr(source), message, DBG_GL_TypeToStr(type), std::to_string(id), DBG_GL_SeverityToStr(severity));
        abort();
    }
    else if (type == GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR || type == GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR 
          || type == GL_DEBUG_TYPE_PORTABILITY || type == GL_DEBUG_TYPE_PERFORMANCE)
    {
        LOG_WARN(
            "<---- OpenGL Debug Log (WARN) ---->\n"
            "    source: {}\n"
            "    message: {}\n"
            "    type: {}\n"
            "    id: {}\n"
            "    severity: {}\n",
        DBG_GL_SourceToStr(source), message, DBG_GL_TypeToStr(type), std::to_string(id), DBG_GL_SeverityToStr(severity));
    }
    else {
        LOG_TRACE(
            "<---- OpenGL Debug Log (TRACE) ---->\n"
            "    source: {}\n"
            "    message: {}\n"
            "    type: {}\n"
            "    id: {}\n"
            "    severity: {}\n",
        DBG_GL_SourceToStr(source), message, DBG_GL_TypeToStr(type), std::to_string(id), DBG_GL_SeverityToStr(severity));
    }
}

#endif

struct DrawCall
{
    size_t numvertices = 0;
    Vertex *vertices = NULL;

    glm::mat4 mvp;
};

struct QuadVertex
{
    glm::vec3 pos;
    glm::vec4 color;
    glm::vec2 texcoords;
};

struct SceneData
{
    static const uint32_t MaxQuads = 20000;
    static const uint32_t MaxIndices = MaxQuads * 6;
    static const uint32_t MaxVertices = MaxQuads * 4;

    VertexArray* SceneVertexArray = NULL;
    VertexBuffer* SceneVertexBuffer = NULL;
    Shader* SceneShader = NULL;

    uint32_t QuadIndexCount = 0;
    Vertex* QuadVertexBufferBase = NULL;
    Vertex* QuadVertexBufferPtr = NULL;

    glm::vec4 QuadVertexPositions[4];
};

static SceneData scene;

void R_InitScene()
{
    scene.SceneVertexArray = (VertexArray *)Z_Malloc(sizeof(VertexArray), TAG_STATIC, &scene.SceneVertexArray,
        "sceneVAO");
    new (scene.SceneVertexArray) VertexArray();
    scene.SceneVertexBuffer = (VertexBuffer *)Z_Malloc(sizeof(VertexBuffer), TAG_STATIC, &scene.SceneVertexBuffer,
        "sceneVBO");
    new (scene.SceneVertexBuffer) VertexBuffer(sizeof(Vertex) * scene.MaxVertices);
    scene.SceneVertexBuffer->PushVertexAttrib(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void *)offsetof(Vertex, pos));
    scene.SceneVertexBuffer->PushVertexAttrib(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void *)offsetof(Vertex, color));

    scene.QuadVertexBufferBase = (Vertex *)Z_Malloc(sizeof(Vertex) * scene.MaxVertices, TAG_STATIC, &scene.QuadVertexBufferBase,
        "quadBuffer");
    scene.QuadVertexBufferPtr = scene.QuadVertexBufferBase;

    scene.SceneShader = (Shader *)Z_Malloc(sizeof(Shader), TAG_STATIC, &scene.SceneShader, "sceneShader");
    new (scene.SceneShader) Shader("shader.glsl");

    uint32_t quadIndices[scene.MaxIndices];
    uint32_t offset = 0;
    for (uint32_t i = 0; i < scene.MaxIndices; i += 6) {
        quadIndices[i + 0] = offset + 0;
        quadIndices[i + 1] = offset + 1;
        quadIndices[i + 2] = offset + 2;

        quadIndices[i + 3] = offset + 0;
        quadIndices[i + 4] = offset + 2;
        quadIndices[i + 5] = offset + 3;

        offset += 4;
    }
    std::shared_ptr<IndexBuffer> ibo = IndexBuffer::Create(quadIndices, scene.MaxIndices);

    scene.SceneVertexArray->SetIndexBuffer(ibo);

    scene.QuadVertexPositions[0] = {  0.5f,  0.5f, 0.0f, 1.0f };
    scene.QuadVertexPositions[1] = { -0.5f,  0.5f, 0.0f, 1.0f };
    scene.QuadVertexPositions[2] = { -0.5f, -0.5f, 0.0f, 1.0f };
    scene.QuadVertexPositions[3] = {  0.5f, -0.5f, 0.0f, 1.0f };
//    scene.QuadVertexPositions[4] = {  0.5f,  0.5f, 0.0f, 1.0f };
//    scene.QuadVertexPositions[5] = { -0.5f, -0.5f, 0.0f, 1.0f };
//    scene.QuadVertexPositions[6] = {  0.5f, -0.5f, 0.0f, 1.0f };

    renderer->camera = (Camera *)Z_Malloc(sizeof(Camera), TAG_STATIC, &renderer->camera, "camera");
    new (renderer->camera) Camera(-2.0f, 2.0f, -2.0f, 2.0f);

    ibo->Unbind();
    scene.SceneShader->Unbind();
    scene.SceneVertexArray->Unbind();
    scene.SceneVertexBuffer->Unbind();

    glClearColor(0.1f, 0.1f, 0.1f, 0.0f);
}

void Renderer::BeginScene()
{
    renderer->camera->CalculateViewMatrix();
    scene.SceneShader->Bind();
    scene.SceneShader->SetMat4("u_ViewProjection", renderer->camera->GetVPM());
    glClear(GL_COLOR_BUFFER_BIT);
}

void Renderer::Flush()
{
    if (scene.QuadIndexCount) {
        uint32_t dataSize = (uint32_t)((byte *)scene.QuadVertexBufferPtr - (byte *)scene.QuadVertexBufferBase);
        scene.SceneShader->SetMat4("u_Transform", glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, 0)));
        scene.SceneVertexArray->Bind();
        scene.SceneVertexBuffer->SetData(scene.QuadVertexBufferBase, dataSize);

        scene.SceneVertexArray->GetIndexBuffer()->Bind();

        glDrawElements(GL_TRIANGLES, scene.QuadIndexCount, GL_UNSIGNED_INT, NULL);

        scene.SceneVertexArray->GetIndexBuffer()->Unbind();
        scene.SceneVertexArray->Unbind();
    }
}

void Renderer::StartBatch()
{
    scene.QuadIndexCount = 0;
    scene.QuadVertexBufferPtr = scene.QuadVertexBufferBase;
}

void Renderer::NextBatch()
{
    Flush();
    StartBatch();
}

void Renderer::EndScene()
{
    scene.SceneShader->Unbind();
    SDL_GL_SwapWindow(renderer->window);
}

void Renderer::DrawQuad(const glm::vec2& pos, const glm::vec2& size, const glm::vec4& color)
{
    if (scene.QuadIndexCount >= scene.MaxIndices)
        NextBatch();

    glm::mat4 transform = glm::translate(glm::mat4(1.0f), glm::vec3(pos, 0.0f))
        * glm::scale(glm::mat4(1.0f), glm::vec3(size, 1.0f));
    
    for (uint32_t i = 0; i < 4; ++i) {
        scene.QuadVertexBufferPtr->pos = transform * scene.QuadVertexPositions[i];
        scene.QuadVertexBufferPtr->color = color;
        scene.QuadVertexBufferPtr++;
    }
    scene.QuadIndexCount += 6;
}