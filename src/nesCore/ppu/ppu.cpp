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
    this->reset();
}; 

// Reset the PPU status
void PPU::reset() {
    // Reset OAM memory
    std::fill(m_OAM, m_OAM + sizeof(m_OAM), 0x00);

    m_oddFrame = true;

    // Reset the rendering and address register
    m_wLatch = false;
    m_ppuAddrTmp = 0x0000;
    m_ppuAddrCurrent = 0x0000;

    m_xFineScrolling = 0x00;

    // Reset the register
    m_ppuCtrl = 0x00;
    m_ppuMask = 0x00;
    m_ppuStatus = 0x80;

    m_ppuData = 0x00;

    m_ppuCycles = 0;
    m_scanCycle = 0;
    m_scanLine = 0;

    m_busLatch = 0;
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
            m_oamData = m_OAM[m_oamAddr];

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
            m_OAM[m_oamAddr] = data;

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
                m_ppuAddrTmp = (m_ppuAddrTmp & 0xFC1F) | (static_cast<uint16_t>(data & 0x07) << 2);
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

            // Rendering scan lines
            if (m_scanLine < 240 && m_scanCycle > 0 && m_scanCycle < 257) {

                uint8_t pixel = (((m_backgroundShiftL << m_xFineScrolling) & 0x8000) >> 15) | (((m_backgroundShiftH << m_xFineScrolling) & 0x8000) >> 14);
                uint8_t color = mp_ppuBus->read(0x3F00 + pixel);

                // Update pixel color
                mp_frameBuffer->setPixel(m_scanCycle - 1, m_scanLine, color);
            }

            if (m_scanLine < 240 || m_scanLine == 261) {
                if ((m_scanCycle > 0 && m_scanCycle < 257) || (m_scanCycle > 320 && m_scanCycle < 337)) {
                    m_backgroundShiftH <<= 1;
                    m_backgroundShiftL <<= 1;
                }
            }

            // Address register Increment and updates
            if (m_scanLine < 240 || m_scanLine == 261) {
                // Increment X 
                if ((m_scanCycle % 8 == 0) && ((m_scanCycle > 0 && m_scanCycle < 257) || (m_scanCycle > 320 && m_scanCycle < 337))) {
                    uint8_t tile = mp_ppuBus->read(0x2000 | (m_ppuAddrCurrent & 0x0FFF));
                    uint16_t tileAddr = (m_ppuAddrCurrent >> 12) | static_cast<uint16_t>(tile) << 4;
                    tileAddr |= ((m_ppuCtrl & CTRL_BRG_PATTERN) != 0) << 12;

                    m_backgroundShiftH = (m_backgroundShiftH & 0xFF00) | (mp_ppuBus->read(tileAddr + 8));
                    m_backgroundShiftL = (m_backgroundShiftL & 0xFF00) | (mp_ppuBus->read(tileAddr));

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

         } // Rendering enable

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

        // int y = ((m_ppuAddrCurrent >> 2) & 0x00F8) | (m_ppuAddrCurrent >> 12);
        // int x = ((m_ppuAddrCurrent & 0x001F) << 3) | m_xFineScrolling;
        //
        // int yA = m_scanLine;// / 8;
        // int xA = m_scanCycle;// / 8;

        //std::cout << "Ax: " << xA << "; X: " << x << std::endl;
        //std::cout << "Ay: " << yA << "; Y: " << y << std::endl;
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
