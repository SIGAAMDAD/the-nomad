#include "n_shared.h"
#include "g_game.h"

static inline void done()
{
    LOG_INFO("deallocating and shutting down all systems");
    Game::Get()->~Game();
    exit(EXIT_SUCCESS);
}

static void N_HandleWindowEvent(const SDL_Event& event)
{
    switch (event.window.event) {
    case SDL_WINDOWEVENT_SHOWN:
        LOG_INFO("SDL_WINDOWEVENT_SHOWN triggered");
        break;
    case SDL_WINDOWEVENT_HIDDEN:
        LOG_INFO("SDL_WINDOWEVENT_HIDDEN triggered");
        break;
    case SDL_WINDOWEVENT_MOVED:
        LOG_TRACE("SDL_WINDOWEVENT_MOVED triggered. new position: {}, {}",
            event.window.data1, event.window.data2);
        break;
    case SDL_WINDOWEVENT_RESIZED:
        LOG_INFO("SDL_WINDOWEVENT_RESIZED triggered. new size: {}x{}",
            event.window.data1, event.window.data2);
//        SDL_RenderSetScale(R_GetRenderer(), scf::renderer::width, scf::renderer::height);
        break;
    case SDL_WINDOWEVENT_SIZE_CHANGED:
        LOG_INFO("SDL_WINDOWEVENT_SIZE_CHANGED triggered. new size: {}x{}",
            event.window.data1, event.window.data2);
        break;
    case SDL_WINDOWEVENT_MINIMIZED:
        LOG_INFO("SDL_WINDOWEVENT_MINIMIZED triggered");
        break;
    case SDL_WINDOWEVENT_MAXIMIZED:
        LOG_INFO("SDL_WINDOWEVENT_MAXIMIZED triggered");
        break;
    };
}

static void N_ShowAbout()
{
//    R_ClearScreen();
//    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, "*** The Nomad: About ***", about_str, renderer->SDL_window);
//    R_FlushBuffer();
}

static void N_ShowCredits()
{
//    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, "*** The Nomad: Credits ***", credits_str, renderer->SDL_window);
}

struct profiler_stats
{
    uint64_t total = 0;
    double avg = 0;
};

static void N_Level()
{
    LOG_INFO("gamestate = GS_LEVEL");
    SDL_Event event;
    LOG_INFO("beginning level loop, ticrate: {}", scf::renderer::ticrate);
#ifdef _NOMAD_DEBUG
    float renderer_time, events_time, loop_time;
    profiler_stats renderer, events, loop;
#endif
    while (Game::Get()->gamestate == GS_LEVEL) {
        PROFILE_FUNC(loop_time);
        N_DebugWindowClear();
        while (SDL_PollEvent(&event)) {
            PROFILE_SCOPE(events_time);
            ImGui_ImplSDL2_ProcessEvent(&event);
            if (event.type == SDL_QUIT) {
                done();
            }
            else if (event.type == SDL_WINDOWEVENT) {
                N_HandleWindowEvent(event);
            }
            else if (event.type == SDL_KEYDOWN) {
                switch (event.key.keysym.sym) {
                case SDLK_ESCAPE:
                    Game::Get()->gamestate = GS_PAUSE;
                    break;
                };
                for (const auto& i : scf::kb_binds) {
                    if (i.button == (scf::button_t)event.key.keysym.sym) {
                        i.action();
                    }
                }
            }
        }
        {
            PROFILE_SCOPE(renderer_time);
#if 0
            R_ClearScreen();
            R_DrawFilledBox(0, 170, 40, SCREEN_HEIGHT, 0, 255, 0, 255);
            R_DrawFilledBox(170, 0, SCREEN_WIDTH, 30, 0, 255, 0, 255);
            R_DrawCompass();
            R_DrawFilledBox(SCREEN_HEIGHT - 170, 0, SCREEN_WIDTH, 30, 0, 255, 0, 255);
            R_DrawFilledBox(0, SCREEN_WIDTH - 250, 40, SCREEN_HEIGHT, 0, 255, 0, 255);
            R_DrawFilledBox(SCREEN_HEIGHT - 150, 180, 350, 100, 0, 0, 255, 255);
#endif
        }
#ifdef _NOMAD_DEBUG
        loop.total++;
        renderer.total++;
        events.total++;
        loop.avg += loop_time;
        renderer.avg += renderer_time;
        events.avg += events_time;
#endif
        IMGUI_BEGIN("Profiler");
        IMGUI_TEXT("[Render (Scope)]: %f", renderer_time);
        IMGUI_TEXT("[Event Poll (Scope)]: %f", events_time);
        IMGUI_TEXT("[N_Level (Function)]: %f", loop_time);
        IMGUI_END();
        N_DebugWindowDraw();
    }
    LOG_TRACE("renderer average time: {}", renderer.avg / renderer.total);
    LOG_TRACE("event loop average time: {}", events.avg / events.total);
    LOG_TRACE("loop average time: {}", loop.avg / loop.total);
    LOG_INFO("exiting level loop");
}

