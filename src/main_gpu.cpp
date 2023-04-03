#include <SDL2/SDL.h>
#include <SDL2/SDL_gpu.h>

int main(int argc, char** argv)
{
    SDL_Init(SDL_INIT_EVENTS | SDL_INIT_VIDEO);
    GPU_SetInitWindow(0);
    GPU_Target* gpu = GPU_InitRenderer(GPU_RENDERER_OPENGL_3, 3840, 2160, GPU_INIT_ENABLE_VSYNC);
    GPU_Image* texture = GPU_LoadImage("texmap.bmp");
    bool done = false;
    SDL_Event event;
    GPU_Rect clip;
    clip.w = 16;
    clip.h = 16;
    clip.x = 32;
    clip.y = 0;
    GPU_Rect r;
    r.w = 100;
    r.h = 100;
    r.x = 150;
    r.y = 150;
    while (!done) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                done = true;
            }
            switch (event.type) {
            case SDL_KEYDOWN:
                switch (event.key.keysym.sym) {
                case SDLK_w:
                    r.y -= 150;
                    break;
                case SDLK_a:
                    r.x -= 150;
                    break;
                case SDLK_s:
                    r.y += 150;
                    break;
                case SDLK_d:
                    r.x += 150;
                    break;
                case SDLK_ESCAPE:
                    done = true;
                    break;
                };
                break;
            case SDL_KEYUP:
                break;
            };
        }
        GPU_Clear(gpu);

        GPU_BlitRect(texture, &clip, gpu, &r);

        GPU_Flip(gpu);
    }


    GPU_FreeImage(texture);
    GPU_FreeTarget(gpu);
    SDL_Quit();
    return 0;
}