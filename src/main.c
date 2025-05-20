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

// #define SERVER_IP "127.0.0.1"
#define SERVER_PORT 2000

const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 700;

typedef struct
{
    bool win;
    float time;
} GameResult;

typedef struct
{
    SDL_Window *pWindow;
    SDL_Renderer *pRenderer;
    SDL_Texture *pBackground;

    Snake *snakes[MAX_PLAYERS];
    int playerIndex;
    bool playerIndexSet;

    bool soundOn;

    Mix_Music *music;
    Mix_Chunk *collisionSound;

    TTF_Font *font;

    GameState state;
    ServerData serverData; //testar

    UDPsocket udpSocket;
    IPaddress serverAddr;
    UDPpacket *packet;
} Game;

int initGame(Game *pGame);
void runGame(Game *pGame);
void cleanGame(Game *pGame);

int initSnakeClient(Game *pGame, const char *ipAddress);
void sendSnakePosition(Game *pGame, int x, int y);
void receiveServerUpdate(Game *pGame);
void closeSnakeClient(Game *pGame);
GameResult gameLoop(Snake *snakes[], SDL_Renderer *pRenderer, SDL_Texture  *pBackground, Game *pGame); //testar

int main(int argc, char *argv[])
{
    Game game = {0};

    if (!initGame(&game))
    {
        return 1;
    }
    game.soundOn = true;
    runGame(&game);
    cleanGame(&game);

    return 0;
}

int initGame(Game *pGame)
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0)
    {
        SDL_Log("SDL_Init failed: %s", SDL_GetError());
        return 0;
    }

    if (TTF_Init() < 0)
    {
        SDL_Log("TTF_Init failed: %s", TTF_GetError());
        return 0;
    }

    pGame->pWindow = SDL_CreateWindow("Snake Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, 0);
    if (!pGame->pWindow)
        return 0;

    pGame->pRenderer = SDL_CreateRenderer(pGame->pWindow, -1, SDL_RENDERER_ACCELERATED);
    if (!pGame->pRenderer)
        return 0;

    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0)
    {
        SDL_Log("Mix_OpenAudio failed: %s", Mix_GetError());
        return 0;
    }

    pGame->music = Mix_LoadMUS("resources/bakgrund.wav");
    if (!pGame->music)
        return 0;
    Mix_PlayMusic(pGame->music, -1);

    pGame->collisionSound = Mix_LoadWAV("resources/snake_rattle.wav");
    if (!pGame->collisionSound)
        return 0;

    pGame->pBackground = loadBackground(pGame->pRenderer, "resources/bakgrund.png");
    if (!pGame->pBackground)
        return 0;

    for (int i = 0; i < MAX_PLAYERS; i++)
    {
        pGame->snakes[i] = NULL;
    }

    pGame->playerIndex = 0;
    pGame->playerIndexSet = false;
    pGame->state = START;

    return 1;
}

