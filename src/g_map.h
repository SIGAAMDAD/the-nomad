#ifndef _G_MAP_
#define _G_MAP_

#pragma once

typedef struct pint_s
{
    sprite_t sprite;
    coord_t pos;
    SDL_Rect r_info;
    SDL_Color color;
    
    inline pint_s(sprite_t _sprite, const coord_t& _pos, SDL_Rect _r_info, SDL_Color _color)
        : sprite(_sprite), r_info(_r_info), color(_color)
    { pos = _pos; }
    inline pint_s() = default;
    inline pint_s(sprite_t _sprite, SDL_Rect _r_info, SDL_Color _color)
        : sprite(_sprite), r_info(_r_info), color(_color)
    {
    }
    inline ~pint_s() = default;
    inline pint_s(pint_s &&) = default;
    inline pint_s(const pint_s &) = default;

    inline pint_s& operator=(const pint_s& p) {
        N_memcpy(&(*this), &p, sizeof(pint_s));
        return *this;
    }
} pint_t;

typedef std::atomic<pint_t> atomic_pint;

#endif