#ifndef NROM_CARTRIDGE_H_
#define NROM_CARTRIDGE_H_

#include "cartridge.h"
#include <cstdint>

namespace nesCore {
class NromCartridge : public Cartridge {
public:
    NromCartridge(uint8_t* p_prgRom, uint8_t* p_chrRom, CartridgeOption cartOpt);
    ~NromCartridge() override;

    // Write to a specific address of the cartridge
    void cpuWrite(uint16_t addr, uint8_t data) override;
    // Read to a specific address of the cartridge
    uint8_t cpuRead(uint16_t addr) override;
    // Read the byte of memory at the given address in CHR memory
    virtual uint8_t ppuRead(uint16_t addr) override;
    
    // Get the cartridge name table mirroring type
    virtual MirroringMode getMirroringMode() override;

private:
    uint8_t* mp_prgRom;
    uint8_t* mp_chrRom;

    // Number of install memory bank
    uint8_t m_banksCount;

    // Name table mirroring mode
    MirroringMode m_mirroringMode;
};
}

#endif
