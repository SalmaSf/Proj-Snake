#include <SDL_ttf.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
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

    SDL_Rect knappRect = { 260, 390, 280, 140 }; 
    SDL_Rect knappVisuellRect = knappRect;     
    bool isPressed = false;
    bool iMeny = true;
    SDL_Event event;

    while (iMeny)
    {
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT) {
                // Endast avsluta hela programmet om användaren stänger fönstret!
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
                    knappVisuellRect.y += 4; // tryck-effekt
                }
            }

            if (event.type == SDL_MOUSEBUTTONUP && event.button.button == SDL_BUTTON_LEFT) {
                int mx = event.button.x;
                int my = event.button.y;

                if (isPressed &&
                    mx >= knappRect.x && mx <= knappRect.x + knappRect.w &&
                    my >= knappRect.y && my <= knappRect.y + knappRect.h) {
                    iMeny = false; // släppte musen på knappen → fortsätt till IP-meny
                }

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
    return true;  // Viktigt: returnera true så att vi går vidare till IP-meny
}

bool visaIPMeny(SDL_Renderer* renderer)
{
    SDL_Texture* bakgrund = IMG_LoadTexture(renderer, "resources/ip_meny_bakgrund.png");
    if (!bakgrund) {
        SDL_Log("Kunde inte ladda ip_meny_bakgrund.png: %s", IMG_GetError());
        return false;
    }

    // Definiera rektangeln – centrera + flytta ner lite (ca 10-15 pixlar)
    SDL_Rect inputBox = { 0, 0, 400, 60 };  
    inputBox.x = (800 - inputBox.w) / 2;   // centrerad horisontellt
    inputBox.y = (700 - inputBox.h) / 2 + 15;  // lite ner från mitten (15 pixlar ner)

    bool isRunning = true;
    SDL_Event event;

    while (isRunning)
    {
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT || 
                (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE) || 
                (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_RETURN)) 
            {
                isRunning = false;  // Stäng sidan med ESC eller ENTER
            }
        }

        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, bakgrund, NULL, NULL);

        // Rita vit ruta (inputBox)
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);  // vit fyllning
        SDL_RenderFillRect(renderer, &inputBox);

        // Svart kant runt rutan
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);  // svart kant
        SDL_RenderDrawRect(renderer, &inputBox);

        SDL_RenderPresent(renderer);
        SDL_Delay(16); // ca 60 FPS
    }

    SDL_DestroyTexture(bakgrund);
    return true;
}

#include <math.h> // För sin()

bool visaLobby(SDL_Renderer* renderer)
{
    SDL_Texture* lobbyBakgrund = IMG_LoadTexture(renderer, "resources/lobby.png");
    if (!lobbyBakgrund) {
        SDL_Log("Kunde inte ladda lobby.png: %s", IMG_GetError());
        return false;
    }

    bool isRunning = true;
    SDL_Event event;

    int frame = 0;
    const int visaLobbyTidMS = 5000; // Visa lobbyn i 5 sekunder (5000 millisekunder)
    Uint32 startTime = SDL_GetTicks(); // Tid när lobbyn startar

    SDL_Rect bakgrundRect = {0, 0, 800, 700};

    while (isRunning)
    {
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT) 
            {
                isRunning = false;
                break;
            }
        }

        frame++;

        Uint32 elapsedTime = SDL_GetTicks() - startTime; // Hur länge har vi varit i lobbyn?

        // Om tiden är slut -> avsluta lobbyn automatiskt
        if (elapsedTime >= visaLobbyTidMS) {
            isRunning = false;
        }

        // Gungning (valfritt, fortfarande aktivt under lobbytid)
        float offset = sin(frame * 0.1f) * 12;
        SDL_Rect drawRect = bakgrundRect;
        drawRect.y += (int)offset;

        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, lobbyBakgrund, NULL, &drawRect);
        SDL_RenderPresent(renderer);
        SDL_Delay(16); // ca 60 FPS
    }

    SDL_DestroyTexture(lobbyBakgrund);
    return true;
}
