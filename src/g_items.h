#ifndef _G_ITEMS_
#define _G_ITEMS_

#pragma once

typedef struct weapon_s
{
    const char* name;

    uint16_t dmg;
    int32_t range;
} weapon_t;

typedef struct item_s
{
    const char* name;

    uint16_t cost;
} item_t;

extern std::vector<weapon_t> wpninfo;

#endif