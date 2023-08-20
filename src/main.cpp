#include <SDL2/SDL_events.h>
#include <SDL2/SDL_keycode.h>
#include <SDL2/SDL_video.h>
#include <SDL2/SDL.h>
#include <iostream>

#include "nesCore/inputOutput/IOInterface.h"
#include "nesCore/nesEmulator.h"
#include "nesCore/cpu/cpu6502.h"
#include "nesCore/cpu/cpu6502debug.h"

#include "nesCore/ppu/ppuDebug.h"
#include "nesCore/utility/utilityFunctions.h"
#include "sdl2/sdl2Display.h"
#include "sdl2/sdl2Input.h"

int main (int argc, char *argv[]) {
    // Init Sdl2 
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cout << "Failed to initialized SDL2" << std::endl;
        return -1;
    }

    display::Sdl2Display display;
    display.init();

    input::Sdl2Input sdlGamepad;

    nesCore::NesEmulator emulator = nesCore::NesEmulator();
    emulator.loadPalette("palettes/2C02G.pal");
    
    emulator.loadCartridge("roms/scroll.nes");
    //emulator.loadCartridge("roms/Zelda.NES");
    //emulator.loadCartridge("roms/DonkeyKong.nes");
    //emulator.loadCartridge("roms/nestest.nes");
    //emulator.loadCartridge("roms/helloworld.nes");

    emulator.attachIO(&sdlGamepad);
    display.attackFrameBuffer(emulator.getFrameBuffer());

    bool quit = false;
    bool emulate = true;
    SDL_Event event;

    while (!quit) {
        // Prepare a frame
        while (!emulator.frameReady() && emulate) {
            emulator.step();

            if (false) {
                nesCore::debug::Cpu6502Debug info = emulator.cpuDebugInfo(); 
                nesCore::debug::PPUDebug infoPPU = emulator.ppuDebugInfo();

                if (true || (info.cpuCycle >= 000 && info.cpuCycle <= 2000)) {
                    std::cout << info.log() << " -- ";
                    std::cout << emulator.decompileInstruction(info.pc) << std::endl;
                }
                //std::cout << infoPPU.log() << std::endl;
            }
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
            if ( event.type == SDL_WINDOWEVENT) {
                switch ( event.window.event ) {
                    case SDL_WINDOWEVENT_RESIZED:
                        display.resize();
                        break;
                }
            }

            if ( event.type == SDL_KEYDOWN ) {
                if (event.key.keysym.sym == SDLK_p)
                    emulate = !emulate;

                // Run one emulator step
                if (event.key.keysym.sym == SDLK_t && !emulate) {
                    emulator.step();

                    nesCore::debug::Cpu6502Debug info = emulator.cpuDebugInfo(); 
                    nesCore::debug::PPUDebug infoPPU = emulator.ppuDebugInfo();

                    std::cout << info.log() << " -- ";
                    std::cout << emulator.decompileInstruction(info.pc) << std::endl;
                }
                // Render one frame
                if (event.key.keysym.sym == SDLK_f && !emulate) {
                     while (!emulator.frameReady()) {
                        emulator.step();

                        nesCore::debug::Cpu6502Debug info = emulator.cpuDebugInfo(); 
                        nesCore::debug::PPUDebug infoPPU = emulator.ppuDebugInfo();

                        std::cout << info.log() << " -- ";
                        std::cout << emulator.decompileInstruction(info.pc) << std::endl;
                        
                        std::cout << emulator.formatBusRange(0x0000, 0x07, 8) << std::endl;
                     }
                }
            }
        }
    }

    std::cout << emulator.formatBusRange(0x0100, 0x01F0, 8) << std::endl;

    display.quit();
    SDL_Quit();
    return 0;
}
