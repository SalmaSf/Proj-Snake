#include <SDL.h>
#include <SDL_image.h>
#include <math.h>
#include <stdlib.h>
#include "snake.h"
#include <stdbool.h>
#include <SDL_ttf.h> 

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
    SDL_Texture *pSegmentTexture;
    SDL_Rect headRect;
    float speed;
    int window_width;  
    int window_height; 
    float headRectAngle;

    float historyX[MAX_HISTORY];
    float historyY[MAX_HISTORY];
    int historyIndex;
    Uint32 lastSegmentTime;
    bool isAlive;
};

Snake *createSnake(int x, int y, SDL_Renderer *pRenderer, int window_width, int window_height, const char *headTexturePath, const char *segmentTexturePath)
{
    Snake *pSnake = malloc(sizeof(Snake));
    pSnake->head = malloc(sizeof(Segment));
    pSnake->head->x = x;
    pSnake->head->y = y;
    pSnake->head->next = NULL;
    pSnake->pRenderer = pRenderer;
    pSnake->speed = 3.0f;

    pSnake->historyIndex = 0;

    pSnake->isAlive = true;

    SDL_Surface *pSurface = IMG_Load(headTexturePath);
    if (!pSurface)
    {
        printf("Image Load Error (head): %s\n", SDL_GetError());
        free(pSnake->head);
        free(pSnake);
        return NULL;
    }
    pSnake->pTexture = SDL_CreateTextureFromSurface(pRenderer, pSurface);
    SDL_FreeSurface(pSurface);
    SDL_QueryTexture(pSnake->pTexture, NULL, NULL, &pSnake->headRect.w, &pSnake->headRect.h);
    
    pSnake->headRect.w = 45; // Sätt en fast bredd för huvudet
    pSnake->headRect.h = 45; // Sätt en fast höjd för huvudet

    pSnake->window_width = window_width;
    pSnake->window_height = window_height;

    SDL_Surface *pSegmentSurface = IMG_Load(segmentTexturePath);
    if (!pSegmentSurface)
    {
        printf("Image Load Error (segment): %s\n", SDL_GetError());
        SDL_DestroyTexture(pSnake->pTexture);
        free(pSnake->head);
        free(pSnake);
        return NULL;
    }
    pSnake->pSegmentTexture = SDL_CreateTextureFromSurface(pRenderer, pSegmentSurface);
    SDL_FreeSurface(pSegmentSurface);

    pSnake->window_width = window_width;
    pSnake->window_height = window_height;

    return pSnake;
}

void addSegment(Snake *pSnake)
{
    Segment *newSegment = malloc(sizeof(Segment));
    Segment *tail = pSnake->head;

    while (tail->next != NULL)
        tail = tail->next;

    // Placera det nya segmentet precis där sista segmentet är
    newSegment->x = tail->x;
    newSegment->y = tail->y;
    newSegment->next = NULL;
    tail->next = newSegment;
}

void updateSegments(Snake *pSnake)
{
    Segment *current = pSnake->head->next;
    int segmentIndex = 1;

    while (current)
    {
        int delay = segmentIndex * 3;
        int index = (pSnake->historyIndex - delay + MAX_HISTORY) % MAX_HISTORY;
        current->x = pSnake->historyX[index];
        current->y = pSnake->historyY[index];

        current = current->next;
        segmentIndex++;
    }
}


