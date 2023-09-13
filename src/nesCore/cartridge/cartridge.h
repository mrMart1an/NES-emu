#ifndef CARTRIDGE_H_
#define CARTRIDGE_H_

#include <cstdint>
#include <string>

namespace nesCore {

// Cartridge mirroring type
enum MirroringMode {
    HORIZONTAL_MIRRORING = 0,
    VERTICAL_MIRRORING = 1,
};

struct CartridgeOption {
    uint8_t prgBanksCount;
    uint8_t chrBanksCount;
    // 8Kb banks of prg RAM
    uint8_t prgRamBanksCount;

    MirroringMode mirroringMode;
};

class Cartridge {
public:
    Cartridge() {};
    virtual ~Cartridge() {};

    // Write to a specific address of the cartridge
    virtual void cpuWrite(uint16_t addr, uint8_t data) = 0;
    // Read to a specific address of the cartridge
    virtual uint8_t cpuRead(uint16_t addr) = 0;
    // Read the byte of memory at the given address in CHR memory
    virtual uint8_t ppuRead(uint16_t addr) = 0;

    // Get the cartridge name table mirroring type
    virtual MirroringMode getMirroringMode() = 0;
};
}

#endif
