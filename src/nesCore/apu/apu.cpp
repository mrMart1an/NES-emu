#include "nesPch.h"

#include "nesCore/apu/apu.h"
#include <cstdint>

namespace nesCore {
APU::APU() {

}

uint8_t APU::readRegister(uint16_t addr) {
    // Pulse 1 channel
    if (addr >= 0x4000 && addr <= 0x4003) 
        return m_pulseOne[addr - 0x4000];
    // Pulse 2 channel
    else if (addr >= 0x4003 && addr <= 0x4007) 
        return m_pulseTwo[addr - 0x4004];
    // Triangle channel
    else if (addr >= 0x4008 && addr <= 0x400B) 
        return m_noise[addr - 0x4008];
    // Noice channel
    else if (addr >= 0x400C && addr <= 0x400F) 
        return m_noise[addr - 0x400C];
    // DMC channel
    else if (addr >= 0x4010 && addr <= 0x4013) 
        return m_DMC[addr - 0x4010];

    // Status register
    else if (addr == 0x4015) 
        return m_apuStatus;

    // Default return value
    return 0x00;
}

void APU::writeRegister(uint16_t addr, uint8_t data) {
    // Pulse 1 channel
    if (addr >= 0x4000 && addr <= 0x4003) 
        m_pulseOne[addr - 0x4000] = data;
    // Pulse 2 channel
    else if (addr >= 0x4003 && addr <= 0x4007) 
        m_pulseTwo[addr - 0x4004] = data;
    // Triangle channel
    else if (addr >= 0x4008 && addr <= 0x400B) 
        m_noise[addr - 0x4008] = data;
    // Noice channel
    else if (addr >= 0x400C && addr <= 0x400F) 
        m_noise[addr - 0x400C] = data;
    // DMC channel
    else if (addr >= 0x4010 && addr <= 0x4013) 
        m_DMC[addr - 0x4010] = data;

    // Status register
    else if (addr == 0x4015) 
        m_apuStatus = data;
    // Frame counter
    else if (addr == 0x4017) 
        m_frameCounter = data;
}
}
