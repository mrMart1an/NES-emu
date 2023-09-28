
#ifndef APU_H_
#define APU_H_

#include "nesPch.h"
#include <cstdint>

namespace nesCore {
class APU {
public:
    APU();

    // Register read and write operation
    uint8_t readRegister(uint16_t addr);
    void writeRegister(uint16_t addr, uint8_t data);

private:
    uint8_t m_apuStatus;
    uint8_t m_frameCounter;

    // Channels registers
    uint8_t m_pulseOne[4];
    uint8_t m_pulseTwo[4];
    uint8_t m_triangle[4];
    uint8_t m_noise[4];
    uint8_t m_DMC[4];
};
}

#endif
