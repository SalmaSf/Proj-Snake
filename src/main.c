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

#define SERVER_IP "130.229.182.107"
#define SERVER_PORT 12345

const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 700;

static UDPsocket udpSocket;
static IPaddress serverAddr;
static UDPpacket *packet;

int initSnakeClient(void);
void sendSnakePosition(int x, int y);
void receiveServerUpdate(void);
void closeSnakeClient(void);

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

    Mix_PlayMusic(music, -1); // -1 = loopa musiken

    SDL_Window *pWindow = SDL_CreateWindow("Snake Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, 0);
    SDL_Renderer *pRenderer = SDL_CreateRenderer(pWindow, -1, SDL_RENDERER_ACCELERATED);

    if (!initSnakeClient())
    {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Network Error", "Failed to initialize network connection.", pWindow);
        SDL_DestroyRenderer(pRenderer);
        SDL_DestroyWindow(pWindow);
        IMG_Quit();
        SDLNet_Quit();
        SDL_Quit();
        return 1;
    }

    if (!visaStartMeny(pRenderer))
    {
        SDL_DestroyRenderer(pRenderer);
        SDL_DestroyWindow(pWindow);
        IMG_Quit();
        SDL_Quit();
        return 1;
    }

    if (!visaIPMeny(pRenderer))
    {
        SDL_DestroyRenderer(pRenderer);
        SDL_DestroyWindow(pWindow);
        IMG_Quit();
        SDL_Quit();
        return 1;
    }

    if (!visaLobby(pRenderer))
    {
        SDL_DestroyRenderer(pRenderer);
        SDL_DestroyWindow(pWindow);
        IMG_Quit();
        SDL_Quit();
        return 1;
    }

    SDL_Texture *pBackground = loadBackground(pRenderer, "resources/bakgrund.png");
    if (!pBackground)
        return 1;

    const char *headTexturePath = "resources/default_head.png";
    const char *segmentTexturePath = "resources/default_segment.png";

    Snake *pSnake = createSnake(WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2, pRenderer, WINDOW_WIDTH, WINDOW_HEIGHT, headTexturePath, segmentTexturePath);

    Snake *snake[4];
    snake[0] = createSnake(WINDOW_WIDTH / 2, 0, pRenderer, WINDOW_WIDTH, WINDOW_HEIGHT, "resources/purple_head.png", "resources/purple_body.png");
    snake[1] = createSnake(WINDOW_WIDTH / 2, WINDOW_HEIGHT, pRenderer, WINDOW_WIDTH, WINDOW_HEIGHT, "resources/yellow_head.png", "resources/yellow_body.png");
    snake[2] = createSnake(0, WINDOW_HEIGHT / 2, pRenderer, WINDOW_WIDTH, WINDOW_HEIGHT, "resources/green_head.png", "resources/green_body.png");
    snake[3] = createSnake(WINDOW_WIDTH, WINDOW_HEIGHT / 2, pRenderer, WINDOW_WIDTH, WINDOW_HEIGHT, "resources/pink_head.png", "resources/pink_body.png");

    Mix_Chunk *collisionSound = Mix_LoadWAV("resources/snake_rattle.wav");
    if (!collisionSound)
    {
        SDL_Log("Failed to load collision sound: %s", Mix_GetError());
        return 1;
    }

    gameLoop(snake, pRenderer, pBackground);

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

        for (int i = 0; i < 4; i++)
        {
            for (int j = i + 1; j < 4; j++)
            {
                if (checkCollision(snake[i], snake[j]))
                {
                    Mix_PlayChannel(-1, collisionSound, 0);
                }
            }
        }

        int mouseX, mouseY;
        SDL_GetMouseState(&mouseX, &mouseY);
        sendSnakePosition(mouseX, mouseY);
        receiveServerUpdate();

        updateSnake(pSnake);

        SDL_SetRenderDrawColor(pRenderer, 0, 0, 0, 255);
        SDL_RenderClear(pRenderer);

        SDL_RenderCopy(pRenderer, pBackground, NULL, NULL);

        drawSnake(pSnake);

        SDL_RenderPresent(pRenderer);
        SDL_Delay(16); // ~60 FPS
    }

    for (int i = 0; i < 4; i++)
    {
        destroySnake(snake[i]);
    }

    SDL_DestroyRenderer(pRenderer);
    SDL_DestroyWindow(pWindow);
    SDL_DestroyTexture(pBackground);

    Mix_FreeChunk(collisionSound);
    Mix_FreeMusic(music);
    Mix_CloseAudio();

    closeSnakeClient();
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();

    return 0;
}

int initSnakeClient()
{
    if (SDLNet_Init() < 0)
    {
        SDL_Log("SDLNet_Init: %s\n", SDLNet_GetError());
        return 0;
    }

    udpSocket = SDLNet_UDP_Open(0);
    if (!udpSocket)
    {
        SDL_Log("SDLNet_UDP_Open: %s\n", SDLNet_GetError());
        return 0;
    }

    if (SDLNet_ResolveHost(&serverAddr, SERVER_IP, SERVER_PORT) < 0)
    {
        SDL_Log("SDLNet_ResolveHost: %s\n", SDLNet_GetError());
        return 0;
    }

    packet = SDLNet_AllocPacket(512);
    if (!packet)
    {
        SDL_Log("SDLNet_AllocPacket: %s\n", SDLNet_GetError());
        return 0;
    }

    packet->address.host = serverAddr.host;
    packet->address.port = serverAddr.port;

    return 1;
}

void sendSnakePosition(int x, int y)
{
    struct
    {
        int x, y;
    } SnakeData;
    SnakeData.x = x;
    SnakeData.y = y;
    memcpy(packet->data, &SnakeData, sizeof(SnakeData));
    packet->len = sizeof(SnakeData);
    SDLNet_UDP_Send(udpSocket, -1, packet);
}

void receiveServerUpdate()
{
    if (SDLNet_UDP_Recv(udpSocket, packet))
    {
        struct
        {
            int x, y;
        } serverData;
        memcpy(&serverData, packet->data, sizeof(serverData));
        printf("Received from server: x=%d y=%d\n", serverData.x, serverData.y);
    }
}

void closeSnakeClient()
{
    SDLNet_FreePacket(packet);
    SDLNet_UDP_Close(udpSocket);
    SDLNet_Quit();
}