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

typedef struct {
    IPaddress address;
    int index;
    bool active;
} Client;

typedef struct {
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

int getClientIndex(Game *game, IPaddress addr);
void sendGameData(Game *game);
void handlePacket(Game *game);

int main(int argc, char *argv[]) {
    Game game = {0};
    game.windowWidth = 800;
    game.windowHeight = 700;
    game.running = true;

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0) {
        printf("SDL_Init Error: %s\n", SDL_GetError());
        return 1;
    }

    if (IMG_Init(IMG_INIT_PNG) == 0) {
        printf("IMG_Init Error: %s\n", IMG_GetError());
        return 1;
    }

    if (SDLNet_Init() != 0) {
        printf("SDLNet_Init Error: %s\n", SDLNet_GetError());
        return 1;
    }

    game.window = SDL_CreateWindow("Snake Server", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                   game.windowWidth, game.windowHeight, 0);
    game.renderer = SDL_CreateRenderer(game.window, -1, SDL_RENDERER_ACCELERATED);
    game.background = loadBackground(game.renderer, "resources/bakgrund.png");

    game.socket = SDLNet_UDP_Open(PORT);
    if (!game.socket) {
        printf("SDLNet_UDP_Open Error: %s\n", SDLNet_GetError());
        return 1;
    }

    game.packet = SDLNet_AllocPacket(BUFFER_SIZE);
    if (!game.packet) {
        printf("SDLNet_AllocPacket Error: %s\n", SDLNet_GetError());
        return 1;
    }

    SDL_Event event;
    while (game.running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT)
                game.running = false;
        }

        if (SDLNet_UDP_Recv(game.socket, game.packet)) {
            handlePacket(&game);
        }

        SDL_SetRenderDrawColor(game.renderer, 0, 0, 0, 255);
        SDL_RenderClear(game.renderer);
        SDL_RenderCopy(game.renderer, game.background, NULL, NULL);

        for (int i = 0; i < MAX_PLAYERS; i++) {
            if (game.clients[i].active && game.snakes[i]) {
                drawSnake(game.snakes[i]);
            }
        }

        SDL_RenderPresent(game.renderer);
        sendGameData(&game);
        SDL_Delay(16); // ~60 FPS
    }

    for (int i = 0; i < MAX_PLAYERS; i++) {
        if (game.snakes[i]) {
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

int getClientIndex(Game *game, IPaddress addr) {
    for (int i = 0; i < MAX_PLAYERS; i++) {
        if (game->clients[i].active &&
            addr.host == game->clients[i].address.host &&
            addr.port == game->clients[i].address.port) {
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

    return -1;
}

void handlePacket(Game *game) {
    ClientData clientData;
    memcpy(&clientData, game->packet->data, sizeof(ClientData));

    int clientIndex = getClientIndex(game, game->packet->address);
    if (clientIndex == -1) {
        printf("Too many clients connected. Ignoring packet.\n");
        return;
    }

    setSnakePosition(game->snakes[clientIndex], clientData.x, clientData.y);
    printf("Received from client %d: x=%d y=%d\n", clientIndex, clientData.x, clientData.y);
}

void sendGameData(Game *game) {
    ServerData serverData;
    serverData.numSnakes = game->numClients;

    for (int i = 0; i < MAX_PLAYERS; i++) {
        if (game->clients[i].active && game->snakes[i]) {
            serverData.snakes[i].x = getSnakeHeadX(game->snakes[i]);
            serverData.snakes[i].y = getSnakeHeadY(game->snakes[i]);
        }
    }

    for (int i = 0; i < MAX_PLAYERS; i++) {
        if (game->clients[i].active) {
            memcpy(game->packet->data, &serverData, sizeof(ServerData));
            game->packet->len = sizeof(ServerData);
            game->packet->address = game->clients[i].address;
            SDLNet_UDP_Send(game->socket, -1, game->packet);
        }
    }
}