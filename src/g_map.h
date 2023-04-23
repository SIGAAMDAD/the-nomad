#ifndef _G_MAP_
#define _G_MAP_

#pragma once

#include <glm/vec2.hpp>
#include <glm/vec4.hpp>
#include <glm/gtc/round.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/simd/integer.h>
#include <glm/simd/common.h>
#include <glm/simd/matrix.h>
#include <glm/simd/trigonometric.h>

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
        memmove(&(*this), &p, sizeof(pint_s));
        return *this;
    }
} pint_t;


typedef struct tile_s
{
    struct tile_s* left;
    struct tile_s* right;
    struct tile_s* up;
    struct tile_s* down;

    glm::vec2 pos;
    glm::vec4 color;
    bff_texture_t* texture;
    sprite_t tiletype;
} tile_t;

class Map
{
private:
    mutable tile_t** tilemap;
    
    static std::unique_ptr<Map> world;
public:
    Map();
    ~Map();

    static void Init();
    static void Draw();
    inline tile_t** GetTilemap() const { return tilemap; }
};

typedef std::atomic<pint_t> atomic_pint;

#endif