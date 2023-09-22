#ifndef FRAME_BUFFER_H_
#define FRAME_BUFFER_H_

#include "../nesPch.h"

namespace nesCore {

const int SCREEN_WIDTH = 256;
const int SCREEN_HEIGHT = 240;

class FrameBuffer {
public:
    FrameBuffer();
    ~FrameBuffer();
    
    // Load palette from file
    // Return 0 on success, 1 if the file doesn't exit
    // and 2 if it has the wrong format
    int loadPalette(const std::string& filename);
    
    // Get a pointer to the raw frame data
    uint8_t* data();

    // Set a pixel value to the given color;
    // this function use the frame buffer color palette to 
    // convert the 8 bits color value to a 32 bits RGBA value
    void setPixel(size_t x, size_t y, uint8_t color);

private:
    // RGBA frame buffer
    uint8_t* mp_frameData;

    // Color palette use to convert the NES
    // output color to RGB colors
    uint32_t mp_colorPalette[64];
};
}

#endif
