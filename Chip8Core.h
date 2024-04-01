#ifndef CHIP8_EMU_CHIP8CORE_H
#define CHIP8_EMU_CHIP8CORE_H

#include "types.h"
#include <vector>

class Chip8Core {
public:
    Chip8Core();

    void Initialize();
    void LoadGame(char *gamePath);
    void EmulateCycle();
    void SetKeys(BYTE key, bool state);
    void DecrementTimers();
    [[nodiscard]] bool ShouldPlaySound() const;
    BYTE (*GetScreenData())[64][32];

private:
    static const unsigned int MEMORY_SIZE = 4096;
    static const BYTE REGISTER_COUNT = 16;

    const BYTE chip8FontSet[80] =
            {
                    0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
                    0x20, 0x60, 0x20, 0x20, 0x70, // 1
                    0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
                    0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
                    0x90, 0x90, 0xF0, 0x10, 0x10, // 4
                    0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
                    0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
                    0xF0, 0x10, 0x20, 0x40, 0x40, // 7
                    0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
                    0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
                    0xF0, 0x90, 0xF0, 0x90, 0x90, // A
                    0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
                    0xF0, 0x80, 0x80, 0x80, 0xF0, // C
                    0xE0, 0x90, 0x90, 0x90, 0xE0, // D
                    0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
                    0xF0, 0x80, 0xF0, 0x80, 0x80  // F
            };

    BYTE memory[MEMORY_SIZE]{}; // 4096
    BYTE V[REGISTER_COUNT]{}; // Registers V0-VF
    WORD I{}; // Index
    WORD pc{}; // Program counter
    std::vector<WORD> stack;
    BYTE screenData[64][32]{}; // Display memory, 64x32 pixels
    BYTE keys[16]{}; // Keypad input state, 0x0-0xF
    BYTE delayTimer{};
    BYTE soundTimer{};
    WORD opcode{};

    void LoadFontSet();
    void ClearScreen();

    void GetNextOpcode();
    void ExecuteOpcode();

    void Opcode00E0();
    void Opcode00EE();
    void Opcode1NNN(WORD op);
    void Opcode2NNN(WORD op);
    void Opcode3XNN(WORD op);
    void Opcode4XNN(WORD op);
    void Opcode5XY0(WORD op);
    void Opcode6XNN(WORD op);
    void Opcode7XNN(WORD op);
    void Opcode8XY0(WORD op);
    void Opcode8XY1(WORD op);
    void Opcode8XY2(WORD op);
    void Opcode8XY3(WORD op);
    void Opcode8XY4(WORD op);
    void Opcode8XY5(WORD op);
    void Opcode8XY6(WORD op);
    void Opcode8XY7(WORD op);
    void Opcode8XYE(WORD op);
    void Opcode9XY0(WORD op);
    void OpcodeANNN(WORD op);
    void OpcodeBNNN(WORD op);
    void OpcodeCXNN(WORD op);
    void OpcodeDXYN(WORD op);
    void OpcodeEX9E(WORD op);
    void OpcodeEXA1(WORD op);
    void OpcodeFX07(WORD op);
    void OpcodeFX0A(WORD op);
    void OpcodeFX15(WORD op);
    void OpcodeFX18(WORD op);
    void OpcodeFX1E(WORD op);
    void OpcodeFX29(WORD op);
    void OpcodeFX33(WORD op);
    void OpcodeFX55(WORD op);
    void OpcodeFX65(WORD op);

    static void ErrorInvalidOp(WORD op);
};


#endif //CHIP8_EMU_CHIP8CORE_H
