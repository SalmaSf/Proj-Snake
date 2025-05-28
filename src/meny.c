#include <SDL_ttf.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <stdbool.h>
#include "meny.h"

bool showStartMenu(SDL_Renderer *renderer, bool *soundOn)
{
    SDL_Texture *background = IMG_LoadTexture(renderer, "resources/meny_bakgrund.png");
    SDL_Texture *startButton = IMG_LoadTexture(renderer, "resources/start_knapp.png");
    SDL_Texture *soundOnIcon = IMG_LoadTexture(renderer, "resources/on.png");
    SDL_Texture *soundOffIcon = IMG_LoadTexture(renderer, "resources/off.png");
    SDL_Texture *closeIcon = IMG_LoadTexture(renderer, "resources/close.png");
    SDL_Texture *howToPlayButton = IMG_LoadTexture(renderer, "resources/HTP.png");

    if (!background || !startButton || !soundOnIcon || !soundOffIcon || !closeIcon)
    {
        SDL_Log("Could not load menupictures: %s", IMG_GetError());
        if (background)
            SDL_DestroyTexture(background);
        if (startButton)
            SDL_DestroyTexture(startButton);
        if (soundOnIcon)
            SDL_DestroyTexture(soundOnIcon);
        if (soundOffIcon)
            SDL_DestroyTexture(soundOffIcon);
        if (closeIcon)
            SDL_DestroyTexture(closeIcon);
        return false;
    }

    SDL_Rect buttonRect = {260, 390, 280, 140};
    SDL_Rect buttonVisuellRect = buttonRect;
    SDL_Rect soundRect = {650, 20, 60, 60};
    SDL_Rect soundVisuellRect = soundRect;
    SDL_Rect closeRect = {20, 20, 60, 60};
    SDL_Rect closeVisuellRect = closeRect;
    SDL_Rect howToPlayRect = {260, 550, 280, 140};
    SDL_Rect howToPlayVisuellRect = howToPlayRect;

    bool isPressed = false;
    bool soundPressed = false;
    bool closePressed = false;
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

                if (mx >= buttonRect.x && mx <= buttonRect.x + buttonRect.w &&
                    my >= buttonRect.y && my <= buttonRect.y + buttonRect.h)
                {
                    isPressed = true;
                    buttonVisuellRect.y += 4;
                }

                if (mx >= soundRect.x && mx <= soundRect.x + soundRect.w &&
                    my >= soundRect.y && my <= soundRect.y + soundRect.h)
                {
                    soundPressed = true;
                    soundVisuellRect.y += 4;
                }

                if (mx >= closeRect.x && mx <= closeRect.x + closeRect.w &&
                    my >= closeRect.y && my <= closeRect.y + closeRect.h)
                {
                    closePressed = true;
                    closeVisuellRect.y += 4;
                }

                if (mx >= howToPlayRect.x && mx <= howToPlayRect.x + howToPlayRect.w &&
                    my >= howToPlayRect.y && my <= howToPlayRect.y + howToPlayRect.h)
                {
                    howToPlayPressed = true;
                    howToPlayVisuellRect.y += 4;
                }
            }

            if (event.type == SDL_MOUSEBUTTONUP && event.button.button == SDL_BUTTON_LEFT)
            {
                int mx = event.button.x;
                int my = event.button.y;

                if (isPressed &&
                    mx >= buttonRect.x && mx <= buttonRect.x + buttonRect.w &&
                    my >= buttonRect.y && my <= buttonRect.y + buttonRect.h)
                {
                    SDL_DestroyTexture(background);
                    SDL_DestroyTexture(startButton);
                    SDL_DestroyTexture(soundOnIcon);
                    SDL_DestroyTexture(soundOffIcon);
                    SDL_DestroyTexture(closeIcon);
                    return true;
                }

                if (soundPressed &&
                    mx >= soundRect.x && mx <= soundRect.x + soundRect.w &&
                    my >= soundRect.y && my <= soundRect.y + soundRect.h)
                {
                    *soundOn = !(*soundOn);
                    Mix_VolumeMusic(*soundOn ? MIX_MAX_VOLUME : 0);
                }

                if (closePressed &&
                    mx >= closeRect.x && mx <= closeRect.x + closeRect.w &&
                    my >= closeRect.y && my <= closeRect.y + closeRect.h)
                {
                    SDL_DestroyTexture(background);
                    SDL_DestroyTexture(startButton);
                    SDL_DestroyTexture(soundOnIcon);
                    SDL_DestroyTexture(soundOffIcon);
                    SDL_DestroyTexture(closeIcon);
                    return false;
                }
                if (howToPlayPressed &&
                    mx >= howToPlayRect.x && mx <= howToPlayRect.x + howToPlayRect.w &&
                    my >= howToPlayRect.y && my <= howToPlayRect.y + howToPlayRect.h)
                {
                    showInstructions(renderer);
                }

                isPressed = false;
                soundPressed = false;
                closePressed = false;
                howToPlayPressed = false;
                buttonVisuellRect = buttonRect;
                soundVisuellRect = soundRect;
                closeVisuellRect = closeRect;
                howToPlayVisuellRect = howToPlayRect;
            }
        }

        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, background, NULL, NULL);
        SDL_RenderCopy(renderer, startButton, NULL, &buttonVisuellRect);

        if (*soundOn)
            SDL_RenderCopy(renderer, soundOnIcon, NULL, &soundVisuellRect);
        else
            SDL_RenderCopy(renderer, soundOffIcon, NULL, &soundVisuellRect);

        SDL_RenderCopy(renderer, closeIcon, NULL, &closeVisuellRect);
        SDL_RenderCopy(renderer, howToPlayButton, NULL, &howToPlayVisuellRect);

        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }

    SDL_DestroyTexture(background);
    SDL_DestroyTexture(startButton);
    SDL_DestroyTexture(soundOnIcon);
    SDL_DestroyTexture(soundOffIcon);
    SDL_DestroyTexture(closeIcon);
    SDL_DestroyTexture(howToPlayButton);
    return false;
}

