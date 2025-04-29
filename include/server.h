#ifndef SERVER_H
#define SERVER_H

#include <SDL_net.h>
#include <stdbool.h>
#include <snake.h>
#include <include/bakgrund.h>
#include <snake_data.h>

#define PORT 12345
#define BUFFER_SIZE 512
#define MAX_PLAYERS 4

typedef struct
{
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Texture *background;
    Snake *snakes[MAX_PLAYERS];
    IPaddress clients[MAX_PLAYERS];
    int numClients;

    UDPsocket socket;
    UDPpacket *packet;
    int windowWidth;
    int windowHeight;
    bool running;
} Game;

int initiate(Game *game);
void run(Game *game);
void closeGame(Game *game);
void handlePacket(Game *game);
void sendGameData(Game *game);
int getClientIndex(Game* game, IPaddress addr);

#endif
