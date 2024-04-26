#include <iostream>
#include <SDL2/SDL.h>
#include "Chip8Core.h"
#include "Chip8Display.h"

uint speedMultiplier = 5;
uint cycleCount = 0;

void UpdateKeypressDown(BYTE key, Chip8Core& chip8Core) {
    chip8Core.SetKeys(key, true);
}

void UpdateKeypressRelease(BYTE key, Chip8Core& chip8Core) {
    chip8Core.SetKeys(key, false);
}

int main() {
    SDL_Event event;

    bool running = true;

    if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
        std::cout << "SDL initialization error: " << SDL_GetError();
    }
    else {
        std::cout << "SDL init success\n";
    }

    Chip8Core chip8Core;

    Chip8Display::Initialize();

    while (running) {
        Uint64 start = SDL_GetPerformanceCounter();
        // event polling
        while(SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) { running = false; }
            if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_1) { UpdateKeypressDown(0x00, chip8Core);}
                if (event.key.keysym.sym == SDLK_2) { UpdateKeypressDown(0x01, chip8Core);}
                if (event.key.keysym.sym == SDLK_3) { UpdateKeypressDown(0x02, chip8Core);}
                if (event.key.keysym.sym == SDLK_4) { UpdateKeypressDown(0x03, chip8Core);}
                if (event.key.keysym.sym == SDLK_q) { UpdateKeypressDown(0x04, chip8Core);}
                if (event.key.keysym.sym == SDLK_w) { UpdateKeypressDown(0x05, chip8Core);}
                if (event.key.keysym.sym == SDLK_e) { UpdateKeypressDown(0x06, chip8Core);}
                if (event.key.keysym.sym == SDLK_r) { UpdateKeypressDown(0x07, chip8Core);}
                if (event.key.keysym.sym == SDLK_a) { UpdateKeypressDown(0x08, chip8Core);}
                if (event.key.keysym.sym == SDLK_s) { UpdateKeypressDown(0x09, chip8Core);}
                if (event.key.keysym.sym == SDLK_d) { UpdateKeypressDown(0x0A, chip8Core);}
                if (event.key.keysym.sym == SDLK_f) { UpdateKeypressDown(0x0B, chip8Core);}
                if (event.key.keysym.sym == SDLK_z) { UpdateKeypressDown(0x0C, chip8Core);}
                if (event.key.keysym.sym == SDLK_x) { UpdateKeypressDown(0x0D, chip8Core);}
                if (event.key.keysym.sym == SDLK_c) { UpdateKeypressDown(0x0E, chip8Core);}
                if (event.key.keysym.sym == SDLK_v) { UpdateKeypressDown(0x0F, chip8Core);}
            }
            if (event.type == SDL_KEYUP) {
                if (event.key.keysym.sym == SDLK_1) { UpdateKeypressRelease(0x00, chip8Core);}
                if (event.key.keysym.sym == SDLK_2) { UpdateKeypressRelease(0x01, chip8Core);}
                if (event.key.keysym.sym == SDLK_3) { UpdateKeypressRelease(0x02, chip8Core);}
                if (event.key.keysym.sym == SDLK_4) { UpdateKeypressRelease(0x03, chip8Core);}
                if (event.key.keysym.sym == SDLK_q) { UpdateKeypressRelease(0x04, chip8Core);}
                if (event.key.keysym.sym == SDLK_w) { UpdateKeypressRelease(0x05, chip8Core);}
                if (event.key.keysym.sym == SDLK_e) { UpdateKeypressRelease(0x06, chip8Core);}
                if (event.key.keysym.sym == SDLK_r) { UpdateKeypressRelease(0x07, chip8Core);}
                if (event.key.keysym.sym == SDLK_a) { UpdateKeypressRelease(0x08, chip8Core);}
                if (event.key.keysym.sym == SDLK_s) { UpdateKeypressRelease(0x09, chip8Core);}
                if (event.key.keysym.sym == SDLK_d) { UpdateKeypressRelease(0x0A, chip8Core);}
                if (event.key.keysym.sym == SDLK_f) { UpdateKeypressRelease(0x0B, chip8Core);}
                if (event.key.keysym.sym == SDLK_z) { UpdateKeypressRelease(0x0C, chip8Core);}
                if (event.key.keysym.sym == SDLK_x) { UpdateKeypressRelease(0x0D, chip8Core);}
                if (event.key.keysym.sym == SDLK_c) { UpdateKeypressRelease(0x0E, chip8Core);}
                if (event.key.keysym.sym == SDLK_v) { UpdateKeypressRelease(0x0F, chip8Core);}
            }
        }

        // cpu step
        chip8Core.EmulateCycle();
        if (!(++cycleCount % speedMultiplier)) {
            chip8Core.DecrementTimers();
            cycleCount = 0;
        }

        // render
        Chip8Display::RenderFrame(chip8Core);

        // play sound
        if (chip8Core.ShouldPlaySound()) {
            std::cout << "BEEP\n";
        }

        // calc run time delay
        Uint64 end = SDL_GetPerformanceCounter();
        float elapsedMS = (float)(end - start) / (float)SDL_GetPerformanceFrequency() * 1000.0f;
        // 60-ish Hz @ 1 speedMultiplier
        SDL_Delay(floor((16.666f / (float)speedMultiplier) - elapsedMS));
    }
    // cleanup
    SDL_Quit();
    return 0;
}
