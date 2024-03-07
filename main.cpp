#include <iostream>
#include <vector>
#include <SDL2/SDL.h>
/*
 * Based on VM description as published on Wikipedia:
 * https://en.wikipedia.org/wiki/CHIP-8
 *
 * And Cowgod's technical reference:
 * http://devernay.free.fr/hacks/chip8/C8TECH10.HTM
 *
 * With special thanks to the author of codeslinger.co.uk for helpful insight where I needed it
 * and helping me get started with emulation development:
 * http://www.codeslinger.co.uk/index.html
 * */
typedef unsigned char BYTE;
typedef unsigned short int WORD;
typedef unsigned char TIMER;

BYTE gameMemory[0xFFF];
BYTE registers[16];
WORD addressI;
WORD programCounter;
std::vector<WORD> stack;
BYTE screenData[64][32];
SDL_Rect rectArray[64][32];
BYTE input[16];

TIMER delayTimer, soundTimer = 0;
bool awaitKey = false; // halted for key event?
bool keyPressed = false; // key pressed after halt?
BYTE recentKey; // most recent key


void InitRectArray(SDL_Rect rects[][32]) {
    for (int i = 0; i < 64; i++) {
        for (int j = 0; j < 32; j++) {
            rects[i][j].x = i * 10;
            rects[i][j].y = j * 10;
            rects[i][j].w = 10;
            rects[i][j].h = 10;
        }
    }
}

void UpdateRenderedPixels(SDL_Renderer *renderer) {
    for (int i = 0; i < 64; i++) {
        for (int j = 0; j < 32; j++) {
            if (screenData[i][j]) {
                SDL_SetRenderDrawColor(renderer,255,255,255,255);
                SDL_RenderFillRect(renderer, &rectArray[i][j]);
            }
            else {
                SDL_SetRenderDrawColor(renderer,0,0,0,255);
                SDL_RenderFillRect(renderer, &rectArray[i][j]);
            }
        }
    }
}

void UpdateKeypressDown(BYTE key) {
    input[key] = 1;
    recentKey = key;
    if (awaitKey) keyPressed = true;
}

void UpdateKeypressRelease(BYTE key) {
    input[key] = 0;
}


WORD GetNextOpcode() { // opcode is 2 bytes, stored across 2 addresses from the programCounter location
    WORD res = 0;
    res = gameMemory[programCounter]; // 0xAB
    res <<= 8; // 0xAB00
    res |= gameMemory[programCounter + 1]; //0xABCD
    programCounter += 2; // next instruction, unless halted for key event (Opcode FX0A)
    return res;
}


void ClearDisplay() {
    memset(screenData, 0, sizeof(screenData));
}

// Clears the screen.
void Opcode00E0() {
    ClearDisplay();
}

// Returns from a subroutine.
void Opcode00EE() {
    programCounter = stack.back();
    stack.pop_back();
}

/* Call machine code routine at NNN. For the emulator it does nothing and the instruction is ignored.
 * void Opcode0NNN(WORD opcode) {
 * }
 */

// Jump to NNN.
void Opcode1NNN(WORD opcode) {
    programCounter = opcode & 0xFFF;
}

// Call subroutine at NNN.
void Opcode2NNN(WORD opcode) {
    stack.push_back(programCounter);
    programCounter = opcode & 0xFFF;
}

void Opcode3XNN(WORD opcode) {
    WORD reg = opcode & 0xF00; reg >>= 8;
    if (registers[reg] == (opcode & 0xFF))
        programCounter += 2;
}

void Opcode4XNN(WORD opcode) {
    WORD reg = opcode & 0xF00; reg >>= 8;
    if (registers[reg] != (opcode & 0xFF))
        programCounter += 2;
}

void Opcode5XY0(WORD opcode) {
    WORD regX = opcode & 0xF00; regX >>= 8;
    WORD regY = opcode & 0x0F0; regY >>= 4;
    if (registers[regX] == registers[regY])
        programCounter += 2;
}

void Opcode6XNN(WORD opcode) {
    WORD reg = opcode & 0xF00; reg >>= 8;
    registers[reg] = opcode & 0xFF;
}

void Opcode7XNN(WORD opcode) {
    WORD reg = opcode & 0xF00; reg >>= 8;
    registers[reg] += opcode & 0xFF;
}

void Opcode8XY0(WORD opcode) {
    WORD regX = opcode & 0xF00; regX >>= 8;
    WORD regY = opcode & 0x0F0; regY >>= 4;
    registers[regX] = registers[regY];
}

void Opcode8XY1(WORD opcode) {
    WORD regX = opcode & 0xF00; regX >>= 8;
    WORD regY = opcode & 0x0F0; regY >>= 4;
    registers[regX] |= registers[regY];
}

