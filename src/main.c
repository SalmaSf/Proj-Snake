#include <stdio.h>
#include <SDL.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_timer.h>
#include <SDL2/SDL_image.h>

#define WINDOW_WIDTH 1280
#define WINDOW_HEIGHT 960

int main()
{
    
}

int initilize ()
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

    Unit32 render_flags = SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC;
    SDL_Renderer* rend = SDL_CreateRenderer(win, -1, render_flags);
    if (!rend)
    {
        SDL_DestroyWindow(win);
        SDL_Quit();
        return 1;
    }
    
   /* SDL_Surface* surface = IMG_Load("??????");
    if (!surface)
    {
        printf("eroor creating surface\n");
        SDL_DestroyRenderer(rend);
        SDL_DestroyWindoe(win);
        SDL_Quit();
        return 1;
    }*/
    

    SDL_Delay(5000);
    SDL_DestroyWindow(win);
    SDL_Quit();

    printf("Initialization successful!\n");
    SDL_Quit();

    return 0;
}