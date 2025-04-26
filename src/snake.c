#include <SDL.h>
#include <SDL_image.h>
#include <math.h>
#include <stdlib.h>
#include "snake.h"

/*float historyX[MAX_HISTORY];
float historyY[MAX_HISTORY];
int historyIndex = 0;*/

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
    int window_width;  // New
    int window_height; // New
    float headRectAngle;

    float historyX[MAX_HISTORY];
    float historyY[MAX_HISTORY];
    int historyIndex;
    Uint32 lastSegmentTime;
};

// Uint32 lastSegmentTime = 0;

Snake *createSnake(int x, int y, SDL_Renderer *pRenderer, int window_width, int window_height)
{
    Snake *pSnake = malloc(sizeof(Snake));
    pSnake->head = malloc(sizeof(Segment));
    pSnake->head->x = x;
    pSnake->head->y = y;
    pSnake->head->next = NULL;
    pSnake->pRenderer = pRenderer;
    pSnake->speed = 2.0f;

    pSnake->historyIndex = 0;

    SDL_Surface *pSurface = IMG_Load("resources/snake_head.png");
    if (!pSurface)
    {
        printf("Image Load Error: %s\n", SDL_GetError());
        return NULL;
    }
    pSnake->pTexture = SDL_CreateTextureFromSurface(pRenderer, pSurface);
    SDL_FreeSurface(pSurface);
    SDL_QueryTexture(pSnake->pTexture, NULL, NULL, &pSnake->headRect.w, &pSnake->headRect.h);
    pSnake->headRect.w /= 12;
    pSnake->headRect.h /= 12;

    pSnake->window_width = window_width;
    pSnake->window_height = window_height;

    SDL_Surface *pSegmentSurface = IMG_Load("resources/limeSlice.png");
    if (!pSegmentSurface)
    {
        printf("Segment Image Load Error: %s\n", SDL_GetError());
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
        int delay = segmentIndex * 4;
        /* int index = (historyIndex - delay + MAX_HISTORY) % MAX_HISTORY;
         current->x = historyX[index];
         current->y = historyY[index];*/
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
    float oldX = pSnake->head->x;
    float oldY = pSnake->head->y;

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
    updateSegments(pSnake);

    // Wrap-around
    if (pSnake->head->x < 0)
        pSnake->head->x += pSnake->window_width;
    else if (pSnake->head->x > pSnake->window_width)
        pSnake->head->x -= pSnake->window_width;

    if (pSnake->head->y < 0)
        pSnake->head->y += pSnake->window_height;
    else if (pSnake->head->y > pSnake->window_height)
        pSnake->head->y -= pSnake->window_height;

    // Spara huvudets position i historik
    /*historyX[historyIndex] = pSnake->head->x;
    historyY[historyIndex] = pSnake->head->y;
    historyIndex = (historyIndex + 1) % MAX_HISTORY;*/
    pSnake->historyX[pSnake->historyIndex] = pSnake->head->x;
    pSnake->historyY[pSnake->historyIndex] = pSnake->head->y;
    pSnake->historyIndex = (pSnake->historyIndex + 1) % MAX_HISTORY;

    Uint32 now = SDL_GetTicks();
    if (now - pSnake->lastSegmentTime >= 3000)
    {
        addSegment(pSnake);
        pSnake->lastSegmentTime = now;
    }
}

void drawSnake(Snake *pSnake)
{
    // Rita segment
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

    // Placera så att bildens topp (huvudet) är vid ormens position
    pSnake->headRect.x = (int)(pSnake->head->x - pSnake->headRect.w / 2);
    pSnake->headRect.y = (int)(pSnake->head->y);

    // Rotera runt huvudets position (övre mittpunkt)
    SDL_Point center = {
        pSnake->headRect.w / 2, // mitten av bredden
        pSnake->headRect.h      // toppen av bilden
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