void Opcode8XY2(WORD opcode) {
    WORD regX = opcode & 0xF00; regX >>= 8;
    WORD regY = opcode & 0x0F0; regY >>= 4;
    registers[regX] &= registers[regY];
}

void Opcode8XY3(WORD opcode) {
    WORD regX = opcode & 0xF00; regX >>= 8;
    WORD regY = opcode & 0x0F0; regY >>= 4;
    registers[regX] ^= registers[regY];
}

void Opcode8XY4(WORD opcode) {
    registers[0xF] = 0;
    WORD regX = opcode & 0xF00; regX >>= 8;
    WORD regY = opcode & 0x0F0; regY >>= 4;
    WORD xVal = registers[regX];
    WORD yVal = registers[regY];
    WORD res = xVal + yVal;
    if (res < xVal)
        registers[0xF] = 1;
    registers[regX] = res;
}

void Opcode8XY5(WORD opcode) {
    registers[0xF] = 0;
    WORD regX = opcode & 0xF00; regX >>= 8;
    WORD regY = opcode & 0x0F0; regY >>= 4;
    WORD xVal = registers[regX];
    WORD yVal = registers[regY];
    if (xVal >= yVal)
        registers[0xF] = 1;
    registers[regX] = xVal - yVal;
}

void Opcode8XY6(WORD opcode) {
    WORD regX = opcode & 0xF00; regX >>= 8;
    registers[0xF] = registers[regX] & 0x1;
    registers[regX] >>= 1;
}

void Opcode8XY7(WORD opcode) {
    registers[0xF] = 0;
    WORD regX = opcode & 0xF00; regX >>= 8;
    WORD regY = opcode & 0x0F0; regY >>= 4;
    WORD xVal = registers[regX];
    WORD yVal = registers[regY];
    if (yVal >= xVal)
        registers[0xF] = 1;
    registers[regX] = yVal - xVal;
}

void Opcode8XYE(WORD opcode) {
    WORD regX = opcode & 0xF00; regX >>= 8;
    registers[0xF] = registers[regX] & 0x8;
    registers[regX] <<= 1;
}

void Opcode9XY0(WORD opcode) {
    WORD regX = opcode & 0xF00; regX >>= 8;
    WORD regY = opcode & 0x0F0; regY >>= 4;
    if (registers[regX] != registers[regY])
        programCounter += 2;
}

void OpcodeANNN(WORD opcode) {
    addressI = opcode & 0xFFF;
}

void OpcodeBNNN(WORD opcode) {
    programCounter = registers[0x0] + (opcode & 0xFFF);
}

void OpcodeCXNN(WORD opcode) {
    WORD regX = opcode & 0xF00; regX >>= 8;
    registers[regX] = random() & (opcode & 0xFF);
}

void OpcodeDXYN(WORD opcode) {
    WORD regX = opcode & 0x0F00; regX >>= 8;
    WORD regY = opcode & 0x00F0; regY >>= 4;
    BYTE x = registers[regX];
    BYTE y = registers[regY];
    BYTE n = opcode & 0x000F;
    for (int yIndex = 0; yIndex < n; yIndex++){
        BYTE data = gameMemory[addressI + yIndex];
        int bitOffset = 7;
        for (int xIndex = 0; xIndex < 8; xIndex++, bitOffset--) {
            int mask = 1 << bitOffset;
            if (data & mask) {
                int pixelX = x + xIndex;
                int pixelY = y + yIndex;
                if (screenData[pixelX][pixelY] == 1)
                    registers[0xF] = 1;
                screenData[pixelX][pixelY] ^= 1;
            }
        }
    }

}

void OpcodeEX9E(WORD opcode) {
    WORD reg = opcode & 0x0F00; reg >>= 8;
    BYTE key = registers[reg];
    if (input[key]) programCounter += 2;
}

void OpcodeEXA1(WORD opcode) {
    WORD reg = opcode & 0x0F00; reg >>= 8;
    BYTE key = registers[reg];
    if (!input[key]) programCounter += 2;
}

void OpcodeFX07(WORD opcode) {
    WORD reg = opcode & 0x0F00; reg >>= 8;
    registers[reg] = delayTimer;
}

void OpcodeFX0A(WORD opcode) {
    if (awaitKey && keyPressed) {
        WORD reg = opcode & 0x0F00; reg >>= 8;
        awaitKey = false; programCounter += 2; // resume program
        keyPressed = false;
        registers[reg] = recentKey;
    } else {
        awaitKey = true;
        programCounter -= 2;
    }
}

void OpcodeFX15(WORD opcode) {
    WORD reg = opcode & 0x0F00; reg >>= 8;
    delayTimer = registers[reg];
}

void OpcodeFX18(WORD opcode) {
    WORD reg = opcode & 0x0F00; reg >>= 8;
    soundTimer = registers[reg];
}

