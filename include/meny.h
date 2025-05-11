#ifndef START_MENY_H
#define START_MENY_H 

#include <SDL.h>
#include <SDL_ttf.h>
#include <stdbool.h>
#include "snake.h"  


bool visaStartMeny(SDL_Renderer* renderer, bool* ljudPa);
bool visaIPMeny(SDL_Renderer* renderer, char* ipBuffer, int bufferSize);
bool visaLobby(SDL_Renderer* renderer);
int visaResultatskarm(SDL_Renderer* renderer, bool vann, float tid);
void keepWatching(Snake* snake[], SDL_Renderer* renderer, SDL_Texture* background);
void visaInstruktionsskarm(SDL_Renderer* renderer);



#endif

