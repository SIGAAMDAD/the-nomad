#ifndef _N_SCF_
#define _N_SCF_

#ifndef Q3_VM

#pragma once

#define MAX_FOV_HEIGHT 100
#define MAX_FOV_WIDTH 250
#define TICRATE_MAX 333
#define TICRATE_MIN 35

//extern cvar_t com_demo;

//extern cvar_t g_vert_fov;
//extern cvar_t g_horz_fov;
//extern cvar_t g_pspeed;
//extern cvar_t g_gravity;

//extern cvar_t r_ticrate;
//extern cvar_t r_screenheight;
//extern cvar_t r_screenwidth;
//extern cvar_t r_vsync;
//extern cvar_t r_fullscreen;
//extern cvar_t r_native_fullscreen;
//extern cvar_t r_hidden;
//extern cvar_t r_drawFPS;
//extern cvar_t r_renderapi;
//extern cvar_t r_multisampleAmount;
//extern cvar_t r_multisampleType;
//extern cvar_t r_dither;
//extern cvar_t r_EXT_anisotropicFiltering;
//extern cvar_t r_gammaAmount;
//extern cvar_t r_textureMagFilter;
//extern cvar_t r_textureMinFilter;
//extern cvar_t r_textureFiltering;
//extern cvar_t r_textureCompression;
//extern cvar_t r_textureDetail;
//extern cvar_t r_bloomOn;
//extern cvar_t r_useExtensions;
//extern cvar_t r_fovWidth;
//extern cvar_t r_fovHeight;
//
//extern cvar_t c_fastmobs1;
//extern cvar_t c_fastmobs2;
//extern cvar_t c_fastmobs3;
//extern cvar_t c_deafmobs;
//extern cvar_t c_blindmobs;
//extern cvar_t c_nosmell;
//extern cvar_t c_nomobs;
//extern cvar_t c_godmode;
//extern cvar_t c_infinite_ammo;
//extern cvar_t c_bottomless_clip;

#endif

typedef enum
{
    // player movement
    kbMoveUp = 0,
    kbMoveDown,
    kbMoveLeft,
    kbMoveRight,

    // camera movement
    kbCameraUp,
    kbCameraDown,
    kbCameraLeft,
    kbCameraRight,
    kbZoomIn,
    kbZoomOut,

    // general actions
    kbShoot,
    kbJump,
    kbCrouch,
    
    // misc.
    kbConsole,
    kbExit,

    NUMBINDS
} bind_t;

#ifndef Q3_VM

typedef struct keybind_s
{
    const char *name;
    SDL_KeyCode button;
    SDL_Keymod mod;
    SDL_EventType type;
    bind_t bind;
} keybind_t;

extern keybind_t kb_binds[NUMBINDS];

void Com_LoadConfig(void);

#endif

#endif
