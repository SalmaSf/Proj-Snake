#include <SDL.h>
#include <SDL_image.h>
<<<<<<< HEAD
#include <SDL_ttf.h>  // För text
=======
#include <SDL_net.h>
#include <string.h>
>>>>>>> origin
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

int main(int argc, char *argv[])
{
    SDL_Init(SDL_INIT_VIDEO);
<<<<<<< HEAD
    IMG_Init(IMG_INIT_PNG);
    TTF_Init(); 

    SDL_Window *pWindow = SDL_CreateWindow("Snake Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, 0);
    SDL_Renderer *pRenderer = SDL_CreateRenderer(pWindow, -1, SDL_RENDERER_ACCELERATED);

    SDL_Texture* pBackground = loadBackground(pRenderer, "resources/bakgrund.png");
    if (!pBackground) return 1;
=======
    // IMG_Init(IMG_INIT_PNG);
    int imgFlags = IMG_INIT_PNG;
    if (!(IMG_Init(imgFlags) & imgFlags))
    {
        printf("SDL_image kunde inte initieras! SDL_image Error: %s\n", IMG_GetError());
        return 1;
    }
    else
    {
        printf("SDL_image PNG-stöd initierat korrekt!\n");
    }

    SDL_Window *pWindow = SDL_CreateWindow("Snake Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, 0);
    SDL_Renderer *pRenderer = SDL_CreateRenderer(pWindow, -1, SDL_RENDERER_ACCELERATED);
>>>>>>> origin

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
        // Visa startmeny först
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

    // Starta spelet efter IP-inmatning
    SDL_Texture *pBackground = loadBackground(pRenderer, "resources/bakgrund.png");
    if (!pBackground)
        return 1;

    Snake *pSnake = createSnake(WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2, pRenderer, WINDOW_WIDTH, WINDOW_HEIGHT);
    Snake *snake[4];
    snake[0] = createSnake(WINDOW_WIDTH / 2, 0, pRenderer, WINDOW_WIDTH, WINDOW_HEIGHT);             // Topp mitten
    snake[1] = createSnake(WINDOW_WIDTH / 2, WINDOW_HEIGHT, pRenderer, WINDOW_WIDTH, WINDOW_HEIGHT); // Botten mitten
    snake[2] = createSnake(0, WINDOW_HEIGHT / 2, pRenderer, WINDOW_WIDTH, WINDOW_HEIGHT);            // Vänster mitten
    snake[3] = createSnake(WINDOW_WIDTH, WINDOW_HEIGHT / 2, pRenderer, WINDOW_WIDTH, WINDOW_HEIGHT); // Höger mitten

    gameLoop(snake, pRenderer, pBackground);

   
    Uint64 startTime = SDL_GetTicks64();  
    int gameTime = -1;                    
    TTF_Font* font = TTF_OpenFont("GamjaFlower-Regular.ttf", 24); // EGEN FONT här!
    if (!font)
    {
        printf("Error loading font: %s\n", TTF_GetError());
        return 1;
    }

    SDL_Color textColor = {255, 255, 255, 255}; 
    SDL_Texture* pTimerTexture = NULL;           
    SDL_Rect timerRect;                          
 

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

        updateSnake(pSnake);

       
        int currentTime = (SDL_GetTicks64() - startTime) / 1000;
        if (currentTime > gameTime)
        {
            gameTime = currentTime;
            if (pTimerTexture) SDL_DestroyTexture(pTimerTexture); 

            char timerText[32];
            int minutes = gameTime / 60;
            int seconds = gameTime % 60;
            sprintf(timerText, "%02d:%02d", minutes, seconds);

            SDL_Surface* pSurface = TTF_RenderText_Solid(font, timerText, textColor);
            pTimerTexture = SDL_CreateTextureFromSurface(pRenderer, pSurface);

            timerRect.x = 10;
            timerRect.y = 10;
            timerRect.w = pSurface->w;
            timerRect.h = pSurface->h;

            SDL_FreeSurface(pSurface);
        }

        SDL_SetRenderDrawColor(pRenderer, 0, 0, 0, 255);
        SDL_RenderClear(pRenderer);

        SDL_RenderCopy(pRenderer, pBackground, NULL, NULL);
        drawSnake(pSnake);
<<<<<<< HEAD

        if (pTimerTexture) SDL_RenderCopy(pRenderer, pTimerTexture, NULL, &timerRect); 

=======
>>>>>>> origin
        SDL_RenderPresent(pRenderer);
        SDL_Delay(16);
    }

<<<<<<< HEAD

    if (pTimerTexture) SDL_DestroyTexture(pTimerTexture);
    TTF_CloseFont(font);
    TTF_Quit();
    destroySnake(pSnake);
=======
    // destroySnake(pSnake);
    for (int i = 0; i < 4; i++)
    {
        destroySnake(snake[i]);
    }

>>>>>>> origin
    SDL_DestroyRenderer(pRenderer);
    SDL_DestroyWindow(pWindow);
    SDL_DestroyTexture(pBackground);
    IMG_Quit();
    SDL_Quit();

    return 0;
}
