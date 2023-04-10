#include "n_shared.h"
#include "g_game.h"

color_t red =   {255, 0, 0, 255};
color_t green = {0, 255, 0, 255};
color_t blue =  {0, 0, 255, 255};

static constexpr const char *TEXMAP_PATH = "Files/gamedata/RES/texmap.bmp";

renderer_t *renderer;

constexpr int32_t vert_fov = 24 >> 1;
constexpr int32_t horz_fov = 88 >> 1;

template<typename T>
void R_ScaleToResolution(T &y, T &x)
{
    SDL_Surface* screen = SDL_GetWindowSurface(renderer->SDL_window);
    y *= screen->clip_rect.w;
    x *= screen->clip_rect.h;
}

void R_DrawVMatrix(void)
{
    for (int32_t y = 0; y < 24; ++y) {
        for (int32_t x = 0; x < 88; ++x) {
            
        }
    }
}

void R_GetVMatrix(void)
{
    playr_t* const playr = Game::Get()->playr;
    coord_t startc = {playr->pos.y - vert_fov, playr->pos.x - horz_fov};
    coord_t endc = {playr->pos.y + vert_fov, playr->pos.x + horz_fov};
    for (int32_t y = startc.y; y < endc.y; ++y) {
        for (int32_t x = startc.x; x < endc.x; ++x) {
            renderer->vmatrix[y][x] = Game::Get()->c_map[y][x];
        }
    }
    R_DrawVMatrix();
}

static void I_CacheModels()
{
    LOG_INFO("pre-caching models/textures");

    struct loader
    {
        void *addr = NULL;
        uint32_t index = 0;
        float time = 0; // profiler

        loader(void *_addr, uint32_t _index, float _time)
            : addr(_addr), index(_index), time(_time) { }
    };
    std::clock_t start{}, end{};
    float total{};
    std::vector<loader> stats;
    stats.reserve(modelinfo.size());
    renderer->models = (model_t *)Z_Malloc(sizeof(model_t) * modelinfo.size(), TAG_CACHE, &renderer->models);

    for (uint32_t i = 0; i < NUMMODELS; ++i) {
        start = std::clock();
        LOG_TRACE("loading up model at index {}", i);
        N_memcpy(&renderer->models[i], &modelinfo[i], sizeof(model_t));
        end = std::clock();
        total = (float)(end - start) / (float)CLOCKS_PER_SEC;
        stats.emplace_back(loader{ (void *)&renderer->models[i], i, total });
        start = end = 0;
        total = 0;
    }
    LOG_INFO("finished loading");
    LOG_TRACE("model loading statistics:");
    float longest = 0;
    uint32_t longest_index = 0;
    double avg = 0;
    for (size_t i = 0; i < stats.size(); ++i) {
        LOG_TRACE("\n    index   : {index}\n"
            "    address : {address}\n"
            "    time    : {time}",
        fmt::arg("index", stats[i].index), fmt::arg("address", stats[i].addr), fmt::arg("time", stats[i].time));
        avg += stats[i].time;
        longest = longest > stats[i].time ? longest : stats[i].time;
        longest_index = longest != stats[i].time ? longest_index : stats[i].index;
    }
    LOG_TRACE("average: {}", avg / stats.size());
    LOG_TRACE("longest: {1} (index {0})", longest, longest_index);
}

void R_DrawCompass()
{
    uint32_t mdl = 0;
    switch (Game::GetPlayr()->pdir) {
    case D_NORTH: mdl = MDL_COMPASS_UP; break;
    case D_WEST: mdl = MDL_COMPASS_LEFT; break;
    case D_SOUTH: mdl = MDL_COMPASS_DOWN; break;
    case D_EAST: mdl = MDL_COMPASS_RIGHT; break;
    default:
        LOG_WARN("playr->pdir was an invalid direction, assigning default value of D_NORTH");
        mdl = MDL_COMPASS_UP;
        Game::GetPlayr()->pdir = D_NORTH;
        break;
    };
    SDL_Rect rect;
    rect.x = 0;
    rect.y = 0;
    rect.w = 170;
    rect.h = 170;
    R_DrawTexture(renderer->SDL_spr_sheet, &modelinfo[mdl].offset, &rect);
//    R_DrawTextureFromTable(mdl);
}

inline SDL_Texture* R_GenTextureFromFont(TTF_Font* font, const char* str, const SDL_Color& color)
{
    assert(font);
    assert(str);

    SDL_Surface *surface = TTF_RenderText_Solid(font, str, color);
    if (!surface) {
        N_Error("R_GenTextureFromFont: TTF_RenderTextSolid returned NULL, SDL2 error message: %s",
            SDL_GetError());
    }
    assert(surface);

    SDL_Texture* texture = SDL_CreateTextureFromSurface(R_GetRenderer(), surface);
    if (!texture) {
        N_Error("R_GenTextureFromFont: SDL_CreateTextureFromSurface returned NULL, SDL2 error message: %s",
            SDL_GetError());
    }
    SDL_FreeSurface(surface);
    assert(texture);
    return texture;
}

