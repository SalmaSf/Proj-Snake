#include <stdlib.h>
#include <string.h>
#include "meny.h"

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

