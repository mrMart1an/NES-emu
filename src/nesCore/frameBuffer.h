#ifndef FRAME_BUFFER_H_
#define FRAME_BUFFER_H_

#include <cstddef>
#include <cstdint>
namespace nesCore {

const int SCREEN_WIDTH = 256;
const int SCREEN_HEIGHT = 240;

class FrameBuffer {
public:
    FrameBuffer();
    ~FrameBuffer();
    
    // Get a pointer to the raw frame data
    uint32_t* data();

    // Set a pixel value to the given color;
    // this function use the frame buffer color palette to 
    // convert the 8 bits color value to a 32 bits ARGB value
    void setPixel(size_t x, size_t y, uint8_t color);

private:
    // ARGB frame buffer
    uint32_t* mp_frameData;

    // Color palette use to convert the NES
    // output color to ARGB colors
    uint32_t mp_colorPalette[64];
};
}

#endif
