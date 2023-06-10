#ifndef _SG_PUBLIC_
#define _SG_PUBLIC_

#pragma once

#ifdef Q3_VM
#include "qvmstdlib.h"
#include "nomadlib.h"
#endif

typedef enum
{
    SYS_CON_PRINTF = 0,
    SYS_CON_ERROR,
    CON_FLUSH,
    G_PLAYSFX,
    G_GETTILEMAP,
    G_SAVEGAME,
    G_LOADGAME,
    G_GETEVENTS,
    CVAR_REGISTER,
    CVAR_CHANGEVALUE,
    CVAR_REGISTERNAME,

    NUM_SGAME_IMPORT
} sgameImport_t;

typedef enum
{
    SGAME_INIT,
    SGAME_SHUTDOWN,
    SGAME_RUNTIC,
    SGAME_STARTLEVEL,
    SGAME_ENDLEVEL,
} sgameExport_t;

void G_GetTilemap(sprite_t tilemap[MAP_MAX_Y][MAP_MAX_X]);
void G_SaveGame(uint32_t slot, const void *data, const uint64_t size);
void G_LoadGame(uint32_t slot, void *data, uint64_t *size);

#ifdef Q3_VM

typedef enum SDLKey
{
    SDLK_UNKNOWN = 0,
    SDLK_FIRST = 0,
    SDLK_BACKSPACE = 8,
    SDLK_TAB = 9,
    SDLK_CLEAR = 12,
    SDLK_RETURN = 13,
    SDLK_PAUSE = 19,
    SDLK_ESCAPE = 27,
    SDLK_SPACE = 32,
    SDLK_EXCLAIM = 33,
    SDLK_QUOTEDBL = 34,
    SDLK_HASH = 35,
    SDLK_DOLLAR = 36,
    SDLK_AMPERSAND = 38,
    SDLK_QUOTE = 39,
    SDLK_LEFTPAREN = 40,
    SDLK_RIGHTPAREN = 41,
    SDLK_ASTERISK = 42,
    SDLK_PLUS = 43,
    SDLK_COMMA = 44,
    SDLK_MINUS = 45,
    SDLK_PERIOD = 46,
    SDLK_SLASH = 47,
    SDLK_0 = 48,
    SDLK_1 = 49,
    SDLK_2 = 50,
    SDLK_3 = 51,
    SDLK_4 = 52,
    SDLK_5 = 53,
    SDLK_6 = 54,
    SDLK_7 = 55,
    SDLK_8 = 56,
    SDLK_9 = 57,
    SDLK_COLON = 58,
    SDLK_SEMICOLON = 59,
    SDLK_LESS = 60,
    SDLK_EQUALS = 61,
    SDLK_GREATER = 62,
    SDLK_QUESTION = 63,
    SDLK_AT = 64,
    SDLK_LEFTBRACKET = 91,
    SDLK_BACKSLASH = 92,
    SDLK_RIGHTBRACKET = 93,
    SDLK_CARET = 94,
    SDLK_UNDERSCORE = 95,
    SDLK_BACKQUOTE = 96,
    SDLK_a = 97,
    SDLK_b = 98,
    SDLK_c = 99,
    SDLK_d = 100,
    SDLK_e = 101,
    SDLK_f = 102,
    SDLK_g = 103,
    SDLK_h = 104,
    SDLK_i = 105,
    SDLK_j = 106,
    SDLK_k = 107,
    SDLK_l = 108,
    SDLK_m = 109,
    SDLK_n = 110,
    SDLK_o = 111,
    SDLK_p = 112,
    SDLK_q = 113,
    SDLK_r = 114,
    SDLK_s = 115,
    SDLK_t = 116,
    SDLK_u = 117,
    SDLK_v = 118,
    SDLK_w = 119,
    SDLK_x = 120,
    SDLK_y = 121,
    SDLK_z = 122,
    SDLK_DELETE = 127,
    SDLK_WORLD_0 = 160,
    SDLK_WORLD_1 = 161,
    SDLK_WORLD_2 = 162,
    SDLK_WORLD_3 = 163,
    SDLK_WORLD_4 = 164,
    SDLK_WORLD_5 = 165,
    SDLK_WORLD_6 = 166,
    SDLK_WORLD_7 = 167,
    SDLK_WORLD_8 = 168,
    SDLK_WORLD_9 = 169,
    SDLK_WORLD_10 = 170,
    SDLK_WORLD_11 = 171,
    SDLK_WORLD_12 = 172,
    SDLK_WORLD_13 = 173,
    SDLK_WORLD_14 = 174,
    SDLK_WORLD_15 = 175,
    SDLK_WORLD_16 = 176,
    SDLK_WORLD_17 = 177,
    SDLK_WORLD_18 = 178,
    SDLK_WORLD_19 = 179,
    SDLK_WORLD_20 = 180,
    SDLK_WORLD_21 = 181,
    SDLK_WORLD_22 = 182,
    SDLK_WORLD_23 = 183,
    SDLK_WORLD_24 = 184,
    SDLK_WORLD_25 = 185,
    SDLK_WORLD_26 = 186,
    SDLK_WORLD_27 = 187,
    SDLK_WORLD_28 = 188,
    SDLK_WORLD_29 = 189,
    SDLK_WORLD_30 = 190,
    SDLK_WORLD_31 = 191,
    SDLK_WORLD_32 = 192,
    SDLK_WORLD_33 = 193,
    SDLK_WORLD_34 = 194,
    SDLK_WORLD_35 = 195,
    SDLK_WORLD_36 = 196,
    SDLK_WORLD_37 = 197,
    SDLK_WORLD_38 = 198,
    SDLK_WORLD_39 = 199,
    SDLK_WORLD_40 = 200,
    SDLK_WORLD_41 = 201,
    SDLK_WORLD_42 = 202,
    SDLK_WORLD_43 = 203,
    SDLK_WORLD_44 = 204,
    SDLK_WORLD_45 = 205,
    SDLK_WORLD_46 = 206,
    SDLK_WORLD_47 = 207,
    SDLK_WORLD_48 = 208,
    SDLK_WORLD_49 = 209,
    SDLK_WORLD_50 = 210,
    SDLK_WORLD_51 = 211,
    SDLK_WORLD_52 = 212,
    SDLK_WORLD_53 = 213,
    SDLK_WORLD_54 = 214,
    SDLK_WORLD_55 = 215,
    SDLK_WORLD_56 = 216,
    SDLK_WORLD_57 = 217,
    SDLK_WORLD_58 = 218,
    SDLK_WORLD_59 = 219,
    SDLK_WORLD_60 = 220,
    SDLK_WORLD_61 = 221,
    SDLK_WORLD_62 = 222,
    SDLK_WORLD_63 = 223,
    SDLK_WORLD_64 = 224,
    SDLK_WORLD_65 = 225,
    SDLK_WORLD_66 = 226,
    SDLK_WORLD_67 = 227,
    SDLK_WORLD_68 = 228,
    SDLK_WORLD_69 = 229,
    SDLK_WORLD_70 = 230,
    SDLK_WORLD_71 = 231,
    SDLK_WORLD_72 = 232,
    SDLK_WORLD_73 = 233,
    SDLK_WORLD_74 = 234,
    SDLK_WORLD_75 = 235,
    SDLK_WORLD_76 = 236,
    SDLK_WORLD_77 = 237,
    SDLK_WORLD_78 = 238,
    SDLK_WORLD_79 = 239,
    SDLK_WORLD_80 = 240,
    SDLK_WORLD_81 = 241,
    SDLK_WORLD_82 = 242,
    SDLK_WORLD_83 = 243,
    SDLK_WORLD_84 = 244,
    SDLK_WORLD_85 = 245,
    SDLK_WORLD_86 = 246,
    SDLK_WORLD_87 = 247,
    SDLK_WORLD_88 = 248,
    SDLK_WORLD_89 = 249,
    SDLK_WORLD_90 = 250,
    SDLK_WORLD_91 = 251,
    SDLK_WORLD_92 = 252,
    SDLK_WORLD_93 = 253,
    SDLK_WORLD_94 = 254,
    SDLK_WORLD_95 = 255,
    SDLK_KP0 = 256,
    SDLK_KP1 = 257,
    SDLK_KP2 = 258,
    SDLK_KP3 = 259,
    SDLK_KP4 = 260,
    SDLK_KP5 = 261,
    SDLK_KP6 = 262,
    SDLK_KP7 = 263,
    SDLK_KP8 = 264,
    SDLK_KP9 = 265,
    SDLK_KP_PERIOD = 266,
    SDLK_KP_DIVIDE = 267,
    SDLK_KP_MULTIPLY = 268,
    SDLK_KP_MINUS = 269,
    SDLK_KP_PLUS = 270,
    SDLK_KP_ENTER = 271,
    SDLK_KP_EQUALS = 272,
    SDLK_UP = 273,
    SDLK_DOWN = 274,
    SDLK_RIGHT = 275,
    SDLK_LEFT = 276,
    SDLK_INSERT = 277,
    SDLK_HOME = 278,
    SDLK_END = 279,
    SDLK_PAGEUP = 280,
    SDLK_PAGEDOWN = 281,
    SDLK_F1 = 282,
    SDLK_F2 = 283,
    SDLK_F3 = 284,
    SDLK_F4 = 285,
    SDLK_F5 = 286,
    SDLK_F6 = 287,
    SDLK_F7 = 288,
    SDLK_F8 = 289,
    SDLK_F9 = 290,
    SDLK_F10 = 291,
    SDLK_F11 = 292,
    SDLK_F12 = 293,
    SDLK_F13 = 294,
    SDLK_F14 = 295,
    SDLK_F15 = 296,
    SDLK_NUMLOCK = 300,
    SDLK_CAPSLOCK = 301,
    SDLK_SCROLLOCK = 302,
    SDLK_RSHIFT = 303,
    SDLK_LSHIFT = 304,
    SDLK_RCTRL = 305,
    SDLK_LCTRL = 306,
    SDLK_RALT = 307,
    SDLK_LALT = 308,
    SDLK_RMETA = 309,
    SDLK_LMETA = 310,
    SDLK_LSUPER = 311,
    SDLK_RSUPER = 312,
    SDLK_MODE = 313,
    SDLK_COMPOSE = 314,
    SDLK_HELP = 315,
    SDLK_PRINT = 316,
    SDLK_SYSREQ = 317,
    SDLK_BREAK = 318,
    SDLK_MENU = 319,
    SDLK_POWER = 320,
    SDLK_EURO = 321,
    SDLK_UNDO = 322,
    SDLK_LAST
} SDLKey;

