#ifndef __UI_STRINGS__
#define __UI_STRINGS__

#pragma once

#define UI_STRING(x) #x,
inline const char *uiStrings[] = {
    #include "ui_string_table.h"
    "Unknown"
};
#undef UI_STRING

#define UI_STRING(x) x,
typedef enum : uint64_t
{
    #include "ui_string_table.h"
    NUM_UI_STRINGS
} uiString_t;
#undef UI_STRING

#endif