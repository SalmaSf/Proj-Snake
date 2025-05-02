#ifndef SNAKE_H
#define SNAKE_H

#include <SDL.h>
#include <stdbool.h>

#define MAX_HISTORY 1000

extern float historyX[MAX_HISTORY];
extern float historyY[MAX_HISTORY];
extern int historyIndex;

typedef struct segment Segment;
typedef struct snake Snake;
typedef struct {
    bool win;
    float time;
} GameResult;

Snake *createSnake(int x, int y, SDL_Renderer *pRenderer, int window_width, int window_height,const char *headTexturePath, const char *segmentTexturePath);

void updateSnake(Snake *pSnake);
void drawSnake(Snake *pSnake);
void destroySnake(Snake *pSnake);
void addSegment(Snake *pSnake);
void updateSegments(Snake *pSnake);

bool checkCollision(Snake *attacker, Snake *target);
bool isSnakeAlive(Snake *snake);
void killSnake(Snake *snake);
GameResult gameLoop(Snake *snake[], SDL_Renderer *pRenderer, SDL_Texture *pBackground, int spelarIndex);
//void sendSnakePosition(int x, int y);
//void receiveServerUpdate();




#endif