typedef enum SDLMod
{
    KMOD_NONE = 0x0000,
    KMOD_LSHIFT= 0x0001,
    KMOD_RSHIFT= 0x0002,
    KMOD_LCTRL = 0x0040,
    KMOD_RCTRL = 0x0080,
    KMOD_LALT = 0x0100,
    KMOD_RALT = 0x0200,
    KMOD_LMETA = 0x0400,
    KMOD_RMETA = 0x0800,
    KMOD_NUM = 0x1000,
    KMOD_CAPS = 0x2000,
    KMOD_MODE = 0x4000,
    KMOD_RESERVED = 0x8000
} SDLMod;

typedef struct SDL_ActiveEvent
{
    byte type;
    byte gain;
    byte state;
} SDL_ActiveEvent;

typedef struct SDL_keysym
{
    byte scancode;
    SDLKey sym;
    SDLMod mod;
    unsigned short unicode;
} SDL_keysym;

typedef struct SDL_KeyboardEvent
{
    byte type;
    byte which;
    byte state;
    SDL_keysym keysym;
} SDL_KeyboardEvent;

typedef struct SDL_MouseMotionEvent
{
    byte type;
    byte which;
    byte state;
    unsigned short x;
    unsigned short y;
    short xrel;
    short yrel;
} SDL_MouseMotionEvent;

