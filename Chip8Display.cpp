#include "Chip8Display.h"
#include "Chip8Core.h"
#include <SDL2/SDL.h>
#include <iostream>

SDL_Rect rectArray[64][32];
SDL_Window *window;
SDL_Renderer *renderer;

BYTE (*screenData)[64][32];

void Chip8Display::Initialize() {


    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cout << "SDL initialization error: " << SDL_GetError();
    }
    else {
        std::cout << "SDL init success\n";
    }

    window = SDL_CreateWindow(
            "chip-8 emu",
            SDL_WINDOWPOS_CENTERED,
            SDL_WINDOWPOS_CENTERED,
            640,
            320,
            SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL
    );
    if (window == nullptr) {
        std::cout << "Window creation error: " << SDL_GetError();
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == nullptr) {
        std::cout << "Renderer creation error: " << SDL_GetError();
    }

    ClearScreen(renderer);

    InitRectArray(rectArray);
}

void Chip8Display::RenderFrame(Chip8Core& chip8Core) {
    UpdateScreen(renderer, chip8Core);
    RenderScreen(renderer);
}

void Chip8Display::InitRectArray(SDL_Rect pRect[64][32]) {
    for (int i = 0; i < 64; i++) {
        for (int j = 0; j < 32; j++) {
            pRect[i][j].x = i * 10;
            pRect[i][j].y = j * 10;
            pRect[i][j].w = 10;
            pRect[i][j].h = 10;
        }
    }
}

void Chip8Display::UpdateScreen(SDL_Renderer *pRenderer, Chip8Core& chip8Core) {
    screenData = chip8Core.GetScreenData();
    for (int i = 0; i < 64; i++) {
        for (int j = 0; j < 32; j++) {
            if ((*screenData)[i][j]) {
                SDL_SetRenderDrawColor(pRenderer, 255, 255, 255, 255);
                SDL_RenderFillRect(pRenderer, &rectArray[i][j]);
            }
            else {
                SDL_SetRenderDrawColor(pRenderer, 0, 0, 0, 255);
                SDL_RenderFillRect(pRenderer, &rectArray[i][j]);
            }
        }
    }
}

void Chip8Display::RenderScreen(SDL_Renderer *pRenderer) {
    SDL_RenderPresent(pRenderer);
}

void Chip8Display::ClearScreen(SDL_Renderer *pRenderer) {
    SDL_SetRenderDrawColor(pRenderer, 0, 0, 0, 255);
    SDL_RenderClear(pRenderer);
}