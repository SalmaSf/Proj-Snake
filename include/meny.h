#ifndef START_MENY_H
#define START_MENY_H 

#include <SDL.h>
#include <SDL_ttf.h>
#include <stdbool.h>
#include "snake.h"  


bool visaStartMeny(SDL_Renderer* renderer);
bool visaIPMeny(SDL_Renderer* renderer);
bool visaLobby(SDL_Renderer* renderer);
int visaResultatskärm(SDL_Renderer* renderer, bool vann, float tid);
void keepWatching(Snake* snake[], SDL_Renderer* renderer, SDL_Texture* background);


#endif