bool showIPMenu(SDL_Renderer *renderer, char *ipBuffer, int bufferSize)
{

    SDL_Texture *background = IMG_LoadTexture(renderer, "resources/ip_meny_bakgrund.png");
    if (!background)
    {
        SDL_Log("Could not load ip_menu_bakgrund.png: %s", IMG_GetError());
        if (!background)
        {
            printf("Background could not load\n");
        }
        return false;
    }

    SDL_Rect inputBox = {0, 0, 400, 60};
    inputBox.x = (800 - inputBox.w) / 2;
    inputBox.y = (700 - inputBox.h) / 2 + 15;

    char ipInput[64] = "";
    SDL_StartTextInput();
    SDL_StartTextInput();
    if (!SDL_IsTextInputActive())
    {
        SDL_Log("SDL text input could not be started!");
    }

    TTF_Font *font = TTF_OpenFont("resources/GamjaFlower-Regular.ttf", 32);
    if (!font)
    {
        SDL_Log("Could not load font: %s", TTF_GetError());
        SDL_DestroyTexture(background);
        SDL_StopTextInput();
        return false;
    }

    SDL_Color svart = {0, 0, 0, 255};

    bool isRunning = true;
    SDL_Event event;

    while (isRunning)
    {

        SDL_RaiseWindow(SDL_GetWindowFromID(1));
        while (SDL_PollEvent(&event))
        {

            if (event.type == SDL_QUIT)
            {
                isRunning = false;
            }
            else if (event.type == SDL_KEYDOWN)
            {

                if (event.key.keysym.sym == SDLK_BACKSPACE && strlen(ipInput) > 0)
                {
                    ipInput[strlen(ipInput) - 1] = '\0';
                }
                else if (event.key.keysym.sym == SDLK_RETURN)
                {
                    if (strlen(ipInput) > 0)
                    {
                        isRunning = true;
                        strncpy(ipBuffer, ipInput, bufferSize - 1);
                        ipBuffer[bufferSize - 1] = '\0';
                        SDL_StopTextInput();
                        TTF_CloseFont(font);
                        SDL_DestroyTexture(background);
                        return true;
                    }
                    else
                    {
                        SDL_Log("No IP-adress written.");
                    }
                }
                else if (event.key.keysym.sym == SDLK_ESCAPE)
                {
                    SDL_StopTextInput();
                    TTF_CloseFont(font);
                    SDL_DestroyTexture(background);
                    return false;
                }
            }
            else if (event.type == SDL_TEXTINPUT)
            {
                if (strlen(ipInput) + strlen(event.text.text) < sizeof(ipInput) - 1)
                    strcat(ipInput, event.text.text);
            }
        }

        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, background, NULL, NULL);

        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderFillRect(renderer, &inputBox);

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderDrawRect(renderer, &inputBox);

        if (strlen(ipInput) > 0 && SDL_IsTextInputActive())
        {
            SDL_Surface *textSurface = TTF_RenderText_Solid(font, ipInput, svart);
            if (textSurface)
            {
                SDL_Texture *textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
                if (textTexture)
                {
                    SDL_Rect textRect = {
                        inputBox.x + 10,
                        inputBox.y + (inputBox.h - textSurface->h) / 2,
                        textSurface->w,
                        textSurface->h};
                    SDL_RenderCopy(renderer, textTexture, NULL, &textRect);
                    SDL_DestroyTexture(textTexture);
                }
                SDL_FreeSurface(textSurface);
            }
            else
            {
                SDL_Log("INFO: Could not render: %s", TTF_GetError());
            }
        }
        if (strlen(ipInput) > 0 && SDL_IsTextInputActive())
        {
            SDL_Surface *textSurface = TTF_RenderText_Solid(font, ipInput, svart);
            if (textSurface)
            {
                SDL_Texture *textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
                if (textTexture)
                {
                    SDL_Rect textRect = {
                        inputBox.x + 10,
                        inputBox.y + (inputBox.h - textSurface->h) / 2,
                        textSurface->w,
                        textSurface->h};
                    SDL_RenderCopy(renderer, textTexture, NULL, &textRect);
                    SDL_DestroyTexture(textTexture);
                }
                SDL_FreeSurface(textSurface);
            }
            else
            {
                SDL_Log("INFO: Could not render: %s", TTF_GetError());
            }
        }
        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }

    SDL_StopTextInput();
    TTF_CloseFont(font);
    SDL_DestroyTexture(background);

    strncpy(ipBuffer, ipInput, bufferSize - 1);
    ipBuffer[bufferSize - 1] = '\0';
    return true;
}

