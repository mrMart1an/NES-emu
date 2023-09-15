#include <SDL2/SDL.h>
#include <SDL2/SDL_rect.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_video.h>

#include <cstdint>

#include "../nesCore/frameBuffer.h"

namespace display {

class Sdl2Display {
public:
    Sdl2Display();

    // Initialize the display
    // return 0 on success
    int init(bool hideDangerZone = true);
    // Display quit function
    void quit();

    // Draw the frame buffer on the screen
    void update();
    // Handle the window resize
    void resize();

    // Attach the frame buffer to the display
    void attackFrameBuffer(nesCore::FrameBuffer* buffer);

    // Toggle window full screen mode
    void toggleFullscreen();

private:
    // Raw RGBA frame buffer
    uint32_t* mp_frameBuffer;

    SDL_Window* mp_window;

    SDL_Renderer *mp_renderer;
    SDL_Texture *mp_texture;

    SDL_Rect m_srcRect;
    SDL_Rect m_destRect;
};

}
