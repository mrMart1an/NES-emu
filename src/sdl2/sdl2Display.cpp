#include <SDL2/SDL.h>
#include <SDL2/SDL_pixels.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_stdinc.h>
#include <SDL2/SDL_video.h>

#include <algorithm>
#include <cstdint>
#include <iostream>

#include "sdl2Display.h"
#include "../nesCore/frameBuffer.h"

namespace display {
Sdl2Display::Sdl2Display()
    : mp_frameBuffer(nullptr), mp_window(nullptr), mp_renderer(nullptr), mp_texture(nullptr) {}

int Sdl2Display::init() {
    mp_window = SDL_CreateWindow("SDL2 Window",
                                SDL_WINDOWPOS_CENTERED,
                                SDL_WINDOWPOS_CENTERED,
                                680, 480,
    SDL_WINDOW_RESIZABLE);

    if (!mp_window) {
        std::cout << "Failed to initialized SDL2 window" << std::endl;
        return -2;
    }

    mp_renderer = SDL_CreateRenderer(mp_window, -1, SDL_RENDERER_ACCELERATED);
    mp_texture = SDL_CreateTexture(mp_renderer, 
                                  SDL_PIXELFORMAT_RGBA8888, 
                                  SDL_TEXTUREACCESS_STREAMING, 
                                  nesCore::SCREEN_WIDTH, 
                                  nesCore::SCREEN_HEIGHT);
    
    SDL_RenderSetVSync(mp_renderer, 1);

    // Initialized the texture rect
    this->resize();

    return 0;
}

void Sdl2Display::attackFrameBuffer(nesCore::FrameBuffer* buffer) {
    mp_frameBuffer = buffer->data();
}

void Sdl2Display::resize() {
    // Get the window size and calculate aspect ratio
    int w, h;
    SDL_GetWindowSize(mp_window, &w, &h);

    float aspectRatio = (float)nesCore::SCREEN_WIDTH / (float)nesCore::SCREEN_HEIGHT;

    // Calculate the width, height and position
    int width = w;
    int height = width / aspectRatio;

    int posX = 0;
    int posY = (h / 2) - (height / 2);

    // If height is greater that window height recalculate the dimensions
    if (h <= height) {
        height = h;
        width = height * aspectRatio;

        posY = 0;
        posX = (w / 2) - (width / 2);
    }

    // Set the texture rect
    m_textureRect.x = posX; m_textureRect.y = posY; 
    m_textureRect.w = width; m_textureRect.h = height; 
}

void Sdl2Display::update() { 	
    SDL_RenderClear(mp_renderer);

    if (mp_frameBuffer != nullptr)
        SDL_UpdateTexture(mp_texture, NULL, mp_frameBuffer, nesCore::SCREEN_WIDTH * sizeof(uint32_t));
    
    SDL_RenderClear(mp_renderer);
    SDL_RenderCopy(mp_renderer, mp_texture, NULL, &m_textureRect);
    SDL_RenderPresent(mp_renderer);
}

void Sdl2Display::quit() {
  	SDL_DestroyTexture(mp_texture);
	SDL_DestroyRenderer(mp_renderer);

	SDL_DestroyWindow(mp_window);
}
}