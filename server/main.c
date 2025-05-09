#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_net.h>
#include "snake.h"
#include "bakgrund.h"
#include "snake_data.h"

#define PORT 2000
#define BUFFER_SIZE 512
#define MAX_PLAYERS 4

typedef struct
{
    IPaddress address;
    int index;
    bool active;
} Client;

typedef struct
{
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Texture *background;
    UDPsocket socket;
    UDPpacket *packet;
    Client clients[MAX_PLAYERS];
    Snake *snakes[MAX_PLAYERS];
    int numClients;
    int windowWidth;
    int windowHeight;
    bool running;
} Game;

enum gameState
{
    START,
    ONGOING,
    GAME_OVER
};
typedef enum gameState GameState;

int getClientIndex(Game *game, IPaddress addr);
void sendGameData(Game *game);
void handlePacket(Game *game);
void initServer(Game *pGame, SDL_Renderer *renderer);

int main(int argc, char *argv[])
{
    Game game = {0};
    game.windowWidth = 800;
    game.windowHeight = 700;
    game.running = true;
    Uint64 gameStartTime = 0;
    bool gameStarted = false;

    if (!gameStarted && gameStartTime > 0 && SDL_GetTicks64() - gameStartTime >= 5000)
    {
        gameStarted = true;
        printf("Spelet har börjat! Skickar START till klienter...\n");

        const char *startMsg = "START";
        for (int i = 0; i < MAX_PLAYERS; i++)
        {
            if (game.clients[i].active)
            {
                memcpy(game.packet->data, startMsg, strlen(startMsg) + 1);
                game.packet->len = strlen(startMsg) + 1;
                game.packet->address = game.clients[i].address;
                SDLNet_UDP_Send(game.socket, -1, game.packet);
            }
        }
    }

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0)
    {
        printf("SDL_Init Error: %s\n", SDL_GetError());
        return 1;
    }

    if (IMG_Init(IMG_INIT_PNG) == 0)
    {
        printf("IMG_Init Error: %s\n", IMG_GetError());
        return 1;
    }

    if (SDLNet_Init() != 0)
    {
        printf("SDLNet_Init Error: %s\n", SDLNet_GetError());
        return 1;
    }

    game.window = SDL_CreateWindow("Snake Server", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                   game.windowWidth, game.windowHeight, 0);
    game.renderer = SDL_CreateRenderer(game.window, -1, SDL_RENDERER_ACCELERATED);
    game.background = loadBackground(game.renderer, "resources/bakgrund.png");

    game.socket = SDLNet_UDP_Open(PORT);
    if (!game.socket)
    {
        printf("SDLNet_UDP_Open Error: %s\n", SDLNet_GetError());
        return 1;
    }

    game.packet = SDLNet_AllocPacket(BUFFER_SIZE);
    if (!game.packet)
    {
        printf("SDLNet_AllocPacket Error: %s\n", SDLNet_GetError());
        return 1;
    }

    SDL_Event event;
    while (game.running)
    {
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
                game.running = false;
        }

        if (SDLNet_UDP_Recv(game.socket, game.packet))
        {
            handlePacket(&game);
        }

        if (game.numClients == 2 && gameStartTime == 0)
        {
            gameStartTime = SDL_GetTicks64();
            printf("Två spelare anslutna. Startar om 5 sekunder...\n");
        }

        // Skicka START efter 5 sekunder
        if (!gameStarted && gameStartTime > 0 && SDL_GetTicks64() - gameStartTime >= 5000)
        {
            gameStarted = true;
            printf("Spelet har börjat! Skickar START till klienter...\n");

            const char *startMsg = "START";
            for (int i = 0; i < MAX_PLAYERS; i++)
            {
                if (game.clients[i].active)
                {
                    memcpy(game.packet->data, startMsg, strlen(startMsg) + 1);
                    game.packet->len = strlen(startMsg) + 1;
                    game.packet->address = game.clients[i].address;
                    SDLNet_UDP_Send(game.socket, -1, game.packet);
                }
            }
        }

        SDL_SetRenderDrawColor(game.renderer, 0, 0, 0, 255);
        SDL_RenderClear(game.renderer);
        SDL_RenderCopy(game.renderer, game.background, NULL, NULL);

        for (int i = 0; i < MAX_PLAYERS; i++)
        {
            if (game.clients[i].active && game.snakes[i])
            {
                drawSnake(game.snakes[i]);
            }
        }

        SDL_RenderPresent(game.renderer);
        sendGameData(&game);
        SDL_Delay(16); // ~60 FPS
    }

    for (int i = 0; i < MAX_PLAYERS; i++)
    {
        if (game.snakes[i])
        {
            destroySnake(game.snakes[i]);
        }
    }
    SDL_DestroyRenderer(game.renderer);
    SDL_DestroyWindow(game.window);
    SDL_DestroyTexture(game.background);
    SDLNet_FreePacket(game.packet);
    SDLNet_UDP_Close(game.socket);
    SDLNet_Quit();
    IMG_Quit();
    SDL_Quit();

    return 0;
}
/*
void initServer(Game *pGame, SDL_Renderer *renderer)
{
    pGame->renderer = renderer;
    pGame->windowWidth = 800;
    pGame->windowHeight = 700;
    pGame->numClients = 0;

    // Skapa 4 färdiga ormar
    pGame->snakes[0] = createSnake(400, 0, renderer, 800, 700, "resources/purple_head.png", "resources/purple_body.png");
    pGame->snakes[1] = createSnake(400, 700, renderer, 800, 700, "resources/yellow_head.png", "resources/yellow_body.png");
    pGame->snakes[2] = createSnake(0, 350, renderer, 800, 700, "resources/green_head.png", "resources/green_body.png");
    pGame->snakes[3] = createSnake(800, 350, renderer, 800, 700, "resources/pink_head.png", "resources/pink_body.png");

    // Markera alla klientplatser som tomma
    for (int i = 0; i < MAX_PLAYERS; i++)
    {
        pGame->clients[i].active = false;
    }
}*/

