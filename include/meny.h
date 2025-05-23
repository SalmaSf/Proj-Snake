#ifndef START_MENY_H
#define START_MENY_H 

#include <SDL.h>
#include <SDL_ttf.h>
#include <stdbool.h>
#include "snake.h"  


bool showStartMenu(SDL_Renderer* renderer, bool* ljudPa);
bool showIPMenu(SDL_Renderer* renderer, char* ipBuffer, int bufferSize);
bool showLobby(SDL_Renderer *renderer, int numPlayers);
int showResult(SDL_Renderer* renderer, bool won, float time); 
void showInstructions(SDL_Renderer* renderer);



#endif

