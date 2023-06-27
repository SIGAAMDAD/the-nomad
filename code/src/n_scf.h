#ifndef _N_SCF_
#define _N_SCF_

#pragma once

#define MAX_FOV_HEIGHT 100
#define MAX_FOV_WIDTH 250
#define TICRATE_MAX 333
#define TICRATE_MIN 35

typedef void(*pactionp_t)();

extern cvar_t g_vert_fov;
extern cvar_t g_horz_fov;
extern cvar_t g_pspeed;
extern cvar_t g_gravity;

extern cvar_t snd_musicvol;
extern cvar_t snd_sfxvol;
extern cvar_t snd_musicon;
extern cvar_t snd_sfxon;

extern cvar_t r_ticrate;
extern cvar_t r_texture_magfilter;
extern cvar_t r_texture_minfilter;
extern cvar_t r_screenheight;
extern cvar_t r_screenwidth;
extern cvar_t r_vsync;
extern cvar_t r_fullscreen;
extern cvar_t r_hidden;
extern cvar_t r_native_fullscreen;
extern cvar_t r_drawFPS;
extern cvar_t r_renderapi;
extern cvar_t r_msaa_amount;
extern cvar_t r_dither;

extern eastl::vector<const byte*> api_extensions;

extern cvar_t c_fastmobs1;
extern cvar_t c_fastmobs2;
extern cvar_t c_fastmobs3;
extern cvar_t c_deafmobs;
extern cvar_t c_blindmobs;
extern cvar_t c_nosmell;
extern cvar_t c_nomobs;
extern cvar_t c_godmode;
extern cvar_t c_infinite_ammo;
extern cvar_t c_bottomless_clip;
extern cvar_t c_devmode;
extern cvar_t c_cheatsallowed;

typedef enum
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

void G_LoadSCF();
cvar_t** G_GetCvars(void);
uint32_t G_NumCvars(void);

#endif
