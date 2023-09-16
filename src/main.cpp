#include <argparse/argparse.hpp>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_keycode.h>
#include <SDL2/SDL_video.h>
#include <SDL2/SDL.h>
#include <iostream>
#include <string>

#include "nesCore/inputOutput/IOInterface.h"
#include "nesCore/nesEmulator.h"
#include "nesCore/cpu/cpu6502.h"
#include "nesCore/cpu/cpu6502debug.h"

#include "nesCore/ppu/ppuDebug.h"
#include "nesCore/utility/utilityFunctions.h"
#include "sdl2/sdl2Display.h"
#include "sdl2/sdl2Input.h"

int main (int argc, char *argv[]) {
    // Parse commands line arguments
    argparse::ArgumentParser argParser("nes_emu");

    argParser.add_argument("romPath")
        .help("specify the ROM file path");

    argParser.add_argument("-p", "--palettes")
        .default_value(std::string("palettes/2C02G.pal"))
        .required()
        .help("specify the color palettes file");

    argParser.add_argument("-w", "--windowed")
        .implicit_value(true)
        .default_value(false)
        .help("start the emulator as a window");

    // Attempt to parse the arguments
    try {
        argParser.parse_args(argc, argv);
    }
    catch (const std::runtime_error& err) {
        std::cerr << err.what() << std::endl;
        std::cerr << argParser;
        return 1;
    }    

    // Program options
    std::string romPath = argParser.get("romPath");
    std::string palettePath = argParser.get("palettes");
    bool windowed = argParser.get<bool>("windowed");

    // SDL2 initialization 
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER) < 0) {
        std::cerr << "Failed to initialized SDL2" << std::endl;
        return 2;
    }

    display::Sdl2Display display;
    display.init();

    if (!windowed)
        display.toggleFullscreen();

    input::Sdl2Input sdlGamepad;

    // Emulator initialization
    nesCore::NesEmulator emulator = nesCore::NesEmulator();

    // Attempt to load the emulator color palette
    int paletteErr = emulator.loadPalette(palettePath);
    if (paletteErr != 0) {
        std::cerr << "Failed to load color palette, error code: ";
        std::cerr << paletteErr << std::endl;
        return 3;
    }
    
    // Attempt to load the game ROM
    int cartErr = emulator.loadCartridge(romPath);
    if (cartErr != 0){
        std::cerr << "Failed to load game ROM, error code: ";
        std::cerr << cartErr << std::endl;
        return 4;
    }

    // Attach sdl components to the emulator
    emulator.attachIO(&sdlGamepad);
    display.attackFrameBuffer(emulator.getFrameBuffer());

    bool quit = false;
    bool emulate = true;
    SDL_Event event;

    while (!quit) {
        // Prepare a frame
        while (!emulator.frameReady() && emulate) {
            emulator.step();
        }
    
        // Update the display
        display.update();
    
        // Handle event queue
        while (SDL_PollEvent(&event) != 0) {
            sdlGamepad.updateInput(&event);

            // Handle quit event
            if ( event.type == SDL_QUIT )
                quit = true;
            // Handle resize event
            if ( event.type == SDL_WINDOWEVENT ) {
                switch ( event.window.event ) {
                    case SDL_WINDOWEVENT_RESIZED:
                        display.resize();
                        break;
                }
            }

            // Toggle window fullscreen
            if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_F11)
                display.toggleFullscreen();

            // Reset the emulator
            if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_F5)
                emulator.reset();

            if ( event.type == SDL_KEYDOWN ) {
                if (event.key.keysym.sym == SDLK_p)
                    emulate = !emulate;

                // Run one emulator step
                if (event.key.keysym.sym == SDLK_t && !emulate) {
                    emulator.step();

                    nesCore::debug::Cpu6502Debug info = emulator.cpuDebugInfo(); 

                    std::cout << info.log() << " -- ";
                    std::cout << emulator.decompileInstruction(info.pc) << std::endl;
                }
                // Render one frame
                if (event.key.keysym.sym == SDLK_f && !emulate) {
                     while (!emulator.frameReady()) {
                        emulator.step();

                        nesCore::debug::Cpu6502Debug info = emulator.cpuDebugInfo(); 

                        std::cout << info.log() << " -- ";
                        std::cout << emulator.decompileInstruction(info.pc) << std::endl;
                     }
                }
            }
        }
    }

    std::cout << emulator.formatBusRange(0x0200, 0x02FF, 16) << std::endl;

    display.quit();
    SDL_Quit();
    return 0;
}