void OpcodeFX1E(WORD opcode) {
    WORD reg = opcode & 0x0F00; reg >>= 8;
    addressI += registers[reg];
}

void OpcodeFX29(WORD opcode) {
    WORD reg = opcode & 0x0F00; reg >>= 8;
    std::cout << "FX29 called for character: " << std::hex << registers[reg];
    std::cout << "\nYou should make that font map, buddy.\n";
}

void OpcodeFX33(WORD opcode) {
    BYTE number, hundreds, tens, ones;
    WORD reg = opcode & 0x0F00; reg >>= 8;
    number = registers[reg];
    hundreds = number / 100;
    tens = (number % 100) / 10;
    ones = (number % 100) % 10;
    gameMemory[addressI] = hundreds;
    gameMemory[addressI+1] = tens;
    gameMemory[addressI+2] = ones;
}

void OpcodeFX55(WORD opcode) {
    WORD reg = opcode & 0x0F00; reg >>= 8;
    for (int i = 0; i <= reg; i++) {
        gameMemory[addressI + i] = registers[i];
    }
}

void OpcodeFX65(WORD opcode) {
    WORD reg = opcode & 0x0F00; reg >>= 8;
    for (int i = 0; i <= reg; i++) {
        registers[i] = gameMemory[addressI + i];
    }
}

void ErrorInvalidOp(WORD opcode) {
    std::cout << "ERROR: Invalid Opcode: " << std::hex << opcode;
    std::cout << "\n";
}

void ExecOp() {
    WORD opcode = GetNextOpcode();
    switch (opcode & 0xF000) {
        case 0x0000: {
            switch (opcode & 0x0FFF) {
                case 0x00E0: Opcode00E0(); break;
                case 0x00EE: Opcode00EE(); break;
                default: /*Opcode0NNN(opcode);*/ break;
            }
        } break;
        case 0x1000: Opcode1NNN(opcode); break;
        case 0x2000: Opcode2NNN(opcode); break;
        case 0x3000: Opcode3XNN(opcode); break;
        case 0x4000: Opcode4XNN(opcode); break;
        case 0x5000: Opcode5XY0(opcode); break;
        case 0x6000: Opcode6XNN(opcode); break;
        case 0x7000: Opcode7XNN(opcode); break;
        case 0x8000: {
            switch (opcode & 0x000F) {
                case 0x0000: Opcode8XY0(opcode); break;
                case 0x0001: Opcode8XY1(opcode); break;
                case 0x0002: Opcode8XY2(opcode); break;
                case 0x0003: Opcode8XY3(opcode); break;
                case 0x0004: Opcode8XY4(opcode); break;
                case 0x0005: Opcode8XY5(opcode); break;
                case 0x0006: Opcode8XY6(opcode); break;
                case 0x0007: Opcode8XY7(opcode); break;
                case 0x000E: Opcode8XYE(opcode); break;
                default: ErrorInvalidOp(opcode); break;
            }
        } break;
        case 0x9000: Opcode9XY0(opcode); break;
        case 0xA000: OpcodeANNN(opcode); break;
        case 0xB000: OpcodeBNNN(opcode); break;
        case 0xC000: OpcodeCXNN(opcode); break;
        case 0xD000: OpcodeDXYN(opcode); break;
        case 0xE000: {
            switch (opcode & 0x00FF) {
                case 0x009E: OpcodeEX9E(opcode); break;
                case 0x00A1: OpcodeEXA1(opcode); break;
                default: ErrorInvalidOp(opcode); break;
            }
        } break;
        case 0xF000: {
            switch (opcode & 0x00FF) {
                case 0x0007: OpcodeFX07(opcode); break;
                case 0x000A: OpcodeFX0A(opcode);break;
                case 0x0015: OpcodeFX15(opcode);break;
                case 0x0018: OpcodeFX18(opcode);break;
                case 0x001E: OpcodeFX1E(opcode);break;
                case 0x0029: OpcodeFX29(opcode);break;
                case 0x0033: OpcodeFX33(opcode);break;
                case 0x0055: OpcodeFX55(opcode);break;
                case 0x0065: OpcodeFX65(opcode);break;
                default: ErrorInvalidOp(opcode); break;
            }
        } break;
        default: ErrorInvalidOp(opcode); break;
    }

}

void CPUReset() {
    addressI = 0;
    programCounter = 0x200; // program will be initialized at 0x200
    memset(registers, 0, sizeof(registers)); // all reg. to 0

    // load ROM into memory
    FILE *in;
    in = fopen( "/Users/andrew/CLionProjects/chip8-emu/Brix 1990.ch8", "r");
    fread(&gameMemory[0x200], 0xFFF, 1, in);
    fclose(in);
}