void runGame(Game *pGame)
{
    bool programRunning = true;
    char ipBuffer[256];

    while (programRunning)
    {
        if (!showStartMenu(pGame->pRenderer, &pGame->soundOn))
            break;

        bool playAgain = true;
        while (playAgain)
        {
            // 1. IP-meny
            if (!showIPMenu(pGame->pRenderer, ipBuffer, sizeof(ipBuffer)))
                break;

            if (strlen(ipBuffer) > 0)
                printf("User entered IP: %s\n", ipBuffer);
            else
                printf("No IP address entered.\n");

            // 2. Initiera nätverk
            if (!initSnakeClient(pGame, ipBuffer))
            {
                SDL_Log("Network initialization failed.");
                break;
            }
            // Sätt alla snakes till NULL innan du börjar ta emot data från servern
            for (int i = 0; i < MAX_PLAYERS; i++)
            {
                pGame->snakes[i] = NULL;
            }
            // 3. Skicka "joined"
            const char *joinedMsg = "Joined";
            memcpy(pGame->packet->data, joinedMsg, strlen(joinedMsg) + 1);
            pGame->packet->len = strlen(joinedMsg) + 1;
            SDLNet_UDP_Send(pGame->udpSocket, -1, pGame->packet);

            // 4. Vänta i lobby
            bool inLobby = true;
            while (inLobby)
            {
                receiveServerUpdate(pGame); // Få state från server

                if (pGame->state == ONGOING)
                {
                    printf("[LOBBY] Game starting!\n");
                    inLobby = false;
                    break;
                }

                if (!showLobby(pGame->pRenderer)) // ESC eller avbryt
                {
                    playAgain = false;
                    inLobby = false;
                    break;
                }

                SDL_Delay(100); // För att spara CPU
            }

            // 5. Skapa ormar
            for (int i = 0; i < MAX_PLAYERS; i++)
            {
                if (pGame->snakes[i])
                    destroySnake(pGame->snakes[i]);
            }

            pGame->snakes[0] = createSnake(WINDOW_WIDTH / 2, 0, pGame->pRenderer, WINDOW_WIDTH, WINDOW_HEIGHT, "resources/purple_head.png", "resources/purple_body.png");
            pGame->snakes[1] = createSnake(WINDOW_WIDTH / 2, WINDOW_HEIGHT, pGame->pRenderer, WINDOW_WIDTH, WINDOW_HEIGHT, "resources/yellow_head.png", "resources/yellow_body.png");
            pGame->snakes[2] = createSnake(0, WINDOW_HEIGHT / 2, pGame->pRenderer, WINDOW_WIDTH, WINDOW_HEIGHT, "resources/green_head.png", "resources/green_body.png");
            pGame->snakes[3] = createSnake(WINDOW_WIDTH, WINDOW_HEIGHT / 2, pGame->pRenderer, WINDOW_WIDTH, WINDOW_HEIGHT, "resources/pink_head.png", "resources/pink_body.png");

            // 6. Starta spelet
            GameResult result = gameLoop(pGame->snakes, pGame->pRenderer, pGame->pBackground, pGame); //testar
            //GameResult result = gameLoop(pGame->snakes, pGame->pRenderer, pGame->pBackground, pGame->playerIndex, pGame);
            int val = showResult(pGame->pRenderer, result.win, result.time);
            if (val == 0)
                playAgain = false;
        }
    }
}

