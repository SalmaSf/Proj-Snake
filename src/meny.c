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

bool visaLobby(SDL_Renderer* renderer)
{
    SDL_Texture* lobbyBakgrund = IMG_LoadTexture(renderer, "resources/lobby.png");
    if (!lobbyBakgrund) {
        SDL_Log("Kunde inte ladda lobby.png: %s", IMG_GetError());
        return false;
    }

    // Ladda ormbilder
    SDL_Texture* ormRosa  = IMG_LoadTexture(renderer, "resources/pink_head.png");
    SDL_Texture* ormGul   = IMG_LoadTexture(renderer, "resources/purple_head.png");
    SDL_Texture* ormGrön  = IMG_LoadTexture(renderer, "resources/yellow_head.png");
    SDL_Texture* ormLila  = IMG_LoadTexture(renderer, "resources/snake_head.png");

    if (!ormRosa || !ormGul || !ormGrön || !ormLila) { //ändra namn
        SDL_Log("Kunde inte ladda en eller flera ormbilder");
        SDL_DestroyTexture(lobbyBakgrund);
        return false;
    }

    bool isRunning = true;
    SDL_Event event;

    const int visaLobbyTidMS = 5000;
    Uint32 startTime = SDL_GetTicks();

    SDL_Rect bakgrundRect = {0, 0, 800, 700};

    // Positioner för varje orm (stora huvuden: 170x170)
    SDL_Rect rosaRect = {15, 15, 170, 170};       // Vänster upp
    SDL_Rect gulRect  = {615, 15, 170, 170};      // Höger upp
    SDL_Rect gronRect = {15, 515, 170, 170};      // Vänster ner
    SDL_Rect lilaRect = {615, 515, 170, 170};     // Höger ner

    while (isRunning)
    {
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT) {
                isRunning = false;
                break;
            }
        }

        Uint32 elapsedTime = SDL_GetTicks() - startTime;
        if (elapsedTime >= visaLobbyTidMS) {
            isRunning = false;
        }

        SDL_RenderClear(renderer);

        // Rita stillastående bakgrund
        SDL_RenderCopy(renderer, lobbyBakgrund, NULL, &bakgrundRect);

        // Rita ormar
        SDL_RenderCopy(renderer, ormRosa, NULL, &rosaRect);
        SDL_RenderCopy(renderer, ormGul,  NULL, &gulRect);
        SDL_RenderCopy(renderer, ormGrön, NULL, &gronRect);
        SDL_RenderCopy(renderer, ormLila, NULL, &lilaRect);

        // Tunga-animering (blinkar var 300 ms)
        bool visaTunga = ((SDL_GetTicks() / 300) % 2 == 0);
        if (visaTunga) {
            SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);

            // Rita tungor
            SDL_Rect tungaRosa = {
                rosaRect.x + rosaRect.w / 2 - 3,
                rosaRect.y + rosaRect.h - 5,
                6, 10
            };
            SDL_Rect tungaGul = {
                gulRect.x + gulRect.w / 2 - 3,
                gulRect.y + gulRect.h - 5,
                6, 10
            };
            SDL_Rect tungaGrön = {
                gronRect.x + gronRect.w / 2 - 3,
                gronRect.y + gronRect.h - 5,
                6, 10
            };
            SDL_Rect tungaLila = {
                lilaRect.x + lilaRect.w / 2 - 3,
                lilaRect.y + lilaRect.h - 5,
                6, 10
            };

            SDL_RenderFillRect(renderer, &tungaRosa);
            SDL_RenderFillRect(renderer, &tungaGul);
            SDL_RenderFillRect(renderer, &tungaGrön);
            SDL_RenderFillRect(renderer, &tungaLila);
        }

        SDL_RenderPresent(renderer);
        SDL_Delay(16); // ca 60 FPS
    }

    SDL_DestroyTexture(lobbyBakgrund);
    SDL_DestroyTexture(ormRosa);
    SDL_DestroyTexture(ormGul);
    SDL_DestroyTexture(ormGrön);
    SDL_DestroyTexture(ormLila);

    return true;
}

