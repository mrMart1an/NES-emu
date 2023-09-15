#include "ppu.h"
#include "../utility/utilityFunctions.h"
#include "../ppuBus.h"
#include "ppuDebug.h"
#include <algorithm>
#include <cstdint>
#include <ios>
#include <iostream>

namespace nesCore {
PPU::PPU(PpuBus* ppuBus) : mp_ppuBus(ppuBus), mp_frameBuffer(nullptr) {
    // Reset OAM memory
    uint8_t* p_OAM = reinterpret_cast<uint8_t*>(m_OAM);
    uint8_t* p_secondaryOAM = reinterpret_cast<uint8_t*>(m_secondaryOAM);

    std::fill(p_OAM, p_OAM + sizeof(m_OAM), 0x00);
    std::fill(p_secondaryOAM, p_secondaryOAM + sizeof(m_secondaryOAM), 0x00);

    // Initialize the rendering and address register
    m_oamAddr = 0x00;

    // Initialize the register
    m_ppuStatus = 0b10100000;
    

    this->reset();
}; 

// Reset the PPU status
void PPU::reset() {
    m_oddFrame = false;
    m_ppuCycles = 0;

    // Reset the rendering and address register
    m_ppuAddrTmp = 0x0000;
    m_ppuAddrCurrent = 0x0000;
    m_wLatch = false;

    m_xFineScrolling = 0x00;

    // Reset the register
    m_ppuCtrl = 0x00;
    m_ppuMask = 0x00;

    m_ppuData = 0x00;
    m_oamData = 0x00;

    m_ppuCycles = 0;
    m_scanCycle = 0;
    m_scanLine = 0;

    m_busLatch = 0;

    m_primaryOAMpos = 0;
    m_secondaryOAMpos = 0;
    m_spritePosition = 0;

    m_spriteEvaCycle = 0;
    m_spriteZeroScanline = false;
}

// Get debug info
debug::PPUDebug PPU::getDebugInfo() {
    debug::PPUDebug output;

    output.ppuCycles = m_ppuCycles;

    output.scanLine = m_scanLine;
    output.scanCycle = m_scanCycle;

    output.ppuCtrl = m_ppuCtrl;
    output.ppuMask = m_ppuMask;
    output.ppuStatus = m_ppuStatus;
    output.oamAddr = m_oamAddr;
    output.oamData = m_oamData;
                             
    output.ppuAddr = m_ppuAddrCurrent;
    output.ppuData = m_ppuData;

    return output;
}

// Attach the frame buffer to the PPU
void PPU::attachFrameBuffer(FrameBuffer* buffer) {
    mp_frameBuffer = buffer;
}

// Register read and write
uint8_t PPU::readRegister(uint16_t addr) {
    addr = (addr - 0x2000) % 0x0008;

    switch (addr) {
        case 0x0002: {
            uint8_t status = m_ppuStatus & 0b11100000;
            status |= m_busLatch & 0b00011111;

            // Reset w latch and reset the vblack flag
            m_wLatch = false;
            m_ppuStatus &= 0b01100000;

            m_busLatch = status;
            return status;
        }

        // Read data from the OAM buffer
        case 0x0004:
            if ((m_ppuMask & MASK_LEFT_BRG || m_scanCycle > 8) && (m_scanCycle >= 1 && m_scanCycle <= 64)) {
                m_oamData = 0xFF;
            } else {
                m_oamData = reinterpret_cast<uint8_t*>(m_OAM)[m_oamAddr];
            }

            m_busLatch = m_oamData;
            return m_oamData;

        // Read PPU bus data
        case 0x0007: {
            uint8_t oldData = m_ppuData;
            uint16_t oldAddr = m_ppuAddrCurrent;
            // Read new data from the PPU bus
            m_ppuData = mp_ppuBus->read(oldAddr);

            // Increment the address register
            if ((m_ppuCtrl & CTRL_ADDR_INC) == 0)
                m_ppuAddrCurrent += 1;
            else
                m_ppuAddrCurrent += 32;

            // If palette data is being read return it immediately
            if (oldAddr >= 0x3F00 && oldAddr <= 0x3FFF) {
                m_busLatch = m_ppuData;
                return m_ppuData;
            } else {
                m_busLatch = oldData;
                return oldData;
            }
        }
        default:
            std::cout << "Warn: Attempt to read write only PPU reg: " << addr << std::endl;
            return m_busLatch;
    }

    return 0x00;
}
void PPU::writeRegister(uint16_t addr, uint8_t data) {
    // Ignore all write operation before cycle 29658
    if (m_ppuCycles <= 29658)
        return;

    addr = (addr - 0x2000) % 0x0008;

    // Store the data on as bus latch
    m_busLatch = data;

    switch (addr) {
        case 0x0000:
            m_ppuCtrl = data; 
            // Update name table address
            m_ppuAddrTmp = (m_ppuAddrTmp & 0xF3FF) | (static_cast<uint16_t>(data & 0x03) << 10);
            break;    
        case 0x0001: 
            m_ppuMask = data; break;
        case 0x0003: 
            m_oamAddr = data; break;

        // Write to OAM buffer
        case 0x0004: 
            m_oamData = data;
            reinterpret_cast<uint8_t*>(m_OAM)[m_oamAddr] = data;

            // Increment the OAM address
            m_oamAddr += 1;
            break;

        // Two byte scroll register write
        case 0x0005: 
            if (!m_wLatch) {
                m_xFineScrolling = data & 0b00000111;
                m_ppuAddrTmp = (m_ppuAddrTmp & 0xFFE0) | (static_cast<uint16_t>(data) >> 3);
            } else {
                m_ppuAddrTmp = (m_ppuAddrTmp & 0x8FFF) | (static_cast<uint16_t>(data) << 12);
                m_ppuAddrTmp = (m_ppuAddrTmp & 0xFC1F) | (static_cast<uint16_t>(data & 0xF8) << 2);
            }

            m_wLatch = !m_wLatch;
            break;

        // Two byte address register write
        case 0x0006: 
            if (!m_wLatch) {
                m_ppuAddrTmp = (m_ppuAddrTmp & 0x80FF) | (static_cast<uint16_t>(data & 0x3F) << 8);
            } else {
                m_ppuAddrTmp = (m_ppuAddrTmp & 0xFF00) | (static_cast<uint16_t>(data));
                m_ppuAddrCurrent = m_ppuAddrTmp;
            }

            m_wLatch = !m_wLatch;
            break;    

        // PPU bus write
        case 0x0007: {
            // Write the data to the bus and update the bus latch
            mp_ppuBus->write(m_ppuAddrCurrent, data);
            m_ppuData = data;
            m_busLatch = data;
            
            // Increment the address register
            if ((m_ppuCtrl & CTRL_ADDR_INC) == 0)
                m_ppuAddrCurrent += 1;
            else
                m_ppuAddrCurrent += 32;

            break;
        }

        default:
            std::cout << "Warn: Attempt to write read only PPU reg: " << addr << std::endl;
            break;        
    }
}

// Increments functions
inline void PPU::coarseIncX() {
    if ((m_ppuAddrCurrent & 0x001F) == 31) {
        m_ppuAddrCurrent &= ~0x001F;
        m_ppuAddrCurrent ^= 0x0400;
    } else {
        m_ppuAddrCurrent++;
    }
}
inline void PPU::coarseIncY() {
if ((m_ppuAddrCurrent & 0x7000) != 0x7000) {
        m_ppuAddrCurrent += 0x1000;
    } else {
        m_ppuAddrCurrent &= ~0x7000;

        uint16_t y = (m_ppuAddrCurrent & 0x03E0) >> 5;
        if (y == 29) {
            y = 0;
            m_ppuAddrCurrent ^= 0x0800;
        } else if (y == 31)
            y = 0;
        else
            y += 1;  

        m_ppuAddrCurrent = (m_ppuAddrCurrent & ~0x03E0) | (y << 5);
    }
}

inline void PPU::coarseResetX() {
    m_ppuAddrCurrent = (m_ppuAddrCurrent & 0x7BE0) | (m_ppuAddrTmp & 0x41F);

}
inline void PPU::coarseResetY() {       
    m_ppuAddrCurrent = (m_ppuAddrCurrent & 0x041F) | (m_ppuAddrTmp & 0x7BE0);
}

// Execute all the operation necessary for background rendering
inline void PPU::backgroundEvaluation() {
    // Shift the pattern table and attribute register
    if (m_scanLine < 240 || m_scanLine == 261) {
        if ((m_scanCycle > 0 && m_scanCycle < 257) || (m_scanCycle > 320 && m_scanCycle < 337)) {
            m_backgroundShiftH <<= 1;
            m_backgroundShiftL <<= 1;

            m_attributeShiftH <<= 1;
            m_attributeShiftL <<= 1;
        }
    }

    // Address register Increment and updates
    if (m_scanLine < 240 || m_scanLine == 261) {
        if ((m_scanCycle % 8 == 0) && ((m_scanCycle > 0 && m_scanCycle < 257) || (m_scanCycle > 320 && m_scanCycle < 337))) {
            // Load the next tile attribute datas
            uint16_t attributeAddr = 0x23C0;
            attributeAddr |= (m_ppuAddrCurrent & 0x0C00) | ((m_ppuAddrCurrent >> 4) & 0x38) | ((m_ppuAddrCurrent >> 2) & 0x07);
            
            uint8_t attributeShift = (m_ppuAddrCurrent & 0x0002) ;
            attributeShift |= (m_ppuAddrCurrent & 0x0040) >> 4;

            uint8_t attribute = mp_ppuBus->read(attributeAddr);
            attribute = attribute >> attributeShift;
            
            m_attributeShiftH |= attribute & 0x02 ? 0xFF : 0x00;
            m_attributeShiftL |= attribute & 0x01 ? 0xFF : 0x00;

            // Load new data in pattern table shift registers 
            uint16_t patternTable = m_ppuCtrl & CTRL_BRG_PATTERN ? 0x1000: 0x0000;
            uint8_t tile = mp_ppuBus->read(0x2000 | patternTable | (m_ppuAddrCurrent & 0x0FFF));

            uint16_t patternAddr = (m_ppuAddrCurrent >> 12) | static_cast<uint16_t>(tile) << 4;
            patternAddr |= ((m_ppuCtrl & CTRL_BRG_PATTERN) != 0) << 12;

            m_backgroundShiftH |= mp_ppuBus->read(patternAddr | 8);
            m_backgroundShiftL |= mp_ppuBus->read(patternAddr);

            // Increment X 
            coarseIncX();
        }

        // Increment Y
        if (m_scanCycle == 256) {
            coarseIncY();
        }

        // Copy address register horizontal components and Increment fine Y scrolling
        if (m_scanCycle == 257) 
            coarseResetX();

        // Copy address register vertical components
        if (m_scanLine == 261 && m_scanCycle >= 280 && m_scanCycle <= 304)
            coarseResetY();
    }
}

// Execute all the sprite evaluation operation
inline void PPU::spriteEvaluation() {
    if (m_scanCycle == 0) {
        m_primaryOAMpos = 0;
        m_secondaryOAMpos = 0;
        m_spritePosition = 0;

        m_spriteEvaCycle = 65;
        m_spriteZeroNextScanline = false;
    }

    // Reset the secondary OAM
    if (m_scanCycle >= 1 && m_scanCycle <= 64) 
        reinterpret_cast<uint8_t*>(m_secondaryOAM)[(m_scanCycle - 1) / 2] = 0xFF;

    // Get the sprite dimension
    uint8_t spriteSize = m_ppuCtrl & CTRL_SPRITE_SIZE ? 16 : 8;

    // Copy data to secondary OAM
    if (m_scanCycle >= m_spriteEvaCycle && m_scanCycle <= 256) {
        if (m_secondaryOAMpos < 8) {
            // Copy the Y coordinate of the sprite at the current position 
            // of the primary OAM to a free slot of the secondary OAM
            // If the Y value is in range copy the remaining 3 sprite bytes
            // and increment secondary OAM position
            uint8_t valueY = m_OAM[m_primaryOAMpos].y;

            // Copy if in range
            if (m_scanLine >= valueY && m_scanLine < valueY + spriteSize) {
                for (int i = 0; i < 4; i++)
                    m_secondaryOAM[m_secondaryOAMpos] = m_OAM[m_primaryOAMpos];

                if (m_primaryOAMpos == 0)
                    m_spriteZeroNextScanline = true;

                m_secondaryOAMpos += 1;
                m_spriteEvaCycle += 6;
            }
        } else {
            // Check if another sprite is on the same scanline and set the sprite overflow flag
            uint8_t value = reinterpret_cast<uint8_t*>(m_OAM)[(m_primaryOAMpos * 4) + m_spritePosition];
            if (m_scanLine >= value && m_scanLine < value + spriteSize) {
                m_ppuStatus |= STATUS_SPR_OVERFLOW;
            } else {
                // Increment the sprite read position,
                // this is a PPU hardware bug
                m_spritePosition += 1;
                if (m_spritePosition >= 4)
                    m_spritePosition = 0;
            }
        }
            
        m_spriteEvaCycle += 2;

        // Increment the primary OAM position
        m_primaryOAMpos += 1;
        if (m_primaryOAMpos >= 64)
            m_spriteEvaCycle = 257;
    }

    // Copy data to rendering register for the next scanline
    if (m_scanCycle  == 257) {
        // Set sprite zero hit scanline for the incoming scanline
        m_spriteZeroScanline = m_spriteZeroNextScanline;

        int i = 0;

        // Loop for not empty sprite slot
        for (; i < 8; i++) {
            uint8_t spriteY = m_secondaryOAM[i].y;
            uint8_t attribute = m_secondaryOAM[i].attribute;

            if (spriteY == 0xFF)
                break;

            m_spriteAttribute[i] = attribute;
            m_spriteX[i] = m_secondaryOAM[i].x;

            uint8_t tileNumber = m_secondaryOAM[i].tile;
            bool vFlip = attribute & SPRITE_FLIP_V;

            uint8_t tileY;
            if (vFlip)
                tileY = (spriteSize - 1) - (m_scanLine - spriteY);
            else 
                tileY = m_scanLine - spriteY;

            // Calculate the tile pattern address
            uint16_t patternAddr;
            if (spriteSize == 8) {
                uint16_t patternTable = m_ppuCtrl & CTRL_SPR_PATTERN ? 0x1000 : 0x0000;
                patternAddr = patternTable | (tileNumber << 4) | tileY;
            } else {
                uint16_t patternTable = tileNumber & 0x01 ? 0x1000 : 0x0000;
                patternAddr = patternTable | ((tileNumber & 0xFE) << 4) | tileY;
            }

            // Fetch the pattern data
            m_spriteShiftL[i] = mp_ppuBus->read(patternAddr);
            m_spriteShiftH[i] = mp_ppuBus->read(patternAddr | 0x08);
        }

        // Remaining empty sprite slot 
        for (; i < 8; i++) {
            // Fill the sprite shift register with 0
            m_spriteShiftL[i] = 0x00;
            m_spriteShiftH[i] = 0x00;

            m_spriteX[i] = 0xFF;
            m_spriteAttribute[i] = 0xFF;
        }
    }
}

// Rendering function
inline void PPU::rendering() {
    // Render the visible scan lines
    if (m_scanLine < 240 && m_scanCycle > 0 && m_scanCycle < 257) {
        uint8_t outputPixel = 0x00;
        uint16_t outputPaletteAddr = 0x3F00;

        // Check if background rendering is disable
        // If left most rendering is disable don't render the left most tile
        if ((m_ppuMask & MASK_LEFT_BRG || m_scanCycle > 8) && m_ppuMask & MASK_SHOW_BRG) {
            // Get background pixel color
            uint8_t bgPixel = ((m_backgroundShiftL << m_xFineScrolling) & 0x8000) >> 15;
            bgPixel |= ((m_backgroundShiftH << m_xFineScrolling) & 0x8000) >> 14;

            // Get attribute data
            uint8_t bgPalette = ((m_attributeShiftL << m_xFineScrolling) & 0x8000) >> 13;
            bgPalette |= ((m_attributeShiftH << m_xFineScrolling) & 0x8000) >> 12;

            outputPixel = bgPixel;
            outputPaletteAddr = 0x3F00 | bgPalette;
        }

        // Handle sprite rendering and priority 
        if (m_ppuMask & MASK_SHOW_SPR) {
            uint8_t sprAttribute = 0x00;
            uint8_t sprPixel = 0x00;
            uint8_t sprNumber = 1;

            for (int i = 7; i >= 0; i--) {
                if (m_spriteX[i] == 0) {
                    uint8_t tmpSprAttribute = m_spriteAttribute[i];
                    uint8_t tmpSprPixel = 0x00;

                    // Get pixel value and shift the pattern registers
                    // Flip horizontally if necessary 
                    if (tmpSprAttribute & SPRITE_FLIP_H) {
                        tmpSprPixel = m_spriteShiftL[i] & 0x01;
                        tmpSprPixel |= (m_spriteShiftH[i] & 0x01) << 1;

                        m_spriteShiftL[i] >>= 1;
                        m_spriteShiftH[i] >>= 1;
                    } else {
                        tmpSprPixel = (m_spriteShiftL[i] & 0x80) >> 7;
                        tmpSprPixel |= (m_spriteShiftH[i] & 0x80) >> 6;

                        m_spriteShiftL[i] <<= 1;
                        m_spriteShiftH[i] <<= 1;
                    }

                    if (tmpSprPixel != 0) {
                        sprAttribute = tmpSprAttribute;
                        sprPixel = tmpSprPixel;
                        sprNumber = i;
                    }
                } else {
                    // Decrements each sprite x counter
                    m_spriteX[i] -= 1;
                }
            }

            // Determine render priority
            if (sprPixel != 0 && (m_ppuMask & MASK_LEFT_SPR || m_scanCycle > 8)) {
                // Check for sprite zero hit
                if (m_spriteZeroScanline && sprNumber == 0 && outputPixel != 0) {
                    m_ppuStatus |= STATUS_SPR_HIT;
                }

                if ((sprAttribute & SPRITE_PRIORITY) == 0 || outputPixel == 0) {
                    outputPixel = sprPixel;
                    outputPaletteAddr = 0x3F10 | ((sprAttribute & SPRITE_PALETTE) << 2);
                }
            }
        }

        // Update pixel color
        uint8_t outputColor;
        if (outputPixel) 
            outputColor = mp_ppuBus->read(outputPaletteAddr | outputPixel);
        else
            outputColor = mp_ppuBus->read(0x3F00);

        mp_frameBuffer->setPixel(m_scanCycle - 1, m_scanLine, outputColor);
    }
}

// PPU processing 
Interrupt6502 PPU::clock(size_t cpuCycle) {
    uint16_t ppuCycle = cpuCycle * 3;
    Interrupt6502 outputInterrupt = NOINT;

    for (uint16_t i = 0; i < ppuCycle; i++) {
        // Pre-rendering scan line
        if (m_scanLine == 261) {
            // Clear flags
            if (m_scanCycle == 1)
                m_ppuStatus = 0x00;
        }

        // Check if rendering is enable
        if (m_ppuMask & (MASK_SHOW_SPR | MASK_SHOW_BRG)) {
            // Pre-rendering scan line
            if (m_scanLine == 261) {
                // Skip one tick on odd frames
                if (m_oddFrame && m_scanCycle == 339) {
                    m_scanCycle = 0; m_scanLine = 0; continue;
                }
            }

            spriteEvaluation();
            rendering();
            backgroundEvaluation();
         }

        // Post rendering scan line
        if (m_scanLine == 241 && m_scanCycle == 1) {
            m_vblankStart = true;

            if ((m_ppuCtrl & CTRL_VBLANK_NMI) != 0)
                outputInterrupt = NMI;

            m_ppuStatus |= STATUS_VBLACK;
        } 

        // Increment scan cycle and scan line
        m_scanCycle += 1;
        if (m_scanCycle >= 341) {
            m_scanLine += 1;
            m_scanCycle = 0;

            if (m_scanLine >= 262)
                m_scanLine = 0;
        }

        m_oddFrame = !m_oddFrame;
    }

    // Increment PPU cycles counter and return the interrupt
    m_ppuCycles += ppuCycle;
    return outputInterrupt;
}

bool PPU::frameReady() {
    bool vblanck = m_vblankStart;
    m_vblankStart = false;

    return vblanck;
}
}
