#include "ppu.h"
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
                m_ppuAddrTmp = (m_ppuAddrTmp & 0xFE0F) | (static_cast<uint16_t>(data & 0x07) << 2);
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

// PPU processing 
Interrupt6502 PPU::clock(uint16_t cpuCycle) {
    uint16_t ppuCycle = cpuCycle * 3;
    Interrupt6502 outputInterrupt = NOINT;

    for (uint16_t i = 0; i < ppuCycle; i++) {
        // Check if rendering is enable
        if (m_ppuMask & (MASK_SHOW_SPR | MASK_SHOW_BRG)) {
            if (m_scanLine < 240 && m_scanCycle < 256) {
                uint8_t tile = mp_ppuBus->read(0x2000 | (m_ppuAddrCurrent & 0x0FFF));
                uint16_t tileAddr = (m_ppuAddrCurrent >> 12) | static_cast<uint16_t>(tile) << 4;
                tileAddr |= ((m_ppuCtrl & CTRL_BRG_PATTERN) != 0) << 12;

                uint8_t lowByte = mp_ppuBus->read(tileAddr);
                uint8_t highByte = mp_ppuBus->read(tileAddr + 8);

                uint8_t pixel = ((lowByte >> (7 - m_xFineScrolling)) & 0x01) | ((highByte >> (7 - m_xFineScrolling)) & 0x01) << 1;

                mp_frameBuffer->setPixel(m_scanCycle, m_scanLine, pixel);

                // Increment fine scroll X
                m_xFineScrolling++;
                if (m_xFineScrolling > 7) {
                    m_xFineScrolling = 0;

                    if ((m_ppuAddrCurrent & 0x001F) == 31) {
                        m_ppuAddrCurrent &= ~0x001F;
                        m_ppuAddrCurrent ^= 0x0400;
                    } else {
                        m_ppuAddrCurrent++;
                    }
                }
            }  

            // Copy address register horizontal components and Increment fine Y scrolling
            else if (m_scanCycle == 257) {
                m_ppuAddrCurrent = (m_ppuAddrCurrent & 0x7BE0) | (m_ppuAddrTmp & 0x841F);
                
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
    
            // Pre-rendering scan line 
            else if (m_scanLine == 260) {
                // Clear vblack flag
                if (m_scanCycle == 1)
                    m_ppuStatus &= ~STATUS_VBLACK;
                // Copy address register vertical components
                else if (m_scanCycle == 304)
                    m_ppuAddrCurrent = (m_ppuAddrCurrent & 0x841F) | (m_ppuAddrTmp & 0x7BE0);
            } 
        }

        // Set the vblack flag
        if (m_scanLine == 241 && m_scanCycle == 1) {
            m_vblank = true;
            outputInterrupt = NMI;
            m_ppuStatus |= STATUS_VBLACK;
        }
        
        // Increment scan cycle and scan line
        m_scanCycle += 1;
        if (m_scanCycle >= 340) {
            m_scanLine += 1;
            m_scanCycle = 0;

            if (m_scanLine >= 261)
                m_scanLine = 0;
        }


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
    bool vblanck = m_vblank;
    m_vblank = false;

    return vblanck;
}
}
