#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>  // För text
#include <stdbool.h>
#include <stdio.h>

#include "snake.h"
#include "bakgrund.h"

const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;

int main(int argc, char *argv[])
{
    SDL_Init(SDL_INIT_VIDEO);
    IMG_Init(IMG_INIT_PNG);
    TTF_Init(); 

    SDL_Window *pWindow = SDL_CreateWindow("Snake Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, 0);
    SDL_Renderer *pRenderer = SDL_CreateRenderer(pWindow, -1, SDL_RENDERER_ACCELERATED);

    SDL_Texture* pBackground = loadBackground(pRenderer, "resources/bakgrund.png");
    if (!pBackground) return 1;

    Snake *pSnake = createSnake(WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2, pRenderer);

   
    Uint64 startTime = SDL_GetTicks64();  
    int gameTime = -1;                    
    TTF_Font* font = TTF_OpenFont("GamjaFlower-Regular.ttf", 24); // EGEN FONT här!
    if (!font)
    {
        printf("Error loading font: %s\n", TTF_GetError());
        return 1;
    }

    SDL_Color textColor = {255, 255, 255, 255}; 
    SDL_Texture* pTimerTexture = NULL;           
    SDL_Rect timerRect;                          
 

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

        updateSnake(pSnake);

       
        int currentTime = (SDL_GetTicks64() - startTime) / 1000;
        if (currentTime > gameTime)
        {
            gameTime = currentTime;
            if (pTimerTexture) SDL_DestroyTexture(pTimerTexture); 

            char timerText[32];
            int minutes = gameTime / 60;
            int seconds = gameTime % 60;
            sprintf(timerText, "%02d:%02d", minutes, seconds);

            SDL_Surface* pSurface = TTF_RenderText_Solid(font, timerText, textColor);
            pTimerTexture = SDL_CreateTextureFromSurface(pRenderer, pSurface);

            timerRect.x = 10;
            timerRect.y = 10;
            timerRect.w = pSurface->w;
            timerRect.h = pSurface->h;

            SDL_FreeSurface(pSurface);
        }

        SDL_SetRenderDrawColor(pRenderer, 0, 0, 0, 255);
        SDL_RenderClear(pRenderer);

        SDL_RenderCopy(pRenderer, pBackground, NULL, NULL);
        drawSnake(pSnake);

        if (pTimerTexture) SDL_RenderCopy(pRenderer, pTimerTexture, NULL, &timerRect); 

        SDL_RenderPresent(pRenderer);
        SDL_Delay(16);
    }


    if (pTimerTexture) SDL_DestroyTexture(pTimerTexture);
    TTF_CloseFont(font);
    TTF_Quit();
    destroySnake(pSnake);
    SDL_DestroyRenderer(pRenderer);
    SDL_DestroyWindow(pWindow);
    SDL_DestroyTexture(pBackground);
    IMG_Quit();
    SDL_Quit();

    return 0;
}