void R_DrawVitals()
{
}

void R_DrawScreen(void)
{
    R_ClearScreen();
    switch (Game::Get()->gamestate) {
    case GS_MENU:
    case GS_LEVEL:
        break;
    };
    R_FlushBuffer();
}

static Uint32 R_GetWindowFlags(void)
{
    Uint32 flags = 0;
    if (scf::renderer::hidden)
        flags |= SDL_WINDOW_HIDDEN;
    else
        flags |= SDL_WINDOW_SHOWN;
    
    if (scf::renderer::fullscreen && !scf::renderer::native_fullscreen)
        flags |= SDL_WINDOW_FULLSCREEN;
    else if (scf::renderer::native_fullscreen)
        flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
    
    return flags;
}

void R_Init()
{
    renderer = (renderer_t *)Z_Malloc(sizeof(renderer_t), TAG_STATIC, &renderer);
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) < 0) {
        N_Error("R_Init: failed to initialize SDL2, error message: %s",
            SDL_GetError());
    }
    if (TTF_Init() < 0) {
        N_Error("R_Init: failed to initialize SDL2_ttf, error message: %s",
            TTF_GetError());
    }

    LOG_INFO("alllocating memory to the SDL_Window context");
    renderer->SDL_window = SDL_CreateWindow(
                            "The Nomad",
                            SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                            scf::renderer::width, scf::renderer::height,
                            R_GetWindowFlags()
                        );
    if (!renderer->SDL_window) {
        N_Error("R_Init: failed to initialize an SDL2 window (a std::shared_ptr), error message: %s",
            SDL_GetError());
    }
    assert(renderer->SDL_window);
    LOG_INFO("success");
    Game::Get()->window = renderer->SDL_window;
    
    LOG_INFO("allocating memory to the SDL_Renderer context");
    renderer->screen.screen = SDL_CreateRenderer(renderer->SDL_window, -1, SDL_RENDERER_ACCELERATED |
        (scf::renderer::vsync ? SDL_RENDERER_PRESENTVSYNC : 0));
    if (!renderer->screen.screen) {
        con.ConError("R_Init: failed to initialize an SDL2 rendering context, error message: %s\n"
                        "        Setting up software rendering fallback",
        SDL_GetError());

        renderer->screen.software = SDL_CreateRenderer(renderer->SDL_window, -1, SDL_RENDERER_SOFTWARE |
            (scf::renderer::vsync ? SDL_RENDERER_PRESENTVSYNC : 0));
        if (!renderer->screen.software) {
            N_Error("R_Init: failed to initialize an SDL2 software rendering context, error message: %s",
                SDL_GetError());
        }
        assert(renderer->screen.software);
    }
    assert(R_GetRenderer());
    LOG_INFO("success");

    LOG_INFO("loading the sprite sheet from {}", TEXMAP_PATH);
    renderer->SDL_spr_sheet = IMG_LoadTexture(R_GetRenderer(), TEXMAP_PATH);
    
    if (!renderer->SDL_spr_sheet) {
        N_Error("R_Init: failed to load required texture file %s for rendering, error message: %s",
            TEXMAP_PATH, SDL_GetError());
    }
    assert(renderer->SDL_spr_sheet);
    LOG_INFO("success");

    renderer->SDL_win_sur = SDL_GetWindowSurface(renderer->SDL_window);
    assert(renderer->SDL_win_sur);

    sdl_on = true;
    N_memset((void *)renderer->vmatrix, 0, sizeof(renderer->vmatrix));
    LOG_INFO("successful initialization of SDL2 context");
    I_CacheModels();
}
void R_ShutDown()
{
    if (!sdl_on)
        return;

    con.ConPrintf("R_ShutDown: deallocating SDL2 contexts and window");
    if (R_GetRenderer()) {
        SDL_DestroyRenderer(R_GetRenderer());
    }
    if (renderer->SDL_spr_sheet) {
        SDL_DestroyTexture(renderer->SDL_spr_sheet);
    }
    if (renderer->SDL_win_sur) {
        SDL_FreeSurface(renderer->SDL_win_sur);
    }
    if (renderer->SDL_window) {
        SDL_DestroyWindow(renderer->SDL_window);
    }
    Z_Free(renderer);
    TTF_Quit();
    SDL_Quit();
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
    for (int i = 0; i < NUMMENU; ++i) {
        menus[i] = R_GenTextureFromFont(font, choices[i].c_str(), color[0]);
    }
    // get the title
    SDL_Texture* title_texture = R_GenTextureFromFont(font, title, {0, 255, 255});

    int w, h;
    SDL_Call(SDL_QueryTexture(title_texture, NULL, NULL, &w, &h));
    SDL_Rect title_rect;
    title_rect.x = 75;
    title_rect.y = 50;
    title_rect.w = w;
    title_rect.h = h;

    int round = 0;
    for (int i = 0; i < NUMMENU; ++i) {
        SDL_Call(SDL_QueryTexture(menus[i], NULL, NULL, &w, &h));
        pos[i].x = 100;
        pos[i].y = 150 + round;
        pos[i].w = w;
        pos[i].h = h;
        round += 70;
    }

    R_ResetScreenColor();
    R_ClearScreen();

    SDL_Event event;
    while (1) {
        N_DebugWindowClear();
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                for (auto* i : menus) {
                    SDL_DestroyTexture(i);
                }
                SDL_DestroyTexture(title_texture);
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
                            menus[i] = R_GenTextureFromFont(font, choices[i].c_str(), color[1]);
                        }
                    }
                    else {
                        if (selected[i]) {
                            selected[i] = false;
                            SDL_DestroyTexture(menus[i]);
                            menus[i] = R_GenTextureFromFont(font, choices[i].c_str(), color[0]);
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
        R_ResetScreenColor();
        R_DrawTexture(title_texture, NULL, &title_rect);
        for (int i = 0; i < NUMMENU; ++i)
            R_DrawTexture(menus[i], NULL, &pos[i]);
        N_DebugWindowDraw();
    }
done:
    N_DebugWindowDraw();
    for (auto* i : menus) {
        SDL_DestroyTexture(i);
    }
    SDL_DestroyTexture(title_texture);
    return -1;
}

