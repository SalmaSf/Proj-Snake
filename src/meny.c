#include <stdlib.h>
#include <string.h>
#include <SDL.h>
#include <SDL_image.h>
#include <stdbool.h>
#include "meny.h"


bool visaStartMeny(SDL_Renderer* renderer)
{
    SDL_Texture* bakgrund = IMG_LoadTexture(renderer, "resources/meny_bakgrund.png");
    SDL_Texture* startKnapp = IMG_LoadTexture(renderer, "resources/start_knapp.png");

    if (!bakgrund || !startKnapp) {
        SDL_Log("Kunde inte ladda menybilder: %s", IMG_GetError());
        if (bakgrund) SDL_DestroyTexture(bakgrund);
        if (startKnapp) SDL_DestroyTexture(startKnapp);
        return false;
    }

    SDL_Rect knappRect = { /*300, 400, 200, 80*/ 260, 390, 280, 140 }; // normal position
    SDL_Rect knappVisuellRect = knappRect;     // visuell position (för animation)
    bool isPressed = false;
    bool iMeny = true;
    SDL_Event event;

    while (iMeny)
    {
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT) {
                SDL_DestroyTexture(bakgrund);
                SDL_DestroyTexture(startKnapp);
                return false;
            }

            if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
                int mx = event.button.x;
                int my = event.button.y;

                if (mx >= knappRect.x && mx <= knappRect.x + knappRect.w &&
                    my >= knappRect.y && my <= knappRect.y + knappRect.h) {
                    isPressed = true;
                    knappVisuellRect.y += 4; // tryck-effekt: rör knappen nedåt
                }
            }

            if (event.type == SDL_MOUSEBUTTONUP && event.button.button == SDL_BUTTON_LEFT) {
                int mx = event.button.x;
                int my = event.button.y;

                if (isPressed &&
                    mx >= knappRect.x && mx <= knappRect.x + knappRect.w &&
                    my >= knappRect.y && my <= knappRect.y + knappRect.h) {
                    iMeny = false; // släppte musen på knappen → starta spelet
                }

                // oavsett var släppet skedde, nollställ tryck-effekt
                isPressed = false;
                knappVisuellRect = knappRect;
            }
        }

        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, bakgrund, NULL, NULL);
        SDL_RenderCopy(renderer, startKnapp, NULL, &knappVisuellRect);
        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }

    SDL_DestroyTexture(bakgrund);
    SDL_DestroyTexture(startKnapp);
    return true;
}

