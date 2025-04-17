#ifndef SNAKE_H
#define SNAKE_H

#include <SDL.h>

typedef struct segment Segment;
typedef struct snake Snake;

Snake *createSnake(int x, int y, SDL_Renderer *pRenderer, int window_width, int window_height);

void updateSnake(Snake *pSnake);
void drawSnake(Snake *pSnake);
void destroySnake(Snake *pSnake);

#endif