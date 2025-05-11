#include <SDL_ttf.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <stdbool.h>
#include "meny.h"

bool visaStartMeny(SDL_Renderer* renderer, bool* ljudPa)
{
    SDL_Texture* bakgrund     = IMG_LoadTexture(renderer, "resources/meny_bakgrund.png");
    SDL_Texture* startKnapp   = IMG_LoadTexture(renderer, "resources/start_knapp.png");
    SDL_Texture* soundOnIcon  = IMG_LoadTexture(renderer, "resources/on.png");
    SDL_Texture* soundOffIcon = IMG_LoadTexture(renderer, "resources/off.png");
    SDL_Texture* closeIcon    = IMG_LoadTexture(renderer, "resources/close.png");
    SDL_Texture* howToPlayButton = IMG_LoadTexture(renderer, "resources/HTP.png");


    if (!bakgrund || !startKnapp || !soundOnIcon || !soundOffIcon || !closeIcon) {
        SDL_Log("Kunde inte ladda menybilder: %s", IMG_GetError());
        if (bakgrund)     SDL_DestroyTexture(bakgrund);
        if (startKnapp)   SDL_DestroyTexture(startKnapp);
        if (soundOnIcon)  SDL_DestroyTexture(soundOnIcon);
        if (soundOffIcon) SDL_DestroyTexture(soundOffIcon);
        if (closeIcon)    SDL_DestroyTexture(closeIcon);
        return false;
    }

    SDL_Rect knappRect        = { 260, 390, 280, 140 };
    SDL_Rect knappVisuellRect = knappRect;
    SDL_Rect soundRect        = { 650, 20, 60, 60 };
    SDL_Rect soundVisuellRect = soundRect;
    SDL_Rect closeRect        = { 20, 20, 60, 60 };
    SDL_Rect closeVisuellRect = closeRect;
    SDL_Rect howToPlayRect = { 260, 550, 280, 140 };  // 
    SDL_Rect howToPlayVisuellRect = howToPlayRect;


    bool isPressed     = false;
    bool soundPressed  = false;
    bool closePressed  = false;
    bool howToPlayPressed = false;


    SDL_Event event;
    bool running = true;

    while (running)
    {
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
                running = false;

            if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT)
            {
                int mx = event.button.x;
                int my = event.button.y;

                // Start-knapp
                if (mx >= knappRect.x && mx <= knappRect.x + knappRect.w &&
                    my >= knappRect.y && my <= knappRect.y + knappRect.h)
                {
                    isPressed = true;
                    knappVisuellRect.y += 4;
                }

                // Ljud-knapp
                if (mx >= soundRect.x && mx <= soundRect.x + soundRect.w &&
                    my >= soundRect.y && my <= soundRect.y + soundRect.h)
                {
                    soundPressed = true;
                    soundVisuellRect.y += 4;
                }

                // Close-knapp
                if (mx >= closeRect.x && mx <= closeRect.x + closeRect.w &&
                    my >= closeRect.y && my <= closeRect.y + closeRect.h)
                {
                    closePressed = true;
                    closeVisuellRect.y += 4;
                }
                // How to Play-knapp
                if (mx >= howToPlayRect.x && mx <= howToPlayRect.x + howToPlayRect.w &&
                    my >= howToPlayRect.y && my <= howToPlayRect.y + howToPlayRect.h)
                {
                    howToPlayPressed = true;
                    howToPlayVisuellRect.y += 4; // Tryck-effekt
                }
                

            }

            if (event.type == SDL_MOUSEBUTTONUP && event.button.button == SDL_BUTTON_LEFT)
            {
                int mx = event.button.x;
                int my = event.button.y;

                // Start
                if (isPressed &&
                    mx >= knappRect.x && mx <= knappRect.x + knappRect.w &&
                    my >= knappRect.y && my <= knappRect.y + knappRect.h)
                {
                    SDL_DestroyTexture(bakgrund);
                    SDL_DestroyTexture(startKnapp);
                    SDL_DestroyTexture(soundOnIcon);
                    SDL_DestroyTexture(soundOffIcon);
                    SDL_DestroyTexture(closeIcon);
                    return true;
                }

                // Ljud
                if (soundPressed &&
                    mx >= soundRect.x && mx <= soundRect.x + soundRect.w &&
                    my >= soundRect.y && my <= soundRect.y + soundRect.h)
                {
                    *ljudPa = !(*ljudPa);
                    Mix_VolumeMusic(*ljudPa ? MIX_MAX_VOLUME : 0);
                }

                // Close
                if (closePressed &&
                    mx >= closeRect.x && mx <= closeRect.x + closeRect.w &&
                    my >= closeRect.y && my <= closeRect.y + closeRect.h)
                {
                    SDL_DestroyTexture(bakgrund);
                    SDL_DestroyTexture(startKnapp);
                    SDL_DestroyTexture(soundOnIcon);
                    SDL_DestroyTexture(soundOffIcon);
                    SDL_DestroyTexture(closeIcon);
                    return false;
                }
                if (howToPlayPressed &&
                    mx >= howToPlayRect.x && mx <= howToPlayRect.x + howToPlayRect.w &&
                    my >= howToPlayRect.y && my <= howToPlayRect.y + howToPlayRect.h)
                {
                    visaInstruktionsskarm(renderer);
                }
                
            
                

                // Återställ visuellt
                isPressed = false;
                soundPressed = false;
                closePressed = false;
                howToPlayPressed = false;
                knappVisuellRect = knappRect;
                soundVisuellRect = soundRect;
                closeVisuellRect = closeRect;
                howToPlayVisuellRect = howToPlayRect;
            }
        }

        // Rendering
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, bakgrund, NULL, NULL);
        SDL_RenderCopy(renderer, startKnapp, NULL, &knappVisuellRect);

        if (*ljudPa)
            SDL_RenderCopy(renderer, soundOnIcon, NULL, &soundVisuellRect);
        else
            SDL_RenderCopy(renderer, soundOffIcon, NULL, &soundVisuellRect);

        SDL_RenderCopy(renderer, closeIcon, NULL, &closeVisuellRect);
        SDL_RenderCopy(renderer, howToPlayButton, NULL, &howToPlayVisuellRect);

        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }

    SDL_DestroyTexture(bakgrund);
    SDL_DestroyTexture(startKnapp);
    SDL_DestroyTexture(soundOnIcon);
    SDL_DestroyTexture(soundOffIcon);
    SDL_DestroyTexture(closeIcon);
    SDL_DestroyTexture(howToPlayButton);
    return false;
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
    //const char* bildFil = vann ? "resources/vann.png" : "resources/lose.png"; //kanske funkar med servern??
    //SDL_Texture* background = IMG_LoadTexture(renderer, bildFil);
    if (!background)
    {
        SDL_Log("Kunde inte ladda vann.png: %s", IMG_GetError());
        return 0;
    } 

    // 2. Ladda font
    TTF_Font* font = TTF_OpenFont("resources/GamjaFlower-Regular.ttf", 40);
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
void visaInstruktionsskarm(SDL_Renderer* renderer)
{
    SDL_Texture* instrBild = IMG_LoadTexture(renderer, "resources/HTPS.png");
    SDL_Texture* closeIcon = IMG_LoadTexture(renderer, "resources/close.png");

    if (!instrBild || !closeIcon) {
        SDL_Log("Kunde inte ladda instruktionsbild eller stängknapp");
        if (instrBild) SDL_DestroyTexture(instrBild);
        if (closeIcon) SDL_DestroyTexture(closeIcon);
        return;
    }

    SDL_Rect closeRect        = { 20, 20, 60, 60 };
    SDL_Rect closeVisualRect  = closeRect;
    bool closePressed         = false;

    bool isRunning = true;
    SDL_Event event;

    while (isRunning)
    {
        while (SDL_PollEvent(&event))
        {
            int mx = event.button.x;
            int my = event.button.y;

            if (event.type == SDL_QUIT)
                isRunning = false;

            if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT)
            {
                if (mx >= closeRect.x && mx <= closeRect.x + closeRect.w &&
                    my >= closeRect.y && my <= closeRect.y + closeRect.h)
                {
                    closePressed = true;
                    closeVisualRect.y += 4; // Visuell feedback
                }
            }

            if (event.type == SDL_MOUSEBUTTONUP && event.button.button == SDL_BUTTON_LEFT)
            {
                if (closePressed &&
                    mx >= closeRect.x && mx <= closeRect.x + closeRect.w &&
                    my >= closeRect.y && my <= closeRect.y + closeRect.h)
                {
                    isRunning = false;
                }

                closePressed = false;
                closeVisualRect = closeRect; // Återställ visuell position
            }

            if (event.type == SDL_KEYDOWN &&
                event.key.keysym.sym == SDLK_ESCAPE)
            {
                isRunning = false;
            }
        }

        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, instrBild, NULL, NULL);
        SDL_RenderCopy(renderer, closeIcon, NULL, &closeVisualRect);
        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }

    SDL_DestroyTexture(instrBild);
    SDL_DestroyTexture(closeIcon);
}
