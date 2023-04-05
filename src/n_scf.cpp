#include "n_shared.h"
#include "g_game.h"

namespace scf {
    namespace audio {
        float music_vol = 0.8f;
        float sfx_vol = 1.0f;
        bool music_on = true;
        bool sfx_on = true;
    };

    namespace renderer {
        renderapi_t api = R_SDL2;
        uint32_t height = res_height_1080;
        uint32_t width = res_width_1080;
        bool vsync = true;
        bool hidden = false;
        bool fullscreen = false;
        bool native_fullscreen = false;
        float ratio = width / framerate;
        uint16_t framerate = 125;
#ifdef _NOMAD_DEBUG
        bool drawfps = true;
#else
        bool drawfps = false;
#endif
    };
    namespace launch {
        bool fastmobs1 = false;
		bool fastmobs2 = false;
		bool fastmobs3 = false;
		bool ext_bff = false;
		bool ext_scf = false;
		bool deafmobs = false;
		bool blindmobs = false;
		bool nosmell = false;
		bool nomobs = false;
		bool godmode = false;
		bool infinite_ammo = false;
		bool bottomless_clip = false;
		bool devmode = false;
    };
};

#define PRINTARG(x) fmt::arg(#x, x)

void G_LoadSCF()
{
    try {
        std::ifstream stream("default.scf", std::ios::in);
        if (stream.fail()) {
            LOG_ERROR("G_LoadSCF: failed to open a std::ifstream for file default.scf, using default scf config params");
            return;
        }
        json data = json::parse(stream);
        stream.close();
        using namespace scf;
        // cheats/launch params
        if (data.contains("launch")) {
            launch::fastmobs1 = data["launch"].contains("fastmobs1") ? static_cast<bool>(data["launch"]["fastmobs1"]) : false;
    	    launch::fastmobs2 = data["launch"].contains("fastmobs2") ? static_cast<bool>(data["launch"]["fastmobs2"]) : false;
    	    launch::fastmobs3 = data["launch"].contains("fastmobs3") ? static_cast<bool>(data["launch"]["fastmobs3"]) : false;
    	    launch::ext_bff = data["launch"].contains("ext_bff") ? static_cast<bool>(data["launch"]["ext_bff"]) : false;
    	    launch::ext_scf = data["launch"].contains("ext_scf") ? static_cast<bool>(data["launch"]["ext_scf"]) : false;
    	    launch::deafmobs = data["launch"].contains("deafmobs") ? static_cast<bool>(data["launch"]["deafmobs"]) : false;
    	    launch::blindmobs = data["launch"].contains("blindmobs") ? static_cast<bool>(data["launch"]["blindmobs"]) : false;
            launch::nosmell = data["launch"].contains("nosmell") ? static_cast<bool>(data["launch"]["nosmell"]) : false;
            launch::nomobs = data["launch"].contains("nomobs") ? static_cast<bool>(data["launch"]["nomobs"]) : false;
            launch::godmode = data["launch"].contains("godmode") ? static_cast<bool>(data["launch"]["godmode"]) : false;
            launch::infinite_ammo = data["launch"].contains("infinite_ammo") ? static_cast<bool>(data["launch"]["infinite_ammo"]) : false;
            launch::bottomless_clip = data["launch"].contains("bottomless_clip") ? static_cast<bool>(data["launch"]["bottomless_clip"]) : false;
            launch::devmode = data["launch"].contains("devmode") ? static_cast<bool>(data["launch"]["devmode"]) : false;
        }
        // renderering stuff
        const std::string api = data["renderer"]["api"];
        if (api == "R_SDL2")
            renderer::api = R_SDL2;
        else if (api == "R_OPENGL")
            N_Error("G_LoadSCF: OpenGL renderering isn't yet available");
        else if (api == "R_VULKAN")
            N_Error("G_LoadSCF: Vulkan renderering isn't yet available");
        else
            N_Error("G_LoadSCF: unknown renndering api: %s", api.c_str());
        renderer::drawfps = static_cast<bool>(data["renderer"]["drawfps"]);
        renderer::framerate = static_cast<uint16_t>(data["renderer"]["framecap"]);
        renderer::fullscreen = static_cast<bool>(data["renderer"]["fullscreen"]);
        renderer::native_fullscreen = static_cast<bool>(data["renderer"]["native_fullscreen"]);
        renderer::hidden = static_cast<bool>(data["renderer"]["window_hidden"]);
        renderer::vsync = static_cast<bool>(data["renderer"]["vsync"]);
        // music/audio stuff
        audio::music_vol = static_cast<float>(data["audio"]["music_vol"]);
        audio::sfx_vol = static_cast<float>(data["audio"]["sfx_vol"]);
        audio::sfx_on = static_cast<bool>(data["audio"]["sfx_on"]);
        audio::music_on = static_cast<bool>(data["audio"]["music_on"]);
        LOG_INFO(
            "scf configuration:\n"
            "  launch::\n"
            "    launch::fastmobs1          = {}\n"
            "    launch::fastmobs2          = {}\n"
            "    launch::fastmobs3          = {}\n"
            "    launch::ext_bff            = {}\n"
            "    launch::ext_scf            = {}\n"
            "    launch::deafmobs           = {}\n"
            "    launch::blindmobs          = {}\n"
            "    launch::nosmell            = {}\n"
            "    launch::nomobs             = {}\n"
            "    launch::godmode            = {}\n"
            "    launch::infinite_ammo      = {}\n"
            "    launch::bottomless_clip    = {}\n"
            "    launch::devmode            = {}\n"
            "  renderer::\n"
            "    renderer::api              = {}\n"
            "    renderer::drawfps          = {}\n"
            "    renderer::framerate        = {}\n"
            "    renderer::fullscreen       = {}\n"
            "    renderer::native_fullscreen= {}\n"
            "    renderer::hidden           = {}\n"
            "    renderer::vsync            = {}\n"
            "  audio::\n"
            "    audio::music_vol           = {}\n"
            "    audio::sfx_vol             = {}\n"
            "    audio::music_on            = {}\n"
            "    audio::sfx_on              = {}\n",
        launch::fastmobs1, launch::fastmobs2, launch::fastmobs3,
        launch::ext_bff, launch::ext_scf, launch::deafmobs,
        launch::blindmobs, launch::nosmell, launch::nomobs,
        launch::godmode, launch::infinite_ammo, launch::bottomless_clip,
        launch::devmode,
        renderer::api, renderer::drawfps, renderer::framerate,
        renderer::fullscreen, renderer::native_fullscreen, renderer::hidden,
        renderer::vsync,
        audio::music_vol, audio::sfx_vol, audio::music_on, audio::sfx_on);
        LOG_INFO("done parsing scf file");
    } catch (const json::exception& e) {
        N_Error("G_LoadSCF: json error occurred, error id: %i, error message: %s", e.id, e.what());
    }
}