void updateSnake(Snake *pSnake)
{
    int mouseX, mouseY;
    SDL_GetMouseState(&mouseX, &mouseY);

    static float lastAngle = 0.0f;
    float dx = mouseX - pSnake->head->x;
    float dy = mouseY - pSnake->head->y;

    pSnake->headRectAngle = atan2f(dy, dx) * 180.0f / M_PI;

    if (mouseX >= 0 && mouseX <= pSnake->window_width &&
        mouseY >= 0 && mouseY <= pSnake->window_height) 
    {
        lastAngle = atan2f(dy, dx);
    }

    float distance = sqrtf(dx * dx + dy * dy);

    float moveX = cosf(lastAngle) * pSnake->speed;
    float moveY = sinf(lastAngle) * pSnake->speed;

    // 1. Flytta huvudet
    if (distance > pSnake->speed)
    {
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
    //WRAP huvudets position
    if (pSnake->head->x < 0)
        pSnake->head->x += pSnake->window_width;
    else if (pSnake->head->x >= pSnake->window_width)
        pSnake->head->x -= pSnake->window_width;

    if (pSnake->head->y < 0)
        pSnake->head->y += pSnake->window_height;
    else if (pSnake->head->y >= pSnake->window_height)
        pSnake->head->y -= pSnake->window_height;

    // 3. Uppdatera segmenten
    updateSegments(pSnake);

    // 4. Spara huvudets position i historik
    pSnake->historyX[pSnake->historyIndex] = pSnake->head->x;
    pSnake->historyY[pSnake->historyIndex] = pSnake->head->y;
    pSnake->historyIndex = (pSnake->historyIndex + 1) % MAX_HISTORY;

    // 5. Lägg till nytt segment om det är dags
    Uint32 now = SDL_GetTicks();
    if (now - pSnake->lastSegmentTime >= 2000)
    {
        addSegment(pSnake);
        pSnake->lastSegmentTime = now;
    }
}

bool checkCollision(Snake *attacker, Snake *target)
{
    if (!attacker->isAlive || !target->isAlive)
        return false; // Om nån är död, hoppa över 

    Segment *current = target->head;

    while (current)
    {
        float dx = attacker->head->x - current->x;
        float dy = attacker->head->y - current->y;
        float distance = sqrtf(dx * dx + dy * dy);

        if (distance < 10.0f) // Mindre än 10 pixlar => träff
        {
            return true;
        }

        current = current->next;
    }

    return false;
}
int getSnakeHeadX(Snake *snake) {
    return (int)(snake->head->x);
}

int getSnakeHeadY(Snake *snake) {
    return (int)(snake->head->y);
}


bool isSnakeAlive(Snake *snake)
{
    return snake->isAlive;
}

void killSnake(Snake *snake)
{
    snake->isAlive = false;
}

void setSnakePosition(Snake *snake, int x, int y) {
    if (snake && snake->head) {
        snake->head->x = x;
        snake->head->y = y;
    }
}

void drawSnake(Snake *pSnake)
{
    // Rita segment
    Segment *seg = pSnake->head->next;
    SDL_Rect rect;
    rect.w = pSnake->headRect.w;
    rect.h = pSnake->headRect.h;
    while (seg) //????
    {
        rect.x = (int)(seg->x - rect.w / 2);
        rect.y = (int)(seg->y - rect.h / 2);
        SDL_RenderCopy(pSnake->pRenderer, pSnake->pSegmentTexture, NULL, &rect);
        seg = seg->next;
    }

    // Placera så att bildens topp (huvudet) är vid ormens position
    pSnake->headRect.x = (int)(pSnake->head->x - pSnake->headRect.w / 2);
    pSnake->headRect.y = (int)(pSnake->head->y - pSnake->headRect.h / 2);


    // Rotera runt huvudets position (övre mittpunkt)
    SDL_Point center = {
        pSnake->headRect.w / 2,
        pSnake->headRect.h / 2
    };
    

    SDL_RenderCopyEx(
        pSnake->pRenderer,
        pSnake->pTexture,
        NULL,
        &pSnake->headRect,
        pSnake->headRectAngle + 90,
        &center,
        SDL_FLIP_NONE);
}

void destroySnake(Snake *pSnake)
{
    Segment *seg = pSnake->head;
    while (seg)
    {
        Segment *next = seg->next;
        free(seg); 
        seg = next;
    }
    if (pSnake->pTexture)
        SDL_DestroyTexture(pSnake->pTexture);
    if (pSnake->pSegmentTexture)
        SDL_DestroyTexture(pSnake->pSegmentTexture);
    free(pSnake);
}