#ifndef _N_SCF_
#define _N_SCF_

#pragma once

typedef void(*pactionp_t)();

#define framerate_macro (1000 / scf::renderer::framerate)
#define timeperframe    framerate_macro

constexpr uint16_t res_height_4k = 2160;
constexpr uint16_t res_width_4k = 3840;
constexpr uint16_t res_height_1080 = 1080;
constexpr uint16_t res_width_1080 = 1920;
constexpr uint16_t res_height_720 = 720;
constexpr uint16_t res_width_720 = 1280;

namespace scf {
    namespace audio {
        extern float music_vol;
        extern float sfx_vol;
        extern bool music_on;
        extern bool sfx_on;
    };

    typedef enum renderapi_e : uint8_t
    {
        R_SDL2,
        R_OPENGL,
        R_VULKAN
    } renderapi_t;
    
    enum : uint8_t
    {
        RES_720,
        RES_1080,
        RES_4K,
        NUMRESOLUTIONS
    };

    namespace renderer {
        extern renderapi_t api;
        extern uint32_t height;
        extern uint32_t width;
        extern uint8_t resolution;
        extern bool vsync;
        extern bool hidden;
        extern bool fullscreen;
        extern bool native_fullscreen;
        extern float ratio;
        extern uint16_t framerate;
        extern bool drawfps;
        
        // min-max fps caps
        constexpr uint16_t framerate_max = 1000;
        constexpr uint16_t framerate_min = 30;
    };
    namespace launch {
        extern bool fastmobs1;
		extern bool fastmobs2;
		extern bool fastmobs3;
		extern bool ext_bff;
		extern bool ext_scf;
		extern bool deafmobs;
		extern bool blindmobs;
		extern bool nosmell;
		extern bool nomobs;
		extern bool godmode;
		extern bool infinite_ammo;
		extern bool bottomless_clip;
		extern bool devmode;
		constexpr uint16_t numlaunchparams = 11;
    };


    typedef enum bind_e : uint16_t
    {
        kbMove_n,
        kbMove_s,
        kbStrafe_l,
        kbStrafe_r,
        kbSlide_n,
        kbSlide_w,
        kbSlide_s,
        kbSlide_e,
        kbDash_n,
        kbDash_w,
        kbDash_s,
        kbDash_e,
        kbUseWeapon,
        kbSwapWeapon_1,
        kbSwapWeapon_2,
        kbSwapWeapon_3,
        kbSwapWeapon_4,
        kbSwapWeapon_5,
        kbSwapWeapon_6,
        kbSwapWeapon_7,
        kbSwapWeapon_8,
        kbSwapWeapon_9,
        kbSwapWeapon_10,
        kbNextWeapon,
        kbPrevWeapon,
        kbQuickSwap,
        kbChangeDirL,
        kbChangeDirR,

        NUMBINDS
    } bind_t;

    typedef uint32_t button_t;

    typedef struct keybind_s
    {
        const char* name;
        button_t button;
        SDL_Keymod mod;
        SDL_EventType type;
        bind_t bind;
        pactionp_t action;
    } keybind_t;

    extern keybind_t kb_binds[NUMBINDS];
};

void G_LoadSCF();

// hard-coded sprite positions for the various resolutions
const non_atomic_coord_t playr_pos[scf::NUMRESOLUTIONS] = {
    {565, 300},
    {490, 910},
    {885, 1845}
};

#endif