#include "server.h"
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_net.h>
#include <stdio.h>

int initiate(Game *game)
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0)
    {
        printf("SDL_Init Error: %s\n", SDL_GetError());
        return 0;
    }

    if (IMG_Init(IMG_INIT_PNG) == 0)
    {
        printf("IMG_Init Error: %s\n", IMG_GetError());
        return 0;
    }

    if (SDLNet_Init() != 0)
    {
        printf("SDLNet_Init Error: %s\n", SDLNet_GetError());
        return 0;
    }

    game->window = SDL_CreateWindow("Snake Server", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                    game->windowWidth, game->windowHeight, 0);
    game->renderer = SDL_CreateRenderer(game->window, -1, SDL_RENDERER_ACCELERATED);

    game->background = loadBackground(game->renderer, "resources/bakgrund.png");

    game->socket = SDLNet_UDP_Open(PORT);
    if (!game->socket)
    {
        printf("SDLNet_UDP_Open Error: %s\n", SDLNet_GetError());
        return 0;
    }

    game->packet = SDLNet_AllocPacket(BUFFER_SIZE);
    if (!game->packet)
    {
        printf("SDLNet_AllocPacket Error: %s\n", SDLNet_GetError());
        return 0;
    }

    game->running = true;
    game->numClients = 0;

    return 1;
}

void run(Game *game)
{
    SDL_Event event;

    while (game->running)
    {
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
                game->running = false;
        }

        if (SDLNet_UDP_Recv(game->socket, game->packet))
        {
            handlePacket(game);
        }

        for (int i = 0; i < game->numClients; i++) {
            updateSnake(game->snakes[i]);
        }

        SDL_SetRenderDrawColor(game->renderer, 0, 0, 0, 255);
        SDL_RenderClear(game->renderer);

        SDL_RenderCopy(game->renderer, game->background, NULL, NULL);

        for (int i = 0; i < game->numClients; i++) {
            drawSnake(game->snakes[i]);
        }

        SDL_RenderPresent(game->renderer);

        sendGameData(game);
        SDL_Delay(16); // ~60 FPS repeats this 60 times per second
                        // delays 16 milliseconds 1000 ms (1 second) ÷ 16 ms ≈ 60 frames per second
    }
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

    for (int i = 0; i < game->numClients; i++) {
        serverData.snakes[i].x = (int)getSnakeX(game->snakes[i]);
        serverData.snakes[i].y = (int)getSnakeY(game->snakes[i]);
    }

    for (int i = 0; i < game->numClients; i++) {
        memcpy(game->packet->data, &serverData, sizeof(ServerData));
        game->packet->len = sizeof(ServerData);
        game->packet->address = game->clients[i];
        SDLNet_UDP_Send(game->socket, -1, game->packet);
    }
}

int getClientIndex(Game *game, IPaddress addr)
{
    for (int i = 0; i < game->numClients; i++)
    {
        if (addr.host == game->clients[i].host && addr.port == game->clients[i].port)
        {
            return i;
        }
    }

    if (game->numClients < MAX_PLAYERS)
    {
        game->clients[game->numClients] = addr;
        game->snakes[game->numClients] = createSnake(
            game->windowWidth / 2,
            game->windowHeight / 2,
            game->renderer,
            game->windowWidth,
            game->windowHeight);
        printf("New client added at index %d\n", game->numClients);
        return game->numClients++;
    }

    return -1; 
}

void closeGame(Game *game)
{
    for (int i = 0; i < game->numClients; i++)
    {
        destroySnake(game->snakes[i]);
    }
    SDL_DestroyRenderer(game->renderer);
    SDL_DestroyWindow(game->window);
    SDL_DestroyTexture(game->background);
    SDLNet_FreePacket(game->packet);
    SDLNet_UDP_Close(game->socket);
    SDLNet_Quit();
    IMG_Quit();
    SDL_Quit();
}
