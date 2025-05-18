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

    pGame->packet = SDLNet_AllocPacket(sizeof(ServerData));
    if (!pGame->packet)
    {
        printf("SDLNet_AllocPacket Error: %s\n", SDLNet_GetError());
        cleanup(pGame);
        return 0;
    }
    printf("Initiering klar! waiting for players...\n");
    printf("✅ packet->maxlen = %d, sizeof(ServerData) = %lu\n", pGame->packet->maxlen, sizeof(ServerData));

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
                // ✔✔ FIX 3: Ignorera inkommande klientpaket om spelet inte pågår
                if (pGame->state != ONGOING)
                    continue;

                memcpy(&cData, pGame->packet->data, sizeof(ClientData));
                int clientIndex = getClientIndex(pGame, &pGame->packet->address);
                if (clientIndex >= 0)
                {
                    pGame->clients[clientIndex].data = cData;
                    printf("[RECV] Data från klient %d: x=%d, y=%d, alive=%d\n",
                           clientIndex, cData.x, cData.y, cData.alive);
                }
                else
                {
                    printf("[RECV] Okänd klient. Paket ignorerat.\n");
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
                    printf("[SNAKE] Orm %d död.\n", i);
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
                            printf("[KOLLISION] Orm %d kolliderade med orm %d och dog.\n", j, i);
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
            printf("[STATUS] Antal levande ormar: %d\n", aliveCount);

            if (aliveCount <= 1)
            {
                printf("[GAME OVER] Endast %d orm(ar) kvar. Spelet avslutas.\n", aliveCount);

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
            printf("[STATE] GAME_OVER - skickar slutdata till klienter.\n");

            sendGameData(pGame);
            SDL_SetRenderDrawColor(pGame->renderer, 0, 0, 0, 255);
            SDL_RenderClear(pGame->renderer);
            // TODO: Lägg till "Game Over" grafik eller text
            SDL_RenderPresent(pGame->renderer);
            break;

        case START:
            while (SDL_PollEvent(&event))
            {
                if (event.type == SDL_QUIT)
                    pGame->running = false;
            }
            printf("[STATE] START - Väntar på klienter... (%d/%d anslutna)\n", pGame->numClients, MAX_PLAYERS);

            int recvResult = SDLNet_UDP_Recv(pGame->socket, pGame->packet);
            if (recvResult == 1)
            {
                printf("[NET] Paket mottaget i START.\n");

                addClient(pGame->packet->address, pGame);
                printf("[START] Ny klient ansluten. Totalt: %d\n", pGame->numClients);

                sendGameData(pGame);
                if (pGame->numClients == MAX_PLAYERS)
                {
                    printf("[START] MAX_PLAYERS uppnått. Startar spel...\n");
                    setUpGame(pGame);
                    pGame->state = ONGOING;
                }
            }
            SDL_RenderClear(pGame->renderer);
            // TODO: Lägg till "Väntar på spelare" grafik eller text
            SDL_RenderPresent(pGame->renderer);
<<<<<<< HEAD

            if (SDL_PollEvent(&event) && event.type == SDL_QUIT)
                pGame->running = false;

            break;
=======
>>>>>>> 09822e612e6f3e9402248bf7d4de673752cd920c
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
/*
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
}*/

void setUpGame(Game *pGame)
{
    // Ta bort gamla ormar om de finns
    for (int i = 0; i < MAX_PLAYERS; i++)
    {
        if (pGame->snakes[i])
        {
            destroySnake(pGame->snakes[i]);
            pGame->snakes[i] = NULL;
        }
    }

    // Nollställ clients också (valfritt)
    for (int i = 0; i < MAX_PLAYERS; i++)
    {
        pGame->clients[i].active = false;
    }

    pGame->numClients = 0;
    pGame->state = ONGOING;
}

void sendGameData(Game *pGame)
{
    pGame->sData.numPlayers = pGame->numClients;
    pGame->sData.state = pGame->state;

    /*for (int i = 0; i < MAX_PLAYERS; i++)
    {
        pGame->sData.snakes[i].clientID = i;
        if (pGame->snakes[i])
        {
            pGame->sData.snakes[i].x = getSnakeHeadX(pGame->snakes[i]);
            pGame->sData.snakes[i].y = getSnakeHeadY(pGame->snakes[i]);
            pGame->sData.snakes[i].alive = isSnakeAlive(pGame->snakes[i]);
        }
        else
        {
            pGame->sData.snakes[i].x = 0;
            pGame->sData.snakes[i].y = 0;
            pGame->sData.snakes[i].alive = false;
        }
    }*/
    for (int i = 0; i < MAX_PLAYERS; i++)
    {
        pGame->sData.snakes[i].clientID = i;

        if (pGame->snakes[i] != NULL)
        {
            pGame->sData.snakes[i].x = getSnakeHeadX(pGame->snakes[i]);
            pGame->sData.snakes[i].y = getSnakeHeadY(pGame->snakes[i]);
            pGame->sData.snakes[i].alive = isSnakeAlive(pGame->snakes[i]);
        }
        else
        {
            pGame->sData.snakes[i].x = -1;
            pGame->sData.snakes[i].y = -1;
            pGame->sData.snakes[i].alive = false;
        }
    }

    /*for (int i = 0; i < MAX_PLAYERS; i++)
    {
        if (pGame->clients[i].active)
        {
<<<<<<< HEAD
            if (sizeof(pGame->packet->data) >= sizeof(ServerData))
            {
                memcpy(pGame->packet->data, &pGame->sData, sizeof(ServerData));
                printf("sizeof(ServerData) = %lu, packet->maxlen = %d\n", sizeof(ServerData), pGame->packet->maxlen);
                pGame->packet->len = sizeof(ServerData);
            }
            pGame->packet->address = pGame->clients[i].address;
            SDLNet_UDP_Send(pGame->socket, -1, pGame->packet);
        }
    }*/
    for (int i = 0; i < MAX_PLAYERS; i++)
    {
        if (pGame->clients[i].active)
        {
            if (pGame->packet->maxlen >= sizeof(ServerData)) // ✅ FIXAD KOLL
            {
                memcpy(pGame->packet->data, &pGame->sData, sizeof(ServerData));
                pGame->packet->len = sizeof(ServerData);
                printf("Kopierade ServerData till paket (size = %lu bytes)\n", sizeof(ServerData));
            }
            else
            {
                printf(" ServerData för stor för paketbuffer! maxlen=%d, krävs=%lu\n",
                       pGame->packet->maxlen, sizeof(ServerData));
                continue; // hoppa över sändning
            }

=======
            memcpy(pGame->packet->data, &pGame->sData, sizeof(ServerData));
            pGame->packet->len = sizeof(ServerData);
>>>>>>> 09822e612e6f3e9402248bf7d4de673752cd920c
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
        if (address.host == pGame->clients[i].address.host &&
            address.port == pGame->clients[i].address.port)
            return;
    }

    if (pGame->numClients < MAX_PLAYERS)
    {
        int index = pGame->numClients;
        pGame->clients[index].address = address;
        pGame->clients[index].index = index;
        pGame->clients[index].active = true;
        pGame->numClients++;

        int clientID = index;
        memcpy(pGame->packet->data, &clientID, sizeof(int));
        pGame->packet->len = sizeof(int);
        pGame->packet->address.host = address.host;
        pGame->packet->address.port = address.port;
        if (!SDLNet_UDP_Send(pGame->socket, -1, pGame->packet))
        {
            perror("Kunde inte skicka clientID till klient");
        }
        else
        {
            printf("Skickade clientID %d till klienten\n", clientID);
        }

        printf(" New client added at index %d\n", index);

        // 🐍 Skapa orm för denna klient
        if (pGame->snakes[index])
        {
            destroySnake(pGame->snakes[index]);
        }
        printf("Skickade clientID %d till klienten\n", clientID);
        //  Olika startpositioner och färger per klient
        switch (index)
        {
        case 0:
            pGame->snakes[index] = createSnake(400, 0, pGame->renderer, 800, 700, "resources/purple_head.png", "resources/purple_body.png");
            pGame->sData.snakes[index].alive = 1;
            break;
        case 1:
            pGame->snakes[index] = createSnake(400, 700, pGame->renderer, 800, 700, "resources/yellow_head.png", "resources/yellow_body.png");
            pGame->sData.snakes[index].alive = 1;
            break;
        case 2:
            pGame->snakes[index] = createSnake(0, 350, pGame->renderer, 800, 700, "resources/green_head.png", "resources/green_body.png");
            pGame->sData.snakes[index].alive = 1;
            break;
        case 3:
            pGame->snakes[index] = createSnake(800, 350, pGame->renderer, 800, 700, "resources/pink_head.png", "resources/pink_body.png");
            pGame->sData.snakes[index].alive = 1;
            break;
        default:
            printf(" Ogiltigt ormindex %d\n", index);
        }
    }
    else
    {
        printf(" Max antal klienter (%d) uppnått.\n", MAX_PLAYERS);
    }
}

/*
void addClient(IPaddress address, Game *pGame)
{
    for (int i = 0; i < pGame->numClients; i++)
    {
        if (address.host == pGame->clients[i].address.host &&
             address.port == pGame->clients[i].address.port)
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
    else
    {
        printf("Max antal klienter (%d) uppnått.\n", MAX_PLAYERS);
    }
}*/