#ifndef SNAKE_DATA_H
#define SNAKE_DATA_H

#include <stdbool.h>

#define MAX_PLAYERS 4

// Klienten skickar sin position till servern
typedef struct {
    int x;
    int y;
    bool isAlive;
    int clientID;
} ClientData;

// Servern skickar tillbaka info om alla spelare
typedef struct {
    int x[MAX_PLAYERS];         // Huvudets x-position för varje orm
    int y[MAX_PLAYERS];         // Huvudets y-position för varje orm
    bool isAlive[MAX_PLAYERS];  // Lever orm i index i?
    int clientID[MAX_PLAYERS];  // Identifierar varje spelare
    int numSnakes; // Antal aktiva ormar
    int myClientID;
} ServerData;

#endif
