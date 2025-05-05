#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <SDL_net.h>
#include "snake_data.h"
#include "server.h"
#define MAX_PLAYERS 4
#define SERVER_PORT 1234
#define PACKET_SIZE 1400
#define UPDATE_INTERVAL_MS 33

// Simple fixed spawn positions (non-overlapping)
int spawnX[MAX_PLAYERS] = {100, 600, 100, 600};
int spawnY[MAX_PLAYERS] = {100, 100, 400, 400};

int main(int argc, char *argv[])
{
    if (SDLNet_Init() < 0)
    {
        fprintf(stderr, "SDLNet_Init failed: %s\n", SDLNet_GetError());
        return EXIT_FAILURE;
    }

    UDPsocket socket = SDLNet_UDP_Open(SERVER_PORT);
    if (!socket)
    {
        fprintf(stderr, "SDLNet_UDP_Open failed: %s\n", SDLNet_GetError());
        SDLNet_Quit();
        return EXIT_FAILURE;
    }

    UDPpacket *recvPacket = SDLNet_AllocPacket(PACKET_SIZE);
    UDPpacket *sendPacket = SDLNet_AllocPacket(PACKET_SIZE);
    if (!recvPacket || !sendPacket)
    {
        fprintf(stderr, "Packet allocation failed\n");
        SDLNet_Quit();
        return EXIT_FAILURE;
    }

    SerializableSnake allSnakes[MAX_PLAYERS];
    IPaddress clientAddrs[MAX_PLAYERS];
    int numConnected = 0;
    Uint32 lastSendTime = SDL_GetTicks();

    printf("Server running on port %d...\n", SERVER_PORT);

    while (1)
    {
        // Receive packets
        while (SDLNet_UDP_Recv(socket, recvPacket))
        {
            if (recvPacket->len < sizeof(SerializableSnake))
                continue;

            SerializableSnake *recvSnake = (SerializableSnake *)recvPacket->data;

            IPaddress senderAddr = recvPacket->address;

            // Match or add client
            int clientIndex = -1;
            for (int i = 0; i < numConnected; i++)
            {
                if (clientAddrs[i].host == senderAddr.host &&
                    clientAddrs[i].port == senderAddr.port)
                {
                    clientIndex = i;
                    break;
                }
            }
            // new client
            if (clientIndex == -1 && numConnected < MAX_PLAYERS)
            {
                clientIndex = numConnected;
                clientAddrs[clientIndex] = senderAddr;

                // Assign initial snake state
                allSnakes[clientIndex] = *recvSnake; // copy data client sent (optional)
                allSnakes[clientIndex].playerIndex = clientIndex;

                allSnakes[clientIndex].segmentCount = 1;
                allSnakes[clientIndex].segments[0].x = spawnX[clientIndex];
                allSnakes[clientIndex].segments[0].y = spawnY[clientIndex];
                allSnakes[clientIndex].direction = 1;         // Example: 0 = up, 1 = right, 2 = down, 3 = left
                allSnakes[clientIndex].colorId = clientIndex; // 0, 1, 2, 3
                allSnakes[clientIndex].length = 1;

                numConnected++;
                printf("New client %d connected.\n", clientIndex);
            }

            if (clientIndex != -1)
            {
                // Only update movement/length — keep server-assigned info intact
                allSnakes[clientIndex].direction = recvSnake->direction;
                allSnakes[clientIndex].length = recvSnake->length;

                printf("Server: got snake from client %d, pos (%d,%d), dir %d, len %d\n",
                       recvSnake->playerIndex,
                       recvSnake->segments[0].x,
                       recvSnake->segments[0].y,
                       recvSnake->direction,
                       recvSnake->length);
            }
        }

        // Broadcast update to all clients
        Uint32 now = SDL_GetTicks();
        if (now - lastSendTime >= UPDATE_INTERVAL_MS)
        {
            struct
            {
                int numSnakes;
                SerializableSnake snakes[MAX_PLAYERS];
            } SnakeUpdate;
            SnakeUpdate.numSnakes = numConnected; // was inside the for loop below
            for (int i = 0; i < numConnected; i++)
            {

                // Copy and tag each snake with isYou flag
                for (int j = 0; j < numConnected; j++)
                {
                    SnakeUpdate.snakes[j] = allSnakes[j];
                    SnakeUpdate.snakes[j].isYou = (i == j) ? 1 : 0; // Mark the recipient's snake
                }
                printf("Sending to client %d:\n", i);
                for (int j = 0; j < numConnected; j++)
                {
                    SerializableSnake *s = &SnakeUpdate.snakes[j];
                    printf("  Snake %d: head=(%d, %d), segments=%d, colorId=%d, isYou=%d\n",
                           s->playerIndex,
                           s->segments[0].x,
                           s->segments[0].y,
                           s->segmentCount,
                           s->colorId,
                           s->isYou);
                }

                memcpy(sendPacket->data, &SnakeUpdate, sizeof(SnakeUpdate));
                sendPacket->len = sizeof(SnakeUpdate);
                sendPacket->address = clientAddrs[i];
                SDLNet_UDP_Send(socket, -1, sendPacket);
            }

            lastSendTime = now;
        }

        SDL_Delay(1);

        /*if (clientIndex != -1)
        {
            // Only update movement/length — keep server-assigned info intact
            allSnakes[clientIndex].direction = recvSnake->direction;
            allSnakes[clientIndex].length = recvSnake->length;
        }*/
    }
    SDLNet_FreePacket(recvPacket);
    SDLNet_FreePacket(sendPacket);
    SDLNet_Quit();
    return EXIT_SUCCESS;
}
