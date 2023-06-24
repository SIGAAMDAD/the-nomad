#ifndef _SG_PUBLIC_
#define _SG_PUBLIC_

#pragma once

#ifdef Q3_VM
#include "qvmstdlib.h"
#include "nomadlib.h"
#endif

typedef enum
{
    SYS_COM_PRINTF = 0,
    SYS_COM_ERROR,
    G_GETTILEMAP,
    G_GETKEYBOARDSTATE,
    SYS_MEMCPY,
    SYS_MEMMOVE,
    SYS_MEMSET,

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

    NUMKEYS
};

#endif
