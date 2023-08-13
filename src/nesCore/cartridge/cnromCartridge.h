#ifndef CNROM_CARTRIDGE_H_
#define CNROM_CARTRIDGE_H_

#include "cartridge.h"
#include <cstdint>

namespace nesCore {
class CnromCartridge : public Cartridge {
public:
    CnromCartridge(uint8_t* p_prgRom, uint8_t* p_chrRom, CartridgeOption cartOpt);
    ~CnromCartridge() override;

    // Write to a specific address of the cartridge
    void cpuWrite(uint16_t addr, uint8_t data) override;
    // Read to a specific address of the cartridge
    uint8_t cpuRead(uint16_t addr) override;
    // Read to a specific address in CHR memory
    uint8_t ppuRead(uint16_t addr) override;

    // Get the cartridge name table mirroring type
    virtual MirroringMode getMirroringMode() override;

private:
    uint8_t* mp_prgRom;
    uint8_t* mp_chrRom;

    // The pointer to the current chr memory bank
    uint8_t* mp_chrWindow;

    // Number of installed memory bank
    uint8_t m_prgBanksCount;
    uint8_t m_chrBanksCount;
    
    // Name table mirroring mode
    MirroringMode m_mirroringMode;
};
}

#endif