GameResult gameLoop(Snake *snakes[], SDL_Renderer *renderer, SDL_Texture  *background, Game *pGame)          //Testar    /* ← playerIndex tas från pGame   */
{
    int  myID = pGame->playerIndex;      /* vilken orm är jag?            */
    bool isRunning = true;
    SDL_Event ev;

    Uint64 startTime = SDL_GetTicks64();
    int     gameTime = -1;

    /* --- timer-text --- */
    TTF_Font  *font = TTF_OpenFont("resources/GamjaFlower-Regular.ttf", 24);
    SDL_Color  txtColor = {255,255,255,255};
    SDL_Texture *timerTex = NULL;
    SDL_Rect     timerRect;

    /* ------------------ huvud-loop ------------------ */
    while (isRunning)
    {
        /* 1. Ta emot senaste paketet från servern */
        receiveServerUpdate(pGame);           /* fyller pGame->serverData       */
        if (pGame->state == GAME_OVER)
            isRunning = false;

        /* 2. Event/quit                                 */
        while (SDL_PollEvent(&ev)) {
            if (ev.type == SDL_QUIT ||
               (ev.type == SDL_KEYDOWN && ev.key.keysym.sym == SDLK_ESCAPE))
                isRunning = false;
        }

        /* 3. Vänta tills min orm faktiskt finns         */
        if (!pGame->snakes[myID]) {
            SDL_Delay(50);
            continue;
        }

        /* 4. Läs musen EN gång                          */
        int mx, my;
        SDL_GetMouseState(&mx, &my);

        /* 5. Uppdatera alla levande ormar               */
        for (int i = 0; i < MAX_PLAYERS; ++i) {
            if (!snakes[i] || !isSnakeAlive(snakes[i])) continue;

            if (i == myID) {
                /* min egen orm → musposition            */
                updateSnake(snakes[i], true, 50, 50); // skippa myID
            } else {
                /* fjärr-orm → koordinater från servern  */
                SnakeInfo *si = &pGame->serverData.snakes[i];
                updateSnake(snakes[i], false, si->x, si->y);
                if (!si->alive) killSnake(snakes[i]);
            }
        }

        /* 6. Skicka MIN nya position till servern       */
        int headX = getSnakeHeadX(snakes[myID]);
        int headY = getSnakeHeadY(snakes[myID]);
        sendSnakePosition(pGame, headX, headY); //Skicka "get mouse state" Ulrika tips, skicka mus data och inte vart huvudet är
 
        /* 7. Uppdatera timer-text                       */
        int curTime = (SDL_GetTicks64() - startTime) / 1000;
        if (curTime > gameTime) {
            gameTime = curTime;
            if (timerTex) SDL_DestroyTexture(timerTex);

            char buf[32];
            sprintf(buf, "%02d:%02d", gameTime/60, gameTime%60);
            SDL_Surface *surf = TTF_RenderText_Solid(font, buf, txtColor);
            timerTex  = SDL_CreateTextureFromSurface(renderer, surf);
            timerRect = (SDL_Rect){10,10, surf->w, surf->h};
            SDL_FreeSurface(surf);
        }

        /* 8. Render                                     */
        SDL_SetRenderDrawColor(renderer, 0,0,0,255);
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, background, NULL, NULL);

        for (int i = 0; i < MAX_PLAYERS; ++i)
            if (snakes[i] && isSnakeAlive(snakes[i]))
                drawSnake(snakes[i]);

        if (timerTex)
            SDL_RenderCopy(renderer, timerTex, NULL, &timerRect);

        SDL_RenderPresent(renderer);
        SDL_Delay(16);                         /* ≈60 fps                       */
    }

    /* ---------- städa upp ---------- */
    if (timerTex) SDL_DestroyTexture(timerTex);
    TTF_CloseFont(font);

    return (GameResult){
        .win  = isSnakeAlive(snakes[myID]),
        .time = (float)gameTime
    };
}

/*GameResult gameLoop(Snake *snakes[], SDL_Renderer *renderer, SDL_Texture *background, int playerIndex, Game *pGame)
{
    bool isRunning = true;
    SDL_Event event;
    Uint64 startTime = SDL_GetTicks64();
    int gameTime = -1;

    TTF_Font *font = TTF_OpenFont("resources/GamjaFlower-Regular.ttf", 24);
    if (!font)
    {
        printf("Error loading font: %s\n", TTF_GetError());
        return (GameResult){false, 0.0f};
    }
    printf("Game loop started\n"); // Debugging här

    SDL_Color textColor = {255, 255, 255, 255};
    SDL_Texture *timerTexture = NULL;
    SDL_Rect timerRect;

    while (isRunning)
    {
        if (!pGame->snakes[playerIndex])
        {
            printf(" Waiting for your snake to be created...\n");
            SDL_Delay(50); // vänta på att servern skickar snakeinfo
            continue;      // hoppa över resten av loopen tills ormen finns
        }

        //  Om ormen finns och lever – skicka dess position
        if (isSnakeAlive(pGame->snakes[playerIndex]))
        {
            int x = getSnakeHeadX(pGame->snakes[playerIndex]);
            int y = getSnakeHeadY(pGame->snakes[playerIndex]);
            printf("Player %d snake position: (%d, %d)\n", playerIndex, x, y);
            sendSnakePosition(pGame, x, y);
        }

        receiveServerUpdate(pGame);

        if (pGame->state == GAME_OVER)
            isRunning = false;

        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT ||
                (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE))
                isRunning = false;
        }

        for (int i = 0; i < MAX_PLAYERS; i++)
        {
            if (isSnakeAlive(snakes[i]))
            {
                updateSnake(snakes[i]);
                // printf("Snake %d updated\n", i);  // Debugging här
            }
        }

        int currentTime = (SDL_GetTicks64() - startTime) / 1000;
        if (currentTime > gameTime)
        {
            gameTime = currentTime;

            if (timerTexture)
                SDL_DestroyTexture(timerTexture);

            char text[32];
            sprintf(text, "%02d:%02d", gameTime / 60, gameTime % 60);
            SDL_Surface *surface = TTF_RenderText_Solid(font, text, textColor);
            timerTexture = SDL_CreateTextureFromSurface(renderer, surface);
            timerRect.x = 10;
            timerRect.y = 10;
            timerRect.w = surface->w;
            timerRect.h = surface->h;
            SDL_FreeSurface(surface);
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, background, NULL, NULL);

        for (int i = 0; i < MAX_PLAYERS; i++)
        {
            if (isSnakeAlive(snakes[i]))
            {
                drawSnake(snakes[i]);
            }
        }

        if (timerTexture)
        {
            SDL_RenderCopy(renderer, timerTexture, NULL, &timerRect);
        }

        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }

    if (timerTexture)
        SDL_DestroyTexture(timerTexture);
    TTF_CloseFont(font);

    printf("[GAME] Game over. Waiting for Server results...\n");

    return (GameResult){
        .win = isSnakeAlive(snakes[playerIndex]),
        .time = (float)gameTime};
}*/

