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
    ClientData data; // Lägger till ClientData i klienten
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
    GameState state;
    ServerData sData;
} Game;

int initiate(Game *pGame);
void run(Game *pGame);
void cleanup(Game *pGame);
void sendGameData(Game *pGame);
void addClient(IPaddress address, Game *pGame);
void setUpGame(Game *pGame);
void handlePacket(Game *pGame);
int getClientIndex(Game *pGame, IPaddress *address);

int main(int argc, char *argv[])
{
    Game pGame = {0};

    printf("Startar Snake-servern...\n");

    if (!initiate(&pGame))
    {
        printf("Initiering misslyckades.\n");
        return 1;
    }

    printf("Initiering lyckades. Run game.\n");
    run(&pGame);

    cleanup(&pGame);
    return 0;
}

int initiate(Game *pGame)
{
    pGame->windowWidth = 800;
    pGame->windowHeight = 700;

    printf("Initierar SDL...\n");

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0)
    {
        printf("SDL_Init Error: %s\n", SDL_GetError());
        cleanup(pGame);
        return 0;
    }
    printf("Initierar SDL_image...\n");

    if (IMG_Init(IMG_INIT_PNG) == 0)
    {
        printf("IMG_Init Error: %s\n", IMG_GetError());
        cleanup(pGame);
        return 0;
    }

    printf("Initierar SDL_net...\n");

    if (SDLNet_Init() != 0)
    {
        printf("SDLNet_Init Error: %s\n", SDLNet_GetError());
        cleanup(pGame);
        return 0;
    }
    printf("Create window...\n");

    pGame->window = SDL_CreateWindow("Snake Server",
                                     SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                     pGame->windowWidth, pGame->windowHeight, 0);
    if (!pGame->window)
    {
        printf("SDL_CreateWindow Error: %s\n", SDL_GetError());
        cleanup(pGame);
        return 0;
    }
    printf("Skapar renderer...\n");

    pGame->renderer = SDL_CreateRenderer(pGame->window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!pGame->renderer)
    {
        printf("SDL_CreateRenderer Error: %s\n", SDL_GetError());
        cleanup(pGame);
        return 0;
    }
    printf("Open UPD_socet att  %d...\n", PORT);

    pGame->socket = SDLNet_UDP_Open(PORT);
    if (!pGame->socket)
    {
        printf("SDLNet_UDP_Open Error: %s\n", SDLNet_GetError());
        cleanup(pGame);
        return 0;
    }
    printf("Allokerar UDP-paket...\n");

    pGame->packet = SDLNet_AllocPacket(BUFFER_SIZE);
    if (!pGame->packet)
    {
        printf("SDLNet_AllocPacket Error: %s\n", SDLNet_GetError());
        cleanup(pGame);
        return 0;
    }
    printf("Initiering klar! waiting for players...\n");

    pGame->running = true;
    pGame->state = START;

    return 1;
}

void run(Game *pGame)
{
    SDL_Event event;
    ClientData cData;

    while (pGame->running)
    {
        switch (pGame->state)
        {
        case ONGOING:
            sendGameData(pGame);

            while (SDLNet_UDP_Recv(pGame->socket, pGame->packet) == 1)
            {
                memcpy(&cData, pGame->packet->data, sizeof(ClientData));
                int clientIndex = getClientIndex(pGame, &pGame->packet->address);
                if (clientIndex >= 0)
                {
                    pGame->clients[clientIndex].data = cData;
                }
            }

            if (SDL_PollEvent(&event) && event.type == SDL_QUIT)
                pGame->running = false;

            for (int i = 0; i < MAX_PLAYERS; i++)
            {
                if (pGame->clients[i].active && pGame->snakes[i])
                {
                    ClientData *data = &pGame->clients[i].data;
                    if (data->alive)
                        setSnakePosition(pGame->snakes[i], data->x, data->y);
                    else
                        killSnake(pGame->snakes[i]);
                }
            }

            for (int i = 0; i < MAX_PLAYERS; i++)
            {
                for (int j = 0; j < MAX_PLAYERS; j++)
                {
                    if (i != j && pGame->snakes[i] && pGame->snakes[j])
                    {
                        if (checkCollision(pGame->snakes[i], pGame->snakes[j]))
                        {
                            killSnake(pGame->snakes[j]);
                        }
                    }
                }
            }

            // Kolla om bara en orm är kvar
            int aliveCount = 0;
            for (int i = 0; i < MAX_PLAYERS; i++)
            {
                if (pGame->snakes[i] && isSnakeAlive(pGame->snakes[i]))
                {
                    aliveCount++;
                }
            }

            if (aliveCount <= 1)
            {
                pGame->state = GAME_OVER;
            }

            // TODO: Lägg till kollisioner här och uppdatera state till GAME_OVER vid behov

            SDL_SetRenderDrawColor(pGame->renderer, 0, 0, 0, 255);
            SDL_RenderClear(pGame->renderer);

            SDL_RenderCopy(pGame->renderer, pGame->background, NULL, NULL);

            for (int i = 0; i < MAX_PLAYERS; i++)
            {
                if (pGame->clients[i].active && pGame->snakes[i])
                    drawSnake(pGame->snakes[i]);
            }

            SDL_RenderPresent(pGame->renderer);
            break;

        case GAME_OVER:
            sendGameData(pGame);
            SDL_SetRenderDrawColor(pGame->renderer, 0, 0, 0, 255);
            SDL_RenderClear(pGame->renderer);
            // TODO: Lägg till "Game Over" grafik eller text
            SDL_RenderPresent(pGame->renderer);
            break;

        case START:
    
            SDL_SetRenderDrawColor(pGame->renderer, 0, 0, 0, 255);
            SDL_RenderClear(pGame->renderer);
            // TODO: Lägg till "Väntar på spelare" grafik eller text
            SDL_RenderPresent(pGame->renderer);

            if (SDL_PollEvent(&event) && event.type == SDL_QUIT)
                pGame->running = false;

            if (SDLNet_UDP_Recv(pGame->socket, pGame->packet) == 1)
            {
                
                addClient(pGame->packet->address, pGame);
                if (pGame->numClients == MAX_PLAYERS)
                {
                    setUpGame(pGame);
                    pGame->state = ONGOING;
                }
            }
            break;
        }

        SDL_Delay(1000 / 60);
    }
}

