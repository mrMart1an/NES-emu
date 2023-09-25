#include "nesPch.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_mouse.h>
#include <SDL2/SDL_pixels.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_stdinc.h>
#include <SDL2/SDL_video.h>
#include "glad/glad.h"

#include "nesCore/frameBuffer.h"
#include "sdl2Display.h"

namespace display {
Sdl2Display::Sdl2Display()
    : mp_frameBuffer(nullptr), mp_window(nullptr) {}

int Sdl2Display::init(
    bool hideDangerZone,
    bool windowed,
    bool useVsync,
    const std::string& vertexPath, 
    const std::string& fragmentPath
) {
    m_hideDangerZone = hideDangerZone;

    // Create SDL window
    mp_window = SDL_CreateWindow("SDL2 Window",
                                SDL_WINDOWPOS_CENTERED,
                                SDL_WINDOWPOS_CENTERED,
                                680, 480,
                                SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL
    );
    if (!mp_window) {
        std::cerr << "Failed to initialized SDL2 window" << std::endl;
        return 1;
    }

    // Make the application fullscreen at startup
    if (!windowed)
        toggleFullscreen();

    // Create an openGL context
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    m_glContext = SDL_GL_CreateContext(mp_window);
    if (m_glContext == NULL) {
        std::cerr << "Failed to initialized openGL context" << std::endl;
        return 2;
    }
    gladLoadGLLoader(SDL_GL_GetProcAddress);

    // Load and compile the shaders
    if (loadShaders(vertexPath, fragmentPath) != 0) {
        std::cerr << "Failed to load openGL shaders" << std::endl;
        return 3;
    }
    
    // Create vertex buffer
    glGenBuffers(1, &m_VBO);  
    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);  
    glBufferData(GL_ARRAY_BUFFER, sizeof(m_quadVertices), m_quadVertices, GL_STATIC_DRAW);

    // Create the vertex array
    glGenVertexArrays(1, &m_VAO);
    glBindVertexArray(m_VAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);

    // Create element buffer
    glGenBuffers(1, &m_EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(m_vertexIndices), m_vertexIndices, GL_STATIC_DRAW);
    
    // Specify vertex layout
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);  
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Create and set texture propriety 
    glGenTextures(1, &m_glTexture);

    glBindTexture(GL_TEXTURE_2D, m_glTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

    float borderColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // Enable vsync and hide cursor
    SDL_GL_SetSwapInterval(useVsync);
    SDL_ShowCursor(SDL_DISABLE);

    // Initialized the destination and source rects
    this->resize();

    // Bind all the buffer
    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);  
    glGenBuffers(1, &m_EBO);
    glBindVertexArray(m_VAO);
    glBindTexture(GL_TEXTURE_2D, m_glTexture);

    // Activate shaders pipeline
    glUseProgram(m_shader);

    return 0;
}

int Sdl2Display::loadShaders(
    const std::string& vertexPath, 
    const std::string& fragmentPath
) {
    // Load shaders
    std::ifstream vertexFile(vertexPath, std::ifstream::ate | std::ios_base::binary);
    std::ifstream fragmentFile(fragmentPath, std::ifstream::ate | std::ios_base::binary);

    if (!vertexFile.good() || !fragmentFile.good()) 
        return 1;

    int vertexSize = vertexFile.tellg();
    int fragmentSize = fragmentFile.tellg();
    vertexFile.seekg(0);
    fragmentFile.seekg(0);

    char* vertexSrc = new char[vertexSize + 1];
    char* fragmentSrc = new char[fragmentSize + 1];

    // Read the shaders file and add string terminator
    vertexFile.read(vertexSrc, vertexSize);
    fragmentFile.read(fragmentSrc, fragmentSize);
    vertexSrc[vertexSize] = 0x00;
    fragmentSrc[fragmentSize] = 0x00;

    // Compile the shaders
    int success;
    unsigned int vertexShader;
    unsigned int fragmentShader;

    // Vertex compilation
    vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexSrc, NULL);
    glCompileShader(vertexShader);
    
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cerr << "Vertex shader compilation error:\n" << infoLog << std::endl;
        return 2;
    }
    
    // Fragment compilation
    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentSrc, NULL);
    glCompileShader(fragmentShader);

    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cerr << "Fragment shader compilation error:\n" << infoLog << std::endl;
        return 2;
    }

    // Create the shaders program
    m_shader = glCreateProgram();
    glAttachShader(m_shader, vertexShader);
    glAttachShader(m_shader, fragmentShader);
    glLinkProgram(m_shader);

    // Clean up
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    delete [] vertexSrc;
    delete [] fragmentSrc;

    return 0;
}

