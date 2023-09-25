#ifndef SDL2_DISPLAY_H_
#define SDL2_DISPLAY_H_

#include "nesPch.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_rect.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_video.h>
#include "glad/glad.h"

#include "nesCore/frameBuffer.h"

namespace display {

class Sdl2Display {
public:
    Sdl2Display();

    // Initialize the display
    // return 0 on success
    int init(
        bool hideDangerZone = true,
        bool windowed = false,
        bool useVsync = true,
        const std::string& vertexPath = "resources/shaders/shader.vert", 
        const std::string& fragmentPath = "resources/shaders/shader.frag"
    );
    // Display quit function
    void quit();

    // Draw the frame buffer on the screen
    void update();
    // Handle the window resize
    void resize();

    // Attach the frame buffer to the display
    void attachFrameBuffer(nesCore::FrameBuffer* buffer);

    // Toggle window full screen mode
    void toggleFullscreen();
    // Toggle vsync
    void toggleVsync();

private:
    // Modify the display quad vertex coordinates
    void updateQuad(
        float topRX, float topRY,
        float bottomLX, float bottomLY
    );

    // Load and compile the shaders
    // Return 0 on success
    int loadShaders(
        const std::string& vertexPath, 
        const std::string& fragmentPath
    );

private:
    bool m_hideDangerZone;

    // Raw RGBA frame buffer
    uint8_t* mp_frameBuffer;

    // SDL window pointer
    SDL_Window* mp_window;

    // OpenGL rendering variable
    SDL_GLContext m_glContext;
    GLuint m_VAO, m_VBO, m_EBO;
    GLuint m_glTexture;
    GLuint m_shader;

    // Display quad vertices and indices
    float m_quadVertices[16] = {
        1.0f ,  1.0f, 1.0f, 1.0f,
        1.0f , -1.0f, 1.0f, 0.0f,
        -1.0f, -1.0f, 0.0f, 0.0f,
        -1.0f,  1.0f, 0.0f, 1.0f,
    };
    const unsigned int m_vertexIndices[6] = {      
        0, 1, 3,       
        1, 2, 3    
    };  
};
}

#endif
