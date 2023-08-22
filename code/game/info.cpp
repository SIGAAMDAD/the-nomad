#include "n_shared.h"
#include "g_game.h"
#include "n_scf.h"

keybind_t kb_binds[NUMBINDS] = {
    {"Move North",            (button_t)SDLK_w,            KMOD_NONE,  SDL_KEYDOWN,         kbMove_n,        P_MoveN},
    {"Move South",            (button_t)SDLK_s,            KMOD_NONE,  SDL_KEYDOWN,         kbMove_s,        P_MoveS},
    {"Strafe Left",           (button_t)SDLK_a,            KMOD_NONE,  SDL_KEYDOWN,         kbStrafe_l,      P_MoveW},
    {"Strafe Right",          (button_t)SDLK_d,            KMOD_NONE,  SDL_KEYDOWN,         kbStrafe_r,      P_MoveE},
    {"Slide North",           (button_t)SDLK_w,            KMOD_CTRL,  SDL_KEYDOWN,         kbSlide_n,       P_SlideN},
    {"Slide West",            (button_t)SDLK_a,            KMOD_CTRL,  SDL_KEYDOWN,         kbSlide_w,       P_SlideW},
    {"Slide South",           (button_t)SDLK_s,            KMOD_CTRL,  SDL_KEYDOWN,         kbSlide_s,       P_SlideS},
    {"Slide East",            (button_t)SDLK_d,            KMOD_CTRL,  SDL_KEYDOWN,         kbSlide_e,       P_SlideE},
    {"Dash North",            (button_t)SDLK_w,            KMOD_SHIFT, SDL_KEYDOWN,         kbDash_n,        P_DashN},
    {"Dash West",             (button_t)SDLK_a,            KMOD_SHIFT, SDL_KEYDOWN,         kbDash_w,        P_DashW},
    {"Dash South",            (button_t)SDLK_s,            KMOD_SHIFT, SDL_KEYDOWN,         kbDash_s,        P_DashS},
    {"Dash East",             (button_t)SDLK_e,            KMOD_SHIFT, SDL_KEYDOWN,         kbDash_e,        P_DashE},
    {"Use Weapon",            (button_t)SDL_BUTTON_LEFT,   KMOD_NONE,  SDL_MOUSEBUTTONDOWN, kbUseWeapon,     P_UseWeapon},
    {"Swap to Right Arm",     (button_t)SDLK_1,            KMOD_NONE,  SDL_KEYDOWN,         kbSwapWeapon_1,  P_SwapWeapon1},
    {"Swap to Left Arm",      (button_t)SDLK_2,            KMOD_NONE,  SDL_KEYDOWN,         kbSwapWeapon_2,  P_SwapWeapon2},
    {"Swap to Sidearm",       (button_t)SDLK_3,            KMOD_NONE,  SDL_KEYDOWN,         kbSwapWeapon_3,  P_SwapWeapon3},
    {"Swap to Heavy Sidearm", (button_t)SDLK_4,            KMOD_NONE,  SDL_KEYDOWN,         kbSwapWeapon_4,  P_SwapWeapon4},
    {"Swap to Primary",       (button_t)SDLK_5,            KMOD_NONE,  SDL_KEYDOWN,         kbSwapWeapon_5,  P_SwapWeapon5},
    {"Swap to Heavy Primary", (button_t)SDLK_6,            KMOD_NONE,  SDL_KEYDOWN,         kbSwapWeapon_6,  P_SwapWeapon6},
    {"Swap to Shotgun",       (button_t)SDLK_7,            KMOD_NONE,  SDL_KEYDOWN,         kbSwapWeapon_7,  P_SwapWeapon7},
    {"Swap to Melee 1",       (button_t)SDLK_8,            KMOD_NONE,  SDL_KEYDOWN,         kbSwapWeapon_8,  P_SwapWeapon8},
    {"Swap to Melee 2",       (button_t)SDLK_9,            KMOD_NONE,  SDL_KEYDOWN,         kbSwapWeapon_9,  P_SwapWeapon9},
    {"Swap to Melee 3",       (button_t)SDLK_0,            KMOD_NONE,  SDL_KEYDOWN,         kbSwapWeapon_10, P_SwapWeapon10},
    {"Next Weapon",           (button_t)MOUSE_WHEELDOWN,   KMOD_NONE,  SDL_MOUSEWHEEL,      kbNextWeapon,    P_NextWeapon},
    {"Prev Weapon",           (button_t)MOUSE_WHEELUP,     KMOD_NONE,  SDL_MOUSEWHEEL,      kbPrevWeapon,    P_PrevWeapon},
    {"Quick Swap",            (button_t)SDL_BUTTON_MIDDLE, KMOD_NONE,  SDL_MOUSEBUTTONDOWN, kbQuickSwap,     P_QuickSwap},
    {"Change Direction Left", (button_t)SDLK_q,            KMOD_NONE,  SDL_KEYDOWN,         kbChangeDirL,    P_ChangeDirL},
    {"Change Direction Right",(button_t)SDLK_e,            KMOD_NONE,  SDL_KEYDOWN,         kbChangeDirR,    P_ChangeDirR},
};

mobj_t mobinfo[NUMMOBS] = {
    {
        "Hulk",
        250,
        MF_HAS_MELEE | MF_HAS_PROJECTILE | MF_IS_LEADER
    },
};