bool showLobby(SDL_Renderer *renderer, int numPlayers)
{
    SDL_Texture *lobbyBackground = IMG_LoadTexture(renderer, "resources/lobby.png");
    if (!lobbyBackground)
    {
        SDL_Log("Could not load lobby.png: %s", IMG_GetError());
        return false;
    }

    SDL_Texture *snakePink = IMG_LoadTexture(renderer, "resources/pink_head.png");
    SDL_Texture *snakeYellow = IMG_LoadTexture(renderer, "resources/purple_head.png");
    SDL_Texture *snakeGreen = IMG_LoadTexture(renderer, "resources/yellow_head.png");
    SDL_Texture *snakePurple = IMG_LoadTexture(renderer, "resources/snake_head.png");

    if (!snakePink || !snakeYellow || !snakeGreen || !snakePurple)
    {
        SDL_Log("Could not load one or more snake pictures");
        SDL_DestroyTexture(lobbyBackground);
        return false;
    }

    bool isRunning = true;
    SDL_Event event;

    SDL_Rect backgroundRect = {0, 0, 800, 700};

    SDL_Rect pinkRect = {15, 15, 170, 170};
    SDL_Rect yellowRect = {615, 15, 170, 170};
    SDL_Rect greenRect = {15, 515, 170, 170};
    SDL_Rect purpleRect = {615, 515, 170, 170};

    while (SDL_PollEvent(&event))
    {
        if (event.type == SDL_QUIT)
        {
            isRunning = false;
            break;
        }
    }

    SDL_RenderClear(renderer);

    SDL_RenderCopy(renderer, lobbyBackground, NULL, &backgroundRect);

    SDL_RenderCopy(renderer, snakePink, NULL, &pinkRect);
    SDL_RenderCopy(renderer, snakeYellow, NULL, &yellowRect);
    SDL_RenderCopy(renderer, snakeGreen, NULL, &greenRect);
    SDL_RenderCopy(renderer, snakePurple, NULL, &purpleRect);

    bool showTongue = ((SDL_GetTicks() / 300) % 2 == 0);
    if (showTongue)
    {
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);

        SDL_Rect tonguePink = {
            pinkRect.x + pinkRect.w / 2 - 3,
            pinkRect.y + pinkRect.h - 5,
            6, 10};
        SDL_Rect toungeYellow = {
            yellowRect.x + yellowRect.w / 2 - 3,
            yellowRect.y + yellowRect.h - 5,
            6, 10};
        SDL_Rect toungeGreen = {
            greenRect.x + greenRect.w / 2 - 3,
            greenRect.y + greenRect.h - 5,
            6, 10};
        SDL_Rect toungePurple = {
            purpleRect.x + purpleRect.w / 2 - 3,
            purpleRect.y + purpleRect.h - 5,
            6, 10};

        SDL_RenderFillRect(renderer, &tonguePink);
        SDL_RenderFillRect(renderer, &toungeYellow);
        SDL_RenderFillRect(renderer, &toungeGreen);
        SDL_RenderFillRect(renderer, &toungePurple);
    }

    SDL_RenderPresent(renderer);
    SDL_Delay(16);

    SDL_DestroyTexture(lobbyBackground);
    SDL_DestroyTexture(snakePink);
    SDL_DestroyTexture(snakeYellow);
    SDL_DestroyTexture(snakeGreen);
    SDL_DestroyTexture(snakePurple);

    return true;
}

