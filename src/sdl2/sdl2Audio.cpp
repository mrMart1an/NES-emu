#include "nesPch.h"
#include "sdl2/sdl2Audio.h"
#include <SDL_audio.h>

namespace audio {

Sdl2Audio::Sdl2Audio() {
    int sample_nr = 0;

    SDL_AudioSpec ds;
    ds.freq = 44100;
    ds.format = AUDIO_S16SYS;
    ds.channels = 1;
    ds.samples = 4096;
    ds.callback = NULL;
    ds.userdata = &sample_nr;

    SDL_AudioSpec os;
    m_audioDevice = SDL_OpenAudioDevice(NULL, 0, &ds, &os, 0);

}

Sdl2Audio::~Sdl2Audio()
{
    SDL_CloseAudioDevice(m_audioDevice);
}

}
