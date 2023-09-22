#ifndef SDL2_INPUT_H_
#define SDL2_INPUT_H_

#include "../nesPch.h"

#include "../nesCore/inputOutput/IOInterface.h"
#include <SDL2/SDL_events.h>

namespace input {

enum GAMEPAD_BIT {
    GP_A = 0b00000001,
    GP_B = 0b00000010,
    GP_SELECT = 0b00000100,
    GP_START = 0b00001000,
    GP_UP = 0b00010000,
    GP_DOWN = 0b00100000,
    GP_LEFT = 0b01000000,
    GP_RIGHT = 0b10000000,
};

enum JOYSTICK_DIRECTION {
    STICK_UP = 0b0001,
    STICK_DOWN = 0b0010,
    STICK_LEFT = 0b0100,
    STICK_RIGHT = 0b1000,
};

class Sdl2Input: public nesCore::IOInterface {
    // Joystick movement control threshold
    const int32_t JOYSTICK_THRESHOLD = 20000;

public:
    Sdl2Input();

    // Write on the output port
    void writeOutput(uint8_t data) override;

    // Read data on the input port
    uint8_t readInputOne() override;
    uint8_t readInputTwo() override;

    // Update the input status
    void updateInput(SDL_Event* event);

private:
    // IO registers
    uint8_t m_latch;

    uint8_t m_tmpInput;
    uint8_t m_inputRegister;

    SDL_GameController* joy;
    uint8_t m_joystickStatus;
};    
}

#endif 
