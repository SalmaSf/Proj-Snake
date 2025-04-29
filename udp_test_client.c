#include <SDL.h>
#include <SDL_net.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/snake_data.h"

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 12345
#define BUFFER_SIZE 512

int main(int argc, char* argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        printf("SDL_Init error: %s\n", SDL_GetError());
        return 1;
    }

    if (SDLNet_Init() != 0) {
        printf("SDLNet_Init error: %s\n", SDLNet_GetError());
        SDL_Quit();
        return 1;
    }

    UDPsocket clientSocket = SDLNet_UDP_Open(0); // 0 = let system assign a port
    if (!clientSocket) {
        printf("SDLNet_UDP_Open error: %s\n", SDLNet_GetError());
        SDLNet_Quit();
        SDL_Quit();
        return 1;
    }

    IPaddress serverAddress;
    if (SDLNet_ResolveHost(&serverAddress, SERVER_IP, SERVER_PORT) != 0) {
        printf("SDLNet_ResolveHost error: %s\n", SDLNet_GetError());
        SDLNet_Quit();
        SDL_Quit();
        return 1;
    }

    UDPpacket* packet = SDLNet_AllocPacket(BUFFER_SIZE);
    if (!packet) {
        printf("SDLNet_AllocPacket error: %s\n", SDLNet_GetError());
        SDLNet_UDP_Close(clientSocket);
        SDLNet_Quit();
        SDL_Quit();
        return 1;
    }

    // Fake mouse coordinates to send
    int fakeMouseX[] = {100, 150, 200, 250, 300};
    int fakeMouseY[] = {100, 120, 140, 160, 180};

    for (int i = 0; i < 5; i++) {
        ClientData clientData = { .x = fakeMouseX[i], .y = fakeMouseY[i] };

        memcpy(packet->data, &clientData, sizeof(ClientData));
        packet->len = sizeof(ClientData);
        packet->address = serverAddress;

        SDLNet_UDP_Send(clientSocket, -1, packet);
        printf("Sent: x = %d, y = %d\n", clientData.x, clientData.y);

        // Wait for server response
        if (SDLNet_UDP_Recv(clientSocket, packet)) {
            ServerData serverData;
            memcpy(&serverData, packet->data, sizeof(ServerData));
            printf("Received from server: x = %d, y = %d\n", serverData.x, serverData.y);
        } else {
            printf("No response from server\n");
        }

        SDL_Delay(1000); // Wait 1 second before sending next position
    }

    SDLNet_FreePacket(packet);
    SDLNet_UDP_Close(clientSocket);
    SDLNet_Quit();
    SDL_Quit();

    return 0;
}
