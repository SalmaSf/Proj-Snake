#ifndef SNAKE_DATA_H
#define SNAKE_DATA_H

#define MAX_PLAYERS 4

typedef struct
{
    int x;
    int y;
} SnakeData;


typedef struct
{
    int x;
    int y;
} ClientData;


typedef struct
{
    SnakeData snakes[MAX_PLAYERS]; 
    int numSnakes;                 
} ServerData;

#endif
