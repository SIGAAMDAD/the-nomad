#include "n_shared.h"
#include "g_game.h"

color_t red =   {255, 0, 0, 255};
color_t green = {0, 255, 0, 255};
color_t blue =  {0, 0, 255, 255};

static constexpr const char *TEXMAP_PATH = "Files/gamedata/RES/texmap.bmp";

std::unique_ptr<Renderer> renderer;

constexpr int32_t vert_fov = 24 >> 1;
constexpr int32_t horz_fov = 88 >> 1;

void R_DrawCompass()
{
    uint32_t mdl = 0;
    switch (Game::Get()->entities.front().dir) {
    case D_NORTH: mdl = MDL_COMPASS_UP; break;
    case D_WEST: mdl = MDL_COMPASS_LEFT; break;
    case D_SOUTH: mdl = MDL_COMPASS_DOWN; break;
    case D_EAST: mdl = MDL_COMPASS_RIGHT; break;
    default:
        LOG_WARN("playr->pdir was an invalid direction, assigning default value of D_NORTH");
        mdl = MDL_COMPASS_UP;
        Game::Get()->entities.front().dir = D_NORTH;
        break;
    };
    SDL_Rect rect;
    rect.x = 0;
    rect.y = 0;
    rect.w = 170;
    rect.h = 170;
//    R_DrawTexture(renderer->SDL_spr_sheet, &modelinfo[mdl].offset, &rect);
//    R_DrawTextureFromTable(mdl);
}

void R_DrawVitals()
{
}

void R_DrawMap()
{
    glm::vec2 startc = {Game::GetPlayr()->p->pos.coords.y - (vert_fov >> 1), Game::GetPlayr()->p->pos.coords.x - (horz_fov >> 1)};
    glm::vec2 endc = {Game::GetPlayr()->p->pos.coords.y + (vert_fov >> 1), Game::GetPlayr()->p->pos.coords.x + (horz_fov >> 1)};
}

void R_DrawScreen(void)
{
    switch (Game::Get()->gamestate) {
    case GS_MENU:
    case GS_LEVEL:
        break;
    };
}

static Uint32 R_GetWindowFlags(void)
{
    Uint32 flags = 0;
    if (scf::renderer::hidden)
        flags |= SDL_WINDOW_HIDDEN;
    
    if (scf::renderer::fullscreen && !scf::renderer::native_fullscreen)
        flags |= SDL_WINDOW_FULLSCREEN;
    else if (scf::renderer::native_fullscreen)
        flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
    
    flags |= SDL_WINDOW_OPENGL;
    
    return flags;
}

void R_Init()
{
    renderer = std::make_unique<Renderer>();
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
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 5);
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
    printf("GL Context Params:\n");
    // integers - only works if the order is 0-10 integer return types
    for (int i = 0; i < 10; i++) {
        int v = 0;
        glGetIntegerv(params[i], &v);
        con.ConPrintf("{}: {}" , names[i], v);
    }
    // others
    int v[2];
    v[0] = v[1] = 0;
    glGetIntegerv(params[10], v);
    con.ConPrintf("{}: {} {}", names[10], v[0], v[1]);
    unsigned char s = 0;
    glGetBooleanv(params[11], &s);
    con.ConPrintf("{}: {}", names[11], (unsigned int)s);
    printf("\n");

    printf("-----------------------------\n");
    printf("GL Extensions:\n");
    int extension_count = 0;
    glGetIntegerv(GL_NUM_EXTENSIONS, &extension_count);
    for (int i = 0; i < extension_count; ++i) {
        const GLubyte* name = glGetStringi(GL_EXTENSIONS, i);
        printf("%s\n", name);
    }
    printf("\n");

    LOG_INFO("successful initialization of SDL2 context");

    glEnable(GL_BLEND);
//    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_CULL_FACE);
#if 0 // for future 3d...
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CCW);
#endif
}

void Renderer::Draw(const Shader& shader, const VertexArray& va) const
{
    shader.Bind();
    va.Bind();
    if (va.HasIBO()) {
        va.BindIBO();
        glCall(glDrawElements(GL_TRIANGLES, va.GetIBO()->numindices(), GL_UNSIGNED_INT, NULL));
    }
    else if (va.HasVBO()) {
        va.BindVBO();
        glCall(glDrawArrays(GL_TRIANGLES, 0, va.GetVBO()->numvertices()));
    }
    va.Unbind();
    va.GetIBO()->Unbind();
    SDL_GL_SwapWindow(window);
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

Camera::Camera(float left, float right, float bottom, float top, float zFar, float zNear)
    : m_ProjectionMatrix(glm::ortho(left, right, bottom, top, -1.0f, 1.0f)), m_ViewMatrix(1.0f)
{
    m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;
    m_CameraPos = glm::vec3(0.5f, 0.5f, 0.0f);
}

void Camera::CalculateViewMatrix()
{
    glm::mat4 transform = glm::translate(glm::mat4(1.0f), m_CameraPos) *
        glm::rotate(glm::mat4(1.0f), glm::radians(m_Rotation), glm::vec3(0, 0, 1));
    
//    m_ViewMatrix = glm::inverse(transform);
    m_ViewMatrix = glm::translate(transform, glm::vec3(0, 0, 0));
    m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewProjectionMatrix;
}

int R_DrawMenu(const char* fontfile, const std::vector<std::string>& choices,
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