int getClientIndex(Game *game, IPaddress addr)
{
    for (int i = 0; i < MAX_PLAYERS; i++)
    {
        if (game->clients[i].active &&
            addr.host == game->clients[i].address.host &&
            addr.port == game->clients[i].address.port)
        {
            return game->clients[i].index;
        }
    }

    if (game->numClients < MAX_PLAYERS) {
        int index = game->numClients;
        game->clients[index].address = addr;
        game->clients[index].index = index;
        game->clients[index].active = true;
        game->snakes[index] = createSnake(
            game->windowWidth / 2,
            game->windowHeight / 2,
            game->renderer,
            game->windowWidth,
            game->windowHeight,
            "resources/purple_head.png",
            "resources/purple_body.png");
        game->numClients++;
        printf("New client added at index %d\n", index);
        return index;
    }

    /*if (game->numClients < MAX_PLAYERS)
    {
        int index = game->numClients;
        game->clients[index].address = addr;
        game->clients[index].index = index;
        game->clients[index].active = true;

        // Ormen finns redan i game->snakes[index], så ingen ny orm skapas här
        game->numClients++;
        printf("New client added at index %d\n", index);
        return index;
    }*/

    return -1;
}

void handlePacket(Game *game)
{
    ClientData clientData;
    memcpy(&clientData, game->packet->data, sizeof(ClientData));

    int clientIndex = getClientIndex(game, game->packet->address);
    if (clientIndex == -1)
    {
        printf("Too many clients connected. Ignoring packet.\n");
        return;
    }

    setSnakePosition(game->snakes[clientIndex], clientData.x, clientData.y);
    printf("Received from client %d: x=%d y=%d\n", clientIndex, clientData.x, clientData.y);
}

void sendGameData(Game *game)
{
    ServerData serverData;
    serverData.numSnakes = game->numClients;

    for (int i = 0; i < MAX_PLAYERS; i++) {
        if (game->clients[i].active && game->snakes[i]) {
            serverData.x[i] = getSnakeHeadX(game->snakes[i]);
            serverData.y[i] = getSnakeHeadY(game->snakes[i]);
            serverData.isAlive[i] = isSnakeAlive(game->snakes[i]);
            serverData.clientID[i] = game->clients[i].index;
        }
    }

    for (int i = 0; i < MAX_PLAYERS; i++) {
        if (game->clients[i].active) {
            serverData.myClientID = game->clients[i].index; // Set the ID for *this* client
            printf("Sending to client %d: myClientID = %d\n", i, serverData.myClientID);
            memcpy(game->packet->data, &serverData, sizeof(ServerData));
            game->packet->len = sizeof(ServerData);
            game->packet->address = game->clients[i].address;

            SDLNet_UDP_Send(game->socket, -1, game->packet);
        }
    }
}