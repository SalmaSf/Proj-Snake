#include "bakgrund.h"

SDL_Texture* loadBackground(SDL_Renderer* renderer, const char* filePath) 
{
    SDL_Surface* surface = IMG_Load(filePath);
    if (!surface) 
    {
        SDL_Log("Unable to load image %s! SDL_image Error: %s", filePath, IMG_GetError());
        return NULL;
    }

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);

    if (!texture) 
    {
        SDL_Log("Unable to create texture from %s! SDL Error: %s", filePath, SDL_GetError());
        return NULL;
    }

    return texture;
}
