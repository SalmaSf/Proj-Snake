<<<<<<< HEAD
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

 // Visa startmeny först
    if (!visaStartMeny(pRenderer)) 
    {
        SDL_DestroyRenderer(pRenderer);
        SDL_DestroyWindow(pWindow);
        IMG_Quit();
        SDL_Quit();
        return 1;
    }
    if (!visaIPMeny(pRenderer))    
    {
        SDL_DestroyRenderer(pRenderer);
        SDL_DestroyWindow(pWindow);
        IMG_Quit();
        SDL_Quit();
        return 1;
    }

// Starta spelet efter IP-inmatning
    SDL_Texture* pBackground = loadBackground(pRenderer, "resources/bakgrund.png");
    if (!pBackground) return 1;

<<<<<<< HEAD
=======
    Snake *pSnake = createSnake(WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2, pRenderer, WINDOW_WIDTH, WINDOW_HEIGHT);
    Snake *snake[4];
    snake[0] = createSnake(WINDOW_WIDTH / 2, 0, pRenderer, WINDOW_WIDTH, WINDOW_HEIGHT);             // Topp mitten
    snake[1] = createSnake(WINDOW_WIDTH / 2, WINDOW_HEIGHT, pRenderer, WINDOW_WIDTH, WINDOW_HEIGHT); // Botten mitten
    snake[2] = createSnake(0, WINDOW_HEIGHT / 2, pRenderer, WINDOW_WIDTH, WINDOW_HEIGHT);            // Vänster mitten
    snake[3] = createSnake(WINDOW_WIDTH, WINDOW_HEIGHT / 2, pRenderer, WINDOW_WIDTH, WINDOW_HEIGHT); // Höger mitten


>>>>>>> 23f984c (ip sida)
    gameLoop(snake, pRenderer, pBackground);

    bool isRunning = true;
    SDL_Event event;

    while (isRunning)
    {
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT || (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE))
            {
                isRunning = false;
            }
        }

        updateSnake(pSnake);

        SDL_SetRenderDrawColor(pRenderer, 0, 0, 0, 255);
        SDL_RenderClear(pRenderer);

        SDL_RenderCopy(pRenderer, pBackground, NULL, NULL);

        drawSnake(pSnake);
        SDL_RenderPresent(pRenderer);
        SDL_Delay(16); // ~60 FPS
    }

    // destroySnake(pSnake);
    for (int i = 0; i < 4; i++)
    {
        destroySnake(snake[i]);
    }

    SDL_DestroyRenderer(pRenderer);
    SDL_DestroyWindow(pWindow);
    SDL_DestroyTexture(pBackground);
    IMG_Quit();
    SDL_Quit();

    return 0;
}
=======
#include <SDL.h>
#include <SDL_image.h>
#include <stdbool.h>
#include "snake.h"
#include "bakgrund.h"
#include "meny.h"

const int WINDOW_WIDTH =800;
const int WINDOW_HEIGHT = 700;

int main(int argc, char *argv[])
{
    SDL_Init(SDL_INIT_VIDEO);
    IMG_Init(IMG_INIT_PNG);

    SDL_Window *pWindow = SDL_CreateWindow("Snake Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, 0);
    SDL_Renderer *pRenderer = SDL_CreateRenderer(pWindow, -1, SDL_RENDERER_ACCELERATED);

 // Visa startmeny först
    if (!visaStartMeny(pRenderer)) 
    {
        SDL_DestroyRenderer(pRenderer);
        SDL_DestroyWindow(pWindow);
        IMG_Quit();
        SDL_Quit();
        return 1;
    }
    if (!visaIPMeny(pRenderer))    
    {
        SDL_DestroyRenderer(pRenderer);
        SDL_DestroyWindow(pWindow);
        IMG_Quit();
        SDL_Quit();
        return 1;
    }

// Starta spelet efter IP-inmatning
    SDL_Texture* pBackground = loadBackground(pRenderer, "resources/bakgrund.png");
    if (!pBackground) return 1;

    Snake *pSnake = createSnake(WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2, pRenderer, WINDOW_WIDTH, WINDOW_HEIGHT);
    Snake *snake[4];
    snake[0] = createSnake(WINDOW_WIDTH / 2, 0, pRenderer, WINDOW_WIDTH, WINDOW_HEIGHT);             // Topp mitten
    snake[1] = createSnake(WINDOW_WIDTH / 2, WINDOW_HEIGHT, pRenderer, WINDOW_WIDTH, WINDOW_HEIGHT); // Botten mitten
    snake[2] = createSnake(0, WINDOW_HEIGHT / 2, pRenderer, WINDOW_WIDTH, WINDOW_HEIGHT);            // Vänster mitten
    snake[3] = createSnake(WINDOW_WIDTH, WINDOW_HEIGHT / 2, pRenderer, WINDOW_WIDTH, WINDOW_HEIGHT); // Höger mitten


    gameLoop(snake, pRenderer, pBackground);

    bool isRunning = true;
    SDL_Event event;

    while (isRunning)
    {
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT || (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE))
            {
                isRunning = false;
            }
        }

        updateSnake(pSnake);

        SDL_SetRenderDrawColor(pRenderer, 0, 0, 0, 255);
        SDL_RenderClear(pRenderer);

        SDL_RenderCopy(pRenderer, pBackground, NULL, NULL);

        drawSnake(pSnake);
        SDL_RenderPresent(pRenderer);
        SDL_Delay(16); // ~60 FPS
    }

    destroySnake(pSnake);
    SDL_DestroyRenderer(pRenderer);
    SDL_DestroyWindow(pWindow);
    SDL_DestroyTexture(pBackground);
    IMG_Quit();
    SDL_Quit();

    return 0;
}
>>>>>>> 23f984cbbcdc8c92859804c4764f7d98b8860537
