cmake_minimum_required(VERSION 3.2)
project(nes_emu)

include_directories(include)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "-Wall -Winline -g")

set(SOURCE_FILES 
    src/main.cpp
)

set(SOURCE_FILES_CORE
    src/nesCore/cpuBus.cpp
    src/nesCore/ppuBus.cpp
    src/nesCore/nesEmulator.cpp
    src/nesCore/frameBuffer.cpp

    src/nesCore/inputOutput/dummyIO.cpp

    src/nesCore/cpu/cpu6502.cpp
    src/nesCore/cpu/cpu6502op.cpp
    src/nesCore/cpu/cpu6502debug.cpp

    src/nesCore/ppu/ppu.cpp
    src/nesCore/ppu/ppuDebug.cpp

    src/nesCore/cartridge/nromCartridge.cpp
    src/nesCore/cartridge/cnromCartridge.cpp

    src/nesCore/utility/utilityFunctions.cpp
)

set(SOURCE_FILE_SDL2
    src/sdl2/sdl2Display.cpp    
    src/sdl2/sdl2Input.cpp    
)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
find_package(SDL2)

add_executable(nes_emu 
    ${SOURCE_FILES} 
    ${SOURCE_FILES_CORE} 
    ${SOURCE_FILE_SDL2}
)
target_link_libraries(nes_emu SDL2::Main)
