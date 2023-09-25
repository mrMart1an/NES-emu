cmake_minimum_required(VERSION 3.2)
project(nes_emu)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-Winline -O3")

set(SOURCE_FILES 
    src/main.cpp
    src/argumentParser.cpp
)

set(SOURCE_FILES_CORE
    src/nesCore/cpuBus.cpp
    src/nesCore/ppuBus.cpp
    src/nesCore/nesEmulator.cpp
    src/nesCore/frameBuffer.cpp

    src/nesCore/inputOutput/dummyIO.cpp

    src/nesCore/cpu/cpu6502.cpp
    src/nesCore/cpu/cpu6502debug.cpp

    src/nesCore/ppu/ppu.cpp
    src/nesCore/ppu/ppuDebug.cpp


    src/nesCore/cartridge/cartridge.cpp
    src/nesCore/cartridge/nromCartridge.cpp
    src/nesCore/cartridge/cnromCartridge.cpp

    src/nesCore/utility/utilityFunctions.cpp
)

set(SOURCE_FILE_SDL2
    src/sdl2/sdl2Display.cpp    
    src/sdl2/sdl2Input.cpp    
)

# External library source files
set(EXTERN_SRC
    extern/glad/src/glad.c
)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
find_package(SDL2)

# Include external library headers folders
include_directories(extern/argparse/include)
include_directories(extern/glad/include)

# Allow include relative to src in the code base
include_directories(src)

# Create an executable
add_executable(nes_emu 
    ${SOURCE_FILES} 
    ${SOURCE_FILES_CORE} 
    ${SOURCE_FILE_SDL2}
    ${EXTERN_SRC}
)

# Output the binary in the binary folder
set_target_properties(
    nes_emu PROPERTIES 
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin"
)

# Dynamic linking to the SDL library
target_link_libraries(nes_emu SDL2::Main)

# Use pre-compiled headers
target_precompile_headers(nes_emu PRIVATE src/nesPch.h)

# Post build commands
# Copy resources in build directory
add_custom_command(
    TARGET nes_emu POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory              
         ${CMAKE_SOURCE_DIR}/resources $<TARGET_FILE_DIR:nes_emu>/resources
    COMMENT "Copying resources" VERBATIM
)

# Export compile commands in the root directory
add_custom_target(
    copy-compile-commands ALL
    ${CMAKE_COMMAND} -E copy_if_different
    ${CMAKE_BINARY_DIR}/compile_commands.json
    ${CMAKE_SOURCE_DIR}
    COMMENT "Copying compile commands" VERBATIM
)
