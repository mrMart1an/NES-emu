#include "sdl2Input.h"
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_gamecontroller.h>
#include <SDL2/SDL_joystick.h>
#include <SDL2/SDL_keycode.h>
#include <cstdint>
#include <iostream>

namespace input {
// Initialize all value to 0
Sdl2Input::Sdl2Input() : m_latch(0), m_tmpInput(0), m_inputRegister(0) {
    // Get controller if it exist
    joy = SDL_GameControllerOpen(0);
    //for (int i = 0; i < SDL_NumJoysticks(); i++) {
    //    if (SDL_IsGameController(i)) {
    //        mp_controller = SDL_GameControllerOpen(i);
    //    }
    //}
}

void Sdl2Input::updateInput(SDL_Event* event) {
    uint8_t keyStatus = (event->type == SDL_KEYUP) ? 0x00 : 0x01;
    keyStatus *= (event->type == SDL_CONTROLLERBUTTONUP) ? 0x00 : 0x01;

    // Keyboard input
    if (event->type == SDL_KEYDOWN || event->type == SDL_KEYUP) {
        switch (event->key.keysym.sym) {
            case SDLK_z:
                m_tmpInput = (m_tmpInput & ~GP_A) | (GP_A * keyStatus);
                break;
            case SDLK_x:
                m_tmpInput = (m_tmpInput & ~GP_B) | (GP_B * keyStatus);
                break;

            case SDLK_w: case SDLK_UP:
                m_tmpInput = (m_tmpInput & ~GP_UP) | (GP_UP * keyStatus);
                break;
            case SDLK_s: case SDLK_DOWN:
                m_tmpInput = (m_tmpInput & ~GP_DOWN) | (GP_DOWN * keyStatus);
                break;
            case SDLK_a: case SDLK_LEFT:
                m_tmpInput = (m_tmpInput & ~GP_LEFT) | (GP_LEFT * keyStatus);
                break;
            case SDLK_d: case SDLK_RIGHT:
                m_tmpInput = (m_tmpInput & ~GP_RIGHT) | (GP_RIGHT * keyStatus);
                break;

            case SDLK_RETURN:
                m_tmpInput = (m_tmpInput & ~GP_START) | (GP_START * keyStatus);
                break;
            case SDLK_SPACE:
                m_tmpInput = (m_tmpInput & ~GP_SELECT) | (GP_SELECT * keyStatus);
                break;
        }
    }

    // #TODO key mapping
    // Joypad input
    if (event->type == SDL_CONTROLLERBUTTONDOWN || event->type == SDL_CONTROLLERBUTTONUP) {
        switch (event->cbutton.button) {
            case SDL_CONTROLLER_BUTTON_A:
                m_tmpInput = (m_tmpInput & ~GP_A) | (GP_A * keyStatus);
                break;
            case SDL_CONTROLLER_BUTTON_B:
                m_tmpInput = (m_tmpInput & ~GP_B) | (GP_B * keyStatus);
                break;

            case SDL_CONTROLLER_BUTTON_DPAD_UP:
                m_tmpInput = (m_tmpInput & ~GP_UP) | (GP_UP * keyStatus);
                break;
            case SDL_CONTROLLER_BUTTON_DPAD_DOWN:
                m_tmpInput = (m_tmpInput & ~GP_DOWN) | (GP_DOWN * keyStatus);
                break;
            case SDL_CONTROLLER_BUTTON_DPAD_LEFT:
                m_tmpInput = (m_tmpInput & ~GP_LEFT) | (GP_LEFT * keyStatus);
                break;
            case SDL_CONTROLLER_BUTTON_DPAD_RIGHT:
                m_tmpInput = (m_tmpInput & ~GP_RIGHT) | (GP_RIGHT * keyStatus);
                break;

            case SDL_CONTROLLER_BUTTON_START:
                m_tmpInput = (m_tmpInput & ~GP_START) | (GP_START * keyStatus);
                break;
            case SDL_CONTROLLER_BUTTON_BACK:
                m_tmpInput = (m_tmpInput & ~GP_SELECT) | (GP_SELECT * keyStatus);
                break;
        }
    }
}

void Sdl2Input::writeOutput(uint8_t data) {
    m_latch = data & 0b00000001;

    // If the latch is set
    // Update the input register with the sdl input data
    if (m_latch == 0x01) 
        m_inputRegister = m_tmpInput;
}

uint8_t Sdl2Input::readInputOne() {
    uint8_t outputBit = m_inputRegister & 0x01;

    // Shift the input register
    m_inputRegister >>= 1;

    // If the latch is set
    // Update the input register with the sdl input data
    if (m_latch == 0x01) 
        m_inputRegister = m_tmpInput;

    return outputBit;
}

// Disable the game pad two
uint8_t Sdl2Input::readInputTwo() {
    return 0x00;
}
}
