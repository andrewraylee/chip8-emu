cmake_minimum_required(VERSION 3.27)
project(chip8-emu)

set(CMAKE_CXX_STANDARD 23)

include_directories(/opt/homebrew/Cellar/sdl2/2.30.0/include/)
link_directories(/opt/homebrew/Cellar/sdl2/2.30.0/lib)

add_executable(chip8-emu main.cpp
        types.h
        Chip8Core.cpp
        Chip8Core.h
        Chip8Display.cpp
        Chip8Display.h)
target_link_libraries(chip8-emu sdl2)