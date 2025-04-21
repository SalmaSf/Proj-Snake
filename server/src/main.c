#include <SDL.h>
#include <SDL_image.h>
#include <SDL_net.h>
#include <stdbool.h>
#include <stdio.h>
#include "../include/snake.h"
#include "../include/bakgrund.h"
#include "../include/snake_data.h"

#define PORT 12345
#define BUFFER_SIZE 512

typedef struct
{
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Texture *background;
    Snake *snake;
    UDPsocket socket;
    UDPpacket *packet;
    IPaddress clientAddress;
    int windowWidth;
    int windowHeight;
    bool running;
} Game;

int initiate(Game *game);
void run(Game *game);
void close(Game *game);
void handlePacket(Game *game);
void sendGameData(Game *game);

int main()
{
    Game game = {0};
    game.windowWidth = 800;
    game.windowHeight = 600;

    if (!initiate(&game))
        return 1;
    run(&game);
    close(&game);

    return 0;
}

int initiate(Game *game)
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0)
    {
        printf("SDL_Init Error: %s\n", SDL_GetError());
        return 0;
    }

    if (IMG_Init(IMG_INIT_PNG) == 0)
    {
        printf("IMG_Init Error: %s\n", IMG_GetError());
        return 0;
    }

    if (SDLNet_Init() != 0)
    {
        printf("SDLNet_Init Error: %s\n", SDLNet_GetError());
        return 0;
    }

    game->window = SDL_CreateWindow("Snake Server", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                    game->windowWidth, game->windowHeight, 0);
    game->renderer = SDL_CreateRenderer(game->window, -1, SDL_RENDERER_ACCELERATED);

    game->background = loadBackground(game->renderer, "resources/bakgrund.png");
    game->snake = createSnake(game->windowWidth / 2, game->windowHeight / 2, game->renderer, game->windowWidth, game->windowHeight);

    game->socket = SDLNet_UDP_Open(PORT);
    if (!game->socket)
    {
        printf("SDLNet_UDP_Open Error: %s\n", SDLNet_GetError());
        return 0;
    }

    game->packet = SDLNet_AllocPacket(BUFFER_SIZE);
    if (!game->packet)
    {
        printf("SDLNet_AllocPacket Error: %s\n", SDLNet_GetError());
        return 0;
    }

    game->running = true;
    return 1;
}

void run(Game *game)
{
    SDL_Event event;

    while (game->running)
    {
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
                game->running = false;
        }

        if (SDLNet_UDP_Recv(game->socket, game->packet))
        {
            handlePacket(game);
        }

        updateSnake(game->snake);

        SDL_SetRenderDrawColor(game->renderer, 0, 0, 0, 255);
        SDL_RenderClear(game->renderer);

        SDL_RenderCopy(game->renderer, game->background, NULL, NULL);
        drawSnake(game->snake);
        SDL_RenderPresent(game->renderer);

        sendGameData(game);
        SDL_Delay(16); // ~60 FPS
    }
}

void handlePacket(Game *game)
{
    ClientData clientData;
    memcpy(&clientData, game->packet->data, sizeof(ClientData));

    /*game->snake->head->x = clientData.x;
    game->snake->head->y = clientData.y;*/

    setSnakePosition(game->snake, clientData.x, clientData.y);

    // Save client address to reply later
    game->clientAddress = game->packet->address;
    printf("Received position: x=%d y=%d\n", clientData.x, clientData.y);
}

void sendGameData(Game *game)
{
    /*ServerData serverData = {
        .x = (int)game->snake->head->x,
        .y = (int)game->snake->head->y};*/

    SDL_Rect data =
        {
            .x = (int)getSnakeX(game->snake),
            .y = (int)getSnakeY(game->snake)};

    ServerData serverData =
        {
            .x = data.x,
            .y = data.y};

    memcpy(game->packet->data, &serverData, sizeof(ServerData));
    game->packet->len = sizeof(ServerData);
    game->packet->address = game->clientAddress;

    SDLNet_UDP_Send(game->socket, -1, game->packet);
}

void close(Game *game)
{
    destroySnake(game->snake);
    SDL_DestroyRenderer(game->renderer);
    SDL_DestroyWindow(game->window);
    SDL_DestroyTexture(game->background);
    SDLNet_FreePacket(game->packet);
    SDLNet_UDP_Close(game->socket);
    SDLNet_Quit();
    IMG_Quit();
    SDL_Quit();
}
