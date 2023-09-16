#ifndef PPU_DEBUG_H_
#define PPU_DEBUG_H_

#include <cstdint>
#include <string>

namespace nesCore {
namespace debug {

// Store the PPU debug info
struct PPUDebug {
    // Return a summary of the PPU status in one line
    std::string log();

    // PPU data
    uint64_t ppuCycles;

    uint16_t scanLine;
    uint16_t scanCycle;

    // PPU registers
    uint8_t ppuCtrl;
    uint8_t ppuMask;
    uint8_t ppuStatus;
    uint8_t oamAddr;
    uint8_t oamData;

    uint8_t ppuScrollX;
    uint8_t ppuScrollY;
    uint16_t ppuAddr;
    uint8_t ppuData;
};
    
}
}

#endif
