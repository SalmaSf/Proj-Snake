#ifndef SNAKE_H
#define SNAKE_H

#include <SDL.h>

#define MAX_HISTORY 1000

extern float historyX[MAX_HISTORY];
extern float historyY[MAX_HISTORY];
extern int historyIndex;

typedef struct segment Segment;
typedef struct snake Snake;

Snake *createSnake(int x, int y, SDL_Renderer *pRenderer, int window_width, int window_height);

void updateSnake(Snake *pSnake);
void drawSnake(Snake *pSnake);
void destroySnake(Snake *pSnake);
void addSegment(Snake *pSnake);
void updateSegments(Snake *pSnake);

#endif
