#ifndef _N_SCF_
#define _N_SCF_

#pragma once

typedef void(*pactionp_t)();
extern int g_vert_fov;
extern int g_horz_fov;
extern float g_pspeed;
extern float g_gravity;

extern float snd_musicvol;
extern float snd_sfxvol;
extern qboolean snd_musicon;
extern qboolean snd_sfxon;

extern int r_ticrate;
extern int r_texture_magfilter;
extern int r_texture_minfilter;
extern int r_screenheight;
extern int r_screenwidth;
extern int r_renderapi;
extern qboolean r_vsync;
extern qboolean r_fullscreen;
extern qboolean r_hidden;
extern qboolean r_drawFPS;
extern int r_renderapi;
extern int r_msaa_amount;

extern nomadvector<const byte*> api_extensions;

extern qboolean c_fastmobs1;
extern qboolean c_fastmobs2;
extern qboolean c_fastmobs3;
extern qboolean c_deafmobs;
extern qboolean c_blindmobs;
extern qboolean c_nosmell;
extern qboolean c_nomobs;
extern qboolean c_godmode;
extern qboolean c_infinite_ammo;
extern qboolean c_bottomless_clip;
extern qboolean c_devmode;

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
vmCvar_t* G_GetCvars(void);
uint32_t G_NumCvars(void);

#endif
