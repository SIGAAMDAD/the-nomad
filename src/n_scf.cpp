#include "n_shared.h"
#include "g_game.h"

namespace scf {
    namespace audio {
        float music_vol = 0.8f;
        float sfx_vol = 1.0f;
        bool music_on = false;
        bool sfx_on = false;
    };

    namespace renderer {
        renderapi_t api = R_SDL2;
        uint32_t height = 720;
        uint32_t width = 1024;
        bool vsync = true;
        bool hidden = false;
        bool fullscreen = false;
        bool native_fullscreen = false;
//        float ratio = width / height;
        uint16_t ticrate = 35;
        uint8_t vert_fov = 44;
        uint8_t horz_fov = 88;
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

static void I_ProcessArgs(void)
{
    for (int i = 0; i < myargc; ++i) {

    }
}

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
            scf::launch::fastmobs1 = data["launch"].contains("fastmobs1") ? static_cast<bool>(data["launch"]["fastmobs1"]) : false;
    	    scf::launch::fastmobs2 = data["launch"].contains("fastmobs2") ? static_cast<bool>(data["launch"]["fastmobs2"]) : false;
    	    scf::launch::fastmobs3 = data["launch"].contains("fastmobs3") ? static_cast<bool>(data["launch"]["fastmobs3"]) : false;
    	    scf::launch::ext_bff = data["launch"].contains("ext_bff") ? static_cast<bool>(data["launch"]["ext_bff"]) : false;
    	    scf::launch::ext_scf = data["launch"].contains("ext_scf") ? static_cast<bool>(data["launch"]["ext_scf"]) : false;
    	    scf::launch::deafmobs = data["launch"].contains("deafmobs") ? static_cast<bool>(data["launch"]["deafmobs"]) : false;
    	    scf::launch::blindmobs = data["launch"].contains("blindmobs") ? static_cast<bool>(data["launch"]["blindmobs"]) : false;
            scf::launch::nosmell = data["launch"].contains("nosmell") ? static_cast<bool>(data["launch"]["nosmell"]) : false;
            scf::launch::nomobs = data["launch"].contains("nomobs") ? static_cast<bool>(data["launch"]["nomobs"]) : false;
            scf::launch::godmode = data["launch"].contains("godmode") ? static_cast<bool>(data["launch"]["godmode"]) : false;
            scf::launch::infinite_ammo = data["launch"].contains("infinite_ammo") ? static_cast<bool>(data["launch"]["infinite_ammo"]) : false;
            scf::launch::bottomless_clip = data["launch"].contains("bottomless_clip") ? static_cast<bool>(data["launch"]["bottomless_clip"]) : false;
            scf::launch::devmode = data["launch"].contains("devmode") ? static_cast<bool>(data["launch"]["devmode"]) : false;
        }
        // renderering stuff
        const std::string api = data["renderer"]["api"];
        if (api == "R_SDL2")
            scf::renderer::api = R_SDL2;
        else if (api == "R_OPENGL")
            N_Error("G_LoadSCF: OpenGL renderering isn't yet available");
        else if (api == "R_VULKAN")
            N_Error("G_LoadSCF: Vulkan renderering isn't yet available");
        else
            N_Error("G_LoadSCF: unknown rendering api: %s", api.c_str());

        scf::renderer::drawfps = static_cast<bool>(data["renderer"]["drawfps"]);
        scf::renderer::ticrate = static_cast<uint16_t>(data["renderer"]["ticrate"]);
        scf::renderer::fullscreen = static_cast<bool>(data["renderer"]["fullscreen"]);
        scf::renderer::native_fullscreen = static_cast<bool>(data["renderer"]["native_fullscreen"]);
        scf::renderer::hidden = static_cast<bool>(data["renderer"]["window_hidden"]);
        scf::renderer::vsync = static_cast<bool>(data["renderer"]["vsync"]);
        // music/audio stuff
        scf::audio::music_vol = static_cast<float>(data["audio"]["music_vol"]);
        scf::audio::sfx_vol = static_cast<float>(data["audio"]["sfx_vol"]);
        scf::audio::sfx_on = static_cast<bool>(data["audio"]["sfx_on"]);
        scf::audio::music_on = static_cast<bool>(data["audio"]["music_on"]);
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
            "    renderer::ticrate          = {}\n"
            "    renderer::fullscreen       = {}\n"
            "    renderer::native_fullscreen= {}\n"
            "    renderer::hidden           = {}\n"
            "    renderer::vsync            = {}\n"
            "  audio::\n"
            "    audio::music_vol           = {}\n"
            "    audio::sfx_vol             = {}\n"
            "    audio::music_on            = {}\n"
            "    audio::sfx_on              = {}\n",
        scf::launch::fastmobs1, scf::launch::fastmobs2, scf::launch::fastmobs3,
        scf::launch::ext_bff, scf::launch::ext_scf, scf::launch::deafmobs,
        scf::launch::blindmobs, scf::launch::nosmell, scf::launch::nomobs,
        scf::launch::godmode, scf::launch::infinite_ammo, scf::launch::bottomless_clip,
        scf::launch::devmode,
        scf::renderer::api, scf::renderer::drawfps, scf::renderer::ticrate,
        scf::renderer::fullscreen, scf::renderer::native_fullscreen, scf::renderer::hidden,
        scf::renderer::vsync,
        scf::audio::music_vol, scf::audio::sfx_vol, scf::audio::music_on, scf::audio::sfx_on);
        LOG_INFO("done parsing scf file");
    } catch (const json::exception& e) {
        N_Error("G_LoadSCF: json error occurred, error id: %i, error message: %s", e.id, e.what());
    }
}
