#include <SDL.h>
#include <SDL_image.h>
#include <math.h>
#include <stdlib.h>
#include "snake.h"

struct segment
{
    float x, y;
    struct segment *next;
};

struct snake
{
    Segment *head;
    SDL_Renderer *pRenderer;
    SDL_Texture *pTexture;
    SDL_Rect headRect;
    float speed;
    int window_width;  // New
    int window_height; // New
};

Snake *createSnake(int x, int y, SDL_Renderer *pRenderer, int window_width, int window_height)
{
    Snake *pSnake = malloc(sizeof(Snake));
    pSnake->head = malloc(sizeof(Segment));
    pSnake->head->x = x;
    pSnake->head->y = y;
    pSnake->head->next = NULL;
    pSnake->pRenderer = pRenderer;
    pSnake->speed = 3.0f;

    SDL_Surface *pSurface = IMG_Load("resources/snake_head.png");
    if (!pSurface)
    {
        printf("Image Load Error: %s\n", SDL_GetError());
        return NULL;
    }
    pSnake->pTexture = SDL_CreateTextureFromSurface(pRenderer, pSurface);
    SDL_FreeSurface(pSurface);
    SDL_QueryTexture(pSnake->pTexture, NULL, NULL, &pSnake->headRect.w, &pSnake->headRect.h);
    pSnake->headRect.w /= 7;
    pSnake->headRect.h /= 7;

    pSnake->window_width = window_width;
    pSnake->window_height = window_height;

    return pSnake;
}

void updateSnake(Snake *pSnake)
{
    int mouseX, mouseY;
    SDL_GetMouseState(&mouseX, &mouseY);

    static float lastAngle = 0.0f;
    float dx = mouseX - pSnake->head->x;
    float dy = mouseY - pSnake->head->y;

    if (mouseX >= 0 && mouseX <= pSnake->window_width &&
        mouseY >= 0 && mouseY <= pSnake->window_height)
    {
        lastAngle = atan2f(dy, dx);
    }

    float distance = sqrtf(dx * dx + dy * dy);

    float moveX = cosf(lastAngle) * pSnake->speed;
    float moveY = sinf(lastAngle) * pSnake->speed;

    if (distance > pSnake->speed)
    {
        float moveX = cosf(lastAngle) * pSnake->speed;
        float moveY = sinf(lastAngle) * pSnake->speed;

        pSnake->head->x += moveX;
        pSnake->head->y += moveY;
    }
    else
    {
        pSnake->head->x = mouseX;
        pSnake->head->y = mouseY;
    }

    pSnake->head->x += moveX;
    pSnake->head->y += moveY;

    // Wrap-around
    if (pSnake->head->x < 0)
        pSnake->head->x += pSnake->window_width;
    else if (pSnake->head->x > pSnake->window_width)
        pSnake->head->x -= pSnake->window_width;

    if (pSnake->head->y < 0)
        pSnake->head->y += pSnake->window_height;
    else if (pSnake->head->y > pSnake->window_height)
        pSnake->head->y -= pSnake->window_height;
}

void drawSnake(Snake *pSnake)
{
    pSnake->headRect.x = (int)pSnake->head->x - pSnake->headRect.w / 2;
    pSnake->headRect.y = (int)pSnake->head->y - pSnake->headRect.h / 2;
    SDL_RenderCopy(pSnake->pRenderer, pSnake->pTexture, NULL, &pSnake->headRect);
}

void destroySnake(Snake *pSnake)
{
    if (pSnake->head)
    {
        free(pSnake->head);
    }
    if (pSnake->pTexture)
    {
        SDL_DestroyTexture(pSnake->pTexture);
    }
    free(pSnake);
}