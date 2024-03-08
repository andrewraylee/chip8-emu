#include "Chip8Core.h"
#include <iostream>
#include <ctime>

Chip8Core::Chip8Core() {
    Initialize();
}

Chip8Core::~Chip8Core() = default;

void Chip8Core::Initialize() {
    I = 0; // reset index
    pc = 0x200; // program will be initialized at 0x200
    memset(V, 0, sizeof(V)); // all V-reg. to 0
    opcode = 0; // reset opcode
    stack.clear(); // clear stack
    ClearScreen(); // clear screen
    LoadFontSet(); // load font set
    memset(keys, 0, sizeof(keys)); // clear keys
    delayTimer = 0; // reset timers
    soundTimer = 0;

    srand(time(nullptr)); // seed rng | NOLINT(*-msc51-cpp) [this is good enough in this scope]

    LoadGame((char *)"/Users/andrew/CLionProjects/chip8-emu/Test Opcode.ch8"); // load game
}

void Chip8Core::LoadGame(char *gamePath) {
    // load ROM into memory
    FILE *in;
    in = fopen(gamePath, "r");
    fread(&memory[0x200], 4096, 1, in);
    fclose(in);
}

void Chip8Core::EmulateCycle() {
    GetNextOpcode();
    ExecuteOpcode();
    DecrementTimers();
    std::cout << "PC: " << std::hex << pc << std::endl;
    std::cout << "I: " << std::hex << I << std::endl;
    std::cout << "opcode: " << std::hex << opcode << std::endl;
}

void Chip8Core::SetKeys(BYTE key, bool state) {
    this->keys[key] = state;
}

void Chip8Core::DecrementTimers() {
    if (delayTimer > 0) delayTimer--;

    if (soundTimer > 0) soundTimer--;
}

bool Chip8Core::ShouldPlaySound() const {
    return soundTimer > 0;
}

BYTE (*Chip8Core::GetScreenData())[64][32] { // this is ***wild*** after months of JS
    return &screenData;
}

void Chip8Core::GetNextOpcode() {
    opcode = memory[pc]; // 0x00AB
    opcode <<= 8; // 0xAB00
    opcode |= memory[pc + 1]; // 0xABCD
    pc += 2; // increment program counter
}

void Chip8Core::ExecuteOpcode() {
    switch (opcode & 0xF000) {
        case 0x0000:
            switch (opcode & 0x0FFF) {
                case 0x00E0: Opcode00E0(); break;
                case 0x00EE: Opcode00EE(); break;
                default: ErrorInvalidOp(opcode); break;
            }
        break;
        case 0x1000: Opcode1NNN(opcode); break;
        case 0x2000: Opcode2NNN(opcode); break;
        case 0x3000: Opcode3XNN(opcode); break;
        case 0x4000: Opcode4XNN(opcode); break;
        case 0x5000: Opcode5XY0(opcode); break;
        case 0x6000: Opcode6XNN(opcode); break;
        case 0x7000: Opcode7XNN(opcode); break;
        case 0x8000:
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
        break;
        case 0x9000: Opcode9XY0(opcode); break;
        case 0xA000: OpcodeANNN(opcode); break;
        case 0xB000: OpcodeBNNN(opcode); break;
        case 0xC000: OpcodeCXNN(opcode); break;
        case 0xD000: OpcodeDXYN(opcode); break;
        case 0xE000:
            switch (opcode & 0x00FF) {
                case 0x009E: OpcodeEX9E(opcode); break;
                case 0x00A1: OpcodeEXA1(opcode); break;
                default: ErrorInvalidOp(opcode); break;
            }
        break;
        case 0xF000:
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
        break;
        default: ErrorInvalidOp(opcode); break;
    }
}

void Chip8Core::LoadFontSet() {
    for (int i = 0; i < 80; ++i) {
        memory[i] = chip8FontSet[i];
    }
}

