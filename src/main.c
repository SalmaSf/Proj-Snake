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
#include "snake_data.h"

#define SERVER_PORT 2000

const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 700;

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

    Snake *snakes[MAX_PLAYERS];
    int playerIndex;
    bool playerIndexSet;

    bool soundOn;

    Mix_Music *music;
    Mix_Chunk *collisionSound;

    TTF_Font *font;

    GameState state;
    ServerData serverData;

    UDPsocket udpSocket;
    IPaddress serverAddr;
    UDPpacket *packet;
} Game;

int initGame(Game *pGame);
void runGame(Game *pGame);
void cleanGame(Game *pGame);

int initSnakeClient(Game *pGame, const char *ipAddress);
void sendSnakePosition(Game *pGame, int x, int y);
void receiveServerUpdate(Game *pGame);
void closeSnakeClient(Game *pGame);
GameResult gameLoop(Snake *snakes[], SDL_Renderer *pRenderer, SDL_Texture *pBackground, Game *pGame);
void showPlayerIdentity(SDL_Renderer *renderer, int clientID);
int main(int argc, char *argv[])
{
    Game game = {0};

    if (!initGame(&game))
    {
        return 1;
    }
    game.soundOn = true;
    runGame(&game);
    cleanGame(&game);

    return 0;
}

int initGame(Game *pGame)
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0)
    {
        SDL_Log("SDL_Init failed: %s", SDL_GetError());
        return 0;
    }

    if (TTF_Init() < 0)
    {
        SDL_Log("TTF_Init failed: %s", TTF_GetError());
        return 0;
    }

    pGame->pWindow = SDL_CreateWindow("Snake Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, 0);
    if (!pGame->pWindow)
        return 0;

    pGame->pRenderer = SDL_CreateRenderer(pGame->pWindow, -1, SDL_RENDERER_ACCELERATED);
    if (!pGame->pRenderer)
        return 0;

    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0)
    {
        SDL_Log("Mix_OpenAudio failed: %s", Mix_GetError());
        return 0;
    }

    pGame->music = Mix_LoadMUS("resources/bakgrund.wav");
    if (!pGame->music)
        return 0;
    Mix_PlayMusic(pGame->music, -1);

    pGame->collisionSound = Mix_LoadWAV("resources/snake_rattle.wav");
    if (!pGame->collisionSound)
        return 0;

    pGame->pBackground = loadBackground(pGame->pRenderer, "resources/bakgrund.png");
    if (!pGame->pBackground)
        return 0;

    for (int i = 0; i < MAX_PLAYERS; i++)
    {
        pGame->snakes[i] = NULL;
    }

    pGame->playerIndex = 0;
    pGame->playerIndexSet = false;
    pGame->state = START;

    return 1;
}

void runGame(Game *pGame)
{
    bool programRunning = true;
    char ipBuffer[256];

    while (programRunning)
    {
        if (!showStartMenu(pGame->pRenderer, &pGame->soundOn))
            break;

        bool playAgain = true;
        while (playAgain)
        {

            if (!showIPMenu(pGame->pRenderer, ipBuffer, sizeof(ipBuffer)))
                break;

            if (strlen(ipBuffer) > 0)
                printf("User entered IP: %s\n", ipBuffer);
            else
                printf("No IP address entered.\n");

            if (!initSnakeClient(pGame, ipBuffer))
            {
                SDL_Log("Network initialization failed.");
                break;
            }

            for (int i = 0; i < MAX_PLAYERS; i++)
            {
                pGame->snakes[i] = NULL;
            }

            const char *joinedMsg = "Joined";
            memcpy(pGame->packet->data, joinedMsg, strlen(joinedMsg) + 1);
            pGame->packet->len = strlen(joinedMsg) + 1;
            SDLNet_UDP_Send(pGame->udpSocket, -1, pGame->packet);

            bool inLobby = true;
            while (inLobby)
            {
                receiveServerUpdate(pGame);

                if (pGame->state == ONGOING)
                {
                    showPlayerIdentity(pGame->pRenderer, pGame->playerIndex);

                    inLobby = false;
                    break;
                }

                if (!showLobby(pGame->pRenderer, pGame->serverData.numPlayers))
                {
                    playAgain = false;
                    inLobby = false;
                    break;
                }

                SDL_Delay(100);
            }

            for (int i = 0; i < MAX_PLAYERS; i++)
            {
                if (pGame->snakes[i])
                    destroySnake(pGame->snakes[i]);
            }

            pGame->snakes[0] = createSnake(WINDOW_WIDTH / 2, 0, pGame->pRenderer, WINDOW_WIDTH, WINDOW_HEIGHT, "resources/purple_head.png", "resources/purple_body.png");
            pGame->snakes[1] = createSnake(WINDOW_WIDTH / 2, WINDOW_HEIGHT, pGame->pRenderer, WINDOW_WIDTH, WINDOW_HEIGHT, "resources/yellow_head.png", "resources/yellow_body.png");
            pGame->snakes[2] = createSnake(0, WINDOW_HEIGHT / 2, pGame->pRenderer, WINDOW_WIDTH, WINDOW_HEIGHT, "resources/green_head.png", "resources/green_body.png");
            pGame->snakes[3] = createSnake(WINDOW_WIDTH, WINDOW_HEIGHT / 2, pGame->pRenderer, WINDOW_WIDTH, WINDOW_HEIGHT, "resources/pink_head.png", "resources/pink_body.png");

            GameResult result = gameLoop(pGame->snakes, pGame->pRenderer, pGame->pBackground, pGame);

            if (result.win)
            {
                int choice = showResult(pGame->pRenderer, true, result.time);
                if (choice == 0)
                    playAgain = false;
            }
        }
    }
}