static gamestate_t prev_settings_menu;
static gamestate_t prev_io_menu;

static void N_PauseMenu()
{
    LOG_INFO("gamestate = GS_PAUSE");
    while (Game::Get()->gamestate == GS_PAUSE) {
        int selected = R_DrawMenu("AlegreyaFont.ttf",
                                {"Resume", "Load Save", "Save Game", "Settings", "Exit To Desktop", "Exit To Menu"},
                                "Pause Menu");
        switch (selected) {
        case 0:
            Game::Get()->gamestate = GS_LEVEL;
            break;
        case 1:
            G_LoadGame();
            break;
        case 2:
            G_SaveGame();
            break;
        case 3:
            prev_settings_menu = GS_PAUSE;
            Game::Get()->gamestate = GS_SETTINGS;
            break;
        case 4:
            done();
            break;
        case 5:
            Game::Get()->gamestate = GS_MENU;
            break;
        default: break;
        };
    }
}

static void N_MainMenu()
{
    LOG_INFO("gamestate = GS_MENU");
    while (Game::Get()->gamestate == GS_MENU) {
        int selected = R_DrawMenu("AlegreyaFont.ttf",
                                {"New Game", "Load Game", "Settings", "About", "Credits", "Quit Game"},
                                "Main Menu");
        switch (selected) {
        case 0:
            Game::Get()->gamestate = GS_LEVEL;
            break;
        case 1:
            G_LoadGame();
            break;
        case 2:
            prev_settings_menu = GS_MENU;
            Game::Get()->gamestate = GS_SETTINGS;
            break;
        case 3:
            N_ShowAbout();
            break;
        case 4:
            N_ShowCredits();
            break;
        case 5:
            done();
            break;
        default: break;
        };
    }
    LOG_INFO("exiting main menu");
}

#if 0
static void N_SaveMenu()
{
    LOG_INFO("gamestate = GS_SAVE");
    
    const ImVec4 selected(0, 180, 50, 255);
    char slotname[256];
    memset(slotname, 0, sizeof(slotname));
    int selector = 0;

    bool open = true;
    SDL_Event event;
    json& savedata = N_GetSaveJSon();
    std::array<std::string, MAXSAVES> files;
    
    enum
    {
        BROWSING,
        OVERWRITE,
        STANDARD_SAVE
    };
    uint8_t state = BROWSING;

    for (int i = 0; i < MAXSAVES; ++i) {
        std::string node_name = "sv_"+std::to_string(i);
        files[i] = savedata[node_name]["used"] ? savedata[node_name]["name"] : "empty";
    }
    
    while (Game::Get()->gamestate == GS_SAVE || open) {
        N_DebugWindowClear();
        while (SDL_PollEvent(&event)) {
            ImGui_ImplSDL2_ProcessEvent(&event);
            if (event.type == SDL_QUIT) {
                done();
            }
            else if (event.type == SDL_KEYDOWN) {
                switch (event.key.keysym.sym) {
                case SDLK_ESCAPE:
                case SDLK_BACKSPACE:
                    Game::Get()->gamestate = prev_io_menu;
                    break;
                case SDLK_RETURN:
                    state = STANDARD_SAVE;
                    break;
                case SDLK_TAB:
                    state = OVERWRITE;
                    break;
                case SDLK_UP:
                    selector = (selector - 1) == -1 ? MAXSAVES - 1 : selector - 1;
                case SDLK_DOWN:
                    selector = (selector + 1) == MAXSAVES ? 0 : selector + 1;
                    break;
                };
            }
        }
        if (!open) {
            Game::Get()->gamestate = prev_io_menu;
        }
        {
            ImGui::Begin("Save Menu", &open, IMGUI_STANDARD_FLAGS);
            for (int i = 0; i < MAXSAVES; ++i) {
                if (selector == i)
                    ImGui::TextColored(selected, "SAVE SLOT %i: %s", files[i].c_str());
                else
                    ImGui::Text("SAVE SLOT %i: %s", files[i].c_str());
            }
            ImGui::End();
        }
        N_DebugWindowDraw();
    }
    LOG_INFO("exiting save state");
}
#endif