int initSnakeClient(Game *pGame, const char *ipAddress)
{
    if (SDLNet_Init() < 0)
    {
        SDL_Log("SDLNet_Init: %s\n", SDLNet_GetError());
        return 0;
    }

    pGame->udpSocket = SDLNet_UDP_Open(0);
    if (!pGame->udpSocket)
    {
        SDL_Log("SDLNet_UDP_Open: %s\n", SDLNet_GetError());
        return 0;
    }

    if (SDLNet_ResolveHost(&pGame->serverAddr, ipAddress, SERVER_PORT) < 0)
    {
        SDL_Log("SDLNet_ResolveHost: %s\n", SDLNet_GetError());
        return 0;
    }
    printf("Successfully connected to server %s:%d\n", ipAddress, SERVER_PORT); // Debugging här

    pGame->packet = SDLNet_AllocPacket(512);
    if (!pGame->packet)
    {
        SDL_Log("SDLNet_AllocPacket: %s\n", SDLNet_GetError());
        return 0;
    }

    // Skicka JOIN
    const char *joinMsg = "JOIN";
    memcpy(pGame->packet->data, joinMsg, strlen(joinMsg) + 1);
    pGame->packet->address = pGame->serverAddr;
    pGame->packet->len = strlen(joinMsg) + 1;
    SDLNet_UDP_Send(pGame->udpSocket, -1, pGame->packet);

    // Vänta på clientID
    Uint32 start = SDL_GetTicks();
    while (SDL_GetTicks() - start < 5000)
    {
        if (SDLNet_UDP_Recv(pGame->udpSocket, pGame->packet))
        {
            int clientID;
            memcpy(&clientID, pGame->packet->data, sizeof(int));
            pGame->playerIndex = clientID;
            pGame->playerIndexSet = true;
            printf(" My clientID received from the server: %d\n", clientID);
            break;
        }
    }

    if (!pGame->playerIndexSet)
    {

        printf(" Could not accept ClientID from server.\n");
        return 0;
    }
    // pGame->packet->address.host = pGame->serverAddr.host;
    // pGame->packet->address.port = pGame->serverAddr.port;

    return 1;
}

void sendSnakePosition(Game *pGame, int x, int y)
{
    if (pGame->state != ONGOING)
        return; // Skicka inget om spelet inte är igång

    ClientData data = {0};
    data.x = x;
    data.y = y;
    data.clientID = pGame->playerIndex;
    if (pGame->snakes[pGame->playerIndex])
        data.alive = isSnakeAlive(pGame->snakes[pGame->playerIndex]);
    else
        data.alive = true; // weird
    data.state = pGame->state;

    memcpy(pGame->packet->data, &data, sizeof(ClientData));
    pGame->packet->len = sizeof(ClientData);
    SDLNet_UDP_Send(pGame->udpSocket, -1, pGame->packet);
}

