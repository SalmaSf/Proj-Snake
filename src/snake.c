#include <SDL.h>
#include <SDL_image.h>
#include <math.h>
#include <stdlib.h>
#include "snake.h"

struct segment {
    float x, y;
    struct segment *next;
};

struct snake {
    Segment *head;
    SDL_Renderer *pRenderer;
    SDL_Texture *pTexture;
    SDL_Rect headRect;
    float speed;
};

Snake *createSnake(int x, int y, SDL_Renderer *pRenderer) {
    Snake *pSnake = malloc(sizeof(Snake));
    pSnake->head = malloc(sizeof(Segment));
    pSnake->head->x = x;
    pSnake->head->y = y;
    pSnake->head->next = NULL;
    pSnake->pRenderer = pRenderer;
    pSnake->speed = 3.0f;

    SDL_Surface *pSurface = IMG_Load("resources/snake_head.png");
    if (!pSurface) {
        printf("Image Load Error: %s\n", SDL_GetError());
        return NULL;
    }
    pSnake->pTexture = SDL_CreateTextureFromSurface(pRenderer, pSurface);
    SDL_FreeSurface(pSurface);
    SDL_QueryTexture(pSnake->pTexture, NULL, NULL, &pSnake->headRect.w, &pSnake->headRect.h);
    pSnake->headRect.w /= 4;
    pSnake->headRect.h /= 4;

    return pSnake;
}

void updateSnake(Snake *pSnake, int windowWidth, int windowHeight) {
    int mouseX, mouseY;
    SDL_GetMouseState(&mouseX, &mouseY);

    float dx = mouseX - pSnake->head->x;
    float dy = mouseY - pSnake->head->y;
    float angle = atan2f(dy, dx);

    pSnake->head->x += cosf(angle) * pSnake->speed;
    pSnake->head->y += sinf(angle) * pSnake->speed;

    // Wrap-around logik
    if (pSnake->head->x < 0) {
        pSnake->head->x = windowWidth - 1;
    } else if (pSnake->head->x >= windowWidth) {
        pSnake->head->x = 0;
    }
    
    if (pSnake->head->y < 0) {
        pSnake->head->y = windowHeight - 1;
    } else if (pSnake->head->y >= windowHeight) {
        pSnake->head->y = 0;
    }
}

void drawSnake(Snake *pSnake) {
    pSnake->headRect.x = (int)pSnake->head->x - pSnake->headRect.w / 2;
    pSnake->headRect.y = (int)pSnake->head->y - pSnake->headRect.h / 2;
    SDL_RenderCopy(pSnake->pRenderer, pSnake->pTexture, NULL, &pSnake->headRect);
}

void destroySnake(Snake *pSnake) {
    if (pSnake->head) {
        free(pSnake->head);
    }
    if (pSnake->pTexture) {
        SDL_DestroyTexture(pSnake->pTexture);
    }
    free(pSnake);
}
