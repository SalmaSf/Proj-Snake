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
    float netX, netY;
};

Snake *createSnake(int x, int y, SDL_Renderer *pRenderer, int window_width, int window_height, const char *headTexturePath, const char *segmentTexturePath)
{
    Snake *pSnake = malloc(sizeof(Snake));
    if (!pSnake)
        return NULL;

    pSnake->head = malloc(sizeof(Segment));

    pSnake->head->x = x;
    pSnake->head->y = y;
    pSnake->head->next = NULL;

    pSnake->pRenderer = pRenderer;
    pSnake->speed = 3.0f;

    pSnake->historyIndex = 0;
    pSnake->isAlive = true;

    pSnake->netX = (float)x;
    pSnake->netY = (float)y;

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

    pSnake->headRect.w = 45;
    pSnake->headRect.h = 45;

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

void updateSnake(Snake *s, bool isLocalPlayer, int targetX, int targetY)
{
    int mouseX, mouseY;

    mouseX = targetX;
    mouseY = targetY;

    static float lastAngle = 0.0f;
    float dx = mouseX - s->head->x;
    float dy = mouseY - s->head->y;

    s->headRectAngle = atan2f(dy, dx) * 180.0f / M_PI;
    if (mouseX >= 0 && mouseX <= s->window_width &&
        mouseY >= 0 && mouseY <= s->window_height)
    {
        lastAngle = atan2f(dy, dx);
    }

    float distance = sqrtf(dx * dx + dy * dy);
    float moveX = cosf(lastAngle) * s->speed;
    float moveY = sinf(lastAngle) * s->speed;

    if (distance > s->speed)
    {
        s->head->x += moveX;
        s->head->y += moveY;
    }
    else
    {
        s->head->x = mouseX;
        s->head->y = mouseY;
    }
    s->head->x += moveX;
    s->head->y += moveY;

    if (s->head->x < 0)
        s->head->x += s->window_width;
    else if (s->head->x >= s->window_width)
        s->head->x -= s->window_width;

    if (s->head->y < 0)
        s->head->y += s->window_height;
    else if (s->head->y >= s->window_height)
        s->head->y -= s->window_height;

    updateSegments(s);

    s->historyX[s->historyIndex] = s->head->x;
    s->historyY[s->historyIndex] = s->head->y;
    s->historyIndex = (s->historyIndex + 1) % MAX_HISTORY;

    Uint32 now = SDL_GetTicks();
    if (now - s->lastSegmentTime >= 2000)
    {
        addSegment(s);
        s->lastSegmentTime = now;
    }
}

bool checkCollision(Snake *attacker, Snake *target)
{
    if (!attacker->isAlive || !target->isAlive)
        return false;

    Segment *current = target->head;

    while (current)
    {
        float dx = attacker->head->x - current->x;
        float dy = attacker->head->y - current->y;
        float distance = sqrtf(dx * dx + dy * dy);

        if (distance < 10.0f)
        {
            return true;
        }

        current = current->next;
    }

    return false;
}
int getSnakeHeadX(Snake *snake)
{
    return (int)(snake->head->x);
}

int getSnakeHeadY(Snake *snake)
{
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

void setSnakePosition(Snake *snake, int x, int y)
{
    if (snake && snake->head)
    {
        snake->head->x = x;
        snake->head->y = y;
    }
}

void drawSnake(Snake *pSnake)
{

    Segment *seg = pSnake->head->next;
    SDL_Rect rect;
    rect.w = pSnake->headRect.w;
    rect.h = pSnake->headRect.h;
    while (seg)
    {
        rect.x = (int)(seg->x - rect.w / 2);
        rect.y = (int)(seg->y - rect.h / 2);
        SDL_RenderCopy(pSnake->pRenderer, pSnake->pSegmentTexture, NULL, &rect);
        seg = seg->next;
    }

    pSnake->headRect.x = (int)(pSnake->head->x - pSnake->headRect.w / 2);
    pSnake->headRect.y = (int)(pSnake->head->y - pSnake->headRect.h / 2);

    SDL_Point center = {
        pSnake->headRect.w / 2,
        pSnake->headRect.h / 2};

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
