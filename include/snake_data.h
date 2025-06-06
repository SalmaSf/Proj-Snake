#ifndef SNAKE_DATA_H
#define SNAKE_DATA_H

#define MAX_PLAYERS 4

typedef enum
{
    START,
    ONGOING,
    GAME_OVER
} GameState;

typedef struct
{
    int clientID;
    int x;
    int y;
    bool alive;
    GameState state;
} ClientData;

typedef struct
{
    int clientID;
    int x;
    int y;
    bool alive;
} SnakeInfo;

typedef struct
{
    SnakeInfo snakes[MAX_PLAYERS];
    int numPlayers;
    GameState state;
} ServerData;

#endif