void Sdl2Display::updateQuad(
    float topRX, float topRY,
    float bottomLX, float bottomLY
) {
    // Top right corner
    m_quadVertices[0] = topRX;
    m_quadVertices[1] = topRY;

    // Bottom right corner
    m_quadVertices[1*4 + 0] = topRX;
    m_quadVertices[1*4 + 1] = bottomLY;

    // Bottom left corner
    m_quadVertices[2*4 + 0] = bottomLX;
    m_quadVertices[2*4 + 1] = bottomLY;

    // Top left corner
    m_quadVertices[3*4 + 0] = bottomLX;
    m_quadVertices[3*4 + 1] = topRY;

    // Copy the new vertices in the buffers
    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);  
    glBufferData(GL_ARRAY_BUFFER, sizeof(m_quadVertices), m_quadVertices, GL_STATIC_DRAW);
}

void Sdl2Display::attachFrameBuffer(nesCore::FrameBuffer* buffer) {
    if (m_hideDangerZone)
        mp_frameBuffer = buffer->data() + (nesCore::SCREEN_WIDTH * 8*3);
    else
        mp_frameBuffer = buffer->data();
}

void Sdl2Display::resize() {
    // Get the window size and clear the screen
    int w, h;
    SDL_GetWindowSize(mp_window, &w, &h);
    glViewport(0, 0, w, h);

    // Clear the screen
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f );
    glClear(GL_COLOR_BUFFER_BIT); 

    // Calculate the aspect ratio
    float windowAspectRatio = (float)w / h;

    float targetAspectRatio;
    if (m_hideDangerZone)
        targetAspectRatio = (float)nesCore::SCREEN_WIDTH / (nesCore::SCREEN_HEIGHT - 16);
    else
        targetAspectRatio = (float)nesCore::SCREEN_WIDTH / nesCore::SCREEN_HEIGHT;

    // Calculate the position of the display quad vertices
    // to maintain the right aspect ratio
    float topX, topY, bottomX, bottomY;

    if (windowAspectRatio > targetAspectRatio) {
        float widthRelative = (2.0f / windowAspectRatio) * targetAspectRatio;
        float horizontalPadding = 2.0f - widthRelative;

        topY = 1.0f;
        bottomY = -1.0f;
        topX = 1.0f - (horizontalPadding / 2);
        bottomX = -1.0f + (horizontalPadding / 2);
    } else {
        float heightRelative = (windowAspectRatio * 2) / targetAspectRatio;
        float verticalPadding = 2.0f - heightRelative;

        topX = 1.0f;
        bottomX = -1.0f;
        topY = 1.0 - (verticalPadding / 2);
        bottomY = -1.0 + (verticalPadding / 2);
    }

    updateQuad(topX, topY, bottomX, bottomY);
}

void Sdl2Display::update() { 	
    // Copy the texture and draw it on screen
    int width = nesCore::SCREEN_WIDTH;
    int height = m_hideDangerZone ? nesCore::SCREEN_HEIGHT - 16 : nesCore::SCREEN_HEIGHT;

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, mp_frameBuffer);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    SDL_GL_SwapWindow(mp_window);
}

void Sdl2Display::quit() {
    glDeleteVertexArrays(1, &m_VAO);
    glDeleteBuffers(1, &m_VBO);
    glDeleteBuffers(1, &m_EBO);

    SDL_GL_DeleteContext(m_glContext);
	SDL_DestroyWindow(mp_window);
}

void Sdl2Display::toggleFullscreen() {
    bool isFullscreen = SDL_GetWindowFlags(mp_window) & SDL_WINDOW_FULLSCREEN_DESKTOP;
    SDL_SetWindowFullscreen(mp_window, isFullscreen ? 0 : SDL_WINDOW_FULLSCREEN_DESKTOP);
}
void Sdl2Display::toggleVsync() {
    bool isVsync = SDL_GL_GetSwapInterval();
    SDL_GL_SetSwapInterval(!isVsync);
}
}
