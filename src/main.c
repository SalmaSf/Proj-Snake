#include <SDL.h>
 #include <SDL_image.h>
 #include <stdbool.h>
 #include "snake.h"
 
 const int WINDOW_WIDTH = 800;
 const int WINDOW_HEIGHT = 600;
 
 int main(int argc, char *argv[]) {
 int main(int argc, char *argv[]) 
 {
     SDL_Init(SDL_INIT_VIDEO);
     IMG_Init(IMG_INIT_PNG);
 
     SDL_Window *pWindow = SDL_CreateWindow("Snake Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, 0);
     SDL_Renderer *pRenderer = SDL_CreateRenderer(pWindow, -1, SDL_RENDERER_ACCELERATED);
 
     Snake *pSnake = createSnake(WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2, pRenderer);
 
     bool isRunning = true;
     SDL_Event event;
 
     while (isRunning) {
         while (SDL_PollEvent(&event)) {
             if (event.type == SDL_QUIT) {
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
                 if(event.key.keysym.sym == SDLK_ESCAPE)
                 {
                     isRunning = false;
                 }
             }
         }
 
         updateSnake(pSnake);
 
         SDL_SetRenderDrawColor(pRenderer, 0, 0, 0, 255);
         SDL_RenderClear(pRenderer);
 
         drawSnake(pSnake);
 
         SDL_RenderPresent(pRenderer);
         SDL_Delay(16); // ~60 FPS
     }
 
     destroySnake(pSnake);
     SDL_DestroyRenderer(pRenderer);
     SDL_DestroyWindow(pWindow);
     IMG_Quit();
     SDL_Quit();
 
     return 0;
 }