#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>


constexpr uint16_t res_height_4k = 2160;
constexpr uint16_t res_width_4k = 3840;
constexpr uint16_t res_height_1080 = 1080;
constexpr uint16_t res_width_1080 = 1920;
constexpr uint16_t res_height_720 = 720;
constexpr uint16_t res_width_720 = 1280;


int main (int argc, char** argv)
{
    SDL_Window* window = NULL;
    window = SDL_CreateWindow
    (
        "SDL Test", SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        res_width_1080,
        res_height_1080,
        SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE
    );

    // Setup renderer
    SDL_Renderer* renderer = NULL;
    renderer =  SDL_CreateRenderer( window, -1, SDL_RENDERER_ACCELERATED);
    SDL_Texture* texture = IMG_LoadTexture(renderer, "texmap.bmp");
    SDL_Texture* bkgd = IMG_LoadTexture(renderer, "black_background.bmp");

    // Set render color to red ( background will be rendered in this color )
//    SDL_SetRenderDrawColor( renderer, 0, 0, 0, 255 );

    // Clear winow

    // Creat a rect at pos ( 50, 50 ) that's 50 pixels wide and 50 pixels high.
    const char *choice_strings[6] = {
        "New Game",
        "Load Game",
        "Settings",
        "About",
        "Credits",
        "Exit"
    };
    TTF_Init();
    TTF_Font* font = TTF_OpenFont("AlegreyaFont.ttf", 24);
    SDL_Surface* choices[6];
    SDL_Color fg = {0, 0, 0, 255};
    for (uint8_t i = 0; i < 6; ++i)
        choices[i] = TTF_RenderText_Solid(font, choice_strings[i], fg);

    SDL_Event event;
    bool done = false;
    SDL_Rect r;
    r.w = 150;
    r.h = 150;
    r.x = 150;
    r.x = 150;
    while (!done) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                done = true;
            }
        }
        SDL_RenderClear( renderer );
        SDL_RenderCopy(renderer, SDL_CreateTextureFromSurface(renderer, choices[0]), NULL, &r);

        // Render the rect to the screen
        SDL_RenderPresent(renderer);
    };
    SDL_DestroyTexture(texture);
    SDL_DestroyWindow(window);
    TTF_CloseFont(font);
    for (auto* i : choices)
        SDL_FreeSurface(i);
    TTF_Quit();
    SDL_Quit();

    return EXIT_SUCCESS;
}