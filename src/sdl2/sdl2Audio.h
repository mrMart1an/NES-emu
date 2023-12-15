#ifndef SDL2_AUDIO_H_
#define SDL2_AUDIO_H_

#include "nesPch.h"
#include "SDL.h"
#include <SDL_audio.h>

namespace audio {

class Sdl2Audio {
public:
    Sdl2Audio();
    ~Sdl2Audio();

private:
    SDL_AudioDeviceID m_audioDevice;
};
}

#endif

