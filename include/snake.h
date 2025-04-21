#ifndef SNAKE_H
#define SNAKE_H


#include <SDL.h>

typedef struct segment Segment;
typedef struct snake Snake;


Snake *createSnake(int x, int y, SDL_Renderer *pRenderer, int windowWidth, int windowHeight);
void updateSnake(Snake *pSnake);
void drawSnake(Snake *pSnake);
void destroySnake(Snake *pSnake);
void setSnakePosition(Snake *pSnake, float x, float y);
float getSnakeX(Snake *pSnake);
float getSnakeY(Snake *pSnake);

#endif
