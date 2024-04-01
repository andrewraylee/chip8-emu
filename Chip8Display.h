#ifndef CHIP8_EMU_CHIP8DISPLAY_H
#define CHIP8_EMU_CHIP8DISPLAY_H

#include <SDL2/SDL_rect.h>
#include <SDL2/SDL_render.h>
#include "types.h"
#include "Chip8Core.h"

class Chip8Display {
public:
    static void Initialize();
    static void RenderFrame(Chip8Core &chip8Core);
private:
    static void InitRectArray(SDL_Rect pRect[64][32]);
    static void UpdateScreen(SDL_Renderer *pRenderer, Chip8Core &chip8Core);
    static void RenderScreen(SDL_Renderer *pRenderer);
    static void ClearScreen(SDL_Renderer *pRenderer);
};


#endif //CHIP8_EMU_CHIP8DISPLAY_H
