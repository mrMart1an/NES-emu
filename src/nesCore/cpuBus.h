#ifndef BUS_H_
#define BUS_H_

#include "cpu/cpu6502.h"
#include "cartridge/cartridge.h"
#include "inputOutput/IOInterface.h"
#include "ppu/ppu.h"

#include <cstdint>
#include <string>

namespace nesCore {
// Nes cpu bus
class Bus {
public:
    Bus();

    // Attach a cartridge to the bus 
    void attachCartriadge(Cartridge* cartridge);
    // Attach an IO interface to the bus
    void attachIO(IOInterface* interface);
    // Attach ppu to the bus 
    void attachPpu(PPU* ppu);

    // Read and write functions
    void write(uint16_t addr, uint8_t data);
    uint8_t read (uint16_t addr, bool debugRead = false);

    void write16(uint16_t addr, uint16_t data);
    uint16_t read16(uint16_t addr, bool debugRead = false);
    void write16PageWrap(uint16_t addr, uint16_t data);
    uint16_t read16PageWrap(uint16_t addr, bool debugRead = false);

    // Return a sting with a formatted region of the bus
    // Take a memory range as input (both extreme are included)
    std::string formatRange(uint16_t from, uint16_t to, uint width);

public:
    // NES 6502 based CPU 
    Cpu6502 cpu;
    // NES PPU
    PPU* ppu;
    // NES RAM
    uint8_t ram[2048];

    Cartridge* cartridge;
    IOInterface* ioInterface;
};
}

#endif
