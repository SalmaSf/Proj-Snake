#ifndef SERVER_H
#define SERVER_H

#include <SDL_net.h>
#include <stdbool.h>
#include "snake_data.h" // Only data the server needs

#define PORT 12345
#define BUFFER_SIZE 512
#define MAX_PLAYERS 4

typedef struct
{
    UDPsocket socket;
    UDPpacket *packet;
    IPaddress clients[MAX_PLAYERS];

    int numClients;
    int windowWidth;
    int windowHeight;
    bool running;

    // Server-side snake state (raw data, not graphical Snake structs)
    SerializableSnake snakes[MAX_PLAYERS];
} Game;

// Function declarations for server logic
int initiate(Game *game);
void run(Game *game);
void closeGame(Game *game);
void handlePacket(Game *game);
void sendGameData(Game *game);
int getClientIndex(Game *game, IPaddress addr);

#endif