void Chip8Core::ClearScreen() {
    memset(screenData, 0, sizeof(screenData));
}

void Chip8Core::Opcode00E0() {
    ClearScreen();
}

void Chip8Core::Opcode00EE() {
    pc = stack.back();
    stack.pop_back();
}

void Chip8Core::Opcode1NNN(WORD op) {
    WORD address = op & 0x0FFF;
    pc = address;
}

void Chip8Core::Opcode2NNN(WORD op) {
    stack.push_back(pc);
    WORD address = op & 0x0FFF;
    pc = address;
}

void Chip8Core::Opcode3XNN(WORD op) {
    WORD regIndex = (op & 0x0F00) >> 8;
    BYTE value = op & 0x00FF;
    if (V[regIndex] == value)
        pc += 2;
}

void Chip8Core::Opcode4XNN(WORD op) {
    WORD regIndex = (op & 0x0F00) >> 8;
    BYTE value = op & 0x00FF;
    if (V[regIndex] != value)
        pc += 2;
}

void Chip8Core::Opcode5XY0(WORD op) {
    WORD regX = (op & 0x0F00) >> 8;
    WORD regY = (op & 0x00F0) >> 4;
    if (V[regX] == V[regY])
        pc += 2;
}

void Chip8Core::Opcode6XNN(WORD op) {
    WORD regIndex = (op & 0x0F00) >> 8;
    BYTE value = op & 0x00FF;
    V[regIndex] = value;
}

void Chip8Core::Opcode7XNN(WORD op) {
    WORD regIndex = (op & 0x0F00) >> 8;
    BYTE value = op & 0x00FF;
    V[regIndex] += value;
}

void Chip8Core::Opcode8XY0(WORD op) {
    WORD regX = (op & 0x0F00) >> 8;
    WORD regY = (op & 0x00F0) >> 4;
    V[regX] = V[regY];
}

void Chip8Core::Opcode8XY1(WORD op) {
    WORD regX = (op & 0x0F00) >> 8;
    WORD regY = (op & 0x00F0) >> 4;
    V[regX] |= V[regY];
}

void Chip8Core::Opcode8XY2(WORD op) {
    WORD regX = (op & 0x0F00) >> 8;
    WORD regY = (op & 0x00F0) >> 4;
    V[regX] &= V[regY];
}

void Chip8Core::Opcode8XY3(WORD op) {
    WORD regX = (op & 0x0F00) >> 8;
    WORD regY = (op & 0x00F0) >> 4;
    V[regX] ^= V[regY];
}

void Chip8Core::Opcode8XY4(WORD op) {
    WORD regX = (op & 0x0F00) >> 8;
    WORD regY = (op & 0x00F0) >> 4;
    WORD result = V[regX] + V[regY];
    V[0xF] = (result > 0xFF) ? 1 : 0; // carry (Love to use a good ol' *squints eyes* tyranny operator)
    V[regX] = result & 0xFF;
}

void Chip8Core::Opcode8XY5(WORD op) {
    WORD regX = (op & 0x0F00) >> 8;
    WORD regY = (op & 0x00F0) >> 4;
    V[0xF] = (V[regX] > V[regY]) ? 1 : 0;
    V[regX] -= V[regY];
}

void Chip8Core::Opcode8XY6(WORD op) {
    WORD regX = (op & 0x0F00) >> 8;
    V[0xF] = V[regX] & 0x1;
    V[regX] >>= 1;
}

void Chip8Core::Opcode8XY7(WORD op) {
    WORD regX = (op & 0x0F00) >> 8;
    WORD regY = (op & 0x00F0) >> 4;
    V[0xF] = (V[regY] > V[regX]) ? 1 : 0;
    V[regX] = V[regY] - V[regX];
}

void Chip8Core::Opcode8XYE(WORD op) {
    WORD regX = (op & 0x0F00) >> 8;
    V[0xF] = (V[regX] >> 7) & 0x1;
    V[regX] <<= 1;
}

