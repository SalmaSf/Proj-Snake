#include <stdio.h>
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <SDL_net.h>
#include <SDL_mixer.h>
#include <string.h>
#include <stdbool.h>
#include "snake.h"
#include "bakgrund.h"
#include "meny.h"

#define SERVER_IP "130.229.179.210"
#define SERVER_PORT 2000
#define MAX_SNAKES 4

const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 700;

// static UDPsocket udpSocket;
// static IPaddress serverAddr;
// static UDPpacket *packet;

enum gameState
{
    START,
    ONGOING,
    GAME_OVER
};
typedef enum gameState GameState;

typedef struct
{
    bool win;
    float time;
} GameResult;

typedef struct
{
    SDL_Window *pWindow;
    SDL_Renderer *pRenderer;
    SDL_Texture *pBackground;

    Snake *snakes[MAX_SNAKES];
    int playerIndex;

    Mix_Music *music;
    Mix_Chunk *collisionSound;

    TTF_Font *font;

    GameState state;

    // int startTime;
    // int gameTime;

    // Nätverk
    UDPsocket udpSocket;
    IPaddress serverAddr;
    UDPpacket *packet;
} Game;

int initGame(Game *pGame);
void runGame(Game *pGame);
void cleanGame(Game *pGame);

int initSnakeClient(Game *pGame);
void sendSnakePosition(Game *pGame, int x, int y);
void receiveServerUpdate(Game *pGame);
void closeSnakeClient(Game *pGame);

int main(int argc, char *argv[])
{
    Game game;

    if (!initGame(&game))
        return 1;
    runGame(&game);
    cleanGame(&game);

    return 0;
}

int initGame(Game *pGame)
{
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
    TTF_Init();
    IMG_Init(IMG_INIT_PNG);
    SDLNet_Init();
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048);

    pGame->pWindow = SDL_CreateWindow("Snake Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 700, 0);
    pGame->pRenderer = SDL_CreateRenderer(pGame->pWindow, -1, SDL_RENDERER_ACCELERATED);

    pGame->pBackground = loadBackground(pGame->pRenderer, "resources/bakgrund.png");
    pGame->music = Mix_LoadMUS("resources/bakgrund.wav");
    pGame->collisionSound = Mix_LoadWAV("resources/snake_rattle.wav");

    pGame->snakes[0] = createSnake(400, 0, pGame->pRenderer, 800, 700, "resources/purple_head.png", "resources/purple_body.png");
    pGame->snakes[1] = createSnake(400, 700, pGame->pRenderer, 800, 700, "resources/yellow_head.png", "resources/yellow_body.png");
    pGame->snakes[2] = createSnake(0, 350, pGame->pRenderer, 800, 700, "resources/green_head.png", "resources/green_body.png");
    pGame->snakes[3] = createSnake(800, 350, pGame->pRenderer, 800, 700, "resources/pink_head.png", "resources/pink_body.png");

    pGame->playerIndex = 0; // ← ändra beroende på klient

    Mix_PlayMusic(pGame->music, -1);
    pGame->state = START;

    if (!initSnakeClient(pGame))
    {
        SDL_Log("Failed to initialize client networking");
        return 0;
    }

    // initSnakeClient (UDP) kan läggas in här

    return 1;
}