void receiveServerUpdate(Game *pGame)
{
    if (SDLNet_UDP_Recv(pGame->udpSocket, pGame->packet))
    {
        printf(" Packet from server (len = %d)\n", pGame->packet->len);

        ServerData serverData;
        memcpy(&serverData, pGame->packet->data, sizeof(ServerData));
        pGame->serverData = serverData;   
        printf(" ServerData.numPlayers = %d\n", serverData.numPlayers);

        pGame->state = serverData.state;
        printf("Received server data: state = %d, num players = %d\n", pGame->state, serverData.numPlayers);

        if (!pGame->playerIndexSet)
        {
            // Sätt korrekt playerIndex (inte alltid 0!)
            for (int i = 0; i < serverData.numPlayers; i++)
            {
                if (serverData.snakes[i].x != 0 || serverData.snakes[i].y != 0)
                {
                    pGame->playerIndex = i;
                    pGame->playerIndexSet = true;
                    printf("Client index set: %d\n", i);
                    break;
                }
            }
        }
        printf("Received server data: state = %d, num players = %d\n", pGame->state, serverData.numPlayers); // Debugging här
        printf("Num players in serverData: %d\n", serverData.numPlayers);
        /*for (int i = 0; i < serverData.numPlayers; i++)
        {
            SnakeInfo *s = &serverData.snakes[i];
            setSnakePosition(pGame->snakes[s->clientID], s->x, s->y);
            if (!s->alive)
                killSnake(pGame->snakes[s->clientID]);
        }*/

        for (int i = 0; i < serverData.numPlayers; i++)
        {
            SnakeInfo *s = &serverData.snakes[i];
            printf(" Updating snake %d: position (%d, %d), alive: %d\n", s->clientID, s->x, s->y, s->alive);

            //  Skapa ormen om den inte redan finns och är alive
            /*if (s->alive && pGame->snakes[s->clientID] == NULL)
            {
                const char *head = "resources/purple_head.png";
                const char *body = "resources/purple_body.png";
                // Olika färger för olika ormar
                switch (s->clientID)
                {
                case 0:
                    head = "resources/purple_head.png";
                    body = "resources/purple_body.png";
                    break;
                case 1:
                    head = "resources/yellow_head.png";
                    body = "resources/yellow_body.png";
                    break;
                case 2:
                    head = "resources/green_head.png";
                    body = "resources/green_body.png";
                    break;
                case 3:
                    head = "resources/pink_head.png";
                    body = "resources/pink_body.png";
                    break;
                }

                pGame->snakes[s->clientID] = createSnake(s->x, s->y, pGame->pRenderer, 800, 700, head, body);
                printf("Created snake %d on (%d, %d)\n", s->clientID, s->x, s->y);
            }*/

            //  Kontroll så vi inte försöker uppdatera NULL
            /*if (pGame->snakes[s->clientID])
            {
                if (s->alive)
                    setSnakePosition(pGame->snakes[s->clientID], s->x, s->y);
                else
                    killSnake(pGame->snakes[s->clientID]);
            }
            else
            {
                printf(" pGame->snakes[%d] is NULL, can not update position.\n", s->clientID);
            }*/
        }
    }
}

void closeSnakeClient(Game *pGame)
{
    SDLNet_FreePacket(pGame->packet);
    SDLNet_UDP_Close(pGame->udpSocket);
    SDLNet_Quit();
}

void cleanGame(Game *pGame)
{
    for (int i = 0; i < MAX_PLAYERS; i++)
    {
        if (pGame->snakes[i])
        {
            destroySnake(pGame->snakes[i]);
            pGame->snakes[i] = NULL;
        }
    }

    SDL_DestroyTexture(pGame->pBackground);
    SDL_DestroyRenderer(pGame->pRenderer);
    SDL_DestroyWindow(pGame->pWindow);
    Mix_FreeChunk(pGame->collisionSound);
    Mix_FreeMusic(pGame->music);
    Mix_CloseAudio();
    closeSnakeClient(pGame);
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
}