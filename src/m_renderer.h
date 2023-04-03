#ifndef _M_RENDERER_
#define _M_RENDERER_

#pragma once

#define HARDWARE_RENDERER_FLAGS (SDL_RENDERER_ACCELERATED | (scf::renderer::vsync ? SDL_RENDERER_ACCELERATED : SDL_RENDERER_PRESENTVSYNC))
#define SOFTWARE_RENDERER_FLAGS (SDL_RENDERER_SOFTWARE | (scf::renderer::vsync ? SDL_RENDERER_SOFTWARE : SDL_RENDERER_PRESENTVSYNC))

typedef struct model_s
{
    SDL_Rect screen_pos;
    SDL_Rect offset;
    SDL_Color color;
} model_t;

template <auto fn>
struct sdl_deleter {
    template <typename T>
    constexpr void operator()(T* arg) const { fn(arg); }
};

enum : int32_t
{
    MDL_PLAYER,
    MDL_MERC,
    MDL_HEALTH,
    MDL_ARMOR,
    MDL_COMPASS_UP,
    MDL_COMPASS_DOWN,
    MDL_COMPASS_RIGHT,
    MDL_COMPASS_LEFT,
    MDL_DOOR_OPEN,
    MDL_DOOR_CLOSED,
    MDL_FLOOR_OUTSIDE,
    MDL_FLOOR_INSIDE,
    MDL_WALL_VERTICAL,
    MDL_WALL_CORNER_BR,
    MDL_WALL_CORNER_BL,
    MDL_WALL_CORNER_TR,
    MDL_WALL_CORNER_TL,

    NUMMODELS
};

typedef std::unique_ptr<SDL_Texture, sdl_deleter<SDL_DestroyTexture>> texture_ptr;
typedef std::unique_ptr<SDL_Window, sdl_deleter<SDL_DestroyWindow>> window_ptr;
typedef std::unique_ptr<TTF_Font, sdl_deleter<TTF_CloseFont>> font_ptr;
typedef std::unique_ptr<SDL_Surface, sdl_deleter<SDL_FreeSurface>> surface_ptr;
typedef std::unique_ptr<SDL_Renderer, sdl_deleter<SDL_DestroyRenderer>> renderer_ptr;

typedef union screen_u
{
    renderer_ptr screen;
    renderer_ptr software;
} screen_t;

typedef struct renderer_s
{
    window_ptr SDL_window;
    screen_t screen;
    texture_ptr SDL_spr_sheet;
    surface_ptr SDL_win_sur;
    linked_list<model_t*> models;
    sprite_t vmatrix[24][88];
} renderer_t;

extern std::vector<model_t> modelinfo;
extern renderer_t* renderer;

#define DEFAULT_TEXT_SIZE 50

#ifndef _NOMAD_DEBUG
#define SDL_Call(x) if (x < 0) N_Error("%s: an SDL2 Error occurred, error message: %s\n",__func__,SDL_GetError())
#else
#define SDL_Call(x) assert(x == 0)
#endif

#define R_GetRenderer() (renderer->screen.screen ? renderer->screen.screen.get() : renderer->screen.software.get())
#define R_ResetScreenColor() SDL_Call(SDL_SetRenderDrawColor(R_GetRenderer(), 0, 0, 0, SDL_ALPHA_OPAQUE))

void R_Init();
void R_ShutDown();
void R_DrawScreen();
inline SDL_Texture* R_GetSpriteSheet(void) { return renderer->SDL_spr_sheet.get(); }
int R_DrawMenu(const char* fontfile, const std::vector<std::string>& choices, const char* title);

void R_DrawCompass();
void R_DrawBox(const SDL_Rect& rect);
void R_DrawBox(int32_t y, int32_t x, int32_t width, int32_t height);
void R_DrawBox(int32_t y, int32_t x, int32_t width, int32_t height, const SDL_Color& color);
void R_DrawFilledBox(const SDL_Rect& rect, const SDL_Color& fill_color);
void R_DrawFilledBox(const SDL_Rect& rect, byte r, byte g, byte b, byte a);
void R_DrawFilledBox(const SDL_Rect& rect, color_t fill_color);
void R_DrawFilledBox(int32_t y, int32_t x, int32_t width, int32_t height, const SDL_Color& fill_color);
void R_DrawFilledBox(int32_t y, int32_t x, int32_t width, int32_t height, byte r, byte g, byte b, byte a);
void R_DrawFilledBox(int32_t y, int32_t x, int32_t width, int32_t height, color_t fill_color);

inline void R_ClearScreen()
{
    SDL_Call(SDL_RenderClear(R_GetRenderer()));
}
inline void R_DrawTextureFromTable(uint32_t index)
{
    SDL_Call(SDL_RenderCopy(R_GetRenderer(), renderer->SDL_spr_sheet.get(), &modelinfo[index].offset, &modelinfo[index].screen_pos));
}
inline void R_DrawTexture(texture_ptr& texture, const SDL_Rect* src_rect, const SDL_Rect* dest_rect)
{
    assert(texture);
    SDL_Call(SDL_RenderCopy(R_GetRenderer(), texture.get(), src_rect, dest_rect));
}
inline void R_DrawTexture(SDL_Texture* texture, const SDL_Rect* src_rect, const SDL_Rect* dest_rect)
{
    assert(texture);
    SDL_Call(SDL_RenderCopy(R_GetRenderer(), texture, src_rect, dest_rect));
}
inline void R_FlushBuffer()
{
    SDL_RenderPresent(R_GetRenderer());
}
inline void R_SetScreenColor(const SDL_Color& color)
{
    SDL_Call(SDL_SetRenderDrawColor(R_GetRenderer(), color.r, color.g, color.b, color.a));
}
inline void R_SetScreenColor(color_t color)
{
    SDL_Call(SDL_SetRenderDrawColor(R_GetRenderer(), color[0], color[1], color[2], color[3]));
}
inline void R_SetScreenColor(byte r, byte g, byte b, byte a)
{
    SDL_Call(SDL_SetRenderDrawColor(R_GetRenderer(), r, g, b, a));
}

inline texture_ptr make_texture_from_image(const char* file)
{
    assert(file);
    return texture_ptr(IMG_LoadTexture(R_GetRenderer(), file));
}
inline texture_ptr make_texture_from_font(font_ptr& font, const char* str, const SDL_Color& color)
{
    assert(font && str);
    surface_ptr surface(TTF_RenderText_Solid(font.get(), str, color));
    return texture_ptr(SDL_CreateTextureFromSurface(R_GetRenderer(), surface.get()));
}
inline renderer_ptr make_renderer()
{
    return renderer_ptr(SDL_CreateRenderer(renderer->SDL_window.get(), -1,
        HARDWARE_RENDERER_FLAGS));
}
inline renderer_ptr make_software_renderer()
{
    return renderer_ptr(SDL_CreateRenderer(renderer->SDL_window.get(), -1,
        SOFTWARE_RENDERER_FLAGS));
}
inline window_ptr make_window(const char* name, int x, int y, int w, int h, Uint32 flags) {
    assert(name);
    return window_ptr(SDL_CreateWindow(name, x, y, w, h, flags));
}
inline surface_ptr make_surface()
{ return surface_ptr(SDL_GetWindowSurface(renderer->SDL_window.get())); }
inline font_ptr make_font(const char* fontfile, int fontsize) {
    assert(fontfile);
    return font_ptr(TTF_OpenFont(fontfile, fontsize));
}

#endif