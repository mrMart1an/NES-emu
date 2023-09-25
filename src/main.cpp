#include "nesPch.h"

#include <SDL2/SDL_events.h>
#include <SDL2/SDL_keycode.h>

#include "nesCore/utility/utilityFunctions.h"
#include "nesCore/inputOutput/IOInterface.h"
#include "nesCore/nesEmulator.h"
#include "nesCore/cpu/cpu6502.h"

#include "nesCore/cpu/cpu6502debug.h"
#include "nesCore/ppu/ppuDebug.h"

#include "sdl2/sdl2Display.h"
#include "sdl2/sdl2Input.h"

#include "argumentParser.h"

int main(int argc, char *argv[]) {
    // SDL2 initialization 
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER) < 0) {
        std::cerr << "Failed to initialized SDL2" << std::endl;
        return 2;
    }

    // Argument parsing
    AppOptions options = parseArguments(argc, argv);
 
    // Emulator initialization
    nesCore::NesEmulator emulator = nesCore::NesEmulator();

    // Setup the emulator
    int emuSetupError = emulator.setup(
        options.romPath, 
        options.palettePath
    );

    if (emuSetupError != 0)
        return emuSetupError;

    // Setup display
    display::Sdl2Display display;
    int success = display.init(
        options.hideDangerZone,
        options.windowed,
        options.useVsync,
        "resources/shaders/shader.vert",
        "resources/shaders/shader.frag"
    );

    if (success != 0) {
        std::cerr << "Failed to initialized display" << std::endl;
        return 3;
    }

    display.attachFrameBuffer(emulator.getFrameBuffer());

    // Input setup
    input::Sdl2Input sdlGamepad;
    emulator.attachIO(&sdlGamepad);

    // Emulator main loop
    bool quit = false;
    bool runEmulation = true;

    SDL_Event event;

    while (!quit) {
        // Prepare a frame
        while (!emulator.frameReady() && runEmulation) {
            emulator.step();
        }

        // Update the display
        display.update();
    
        // Handle event in queue
        while (SDL_PollEvent(&event)) {
            // Set all event to the sdl gamepad implementation
            sdlGamepad.updateInput(&event);

            // Handle resize event
            if (event.type == SDL_WINDOWEVENT) {
                switch (event.window.event) {
                    case SDL_WINDOWEVENT_RESIZED:
                        display.resize();
                        break;
                }
            }

            if (event.type == SDL_KEYDOWN) {
                // Toggle window fullscreen
                if (event.key.keysym.sym == SDLK_F11)
                    display.toggleFullscreen();

                // Toggle vsync 
                if (event.key.keysym.sym == SDLK_F8)
                    display.toggleVsync();

                // Reset the emulator
                if (event.key.keysym.sym == SDLK_F5)
                    emulator.reset();
    
                if (event.key.keysym.sym == SDLK_p)
                    runEmulation = !runEmulation;

                // Render one frame
                if (event.key.keysym.sym == SDLK_f && !runEmulation) {
                    while (!emulator.frameReady()) {
                        emulator.step();
                    }
                }

                // Run one emulator step and print debug info
                if (event.key.keysym.sym == SDLK_t && !runEmulation) {
                    emulator.step();

                    nesCore::debug::Cpu6502Debug info = emulator.cpuDebugInfo(); 

                    std::cout << info.log() << " -- ";
                    std::cout << emulator.decompileInstruction(info.pc) << std::endl;
                }
            }

            // Handle quit event
            if (event.type == SDL_QUIT)
                quit = true;
        }
    }

    display.quit();
    SDL_Quit();

    return 0;
}