static inline void N_RenderSettings(bool *open)
{
    ImGui::Begin("Settings", open, IMGUI_STANDARD_FLAGS);
    ImGui::SetWindowSize(ImVec2(800, SCREEN_HEIGHT - 100));
    ImGui::SetWindowFontScale(1.5f);
    {
        ImGui::Text("<------------------------- Graphics Settings ------------------------->");
        ImGui::Checkbox("VSYNC", &scf::renderer::vsync);
        ImGui::SameLine(200);
        ImGui::Text("%s", N_booltostr2(scf::renderer::vsync));
        ImGui::SliderScalar("FPS Cap", ImGuiDataType_U16, &scf::renderer::ticrate,
            &scf::renderer::ticrate_min, &scf::renderer::ticrate_max);
    }
    ImGui::NewLine();
    {
        float sfx_vol = scf::audio::sfx_vol * 100.0f;
        float music_vol = scf::audio::music_vol * 100.0f;
        ImGui::Text("<-------------------------  Audio  Settings  ------------------------->");
        ImGui::Checkbox("SFX On", &scf::audio::sfx_on);
        ImGui::SameLine(200);
        ImGui::Text("%s", N_booltostr2(scf::audio::sfx_on));
        if (scf::audio::sfx_on && scf::audio::sfx_vol < 0.1f)
            scf::audio::sfx_vol = 0.1f;
        if (scf::audio::music_on && scf::audio::music_vol < 0.1f)
            scf::audio::music_vol = 0.1f;
        
        ImGui::Checkbox("Music On", &scf::audio::music_on);
        ImGui::SameLine(200);
        ImGui::Text("%s", N_booltostr2(scf::audio::music_on));
        if (scf::audio::sfx_on)
            ImGui::SliderFloat("SFX Volume", &sfx_vol, 1.0f, 100.0f);
        else
            ImGui::Text("SFX Off");
        if (scf::audio::music_on)
            ImGui::SliderFloat("Music Volume", &music_vol, 1.0f, 100.0f);
        else
            ImGui::Text("Music Off");
        
        scf::audio::music_vol = music_vol / 100.0f;
        scf::audio::sfx_vol = sfx_vol / 100.0f;
    }
    ImGui::NewLine();
    {
        ImGui::Text("<-------------------------  Keyboard Binds   ------------------------->");
        for (const auto& i : scf::kb_binds) {
            ImGui::Text("[%s] %s", i.name, N_ButtonToString(i.button));
        }
    }
    ImGui::End();
}

static void N_SettingsMenu()
{
    LOG_INFO("gamestate = GS_SETTINGS");
    bool open = true;

    SDL_Event event;
    while (Game::Get()->gamestate == GS_SETTINGS || open) {
        N_DebugWindowClear();
        while (SDL_PollEvent(&event)) {
            ImGui_ImplSDL2_ProcessEvent(&event);
            if (event.type == SDL_QUIT) {
                done();
            }
            else if (event.type == SDL_KEYDOWN) {
                switch (event.key.keysym.sym) {
                case SDLK_ESCAPE:
                case SDLK_BACKSPACE:
                    Game::Get()->gamestate = prev_settings_menu;
                    break;
                };
            }
        }
        N_RenderSettings(&open);
        if (!open) {
            Game::Get()->gamestate = prev_settings_menu;
        }
        N_DebugWindowDraw();
    }
#if 0
    while (Game::Get()->gamestate == GS_SETTINGS) {
        int selected = R_DrawMenu("AlegreyaFont.ttf",
                                {"Back To Previous Menu", "Graphics", "Binds", "Audio"},
                                "Settings");
        switch (selected) {
        case 0:
            Game::Get()->gamestate = prev_settings_menu;
            break;
        case 1:
            N_GraphicsSettings();
            break;
        case 2: break;
        case 3: break;
        default: {
            LOG_WARN("selected within switch statement reached \"default\" in N_SettingsMenu");
            break; }
        };
    }
#endif
    if (scf::renderer::vsync) {
//        SDL_RenderSetVSync(R_GetRenderer(), scf::renderer::vsync);
    }
    LOG_INFO("exiting settings menu");
}
static void N_LoadGame()
{
    Game::Get()->gamestate = GS_MENU;
    return;
}
static void N_SaveGame()
{
    Game::Get()->gamestate = GS_MENU;
    return;
}

void N_MainLoop()
{
    Game::Get()->gamestate = GS_MENU;
    while (1) {
        switch (Game::Get()->gamestate) {
        case GS_MENU:
            N_MainMenu();
            break;
        case GS_LEVEL:
            N_Level();
            break;
        case GS_PAUSE:
            N_PauseMenu();
            break;
        case GS_SETTINGS:
            N_SettingsMenu();
            break;
        };
    }
}