#include <stdio.h>
#include <SDL2/SDL.h>

#define WINDOW_WIDTH 1280
#define WINDOW_HEIGHT 960
int main()
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0)
    {
        printf("error initializing SDL: %s\n ", SDL_GetError());
        return 1;
    }
    SDL_Window* win = SDL_CreateWindow("HELLO",
                                       SDL_WINDOWPOS_CENTRED,
                                       SDL_WINDOW_CENTERD,
                                       WINDOW_WIDTH, WINDOW_HEIGHT, 0);
    if (!win)
    {
        printf("error creating window: %s\n", SDL_GetError());
        SDL_Quit();
        return 1; 
    }
    

    printf("Initialization successful!\n");
    SDL_Quit();

    return 0;
}