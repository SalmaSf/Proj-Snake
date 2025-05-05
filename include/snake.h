#ifndef SNAKE_H
#define SNAKE_H

#include <SDL.h>
#include <SDL_net.h>
#include <stdbool.h>
#include "snake_data.h"

#define MAX_HISTORY 1000

extern float historyX[MAX_HISTORY];
extern float historyY[MAX_HISTORY];
extern int historyIndex;

typedef struct segment Segment;
typedef struct snake Snake;

#ifndef SERVER_MODE
Snake *createSnake(int x, int y, SDL_Renderer *pRenderer, int window_width, int window_height, const char *headTexturePath, const char *segmentTexturePath, int colorId);
void updateSnake(Snake *pSnake);
void drawSnake(Snake *pSnake);
void destroySnake(Snake *pSnake);
void addSegment(Snake *pSnake);
void updateSegments(Snake *pSnake);
void gameLoop(Snake *snake[], SDL_Renderer *pRenderer, SDL_Texture *pBackground, int localPlayerIndex);
bool checkCollision(Snake *attacker, Snake *target);
bool isSnakeAlive(Snake *snake);
void killSnake(Snake *snake);
void updateOrCreateSnakeFromSerializable(
    Snake *snakes[],
    SerializableSnake *ss,
    SDL_Renderer *pRenderer,
    int window_width,
    int window_height);

#endif

void serializableToSnake(SerializableSnake *in, Snake *pSnake);
void snakeToSerializable(Snake *pSnake, SerializableSnake *out);
void updateLocalSnakeAndSend(Snake *snakes[], int localPlayerIndex, UDPsocket udpSocket, UDPpacket *sendPacket, IPaddress serverAddress);

#endif
