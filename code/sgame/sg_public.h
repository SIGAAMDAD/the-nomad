#ifndef _SG_PUBLIC_
#define _SG_PUBLIC_

#pragma once

typedef enum
{
    VM_MEMCPY = 0,
    VM_MEMSET,
    VM_MEMMOVE,
    
    VM_COM_PRINTF,
    VM_COM_ERROR,

    VM_FS_FOPENWRITE,
    VM_FS_FOPENREAD,
    VM_FS_FCLOSE,
    VM_FS_WRITE,
    VM_FS_READ,
    VM_FS_FILESEEK,
    VM_FS_FILELENGTH,
    VM_FS_FILEEXISTS,

    VM_G_GETKEYBOARDSTATE,

    VM_CVAR_GET,

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

enum {
    KEY_A,
    KEY_B,
    KEY_C,
    KEY_D,
    KEY_E,
    KEY_F,
    KEY_G,
    KEY_H,
    KEY_I,
    KEY_J,
    KEY_K,
    KEY_L,
    KEY_M,
    KEY_N,
    KEY_O,
    KEY_P,
    KEY_Q,
    KEY_R,
    KEY_S,
    KEY_T,
    KEY_U,
    KEY_V,
    KEY_W,
    KEY_X,
    KEY_Y,
    KEY_Z,
    KEY_0,
    KEY_1,
    KEY_2,
    KEY_3,
    KEY_4,
    KEY_5,
    KEY_6,
    KEY_7,
    KEY_8,
    KEY_9,
    KEY_UP,
    KEY_DOWN,
    KEY_LEFT,
    KEY_RIGHT,
    KEY_BACKQUOTE,
    KEY_SPACE,
    KEY_BACKSPACE,
    KEY_TAB,
    KEY_LCTRL,
    KEY_RCTRL,
    KEY_ESCAPE,
    KEY_F1,
    KEY_F2,
    KEY_F3,
    KEY_F4,
    KEY_F5,
    KEY_F6,
    KEY_F7,
    KEY_F8,
    KEY_F9,
    KEY_F10,
    KEY_F11,
    KEY_F12,
    KEY_LSHIFT,
    KEY_RSHIFT,

    NUMKEYS
};

#endif
