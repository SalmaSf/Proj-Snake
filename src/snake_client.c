#include <SDL.h>
#include <SDL_net.h>
#include <string.h>
#include "snake_client.h"

#define SERVER_IP "130.229.160.219"
#define SERVER_PORT 12345

static UDPsocket udpSocket;
static IPaddress serverAddr;
static UDPpacket* packet;

int initSnakeClient() {
    if (SDLNet_Init() < 0) {
        SDL_Log("SDLNet_Init: %s\n", SDLNet_GetError());
        return 0;
    }

    udpSocket = SDLNet_UDP_Open(0);
    if (!udpSocket) {
        SDL_Log("SDLNet_UDP_Open: %s\n", SDLNet_GetError());
        return 0;
    }

    if (SDLNet_ResolveHost(&serverAddr, SERVER_IP, SERVER_PORT) < 0) {
        SDL_Log("SDLNet_ResolveHost: %s\n", SDLNet_GetError());
        return 0;
    }

    packet = SDLNet_AllocPacket(512);
    if (!packet) {
        SDL_Log("SDLNet_AllocPacket: %s\n", SDLNet_GetError());
        return 0;
    }

    packet->address.host = serverAddr.host;
    packet->address.port = serverAddr.port;

    return 1;
}

void sendSnakePosition(int x, int y) {
    struct {
        int x, y;
    } SnakeData;

    SnakeData.x = x;
    SnakeData.y = y;

    memcpy(packet->data, &SnakeData, sizeof(SnakeData));
    packet->len = sizeof(SnakeData);

    SDLNet_UDP_Send(udpSocket, -1, packet);
}

void receiveServerUpdate() {
    if (SDLNet_UDP_Recv(udpSocket, packet)) {
        struct {
            int x, y;
        } serverData;

        memcpy(&serverData, packet->data, sizeof(serverData));
        printf("Received from server: x=%d y=%d\n", serverData.x, serverData.y);
    }
}

void closeSnakeClient() {
    SDLNet_FreePacket(packet);
    SDLNet_UDP_Close(udpSocket);
    SDLNet_Quit();
}