typedef struct SDL_MouseButtonEvent
{
    byte type;
    byte which;
    byte button;
    byte state;
    unsigned short x;
    unsigned short y;
} SDL_MouseButtonEvent;

typedef struct SDL_JoyAxisEvent
{
    byte type;
    byte which;
    byte axis;
    short value;
} SDL_JoyAxisEvent;

typedef struct SDL_JoyBallEvent
{
    byte type;
    byte which;
    byte ball;
    short xrel;
    short yrel;
} SDL_JoyBallEvent;

typedef struct SDL_JoyHatEvent
{
    byte type;
    byte which;
    byte hat;
    byte value;
} SDL_JoyHatEvent;

typedef struct SDL_JoyButtonEvent
{
    byte type;
    byte which;
    byte button;
    byte state;
} SDL_JoyButtonEvent;

typedef struct SDL_ResizeEvent
{
    byte type;
    int w;
    int h;
} SDL_ResizeEvent;

typedef struct SDL_ExposeEvent
{
    byte type;
} SDL_ExposeEvent;

typedef struct SDL_QuitEvent
{
    byte type;
} SDL_QuitEvent;

typedef struct SDL_UserEvent
{
    byte type;
    int code;
    void *data1;
    void *data2;
} SDL_UserEvent;

struct SDL_SysWMmsg;
typedef struct SDL_SysWMmsg SDL_SysWMmsg;
typedef struct SDL_SysWMEvent
{
    byte type;
    struct SDL_SysWMmsg *msg;
} SDL_SysWMEvent;

typedef union SDL_Event
{
    byte type;
    SDL_ActiveEvent active;
    SDL_KeyboardEvent key;
    SDL_MouseMotionEvent motion;
    SDL_MouseButtonEvent button;
    SDL_JoyAxisEvent jaxis;
    SDL_JoyBallEvent jball;
    SDL_JoyHatEvent jhat;
    SDL_JoyButtonEvent jbutton;
    SDL_ResizeEvent resize;
    SDL_ExposeEvent expose;
    SDL_QuitEvent quit;
    SDL_UserEvent user;
    SDL_SysWMEvent syswm;
} SDL_Event;
#endif

