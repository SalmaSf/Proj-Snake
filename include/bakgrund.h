#ifndef BAKGRUND_H
#define BAKGRUND_H

#include <SDL.h>
#include <SDL_image.h>

SDL_Texture* loadBackground(SDL_Renderer* renderer, const char* filePath);

#endif