#ifndef PPUBUS_H_
#define PPUBUS_H_
#include "../nesPch.h"

#include "cartridge/cartridge.h"
#include "ppu/ppu.h"

namespace nesCore {
class PpuBus {
public:
    PpuBus();

    // Attach a cartridge to the ppu nus
    void attachCartriadge(Cartridge* cartridge);

    // Read a byte from the PPU bus
    uint8_t read(uint16_t addr);
    // Write a byte to the PPU bus
    void write(uint16_t addr, uint8_t data);

private:
    // Set mirroring mode
    void setMirroringMode(MirroringMode mode);

// Public member variables
public:
    PPU ppu;

    uint8_t vram[2048];
    uint8_t palette[64];

    Cartridge* cartridge;

private:
    // Name table pointers
    uint8_t* mp_nametableOne;
    uint8_t* mp_nametableTwo;
    uint8_t* mp_nametableThree;
    uint8_t* mp_nametableFour;
};
}

#endif 