GameResult gameLoop(Snake *snake[], SDL_Renderer *pRenderer, SDL_Texture *pBackground, int spelarIndex, Game *pGame)
{
    bool isRunning = true;
    SDL_Event event;

    // Timer-setup direkt i spelet
    Uint64 startTime = SDL_GetTicks64();
    int gameTime = -1;

    TTF_Font *font = TTF_OpenFont("GamjaFlower-Regular.ttf", 24);
    if (!font)
    {
        printf("Error loading font: %s\n", TTF_GetError());
        return (GameResult){false, 0.0f};
    }

    SDL_Color textColor = {255, 255, 255, 255};
    SDL_Texture *pTimerTexture = NULL;
    SDL_Rect timerRect;

    while (isRunning)
    {

        if (isSnakeAlive(snake[spelarIndex]))
        {
            int headX = getSnakeHeadX(snake[spelarIndex]);
            int headY = getSnakeHeadY(snake[spelarIndex]);

            sendSnakePosition(pGame, headX, headY);
            receiveServerUpdate(pGame); 
        }
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
        for (int i = 0; i < 4; i++)
        {
            for (int j = 0; j < 4; j++)
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

        // Uppdatera alla levande ormar
        for (int i = 0; i < 4; i++)
        {
            if (isSnakeAlive(snake[i]))
            {
                updateSnake(snake[i]);
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

        for (int i = 0; i < 4; i++)
        {
            if (isSnakeAlive(snake[i]))
            {
                drawSnake(snake[i]);
            }
        }
        // Rita timern
        if (pTimerTexture)
        {
            SDL_RenderCopy(pRenderer, pTimerTexture, NULL, &timerRect);
        }
        SDL_RenderPresent(pRenderer);
        SDL_Delay(16); // ~60 FPS
    }
    // Städning efter spelet är slut
    if (pTimerTexture)
        SDL_DestroyTexture(pTimerTexture);
    TTF_CloseFont(font);
    closeSnakeClient(pGame);

    GameResult result;
    result.win = isSnakeAlive(snake[spelarIndex]);
    result.time = (float)gameTime;
    return result;
}

void runGame(Game *pGame)
{
    if (!visaStartMeny(pGame->pRenderer))
        return;
    if (!visaIPMeny(pGame->pRenderer))
        return;
    if (!visaLobby(pGame->pRenderer))
        return;

    GameResult result = gameLoop(pGame->snakes, pGame->pRenderer, pGame->pBackground, pGame->playerIndex,pGame);
    int val = visaResultatskarm(pGame->pRenderer, result.win, result.time);

    if (val == 0)
        visaStartMeny(pGame->pRenderer);
    else
        visaIPMeny(pGame->pRenderer);
}

void cleanGame(Game *pGame)
{
    for (int i = 0; i < MAX_SNAKES; i++)
    {
        destroySnake(pGame->snakes[i]);
    }

    SDL_DestroyTexture(pGame->pBackground);
    SDL_DestroyRenderer(pGame->pRenderer);
    SDL_DestroyWindow(pGame->pWindow);

    Mix_FreeChunk(pGame->collisionSound);
    Mix_FreeMusic(pGame->music);
    Mix_CloseAudio();
    closeSnakeClient(pGame);

    TTF_Quit();
    IMG_Quit();
    SDLNet_Quit();
    SDL_Quit();
}

int initSnakeClient(Game *pGame)
{
    if (SDLNet_Init() < 0)
    {
        SDL_Log("SDLNet_Init: %s\n", SDLNet_GetError());
        return 0;
    }

    pGame->udpSocket = SDLNet_UDP_Open(0);
    if (!pGame->udpSocket)
    {
        SDL_Log("SDLNet_UDP_Open: %s\n", SDLNet_GetError());
        return 0;
    }

    if (SDLNet_ResolveHost(&pGame->serverAddr, SERVER_IP, SERVER_PORT) < 0)
    {
        SDL_Log("SDLNet_ResolveHost: %s\n", SDLNet_GetError());
        return 0;
    }

    pGame->packet = SDLNet_AllocPacket(512);
    if (!pGame->packet)
    {
        SDL_Log("SDLNet_AllocPacket: %s\n", SDLNet_GetError());
        return 0;
    }

    pGame->packet->address.host = pGame->serverAddr.host;
    pGame->packet->address.port = pGame->serverAddr.port;

    return 1;
}

void sendSnakePosition(Game *pGame, int x, int y)
{
    struct
    {
        int x, y;
    } SnakeData = {x, y};

    memcpy(pGame->packet->data, &SnakeData, sizeof(SnakeData));
    pGame->packet->len = sizeof(SnakeData);
    SDLNet_UDP_Send(pGame->udpSocket, -1, pGame->packet);
}

void receiveServerUpdate(Game *pGame)
{
    if (SDLNet_UDP_Recv(pGame->udpSocket, pGame->packet))
    {
        struct
        {
            int x, y;
        } serverData;
        memcpy(&serverData, pGame->packet->data, sizeof(serverData));
        printf("Received from server: x=%d y=%d\n", serverData.x, serverData.y);
    }
}

void closeSnakeClient(Game *pGame)
{
    SDLNet_FreePacket(pGame->packet);
    SDLNet_UDP_Close(pGame->udpSocket);
    SDLNet_Quit();
}
