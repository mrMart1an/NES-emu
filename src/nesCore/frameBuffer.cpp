#include <algorithm>
#include <cstdint>
#include <ios>
#include <iostream>
#include <string>
#include <fstream>

#include "frameBuffer.h"
#include "utility/utilityFunctions.h"

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

// Load frame palette from file
void FrameBuffer::loadPalette(const std::string& filename) {
    // Open the file and read a color buffer
    std::ifstream file(filename, std::ios_base::binary);

    char color[3 * 64];
    file.read(color, 3 * 64);

    // Load the all palette
    uint32_t color32bits;
    for (int i = 0; i < 64; i++) {
        color32bits = 0xFF | (color[(i*3) + 2] & 0xFF) << 8 | (color[(i*3) + 1] & 0xFF) << 8*2 | (color[(i*3) + 0] & 0xFF) << 8*3;
        mp_colorPalette[i] = color32bits;
    }
}

// Get the raw data pointer
uint32_t* FrameBuffer::data() {
    return mp_frameData;
}

// Set a pixel color
void FrameBuffer::setPixel(size_t x, size_t y, uint8_t color) {
    size_t pixelAddress = (SCREEN_WIDTH * y) + x;

    mp_frameData[pixelAddress] = mp_colorPalette[color & 0b00111111];
}
}
