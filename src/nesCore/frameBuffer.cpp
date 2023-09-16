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
int FrameBuffer::loadPalette(const std::string& filename) {
    // Open the file and read the palette content
    std::ifstream file(filename, std::ifstream::ate | std::ios_base::binary);

    // Check if the given palette file exist
    // and if it has the right size
    if (!file.good()) {
        return 1;
    }

    int size = file.tellg();
    file.seekg(0);

    if (size != 192) {
        return 2;
    }

    // Parse the palette file
    char color[3 * 64];
    file.read(color, 3 * 64);

    // Load the all palette
    uint32_t color32bits;
    for (int i = 0; i < 64; i++) {
        color32bits = 0xFF;
        color32bits |= (color[(i*3) + 2] & 0xFF) << 8;
        color32bits |= (color[(i*3) + 1] & 0xFF) << 8*2;
        color32bits |= (color[(i*3) + 0] & 0xFF) << 8*3;

        mp_colorPalette[i] = color32bits;
    }

    return 0;
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
