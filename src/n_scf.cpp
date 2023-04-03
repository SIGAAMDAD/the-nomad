#include "n_shared.h"
#include "n_scf.h"

namespace scf {
    float music_vol = 0.8f;
    float sfx_vol = 1.0f;
    bool music_on = true;
    bool sfx_on = true;

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

void G_LoadSCF(const char *file)
{
    std::ifstream stream(file, std::ios::in);
    if (stream.fail()) {
        LOG_ERROR("G_LoadSCF: failed to open a std::ifstream for file %s, using default scf config params", file);
        return;
    }
    json data = json::parse(stream);
    stream.close();

    using namespace scf;

    // cheats/launch params
    launch::fastmobs1 = data["launch"].contains() ? data["launch"]["fastmobs1"]
	launch::fastmobs2 = data["launch"].contains() ? data["launch"]["fastmobs2"] : false;
	launch::fastmobs3 = data["launch"].contains() ? data["launch"]["fastmobs3"] : false;
	launch::ext_bff = data["launch"].contains() ? data["launch"]["ext_bff"] : false;
	launch::ext_scf = data["launch"].contains() ? data["launch"]["ext_scf"] : false;
	launch::deafmobs = data["launch"].contains() ? data["launch"]["deafmobs"] : false;
	launch::blindmobs = data["launch"].contains() ? data["launch"]["blindmobs"] : false;
	launch::nosmell = data["launch"].contains() ? data["launch"]["nosmell"] : false;
	launch::nomobs = data["launch"].contains() ? data["launch"]["nomobs"] : false;
	launch::godmode = data["launch"].contains() ? data["launch"]["godmode"] : false;
	launch::infinite_ammo = data["launch"].contains() ? data["launch"]["infinite_ammo"] : false;
	launch::bottomless_clip = data["launch"].contains() ? data["launch"]["bottomless_clip"] : false;
	launch::devmode = data["launch"].contains() ? data["launch"]["devmode"] : false;

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
    
    renderer::drawfps = data["renderer"]["drawfps"];
    renderer::framerate = data["renderer"]["framecap"];
    renderer::fullscreen = data["renderer"]["fullscreen"];
    renderer::native_fullscreen = data["renderer"]["native_fullscreen"];
    renderer::hidden = data["renderer"]["window_hidden"];
    renderer::vsync = data["renderer"]["vsync"];

    // music/audio stuff
    music_vol = data["audio"]["music_vol"];
    sfx_vol = data["audio"]["sfx_vol"];
    sfx_on = data["audio"]["sfx_on"];
    music_on = data["audio"]["music_on"];
}
