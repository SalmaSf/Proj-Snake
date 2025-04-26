#include <SDL.h>
#include <SDL_image.h>
#include <stdbool.h>
#include "snake.h"
#include "bakgrund.h"
#include "meny.h"

const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 700;

int main(int argc, char *argv[])
{

    SDL_Init(SDL_INIT_VIDEO);
    IMG_Init(IMG_INIT_PNG);

    SDL_Window *pWindow = SDL_CreateWindow("Snake Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, 0);
    SDL_Renderer *pRenderer = SDL_CreateRenderer(pWindow, -1, SDL_RENDERER_ACCELERATED);

    if (!visaStartMeny(pRenderer))
    {
        SDL_DestroyRenderer(pRenderer);
        SDL_DestroyWindow(pWindow);
        IMG_Quit();
        SDL_Quit();
        return 1;
    }

    SDL_Texture *pBackground = loadBackground(pRenderer, "resources/bakgrund.png");
    if (!pBackground)
        return 1;

    // Snake *pSnake = createSnake(WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2, pRenderer, WINDOW_WIDTH, WINDOW_HEIGHT);
    Snake *snake1 = createSnake(WINDOW_WIDTH / 2, 0, pRenderer, WINDOW_WIDTH, WINDOW_HEIGHT); // Topp mitten
    Snake *snake2 = createSnake(WINDOW_WIDTH / 2, WINDOW_HEIGHT, pRenderer, WINDOW_WIDTH, WINDOW_HEIGHT); // Botten mitten
    Snake *snake3 = createSnake(0, WINDOW_HEIGHT / 2, pRenderer, WINDOW_WIDTH, WINDOW_HEIGHT); // Vänster mitten
    Snake *snake4 = createSnake(WINDOW_WIDTH, WINDOW_HEIGHT / 2, pRenderer, WINDOW_WIDTH, WINDOW_HEIGHT); // Höger mitten
    
    bool isRunning = true;
    SDL_Event event;

    while (isRunning)
    {
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
            {
                isRunning = false;
            }
            else if (event.type == SDL_KEYDOWN)
            {
                if (event.key.keysym.sym == SDLK_ESCAPE)
                {
                    isRunning = false;
                }
            }
        }
        // updateSnake(pSnake);
        updateSnake(snake1);
        updateSnake(snake2);
        updateSnake(snake3);
        updateSnake(snake4);

        SDL_SetRenderDrawColor(pRenderer, 0, 0, 0, 255);
        SDL_RenderClear(pRenderer);

        SDL_RenderCopy(pRenderer, pBackground, NULL, NULL);

        // drawSnake(pSnake);
        drawSnake(snake1);
        drawSnake(snake2);
        drawSnake(snake3);
        drawSnake(snake4);

        SDL_RenderPresent(pRenderer);
        SDL_Delay(16); // ~60 FPS
    }

    // destroySnake(pSnake);
    destroySnake(snake1);
    destroySnake(snake2);
    destroySnake(snake3);
    destroySnake(snake4);

    SDL_DestroyRenderer(pRenderer);
    SDL_DestroyWindow(pWindow);
    SDL_DestroyTexture(pBackground);
    IMG_Quit();
    SDL_Quit();

    return 0;
}