GameResult gameLoop(Snake *snakes[], SDL_Renderer *renderer, SDL_Texture *background, Game *pGame)
{
    int myID = pGame->playerIndex;
    bool isRunning = true;
    bool resultShown = false;
    SDL_Event ev;

    Uint64 startTime = SDL_GetTicks64();
    int gameTime = -1;

    TTF_Font *font = TTF_OpenFont("resources/GamjaFlower-Regular.ttf", 24);
    SDL_Color txtColor = {255, 255, 255, 255};
    SDL_Texture *timerTex = NULL;
    SDL_Rect timerRect;

    while (isRunning)
    {

        receiveServerUpdate(pGame);
        if (pGame->state == GAME_OVER)
            isRunning = false;

        while (SDL_PollEvent(&ev))
        {
            if (ev.type == SDL_QUIT ||
                (ev.type == SDL_KEYDOWN && ev.key.keysym.sym == SDLK_ESCAPE))
                isRunning = false;
        }

        if (!pGame->snakes[myID])
        {
            SDL_Delay(50);
            continue;
        }

        int mx, my;
        SDL_GetMouseState(&mx, &my);

        for (int i = 0; i < MAX_PLAYERS; ++i)
        {
            if (!snakes[i] || !isSnakeAlive(snakes[i]))
                continue;

            if (i == myID)
            {

                updateSnake(snakes[i], true, mx, my);
            }
            else
            {

                SnakeInfo *si = &pGame->serverData.snakes[i];
                updateSnake(snakes[i], false, si->x, si->y);
                if (!si->alive)
                    killSnake(snakes[i]);
            }
        }

        int headX = getSnakeHeadX(snakes[myID]);
        int headY = getSnakeHeadY(snakes[myID]);
        sendSnakePosition(pGame, mx, my);

        int curTime = (SDL_GetTicks64() - startTime) / 1000;
        if (curTime > gameTime)
        {
            gameTime = curTime;
            if (timerTex)
                SDL_DestroyTexture(timerTex);

            char buf[32];
            sprintf(buf, "%02d:%02d", gameTime / 60, gameTime % 60);
            SDL_Surface *surf = TTF_RenderText_Solid(font, buf, txtColor);
            timerTex = SDL_CreateTextureFromSurface(renderer, surf);
            timerRect = (SDL_Rect){10, 10, surf->w, surf->h};
            SDL_FreeSurface(surf);
        }

        if (!isSnakeAlive(snakes[myID]) && !resultShown)
        {
            resultShown = true;
            int choice = showResult(renderer, false, (float)gameTime);
            if (choice == 0)
            {
                isRunning = false;
            }
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, background, NULL, NULL);

        for (int i = 0; i < MAX_PLAYERS; ++i)
        {
            if (!snakes[i] || !isSnakeAlive(snakes[i]))
                continue;

            if (i == myID && resultShown)
                continue;
            drawSnake(snakes[i]);
        }

        if (timerTex)
            SDL_RenderCopy(renderer, timerTex, NULL, &timerRect);

        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }

    if (timerTex)
        SDL_DestroyTexture(timerTex);
    TTF_CloseFont(font);

    return (GameResult){
        .win = isSnakeAlive(snakes[myID]),
        .time = (float)gameTime};
}

