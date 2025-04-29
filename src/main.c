// Fil: main.c
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <SDL_net.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>

#include "snake.h"
#include "bakgrund.h"
#include "meny.h"

#define SERVER_IP "192.168.1.138"
#define SERVER_PORT 12345

const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 700;

static UDPsocket udpSocket;
static IPaddress serverAddr;
static UDPpacket *packet;

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

void closeSnakeClient()
{
    SDLNet_FreePacket(packet);
    SDLNet_UDP_Close(udpSocket);
    SDLNet_Quit();
}

int main(int argc, char *argv[])
{
    SDL_Init(SDL_INIT_VIDEO);
    TTF_Init();
    int imgFlags = IMG_INIT_PNG;
    if (!(IMG_Init(imgFlags) & imgFlags))
    {
        printf("SDL_image kunde inte initieras! SDL_image Error: %s\n", IMG_GetError());
        return 1;
    }

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

    if (!visaStartMeny(pRenderer)) {
        SDL_DestroyRenderer(pRenderer);
        SDL_DestroyWindow(pWindow);
        IMG_Quit();
        SDL_Quit();
        return 1;
    }

    if (!visaIPMeny(pRenderer)) {
        SDL_DestroyRenderer(pRenderer);
        SDL_DestroyWindow(pWindow);
        IMG_Quit();
        SDL_Quit();
        return 1;
    }

    if (!visaLobby(pRenderer)) {
        SDL_DestroyRenderer(pRenderer);
        SDL_DestroyWindow(pWindow);
        IMG_Quit();
        SDL_Quit();
        return 1;
    }

    SDL_Texture *pBackground = loadBackground(pRenderer, "resources/bakgrund.png");
    if (!pBackground)
        return 1;

    Snake *snake[4];
    snake[0] = createSnake(WINDOW_WIDTH / 2, 0, pRenderer, WINDOW_WIDTH, WINDOW_HEIGHT);
    snake[1] = createSnake(WINDOW_WIDTH / 2, WINDOW_HEIGHT, pRenderer, WINDOW_WIDTH, WINDOW_HEIGHT);
    snake[2] = createSnake(0, WINDOW_HEIGHT / 2, pRenderer, WINDOW_WIDTH, WINDOW_HEIGHT);
    snake[3] = createSnake(WINDOW_WIDTH, WINDOW_HEIGHT / 2, pRenderer, WINDOW_WIDTH, WINDOW_HEIGHT);

    gameLoop(snake, pRenderer, pBackground);

    for (int i = 0; i < 4; i++)
    {
        destroySnake(snake[i]);
    }

    SDL_DestroyRenderer(pRenderer);
    SDL_DestroyWindow(pWindow);
    SDL_DestroyTexture(pBackground);
    closeSnakeClient();
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();

    return 0;
}