void R_DrawBox(int32_t y, int32_t x, int32_t width, int32_t height, const SDL_Color& color)
{
    SDL_Rect rect{x, y, width, height};
    R_SetScreenColor(color);
    R_DrawBox(rect);
    R_ResetScreenColor();
}
void R_DrawBox(int32_t y, int32_t x, int32_t width, int32_t height)
{
    SDL_Rect rect{x, y, width, height};
    R_DrawBox(rect);
}

void R_DrawBox(const SDL_Rect& rect)
{
    SDL_Call(SDL_RenderDrawRect(R_GetRenderer(), &rect));
}

void R_DrawFilledBox(const SDL_Rect& rect, const SDL_Color& fill_color)
{
    R_SetScreenColor(fill_color.r, fill_color.g, fill_color.b, fill_color.a);
    SDL_Call(SDL_RenderFillRect(R_GetRenderer(), &rect));
    R_ResetScreenColor();
    R_DrawBox(rect);
}
void R_DrawFilledBox(const SDL_Rect& rect, byte r, byte g, byte b, byte a)
{
    R_SetScreenColor(r, g, b, a);
    SDL_Call(SDL_RenderFillRect(R_GetRenderer(), &rect));
    R_ResetScreenColor();
    R_DrawBox(rect);
}
void R_DrawFilledBox(const SDL_Rect& rect, color_t fill_color)
{
    R_SetScreenColor(fill_color);
    SDL_Call(SDL_RenderFillRect(R_GetRenderer(), &rect));
    R_ResetScreenColor();
    R_DrawBox(rect);
}
void R_DrawFilledBox(int32_t y, int32_t x, int32_t width, int32_t height, const SDL_Color& fill_color)
{
    SDL_Rect rect{x, y, width, height};
    R_SetScreenColor(fill_color);
    SDL_Call(SDL_RenderFillRect(R_GetRenderer(), &rect));
    R_ResetScreenColor();
    R_DrawBox(rect);
}
void R_DrawFilledBox(int32_t y, int32_t x, int32_t width, int32_t height, byte r, byte g, byte b, byte a)
{
    SDL_Rect rect{x, y, width, height};
    R_SetScreenColor(r, g, b, a);
    SDL_Call(SDL_RenderFillRect(R_GetRenderer(), &rect));
    R_ResetScreenColor();
    R_DrawBox(rect);
}
void R_DrawFilledBox(int32_t y, int32_t x, int32_t width, int32_t height, color_t fill_color)
{
    assert(fill_color);
    SDL_Rect rect{x, y, width, height};
    R_SetScreenColor(fill_color);
    SDL_Call(SDL_RenderFillRect(R_GetRenderer(), &rect));
    R_ResetScreenColor();
    R_DrawBox(rect);
}