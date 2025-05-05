#ifndef SNAKE_DATA_H
#define SNAKE_DATA_H

#define MAX_PLAYERS 4
#define MAX_SEGMENTS 100

typedef struct
{
    int x;
    int y;
} SnakeSegment;

typedef struct
{
    int playerIndex; // Unique ID assigned by server
    int segmentCount;
    int direction;
    int colorId;
    int isYou;  // NEW: server sets this
    int length; // Optional: could track number of segments in use
    SnakeSegment segments[MAX_SEGMENTS];

} SerializableSnake;

typedef SerializableSnake ClientData;

/*typedef struct {
    SerializableSnake snake;
} ClientData;
*/

typedef struct
{
    SerializableSnake snakes[MAX_PLAYERS];
    int numSnakes;
} ServerData;

#endif