void Chip8Core::Opcode9XY0(WORD op) {
    WORD regX = (op & 0x0F00) >> 8;
    WORD regY = (op & 0x00F0) >> 4;
    if (V[regX] != V[regY])
        pc += 2;
}

void Chip8Core::OpcodeANNN(WORD op) {
    I = op & 0x0FFF;
}

void Chip8Core::OpcodeBNNN(WORD op) {
    pc = (op & 0x0FFF) + V[0];
}

void Chip8Core::OpcodeCXNN(WORD op) {
    WORD regX = (op & 0x0F00) >> 8;
    BYTE value = op & 0x00FF;
    V[regX] = random() & value;
}

void Chip8Core::OpcodeDXYN(WORD op) {
    WORD regX = (op & 0x0F00) >> 8;
    WORD regY = (op & 0x00F0) >> 4;
    WORD height = op & 0x000F;
    WORD x = V[regX];
    WORD y = V[regY];
    V[0xF] = 0;
    for (int yLine = 0; yLine < height; yLine++) {
        BYTE pixel = memory[I + yLine];
        for (int xLine = 0; xLine < 8; xLine++) {
            if ((pixel & (0x80 >> xLine)) != 0) {
                if (screenData[x + xLine][y + yLine]) {
                    V[0xF] = 1;
                }
                screenData[x + xLine][y + yLine] ^= 1;
            }
        }
    }
}

void Chip8Core::OpcodeEX9E(WORD op) {
    WORD regX = (op & 0x0F00) >> 8;
    if (keys[V[regX]]) {
        pc += 2;
    }
}

void Chip8Core::OpcodeEXA1(WORD op) {
    WORD regX = (op & 0x0F00) >> 8;
    if (!keys[V[regX]]) {
        pc += 2;
    }
}

void Chip8Core::OpcodeFX07(WORD op) {
    WORD regX = (op & 0x0F00) >> 8;
    V[regX] = delayTimer;
}

void Chip8Core::OpcodeFX0A(WORD op) {
    WORD regX = (op & 0x0F00) >> 8;
    bool keyPressed = false;
    for (int i = 0; i < 16; i++) {
        if (keys[i]) {
            V[regX] = i;
            keyPressed = true;
        }
    }
    if (!keyPressed) {
        pc -= 2;
    }
}

void Chip8Core::OpcodeFX15(WORD op) {
    WORD regX = (op & 0x0F00) >> 8;
    delayTimer = V[regX];
}

void Chip8Core::OpcodeFX18(WORD op) {
    WORD regX = (op & 0x0F00) >> 8;
    soundTimer = V[regX];
}

void Chip8Core::OpcodeFX1E(WORD op) {
    WORD regX = (op & 0x0F00) >> 8;
    I += V[regX];
}

void Chip8Core::OpcodeFX29(WORD op) {
    WORD regX = (op & 0x0F00) >> 8;
    I = V[regX] * 5;
}

void Chip8Core::OpcodeFX33(WORD op) {
    WORD regX = (op & 0x0F00) >> 8;
    memory[I] = V[regX] / 100; // hundreds
    memory[I + 1] = (V[regX] / 10) % 10; // tens
    memory[I + 2] = V[regX] % 10; // ones
}

void Chip8Core::OpcodeFX55(WORD op) {
    WORD regX = (op & 0x0F00) >> 8;
    for (int i = 0; i <= regX; i++) {
        memory[I + i] = V[i];
    }
}

void Chip8Core::OpcodeFX65(WORD op) {
    WORD regX = (op & 0x0F00) >> 8;
    for (int i = 0; i <= regX; i++) {
        V[i] = memory[I + i];
    }
}

void Chip8Core::ErrorInvalidOp(WORD op) {
    std::cerr << "Invalid opcode: " << std::hex << op << std::endl;
    exit(1);
}
