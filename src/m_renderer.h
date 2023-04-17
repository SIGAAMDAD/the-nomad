#ifndef _M_RENDERER_
#define _M_RENDERER_

#pragma once

#define HARDWARE_RENDERER_FLAGS (SDL_RENDERER_ACCELERATED | (scf::renderer::vsync ? SDL_RENDERER_ACCELERATED : SDL_RENDERER_PRESENTVSYNC))
#define SOFTWARE_RENDERER_FLAGS (SDL_RENDERER_SOFTWARE | (scf::renderer::vsync ? SDL_RENDERER_SOFTWARE : SDL_RENDERER_PRESENTVSYNC))

#if 0
class Shader
{
private:
    GLuint vert_id;
    GLuint frag_id;
    GLuint geom_id;
    GLuint shader_id;
public:
    Shader(const std::string& vertfile, const std::string& fragfile, const std::string& geomfile);
    Shader(const char* vertfile, const char* fragfile, const char* geomfile);
    Shader() = default;
    Shader(const Shader &) = delete;
    Shader(Shader &&) = default;
    ~Shader();

    void Uniform1f(const char* name, float v);
    void Uniform2f(const char* name, float v0, float v1);
    void Uniform1i(const char* name, int value);
    void UniformVec2(const char* name, const glm::vec2& vec);
    void UniformVec3(const char* name, const glm::vec3& vec);
    void UniformMat4(const char* name, const glm::mat4& mat);

    GLuint Compile(const std::string &filepath, GLuint type);
    GLuint Compile(const char *filepath, GLuint type);

    void Bind(void) const;
};
#endif

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
    SDL_Renderer* screen;
    SDL_Renderer* software;
} screen_t;

typedef struct renderer_s
{
    SDL_Window* SDL_window;
    screen_t screen;
    SDL_Texture* SDL_spr_sheet;
    SDL_Surface* SDL_win_sur;
    model_t* models;
    sprite_t vmatrix[24][88];
} renderer_t;

extern std::vector<model_t> modelinfo;
extern std::unique_ptr<renderer_t> renderer;

#define DEFAULT_TEXT_SIZE 50

#ifndef _NOMAD_DEBUG
#define SDL_Call(x) if (x < 0) N_Error("%s: an SDL2 Error occurred, error message: %s\n",__func__,SDL_GetError())
#else
#define SDL_Call(x) assert(x == 0)
#endif

#define R_GetRenderer() (renderer->screen.screen ? renderer->screen.screen : renderer->screen.software)
#define R_ResetScreenColor() SDL_Call(SDL_SetRenderDrawColor(R_GetRenderer(), 0, 0, 0, SDL_ALPHA_OPAQUE))

void R_Init();
void R_ShutDown();
void R_DrawScreen();
inline SDL_Texture* R_GetSpriteSheet(void) { return renderer->SDL_spr_sheet; }
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
    SDL_Call(SDL_RenderCopy(R_GetRenderer(), renderer->SDL_spr_sheet, &modelinfo[index].offset, &modelinfo[index].screen_pos));
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

#if 0
inline texture_ptr make_texture_from_image(const char* file)
{
    assert(file);
    return texture_ptr(IMG_LoadTexture(R_GetRenderer(), file));
}
inline texture_ptr make_texture_from_font(TTF_Font* font, const char* str, const SDL_Color& color)
{
    assert(font && str);
    SDL_Surface* surface = TTF_RenderText_Solid(font, str, color);
    return texture_ptr(SDL_CreateTextureFromSurface(R_GetRenderer(), surface));
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

#if 0
class Texture
{
private:
    byte *buffer;
    GLuint id;
    uint8_t slot;
public:
    Texture() = default;
    Texture(const Texture &) = delete;
    Texture(Texture &&) = default;
    Texture(const std::string& texfile);
    Texture(const char* texfile);
    ~Texture();
};
class Shader
{
private:
    GLuint vert_id;
    GLuint frag_id;
    GLuint geom_id;
    GLuint shader_id;
public:
    Shader() = default;
    Shader(const Shader &) = delete;
    Shader(Shader &&) = default;
    Shader(const std::string& shaderfile);
    Shader(const char* shaderfile);
    ~Shader();

    void Compile(const char* src, GLuint type);
    void Uniform4f(const char* name, float v0, float v1, float v2, float v3);
    void Uniform4f(const char* name, color_t color);
};

class VertexArray
{
private:
    GLuint vao_id;
    Gluint buffer_id;
    GLuint texbuffer_id;
    std::vector<glm::vec2> vertices;
    std::vector<glm::vec2> texcoords;
public:
    VertexArray() = default;
    VertexArray(const VertexArray &) = delete;
    VertexArray(VertexArray &&) = default;
    ~VertexArray();
    VertexArray(const std::vector<glm::vec2>& _vertices, const std::vector<glm::vec2>& _texcoords);
    inline void Bind(void) const
    { glBindVertexArray(vao_id); }
    inline void UnBind(void) const
    { glBindVertexArray(0); }
    void Draw(const std::shared_ptr<Shader>& shader);
};

class Camera
{
public:
    Camera() = default;
};

void DrawScreen();
#endif

void I_CacheModels();

#endif