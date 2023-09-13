#ifndef PPU_H_
#define PPU_H_

#include "ppuDebug.h"
#include "../frameBuffer.h"
#include "../cpu/cpu6502.h"

#include <cstddef>
#include <iostream>
#include <cstdint>

namespace nesCore {
class PpuBus;

enum PPU_CTRL_BITS {
    CTRL_NAMETABLES = 0b00000011,
    CTRL_ADDR_INC = 0b00000100,
    CTRL_SPR_PATTERN = 0b00001000,
    CTRL_BRG_PATTERN = 0b00010000,
    CTRL_SPRITE_SIZE = 0b00100000,
    CTRL_MASTER_SLAVE = 0b01000000,
    CTRL_VBLANK_NMI = 0b10000000,
};

enum PPU_MASK_BITS {
    MASK_GREYSCALE = 0b00000001,
    MASK_LEFT_BRG = 0b00000010,
    MASK_LEFT_SPR = 0b00000100,
    MASK_SHOW_BRG = 0b00001000,
    MASK_SHOW_SPR = 0b00010000,
    MASK_EMP_RED = 0b00100000,
    MASK_EMP_GREEN = 0b01000000,
    MASK_EMP_BLUE = 0b10000000,
};

enum PPU_STATUS_BITS {
    STATUS_SPR_OVERFLOW = 0b00100000,
    STATUS_SPR_HIT = 0b01000000,
    STATUS_VBLACK = 0b10000000,
};

enum SPRITE_ATTRIBUTE_BITS {
    SPRITE_PALETTE = 0b00000011,
    SPRITE_PRIORITY = 0b00100000,
    SPRITE_FLIP_H = 0b01000000,
    SPRITE_FLIP_V = 0b10000000,
};

class PPU {
private:
    // Sprite memory layout in the OAM memory
    struct Sprite {
        uint8_t y;
        uint8_t tile;
        uint8_t attribute;
        uint8_t x;
    };

public:
    PPU(PpuBus* ppuBus);
    void reset();

    // Get debug info
    debug::PPUDebug getDebugInfo();

    // Run PPU process
    Interrupt6502 clock(size_t cpuCycle);

    // Attach a frame buffer to the PPU
    void attachFrameBuffer(FrameBuffer* buffer);

    // Register read and write operation
    uint8_t readRegister(uint16_t addr);
    void writeRegister(uint16_t addr, uint8_t data);

    bool frameReady();

// Private methods
private:
    inline void coarseIncX();
    inline void coarseIncY();

    inline void coarseResetX();
    inline void coarseResetY();

    inline void rendering();
    inline void spriteEvaluation();
    inline void backgroundEvaluation();

// Private member variable
private:
    uint64_t m_ppuCycles;

    uint16_t m_scanLine;
    uint16_t m_scanCycle;

    // Flag used to inform the emulator of vblank
    bool m_vblankStart;

    // PPU bus
    PpuBus* mp_ppuBus;

    // Output frame buffer
    FrameBuffer* mp_frameBuffer;

    // Rendering and address registers
    bool m_oddFrame;

    // Background registers
    uint16_t m_backgroundShiftH;
    uint16_t m_backgroundShiftL;

    uint16_t m_attributeShiftH;
    uint16_t m_attributeShiftL;

    uint16_t m_ppuAddrTmp;
    uint16_t m_ppuAddrCurrent;

    uint8_t m_xFineScrolling;

    // Sprites registers
    size_t m_spritePosition;

    Sprite m_OAM[64];
    Sprite m_secondaryOAM[8];

    size_t m_primaryOAMpos;
    size_t m_secondaryOAMpos;

    uint8_t m_spriteShiftH[8];
    uint8_t m_spriteShiftL[8];

    uint8_t m_spriteAttribute[8];
    uint8_t m_spriteX[8];

    // Set to true if sprite zero is on the current scanline
    bool m_spriteZeroScanline;

    // Current theoretical cycle the sprite evaluation reached
    // Used to provide cycle accurate sprite evaluation
    size_t m_spriteEvaCycle;

    // PPU registers
    uint8_t m_ppuCtrl;
    uint8_t m_ppuMask;
    uint8_t m_ppuStatus;
    uint8_t m_oamAddr;
    uint8_t m_oamData;

    bool m_wLatch;

    uint8_t m_ppuData;

    uint8_t m_busLatch;
};

}

#endif