int main() {

    SDL_Window *window = nullptr;
    SDL_Renderer *renderer;
    SDL_Event event;

    bool running = true;

    if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
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
            SDL_WINDOW_SHOWN |
            SDL_WINDOW_OPENGL
            );
    if (window == nullptr) {
        std::cout << "Window creation failed: " << SDL_GetError();
        return 1;
    }

    renderer = SDL_CreateRenderer(window, -1, 0);
    if (renderer == nullptr) {
        std::cout << "Renderer creation failed: " << SDL_GetError();
        return 1;
    }

    InitRectArray(rectArray);
    CPUReset();

    while (running) {
        Uint64 start = SDL_GetPerformanceCounter();
        // event polling
        while(SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) { running = false; }
            if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_1) { UpdateKeypressDown(0x00);}
                if (event.key.keysym.sym == SDLK_2) { UpdateKeypressDown(0x01);}
                if (event.key.keysym.sym == SDLK_3) { UpdateKeypressDown(0x02);}
                if (event.key.keysym.sym == SDLK_4) { UpdateKeypressDown(0x03);}
                if (event.key.keysym.sym == SDLK_q) { UpdateKeypressDown(0x04);}
                if (event.key.keysym.sym == SDLK_w) { UpdateKeypressDown(0x05);}
                if (event.key.keysym.sym == SDLK_e) { UpdateKeypressDown(0x06);}
                if (event.key.keysym.sym == SDLK_r) { UpdateKeypressDown(0x07);}
                if (event.key.keysym.sym == SDLK_a) { UpdateKeypressDown(0x08);}
                if (event.key.keysym.sym == SDLK_s) { UpdateKeypressDown(0x09);}
                if (event.key.keysym.sym == SDLK_d) { UpdateKeypressDown(0x0A);}
                if (event.key.keysym.sym == SDLK_f) { UpdateKeypressDown(0x0B);}
                if (event.key.keysym.sym == SDLK_z) { UpdateKeypressDown(0x0C);}
                if (event.key.keysym.sym == SDLK_x) { UpdateKeypressDown(0x0D);}
                if (event.key.keysym.sym == SDLK_c) { UpdateKeypressDown(0x0E);}
                if (event.key.keysym.sym == SDLK_v) { UpdateKeypressDown(0x0F);}
            }
            if (event.type == SDL_KEYUP) {
                if (event.key.keysym.sym == SDLK_1) { UpdateKeypressRelease(0x00);}
                if (event.key.keysym.sym == SDLK_2) { UpdateKeypressRelease(0x01);}
                if (event.key.keysym.sym == SDLK_3) { UpdateKeypressRelease(0x02);}
                if (event.key.keysym.sym == SDLK_4) { UpdateKeypressRelease(0x03);}
                if (event.key.keysym.sym == SDLK_q) { UpdateKeypressRelease(0x04);}
                if (event.key.keysym.sym == SDLK_w) { UpdateKeypressRelease(0x05);}
                if (event.key.keysym.sym == SDLK_e) { UpdateKeypressRelease(0x06);}
                if (event.key.keysym.sym == SDLK_r) { UpdateKeypressRelease(0x07);}
                if (event.key.keysym.sym == SDLK_a) { UpdateKeypressRelease(0x08);}
                if (event.key.keysym.sym == SDLK_s) { UpdateKeypressRelease(0x09);}
                if (event.key.keysym.sym == SDLK_d) { UpdateKeypressRelease(0x0A);}
                if (event.key.keysym.sym == SDLK_f) { UpdateKeypressRelease(0x0B);}
                if (event.key.keysym.sym == SDLK_z) { UpdateKeypressRelease(0x0C);}
                if (event.key.keysym.sym == SDLK_x) { UpdateKeypressRelease(0x0D);}
                if (event.key.keysym.sym == SDLK_c) { UpdateKeypressRelease(0x0E);}
                if (event.key.keysym.sym == SDLK_v) { UpdateKeypressRelease(0x0F);}
            }
        }

        // cpu step (I like the pace of 3 steps/frame usually)
        ExecOp();
        ExecOp();
        ExecOp();

        // render

        SDL_SetRenderDrawColor(renderer,0,0,0,255);
        SDL_RenderClear(renderer);
        UpdateRenderedPixels(renderer);
        SDL_RenderPresent(renderer);

        // update timers
        if (delayTimer) delayTimer--;
        if (soundTimer) {
            // beep boop
            soundTimer--;
        }

        // calc run time delay
        Uint64 end = SDL_GetPerformanceCounter();
        float elapsedMS = (float)(end - start) / (float)SDL_GetPerformanceFrequency() * 1000.0f;
        // 60-ish Hz
        SDL_Delay(floor(16.666f - elapsedMS));
    }
    // cleanup
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
