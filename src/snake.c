
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>

#include <SDL_net.h>
#include <math.h>
#include <stdlib.h>
#include "snake.h"
#include <stdbool.h>

#include "snake_data.h"
#define MAX_PLAYERS 4
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
    bool isAlive;
    int colorId; // NEW
    int playerIndex;
    int segmentCount;
    SDL_Rect segments[MAX_SEGMENTS];
    int direction;
};

// Uint32 lastSegmentTime = 0;

Snake *createSnake(int x, int y, SDL_Renderer *pRenderer, int window_width, int window_height, const char *headTexturePath, const char *segmentTexturePath, int colorId)
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
    pSnake->colorId = colorId; // orm färg salma

    /*SDL_Surface *pSurface = IMG_Load("resources/snake_head.png");
    if (!pSurface)
    {
        printf("Image Load Error: %s\n", SDL_GetError());
        return NULL;
    }*/
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
    // pSnake->headRect.w /= 12;
    // pSnake->headRect.h /= 12;
    pSnake->headRect.w = 45; // Sätt en fast bredd för huvudet
    pSnake->headRect.h = 45; // Sätt en fast höjd för huvudet

    pSnake->window_width = window_width;
    pSnake->window_height = window_height;

    /*SDL_Surface *pSegmentSurface = IMG_Load("resources/limeSlice.png");
     if (!pSegmentSurface)
     {
         printf("Segment Image Load Error: %s\n", SDL_GetError());
         return NULL;
     }*/
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

    /*pSnake->window_width = window_width; // varför finns denna del två gånger i koden?
    pSnake->window_height = window_height;*/

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
        int delay = segmentIndex * 5;
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
    // pSnake->head->x += moveX;
    // pSnake->head->y += moveY;
    // 2. WRAP huvudets position
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

bool isSnakeAlive(Snake *snake)
{
    return snake->isAlive;
}

void killSnake(Snake *snake)
{
    snake->isAlive = false;
}
void gameLoop(Snake *snake[], SDL_Renderer *pRenderer, SDL_Texture *pBackground, int localPlayerIndex)
{
    bool isRunning = true;
    SDL_Event event;

    // 🚀 Timer-setup direkt i spelet
    Uint64 startTime = SDL_GetTicks64();
    int gameTime = -1;

    TTF_Font *font = TTF_OpenFont("GamjaFlower-Regular.ttf", 24);
    if (!font)
    {
        printf("Error loading font: %s\n", TTF_GetError());
        return;
    }

    SDL_Color textColor = {255, 255, 255, 255};
    SDL_Texture *pTimerTexture = NULL;
    SDL_Rect timerRect;

    while (isRunning)
    {
        // Eventhantering
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT ||
                (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE))
            {
                isRunning = false;
            }
        }

        // Kollisioner
        for (int i = 0; i < MAX_PLAYERS; i++)
        {
            for (int j = 0; j < MAX_PLAYERS; j++)
            {
                if (i == j)
                    continue;

                if (checkCollision(snake[i], snake[j]))
                {

                    printf("Orm %d dödade orm %d!\n", i + 1, j + 1);
                    killSnake(snake[j]);
                }
            }
        }

        // Kolla hur många ormar som lever
        int aliveCount = 0;
        int lastAliveIndex = -1;
        for (int i = 0; i < 4; i++)
        {
            if (isSnakeAlive(snake[i]))
            {
                aliveCount++;
                lastAliveIndex = i;
            }
        }

        if (aliveCount == 1)
        {
            printf("Orm %d är den sista som lever!\n", lastAliveIndex + 1);
            isRunning = false;
        }
        else if (aliveCount == 0)
        {
            printf("Alla ormar är döda!\n");
            isRunning = false;
        }

        // Uppdatera alla levande ormar -ändringar för att istället uppdatera en orm istället för alla -salma
        for (int i = 0; i < 4; i++)
        {
            if (isSnakeAlive(snake[i]))
            {
                if (i == localPlayerIndex)
                { // 👈 Update only local player's snake
                    updateSnake(snake[i]);
                }
            }
        }

        int currentTime = (SDL_GetTicks64() - startTime) / 1000;
        if (currentTime > gameTime)
        {
            gameTime = currentTime;

            if (pTimerTexture)
                SDL_DestroyTexture(pTimerTexture);

            char timerText[32];
            int minutes = gameTime / 60;
            int seconds = gameTime % 60;
            sprintf(timerText, "%02d:%02d", minutes, seconds);

            SDL_Surface *pSurface = TTF_RenderText_Solid(font, timerText, textColor);
            pTimerTexture = SDL_CreateTextureFromSurface(pRenderer, pSurface);

            timerRect.x = 10;
            timerRect.y = 10;
            timerRect.w = pSurface->w;
            timerRect.h = pSurface->h;

            SDL_FreeSurface(pSurface);
        } //

        // Rita allt
        SDL_SetRenderDrawColor(pRenderer, 0, 0, 0, 255);
        SDL_RenderClear(pRenderer);

        SDL_RenderCopy(pRenderer, pBackground, NULL, NULL);

        for (int i = 0; i < MAX_PLAYERS; i++)
        {
            if (isSnakeAlive(snake[i]))
            {
                drawSnake(snake[i]);
            }
        }
        // 🚀 Rita timern
        if (pTimerTexture)
        {
            SDL_RenderCopy(pRenderer, pTimerTexture, NULL, &timerRect);
        }

        SDL_RenderPresent(pRenderer);
        SDL_Delay(16); // ~60 FPS
    }
    // 🚀 Städning efter spelet är slut
    if (pTimerTexture)
        SDL_DestroyTexture(pTimerTexture);
    TTF_CloseFont(font);
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
    pSnake->headRect.y = (int)(pSnake->head->y - pSnake->headRect.h / 2);

    // Rotera runt huvudets position (övre mittpunkt)
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

