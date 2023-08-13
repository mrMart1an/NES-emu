#include "sdl2Input.h"
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_keycode.h>
#include <cstdint>

namespace input {
// Initialize all value to 0
Sdl2Input::Sdl2Input() : m_latch(0), m_tmpInput(0), m_inputRegister(0) {}

void Sdl2Input::updateInput(SDL_Event* event) {
    uint8_t keyStatus = (event->type == SDL_KEYUP) ? 0x00 : 0x01;

    if (event->type == SDL_KEYDOWN || event->type == SDL_KEYUP) {
        switch (event->key.keysym.sym) {
            case SDLK_w: case SDLK_UP:
                m_tmpInput = (m_tmpInput & ~GP_UP) | (GP_UP * keyStatus);
                break;
            case SDLK_s: case SDLK_DOWN:
                m_tmpInput = (m_tmpInput & ~GP_DOWN) | (GP_DOWN * keyStatus);
                break;
            case SDLK_RETURN:
                m_tmpInput = (m_tmpInput & ~GP_START) | (GP_START * keyStatus);
                break;
            case SDLK_SPACE:
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