int initSnakeClient(Game *pGame, const char *ipAddress)
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

    if (SDLNet_ResolveHost(&pGame->serverAddr, ipAddress, SERVER_PORT) < 0)
    {
        SDL_Log("SDLNet_ResolveHost: %s\n", SDLNet_GetError());
        return 0;
    }
    printf("Successfully connected to server %s:%d\n", ipAddress, SERVER_PORT);

    pGame->packet = SDLNet_AllocPacket(512);
    if (!pGame->packet)
    {
        SDL_Log("SDLNet_AllocPacket: %s\n", SDLNet_GetError());
        return 0;
    }

    const char *joinMsg = "JOIN";
    memcpy(pGame->packet->data, joinMsg, strlen(joinMsg) + 1);
    pGame->packet->address = pGame->serverAddr;
    pGame->packet->len = strlen(joinMsg) + 1;
    SDLNet_UDP_Send(pGame->udpSocket, -1, pGame->packet);

    Uint32 start = SDL_GetTicks();
    while (SDL_GetTicks() - start < 5000)
    {
        if (SDLNet_UDP_Recv(pGame->udpSocket, pGame->packet))
        {
            int clientID;
            memcpy(&clientID, pGame->packet->data, sizeof(int));
            pGame->playerIndex = clientID;
            pGame->playerIndexSet = true;

            printf(" My clientID received from the server: %d\n", clientID);
            break;
        }
    }

    if (!pGame->playerIndexSet)
    {

        printf(" Could not accept ClientID from server.\n");
        return 0;
    }

    return 1;
}

void sendSnakePosition(Game *pGame, int x, int y)
{
    if (pGame->state != ONGOING)
        return;

    ClientData data = {0};
    data.x = x;
    data.y = y;
    data.clientID = pGame->playerIndex;
    if (pGame->snakes[pGame->playerIndex])
        data.alive = isSnakeAlive(pGame->snakes[pGame->playerIndex]);
    else
        data.alive = true;
    data.state = pGame->state;

    memcpy(pGame->packet->data, &data, sizeof(ClientData));
    pGame->packet->len = sizeof(ClientData);
    SDLNet_UDP_Send(pGame->udpSocket, -1, pGame->packet);
}

void receiveServerUpdate(Game *pGame)
{
    if (SDLNet_UDP_Recv(pGame->udpSocket, pGame->packet))
    {
        printf(" Packet from server (len = %d)\n", pGame->packet->len);

        ServerData serverData;
        memcpy(&serverData, pGame->packet->data, sizeof(ServerData));
        pGame->serverData = serverData;
        printf(" ServerData.numPlayers = %d\n", serverData.numPlayers);

        pGame->state = serverData.state;
        printf("Received server data: state = %d, num players = %d\n", pGame->state, serverData.numPlayers);

        if (!pGame->playerIndexSet)
        {

            for (int i = 0; i < serverData.numPlayers; i++)
            {
                if (serverData.snakes[i].x != 0 || serverData.snakes[i].y != 0)
                {
                    pGame->playerIndex = i;
                    pGame->playerIndexSet = true;
                    printf("Client index set: %d\n", i);
                    break;
                }
            }
        }
        printf("Received server data: state = %d, num players = %d\n", pGame->state, serverData.numPlayers);
        printf("Num players in serverData: %d\n", serverData.numPlayers);

        for (int i = 0; i < serverData.numPlayers; i++)
        {
            SnakeInfo *s = &serverData.snakes[i];
            printf(" Updating snake %d: position (%d, %d), alive: %d\n", s->clientID, s->x, s->y, s->alive);
        }
    }
}

void closeSnakeClient(Game *pGame)
{
    SDLNet_FreePacket(pGame->packet);
    SDLNet_UDP_Close(pGame->udpSocket);
    SDLNet_Quit();
}

void cleanGame(Game *pGame)
{
    for (int i = 0; i < MAX_PLAYERS; i++)
    {
        if (pGame->snakes[i])
        {
            destroySnake(pGame->snakes[i]);
            pGame->snakes[i] = NULL;
        }
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
    SDL_Quit();
}