void snakeToSerializable(Snake *pSnake, SerializableSnake *out) // salma
{
    Segment *seg = pSnake->head;
    int count = 0;

    while (seg && count < MAX_SEGMENTS)
    {
        out->segments[count].x = (int)seg->x;
        out->segments[count].y = (int)seg->y;
        count++;
        seg = seg->next;
    }

    out->segmentCount = count;
    out->direction = (int)pSnake->headRectAngle;
    out->colorId = pSnake->colorId;
}

void serializableToSnake(SerializableSnake *in, Snake *pSnake) // salma
{
    // Free old segment list only
    Segment *seg = pSnake->head;
    while (seg)
    {
        Segment *next = seg->next; // ska du kalla på draesnake härinne också?
        free(seg);
        seg = next;
    }

    Segment *head = malloc(sizeof(Segment));
    head->x = (float)in->segments[0].x;
    head->y = (float)in->segments[0].y;
    head->next = NULL;

    Segment *current = head;
    for (int i = 1; i < in->segmentCount; i++)
    {
        Segment *newSeg = malloc(sizeof(Segment));
        newSeg->x = (float)in->segments[i].x;
        newSeg->y = (float)in->segments[i].y;
        newSeg->next = NULL;
        current->next = newSeg;
        current = newSeg;
    }

    pSnake->head = head;
    pSnake->segmentCount = in->segmentCount;
    pSnake->direction = in->direction;
    pSnake->playerIndex = in->playerIndex;
    pSnake->colorId = in->colorId; // salma
    pSnake->historyIndex = 0;
    pSnake->lastSegmentTime = SDL_GetTicks();
}

void updateLocalSnakeAndSend(Snake *snakes[], int localPlayerIndex, UDPsocket udpSocket, UDPpacket *sendPacket, IPaddress serverAddress)
{
    if (localPlayerIndex == -1 || !snakes[localPlayerIndex])
        return;

    updateSnake(snakes[localPlayerIndex]); // Update movement, etc.

    ClientData clientData;
    snakeToSerializable(snakes[localPlayerIndex], &clientData); // Fill the struct properly

    clientData.playerIndex = localPlayerIndex; // Optional: server may use this
    clientData.isYou = 1;                      // Let server know this is your own snake

    // DEBUG: Print the data being sent to the server
    printf("Client sending Snake %d: head at (%d, %d), segments: %d, direction: %d, colorId: %d\n",
           clientData.playerIndex,
           clientData.segments[0].x,
           clientData.segments[0].y,
           clientData.segmentCount,
           clientData.direction,
           clientData.colorId);

    memcpy(sendPacket->data, &clientData, sizeof(ClientData));
    sendPacket->len = sizeof(ClientData);
    sendPacket->address = serverAddress;

    SDLNet_UDP_Send(udpSocket, -1, sendPacket);
}
void updateOrCreateSnakeFromSerializable(
    Snake *snakes[],
    SerializableSnake *ss,
    SDL_Renderer *pRenderer,
    int window_width,
    int window_height)
{
    const char *head = "resources/default_head.png";
    const char *body = "resources/default_segment.png";

    switch (ss->colorId)
    {
    case 0:
        head = "resources/purple_head.png";
        body = "resources/purple_body.png";
        break;
    case 1:
        head = "resources/yellow_head.png";
        body = "resources/yellow_body.png";
        break;
    case 2:
        head = "resources/green_head.png";
        body = "resources/green_body.png";
        break;
    case 3:
        head = "resources/pink_head.png";
        body = "resources/pink_body.png";
        break;
    }

    int index = ss->playerIndex;

    if (!snakes[index])
    {
        snakes[index] = createSnake(0, 0, pRenderer, window_width, window_height, head, body, ss->colorId);
    }

    // Actually update the snake's segment data
    serializableToSnake(ss, snakes[index]);
   // printf("Updating snake %d, segments: %d\n", in->playerIndex, in->segmentCount);

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