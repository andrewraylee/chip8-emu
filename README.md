# chip8-emu
- - -
A simple CHIP-8 emulator using SDL2. 

Very much not complete, but also very much does work with most .ch8 ROMs. This was a fun project, but definitely use a different Chip-8 emulator if you're looking for something that's more usable for playing ROMs/personal use.

Currently missing a proper way to load files, so the ROM path is hardcoded in the main.cpp file.

## Building
### Dependencies
- SDL2

The CMakeLists.txt file is configured for the path to the SDL2 library on my system (Apple Silicon; SDL2 install via Homebrew w/ default paths). You may need to change the path to the SDL2 library in the CMakeLists.txt file to match the path on your system.


## Controls
```
CHIP-8 Key | Keyboard Key
-----------|-------------
 1 2 3 4   | 1 2 3 4
 5 6 7 8   | Q W E R
 9 0 A B   | A S D F
 C D E F   | Z X C V
```

## License
Unlicensed. Do whatever you want with this code.

## References and Resources
- [Cowgod's Chip-8 Technical Reference](http://devernay.free.fr/hacks/chip8/C8TECH10.HTM)
- [CHIP-8 Wikipedia](https://en.wikipedia.org/wiki/CHIP-8)
- [Timendus' Chip-8 Test Suite](https://github.com/Timendus/chip8-test-suite) (none of the bugs I was running into with ROMs would have been fixed without this, go check it out!)