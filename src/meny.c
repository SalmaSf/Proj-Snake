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

    SDL_Rect knappRect = { 300, 400, 200, 80 }; // normal position
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

/*
struct startButton {
    SDL_Rect rect;
    SDL_Color color;
    SDL_Color textColor;
    SDL_Texture *textTexture;
    SDL_Renderer *renderer;
};


StartButton *createStartButton(SDL_Renderer *renderer, TTF_Font *font, const char *text, int x, int y, int w, int h, SDL_Color btnColor, SDL_Color textColor) {
    StartButton *button = malloc(sizeof(StartButton));
    if (!button){
        return NULL;
    } 
    button->renderer = renderer;
    button->rect.x = x;
    button->rect.y = y;
    button->rect.w = w;
    button->rect.h = h;
    button->color = btnColor;
    button->textColor = textColor;
    // Rendera texten
    SDL_Surface *textSurface = TTF_RenderText_Solid(font, text, textColor);
    if (!textSurface) {
        free(button);
        return NULL;
    }
    button->textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
    SDL_FreeSurface(textSurface);
    if (!button->textTexture) {
        free(button);
        return NULL;
    }
    return button;
}

void renderStartButton(StartButton *button) {
    // Rita bakgrunden (knappens rektangel)
    SDL_SetRenderDrawColor(button->renderer, button->color.r, button->color.g, button->color.b, 255);
    SDL_RenderFillRect(button->renderer, &button->rect);
    // Rita texten centrerat
    int textW, textH;
    SDL_QueryTexture(button->textTexture, NULL, NULL, &textW, &textH);
    SDL_Rect textRect = {
        button->rect.x + (button->rect.w - textW) / 2,
        button->rect.y + (button->rect.h - textH) / 2,
        textW,
        textH
    };
    SDL_RenderCopy(button->renderer, button->textTexture, NULL, &textRect);
}

bool isMouseOverStartButton(StartButton *button, int mouseX, int mouseY) {
    return (mouseX >= button->rect.x && mouseX <= (button->rect.x + button->rect.w) &&
            mouseY >= button->rect.y && mouseY <= (button->rect.y + button->rect.h));
}

void destroyStartButton(StartButton *button) {
    if (button) {
        if (button->textTexture) {
            SDL_DestroyTexture(button->textTexture);
            button->textTexture = NULL; // Sätt till NULL efter frigöring
        }
        free(button);
        button = NULL; // Sätt till NULL efter frigöring
    }
}*/