int visaResultatskarm(SDL_Renderer* renderer, bool vann, float tid)
{
    // 1. Ladda bakgrund
    SDL_Texture* background = IMG_LoadTexture(renderer, "resources/vann.png");
    if (!background)
    {
        SDL_Log("Kunde inte ladda vann.png: %s", IMG_GetError());
        return 0;
    }

    // 2. Ladda font
    TTF_Font* font = TTF_OpenFont("GamjaFlower-Regular.ttf", 40);
    TTF_SetFontStyle(font, TTF_STYLE_ITALIC);
    if (!font)
    {
        SDL_Log("Kunde inte ladda font: %s", TTF_GetError());
        SDL_DestroyTexture(background);
        return 0;
    }

    SDL_Color vit = {255, 255, 255, 255};

    // 3. Skapa tidstext
    char tidText[64];
    int min = (int)tid / 60;
    int sek = (int)tid % 60;
    sprintf(tidText, "Time: %02d:%02d", min, sek);

    SDL_Surface* tidSurface = TTF_RenderText_Solid(font, tidText, vit);
    SDL_Texture* tidTex = SDL_CreateTextureFromSurface(renderer, tidSurface);
    SDL_Rect tidRect = {325, 330, tidSurface->w, tidSurface->h};
    SDL_FreeSurface(tidSurface);

    // 4. Definiera klickbara områden (matchar bilden)
    SDL_Rect playAgainKnapp = {225, 440, 250, 60};
    SDL_Rect quitKnapp      = {225, 520, 250, 60};

    // 5. Event-loop
    SDL_Event event;
    bool running = true;

    while (running)
    {
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
                running = false;

            // Musen trycks ned
            if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT)
            {
                int mx = event.button.x;
                int my = event.button.y;

                // Klick på QUIT
                if (mx >= quitKnapp.x && mx <= quitKnapp.x + quitKnapp.w &&
                    my >= quitKnapp.y && my <= quitKnapp.y + quitKnapp.h)
                {
                    SDL_DestroyTexture(background);
                    SDL_DestroyTexture(tidTex);
                    TTF_CloseFont(font);
                    return 0; // QUIT
                }

                // Klick på PLAY AGAIN
                if (mx >= playAgainKnapp.x && mx <= playAgainKnapp.x + playAgainKnapp.w &&
                    my >= playAgainKnapp.y && my <= playAgainKnapp.y + playAgainKnapp.h)
                {
                    SDL_DestroyTexture(background);
                    SDL_DestroyTexture(tidTex);
                    TTF_CloseFont(font);
                    return 1; // PLAY AGAIN
                }
            }

            // Tangentbordsalternativ
            if (event.type == SDL_KEYDOWN)
            {
                if (event.key.keysym.sym == SDLK_q || event.key.keysym.sym == SDLK_ESCAPE)
                    running = false;
            }
        }

        // Rendera
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, background, NULL, NULL);
        SDL_RenderCopy(renderer, tidTex, NULL, &tidRect);
        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }

    // Städning
    SDL_DestroyTexture(background);
    SDL_DestroyTexture(tidTex);
    TTF_CloseFont(font);

    return 0;
}

/*void keepWatching(Snake* snake[], SDL_Renderer* renderer, SDL_Texture* background)
{
    SDL_Event event;
    bool running = true;

    while (running)
    {
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
                running = false;
            if (event.type == SDL_KEYDOWN)
            {
                if (event.key.keysym.sym == SDLK_q || event.key.keysym.sym == SDLK_ESCAPE)
                    running = false;
            }
        }

        // Uppdatera levande ormar
        for (int i = 0; i < 4; i++)
        {
            if (isSnakeAlive(snake[i]))
                updateSnake(snake[i]);
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        SDL_RenderCopy(renderer, background, NULL, NULL);

        for (int i = 0; i < 4; i++)
        {
            if (isSnakeAlive(snake[i]))
                drawSnake(snake[i]);
        }

        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }
}*/