int showResult(SDL_Renderer *renderer, bool won, float time)
{
    const char *picture = won ? "resources/vann.png" : "resources/lose.png";
    SDL_Texture *background = IMG_LoadTexture(renderer, picture);
    if (!background)
    {
        SDL_Log("Could not load vann.png: %s", IMG_GetError());
        return 0;
    }

    TTF_Font *font = TTF_OpenFont("resources/GamjaFlower-Regular.ttf", 40);
    TTF_SetFontStyle(font, TTF_STYLE_ITALIC);
    if (!font)
    {
        SDL_Log("Could not load font: %s", TTF_GetError());
        SDL_DestroyTexture(background);
        return 0;
    }

    SDL_Color vit = {255, 255, 255, 255};

    char timeText[64];
    int min = (int)time / 60;
    int sek = (int)time % 60;
    sprintf(timeText, "Time: %02d:%02d", min, sek);

    SDL_Surface *timeSurface = TTF_RenderText_Solid(font, timeText, vit);
    SDL_Texture *timeTex = SDL_CreateTextureFromSurface(renderer, timeSurface);
    SDL_Rect tidRect = {325, 330, timeSurface->w, timeSurface->h};
    SDL_FreeSurface(timeSurface);

    SDL_Rect playAgainKnapp = {225, 440, 250, 60};
    SDL_Rect quitKnapp = {225, 520, 250, 60};

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

                if (mx >= quitKnapp.x && mx <= quitKnapp.x + quitKnapp.w &&
                    my >= quitKnapp.y && my <= quitKnapp.y + quitKnapp.h)
                {
                    SDL_DestroyTexture(background);
                    SDL_DestroyTexture(timeTex);
                    TTF_CloseFont(font);
                    return 0;
                }

                if (mx >= playAgainKnapp.x && mx <= playAgainKnapp.x + playAgainKnapp.w &&
                    my >= playAgainKnapp.y && my <= playAgainKnapp.y + playAgainKnapp.h)
                {
                    SDL_DestroyTexture(background);
                    SDL_DestroyTexture(timeTex);
                    TTF_CloseFont(font);
                    return 1;
                }
            }

            if (event.type == SDL_KEYDOWN)
            {
                if (event.key.keysym.sym == SDLK_q || event.key.keysym.sym == SDLK_ESCAPE)
                    running = false;
            }
        }

        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, background, NULL, NULL);
        SDL_RenderCopy(renderer, timeTex, NULL, &tidRect);
        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }

    SDL_DestroyTexture(background);
    SDL_DestroyTexture(timeTex);
    TTF_CloseFont(font);

    return 0;
}
void showInstructions(SDL_Renderer *renderer)
{
    SDL_Texture *instrPicture = IMG_LoadTexture(renderer, "resources/HTPS.png");
    SDL_Texture *closeIcon = IMG_LoadTexture(renderer, "resources/close.png");

    if (!instrPicture || !closeIcon)
    {
        SDL_Log("Could not load instructional image or close button");
        if (instrPicture)
            SDL_DestroyTexture(instrPicture);
        if (closeIcon)
            SDL_DestroyTexture(closeIcon);
        return;
    }

    SDL_Rect closeRect = {20, 20, 60, 60};
    SDL_Rect closeVisualRect = closeRect;
    bool closePressed = false;

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
                    closeVisualRect.y += 4;
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
                closeVisualRect = closeRect;
            }

            if (event.type == SDL_KEYDOWN &&
                event.key.keysym.sym == SDLK_ESCAPE)
            {
                isRunning = false;
            }
        }

        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, instrPicture, NULL, NULL);
        SDL_RenderCopy(renderer, closeIcon, NULL, &closeVisualRect);
        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }

    SDL_DestroyTexture(instrPicture);
    SDL_DestroyTexture(closeIcon);
}

void showPlayerIdentity(SDL_Renderer *renderer, int clientID)
{
    char path[64];
    sprintf(path, "resources/identity_%d.png", clientID);

    SDL_Texture *img = IMG_LoadTexture(renderer, path);
    if (!img)
    {
        SDL_Log("Could not load identity image: %s", IMG_GetError());
        return;
    }

    SDL_Event event;
    bool running = true;
    Uint32 startTime = SDL_GetTicks();

    while (running)
    {
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT ||
                (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_RETURN) ||
                (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT))
            {
                running = false;
            }
        }

        Uint32 elapsed = SDL_GetTicks() - startTime;
        if (elapsed > 3000)
            running = false;

        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, img, NULL, NULL);
        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }

    SDL_DestroyTexture(img);
}
