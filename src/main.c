#include <stdio.h>
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <SDL_net.h>
#include <SDL_mixer.h>
#include <string.h>
#include <stdbool.h>

#include "snake.h"
#include "bakgrund.h"
#include "meny.h"
#include "snake_data.h"

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 1234

const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;

int main(int argc, char *argv[])
{
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
    TTF_Init();

    int imgFlags = IMG_INIT_PNG;
    if (!(IMG_Init(imgFlags) & imgFlags))
    {
        printf("SDL_image kunde inte initieras! SDL_image Error: %s\n", IMG_GetError());
        return 1;
    }

    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0)
    {
        SDL_Log("SDL_mixer init error: %s", Mix_GetError());
        return 1;
    }

    Mix_Music *music = Mix_LoadMUS("resources/bakgrund.wav");
    if (!music)
    {
        SDL_Log("Failed to load music: %s", Mix_GetError());
        return 1;
    }
    Mix_PlayMusic(music, -1);

    SDL_Window *pWindow = SDL_CreateWindow("Snake Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, 0);
    SDL_Renderer *pRenderer = SDL_CreateRenderer(pWindow, -1, SDL_RENDERER_ACCELERATED);

    if (SDLNet_Init() < 0)
    {
        SDL_Log("SDLNet_Init error: %s", SDLNet_GetError());
        return 1;
    }

    UDPsocket udpSocket = SDLNet_UDP_Open(0);
    UDPpacket *sendPacket = SDLNet_AllocPacket(sizeof(ClientData));
    UDPpacket *recvPacket = SDLNet_AllocPacket(1400);

    IPaddress serverAddress;
    if (SDLNet_ResolveHost(&serverAddress, SERVER_IP, SERVER_PORT) < 0)
    {
        SDL_Log("SDLNet_ResolveHost error: %s", SDLNet_GetError());
        return 1;
    }

    if (!visaStartMeny(pRenderer) || !visaIPMeny(pRenderer) || !visaLobby(pRenderer))
    {
        SDL_DestroyRenderer(pRenderer);
        SDL_DestroyWindow(pWindow);
        IMG_Quit();
        SDLNet_Quit();
        SDL_Quit();
        return 1;
    }

    SDL_Texture *pBackground = loadBackground(pRenderer, "resources/bakgrund.png");
    if (!pBackground)
        return 1;

    int localPlayerIndex = -1;
    Snake *snakes[MAX_PLAYERS] = {NULL};

    Mix_Chunk *collisionSound = Mix_LoadWAV("resources/snake_rattle.wav");
    if (!collisionSound)
    {
        SDL_Log("Failed to load collision sound: %s", Mix_GetError());
        return 1;
    }

    bool isRunning = true;
    SDL_Event event;

    while (isRunning)
    {
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT || (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE))
            {
                isRunning = false;
            }
        }

        updateLocalSnakeAndSend(snakes, localPlayerIndex, udpSocket, sendPacket, serverAddress); // salma

        // Receive from server
        if (SDLNet_UDP_Recv(udpSocket, recvPacket))
        {
            ServerData *serverData = (ServerData *)recvPacket->data;

            for (int i = 0; i < serverData->numSnakes; i++)
            {
                SerializableSnake *ss = &serverData->snakes[i];
                if (localPlayerIndex == -1 && ss->isYou == 1)
                {
                    localPlayerIndex = ss->playerIndex;
                    printf("Local player index set to %d\n", localPlayerIndex);
                }

                updateOrCreateSnakeFromSerializable(snakes, ss, pRenderer, WINDOW_WIDTH, WINDOW_HEIGHT);
                //if this doesn't print, I'm not receiving anyhting at all
                printf("Client received Snake %d: head at (%d, %d), segments: %d, colorId: %d\n",
                       ss->playerIndex,
                       ss->segments[0].x,
                       ss->segments[0].y,
                       ss->segmentCount,
                       ss->colorId);
            }
        }

        for (int i = 0; i < MAX_PLAYERS; i++)
        {
            for (int j = i + 1; j < MAX_PLAYERS; j++)
            {
                if (snakes[i] && snakes[j] && checkCollision(snakes[i], snakes[j]))
                {
                    Mix_PlayChannel(-1, collisionSound, 0);
                }
            }
        }

        SDL_SetRenderDrawColor(pRenderer, 0, 0, 0, 255);
        SDL_RenderClear(pRenderer);
        SDL_RenderCopy(pRenderer, pBackground, NULL, NULL);

        for (int i = 0; i < MAX_PLAYERS; i++)
        {
            if (snakes[i])
                drawSnake(snakes[i]);
        }

        SDL_RenderPresent(pRenderer);
        SDL_Delay(16);
    }

    for (int i = 0; i < MAX_PLAYERS; i++)
    {
        if (snakes[i])
            destroySnake(snakes[i]);
    }

    SDL_DestroyRenderer(pRenderer);
    SDL_DestroyWindow(pWindow);
    SDL_DestroyTexture(pBackground);
    Mix_FreeChunk(collisionSound);
    Mix_FreeMusic(music);
    Mix_CloseAudio();

    SDLNet_FreePacket(sendPacket);
    SDLNet_FreePacket(recvPacket);
    SDLNet_UDP_Close(udpSocket);
    SDLNet_Quit();
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();

    return 0;
}