void cleanup(Game *pGame)
{
    if (pGame->packet)
        SDLNet_FreePacket(pGame->packet);
    if (pGame->socket)
        SDLNet_UDP_Close(pGame->socket);
    if (pGame->renderer)
        SDL_DestroyRenderer(pGame->renderer);
    if (pGame->window)
        SDL_DestroyWindow(pGame->window);
    if (pGame->background)
        SDL_DestroyTexture(pGame->background);
    IMG_Quit();
    SDLNet_Quit();
    SDL_Quit();
}

void setUpGame(Game *pGame)
{
    for (int i = 0; i < MAX_PLAYERS; i++)
    {
        if (pGame->snakes[i])
        {
            destroySnake(pGame->snakes[i]);
        }
    }

    pGame->snakes[0] = createSnake(400, 0, pGame->renderer, 800, 700, "resources/purple_head.png", "resources/purple_body.png");
    pGame->snakes[1] = createSnake(400, 700, pGame->renderer, 800, 700, "resources/yellow_head.png", "resources/yellow_body.png");
    pGame->snakes[2] = createSnake(0, 350, pGame->renderer, 800, 700, "resources/green_head.png", "resources/green_body.png");
    pGame->snakes[3] = createSnake(800, 350, pGame->renderer, 800, 700, "resources/pink_head.png", "resources/pink_body.png");

    pGame->state = ONGOING;
}

void sendGameData(Game *pGame)
{
    pGame->sData.numPlayers = pGame->numClients;
    pGame->sData.state = pGame->state;

    for (int i = 0; i < MAX_PLAYERS; i++)
    {
        pGame->sData.snakes[i].clientID = i;
        pGame->sData.snakes[i].x = getSnakeHeadX(pGame->snakes[i]);
        pGame->sData.snakes[i].y = getSnakeHeadY(pGame->snakes[i]);
        // pGame->sData.snakes[i].alive = pGame->snakes[i] && isSnakeAlive(pGame->snakes[i]);
        if (pGame->snakes[i] != NULL && isSnakeAlive(pGame->snakes[i]))
        {
            pGame->sData.snakes[i].alive = true;
        }
        else
        {
            pGame->sData.snakes[i].alive = false;
        }
    }

    for (int i = 0; i < MAX_PLAYERS; i++)
    {
        if (pGame->clients[i].active)
        {
            if (sizeof(pGame->packet->data) >= sizeof(ServerData))
            {
                memcpy(pGame->packet->data, &pGame->sData, sizeof(ServerData));
                pGame->packet->len = sizeof(ServerData);
            }
            pGame->packet->address = pGame->clients[i].address;
            SDLNet_UDP_Send(pGame->socket, -1, pGame->packet);
        }
    }
}

int getClientIndex(Game *pGame, IPaddress *address)
{
    for (int i = 0; i < pGame->numClients; i++)
    {
        if (pGame->clients[i].address.host == address->host &&
            pGame->clients[i].address.port == address->port)
        {
            return i;
        }
    }
    return -1; // No matching client found
}
void handlePacket(Game *pGame)
{
    if (!pGame->packet || pGame->packet->len < sizeof(ClientData))
        return;

    addClient(pGame->packet->address, pGame);

    ClientData clientData;
    memcpy(&clientData, pGame->packet->data, sizeof(ClientData));

    int clientIndex = getClientIndex(pGame, &pGame->packet->address);
    if (clientIndex == -1)
    {
        printf("För många klienter anslutna. Ignorerar paket.\n");
        return;
    }

    // Uppdatera klientens data
    pGame->clients[clientIndex].data = clientData; // Sparar klientens data i Game-strukturen
}

void addClient(IPaddress address, Game *pGame)
{
    for (int i = 0; i < pGame->numClients; i++)
    {
        if (address.host == pGame->clients[i].address.host && address.port == pGame->clients[i].address.port)
            return;
    }

    if (pGame->numClients < MAX_PLAYERS)
    {
        int index = pGame->numClients;
        pGame->clients[index].address = address;
        pGame->clients[index].index = index;
        pGame->clients[index].active = true;
        pGame->numClients++;
        printf("New client added at index %d\n", index);
    }
} 