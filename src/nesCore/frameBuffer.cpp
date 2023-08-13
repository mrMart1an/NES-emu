#include <algorithm>
#include <cstdint>
#include <iostream>

#include "frameBuffer.h"

namespace nesCore {
FrameBuffer::FrameBuffer() {
    // Construct the raw frame buffer
    mp_frameData = new uint32_t[SCREEN_WIDTH * SCREEN_HEIGHT];
    std::fill(mp_frameData, mp_frameData + (SCREEN_WIDTH * SCREEN_HEIGHT), 0);

    // Fill the color palette with white
    std::fill(mp_colorPalette, mp_colorPalette + 64, 0xFFFFFFFF);
}
FrameBuffer::~FrameBuffer() {
    delete [] mp_frameData;
}

// Get the raw data pointer
uint32_t* FrameBuffer::data() {
    return mp_frameData;
}

// Set a pixel color
void FrameBuffer::setPixel(size_t x, size_t y, uint8_t color) {
    size_t pixelAddress = (SCREEN_WIDTH * y) + x;

    color *= 0x55;

    mp_frameData[pixelAddress] = 0xFF | color << 8 | color << 8*2 | color << 8*3;
}
}
