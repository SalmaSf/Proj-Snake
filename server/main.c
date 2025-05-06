#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <SDL_net.h>
#include "snake_data.h"

#define SERVER_PORT 2000
#define MAX_CLIENTS 4
#define PACKET_SIZE 512

typedef struct {
    IPaddress addr;
    int id;
    bool active;
} Client;

int main(int argc, char *argv[]) {
    if (SDLNet_Init() < 0) {
        fprintf(stderr, "SDLNet_Init: %s\n", SDLNet_GetError());
        return 1;
    }

    UDPsocket socket = SDLNet_UDP_Open(SERVER_PORT);
    if (!socket) {
        fprintf(stderr, "SDLNet_UDP_Open: %s\n", SDLNet_GetError());
        SDLNet_Quit();
        return 1;
    }

    UDPpacket *recvPacket = SDLNet_AllocPacket(PACKET_SIZE);
    UDPpacket *sendPacket = SDLNet_AllocPacket(PACKET_SIZE);
    if (!recvPacket || !sendPacket) {
        fprintf(stderr, "SDLNet_AllocPacket error\n");
        SDLNet_Quit();
        return 1;
    }

    Client clients[MAX_CLIENTS] = {0};
    int numClients = 0;

    printf("Server running on port %d...\n", SERVER_PORT);

    while (1) {
        while (SDLNet_UDP_Recv(socket, recvPacket)) {
            int found = 0;
            int clientIndex = -1;

            // Check if this client already exists
            for (int i = 0; i < numClients; i++) {
                if (clients[i].active &&
                    clients[i].addr.host == recvPacket->address.host &&
                    clients[i].addr.port == recvPacket->address.port) {
                    found = 1;
                    clientIndex = clients[i].id;
                    break;
                }
            }

            // New client
            if (!found && numClients < MAX_CLIENTS) {
                clientIndex = numClients;
                clients[numClients].addr = recvPacket->address;
                clients[numClients].id = clientIndex;
                clients[numClients].active = true;
                numClients++;

                printf("New client connected with ID %d\n", clientIndex);
            }

            // Prepare and send response
            if (clientIndex != -1) {
                ClientData data;
                data.clientID = clientIndex;
                data.x = 100 + (clientIndex * 100);  // Example start X
                data.y = 100;                         // Fixed Y for simplicity
                data.isAlive = true;

                memcpy(sendPacket->data, &data, sizeof(ClientData));
                sendPacket->len = sizeof(ClientData);
                sendPacket->address = recvPacket->address;
                SDLNet_UDP_Send(socket, -1, sendPacket);
            }
        }
        SDL_Delay(10);
    }

    SDLNet_FreePacket(recvPacket);
    SDLNet_FreePacket(sendPacket);
    SDLNet_UDP_Close(socket);
    SDLNet_Quit();
    return 0;
}
