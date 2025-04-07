#include <stdio.h>
#include <SDL.h>
#include "rocket.h"

#define WINDOW_WIDTH 1280
#define WINDOW_HEIGHT 960

struct game{
    SDL_Window *pWindow;
    SDL_Renderer *pRenderer;
    Rocket *pRocket;
};
typedef struct game Game;

int initiate(Game *pGame);
void run(Game *pGame);
void close(Game *pGame);
void handleInput(Game *pGame,SDL_Event *pEvent);

int main(int argv, char** args){
    Game g={0};
    if(!initiate(&g)) return 1;
    run(&g);
    close(&g);

    return 0;
}

int initiate(Game *pGame){
    if(SDL_Init(SDL_INIT_VIDEO|SDL_INIT_TIMER)!=0){
        printf("Error: %s\n",SDL_GetError());
        return 0;
    }
    pGame->pWindow = SDL_CreateWindow("Rocket Game",SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED,WINDOW_WIDTH,WINDOW_HEIGHT,0);
    if(!pGame->pWindow){
        printf("Error: %s\n",SDL_GetError());
        close(pGame);
        return 0;
    }
    pGame->pRenderer = SDL_CreateRenderer(pGame->pWindow, -1, SDL_RENDERER_ACCELERATED|SDL_RENDERER_PRESENTVSYNC);
    if(!pGame->pRenderer){
        printf("Error: %s\n",SDL_GetError());
        close(pGame);
        return 0;    
    }

    pGame->pRocket = createRocket(WINDOW_WIDTH/2,WINDOW_HEIGHT/2,pGame->pRenderer,WINDOW_WIDTH,WINDOW_HEIGHT);

    if(!pGame->pRocket){
        printf("Error: %s\n",SDL_GetError());
        close(pGame);
        return 0;
    }
    
    return 1;
}

void run(Game *pGame){
    int close_requested = 0;
    SDL_Event event;
    while(!close_requested){
        while(SDL_PollEvent(&event)){
            if(event.type==SDL_QUIT) close_requested = 1;
            else handleInput(pGame,&event);
        }
        updateRocket(pGame->pRocket);

        SDL_SetRenderDrawColor(pGame->pRenderer,0,0,0,255);
        SDL_RenderClear(pGame->pRenderer);
        SDL_SetRenderDrawColor(pGame->pRenderer,230,230,230,255);
        drawRocket(pGame->pRocket);
        SDL_RenderPresent(pGame->pRenderer);
        SDL_Delay(1000/60-15);//Vanessa testar
    }
}

void handleInput(Game *pGame,SDL_Event *pEvent){
    if(pEvent->type == SDL_KEYDOWN){
        switch(pEvent->key.keysym.scancode){
            case SDL_SCANCODE_W:
            case SDL_SCANCODE_UP:
                accelerate(pGame->pRocket);
                break;
            case SDL_SCANCODE_A:
            case SDL_SCANCODE_LEFT:
                turnLeft(pGame->pRocket);
            break;
            case SDL_SCANCODE_D:
            case SDL_SCANCODE_RIGHT:
                turnRight(pGame->pRocket);
            break;
        }
    }
}

void close(Game *pGame){
    if(pGame->pRocket) destroyRocket(pGame->pRocket);
    if(pGame->pRenderer) SDL_DestroyRenderer(pGame->pRenderer);
    if(pGame->pWindow) SDL_DestroyWindow(pGame->pWindow);
    SDL_Quit();
}