void G_GetEvents(SDL_Event* event);

#ifdef Q3_VM

#define KMOD_CTRL (KMOD_LCTRL|KMOD_RCTRL)
#define KMOD_SHIFT (KMOD_LSHIFT|KMOD_RSHIFT)
#define KMOD_ALT (KMOD_LALT|KMOD_RALT)
#define KMOD_META (KMOD_LMETA|KMOD_RMETA)

typedef enum SDL_EventType
{
    SDL_NOEVENT,
    SDL_ACTIVEEVENT,
    SDL_KEYDOWN,
    SDL_KEYUP,
    SDL_MOUSEMOTION,
    SDL_MOUSEBUTTONDOWN,
    SDL_MOUSEBUTTONUP,
    SDL_JOYAXISMOTION,
    SDL_JOYBALLMOTION,
    SDL_JOYHATMOTION,
    SDL_JOYBUTTONDOWN,
    SDL_JOYBUTTONUP,
    SDL_QUIT,
    SDL_SYSWMEVENT,
    SDL_EVENT_RESERVEDA,
    SDL_EVENT_RESERVEDB,
    SDL_VIDEORESIZE,
    SDL_VIDEOEXPOSE,
    SDL_EVENT_RESERVED2,
    SDL_EVENT_RESERVED3,
    SDL_EVENT_RESERVED4,
    SDL_EVENT_RESERVED5,
    SDL_EVENT_RESERVED6,
    SDL_EVENT_RESERVED7,
    SDL_USEREVENT = 24,
    SDL_NUMEVENTS = 32
} SDL_EventType;

#define SDL_EVENTMASK(X) (1<<(X))
#define SDL_ALLEVENTS 0xFFFFFFFF

typedef enum SDL_EventMask
{
    SDL_ACTIVEEVENTMASK = SDL_EVENTMASK(SDL_ACTIVEEVENT),
    SDL_KEYDOWNMASK = SDL_EVENTMASK(SDL_KEYDOWN),
    SDL_KEYUPMASK = SDL_EVENTMASK(SDL_KEYUP),
    SDL_KEYEVENTMASK = SDL_EVENTMASK(SDL_KEYDOWN)|SDL_EVENTMASK(SDL_KEYUP),
    SDL_MOUSEMOTIONMASK = SDL_EVENTMASK(SDL_MOUSEMOTION),
    SDL_MOUSEBUTTONDOWNMASK = SDL_EVENTMASK(SDL_MOUSEBUTTONDOWN),
    SDL_MOUSEBUTTONUPMASK = SDL_EVENTMASK(SDL_MOUSEBUTTONUP),
    SDL_MOUSEEVENTMASK = SDL_EVENTMASK(SDL_MOUSEMOTION)|SDL_EVENTMASK(SDL_MOUSEBUTTONDOWN)|SDL_EVENTMASK(SDL_MOUSEBUTTONUP),
    SDL_JOYAXISMOTIONMASK = SDL_EVENTMASK(SDL_JOYAXISMOTION),
    SDL_JOYBALLMOTIONMASK = SDL_EVENTMASK(SDL_JOYBALLMOTION),
    SDL_JOYHATMOTIONMASK = SDL_EVENTMASK(SDL_JOYHATMOTION),
    SDL_JOYBUTTONDOWNMASK = SDL_EVENTMASK(SDL_JOYBUTTONDOWN),
    SDL_JOYBUTTONUPMASK = SDL_EVENTMASK(SDL_JOYBUTTONUP),
    SDL_JOYEVENTMASK = SDL_EVENTMASK(SDL_JOYAXISMOTION)|SDL_EVENTMASK(SDL_JOYBALLMOTION)|SDL_EVENTMASK(SDL_JOYHATMOTION)|SDL_EVENTMASK(SDL_JOYBUTTONDOWN)|SDL_EVENTMASK(SDL_JOYBUTTONUP),
    SDL_VIDEORESIZEMASK = SDL_EVENTMASK(SDL_VIDEORESIZE),
    SDL_VIDEOEXPOSEMASK = SDL_EVENTMASK(SDL_VIDEOEXPOSE),
    SDL_QUITMASK = SDL_EVENTMASK(SDL_QUIT),
    SDL_SYSWMEVENTMASK = SDL_EVENTMASK(SDL_SYSWMEVENT)
} SDL_EventMask;
#endif

#endif
